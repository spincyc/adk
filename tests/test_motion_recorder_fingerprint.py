import importlib.util
import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parent.parent
SCRIPT = ROOT / "scripts/check_motion_recorder_resource_probe.py"
SPEC = importlib.util.spec_from_file_location("motion_probe", SCRIPT)
motion_probe = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(motion_probe)


class MotionRecorderFingerprintTest(unittest.TestCase):
    def test_require_through_selects_exact_boundary_prefix(self):
        self.assertEqual(
            [item["lesson"] for item in motion_probe.selected_boundaries("067")],
            ["067"],
        )
        self.assertEqual(
            [item["lesson"] for item in motion_probe.selected_boundaries("069")],
            ["067", "068", "069"],
        )

    def test_each_boundary_self_hashes_probe_implementation(self):
        script = "scripts/check_motion_recorder_resource_probe.py"
        for lesson in ("067", "068", "069"):
            self.assertIn(script, motion_probe.fingerprint_source_paths(lesson))

    def test_l069_limit_edit_does_not_change_earlier_config_hashes(self):
        baseline = {
            lesson: motion_probe.fingerprint_source_hashes(lesson)
            for lesson in ("067", "068", "069")
        }
        changed_text = (
            motion_probe.BOUNDARY_CONFIG_PATHS["069"]
            .read_text(encoding="utf-8")
            .replace('"flash_target": 32768', '"flash_target": 32769')
        )
        changed = {
            lesson: motion_probe.fingerprint_source_hashes(
                lesson, {"069": changed_text}
            )
            for lesson in ("067", "068", "069")
        }
        self.assertEqual(baseline["067"], changed["067"])
        self.assertEqual(baseline["068"], changed["068"])
        self.assertNotEqual(baseline["069"], changed["069"])

    def test_shared_algorithm_edit_changes_every_boundary_projection(self):
        baseline = motion_probe.shared_probe_source_hash("shared algorithm v1")
        changed = motion_probe.shared_probe_source_hash("shared algorithm v2")
        self.assertNotEqual(baseline, changed)
        for lesson in ("067", "068", "069"):
            hashes = motion_probe.fingerprint_source_hashes(lesson)
            self.assertEqual(
                hashes["scripts/check_motion_recorder_resource_probe.py"],
                motion_probe.shared_probe_source_hash(),
            )

    def test_later_components_do_not_buckle_earlier_source_scope(self):
        self.assertNotIn(
            "src/inertial_record_qualification.cpp",
            motion_probe.fingerprint_source_paths("067"),
        )
        self.assertIn(
            "src/inertial_record_qualification.cpp",
            motion_probe.fingerprint_source_paths("068"),
        )
        self.assertNotIn(
            "src/qualified_motion_recorder.cpp",
            motion_probe.fingerprint_source_paths("068"),
        )
        self.assertIn(
            "src/qualified_motion_recorder.cpp",
            motion_probe.fingerprint_source_paths("069"),
        )

    def test_live_source_hashes_match_each_scoped_path(self):
        for lesson in ("067", "068", "069"):
            hashes = motion_probe.fingerprint_source_hashes(lesson)
            self.assertEqual(
                set(hashes), set(motion_probe.fingerprint_source_paths(lesson))
            )
            self.assertTrue(all(len(value) == 64 for value in hashes.values()))

    def test_contract_is_versioned_per_boundary(self):
        self.assertEqual(
            motion_probe.FINGERPRINT_CONTRACT,
            {
                "067": "motion-recorder-resource-v2-l067",
                "068": "motion-recorder-resource-v2-l068",
                "069": "motion-recorder-resource-v2-l069",
            },
        )


if __name__ == "__main__":
    unittest.main()
