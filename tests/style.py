#!/usr/bin/env python3
"""Check the mechanical parts of the ADK C++ style (docs/STYLE.md).

Usage: style.py FILE...

Reports tabs, trailing whitespace, missing final newlines, lines over 100
characters, and calls or declarations without a space before their opening
parenthesis. Alignment is a judgement for people and is not checked.
"""

import re
import sys

LIMIT = 100
TIGHT_CALL = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\(")
STRING = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')


def code_only(line, in_comment):
    """Return the line with strings and comments blanked, and the comment state."""
    text = STRING.sub('""', line)
    result = ""
    index = 0

    while index < len(text):
        if in_comment:
            end = text.find("*/", index)
            if end < 0:
                return result, True
            index = end + 2
            in_comment = False
        elif text.startswith("//", index):
            break
        elif text.startswith("/*", index):
            in_comment = True
            index += 2
        else:
            result += text[index]
            index += 1

    return result, in_comment


def check(path):
    problems = []
    source = open(path, encoding="utf-8").read()

    if source and not source.endswith("\n"):
        problems.append((len(source.splitlines()), "no newline at end of file"))

    in_comment = False

    for number, line in enumerate(source.splitlines(), 1):
        if "\t" in line:
            problems.append((number, "tab"))
        if line != line.rstrip():
            problems.append((number, "trailing whitespace"))
        if len(line) > LIMIT:
            problems.append((number, f"longer than {LIMIT} characters"))

        code, in_comment = code_only(line, in_comment)

        if code.lstrip().startswith("#"):
            continue

        for match in TIGHT_CALL.finditer(code):
            name = match.group(1)
            if not name.isupper() and not name.startswith("_"):
                problems.append((number, f"no space before the parenthesis after '{name}'"))

    return [f"{path}:{number}: {problem}" for number, problem in problems]


def main(paths):
    problems = [problem for path in paths for problem in check(path)]
    print("\n".join(problems)) if problems else None
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
