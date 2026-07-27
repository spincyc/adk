#!/usr/bin/env python3

"""Check the function-parenthesis alignment clang-format cannot represent."""

from __future__ import annotations

import re
import sys
from pathlib import Path


CONTROL_WORDS = {
    "catch",
    "for",
    "if",
    "sizeof",
    "switch",
    "while",
}

FUNCTION_PARENTHESIS = re.compile(
    r"(?P<name>(?:[A-Za-z_]\w*::)*[~A-Za-z_]\w*)(?P<space>[ \t]*)\("
)


def scrub(line: str, in_block_comment: bool) -> tuple[str, bool]:
    """Blank comments and literals without changing source columns."""

    result = list(line)
    index = 0
    quote = ""
    while index < len(line):
        if in_block_comment:
            result[index] = " "
            if line.startswith("*/", index):
                result[index : index + 2] = "  "
                in_block_comment = False
                index += 2
            else:
                index += 1
            continue

        if quote:
            result[index] = " "
            if line[index] == "\\":
                if index + 1 < len(line):
                    result[index + 1] = " "
                    index += 2
                    continue
            elif line[index] == quote:
                quote = ""
            index += 1
            continue

        if line.startswith("//", index):
            result[index:] = " " * (len(line) - index)
            break
        if line.startswith("/*", index):
            result[index : index + 2] = "  "
            in_block_comment = True
            index += 2
            continue
        if line[index] in {'"', "'"}:
            quote = line[index]
            result[index] = " "
        index += 1

    return "".join(result), in_block_comment


def candidates(line: str) -> list[re.Match[str]]:
    if line.lstrip().startswith("#"):
        return []

    return [
        match
        for match in FUNCTION_PARENTHESIS.finditer(line)
        if match.group("name").split("::")[-1] not in CONTROL_WORDS
    ]


def check_file(path: Path) -> list[str]:
    errors: list[str] = []
    group: list[tuple[int, int, str]] = []
    group_indent: int | None = None
    in_block_comment = False

    def finish_group() -> None:
        nonlocal group_indent
        if len(group) < 2:
            group.clear()
            group_indent = None
            return

        columns = {column for _, column, _ in group}
        if len(columns) > 1:
            locations = ", ".join(f"{path}:{line}" for line, _, _ in group)
            errors.append(f"unaligned function parentheses: {locations}")
        group.clear()
        group_indent = None

    for number, line in enumerate(path.read_text().splitlines(), start=1):
        clean_line, in_block_comment = scrub(line, in_block_comment)
        found = candidates(clean_line)
        if not found:
            finish_group()
            continue

        for match in found:
            if not match.group("space"):
                errors.append(
                    f"{path}:{number}: function parenthesis requires a space: "
                    f"{match.group('name')}("
                )

        first = found[0]
        indentation = len(clean_line) - len(clean_line.lstrip())
        if group and indentation != group_indent:
            finish_group()
        group_indent = indentation
        group.append((number, first.end() - 1, line.rstrip()))

    finish_group()
    return errors


def main(arguments: list[str]) -> int:
    errors: list[str] = []
    for argument in arguments:
        errors.extend(check_file(Path(argument)))

    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1

    print("ADK alignment checks passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
