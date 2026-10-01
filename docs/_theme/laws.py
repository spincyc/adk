"""The laws and formulas the lessons rely on, tied to the lessons both ways.

docs/_theme/laws.yml lists the laws, page by page: each page is a file in
docs/laws/, and each law a section of it. A lesson, or a guide page, says
which laws it relies on in its own front matter, and where:

    laws:
      - {law: ohms-law, section: measure-it, for: Finds the LED's current}

`section` is the anchor of the heading whose section relies on the law, and
`for` says what for, in a few words. When the site builds:

    <!-- laws -->        on the laws' index: each page's laws as a table,
                         with where each is first met in either course
    <!-- law ohms-law --> on a law's page, at the start of its section: what
                         the law says, its formula and its units, so each is
                         written once, here in laws.yml
    <!-- relied on -->   on a law's page: every section that relies on its
                         laws, with what it relies on them for

and each section that relies on a law ends with a link to it. The
navigation lists the pages in laws.yml's order. A law nobody relies on, a
use of a law laws.yml doesn't have, a section that isn't on its page, or a
law's page without its markers stops the build.
"""

import os
import posixpath
import re

import yaml
from markdown.extensions.toc import slugify
from mkdocs.exceptions import PluginError

ROOT    = os.path.dirname (os.path.dirname (os.path.dirname (os.path.abspath (__file__))))
DOCS    = os.path.join (ROOT, "docs")
COURSE  = os.path.join (DOCS, "_theme", "course.yml")
FOLDER  = "laws"
TAB     = "Electricity"
GROUP   = "Laws and formulas"
HEADING = re.compile (r"^(#{1,6})\s+(.*?)(?:\s*\{[^}]*?#([\w-]+)[^}]*\})?\s*$")


def front_matter (path):
    with open (path, encoding="utf-8") as file:
        match = re.match (r"---\n(.*?)\n---\n", file.read (), re.S)
    return yaml.safe_load (match[1]) if match else {}


# The course's lessons by slug, with how a page names each.
def lessons ():
    with open (COURSE, encoding="utf-8") as file:
        arcs = yaml.safe_load (file)
    found, counts = {}, {}
    for arc in arcs:
        track = arc.get ("track", "projects")
        for lesson in arc["lessons"]:
            counts[track] = counts.get (track, 0) + 1
            label = f"E{counts[track]:02d}" if track == "electricity" else f"Lesson {counts[track]}"
            found[f"lessons/{lesson['slug']}/index.md"] = dict (
                label=label, shown=f"{label} · {lesson['title']}", track=track, order=len (found))
    return found


# Every page whose front matter names the laws it relies on.
def uses (laws):
    course, found = lessons (), []
    for folder, _, names in os.walk (DOCS):
        if os.path.relpath (folder, DOCS).split (os.sep)[0] == "_theme":
            continue
        for name in sorted (names):
            if not name.endswith (".md"):
                continue
            path = os.path.join (folder, name)
            where = os.path.relpath (path, DOCS).replace (os.sep, "/")
            relied = front_matter (path).get ("laws")
            if not relied:
                continue
            page = course.get (where) or dict (shown=title (path), track="guide", order=len (course))
            for use in relied:
                if use.get ("law") not in laws:
                    raise PluginError (f"{where}: its front matter relies on {use.get ('law')!r}, "
                                       f"which docs/_theme/laws.yml doesn't list")
                if not use.get ("section") or not use.get ("for"):
                    raise PluginError (f"{where}: each law it relies on needs a section and a for")
                found.append (dict (page, where=where, **use))
    return found


def title (path):
    with open (path, encoding="utf-8") as file:
        heading = re.search (r"^# (.+)$", file.read (), re.M)
    return heading[1] if heading else os.path.basename (path)


def load ():
    with open (os.path.join (DOCS, "_theme", "laws.yml"), encoding="utf-8") as file:
        pages = yaml.safe_load (file)
    laws = {}
    for page in pages:
        if not os.path.exists (os.path.join (DOCS, FOLDER, page["page"] + ".md")):
            raise PluginError (f"laws.yml lists {FOLDER}/{page['page']}.md, which doesn't exist")
        for law in page["laws"]:
            if law["id"] in laws:
                raise PluginError (f"laws.yml gives two laws the id {law['id']!r}")
            laws[law["id"]] = dict (law, page=f"{FOLDER}/{page['page']}.md", title=page["title"])
    found = uses (laws)
    for law in laws.values ():
        law["used"] = sorted ((use for use in found if use["law"] == law["id"]),
                              key=lambda use: ({"electricity": 0, "projects": 1, "guide": 2}
                                               [use["track"]], use["order"]))
        if not law["used"]:
            raise PluginError (f"laws.yml: no page relies on {law['id']!r}; say which do in "
                               f"their front matter")
    return pages, laws


PAGES, LAWS = load ()


def on_config (config):
    group = {GROUP: [f"{FOLDER}/index.md"] +
             [{page["title"]: f"{FOLDER}/{page['page']}.md"} for page in PAGES]}
    for item in config["nav"]:
        if isinstance (item, dict) and TAB in item:
            # After the overview and its tools, before the investigations.
            item[TAB].insert (2, group)
            return config
    raise PluginError (f"mkdocs.yml has no {TAB} tab for the laws")


def on_page_markdown (markdown, page, config, files):
    where = page.file.src_uri
    here = posixpath.dirname (where)
    if "<!-- laws -->" in markdown:
        markdown = markdown.replace ("<!-- laws -->", index (here))
    mine = [law for law in LAWS.values () if law["page"] == where]
    if mine:
        for law in mine:
            if f"<!-- law {law['id']} -->" not in markdown:
                raise PluginError (f"{where}: {law['name']} needs <!-- law {law['id']} --> "
                                   f"at the start of its section")
            markdown = markdown.replace (f"<!-- law {law['id']} -->", statement (law))
        if "<!-- relied on -->" not in markdown:
            raise PluginError (f"{where}: a law's page needs <!-- relied on -->")
        markdown = markdown.replace ("<!-- relied on -->", relied_on (mine, here))
    if page.meta.get ("laws"):
        markdown = link_sections (markdown, where, page.meta["laws"])
    return markdown


def link (text, page, section, here):
    target = posixpath.relpath (page, here or ".")
    return f"[{text}]({target}{'#' + section if section else ''})"


def cell (text):
    return str (text).replace ("|", "\\|")


# What a law says, its formula and its units.
def statement (law):
    return "\n".join ([f'<div class="law-statement" markdown>', "",
                       f"{law['words']}", "",
                       f'<p class="formula">{law["formula"]}</p>', "",
                       f'<p class="law-units">{law["units"]}</p>', "", "</div>"])


# Each page's laws as a table under the page's own heading, with the first
# investigation and the first project lesson that rely on each.
def index (here):
    out = []
    for page in PAGES:
        path = f"{FOLDER}/{page['page']}.md"
        out += [f"### {link (page['title'], path, '', here)}", "",
                "| Law | Formula | In words | Met first in |", "|---|---|---|---|"]
        for entry in page["laws"]:
            law = LAWS[entry["id"]]
            first = [next ((use for use in law["used"] if use["track"] == track), None)
                     for track in ("electricity", "projects")]
            met = ", ".join (link (use["label"], use["where"], use["section"], here)
                             for use in first if use) or \
                link (law["used"][0]["shown"], law["used"][0]["where"], law["used"][0]["section"],
                      here)
            out.append (f"| {link (law['name'], path, law['id'], here)} | {cell (law['formula'])} "
                        f"| {cell (law['words'])} | {met} |")
        out.append ("")
    return "\n".join (out)


# Every section that relies on a page's laws: the investigations first,
# where the laws are measured, then the project lessons, then the guides.
def relied_on (laws, here):
    several = len (laws) > 1
    out = ["| Where | Law | What it relies on it for |" if several else
           "| Where | What it relies on it for |",
           "|---|---|---|" if several else "|---|---|"]
    order = {"electricity": 0, "projects": 1, "guide": 2}
    rows = sorted (((use, law) for law in laws for use in law["used"]),
                   key=lambda pair: (order[pair[0]["track"]], pair[0]["order"],
                                     laws.index (pair[1])))
    for use, law in rows:
        middle = f" [{law['name']}](#{law['id']}) |" if several else ""
        out.append (f"| {link (use['shown'], use['where'], use['section'], here)} |{middle} "
                    f"{cell (use['for'])} |")
    return "\n".join (out)


# A link to each law a section relies on, at the end of that section.
def link_sections (markdown, where, relied):
    lines = markdown.split ("\n")
    headings, fenced = [], False
    for number, line in enumerate (lines):
        if line.lstrip ().startswith (("```", "~~~")):
            fenced = not fenced
        match = None if fenced else HEADING.match (line)
        if match:
            text = re.sub (r"\[([^\]]*)\]\([^)]*\)", r"\1", match[2])
            headings.append ((number, len (match[1]), match[3] or slugify (text, "-")))
    ends, here = {}, posixpath.dirname (where)
    for use in relied:
        found = next ((index for index, (_, _, anchor) in enumerate (headings)
                       if anchor == use["section"]), None)
        if found is None:
            raise PluginError (f"{where}: its front matter names section {use['section']!r} "
                               f"for {use['law']}, but no heading has that anchor")
        start, level, _ = headings[found]
        end = next ((number for number, deeper, _ in headings[found + 1:] if deeper <= level),
                    len (lines))
        law = LAWS[use["law"]]
        named = ends.setdefault (end, [])
        if law["id"] not in [item["id"] for item in named]:
            named.append (law)
    for end in sorted (ends, reverse=True):
        names = " · ".join (link (law["name"], law["page"], law["id"], here) for law in ends[end])
        lines[end:end] = ["", f"Laws behind this: {names}", '{: .law-links }', ""]
    return "\n".join (lines)
