"""Reference pages straight from the headers.

The library's headers follow one layout: a comment saying what a part is and
how to wire it, then the struct, with a comment above each public call that
needs one. document () turns that into Markdown, so the reference can never
drift from the code.

    <!-- api led.h Led -->     the Led struct in src/adk/led.h
    <!-- api i2c.h -->         the free functions in src/adk/i2c.h
"""

import re


def document (path, name=None):
    lines = open (path, encoding="utf-8").read ().splitlines ()
    if name:
        return document_struct (lines, name, path)
    return document_functions (lines, path)


def document_struct (lines, name, path):
    start = next ((i for i, line in enumerate (lines)
                   if re.match (rf"\s*struct {name}\b(?!;)", line)), None)
    if start is None:
        raise ValueError (f"{path}: no struct {name}")
    description = comment_above (lines, start)

    body = []
    depth = 0
    for line in lines[start + 1:]:
        stripped = line.strip ()
        if depth == 1 and re.match (r"(protected|private):", stripped):
            break
        depth += stripped.count ("{") - stripped.count ("}")
        if depth <= 0 and stripped.startswith ("}"):
            break
        if depth >= 1 and not (depth == 1 and stripped == "{"):
            body.append (line)

    calls = without_const_twins (declarations (body))
    out = [f"### {name}", "", paragraphs (description), ""]
    if calls:
        out += ["| Call | Does |", "|---|---|"]
        out += [f"| `{signature}` | {comment} |" for signature, comment in calls]
    return "\n".join (out)


def document_functions (lines, path):
    body = []
    depth = 0
    for line in lines:
        depth += line.count ("{") - line.count ("}")
        if re.match (r"\s*struct \w+", line):
            depth = 1000        # skip struct bodies entirely
        if depth >= 1000 and "};" in line:
            depth -= 1000
            continue
        if depth < 1000:
            body.append (line)
    calls = [(signature, comment) for signature, comment in declarations (body)
             if "(" in signature and not signature.startswith (("namespace", "#"))]
    out = ["| Call | Does |", "|---|---|"]
    out += [f"| `{signature}` | {comment} |" for signature, comment in calls]
    return "\n".join (out)


def comment_above (lines, index):
    comment = []
    i = index - 1
    if i >= 0 and lines[i].strip ().startswith ("template"):
        i -= 1
    while i >= 0 and lines[i].strip ().startswith ("//"):
        comment.insert (0, lines[i].strip ()[2:])
        i -= 1
    return comment


# Collect each public declaration with the comment directly above it.
def declarations (body):
    calls = []
    comment = []
    pending = ""
    skipping = 0
    for line in body:
        stripped = line.strip ()
        if skipping:
            skipping += stripped.count ("{") - stripped.count ("}")
            continue
        if not stripped:
            if not pending:
                comment = []
            continue
        if stripped.startswith ("//"):
            comment.append (stripped[2:].strip ())
            continue
        if stripped.startswith (("#", "namespace", "}", "using ", "friend ")) and \
                not pending.startswith ("enum"):
            continue
        pending = (pending + " " + stripped).strip ()
        # An enum is shown whole, with its values.
        if pending.startswith ("enum"):
            if pending.endswith ("};"):
                calls.append ((clean (pending.replace ("{ ", "{").replace (" }", "}")),
                               " ".join (comment)))
                pending, comment = "", []
            continue
        if pending.endswith ((";", "}")) or pending.endswith ("{") or stripped == "{":
            if pending.endswith ("{") or stripped == "{":
                skipping = 1
            signature = clean (pending)
            pending = ""
            if keep (signature):
                calls.append ((signature, " ".join (comment)))
            comment = []
    return calls


def clean (signature):
    signature = re.sub (r"\s+", " ", signature)
    signature = re.sub (r"\s*;\s*$", "", signature)
    signature = re.sub (r"\s*\{\s*$", "", signature)
    if not signature.startswith ("enum"):
        signature = without_body (signature)
    signature = re.sub (r"\s+override$", "", signature)
    signature = re.sub (r"^template <[^>]*> ", "", signature)
    signature = re.sub (r"^constexpr ", "", signature)
    return signature.replace ("|", "\\|").strip ()


# An accessor defined in the header shows only its declaration; so does a
# constructor, without its initializer list.
def without_body (signature):
    if signature.endswith ("}"):
        depth = 0
        for at in range (len (signature) - 1, -1, -1):
            depth += {"}": 1, "{": -1}.get (signature[at], 0)
            if depth == 0:
                signature = signature[:at].rstrip ()
                break
    return re.sub (r"\)\s*:\s.*$", ")", signature)


def keep (signature):
    return not (signature.startswith (("~", "struct ")) or "= delete" in signature or
                signature.startswith (("{", "}")) or not signature)


# A container's const overloads mirror its others: show each call once.
def without_const_twins (calls):
    signatures = {signature for signature, _ in calls}
    return [(signature, comment) for signature, comment in calls
            if not (signature.endswith (" const") and
                    re.sub (r"^const |(?<=\)) const$", "", signature) in signatures)]


def paragraphs (comment):
    # Blank comment lines split paragraphs; lines indented further than the
    # text are a preformatted block, such as a wiring table.
    out, text, block = [], [], []
    for line in comment + [""]:
        if line.startswith ("  ") and line.strip ():
            if text:
                out.append (" ".join (text))
                text = []
            block.append (line[1:])
            continue
        if block:
            out.append ("```text\n" + "\n".join (block) + "\n```")
            block = []
        if line.strip ():
            text.append (line.strip ())
        elif text:
            out.append (" ".join (text))
            text = []
    return "\n\n".join (out)
