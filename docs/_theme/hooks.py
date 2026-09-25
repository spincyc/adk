"""Build lesson pages from their parts.

A lesson page is docs/lessons/NN-name/index.md, with a circuit.py beside it
describing the build and a sketch in examples/. Markers in the page are
replaced when the site builds:

    <!-- bench -->         the pencil drawing of the whole bench
    <!-- closeup -->       a close-up of the breadboard
    <!-- steps -->         the build, step by step
    <!-- connections -->   which Mega pin reaches which part
    <!-- sketch -->        the example sketch, exactly as it compiles
    <!-- measure -->       each reading to take with a multimeter, drawn, and
                           a table of them all

Any page may also use <!-- arcs --> for the course as cards,
<!-- course --> for the course as a table, and <!-- api led.h Led --> for a
part's reference, read from its header. The course comes from course.yml,
which also builds the Course navigation from the lessons that exist.

`make pins` holds each sketch to its circuit: it runs the sketch's setup ()
on the host and fails unless the pins it claims are exactly the pins the
circuit wires, so a drawing can never show a wire the code doesn't use, or
miss one it does.

When a lesson shares parts in the same holes, or wires between the same
points, with the lesson before it, its steps say what to keep, what to take
out and what to add, so a learner carries the build on rather than starting
again.
"""

import os
import re
import sys
from xml.sax.saxutils import escape

import yaml
from mkdocs.exceptions import PluginError

sys.path.insert (0, os.path.dirname (__file__))

from api import document  # noqa: E402
from bench import HC595, L293D, Bench, canonical  # noqa: E402

ROOT = os.path.dirname (os.path.dirname (os.path.dirname (os.path.abspath (__file__))))

COURSE = yaml.safe_load (open (os.path.join (os.path.dirname (__file__), "course.yml"),
                               encoding="utf-8"))
LESSONS = [dict (lesson, arc=arc["arc"], number=index)
           for index, lesson in enumerate ((lesson for arc in COURSE for lesson in arc["lessons"]), 1)
           for arc in [next (a for a in COURSE if lesson in a["lessons"])]]


def written (lesson):
    return os.path.exists (os.path.join (ROOT, "docs", "lessons", lesson["slug"], "index.md"))


# The Course tab lists every arc that has a written lesson.
def on_config (config):
    for item in config["nav"]:
        if isinstance (item, dict) and "Course" in item:
            sections = []
            for arc in COURSE:
                pages = [f"lessons/{lesson['slug']}/index.md" for lesson in arc["lessons"]
                         if written (lesson)]
                if pages:
                    sections.append ({arc["arc"]: pages})
            item["Course"] = ["course.md"] + sections
    return config


def on_page_markdown (markdown, page, config, files):
    meta = page.meta
    markdown = markdown.replace ("<!-- arcs -->", arcs ())
    markdown = markdown.replace ("<!-- course -->", course_table ())
    markdown = re.sub (r"<!-- drawing (\S+) (bench|closeup) -->", drawing, markdown)
    markdown = re.sub (r"<!-- api (\S+)(?: (\w+))? -->", reference, markdown)
    markdown = link_lessons (markdown, page)
    if "lesson" not in meta:
        return markdown

    lesson = LESSONS[meta["lesson"] - 1]
    if page.file.src_uri != f"lessons/{lesson['slug']}/index.md":
        raise PluginError (f"{page.file.src_uri}: course.yml calls lesson {meta['lesson']} "
                           f"{lesson['slug']}")
    meta["arc"] = lesson["arc"]
    meta["project"] = lesson.get ("project", False)
    # Lessons put their sections in the header, leaving the page wide
    # enough for the drawings.
    meta["hide"] = ["toc"]

    folder = os.path.dirname (page.file.abs_src_path)
    bench = load_bench (os.path.join (folder, "circuit.py"))
    sketch_name = meta["sketch"]
    sketch_path = os.path.join (ROOT, "examples", sketch_name, sketch_name + ".ino")
    sketch = open (sketch_path, encoding="utf-8").read ()

    try:
        drawings = bench.svg ("bench", "bench"), bench.svg ("closeup", "closeup")
    except ValueError as error:
        raise PluginError (f"{page.file.src_uri}: {error}") from error
    replacements = {
        "bench": figure (drawings[0], bench.title, "bench"),
        "closeup": figure (drawings[1],
                           "Close-up of the breadboard. Letters name the rows, numbers the columns.",
                           "closeup"),
        "steps": steps (bench, previous (meta["lesson"])),
        "connections": connections (bench),
        "sketch": f'```cpp title="{sketch_name}.ino" linenums="1"\n{sketch.rstrip ()}\n```',
        "measure": measurements (bench),
    }
    for marker, content in replacements.items ():
        markdown = markdown.replace (f"<!-- {marker} -->", content)
    return markdown


# "Lesson 7" in running text becomes a link once Lesson 7 is written. Code,
# headings, existing links and the page's own number are left alone.
def link_lessons (markdown, page):
    here = os.path.dirname (page.file.src_uri)
    own = page.meta.get ("lesson")
    out, fenced = [], False

    def replace (match):
        number = int (match.group (1))
        if number == own or not 1 <= number <= len (LESSONS) or not written (LESSONS[number - 1]):
            return match.group (0)
        target = f"lessons/{LESSONS[number - 1]['slug']}/index.md"
        return f"[{match.group (0)}]({os.path.relpath (target, here or '.')})"

    for line in markdown.split ("\n"):
        if line.lstrip ().startswith (("```", "~~~")):
            fenced = not fenced
        if fenced or line.lstrip ().startswith (("#", "<", "|---")):
            out.append (line)
            continue
        # Leave inline code and existing links as they are.
        pieces = re.split (r"(`[^`]*`|\[[^\]]*\]\([^)]*\))", line)
        for index in range (0, len (pieces), 2):
            pieces[index] = re.sub (r"\bLesson (\d{1,2})\b(?!\s*\])", replace, pieces[index])
        out.append ("".join (pieces))
    return "\n".join (out)


# The reference for a part, read from its header.
def reference (match):
    header, name = match.groups ()
    try:
        return document (os.path.join (ROOT, "src", "adk", header), name)
    except (OSError, ValueError) as error:
        raise PluginError (str (error)) from error


# A drawing from any lesson, for use on another page.
def drawing (match):
    slug, view = match.groups ()
    bench = load_bench (os.path.join (ROOT, "docs", "lessons", slug, "circuit.py"))
    return figure (bench.svg (view, f"{slug}-{view}"), bench.title, view)


def load_bench (path):
    scope = {"Bench": Bench, "HC595": HC595, "L293D": L293D}
    try:
        exec (compile (open (path, encoding="utf-8").read (), path, "exec"), scope)
        return scope["bench"].finish ()
    except Exception as error:
        raise PluginError (f"{path}: {error}") from error


# The lesson before this one, and its bench, when it is written.
def previous (number):
    if number < 2 or not written (LESSONS[number - 2]):
        return None
    lesson = LESSONS[number - 2]
    return lesson, load_bench (os.path.join (ROOT, "docs", "lessons", lesson["slug"], "circuit.py"))


def figure (svg, caption, kind):
    return (f'<figure class="bench-figure bench-{kind}" markdown="0">\n{svg}\n'
            f'<figcaption>{caption}</figcaption>\n</figure>')


# Each measurement drawn small, side by side, numbered as the table below
# them lists them.
def measurements (bench):
    if not bench.measurements:
        return ""
    figures, rows = [], []
    for index, taken in enumerate (bench.measurements, 1):
        (_, red), (_, black) = bench.probes (index - 1)
        figures.append (
            f'<figure class="bench-figure bench-measure" markdown="0" '
            f'style="flex: 1 1 9rem; max-width: 12rem; margin: 0">\n{bench.measure_svg (index - 1)}\n'
            f'<figcaption>{index}. {escape (taken["label"])}</figcaption>\n</figure>')
        rows.append (f"| {index}. {taken['label']} | {red} | {black} | {taken['expect']} | "
                     f"{taken['when'] or ''} |")
    table = ["| Measurement | Red probe | Black probe | Expect | When |",
             "|---|---|---|---|---|"] + rows
    return ('<div class="measurements" markdown="0" style="display: flex; flex-wrap: wrap; '
            'gap: 1rem; align-items: flex-end; margin: 1.25rem 0">\n' + "\n".join (figures) +
            "\n</div>\n\n" + "\n".join (table))


def steps (bench, before=None):
    kept, gone, new = continuity (bench, before[1]) if before else ([], [], bench.items)
    # Only a part or module carried over makes it a continuation; the power
    # wires alone don't.
    if not [kind for kind, _ in kept if kind != "wire"]:
        kept, new = [], bench.items
    lines = [f"{index}. {step}" for index, (_, _, step) in enumerate (new, 1)]
    if kept:
        lesson = before[0]
        things = kept_words (before[1], kept)
        keep = (f"**Keep from [Lesson {LESSONS.index (lesson) + 1}](../{lesson['slug']}/index.md):** "
                f"{join (things)}, just as {'they are' if len (kept) > 1 else 'it is'}.")
        head = [keep, ""]
        if len (gone) > 8:
            head += [f"**Take out** everything else from Lesson {LESSONS.index (lesson) + 1}.", ""]
        elif gone:
            head += [f"**Take out:** {'; '.join (gone)}.", ""]
        head += ["**Add:**" if new else "Nothing else to add.", ""]
        lines = head + lines
    return '<div class="build-steps" markdown>\n\n' + "\n".join (lines) + "\n\n</div>"


# What this bench keeps from the one before (named), what it takes out
# (described), and the items it adds: parts and modules that stand where
# they stood, and wires between the same two points, are kept.
def continuity (bench, before):
    here = {identity (bench, kind, thing): (kind, thing) for kind, thing, _ in bench.items}
    there = {identity (before, kind, thing): (kind, thing) for kind, thing, _ in before.items}
    kept = [there[key] for key in there if key in here]
    gone = [describe (before, *there[key]) for key in there if key not in here]
    new = [item for item in bench.items if identity (bench, item[0], item[1]) not in there]
    return kept, gone, new


def identity (bench, kind, thing):
    if kind == "part":
        holes = tuple (hole for _, hole in thing.legs ())
        settings = (thing.top, thing.bottom) if hasattr (thing, "top") else ()
        return ("part", type (thing).__name__, thing.name, holes, settings)
    if kind == "module":
        return ("module", type (thing.kind).__name__, thing.title, round (thing.x), round (thing.y),
                thing.angle)
    ends = frozenset (end_identity (bench, end) for end in thing[:2])
    return ("wire", ends)


def end_identity (bench, end):
    kind, where = end
    if kind == "module":
        placed, pin = bench._module_pin (where)
        return ("module", placed.title, pin.name)
    return end


# What is kept, in words: the parts, each module with its own wires, the
# Mega's power wires, and how many other wires.
def kept_words (bench, kept):
    modules = {thing.name: thing for kind, thing in kept if kind == "module"}
    parts = [f"the {thing.name}" for kind, thing in kept if kind == "part"]
    wires = [thing for kind, thing in kept if kind == "wire"]
    power = [wire for wire in wires if bench._standard (wire[:2])]
    counts = {}
    rest = 0
    for wire in wires:
        if wire in power:
            continue
        owners = [end[1].partition (".")[0] for end in wire[:2] if end[0] == "module"]
        if owners and owners[0] in modules:
            counts[owners[0]] = counts.get (owners[0], 0) + 1
        else:
            rest += 1
    things = gather (parts)
    for key, placed in modules.items ():
        count = counts.get (key, 0)
        things.append (f"the {placed.title}" + (f" and its {count} wire{'s' if count > 1 else ''}"
                                                 if count else ""))
    feeds = [canonical (end[1]) for wire in power for end in wire[:2] if end[0] == "pin"]
    if feeds:
        things.append (f"the {' and '.join (sorted (feeds))} wire{'s' if len (feeds) > 1 else ''} "
                       f"from the Mega")
    if len (power) > len (feeds):
        links = len (power) - len (feeds)
        things.append (f"the link{'s' if links > 1 else ''} between the rails")
    if rest:
        things.append (f"{rest} {'other ' if things else ''}wire{'s' if rest > 1 else ''}")
    return things


def describe (bench, kind, thing):
    if kind == "module":
        return f"the {thing.title}"
    if kind == "part":
        holes = [hole for _, hole in thing.legs ()]
        return f"the {thing.name} ({holes[0]}{'–' + holes[-1] if len (holes) > 1 else ''})"
    start, end, color, _ = thing
    return f"the {color} wire from {bench.describe (start)} to {bench.describe (end)}"


# Like parts once each: "the 220 Ω resistor" three times is "three 220 Ω
# resistors".
def gather (names):
    counts = {}
    for name in names:
        counts[name] = counts.get (name, 0) + 1
    words = ["", "", "two", "three", "four", "five", "six", "seven", "eight", "nine"]
    out = []
    for name, count in counts.items ():
        if count == 1:
            out.append (name)
        else:
            many = words[count] if count < len (words) else str (count)
            out.append (f"{many} {name.removeprefix ('the ')}s")
    return out


def join (things):
    return things[0] if len (things) == 1 else ", ".join (things[:-1]) + " and " + things[-1]


def connections (bench):
    lines = []
    for pins, legs in bench.connections ():
        things = [f"**{pin}**" for pin in pins] + legs
        lines.append ("- " + " — ".join (things))
    return '<div class="connections" markdown>\n\n' + "\n".join (lines) + "\n\n</div>"


def link (lesson, text=None):
    text = text or lesson["title"]
    return f"[{text}](lessons/{lesson['slug']}/index.md)" if written (lesson) else text


def arcs ():
    cards = ['<div class="arcs" markdown>']
    number = 0
    for index, arc in enumerate (COURSE, 1):
        cards.append (f'<section class="arc" markdown>\n\n<p class="arc-number">{index}</p>\n\n'
                      f'### {arc["arc"]}\n')
        for lesson in arc["lessons"]:
            number += 1
            item = f"{number}. {link (lesson)}"
            if lesson.get ("project"):
                item = f'{number}. <span class="project">★ {link (lesson)}</span>'
            cards.append (item)
        cards.append ("\n</section>")
    cards.append ("</div>")
    return "\n".join (cards)


def course_table ():
    rows = []
    for arc in COURSE:
        rows += [f"\n### {arc['arc']}\n", "| | Lesson | You build |", "|--:|---|---|"]
        for lesson in arc["lessons"]:
            number = LESSONS.index (next (l for l in LESSONS if l["slug"] == lesson["slug"])) + 1
            title = link (lesson)
            if lesson.get ("project"):
                title = f"★ {title}"
            rows.append (f"| {number:02d} | {title} | {lesson['builds']} |")
    return "\n".join (rows)
