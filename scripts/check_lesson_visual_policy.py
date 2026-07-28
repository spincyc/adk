#!/usr/bin/env python3

"""Require an explicit pencil-or-schematic classification for lesson visuals."""

from __future__ import annotations

import pathlib
import re
import sys


MARKER = re.compile(r"^\s*%\s*ADK visual: (pencil|schematic)\s*$")
VISUAL = re.compile(
    r"\\includegraphics"
    r"|\\begin\{(?:tikzpicture|circuitikz)\}"
    r"|\\input\{[^}]*assets/"
)


def findings(path: pathlib.Path) -> list[str]:
    lines = path.read_text(encoding="utf-8").splitlines()
    result: list[str] = []

    for index, line in enumerate(lines):
        if not VISUAL.search(line):
            continue

        marker = MARKER.match(lines[index - 1]) if index > 0 else None
        if marker is None:
            result.append(
                f"{path}:{index + 1}: visual lacks an immediately preceding "
                "'% ADK visual: pencil' or '% ADK visual: schematic' marker"
            )
            continue

        classification = marker.group(1)
        if classification == "schematic" and "\\begin{tikzpicture}" in line:
            result.append(
                f"{path}:{index + 1}: plain tikzpicture cannot claim the "
                "formal-schematic exception"
            )

    return result


def main(arguments: list[str]) -> int:
    if not arguments:
        print("usage: check_lesson_visual_policy.py TEX...", file=sys.stderr)
        return 2

    failures: list[str] = []
    for argument in arguments:
        failures.extend(findings(pathlib.Path(argument)))

    if failures:
        for failure in failures:
            print(failure, file=sys.stderr)
        return 1

    print(f"ADK lesson visual policy passed for {len(arguments)} source file(s).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
