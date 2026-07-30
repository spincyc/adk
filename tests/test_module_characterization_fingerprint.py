import importlib.util
import json
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

    def test_failed_base_gate_is_still_fully_enriched(self):
        report = {
            "boundaries": [
                {
                    "gates": {
                        "flash": "pass",
                        "object": "pass",
                        "static_sram": "target-miss",
                        "synchronous_stack": "pass",
                    },
                    "lesson": "071",
                    "measurements": {
                        "flash_bytes": 12000,
                        "object_bytes": 700,
                        "static_sram_bytes": 1100,
                        "synchronous_stack_bytes": 500,
                    },
                    "status": "review-required",
                }
            ],
            "constants": {},
            "status": "review-required",
            "tools": {},
        }
        symbols = {
            "moduleThresholdDescriptorBytes": 56,
            "moduleThresholdFrameBytes": 56,
            "moduleCharacterizationEvidenceBytes": 391,
            "moduleCharacterizationPointBytes": 80,
        }
        with tempfile.TemporaryDirectory() as directory:
            evidence = pathlib.Path(directory) / "evidence.json"
            evidence.write_text(json.dumps(report), encoding="utf-8")
            with mock.patch.dict(
                module_probe.RESOURCE_LAYOUTS, {"071": symbols}, clear=True
            ):
                result = module_probe.enrich_evidence(evidence)
            enriched = json.loads(evidence.read_text(encoding="utf-8"))

        boundary = enriched["boundaries"][0]
        self.assertEqual(result, 1)
        self.assertEqual(boundary["measurements"]["evidence_bytes"], 391)
        self.assertEqual(boundary["gates"]["evidence"], "hard-fail")
        self.assertEqual(boundary["gates"]["static_sram"], "target-miss")
        self.assertEqual(boundary["gates"]["residual_sram"], "pass")
        self.assertIn("fingerprint_sha256", boundary)
        self.assertIn("fingerprint_source_hashes", boundary)
        self.assertEqual(boundary["status"], "hard-fail")
        self.assertEqual(enriched["status"], "hard-fail")

    def test_review_loader_accepts_only_the_two_authoritative_tuples(self):
        review_path = (
            ROOT / "probes/module_characterization_resource_reviews.json"
        )
        with mock.patch.object(
            module_probe.probe,
            "BOUNDARIES",
            module_probe.selected_boundaries("071"),
        ):
            loaded = module_probe.load_reviews(ROOT, review_path)
        self.assertEqual(loaded, {})
        self.assertEqual(
            set(module_probe.ENRICHED_REVIEWS),
            {("071", "static_sram"), ("071", "evidence")},
        )

    def test_stale_enriched_review_fails_closed(self):
        report = {
            "boundaries": [
                {
                    "accepted_reviews": [],
                    "gates": {
                        "flash": "pass",
                        "object": "pass",
                        "static_sram": "target-miss",
                        "synchronous_stack": "pass",
                    },
                    "lesson": "071",
                    "measurements": {
                        "flash_bytes": 11206,
                        "object_bytes": 483,
                        "static_sram_bytes": 1145,
                        "synchronous_stack_bytes": 339,
                    },
                    "status": "review-required",
                }
            ],
            "constants": {},
            "status": "review-required",
            "tools": {},
        }
        symbols = {
            "moduleThresholdDescriptorBytes": 45,
            "moduleThresholdFrameBytes": 38,
            "moduleCharacterizationEvidenceBytes": 375,
            "moduleCharacterizationPointBytes": 57,
        }
        stale_review = {
            "authority": (
                "docs/design/"
                "LESSON_071_THRESHOLD_CHARACTERIZATION_STRESS_PASS.md"
                "#gate-result"
            ),
            "disposition": "accepted-target-miss",
            "fingerprint_sha256": "0" * 64,
            "hard_bytes": 1536,
            "lesson": "071",
            "metric": "static_sram",
            "observed_bytes": 1145,
            "rationale": "stale fixture",
            "target_bytes": 1024,
        }
        with tempfile.TemporaryDirectory() as directory:
            evidence = pathlib.Path(directory) / "evidence.json"
            evidence.write_text(json.dumps(report), encoding="utf-8")
            with (
                mock.patch.dict(
                    module_probe.RESOURCE_LAYOUTS,
                    {"071": symbols},
                    clear=True,
                ),
                mock.patch.dict(
                    module_probe.ENRICHED_REVIEWS,
                    {("071", "static_sram"): stale_review},
                    clear=True,
                ),
                self.assertRaises(module_probe.probe.ProbeError),
            ):
                module_probe.enrich_evidence(evidence)

    def test_exact_enriched_reviews_resolve_target_misses(self):
        report = {
            "boundaries": [
                {
                    "accepted_reviews": [],
                    "gates": {
                        "flash": "pass",
                        "object": "pass",
                        "static_sram": "target-miss",
                        "synchronous_stack": "pass",
                    },
                    "lesson": "071",
                    "measurements": {
                        "flash_bytes": 11452,
                        "object_bytes": 498,
                        "static_sram_bytes": 1160,
                        "synchronous_stack_bytes": 339,
                    },
                    "status": "review-required",
                }
            ],
            "constants": {},
            "status": "review-required",
            "tools": {},
        }
        symbols = {
            "moduleThresholdDescriptorBytes": 45,
            "moduleThresholdFrameBytes": 38,
            "moduleCharacterizationEvidenceBytes": 375,
            "moduleCharacterizationPointBytes": 57,
        }
        with tempfile.TemporaryDirectory() as directory:
            evidence = pathlib.Path(directory) / "evidence.json"
            evidence.write_text(json.dumps(report), encoding="utf-8")
            with (
                mock.patch.dict(
                    module_probe.RESOURCE_LAYOUTS,
                    {"071": symbols},
                    clear=True,
                ),
                mock.patch.dict(
                    module_probe.ENRICHED_REVIEWS, {}, clear=True
                ),
            ):
                self.assertEqual(module_probe.enrich_evidence(evidence), 1)
            first = json.loads(evidence.read_text(encoding="utf-8"))
            fingerprint = first["boundaries"][0]["fingerprint_sha256"]
            reviews = {}
            for metric, observed, target, hard in (
                ("static_sram", 1160, 1024, 1536),
                ("evidence", 375, 320, 384),
            ):
                reviews[("071", metric)] = {
                    "authority": "authority",
                    "disposition": "accepted-target-miss",
                    "fingerprint_sha256": fingerprint,
                    "hard_bytes": hard,
                    "lesson": "071",
                    "metric": metric,
                    "observed_bytes": observed,
                    "rationale": "exact fixture",
                    "target_bytes": target,
                }
            evidence.write_text(json.dumps(report), encoding="utf-8")
            with (
                mock.patch.dict(
                    module_probe.RESOURCE_LAYOUTS,
                    {"071": symbols},
                    clear=True,
                ),
                mock.patch.dict(
                    module_probe.ENRICHED_REVIEWS, reviews, clear=True
                ),
            ):
                self.assertEqual(module_probe.enrich_evidence(evidence), 0)
            accepted = json.loads(evidence.read_text(encoding="utf-8"))

        boundary = accepted["boundaries"][0]
        self.assertEqual(boundary["status"], "reviewed-target-miss")
        self.assertEqual(accepted["status"], "reviewed-target-miss")
        self.assertEqual(
            {
                boundary["gates"]["static_sram"],
                boundary["gates"]["evidence"],
            },
            {"reviewed-target-miss"},
        )


if __name__ == "__main__":
    unittest.main()
