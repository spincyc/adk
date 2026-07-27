import unittest

from scripts.check_pdf_policy import fontFindings
from scripts.check_pdf_policy import metadataFindings
from scripts.check_pdf_policy import texLogFindings
from scripts.check_pdf_policy import textFindings


class MetadataFindingsTest(unittest.TestCase):
    def test_complete_metadata_passes(self):
        output = """\
Title:           A lesson
Subject:         A circuit
Author:          An author
CreationDate:    Mon Jul 27 09:00:00 2026 CDT
Pages:           4
Encrypted:       no
"""

        self.assertEqual(metadataFindings(output), [])

    def test_metadata_failures_are_complete(self):
        output = """\
Title:
Subject:         A circuit
Pages:           unknown
Encrypted:       yes
"""

        self.assertEqual(
            metadataFindings(output),
            [
                "missing or blank Title metadata",
                "missing or blank Author metadata",
                "missing or blank CreationDate metadata",
                "PDF must not be encrypted",
                "PDF has invalid page metadata",
            ],
        )


class FontFindingsTest(unittest.TestCase):
    def test_embedded_fonts_pass(self):
        output = """\
name                                 type              encoding         emb sub uni object ID
------------------------------------ ----------------- ---------------- --- --- --- ---------
ABCDEF+CMR10                         Type 1            Builtin          yes yes yes     11  0
"""

        self.assertEqual(fontFindings(output), [])

    def test_unembedded_font_fails(self):
        output = """\
name                                 type              encoding         emb sub uni object ID
------------------------------------ ----------------- ---------------- --- --- --- ---------
Helvetica                            Type 1            WinAnsi          no  no  no      8  0
"""

        self.assertEqual(
            fontFindings(output),
            ["font is not embedded: Helvetica"],
        )

    def test_missing_font_records_fail(self):
        output = """\
name                                 type              encoding         emb sub uni object ID
------------------------------------ ----------------- ---------------- --- --- --- ---------
"""

        self.assertEqual(
            fontFindings(output),
            ["PDF contains no inspectable fonts"],
        )


class TextFindingsTest(unittest.TestCase):
    def test_extracted_text_must_not_be_blank(self):
        self.assertEqual(textFindings("\f \n"), ["PDF has no extractable text"])
        self.assertEqual(textFindings("Lesson title\n"), [])


class TexLogFindingsTest(unittest.TestCase):
    def test_final_tex_log_rejects_publication_warnings(self):
        output = """\
Overfull \\hbox (3.0pt too wide) in paragraph at lines 10--11
LaTeX Warning: Reference `missing' on page 2 undefined on input line 20.
LaTeX Warning: Label(s) may have changed. Rerun to get cross-references right.
"""

        findings = texLogFindings(output)

        self.assertEqual(len(findings), 3)
        self.assertTrue(findings[0].startswith("overfull TeX box at log line 1:"))
        self.assertTrue(
            findings[1].startswith(
                "undefined TeX reference or citation at log line 2:"
            )
        )
        self.assertTrue(
            findings[2].startswith("unresolved TeX rerun request at log line 3:")
        )

    def test_final_tex_log_allows_package_descriptions(self):
        output = """\
Package: rerunfilecheck 2025-06-21 v1.11 Rerun checks for auxiliary files (HO)
Underfull \\hbox (badness 10000) in paragraph at lines 1--2
"""

        self.assertEqual(texLogFindings(output), [])


if __name__ == "__main__":
    unittest.main()
