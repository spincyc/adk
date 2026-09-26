#!/usr/bin/env python3
"""Check the mechanical parts of the ADK C++ style (docs/STYLE.md).

    python3 tests/style.py FILE...

Reports tabs, trailing whitespace, missing final newlines, lines over 100
characters (80 in a sketch, which the lesson page and its PDF show whole),
calls or declarations without a space before their opening parenthesis,
and braces out of place:

- a brace that opens or closes a block over several lines (a function, a
  type, a lambda or a statement's body) must be alone on its line, but for
  namespace braces and a ; or , after a closing brace;
- every if, else, for, while, do and switch body is in braces, starting on
  the line below; a do loop's while (...); follows its closing brace.

Braces opened and closed on one line, such as an initializer, an enum's
names or a one-line accessor, are left alone. A file-scope constant in
src/adk/*.cpp must be constexpr, not const. Alignment and naming are for
people to judge.
"""

import os
import re
import sys

LIMIT        = 100
SKETCH_LIMIT = 80
TIGHT_CALL   = re.compile (r"\b([A-Za-z_][A-Za-z0-9_]*)\(")
STRING       = re.compile (r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')

# The statements whose bodies must be in braces, and the text before an
# opening brace that makes it a block's (a function, lambda or statement
# after its parentheses, or a type) rather than a value's.
CONTROL     = re.compile (r"(?:else\s+)?(if|for|while|switch)\s*\(")
BLOCK_AFTER = re.compile (r"(\)(\s*(const|noexcept|override|final|mutable))*|\belse|\bdo)$")
TYPE_BODY   = re.compile (r"^(struct|class|union|enum)\b[^=]*$")

# STYLE.md: constants are constexpr.
CONSTEXPR_IN_LIBRARY = True
LIBRARY_SOURCE       = re.compile (r"(^|/)src/adk/[^/]+\.cpp$")
CONSTANT             = re.compile (r"^(static\s+)?const\s[^(=;{]*[=;{]")


def code_only (line, in_comment):
    """Return the line with strings and comments blanked, and the comment state."""
    text = STRING.sub ('""', line)
    result = ""
    index = 0

    while index < len (text):
        if in_comment:
            end = text.find ("*/", index)
            if end < 0:
                return result, True
            index = end + 2
            in_comment = False
        elif text.startswith ("//", index):
            break
        elif text.startswith ("/*", index):
            in_comment = True
            index += 2
        else:
            result += text[index]
            index += 1

    return result, in_comment


def check (path):
    problems = []
    source = open (path, encoding="utf-8").read ()

    if source and not source.endswith ("\n"):
        problems.append ((len (source.splitlines ()), "no newline at end of file"))

    limit        = SKETCH_LIMIT if path.endswith (".ino") else LIMIT
    library      = LIBRARY_SOURCE.search (path.replace (os.sep, "/")) is not None
    braces       = Braces (constants=CONSTEXPR_IN_LIBRARY and library)
    in_comment   = False
    in_directive = False

    for number, line in enumerate (source.splitlines (), 1):
        if "\t" in line:
            problems.append ((number, "tab"))
        if line != line.rstrip ():
            problems.append ((number, "trailing whitespace"))
        if len (line) > limit:
            problems.append ((number, f"longer than {limit} characters"))

        # A directive, and each line it continues onto, is the preprocessor's.
        code, in_comment = code_only (line, in_comment)
        directive        = in_directive or code.lstrip ().startswith ("#")
        in_directive     = directive and code.rstrip ().endswith ("\\")

        if code.lstrip ().startswith ("#"):
            continue

        for match in TIGHT_CALL.finditer (code):
            name = match.group (1)
            if not name.isupper () and not name.startswith ("_"):
                problems.append ((number, f"no space before the parenthesis after '{name}'"))

        if not directive:
            problems += [(number, problem) for problem in braces.line (code.strip ())]

    return [f"{path}:{number}: {problem}" for number, problem in problems]


class Braces:
    """Follows the braces and statements of one file, a line of code at a time."""

    def __init__ (self, constants):
        self.constants = constants
        self.open      = []      # what each unclosed brace opened: namespace, block, do or value
        self.header    = None    # a statement whose parentheses are still open
        self.body      = None    # the statement whose body the next line must open with a brace
        self.tail      = False   # a do loop has just closed, so its while (...); comes next

    def line (self, code):
        """Take the next line of code, and return what is wrong with it."""
        return self.read (code)[:1] if code else []

    def read (self, code):
        tail, self.tail = self.tail, False
        body, self.body = self.body, None

        if self.header:
            return self.statement (code, 0, *self.header)
        if body and code != "{":
            return [f"no braces around the '{body}' body"] + self.scan (code)
        file_scope = all (kind == "namespace" for kind in self.open)
        if self.constants and file_scope and CONSTANT.match (code):
            return ["a file-scope constant is constexpr, not const"] + self.scan (code)

        control = CONTROL.match (code)
        if control:
            return self.statement (code, control.end () - 1, control.group (1), 0, tail)
        word = re.match (r"(else|do)\b", code)
        if word:
            keyword, rest = word.group (1), code[word.end ():].strip ()
            if not rest:
                self.body = keyword
                return []
            return ([f"the '{keyword}' body goes on the lines below, in braces"]
                    + self.scan (rest, keyword))
        if re.fullmatch (r"namespace\b[^{;]*", code):
            self.body = "namespace"
        return self.scan (code, body)

    # Follows a statement's parentheses from index; once they close, its
    # body must start on the next line.
    def statement (self, code, index, keyword, depth, tail):
        for index in range (index, len (code)):
            depth += {"(": 1, ")": -1}.get (code[index], 0)
            if depth == 0:
                break
        else:
            self.header = (keyword, depth, tail)
            return []
        self.header = None
        rest = code[index + 1:].strip ()
        if keyword == "while" and tail and rest == ";":
            return []
        if rest:
            return [f"the '{keyword}' body goes on the lines below, in braces"] + self.scan (rest)
        self.body = keyword
        return []

    # Follows each brace on the line. One opened and closed on the same line
    # holds values or a one-line body; one left open, or closed from an
    # earlier line, must be alone on its line unless it holds values.
    def scan (self, code, body=None):
        problems = []
        here = []
        for index, character in enumerate (code):
            if character == "{":
                before = code[:index].strip ()
                if code == "{":
                    here.append (body if body in ("do", "namespace") else "block")
                elif before.startswith ("namespace"):
                    here.append ("namespace")
                elif BLOCK_AFTER.search (before) or TYPE_BODY.match (before):
                    here.append ("block")
                else:
                    here.append ("value")
            elif character == "}" and here:
                here.pop ()
            elif character == "}":
                kind = self.open.pop () if self.open else "value"
                self.tail = kind == "do"
                alone = not code[:index].strip () and code[index + 1:].strip () in ("", ";", ",")
                if kind in ("block", "do") and not alone:
                    problems.append ("a block's closing brace goes on a line of its own")
        if "block" in here and code != "{":
            problems.append ("a block's opening brace goes on a line of its own")
        self.open += here
        return problems


def check_makefile (path):
    """The Makefile reads as tables: a continued line's backslashes share one
    column, tabs only start a recipe's lines, and nothing trails or runs long."""
    problems = []
    lines    = open (path, encoding="utf-8").read ().split ("\n")
    block    = []

    def aligned (block):
        columns = {len (text) - 1 for _, text in block}
        if len (columns) > 1:
            problems.append (f"{path}:{block[0][0]}: the backslashes of a continued line "
                             f"share one column")

    for number, text in enumerate (lines, 1):
        if len (text) > LIMIT:
            problems.append (f"{path}:{number}: longer than {LIMIT} characters")
        if text != text.rstrip ():
            problems.append (f"{path}:{number}: trailing space")
        if "\t" in text.lstrip ("\t"):
            problems.append (f"{path}:{number}: a tab only starts a recipe line")
        if text.endswith ("\\"):
            block.append ((number, text))
        elif block:
            aligned (block)
            block = []

    return problems


def main (paths):
    problems = [problem for path in paths
                for problem in (check_makefile (path) if os.path.basename (path) == "Makefile"
                                else check (path))]
    if problems:
        print ("\n".join (problems))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit (main (sys.argv[1:]))
