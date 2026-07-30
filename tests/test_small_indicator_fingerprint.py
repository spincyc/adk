import importlib.util
import json
import pathlib
import tempfile
import unittest
from unittest import mock


ROOT = pathlib.Path(__file__).resolve().parent.parent
SCRIPT = ROOT / "scripts/check_small_indicator_resource_probe.py"
SPEC = importlib.util.spec_from_file_location("small_indicator_probe", SCRIPT)
small_indicator_probe = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(small_indicator_probe)


class SmallIndicatorFingerprintTest(unittest.TestCase):
    def test_exact_plan_gates_are_loaded(self):
        boundary = small_indicator_probe.BOUNDARY
        self.assertEqual(boundary["lesson"], "080")
        self.assertEqual(boundary["flash_target"], 14336)
        self.assertEqual(boundary["flash_hard"], 20480)
        self.assertEqual(boundary["sram_target"], 1024)
        self.assertEqual(boundary["sram_hard"], 1536)
        self.assertEqual(boundary["stack_target"], 448)
        self.assertEqual(boundary["stack_hard"], 640)
        self.assertEqual(boundary["object_target"], 384)
        self.assertEqual(boundary["object_hard"], 512)
        self.assertEqual(boundary["evidence_target"], 256)
        self.assertEqual(boundary["evidence_hard"], 384)

    def test_fingerprint_closure_is_lesson_isolated(self):
        paths = small_indicator_probe.fingerprint_source_paths()
        self.assertIn("src/small_indicator_semantics_policy.cpp", paths)
        self.assertIn("src/bounded_low_side_driver_policy.cpp", paths)
        self.assertIn("src/status.h", paths)
        self.assertIn("src/time.h", paths)
        self.assertIn(
            "probes/component_qualification_boundary_080.json", paths
        )
        self.assertFalse(any("081" in path for path in paths))

    def test_source_edit_changes_fingerprint_input(self):
        with mock.patch.object(
            small_indicator_probe.probe,
            "sha256",
            side_effect=lambda path: str(path),
        ):
            hashes = small_indicator_probe.fingerprint_source_hashes()
        self.assertEqual(
            set(hashes), set(small_indicator_probe.fingerprint_source_paths())
        )

    def test_review_registry_starts_empty_and_fails_closed(self):
        self.assertEqual(
            small_indicator_probe.load_reviews(
                ROOT, "probes/small_indicator_resource_reviews.json"
            ),
            {},
        )
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            review = root / "reviews.json"
            review.write_text(
                json.dumps({"reviews": [{"lesson": "080"}], "schema": 1}),
                encoding="utf-8",
            )
            with self.assertRaises(small_indicator_probe.probe.ProbeError):
                small_indicator_probe.load_reviews(root, review.name)

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
                    "lesson": "080",
                    "measurements": {
                        "flash_bytes": 15000,
                        "object_bytes": 380,
                        "static_sram_bytes": 700,
                        "synchronous_stack_bytes": 400,
                    },
                    "status": "review-required",
                }
            ],
            "commands": [["/tmp/avr-g++"]],
            "constants": {},
            "fqbn": "arduino:avr:mega",
            "status": "review-required",
            "tools": {},
        }
        stale = {
            "authority": (
                "docs/design/"
                "LESSON_080_SMALL_INDICATOR_SEMANTICS_STRESS_PASS.md"
                "#terminal-gate-result"
            ),
            "disposition": "accepted-target-miss",
            "fingerprint_sha256": "0" * 64,
            "hard_bytes": 20480,
            "lesson": "080",
            "metric": "flash",
            "observed_bytes": 15000,
            "rationale": "stale fixture",
            "target_bytes": 14336,
        }
        with tempfile.TemporaryDirectory() as directory:
            evidence = pathlib.Path(directory) / "evidence.json"
            evidence.write_text(json.dumps(report), encoding="utf-8")
            with (
                mock.patch.dict(
                    small_indicator_probe.RESOURCE_LAYOUT,
                    {
                        "smallIndicatorObservationBytes": 200,
                        "smallIndicatorSemanticRequestBytes": 180,
                        "smallIndicatorSemanticResultBytes": 160,
                    },
                    clear=True,
                ),
                mock.patch.dict(
                    small_indicator_probe.REVIEWS,
                    {"flash": stale},
                    clear=True,
                ),
                mock.patch.object(
                    small_indicator_probe,
                    "fingerprint_source_hashes",
                    return_value={},
                ),
                self.assertRaises(small_indicator_probe.probe.ProbeError),
            ):
                small_indicator_probe.enrich_evidence(evidence)

    def test_evidence_gate_uses_upper_limit_semantics(self):
        gate = small_indicator_probe.probe.gate
        self.assertEqual(gate(256, 256, 384), "pass")
        self.assertEqual(gate(257, 256, 384), "target-miss")
        self.assertEqual(gate(385, 256, 384), "hard-fail")


if __name__ == "__main__":
    unittest.main()
