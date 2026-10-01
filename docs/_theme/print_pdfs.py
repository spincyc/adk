"""Print the built lesson pages with a small pool of Chromium processes.

Chromium can hang, crash or stop early without saying so, so each lesson
gets a time limit and one more try, and its PDF must look printed: whole,
from %PDF- to %%EOF; at least two pages; titled as the lesson's page is,
so Chromium printed the lesson and not an error page; and set in the
site's own typeface, so the page had loaded its fonts. Skia, Chromium's
PDF writer, leaves the page objects and font names readable, so the check
needs no PDF library.

Then every PDF must fit --max-pdf-mb, and the whole site --max-site-mb:
GitHub Pages publishes a site of at most 1 GB.
"""

import argparse
import html
import os
import re
import signal
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

TIMEOUT = 180  # seconds for one lesson; most take a few
TRIES   = 2
MB      = 1_000_000


def chromium_version (chromium: str) -> str:
    result = subprocess.run ([chromium, "--version"], capture_output=True, text=True, timeout=60)
    return (result.stdout or result.stderr).strip () or f"{chromium} (version unknown)"


def run (command: list[str]) -> tuple[str | None, str]:
    """Run Chromium in a process group of its own, so a timeout ends all of it.
    Returns what went wrong, if anything, and what Chromium said."""
    process = subprocess.Popen (command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                text=True, start_new_session=True)
    try:
        _, stderr = process.communicate (timeout=TIMEOUT)
    except subprocess.TimeoutExpired:
        os.killpg (process.pid, signal.SIGKILL)
        _, stderr = process.communicate ()
        return f"Chromium took more than {TIMEOUT} s", stderr
    return (f"Chromium exit {process.returncode}" if process.returncode else None), stderr


def pdf_string (raw: bytes) -> str:
    """Decode a PDF string: (literal, with escapes) or <hex>, UTF-16 when it has a BOM."""
    if raw.startswith (b"<"):
        data = bytes.fromhex (raw[1:-1].decode ("ascii"))
    else:
        escapes = {b"n": b"\n", b"r": b"\r", b"t": b"\t", b"b": b"\b", b"f": b"\f"}
        data = re.sub (rb"\\(?:([0-7]{1,3})|(.))",
                       lambda m: bytes ([int (m[1], 8) & 0xFF]) if m[1]
                       else escapes.get (m[2], m[2]),
                       raw[1:-1], flags=re.S)
    if data.startswith (b"\xfe\xff"):
        return data[2:].decode ("utf-16-be")
    return data.decode ("latin-1")


def page_title (page: Path) -> str:
    match = re.search (r"<title>(.*?)</title>", page.read_text (encoding="utf-8"), re.S)
    return " ".join (html.unescape (match[1]).split ()) if match else ""


def text_font (site_dir: Path) -> str:
    """The site's text typeface, as a PDF names it: "AtkinsonHyperlegibleNext"."""
    css = (site_dir / "assets" / "adk.css").read_text (encoding="utf-8")
    match = re.search (r'--md-text-font:\s*"([^"]+)"', css)
    return match[1].replace (" ", "") if match else ""


def problem (output: Path, title: str, font: str) -> str | None:
    """What is wrong with a printed PDF, or None."""
    if not output.is_file () or output.stat ().st_size == 0:
        return "no PDF"
    data = output.read_bytes ()
    if not data.startswith (b"%PDF-") or not data.rstrip ().endswith (b"%%EOF"):
        return "the PDF is cut short"
    pages = len (re.findall (rb"/Type\s*/Page(?![A-Za-z])", data))
    if pages < 2:
        return f"{pages} page{'s' if pages != 1 else ''}, where every lesson has at least two"
    printed = re.search (rb"/Title\s*(\((?:\\.|[^\\)])*\)|<[0-9A-Fa-f\s]*>)", data, re.S)
    printed = " ".join (pdf_string (printed[1]).split ()) if printed else ""
    if printed != title:
        return f'titled "{printed}", not "{title}": Chromium printed another page'
    if font and not re.search (rb"/(?:BaseFont|FontName)\s*/[A-Z]{6}\+" + re.escape (font.encode ()),
                               data):
        return f"no {font} font: the page printed before its fonts loaded"
    return None


def print_lesson (page: Path, pdf_dir: Path, chromium: str, font: str) -> str:
    lesson = page.parent.name
    output = pdf_dir / f"{lesson}.pdf"
    title  = page_title (page)
    command = [
        chromium,
        "--headless=new",
        "--no-sandbox",
        "--disable-gpu",
        "--no-pdf-header-footer",
        "--virtual-time-budget=10000",
        "--run-all-compositor-stages-before-draw",
        f"--print-to-pdf={output.resolve ()}",
        page.resolve ().as_uri (),
    ]
    for attempt in range (1, TRIES + 1):
        output.unlink (missing_ok=True)
        wrong, stderr = run (command)
        wrong = wrong or problem (output, title, font)
        if wrong is None:
            return lesson
        if attempt < TRIES:
            print (f"  PDF  {lesson}: {wrong}; trying again", flush=True)
    output.unlink (missing_ok=True)
    raise RuntimeError (f"Could not print {lesson}: {wrong}.\n{stderr[-2000:]}")


def site_size (site_dir: Path) -> int:
    return sum (path.stat ().st_size for path in site_dir.rglob ("*") if path.is_file ())


def main () -> None:
    parser = argparse.ArgumentParser (description=__doc__.split ("\n\n")[0])
    parser.add_argument ("--site-dir", type=Path, required=True)
    parser.add_argument ("--chromium", required=True)
    parser.add_argument ("--workers", type=int, default=2)
    parser.add_argument ("--max-pdf-mb", type=float, default=8)
    parser.add_argument ("--max-site-mb", type=float, default=500)
    args = parser.parse_args ()
    if args.workers < 1:
        parser.error ("--workers must be at least 1")

    pages = sorted (args.site_dir.glob ("lessons/*/index.html"))
    if not pages:
        parser.error (f"no lesson pages in {args.site_dir}")
    pdf_dir = args.site_dir / "pdf"
    pdf_dir.mkdir (parents=True, exist_ok=True)
    font = text_font (args.site_dir)
    try:
        version = chromium_version (args.chromium)
    except (OSError, subprocess.TimeoutExpired) as error:
        parser.error (f"cannot run {args.chromium}: {error}")

    print (f"  PDF  with {version}", flush=True)
    with ThreadPoolExecutor (max_workers=args.workers) as pool:
        futures = [pool.submit (print_lesson, page, pdf_dir, args.chromium, font)
                   for page in pages]
        for future in as_completed (futures):
            print (f"  PDF  {future.result ()}", flush=True)

    sizes = {pdf.stem: pdf.stat ().st_size for pdf in pdf_dir.glob ("*.pdf")}
    total = site_size (args.site_dir)
    print (f"  PDF  {len (sizes)} lessons, {sum (sizes.values ()) / MB:.0f} MB; "
           f"largest {max (sizes, key=sizes.get)}, {max (sizes.values ()) / MB:.1f} MB; "
           f"site {total / MB:.0f} MB", flush=True)
    over = [f"{lesson} is {size / MB:.1f} MB" for lesson, size in sorted (sizes.items ())
            if size > args.max_pdf_mb * MB]
    if total > args.max_site_mb * MB:
        over.append (f"the site is {total / MB:.0f} MB")
    if over:
        sys.exit (f"Over the size budget ({args.max_pdf_mb:g} MB a lesson, "
                  f"{args.max_site_mb:g} MB the site): " + "; ".join (over) + ".\n"
                  "A drawing that prints as a picture rather than as lines is the usual cause.")


if __name__ == "__main__":
    main ()
