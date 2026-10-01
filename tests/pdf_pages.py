#!/usr/bin/env python3
"""List lesson PDF pages that are less than half full. A hint for someone to
look at, not a build gate.

A page is measured by its ink, drawings and all: poppler's pdftoppm draws it
small, and the lowest dark row above the footer says how far down the page
it is filled. Two pages may end early: the page of answers to Check
yourself, which starts a page of its own, and a lesson's last page of text,
unless it is nearly empty and doesn't start with Check yourself's questions
(pdftotext reads where each page starts). Any other page less than half
full usually holds a box or a heading that the figure, table or box after
it, too tall for what was left, left behind.
"""

import argparse
import re
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

DPI    = 24
TOP    = 12 / 25.4          # inches: the page margins in adk.css's @page
BOTTOM = 14 / 25.4
INK    = 200                # a gray darker than this is ink, not a box's tint


def heights (pdf):
    """How far down each page its ink reaches, from 0 (blank) to 1 (full)."""
    with tempfile.TemporaryDirectory () as folder:
        subprocess.run (["pdftoppm", "-r", str (DPI), "-gray", str (pdf), f"{folder}/page"],
                        check=True)
        pages = []
        for image in sorted (Path (folder).glob ("page-*.pgm"),
                             key=lambda path: int (path.stem.rsplit ("-", 1)[1])):
            width, height, pixels = pgm (image.read_bytes ())
            top, bottom = round (TOP * DPI), height - round (BOTTOM * DPI)
            lowest = next ((row for row in range (bottom - 1, top - 1, -1)
                            if min (pixels[row * width:(row + 1) * width]) < INK), top)
            pages.append ((lowest - top) / (bottom - top))
    return pages


def pgm (data):
    """A binary PGM's width, height and pixels."""
    match = re.match (rb"P5\s+(\d+)\s+(\d+)\s+(\d+)\s", data)
    width, height = int (match[1]), int (match[2])
    return width, height, data[match.end ():match.end () + width * height]


def first_lines (pdf):
    """The first line of text on each page."""
    text = subprocess.run (["pdftotext", "-layout", str (pdf), "-"],
                           check=True, capture_output=True, text=True).stdout
    return [page.strip ().split ("\n", 1)[0].strip () for page in text.split ("\f")]


def sparse (pdf, threshold, last_threshold):
    filled = heights (pdf)
    starts = first_lines (pdf)
    answers = next ((number for number, line in enumerate (starts, 1)
                     if line.startswith ("Answers")), None)
    last = (answers or len (filled) + 1) - 1          # the last page of text
    found = []
    for number, fill in enumerate (filled, 1):
        if number == answers or number > last:
            continue
        # The questions of Check yourself, whole, end a lesson well.
        if number == last and starts[number - 1].startswith ("Check yourself"):
            continue
        if fill < (last_threshold if number == last else threshold):
            found.append ((number, fill, "last" if number == last else ""))
    return pdf.stem, len (filled), found


def main ():
    parser = argparse.ArgumentParser (description=__doc__.split ("\n\n")[0])
    parser.add_argument ("pdf_dir", type=Path, help="directory containing lesson PDFs")
    parser.add_argument ("--threshold", type=float, default=0.5,
                         help="report pages filled less than this (default: 0.5)")
    parser.add_argument ("--last", type=float, default=0.15,
                         help="report a last page of text filled less than this (default: 0.15)")
    args = parser.parse_args ()

    paths = sorted (args.pdf_dir.glob ("[0-9][0-9][0-9]-*.pdf"))
    if not paths:
        parser.error (f"no lesson PDFs in {args.pdf_dir}")
    with ThreadPoolExecutor () as pool:
        results = list (pool.map (lambda path: sparse (path, args.threshold, args.last), paths))

    count = sum (len (found) for _, _, found in results)
    print (f"Pages less than {args.threshold:.0%} full, or a last page of text less than "
           f"{args.last:.0%}: {count} in {len (paths)} lessons")
    for lesson, pages, found in results:
        for number, fill, note in found:
            print (f"  {lesson:<34} page {number:>2} of {pages:<2} {fill:4.0%} full"
                   f"{'  (last of the text)' if note else ''}")
    if count:
        print ("Look at the flagged pages before changing the print styles.")


if __name__ == "__main__":
    sys.exit (main ())
