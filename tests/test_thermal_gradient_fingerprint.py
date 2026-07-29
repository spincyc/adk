import importlib.util
import pathlib
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parent.parent
SCRIPT = ROOT / "scripts/check_thermal_gradient_resource_probe.py"
SPEC = importlib.util.spec_from_file_location("thermal_probe", SCRIPT)
thermal_probe = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(thermal_probe)


class ThermalGradientFingerprintPortabilityTest(unittest.TestCase):
    def test_canonicalizes_equivalent_local_and_ci_paths(self):
        local = {
            "commands": [
                [
                    "/home/local/.arduino15/packages/arduino/tools/"
                    "avr-gcc/7.3.0/bin/avr-g++",
                    "-I/home/local/.arduino15/packages/arduino/hardware/"
                    "avr/1.8.8/cores/arduino",
                    "/home/local/.cache/arduino/sketches/AB12/image.elf",
                    "/usr/bin/c++",
                ]
            ],
            "linker": (
                "\"/home/local/.arduino15/packages/arduino/tools/"
                "avr-gcc/7.3.0/bin/avr-gcc\" "
                "\"-L/home/local/.cache/arduino/sketches/AB12\""
            ),
        }
        ci = {
            "commands": [
                [
                    "/home/runner/.arduino15/packages/arduino/tools/"
                    "avr-gcc/7.3.0/bin/avr-g++",
                    "-I/home/runner/.arduino15/packages/arduino/hardware/"
                    "avr/1.8.8/cores/arduino",
                    "/home/runner/.cache/arduino/sketches/CD34/image.elf",
                    "/opt/host/bin/c++",
                ]
            ],
            "linker": (
                "\"/home/runner/.arduino15/packages/arduino/tools/"
                "avr-gcc/7.3.0/bin/avr-gcc\" "
                "\"-L/home/runner/.cache/arduino/sketches/CD34\""
            ),
        }
        local_markers = {
            "/home/local/.arduino15/packages/arduino/tools/avr-gcc/7.3.0":
                "<avr-toolchain>",
            "/home/local/.arduino15/packages/arduino/hardware/avr/1.8.8":
                "<arduino-avr-platform>",
            "/usr/bin/c++": "<host-cxx>",
        }
        ci_markers = {
            "/home/runner/.arduino15/packages/arduino/tools/avr-gcc/7.3.0":
                "<avr-toolchain>",
            "/home/runner/.arduino15/packages/arduino/hardware/avr/1.8.8":
                "<arduino-avr-platform>",
            "/opt/host/bin/c++": "<host-cxx>",
        }
        self.assertEqual(
            thermal_probe.canonical_review_value(local, local_markers),
            thermal_probe.canonical_review_value(ci, ci_markers),
        )

    def test_canonicalization_preserves_semantic_flag_changes(self):
        markers = {"/home/local/toolchain": "<avr-toolchain>"}
        ordinary = ["/home/local/toolchain/bin/avr-g++", "-Os", "-flto"]
        changed = ["/home/local/toolchain/bin/avr-g++", "-Os", "-fno-lto"]
        self.assertNotEqual(
            thermal_probe.canonical_review_value(ordinary, markers),
            thermal_probe.canonical_review_value(changed, markers),
        )

    def test_review_tools_retain_compiler_and_binutils_not_cli_wrapper(self):
        tools = {
            "arduino_cli": "arduino-cli Version: 1.5.0",
            "compiler": "avr-g++ (GCC) 7.3.0",
            "nm": "GNU nm (GNU Binutils) 2.26.20160125",
        }
        self.assertEqual(
            thermal_probe.review_tool_identities(tools),
            {
                "compiler": "avr-g++ (GCC) 7.3.0",
                "nm": "GNU nm (GNU Binutils) 2.26.20160125",
            },
        )

    def test_actual_compile_units_sort_after_path_canonicalization(self):
        local_root = (
            "/home/z-local/.arduino15/packages/arduino/hardware/avr/1.8.8"
        )
        ci_root = (
            "/opt/arduino/packages/arduino/hardware/avr/1.8.8"
        )
        local_units = [
            {
                "file": f"{local_root}/cores/arduino/Zeta.cpp",
                "arguments": ["avr-g++", f"-I{local_root}/cores/arduino"],
            },
            {
                "file": "/workspace/adk/src/alpha.cpp",
                "arguments": ["avr-g++", "-I/workspace/adk/src"],
            },
        ]
        ci_units = list(reversed([
            {
                "file": f"{ci_root}/cores/arduino/Zeta.cpp",
                "arguments": ["avr-g++", f"-I{ci_root}/cores/arduino"],
            },
            {
                "file": "/home/runner/work/adk/src/alpha.cpp",
                "arguments": ["avr-g++", "-I/home/runner/work/adk/src"],
            },
        ]))
        prior_root = thermal_probe.ROOT
        prior_markers = thermal_probe.REVIEW_PATH_MARKERS
        try:
            thermal_probe.ROOT = pathlib.Path("/workspace/adk")
            thermal_probe.REVIEW_PATH_MARKERS = {
                local_root: "<arduino-avr-platform>"
            }
            local = thermal_probe.canonical_compile_units(local_units)
            thermal_probe.ROOT = pathlib.Path("/home/runner/work/adk")
            thermal_probe.REVIEW_PATH_MARKERS = {
                ci_root: "<arduino-avr-platform>"
            }
            ci = thermal_probe.canonical_compile_units(ci_units)
        finally:
            thermal_probe.ROOT = prior_root
            thermal_probe.REVIEW_PATH_MARKERS = prior_markers
        self.assertEqual(local, ci)

    def test_actual_dependency_shape_canonicalizes_sketch_manifest_root(self):
        local = {
            "manifests": [
                {
                    "path": "sketch/Lesson064.ino.cpp.d",
                    "dependencies": [
                        "/home/local/.cache/arduino/sketches/AB12/"
                        "sketch/Lesson064.ino.cpp",
                        "/workspace/adk/src/one_wire_transaction_policy.h",
                    ],
                }
            ],
            "dependency_hashes": {
                "/home/local/.cache/arduino/sketches/AB12/"
                "sketch/Lesson064.ino.cpp": "abc",
                "/workspace/adk/src/one_wire_transaction_policy.h": "def",
            },
        }
        ci = {
            "manifests": [
                {
                    "path": "sketch/Lesson064.ino.cpp.d",
                    "dependencies": [
                        "/runner/adk/src/one_wire_transaction_policy.h",
                        "/home/runner/.cache/arduino/sketches/CD34/"
                        "sketch/Lesson064.ino.cpp",
                    ],
                }
            ],
            "dependency_hashes": {
                "/runner/adk/src/one_wire_transaction_policy.h": "def",
                "/home/runner/.cache/arduino/sketches/CD34/"
                "sketch/Lesson064.ino.cpp": "abc",
            },
        }
        local_markers = {"/workspace/adk": "<repo>"}
        ci_markers = {"/runner/adk": "<repo>"}
        local_value = thermal_probe.canonical_review_value(
            local, local_markers
        )
        ci_value = thermal_probe.canonical_review_value(ci, ci_markers)
        local_value["manifests"][0]["dependencies"].sort()
        ci_value["manifests"][0]["dependencies"].sort()
        self.assertEqual(local_value, ci_value)

    def test_generated_sketch_hash_normalizes_line_paths_not_source(self):
        local_source = (
            '#line 1 "/workspace/adk/examples/Lesson064/Lesson064.ino"\n'
            "void setup() { beginTransaction(); }\n"
        )
        ci_source = (
            '#line 1 "/home/runner/work/adk/examples/Lesson064/'
            'Lesson064.ino"\n'
            "void setup() { beginTransaction(); }\n"
        )
        changed_source = ci_source.replace(
            "beginTransaction()", "cancelTransaction()"
        )
        prior_root = thermal_probe.ROOT
        try:
            with tempfile.TemporaryDirectory() as directory:
                sketch = pathlib.Path(directory) / "Lesson064.ino.cpp"
                thermal_probe.ROOT = pathlib.Path("/workspace/adk")
                sketch.write_text(local_source, encoding="utf-8")
                local_hash = thermal_probe.dependency_sha256(sketch)
                thermal_probe.ROOT = pathlib.Path("/home/runner/work/adk")
                sketch.write_text(ci_source, encoding="utf-8")
                ci_hash = thermal_probe.dependency_sha256(sketch)
                sketch.write_text(changed_source, encoding="utf-8")
                changed_hash = thermal_probe.dependency_sha256(sketch)
        finally:
            thermal_probe.ROOT = prior_root
        self.assertEqual(local_hash, ci_hash)
        self.assertNotEqual(ci_hash, changed_hash)

    def test_cli_orchestration_intermediate_is_not_semantic_dependency(self):
        canonical_source = pathlib.Path(
            "Lesson064OwnedSingleWireTransactions.ino.cpp"
        )
        cli_intermediate = pathlib.Path(
            "Lesson064OwnedSingleWireTransactions.ino.cpp.merged"
        )
        self.assertFalse(
            thermal_probe.is_orchestration_dependency(canonical_source)
        )
        self.assertTrue(
            thermal_probe.is_orchestration_dependency(cli_intermediate)
        )

    def test_cli_merged_manifest_record_is_ignored_by_path_or_target(self):
        ordinary = pathlib.Path(
            "sketch/Lesson064OwnedSingleWireTransactions.ino.cpp.d"
        )
        merged = pathlib.Path(
            "sketch/Lesson064OwnedSingleWireTransactions.ino.cpp.merged.d"
        )
        ordinary_target = (
            "sketch/Lesson064OwnedSingleWireTransactions.ino.cpp.o"
        )
        merged_target = (
            "sketch/Lesson064OwnedSingleWireTransactions.ino.cpp.merged.o"
        )
        self.assertFalse(
            thermal_probe.is_orchestration_manifest(
                ordinary, ordinary_target
            )
        )
        self.assertTrue(
            thermal_probe.is_orchestration_manifest(merged, ordinary_target)
        )
        self.assertTrue(
            thermal_probe.is_orchestration_manifest(ordinary, merged_target)
        )

    def test_cli_library_discovery_manifest_is_ignored_not_real_compile(self):
        discovery = pathlib.Path(
            "libraries/adk/one_wire_transaction_policy.cpp.libsdetect.d"
        )
        compiled = pathlib.Path(
            "libraries/adk/one_wire_transaction_policy.cpp.d"
        )
        target = "libraries/adk/one_wire_transaction_policy.cpp.o"
        self.assertTrue(
            thermal_probe.is_orchestration_manifest(discovery, target)
        )
        self.assertFalse(
            thermal_probe.is_orchestration_manifest(compiled, target)
        )


if __name__ == "__main__":
    unittest.main()
