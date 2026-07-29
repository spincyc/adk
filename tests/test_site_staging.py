import runpy
import tempfile
import unittest
from pathlib import Path


SITE_TOOL = runpy.run_path("scripts/site", run_name="site_tool_test")
SiteError = SITE_TOOL["SiteError"]
reject_draft_leaks = SITE_TOOL["reject_draft_leaks"]
stage_linked_design_documents = SITE_TOOL["stage_linked_design_documents"]


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


class LinkedDesignDocumentsTest(unittest.TestCase):
    def setUp(self):
        self.temporary_directory = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary_directory.name)
        self.source_design = self.root / "source/design"
        self.staged = self.root / "staged"
        self.staged_docs = self.staged / "docs"
        self.source_design.mkdir(parents=True)
        self.staged_docs.mkdir(parents=True)

    def tearDown(self):
        self.temporary_directory.cleanup()

    def stage_from(self, entry: Path) -> None:
        stage_linked_design_documents(
            [entry],
            self.source_design,
            self.staged_docs,
        )

    def test_stages_recursive_reference_and_parenthesized_links(self):
        entry = self.staged / "index.md"
        plan = self.source_design / "PLAN (055).md"
        stress = self.source_design / "STRESS.md"
        entry.write_text(
            "[plan][escape]\n\n"
            "[escape]: docs/design/PLAN%20(055).md#scope\n"
        )
        plan.write_text("[stress](STRESS.md)\n")
        stress.write_text("# Stress\n")

        self.stage_from(entry)

        self.assertTrue((self.staged_docs / "design/PLAN (055).md").is_file())
        self.assertTrue((self.staged_docs / "design/STRESS.md").is_file())

    def test_ignores_links_in_code_fences_spans_and_comments(self):
        entry = self.staged / "index.md"
        for name in ("FENCE.md", "SPAN.md", "COMMENT.md"):
            (self.source_design / name).write_text("must not publish\n")
        (self.source_design / "TILDE.md").write_text("must not publish\n")
        entry.write_text(
            "```\n[fence](docs/design/FENCE.md)\n```\n"
            "~~~text\n[tilde](docs/design/TILDE.md)\n~~~\n"
            "`[span](docs/design/SPAN.md)`\n"
            "<!-- [comment](docs/design/COMMENT.md) -->\n"
        )

        self.stage_from(entry)

        self.assertFalse((self.staged_docs / "design").exists())

    def test_preserves_percent_encoded_reserved_filename_characters(self):
        entry = self.staged / "index.md"
        (self.source_design / "PLAN#A.md").write_text("# Plan\n")
        entry.write_text("[plan](docs/design/PLAN%23A.md#scope)\n")

        self.stage_from(entry)

        self.assertTrue((self.staged_docs / "design/PLAN#A.md").is_file())

    def test_handles_cycles_once(self):
        entry = self.staged / "index.md"
        (self.source_design / "A.md").write_text("[b](B.md)\n")
        (self.source_design / "B.md").write_text("[a](A.md)\n")
        entry.write_text("[a](docs/design/A.md)\n")

        self.stage_from(entry)

        self.assertTrue((self.staged_docs / "design/A.md").is_file())
        self.assertTrue((self.staged_docs / "design/B.md").is_file())

    def test_enforces_closure_cap(self):
        entry = self.staged / "index.md"
        targets = []
        for number in range(257):
            name = f"D{number:03}.md"
            targets.append(f"[{number}](docs/design/{name})")
            (self.source_design / name).write_text(f"# {number}\n")
        entry.write_text("\n".join(targets))

        with self.assertRaisesRegex(SiteError, "closure exceeds 256 files"):
            self.stage_from(entry)

    def test_excludes_drafts_traversal_and_symlink_escape(self):
        entry = self.staged / "index.md"
        drafts = self.source_design / "drafts"
        drafts.mkdir()
        (drafts / "SECRET.md").write_text("draft\n")
        outside = self.root / "outside.md"
        outside.write_text("unsafe\n")
        (self.source_design / "ESCAPE.md").symlink_to(outside)
        entry.write_text(
            "[draft](docs/design/drafts/SECRET.md)\n"
            "[traversal](../outside.md)\n"
            "[symlink](docs/design/ESCAPE.md)\n"
        )

        self.stage_from(entry)

        self.assertFalse((self.staged_docs / "design").exists())

    def test_stages_links_from_copied_tree_pages(self):
        with tempfile.TemporaryDirectory() as directory:
            copied_tree = self.staged_docs / "research"
            copied_tree.mkdir()
            entry = copied_tree / "REPORT.md"
            (self.source_design / "PLAN.md").write_text("# Plan\n")
            entry.write_text("[plan](../design/PLAN.md)\n")

            stage_linked_design_documents(
                list(self.staged.rglob("*.md")),
                self.source_design,
                self.staged_docs,
            )

            self.assertTrue((self.staged_docs / "design/PLAN.md").is_file())


if __name__ == "__main__":
    unittest.main()
