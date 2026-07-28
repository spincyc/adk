import tempfile
import unittest
from pathlib import Path

from scripts.check_lesson_visual_policy import findings


class LessonVisualPolicyTest(unittest.TestCase):
    def inspect(self, source: str) -> list[str]:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "main.tex"
            path.write_text(source, encoding="utf-8")
            return findings(path)

    def test_accepts_classified_pencil_and_formal_schematic(self):
        self.assertEqual(
            self.inspect(
                "% ADK visual: pencil\n"
                "\\includegraphics{example-pencil.png}\n"
                "% ADK visual: schematic\n"
                "\\begin{circuitikz}\n"
            ),
            [],
        )

    def test_rejects_unclassified_visual(self):
        result = self.inspect("\\includegraphics{example.png}\n")

        self.assertEqual(len(result), 1)
        self.assertIn("lacks an immediately preceding", result[0])

    def test_rejects_plain_tikz_as_formal_schematic(self):
        result = self.inspect(
            "% ADK visual: schematic\n"
            "\\begin{tikzpicture}\n"
        )

        self.assertEqual(len(result), 1)
        self.assertIn("plain tikzpicture", result[0])


if __name__ == "__main__":
    unittest.main()
