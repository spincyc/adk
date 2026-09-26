"""Build lesson pages from their parts.

A lesson page is docs/lessons/NN-name/index.md, with a circuit.py beside it
describing the build and a sketch in examples/. Its title and arc come from
course.yml, and its sketch from its name: examples/Lesson13HelloLcd for
13-hello-lcd. Markers in the page are replaced when the site builds:

    <!-- bench -->         the pencil drawing of the whole bench
    <!-- closeup -->       a close-up of the breadboard
    <!-- steps -->         the build, step by step
    <!-- connections -->   which Mega pin reaches which part
    <!-- sketch -->        the example sketch, exactly as it compiles
    <!-- measure -->       each reading to take with a multimeter, drawn, and
                           a table of them all

A two-board lesson's page gives each marker a board's letter, as in
<!-- bench A --> and <!-- sketch B -->; its circuit.py describes both
builds, and each board's sketch is in a folder of its own in the lesson's
example (docs/contributing.md, Two-board lessons).

Any page may also use <!-- arcs --> for the course as cards,
<!-- course --> for the course as a table, <!-- drawing 01-blink closeup -->
for a drawing from a lesson (with a board's letter after it in a two-board
lesson), and <!-- api led.h Led --> for a part's reference, read from its
header. The course comes from course.yml, which also builds the Course
navigation from the lessons that exist.

`make pins` holds each sketch to its circuit (tests/pins.py says exactly
how): the pins it claims must be the pins the circuit wires, each claimed
as an output or an input as the parts on it need.

When a lesson shares parts in the same holes, or wires between the same
points, with the lesson before it, its steps say what to keep, what to take
out and what to add, so a learner carries the build on rather than starting
again; when only the Mega's power wires carry over, they say to take out
everything else.
"""

import os
import re
import sys
from xml.sax.saxutils import escape

import yaml
from mkdocs.exceptions import PluginError

sys.path.insert (0, os.path.dirname (__file__))

from api import document  # noqa: E402
from bench import canonical, example, load, unbroken  # noqa: E402
from drawing import Drawing  # noqa: E402

ROOT = os.path.dirname (os.path.dirname (os.path.dirname (os.path.abspath (__file__))))

COURSE = yaml.safe_load (open (os.path.join (os.path.dirname (__file__), "course.yml"),
                               encoding="utf-8"))
LESSONS = [dict (lesson, arc=arc["arc"], boards=arc.get ("boards", 1))
           for arc in COURSE for lesson in arc["lessons"]]
for number, lesson in enumerate (LESSONS, 1):
    lesson["number"] = number
MARKERS = ("bench", "closeup", "steps", "connections", "sketch", "measure")
NBSP = "\u00a0"


def written (lesson):
    return os.path.exists (os.path.join (ROOT, "docs", "lessons", lesson["slug"], "index.md"))


# The Course tab lists every arc that has a written lesson, each lesson by
# its number, and each arc with the numbers of its lessons: "13 · Hello,
# LCD" in "Words and weather · 13–15".
def on_config (config):
    for item in config["nav"]:
        if isinstance (item, dict) and "Course" in item:
            sections = []
            for arc in COURSE:
                lessons = [lesson for lesson in LESSONS if lesson["arc"] == arc["arc"]]
                pages = [{f"{lesson['number']}{NBSP}·{NBSP}{lesson['title']}":
                          f"lessons/{lesson['slug']}/index.md"}
                         for lesson in lessons if written (lesson)]
                if pages:
                    sections.append ({arc_title (arc, lessons): pages})
            item["Course"] = ["course.md"] + sections
    return config


def arc_title (arc, lessons):
    title = f"{arc['arc']}{NBSP}·{NBSP}{lessons[0]['number']}–{lessons[-1]['number']}"
    return title + (f"{NBSP}·{NBSP}two boards" if arc.get ("boards", 1) == 2 else "")


def on_page_markdown (markdown, page, config, files):
    meta = page.meta
    markdown = markdown.replace ("<!-- arcs -->", arcs ())
    markdown = markdown.replace ("<!-- course -->", course_table ())
    markdown = re.sub (r"<!-- drawing (\S+) (bench|closeup)(?: ([A-Z]))? -->", lesson_drawing,
                       markdown)
    markdown = re.sub (r"<!-- api (\S+)(?: (\w+))? -->", reference, markdown)
    markdown = link_lessons (markdown, page)
    if "lesson" not in meta:
        return markdown

    lesson = LESSONS[meta["lesson"] - 1]
    where = page.file.src_uri
    if where != f"lessons/{lesson['slug']}/index.md":
        raise PluginError (f"{where}: course.yml calls lesson {meta['lesson']} {lesson['slug']}")
    # The title, arc and sketch come from course.yml and the lesson's name;
    # a page that gives them anyway must agree.
    derived = {"title": lesson["title"], "arc": lesson["arc"]}
    if lesson["boards"] == 1:
        derived["sketch"] = example (lesson["slug"])
    for key, value in meta.items ():
        if key in ("title", "arc", "sketch") and derived.get (key) != value:
            raise PluginError (f"{where}: its {key} comes from course.yml and its name "
                               f"({derived.get (key, 'none: each board has its own')}); leave "
                               f"{key}: {value} out")
    meta.update (derived, project=lesson.get ("project", False))
    # Lessons put their sections in the header, leaving the page wide
    # enough for the drawings.
    meta["hide"] = ["toc"]

    boards = load_circuit (lesson)
    if (lesson["boards"] == 2) != ("" not in boards):
        raise PluginError (f"{where}: course.yml gives its arc {lesson['boards']} board(s), but "
                           f"its circuit.py describes {len (boards)}")
    markers = re.findall (r"<!-- (" + "|".join (MARKERS) + r")(?: ([A-Z]))? -->", markdown)
    for marker, letter in markers:
        if letter not in boards:
            raise PluginError (f"{where}: <!-- {marker} {letter} --> names no board of its circuit"
                               if letter else f"{where}: a two-board page marks each board's "
                               f"{marker}, as <!-- {marker} A -->")
    for letter, bench in boards.items ():
        replacements = board_pieces (lesson, letter, bench)
        for marker, content in replacements.items ():
            markdown = markdown.replace (f"<!-- {marker}{' ' + letter if letter else ''} -->",
                                         content)
    return markdown


# Everything the markers show of one board.
def board_pieces (lesson, letter, bench):
    # examples/Lesson44RemoteDial/Dial/Dial.ino for a board, or
    # examples/Lesson01Blink/Lesson01Blink.ino for a lesson's only one.
    name = bench.sketch or example (lesson["slug"])
    folder = os.path.join (ROOT, "examples", example (lesson["slug"]), bench.sketch or "")
    path = os.path.join (folder, name + ".ino")
    try:
        sketch = open (path, encoding="utf-8").read ()
    except OSError as error:
        raise PluginError (f"{lesson['slug']}: no sketch for board {letter or 'of the lesson'} at "
                           f"{os.path.relpath (path, ROOT)}") from error
    whose = f"Board {letter}'s breadboard" if letter else "the breadboard"
    try:
        drawn = Drawing (bench)
        drawings = drawn.svg ("bench", "bench" + letter), drawn.svg ("closeup", "closeup" + letter)
        return {
            "bench": figure (drawings[0], bench.title, "bench"),
            "closeup": figure (drawings[1],
                               f"Close-up of {whose}. Letters name the rows, numbers the columns.",
                               "closeup"),
            "steps": steps (bench, previous (lesson["number"], letter)),
            "connections": connections (bench),
            "sketch": f'```cpp title="{name}.ino" linenums="1"\n{sketch.rstrip ()}\n```',
            "measure": measurements (bench, drawn, "measure" + letter),
        }
    except ValueError as error:
        raise PluginError (f"{lesson['slug']}: {error}") from error


# "Lesson 7" in running text becomes a link once Lesson 7 is written, and so
# does each number in "Lessons 10, 11 and 12" or "Lessons 37 to 42". Code,
# headings, existing links and the page's own number are left alone.
REFERENCE = re.compile (r"\b(?:(Lesson) (\d{1,2})|(Lessons) (\d{1,2})"
                        r"((?:, \d{1,2})*(?:,? and \d{1,2}|–\d{1,2}| to \d{1,2})?))\b(?!\s*\])")


def link_lessons (markdown, page):
    here = os.path.dirname (page.file.src_uri)
    own = page.meta.get ("lesson")
    out, fenced = [], False

    def linked (text, number):
        if number == own or not 1 <= number <= len (LESSONS) or not written (LESSONS[number - 1]):
            return text
        target = f"lessons/{LESSONS[number - 1]['slug']}/index.md"
        return f"[{text}]({os.path.relpath (target, here or '.')})"

    def replace (match):
        word, first = match.group (1, 2) if match.group (1) else match.group (3, 4)
        more = re.findall (r"(,? and |, |–| to )(\d{1,2})", match.group (5) or "")
        return linked (f"{word} {first}", int (first)) + \
            "".join (between + linked (number, int (number)) for between, number in more)

    for line in markdown.split ("\n"):
        if line.lstrip ().startswith (("```", "~~~")):
            fenced = not fenced
        if fenced or line.lstrip ().startswith (("#", "<", "|---")):
            out.append (line)
            continue
        # Leave inline code and existing links as they are.
        pieces = re.split (r"(`[^`]*`|\[[^\]]*\]\([^)]*\))", line)
        for index in range (0, len (pieces), 2):
            pieces[index] = re.sub (REFERENCE, replace, pieces[index])
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
def lesson_drawing (match):
    slug, view, letter = match.groups ()
    lesson = next ((lesson for lesson in LESSONS if lesson["slug"] == slug), None)
    boards = load_circuit (lesson or {"slug": slug})
    if (letter or "") not in boards:
        raise PluginError (f"<!-- drawing {slug} {view} --> needs one of its boards' letters")
    bench = boards[letter or ""]
    return figure (Drawing (bench).svg (view, f"{slug}-{view}{letter or ''}"), bench.title, view)


def load_circuit (lesson):
    path = os.path.join (ROOT, "docs", "lessons", lesson["slug"], "circuit.py")
    try:
        return load (path)
    except Exception as error:
        raise PluginError (f"{path}: {error}") from error


# The build a board carries on from: the same board in the lesson before,
# when that is written. After a one-board lesson Board A carries on from
# its only board, and Board B starts on a Mega of its own; a one-board
# lesson after a two-board one carries on from Board A. Returns the lesson,
# its board's letter and its bench, or None.
def previous (number, letter=""):
    if number < 2 or not written (LESSONS[number - 2]):
        return None
    lesson = LESSONS[number - 2]
    boards = load_circuit (lesson)
    for mine, theirs in ((letter, letter), ("A", ""), ("", "A")):
        if letter == mine and theirs in boards:
            return lesson, theirs, boards[theirs]
    return None


def figure (svg, caption, kind):
    return (f'<figure class="bench-figure bench-{kind}" markdown="0">\n{svg}\n'
            f'<figcaption>{caption}</figcaption>\n</figure>')


# Each measurement drawn small, side by side, numbered as the table below
# them lists them: what to measure, what to expect, where the probes go and
# when.
def measurements (bench, drawn, prefix="measure"):
    if not bench.measurements:
        return ""
    figures, rows = [], []
    for index, taken in enumerate (bench.measurements, 1):
        (_, red), (_, black) = bench.probes (index - 1)
        figures.append (
            f'<figure class="meter">\n{drawn.measure_svg (index - 1, prefix)}\n'
            f'<figcaption>{index}. {escape (taken["label"])}</figcaption>\n</figure>')
        rows.append (f"| {index}. {taken['label']} | {taken['expect']} | {red} | {black} | "
                     f"{taken['when'] or ''} |")
    table = ["| Measurement | Expect | Red probe | Black probe | When |",
             "|---|---|---|---|---|"] + rows
    return ('<div class="meters" markdown="0">\n' + "\n".join (figures) + "\n</div>\n\n" +
            "\n".join (table))


# A board's build, step by step, after what to keep from the build it
# carries on from and what to take out of it. When no part or module
# carries over, the learner takes out everything but the Mega's power
# wires, and builds the rest.
def steps (bench, before=None):
    kept, gone, new = continuity (bench, before[2]) if before else ([], [], bench.items)
    head = []
    if before:
        lesson, letter, old = before
        source = f"[Lesson {lesson['number']}](../{lesson['slug']}/index.md)" + \
            (f"'s Board {letter}" if letter else "")
        if [kind for kind, _ in kept if kind != "wire"]:
            things = kept_words (old, kept)
            head = [f"**Keep from {source}:** {join (things)}, just as "
                    f"{'they are' if len (kept) > 1 else 'it is'}.", ""]
            if len (gone) > 8:
                head += [f"**Take out** everything else from Lesson {lesson['number']}.", ""]
            elif gone:
                head += [f"**Take out:** {'; '.join (gone)}.", ""]
        else:
            power = [wire for kind, wire in kept if old.is_standard (wire)]
            words = power_words (power)
            head = [f"**Take out** everything from {source}"
                    f"{' except ' + join (words) if words else ''}.", ""]
            kept = [("wire", wire) for wire in power]
            ends = {identity (old, "wire", wire) for wire in power}
            new = [item for item in bench.items if identity (bench, item[0], item[1]) not in ends]
        head += ["**Add:**" if new else "Nothing else to add.", ""]
    lines = head + [f"{index}. {step}" for index, (_, _, step) in enumerate (new, 1)]
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
    return ("wire", frozenset (bench.end_key (end) for end in thing[:2]))


# What is kept, in words: the parts, each module with its own wires, the
# Mega's power wires, and how many other wires.
def kept_words (bench, kept):
    modules = {thing.name: thing for kind, thing in kept if kind == "module"}
    parts = [f"the {thing.name}" for kind, thing in kept if kind == "part"]
    wires = [thing for kind, thing in kept if kind == "wire"]
    power = [wire for wire in wires if bench.is_standard (wire)]
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


# The Mega's power wires and the rails' links, in words: "the Mega's GND and
# 5V wires".
def power_words (wires):
    feeds = [canonical (end[1]) for wire in wires for end in wire[:2] if end[0] == "pin"]
    feeds = [name for name in ("GND", "5V") if name in feeds]
    links = len (wires) - len (feeds)
    words = [f"the Mega's {' and '.join (feeds)} wire{'s' if len (feeds) > 1 else ''}"] \
        if feeds else []
    return words + ([f"the link{'s' if links > 1 else ''} between the rails"] if links else [])


def describe (bench, kind, thing):
    if kind == "module":
        return f"the {thing.title}"
    if kind == "part":
        holes = [unbroken (hole) for _, hole in thing.legs ()]
        return f"the {thing.name} ({holes[0]}{'–' + holes[-1] if len (holes) > 1 else ''})"
    start, end, color, _ = thing
    return f"the {color} wire from {bench.describe (start)} to {bench.describe (end)}"


# Like parts once each: "the 220 Ω resistor" three times is "three 220 Ω
# resistors", and "the tilt switch" twice "two tilt switches".
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
            name = name.removeprefix ("the ")
            out.append (f"{many} {name}{'es' if name.endswith (('s', 'x', 'ch', 'sh')) else 's'}")
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


# The note an arc of two-board projects carries on the home page and the
# course map.
def boards_note (arc):
    return '<p class="two-boards">Two boards</p>\n\n' if arc.get ("boards", 1) == 2 else ""


def arcs ():
    cards = ['<div class="arcs" markdown>']
    for index, arc in enumerate (COURSE, 1):
        note = boards_note (arc)
        cards.append (f'<section class="arc" markdown>\n\n<p class="arc-number">{index}</p>\n\n'
                      f'### {arc["arc"]}\n' + (f"\n{note}" if note else ""))
        for lesson in (lesson for lesson in LESSONS if lesson["arc"] == arc["arc"]):
            item = f"{lesson['number']}. {link (lesson)}"
            if lesson.get ("project"):
                item = f'{lesson["number"]}. <span class="project">★ {link (lesson)}</span>'
            cards.append (item)
        cards.append ("\n</section>")
    cards.append ("</div>")
    return "\n".join (cards)


def course_table ():
    rows = []
    for arc in COURSE:
        rows += [f"\n### {arc['arc']}\n", boards_note (arc) + "| | Lesson | You build |",
                 "|--:|---|---|"]
        for lesson in (lesson for lesson in LESSONS if lesson["arc"] == arc["arc"]):
            title = link (lesson)
            if lesson.get ("project"):
                title = f"★ {title}"
            rows.append (f"| {lesson['number']:02d} | {title} | {lesson['builds']} |")
    return "\n".join (rows)
