import unittest

from scripts.publication import PublicationConfigError
from scripts.publication import PublishedLesson
from scripts.publication import resolve_latest_publication


class PublicationResolverTest(unittest.TestCase):
    def test_resolves_unordered_literal_assignments(self):
        source = """
LESSONS := 002 001 037 036
EXAMPLES := \\
    Lesson001First \\
    Lesson037Newest \\
    Lesson002Second
"""
        self.assertEqual(
            resolve_latest_publication(source),
            PublishedLesson("037", "Lesson037Newest"),
        )

    def test_derived_paths_are_stable(self):
        publication = PublishedLesson("037", "Lesson037Percussion")

        self.assertEqual(publication.lesson_page, "lessons/037/")
        self.assertEqual(publication.pdf_path, "downloads/lessons/037.pdf")
        self.assertEqual(
            publication.sketch_path,
            "downloads/sketches/Lesson037Percussion.ino",
        )

    def test_comments_do_not_create_examples(self):
        source = """
# Lesson037NotConfigured
LESSONS := 036 037 # promoted
EXAMPLES := Lesson036Prior # Lesson037StillNotConfigured
"""
        with self.assertRaisesRegex(
            PublicationConfigError,
            "lesson 037 must have exactly one",
        ):
            resolve_latest_publication(source)

    def test_rejects_missing_and_duplicate_assignments(self):
        fixtures = (
            "EXAMPLES := Lesson001First",
            "LESSONS := 001\nLESSONS := 002\nEXAMPLES := Lesson002Second",
        )
        for source in fixtures:
            with self.subTest(source=source):
                with self.assertRaisesRegex(PublicationConfigError, "exactly one"):
                    resolve_latest_publication(source)

    def test_rejects_empty_malformed_and_duplicate_lessons(self):
        fixtures = (
            ("LESSONS :=\nEXAMPLES :=", "contains no"),
            (
                "LESSONS := 001 two\nEXAMPLES := Lesson001First",
                "malformed entry",
            ),
            (
                "LESSONS := 001 001\nEXAMPLES := Lesson001First",
                "duplicate",
            ),
        )
        for source, message in fixtures:
            with self.subTest(source=source):
                with self.assertRaisesRegex(PublicationConfigError, message):
                    resolve_latest_publication(source)

    def test_rejects_missing_ambiguous_and_malformed_examples(self):
        fixtures = (
            (
                "LESSONS := 001 002\nEXAMPLES := Lesson001First",
                "exactly one",
            ),
            (
                "LESSONS := 001\nEXAMPLES := Lesson001First Lesson001Other",
                "exactly one",
            ),
            (
                "LESSONS := 001\nEXAMPLES := NotALesson",
                "malformed entry",
            ),
        )
        for source, message in fixtures:
            with self.subTest(source=source):
                with self.assertRaisesRegex(PublicationConfigError, message):
                    resolve_latest_publication(source)

    def test_rejects_nonliteral_make_syntax(self):
        fixtures = (
            "LESSONS = 001\nEXAMPLES := Lesson001First",
            "LESSONS := $(PUBLISHED)\nEXAMPLES := Lesson001First",
            "LESSONS := 001\nEXAMPLES += Lesson001First",
            "LESSONS := 001 \\\nEXAMPLES := Lesson001First",
        )
        for source in fixtures:
            with self.subTest(source=source):
                with self.assertRaises(PublicationConfigError):
                    resolve_latest_publication(source)

    def test_promotion_changes_every_derived_artifact(self):
        before = resolve_latest_publication(
            "LESSONS := 035 036\n"
            "EXAMPLES := Lesson035Prior Lesson036Current\n"
        )
        after = resolve_latest_publication(
            "LESSONS := 035 036 037\n"
            "EXAMPLES := Lesson035Prior Lesson036Current Lesson037Next\n"
        )

        self.assertEqual(before.number, "036")
        self.assertEqual(after.number, "037")
        self.assertNotEqual(before.lesson_page, after.lesson_page)
        self.assertNotEqual(before.pdf_path, after.pdf_path)
        self.assertNotEqual(before.sketch_path, after.sketch_path)


if __name__ == "__main__":
    unittest.main()
