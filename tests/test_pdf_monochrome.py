import unittest

from scripts.check_pdf_monochrome import embeddedImageFindings
from scripts.check_pdf_monochrome import renderedInkFindings


class EmbeddedImageFindingsTest(unittest.TestCase):
    def test_accepts_gray_and_mono_images(self):
        output = """
page num type width height color comp bpc enc
   1   0 image 1200 800 gray 1 8 image
   2   1 image 1200 800 mono 1 1 ccitt
"""

        imageCount, findings = embeddedImageFindings(output)

        self.assertEqual(imageCount, 2)
        self.assertEqual(findings, [])

    def test_rejects_rgb_and_cmyk_images(self):
        output = """
   1   0 image 1200 800 rgb 3 8 image
   2   1 image 1200 800 cmyk 4 8 image
"""

        imageCount, findings = embeddedImageFindings(output)

        self.assertEqual(imageCount, 2)
        self.assertEqual(
            findings,
            [
                "image 0 on page 1 uses rgb",
                "image 1 on page 2 uses cmyk",
            ],
        )


class RenderedInkFindingsTest(unittest.TestCase):
    def test_accepts_black_only_pages(self):
        output = """
 0.00000  0.00000  0.00000  0.17134 CMYK OK
 0.00000  0.00000  0.00000  0.01467 CMYK OK
"""

        pageCount, findings = renderedInkFindings(output)

        self.assertEqual(pageCount, 2)
        self.assertEqual(findings, [])

    def test_rejects_color_and_blank_pages(self):
        output = """
 0.02404  0.00000  0.00000  0.04693 CMYK OK
 0.00000  0.00000  0.00000  0.00000 CMYK OK
"""

        pageCount, findings = renderedInkFindings(output)

        self.assertEqual(pageCount, 2)
        self.assertEqual(
            findings,
            [
                "page 1 has CMY coverage 0.02404 0.00000 0.00000",
                "page 2 has no black coverage",
            ],
        )

    def test_rejects_missing_coverage_rows(self):
        pageCount, findings = renderedInkFindings("unrecognized output")

        self.assertEqual(pageCount, 0)
        self.assertEqual(
            findings,
            ["Ghostscript produced no ink-coverage rows"],
        )


if __name__ == "__main__":
    unittest.main()
