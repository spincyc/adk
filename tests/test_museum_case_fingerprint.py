import hashlib
import importlib.util
import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parent.parent
SCRIPT = ROOT / "scripts/check_museum_case_resource_probe.py"
SPEC = importlib.util.spec_from_file_location("museum_resource_probe", SCRIPT)
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class MuseumCaseFingerprintTest(unittest.TestCase):
    @staticmethod
    def fingerprint(payload):
        return hashlib.sha256(MODULE.normalized(payload).encode("utf-8")).hexdigest()

    def test_environment_paths_are_portable(self):
        local = {
            "compiler": (
                "/home/ksh/.arduino15/packages/arduino/tools/avr-gcc/"
                "7.3.0/bin/avr-g++"
            ),
            "include": "-I/home/ksh/.arduino15/packages/arduino/core",
            "recipe": (
                "/home/ksh/.cache/arduino/sketches/ABC/example.elf "
                "-Wl,--gc-sections"
            ),
            "command": ["/usr/local/bin/arduino-cli", "compile", "-Os"],
        }
        runner = {
            "compiler": (
                "/home/runner/.arduino15/packages/arduino/tools/avr-gcc/"
                "7.3.0/bin/avr-g++"
            ),
            "include": "-I/home/runner/.arduino15/packages/arduino/core",
            "recipe": (
                "/home/runner/.cache/arduino/sketches/987/example.elf "
                "-Wl,--gc-sections"
            ),
            "command": ["/runner/_temp/bin/arduino-cli", "compile", "-Os"],
        }
        self.assertEqual(self.fingerprint(local), self.fingerprint(runner))

    def test_compiler_flags_remain_bound(self):
        payload = {"command": ["/usr/bin/arduino-cli", "compile", "-Os"]}
        changed = {"command": ["/usr/bin/arduino-cli", "compile", "-O0"]}
        self.assertNotEqual(self.fingerprint(payload), self.fingerprint(changed))

    def test_cli_wrapper_version_is_evidence_only(self):
        local = {
            "arduino_cli": "arduino-cli Version: 1.4.0",
            "compiler": "avr-g++ (GCC) 7.3.0",
        }
        runner = {
            "arduino_cli": "arduino-cli Version: 1.5.0",
            "compiler": "avr-g++ (GCC) 7.3.0",
        }
        self.assertEqual(
            self.fingerprint(MODULE.fingerprint_tools(local)),
            self.fingerprint(MODULE.fingerprint_tools(runner)),
        )

    def test_compiler_version_remains_bound(self):
        old = {
            "arduino_cli": "arduino-cli Version: 1.4.0",
            "compiler": "avr-g++ (GCC) 7.3.0",
        }
        changed = {
            "arduino_cli": "arduino-cli Version: 1.4.0",
            "compiler": "avr-g++ (GCC) 8.1.0",
        }
        self.assertNotEqual(
            self.fingerprint(MODULE.fingerprint_tools(old)),
            self.fingerprint(MODULE.fingerprint_tools(changed)),
        )


if __name__ == "__main__":
    unittest.main()
