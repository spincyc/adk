import importlib.util
import json
import pathlib
import tempfile
import unittest
from unittest import mock


ROOT = pathlib.Path(__file__).resolve().parent.parent
SCRIPT = ROOT / "scripts/check_component_qualification_resource_probe.py"
SPEC = importlib.util.spec_from_file_location("component_probe", SCRIPT)
component_probe = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(component_probe)


class ComponentQualificationFingerprintTest(unittest.TestCase):
    def test_exact_plan_gates_are_loaded(self):
        self.assertEqual(component_probe.BOUNDARY["lesson"], "079")
        self.assertEqual(component_probe.BOUNDARY["flash_target"], 10240)
        self.assertEqual(component_probe.BOUNDARY["flash_hard"], 14336)
        self.assertEqual(component_probe.BOUNDARY["sram_target"], 768)
        self.assertEqual(component_probe.BOUNDARY["sram_hard"], 1024)
        self.assertEqual(component_probe.BOUNDARY["stack_target"], 320)
        self.assertEqual(component_probe.BOUNDARY["stack_hard"], 448)
        self.assertEqual(component_probe.BOUNDARY["object_target"], 192)
        self.assertEqual(component_probe.BOUNDARY["object_hard"], 256)
        self.assertEqual(component_probe.DESCRIPTOR_TARGET, 96)
        self.assertEqual(component_probe.DESCRIPTOR_HARD, 128)
        self.assertEqual(component_probe.RESIDUAL_SRAM_TARGET, 4096)
        self.assertEqual(component_probe.RESIDUAL_SRAM_HARD, 3072)

    def test_fingerprint_closure_is_lesson_isolated(self):
        paths = component_probe.fingerprint_source_paths()
        self.assertIn("src/bounded_low_side_driver_policy.cpp", paths)
        self.assertIn(
            "probes/component_qualification_boundary_079.json", paths
        )
        self.assertFalse(any("080" in path or "081" in path for path in paths))

    def test_source_edit_changes_fingerprint_input(self):
        with mock.patch.object(
            component_probe.probe, "sha256", side_effect=lambda path: str(path)
        ):
            hashes = component_probe.fingerprint_source_hashes()
        self.assertEqual(set(hashes), set(component_probe.fingerprint_source_paths()))

    def test_review_registry_starts_empty_and_fails_closed(self):
        self.assertEqual(
            component_probe.load_reviews(
                ROOT, "probes/component_qualification_resource_reviews.json"
            ),
            {},
        )
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            review = root / "reviews.json"
            review.write_text(
                json.dumps({"reviews": [{"lesson": "079"}], "schema": 1}),
                encoding="utf-8",
            )
            with self.assertRaises(component_probe.probe.ProbeError):
                component_probe.load_reviews(root, review.name)

    def test_stale_fingerprint_review_is_rejected_during_enrichment(self):
        report = {
            "boundaries": [
                {
                    "accepted_reviews": [],
                    "gates": {
                        "flash": "target-miss",
                        "object": "pass",
                        "static_sram": "pass",
                        "synchronous_stack": "pass",
                    },
                    "lesson": "079",
                    "measurements": {
                        "flash_bytes": 10730,
                        "object_bytes": 190,
                        "static_sram_bytes": 566,
                        "synchronous_stack_bytes": 300,
                    },
                    "status": "review-required",
                }
            ],
            "constants": {},
            "status": "review-required",
            "tools": {},
        }
        stale = {
            "authority": (
                "docs/design/"
                "LESSON_079_BOUNDED_LOW_SIDE_DRIVER_STRESS_PASS.md"
                "#terminal-gate-result"
            ),
            "disposition": "accepted-target-miss",
            "fingerprint_sha256": "0" * 64,
            "hard_bytes": 14336,
            "lesson": "079",
            "metric": "flash",
            "observed_bytes": 10730,
            "rationale": "stale fixture",
            "target_bytes": 10240,
        }
        with tempfile.TemporaryDirectory() as directory:
            evidence = pathlib.Path(directory) / "evidence.json"
            evidence.write_text(json.dumps(report), encoding="utf-8")
            with (
                mock.patch.dict(
                    component_probe.RESOURCE_LAYOUT,
                    {"lowSideDriverDescriptorBytes": 96},
                    clear=True,
                ),
                mock.patch.dict(
                    component_probe.REVIEWS, {"flash": stale}, clear=True
                ),
                mock.patch.object(
                    component_probe, "fingerprint_source_hashes", return_value={}
                ),
                self.assertRaises(component_probe.probe.ProbeError),
            ):
                component_probe.enrich_evidence(evidence)

    def test_descriptor_gate_uses_upper_limit_semantics(self):
        self.assertEqual(component_probe.probe.gate(96, 96, 128), "pass")
        self.assertEqual(
            component_probe.probe.gate(97, 96, 128), "target-miss"
        )
        self.assertEqual(
            component_probe.probe.gate(129, 96, 128), "hard-fail"
        )

    def test_residual_sram_uses_minimum_limit_semantics(self):
        self.assertEqual(
            component_probe.residual_sram_gate(4096), "pass"
        )
        self.assertEqual(
            component_probe.residual_sram_gate(3072), "target-miss"
        )
        self.assertEqual(
            component_probe.residual_sram_gate(3071), "hard-fail"
        )


if __name__ == "__main__":
    unittest.main()
