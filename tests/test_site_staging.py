import runpy
import tempfile
import unittest
from pathlib import Path


SITE_TOOL = runpy.run_path("scripts/site", run_name="site_tool_test")
SiteError = SITE_TOOL["SiteError"]
reject_draft_leaks = SITE_TOOL["reject_draft_leaks"]


class DraftIsolationTest(unittest.TestCase):
    def test_rejects_renamed_exact_draft(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            drafts = root / "drafts"
            staged = root / "staged"
            drafts.mkdir()
            staged.mkdir()
            (drafts / "lesson.md").write_text("noncanonical draft\n")
            (staged / "renamed.md").write_text("noncanonical draft\n")

            with self.assertRaisesRegex(
                SiteError,
                "noncanonical draft entered staged site",
            ):
                reject_draft_leaks(drafts, staged)

    def test_allows_unrelated_staged_material(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            drafts = root / "drafts"
            staged = root / "staged"
            drafts.mkdir()
            staged.mkdir()
            (drafts / "lesson.md").write_text("noncanonical draft\n")
            (staged / "lesson.md").write_text("reviewed canonical lesson\n")

            reject_draft_leaks(drafts, staged)


if __name__ == "__main__":
    unittest.main()
