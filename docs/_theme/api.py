"""Reference pages straight from the headers.

The library's headers follow one layout: a comment saying what a part is and
how to wire it, then the struct, with a comment above each public call that
needs one. A comment covers the calls right after it, up to a blank line,
as in

    // Find the next station up or down the band.
    void seekUp   ();
    void seekDown ();

document () turns that into Markdown, so the reference can never drift from
the code: the calls that need no comment listed plainly, then a table of
the others with what each does.

    <!-- api led.h Led -->     the Led struct in src/adk/led.h
    <!-- api i2c.h -->         the free functions in src/adk/i2c.h

A header's free functions leave out its structs, which have markers of their
own, and the helpers its other functions call, such as print.h's
printPart (). A function with no comment of its own that makes the struct
just above it, as fixed () makes a Fixed, takes the struct's comment.
"""

import re


def document (path, name=None):
    lines = open (path, encoding="utf-8").read ().splitlines ()
    if name:
        return document_struct (lines, name, path)
    return document_functions (lines)


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

    out = [f"### {name}", "", paragraphs (description), ""]
    return "\n".join (out + calls (without_const_twins (declarations (body))))


def document_functions (lines):
    body, made = [], {}
    index = 0
    while index < len (lines):
        match = re.match (r"\s*struct (\w+)\b(?!.*;)", lines[index])
        if not match:
            body.append (lines[index])
            index += 1
            continue
        # A struct is left to its own marker; its comment and template line
        # go with it, and a blank line stands in its place.
        made[match.group (1)] = " ".join (line.strip () for line in comment_above (lines, index))
        while body and body[-1].strip ().startswith (("//", "template")):
            body.pop ()
        depth = 0
        while True:
            depth += lines[index].count ("{") - lines[index].count ("}")
            index += 1
            if depth == 0 and "}" in lines[index - 1]:
                break
        body.append ("")
    # Functions only, the header's types having markers of their own.
    groups = only ([[[s for s in signatures if "(" in s], comment]
                    for signatures, comment in declarations (body)])
    for group in groups:
        returns = re.match (r"(?:inline )?(\w+) ", group[0][0])
        if not group[1] and returns and returns.group (1) in made:
            group[1] = made[returns.group (1)]
    # A helper is a function the header's own functions call.
    code = "\n".join (line for line in lines if not line.strip ().startswith ("//"))
    declared = [name_of (signature) for signatures, _ in groups for signature in signatures]
    helper = lambda signature: len (re.findall (rf"\b{name_of (signature)} \(", code)) > \
        declared.count (name_of (signature))
    return "\n".join (calls (only ([[[s for s in signatures if comment or not helper (s)], comment]
                                    for signatures, comment in groups])))


def only (groups):
    return [group for group in groups if group[0]]


def name_of (signature):
    match = re.search (r"([\w:~]+|operator\S+) \(", signature)
    return re.escape (match.group (1)) if match else None


def comment_above (lines, index):
    comment = []
    i = index - 1
    if i >= 0 and lines[i].strip ().startswith ("template"):
        i -= 1
    while i >= 0 and lines[i].strip ().startswith ("//"):
        comment.insert (0, lines[i].strip ()[2:])
        i -= 1
    return comment


# Each public declaration, in groups: the calls a comment covers, up to a
# blank line or the next comment, as [signatures, comment]. Calls with no
# comment above them make groups of their own, with an empty comment.
def declarations (body):
    groups, group = [], None
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
                comment, group = [], None
            continue
        if stripped.startswith ("//"):
            if group:
                comment, group = [], None
            comment.append (stripped[2:].strip ())
            continue
        if stripped.startswith (("#", "namespace", "}", "using ", "friend ")) and \
                not pending.startswith ("enum"):
            continue
        pending = (pending + " " + stripped).strip ()
        # An enum is shown whole, with its values.
        if pending.startswith ("enum"):
            if pending.endswith ("};"):
                signature, pending = clean (pending.replace ("{ ", "{").replace (" }", "}")), ""
                group = [[signature], " ".join (comment)]
                groups.append (group)
            continue
        if pending.endswith ((";", "}")) or pending.endswith ("{") or stripped == "{":
            if pending.endswith ("{") or stripped == "{":
                skipping = 1
            signature = clean (pending)
            pending = ""
            if keep (signature):
                if group is None:
                    group = [[], " ".join (comment)]
                    groups.append (group)
                group[0].append (signature)
    return groups


# The calls that need no words listed plainly, as the header declares them,
# and a table of the others with what each group does.
def calls (groups):
    plain = [signature for signatures, comment in groups if not comment for signature in signatures]
    described = [(signatures, comment) for signatures, comment in groups if comment]
    out = []
    if plain:
        out += ["```cpp"] + [s if s.endswith (";") else s + ";" for s in plain] + ["```", ""]
    if described:
        out += ["| Call | Does |", "|---|---|"]
        out += ["| " + "<br>".join (f"`{cell (s)}`" for s in signatures) + f" | {cell (comment)} |"
                for signatures, comment in described]
    return out


def cell (text):
    return text.replace ("|", "\\|")


def clean (signature):
    signature = re.sub (r"\s+", " ", signature)
    signature = re.sub (r"\s*;\s*$", "", signature)
    signature = re.sub (r"\s*\{\s*$", "", signature)
    if not signature.startswith ("enum"):
        signature = without_body (signature)
    signature = re.sub (r"\s+override$", "", signature)
    signature = re.sub (r"^template <[^>]*> ", "", signature)
    signature = re.sub (r"^constexpr ", "", signature)
    return signature.strip ()


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


# Not a call: a destructor, a deleted function, a nested struct, or a
# deduction guide such as Array (T, More...) -> Array<...>.
def keep (signature):
    return not (signature.startswith (("~", "struct ")) or "= delete" in signature or
                signature.startswith (("{", "}")) or not signature or
                re.match (r"[\w:]+ \(.*\) -> ", signature))


# A container's const overloads mirror its others: show each call once.
def without_const_twins (groups):
    signatures = {signature for group, _ in groups for signature in group}
    twin = lambda signature: signature.endswith (" const") and \
        re.sub (r"^const |(?<=\)) const$", "", signature) in signatures
    return only ([[[signature for signature in group if not twin (signature)], comment]
                  for group, comment in groups])


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
