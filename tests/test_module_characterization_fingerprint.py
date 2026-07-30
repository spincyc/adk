import importlib.util
import pathlib
import tempfile
import unittest
from unittest import mock


ROOT = pathlib.Path(__file__).resolve().parent.parent
SCRIPT = ROOT / "scripts/check_module_characterization_resource_probe.py"
SPEC = importlib.util.spec_from_file_location("module_probe", SCRIPT)
module_probe = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(module_probe)


class ModuleCharacterizationFingerprintTest(unittest.TestCase):
    def test_boundaries_are_enabled_in_dependency_order(self):
        self.assertEqual(
            [item["lesson"] for item in module_probe.selected_boundaries("070")],
            ["070"],
        )
        self.assertEqual(
            [item["lesson"] for item in module_probe.selected_boundaries("071")],
            ["070", "071"],
        )

    def test_l070_config_is_owned_independently(self):
        self.assertEqual(
            module_probe.fingerprint_source_paths("070")[2],
            "probes/module_characterization_boundary_070.json",
        )
        changed = (
            module_probe.BOUNDARY_CONFIG_PATHS["070"]
            .read_text(encoding="utf-8")
            .replace('"flash_target": 10240', '"flash_target": 10241')
        )
        self.assertNotEqual(
            module_probe.boundary_configuration_hash("070"),
            module_probe.boundary_configuration_hash("070", changed),
        )

    def test_shared_algorithm_edit_changes_shared_hash(self):
        self.assertNotEqual(
            module_probe.shared_probe_source_hash("shared algorithm v1"),
            module_probe.shared_probe_source_hash("shared algorithm v2"),
        )

    def test_l071_chains_from_l070_without_enabling_l072(self):
        paths = module_probe.fingerprint_source_paths("070")
        self.assertFalse(any("071" in path or "072" in path for path in paths))
        paths = module_probe.fingerprint_source_paths("071")
        self.assertIn("src/module_threshold_descriptor.cpp", paths)
        self.assertIn("src/module_characterization.cpp", paths)
        self.assertIn(
            "probes/module_characterization_boundary_070.json", paths
        )
        self.assertIn(
            "probes/module_characterization_boundary_071.json", paths
        )
        self.assertFalse(any("072" in path for path in paths))
        with self.assertRaises(ValueError):
            module_probe.fingerprint_source_paths("072")

    def test_l071_fingerprint_changes_with_l070_configuration(self):
        changed_070 = (
            module_probe.BOUNDARY_CONFIG_PATHS["070"]
            .read_text(encoding="utf-8")
            .replace('"flash_target": 10240', '"flash_target": 10241')
        )
        with mock.patch.object(
            module_probe.probe, "sha256", return_value="source-hash"
        ):
            original = module_probe.fingerprint_source_hashes("071")
            changed = module_probe.fingerprint_source_hashes(
                "071", {"070": changed_070}
            )
        self.assertNotEqual(
            original["probes/module_characterization_boundary_070.json"],
            changed["probes/module_characterization_boundary_070.json"],
        )

    def test_exact_plan_gates_are_loaded(self):
        boundary = module_probe.ALL_BOUNDARIES[0]
        self.assertEqual(
            {
                key: boundary[key]
                for key in (
                    "flash_target",
                    "flash_hard",
                    "sram_target",
                    "sram_hard",
                    "stack_target",
                    "stack_hard",
                    "object_target",
                    "object_hard",
                )
            },
            {
                "flash_target": 10240,
                "flash_hard": 14336,
                "sram_target": 768,
                "sram_hard": 1024,
                "stack_target": 320,
                "stack_hard": 448,
                "object_target": 192,
                "object_hard": 256,
            },
        )

        boundary = module_probe.ALL_BOUNDARIES[1]
        self.assertEqual(
            {
                key: boundary[key]
                for key in (
                    "flash_target",
                    "flash_hard",
                    "sram_target",
                    "sram_hard",
                    "stack_target",
                    "stack_hard",
                    "object_target",
                    "object_hard",
                )
            },
            {
                "flash_target": 12288,
                "flash_hard": 16384,
                "sram_target": 1024,
                "sram_hard": 1536,
                "stack_target": 448,
                "stack_hard": 640,
                "object_target": 512,
                "object_hard": 768,
            },
        )

    def test_l071_layout_gates_match_the_plan(self):
        self.assertEqual(module_probe.EVIDENCE_TARGET, 320)
        self.assertEqual(module_probe.EVIDENCE_HARD, 384)
        self.assertEqual(module_probe.POINT_TARGET, 96)
        self.assertEqual(module_probe.POINT_HARD, 128)

    def test_residual_sram_uses_minimum_gate_semantics(self):
        self.assertEqual(module_probe.minimum_gate(4096, 4096, 3072), "pass")
        self.assertEqual(
            module_probe.minimum_gate(4095, 4096, 3072), "target-miss"
        )
        self.assertEqual(
            module_probe.minimum_gate(3072, 4096, 3072), "target-miss"
        )
        self.assertEqual(
            module_probe.minimum_gate(3071, 4096, 3072), "hard-fail"
        )

    def test_missing_evidence_is_an_error(self):
        with tempfile.TemporaryDirectory() as directory:
            missing = pathlib.Path(directory) / "missing.json"
            self.assertEqual(module_probe.enrich_evidence(missing), 1)


if __name__ == "__main__":
    unittest.main()
