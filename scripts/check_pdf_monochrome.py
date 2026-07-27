#!/usr/bin/env python3

import argparse
import pathlib
import re
import shutil
import subprocess
import sys


IMAGE_ROW = re.compile(r"^\s*\d+\s+\d+\s+")
INK_ROW = re.compile(
    r"^\s*"
    r"([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[Ee][+-]?\d+)?)\s+"
    r"([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[Ee][+-]?\d+)?)\s+"
    r"([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[Ee][+-]?\d+)?)\s+"
    r"([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[Ee][+-]?\d+)?)\s+CMYK\b"
)


def embeddedImageFindings(output):
    findings = []
    imageCount = 0

    for line in output.splitlines():
        if not IMAGE_ROW.match(line):
            continue

        columns = line.split()
        if len(columns) < 6:
            findings.append(f"could not parse pdfimages row: {line.strip()}")
            continue

        imageCount += 1
        colorSpace = columns[5].lower()
        if colorSpace not in {"gray", "mono"}:
            findings.append(
                f"image {columns[1]} on page {columns[0]} uses {colorSpace}"
            )

    return imageCount, findings


def renderedInkFindings(output):
    findings = []
    pageCount = 0

    for line in output.splitlines():
        match = INK_ROW.match(line)
        if match is None:
            continue

        pageCount += 1
        cyan, magenta, yellow, black = (float(value) for value in match.groups())
        if cyan != 0.0 or magenta != 0.0 or yellow != 0.0:
            findings.append(
                f"page {pageCount} has CMY coverage "
                f"{cyan:.5f} {magenta:.5f} {yellow:.5f}"
            )
        if black <= 0.0:
            findings.append(f"page {pageCount} has no black coverage")

    if pageCount == 0:
        findings.append("Ghostscript produced no ink-coverage rows")

    return pageCount, findings


def runTool(command):
    completed = subprocess.run(
        command,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if completed.returncode != 0:
        detail = completed.stderr.strip() or completed.stdout.strip()
        raise RuntimeError(
            f"{command[0]} exited with status {completed.returncode}: {detail}"
        )
    return completed.stdout


def checkPdf(path):
    if not path.is_file():
        return [f"{path}: file not found"]

    findings = []
    try:
        imageOutput = runTool(["pdfimages", "-list", str(path)])
        _, imageFindings = embeddedImageFindings(imageOutput)
        findings.extend(f"{path}: {finding}" for finding in imageFindings)

        inkOutput = runTool(
            ["gs", "-q", "-o", "-", "-sDEVICE=inkcov", str(path)]
        )
        _, inkFindings = renderedInkFindings(inkOutput)
        findings.extend(f"{path}: {finding}" for finding in inkFindings)
    except RuntimeError as error:
        findings.append(f"{path}: {error}")

    return findings


def main():
    parser = argparse.ArgumentParser(
        description="Reject lesson PDFs containing color content."
    )
    parser.add_argument("pdfs", nargs="+", type=pathlib.Path)
    arguments = parser.parse_args()

    missingTools = [
        tool for tool in ("pdfimages", "gs") if shutil.which(tool) is None
    ]
    if missingTools:
        print(
            "missing required PDF tools: " + ", ".join(missingTools),
            file=sys.stderr,
        )
        return 2

    findings = []
    for pdf in arguments.pdfs:
        findings.extend(checkPdf(pdf))

    if findings:
        for finding in findings:
            print(finding, file=sys.stderr)
        return 1

    print(f"ADK monochrome PDF checks passed for {len(arguments.pdfs)} file(s).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
