#!/usr/bin/env python3

import argparse
import pathlib
import re
import shutil
import subprocess
import sys


REQUIRED_METADATA = ("Title", "Subject", "Author", "CreationDate")


def parsePdfInfo(output):
    metadata = {}

    for line in output.splitlines():
        if ":" not in line:
            continue

        name, value = line.split(":", 1)
        metadata[name.strip()] = value.strip()

    return metadata


def metadataFindings(output):
    metadata = parsePdfInfo(output)
    findings = []

    for name in REQUIRED_METADATA:
        if not metadata.get(name):
            findings.append(f"missing or blank {name} metadata")

    if metadata.get("Encrypted") != "no":
        findings.append("PDF must not be encrypted")

    try:
        if int(metadata.get("Pages", "0")) < 1:
            findings.append("PDF has no pages")
    except ValueError:
        findings.append("PDF has invalid page metadata")

    return findings


def fontFindings(output):
    findings = []
    rows = output.splitlines()[2:]

    for row in rows:
        columns = row.split()
        if not columns:
            continue
        if len(columns) < 7:
            findings.append(f"cannot parse font record: {row.strip()}")
            continue

        fontName = columns[0]
        embedded = columns[-5]

        if embedded != "yes":
            findings.append(f"font is not embedded: {fontName}")

    if not any(row.split() for row in rows):
        findings.append("PDF contains no inspectable fonts")

    return findings


def textFindings(output):
    if not output.strip():
        return ["PDF has no extractable text"]

    return []


def texLogFindings(output):
    patterns = (
        (re.compile(r"Overfull \\[hv]box"), "overfull TeX box"),
        (
            re.compile(
                r"(?:Citation|Reference).*\bundefined\b"
                r"|There were undefined (?:references|citations)",
                re.IGNORECASE,
            ),
            "undefined TeX reference or citation",
        ),
        (
            re.compile(r"Label\(s\) may have changed\. Rerun"),
            "unresolved TeX rerun request",
        ),
    )
    findings = []

    for lineNumber, line in enumerate(output.splitlines(), 1):
        for pattern, description in patterns:
            if pattern.search(line):
                findings.append(f"{description} at log line {lineNumber}: {line.strip()}")
                break

    return findings


def runTool(arguments):
    completed = subprocess.run(
        arguments,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    if completed.returncode != 0:
        detail = completed.stderr.strip() or completed.stdout.strip()
        raise RuntimeError(f"{' '.join(arguments)} failed: {detail}")

    return completed.stdout


def inspectPdf(pdf, buildDirectory):
    findings = []

    try:
        findings.extend(metadataFindings(runTool(["pdfinfo", str(pdf)])))
        findings.extend(fontFindings(runTool(["pdffonts", str(pdf)])))
        findings.extend(textFindings(runTool(["pdftotext", "-layout", str(pdf), "-"])))
    except RuntimeError as error:
        findings.append(str(error))

    log = buildDirectory / pdf.stem / "main.log"
    if not log.is_file():
        findings.append(f"missing final TeX log: {log}")
    else:
        findings.extend(texLogFindings(log.read_text(encoding="utf-8", errors="replace")))

    return findings


def main():
    parser = argparse.ArgumentParser(
        description="Check ADK lesson metadata, fonts, text, and final TeX logs."
    )
    parser.add_argument(
        "--build-directory",
        default="build/lessons",
        type=pathlib.Path,
        help="directory containing one final main.log per lesson",
    )
    parser.add_argument("pdfs", nargs="+", type=pathlib.Path)
    arguments = parser.parse_args()

    missingTools = [
        tool
        for tool in ("pdfinfo", "pdffonts", "pdftotext")
        if shutil.which(tool) is None
    ]
    if missingTools:
        print(
            "missing required PDF tools: " + ", ".join(missingTools),
            file=sys.stderr,
        )
        return 2

    failed = False

    for pdf in arguments.pdfs:
        findings = inspectPdf(pdf, arguments.build_directory)
        for finding in findings:
            print(f"{pdf}: {finding}", file=sys.stderr)
            failed = True

    if failed:
        return 1

    print(f"ADK PDF policy checks passed for {len(arguments.pdfs)} file(s).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
