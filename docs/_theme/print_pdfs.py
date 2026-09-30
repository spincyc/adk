"""Print the built lesson pages with a small pool of Chromium processes."""

import argparse
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path


def print_lesson (page: Path, pdf_dir: Path, chromium: str) -> str:
    lesson = page.parent.name
    output = pdf_dir / f"{lesson}.pdf"
    output.unlink (missing_ok=True)
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
    result = subprocess.run (command, capture_output=True, text=True)
    if result.returncode != 0 or not output.is_file () or output.stat ().st_size == 0:
        raise RuntimeError (
            f"Could not print {lesson} (Chromium exit {result.returncode}).\n"
            f"{result.stderr[-2000:]}"
        )
    return lesson


def main () -> None:
    parser = argparse.ArgumentParser (description=__doc__)
    parser.add_argument ("--site-dir", type=Path, required=True)
    parser.add_argument ("--chromium", required=True)
    parser.add_argument ("--workers", type=int, default=2)
    args = parser.parse_args ()
    if args.workers < 1:
        parser.error ("--workers must be at least 1")

    pages = sorted (args.site_dir.glob ("lessons/*/index.html"))
    if not pages:
        parser.error (f"no lesson pages in {args.site_dir}")
    pdf_dir = args.site_dir / "pdf"
    pdf_dir.mkdir (parents=True, exist_ok=True)

    with ThreadPoolExecutor (max_workers=args.workers) as pool:
        futures = [pool.submit (print_lesson, page, pdf_dir, args.chromium) for page in pages]
        for future in as_completed (futures):
            print (f"  PDF  {future.result ()}", flush=True)


if __name__ == "__main__":
    main ()
