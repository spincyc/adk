#!/usr/bin/env python3
"""Report short final PDF pages. This is a visual-review hint, not a build gate."""

import argparse
from pathlib import Path
import re
import subprocess


FOOTER = re.compile (r"^ADK · (?:Lesson \d+|E\d+) · .+\d+\s*/\s*\d+$")


def last_page_chars (path):
    text = subprocess.run (["pdftotext", "-layout", str (path), "-"],
                           check=True, capture_output=True, text=True).stdout
    pages = [page for page in text.split ("\f") if page.strip ()]
    last = "\n".join (line for line in pages[-1].splitlines ()
                      if not FOOTER.match (line.strip ()))
    return len (pages), len (" ".join (last.split ()))


def main ():
    parser = argparse.ArgumentParser (description=__doc__)
    parser.add_argument ("pdf_dir", type=Path, help="directory containing lesson PDFs")
    parser.add_argument ("--threshold", type=int, default=500,
                         help="report final pages with fewer characters (default: 500)")
    args = parser.parse_args ()

    paths = sorted (args.pdf_dir.glob ("[0-9][0-9][0-9]-*.pdf"))
    if not paths:
        parser.error (f"no lesson PDFs in {args.pdf_dir}")

    sparse = []
    for path in paths:
        pages, chars = last_page_chars (path)
        if chars < args.threshold:
            sparse.append ((path.stem, pages, chars))

    print (f"Final pages under {args.threshold} text characters: "
           f"{len (sparse)}/{len (paths)}")
    for lesson, pages, chars in sparse:
        print (f"  {lesson:<34} {pages:>2} pages  {chars:>4} chars")
    print ("Text extraction misses diagrams; inspect flagged pages before changing pagination.")


if __name__ == "__main__":
    main ()
