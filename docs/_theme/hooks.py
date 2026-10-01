"""Build lesson pages from their parts.

A lesson page is docs/lessons/NNN-name/index.md, with a circuit.py beside it
describing the build and a sketch in examples/. Its title and arc come from
course.yml, and its sketch from its name: examples/lessons/013-hello-lcd for
13-hello-lcd. Markers in the page are replaced when the site builds:

    <!-- bench -->         the pencil drawing of the whole bench
    <!-- closeup -->       the opening picture, including off-board screens
    <!-- steps -->         the build, step by step, in stages
    <!-- connections -->   which Mega pin reaches which part
    <!-- sketch -->        the example sketch, exactly as it compiles
    <!-- measure -->       each reading to take with a multimeter, drawn, and
                           a table of them all
    <!-- probe -->         each trace to watch on an oscilloscope, its probe
                           drawn on the board, and a table of them all

A two-board lesson's page gives each marker a board's letter, as in
<!-- bench A --> and <!-- sketch B -->; its circuit.py describes both
builds, and each board's sketch is in a folder of its own in the lesson's
example (docs/contributing.md, Two-board lessons).

Any page may also use <!-- arcs --> for the course as cards,
<!-- course --> for the course as a table, <!-- drawing 001-blink closeup -->
for a drawing from a lesson (with a board's letter after it in a two-board
lesson), and <!-- api led.h Led --> for a part's reference, read from its
header. Both tracks come from course.yml, which also builds their navigation
from the lessons that exist. The global lesson number stays in page metadata
and paths; the visible number belongs to its track.

`make pins` holds each sketch to its circuit (tests/pins.py says exactly
how): the pins it claims must be the pins the circuit wires, each claimed
as an output or an input as the parts on it need.

Where a board's steps follow its bench drawing straight away, the two
share a box. The Build along view (steps.js) pairs each instruction with
an enlarged crop and a whole-board map, using the circuit's coordinates.
The complete drawing and staged steps remain available without JavaScript
and in print.

When a lesson shares parts in the same holes, or wires between the same
points, with the lesson before it, its steps say what to keep, what to take
out and what to add, so a learner carries the build on rather than starting
again; when only the Mega's power wires carry over, they say to take out
everything else.
"""

import hashlib
import json
import os
import re
import sys
from xml.sax.saxutils import escape, quoteattr

import yaml
from mkdocs.exceptions import PluginError

sys.path.insert (0, os.path.dirname (__file__))

from api import document  # noqa: E402
from bench import canonical, example, hole_words, load  # noqa: E402
from drawing import Drawing, draw_all  # noqa: E402
import meter  # noqa: E402
from parts import Chip, Led, Resistor  # noqa: E402
from pencil import WIRES  # noqa: E402

ROOT = os.path.dirname (os.path.dirname (os.path.dirname (os.path.abspath (__file__))))

COURSE = yaml.safe_load (open (os.path.join (os.path.dirname (__file__), "course.yml"),
                               encoding="utf-8"))
LESSONS = [dict (lesson, arc=arc["arc"], boards=arc.get ("boards", 1),
                track=arc.get ("track", "projects"))
           for arc in COURSE for lesson in arc["lessons"]]
TRACKS = {"projects": ("Course", "course.md"),
          "electricity": ("Electricity", "electricity/index.md")}
ELECTRICITY_GUIDES = [{"Measurement skills": "electricity/skills.md"},
                      {"Read a schematic": "electricity/schematics.md"},
                      {"Diagnose a result": "electricity/diagnose.md"},
                      {"Design challenges": "electricity/challenges.md"}]
track_counts = {track: 0 for track in TRACKS}
for number, lesson in enumerate (LESSONS, 1):
    lesson["number"] = number
    track_counts[lesson["track"]] += 1
    lesson["track_number"] = track_counts[lesson["track"]]
ELECTRICITY_LESSONS = [lesson for lesson in LESSONS if lesson["track"] == "electricity"]
MARKERS = ("bench", "closeup", "steps", "connections", "sketch", "measure", "probe")
CIRCUITS = {}                       # each circuit.py this build has read, by path
DRAWINGS = {}                       # each drawing in a page, by its digest
DRAWN_HERE = re.compile (r"<!-- drawn ([0-9a-f]{40}) -->")
NBSP = "\u00a0"


def written (lesson):
    return os.path.exists (os.path.join (ROOT, "docs", "lessons", lesson["slug"], "index.md"))


# Each track lists its written lessons under their own visible numbers.
def on_config (config):
    for item in config["nav"]:
        if not isinstance (item, dict):
            continue
        for track, (tab, overview) in TRACKS.items ():
            if tab not in item:
                continue
            sections = []
            for arc in COURSE:
                if arc.get ("track", "projects") != track:
                    continue
                lessons = [lesson for lesson in LESSONS
                           if lesson["arc"] == arc["arc"] and lesson["track"] == track]
                pages = [{f"{visible_number (lesson)}{NBSP}·{NBSP}{lesson['title']}":
                          f"lessons/{lesson['slug']}/index.md"}
                         for lesson in lessons if written (lesson)]
                if pages:
                    sections.append ({arc_title (arc, lessons): pages})
            guides = [{"Tools and challenges": ELECTRICITY_GUIDES}] if track == "electricity" else []
            item[tab] = [overview] + guides + sections
    return config


def on_nav (nav, config, files):
    pages = {page.file.src_uri: page for page in nav.pages}
    for track, (_, overview) in TRACKS.items ():
        sequence = [pages[f"lessons/{lesson['slug']}/index.md"]
                    for lesson in LESSONS if lesson["track"] == track and written (lesson)]
        pages[overview].previous_page = None
        pages[overview].next_page = sequence[0] if sequence else None
        for index, page in enumerate (sequence):
            page.previous_page = sequence[index - 1] if index else pages[overview]
            page.next_page = sequence[index + 1] if index + 1 < len (sequence) else pages[overview]
    return nav


def visible_number (lesson):
    return f"E{lesson['track_number']:02d}" if lesson["track"] == "electricity" else \
        str (lesson["track_number"])


def visible_reference (lesson):
    return visible_number (lesson) if lesson.get ("track") == "electricity" else \
        f"Lesson {lesson['number']}"


def arc_title (arc, lessons):
    name = arc["arc"].removeprefix ("Electricity · ")
    first, last = visible_number (lessons[0]), visible_number (lessons[-1])
    title = f"{name}{NBSP}·{NBSP}{first}" + (f"–{last}" if last != first else "")
    return title + (f"{NBSP}·{NBSP}two boards" if arc.get ("boards", 1) == 2 else "")


# Inline code would break at the space before a call's parentheses, leaving
# "delay" at the end of one line and "()" at the start of the next. Each
# name is held to its opening parenthesis; code blocks are left alone, and
# the text copied from the page is unchanged.
# Drawing is nearly all of the build's time, so every lesson is drawn at
# once before the pages are built, and each page finds its drawings ready.
# Each build reads each circuit once, for its page and the next lesson's.
def on_pre_build (config):
    CIRCUITS.clear ()
    DRAWINGS.clear ()
    STAGES.clear ()
    draw_all ([os.path.join (ROOT, "docs", "lessons", lesson["slug"], "circuit.py")
               for lesson in LESSONS if written (lesson)])


# A drawing's thousands of elements would cost Markdown, and then the search
# index, far more than the rest of its page; each stands in the page as a
# comment until the page is built, and then takes its place.
def on_post_page (output, page, config):
    return DRAWN_HERE.sub (lambda match: DRAWINGS[match.group (1)], output)


def on_page_content (html, page, config, files):
    def hold (code):
        text = re.sub (r"([\w:.\]]+) \(", r'<span class="call">\1 (</span>', code.group (1))
        return f"<code>{text}</code>"

    parts = re.split (r"(<pre\b.*?</pre>)", html, flags=re.S)
    parts[::2] = [re.sub (r"<code>([^<]*)</code>", hold, part) for part in parts[::2]]
    html = "".join (parts)
    if "lesson" in page.meta:
        # Keep the promised result beside the title, before the inventory.
        # The original heading and anchor still belong to the lesson's TOC.
        opening = re.match (r"(<h2\b.*?)(?=<h2\b)", html, flags=re.S)
        if opening:
            page.meta["opening"] = opening.group (1)
            html = html[opening.end ():]
    return html


def on_page_markdown (markdown, page, config, files):
    meta = page.meta
    markdown = markdown.replace ("<!-- arcs -->", arcs ())
    markdown = markdown.replace ("<!-- course -->", course_table ())
    markdown = re.sub (r"<!-- drawing (\S+) (bench|closeup)(?: ([A-Z]))? -->", lesson_drawing,
                       markdown)
    markdown = re.sub (r"<!-- api (\S+)(?: (\w+))? -->", reference, markdown)
    markdown = link_lessons (markdown, page)
    if "lesson" not in meta:
        return words_for_rails (markdown, page.file.src_uri)

    number = meta["lesson"]
    if (not isinstance (number, int) or isinstance (number, bool) or
            not 1 <= number <= len (LESSONS)):
        raise PluginError (f"{page.file.src_uri}: invalid global lesson number {number!r}")
    lesson = LESSONS[number - 1]
    where = page.file.src_uri
    if where != f"lessons/{lesson['slug']}/index.md":
        raise PluginError (f"{where}: course.yml calls lesson {meta['lesson']} {lesson['slug']}")
    # The title, arc and sketch come from course.yml and the lesson's name;
    # a page that gives them anyway must agree.
    derived = {"title": lesson["title"], "arc": lesson["arc"],
               "track": lesson["track"], "display_id": visible_reference (lesson)}
    if lesson["boards"] == 1:
        derived["sketch"] = example (lesson["slug"])
    for key, value in meta.items ():
        if key in derived and derived[key] != value:
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
    # Where a board's steps follow its drawing straight away, the two share
    # a box, in which the drawing can stay in sight while the steps scroll.
    markdown = re.sub (r"<!-- bench((?: [A-Z])?) -->\s*<!-- steps\1 -->",
                       r'<div class="build" markdown>\n\n<!-- bench\1 -->\n\n<!-- steps\1 -->'
                       r"\n\n</div>", markdown)
    markers = re.findall (r"<!-- (" + "|".join (MARKERS) + r")(?: ([A-Z]))? -->", markdown)
    for marker, letter in markers:
        if letter not in boards:
            raise PluginError (f"{where}: <!-- {marker} {letter} --> names no board of its circuit"
                               if letter else f"{where}: a two-board page marks each board's "
                               f"{marker}, as <!-- {marker} A -->")
    if lesson["boards"] == 1:
        check_wires (lesson, boards[""], meta.get ("parts"))
    for letter, bench in boards.items ():
        replacements = board_pieces (lesson, letter, bench)
        for marker, content in replacements.items ():
            markdown = markdown.replace (f"<!-- {marker}{' ' + letter if letter else ''} -->",
                                         content)
    return words_for_rails (markdown, where)


# A rail's hole goes by its name, B-3 or T+61, only in circuit.py and in
# code: its B reads as row b, and its − as a minus. A page, its build steps
# among it, says "the bottom − rail by column 3".
RAIL_NAME = re.compile (r"(?<![\w`-])[BT][-+−]\u2060?\d{1,2}\b")


def words_for_rails (markdown, where):
    fenced = False
    for line in markdown.split ("\n"):
        if line.lstrip ().startswith (("```", "~~~")):
            fenced = not fenced
        found = None if fenced else RAIL_NAME.search (re.sub (r"`[^`]*`", "", line))
        if found:
            raise PluginError (f"{where}: say which rail in words, as \"the bottom − rail by "
                               f"column 3\", not {found.group ()}")
    return markdown


# Everything the markers show of one board.
def board_pieces (lesson, letter, bench):
    # examples/lessons/044-remote-dial/Dial/Dial.ino for a board, or
    # examples/lessons/001-blink/001-blink.ino for a lesson's only one.
    name = bench.sketch or lesson["slug"]
    folder = os.path.join (ROOT, "examples", example (lesson["slug"]), bench.sketch or "")
    path = os.path.join (folder, name + ".ino")
    try:
        sketch = open (path, encoding="utf-8").read ()
    except OSError as error:
        raise PluginError (f"{lesson['slug']}: no sketch for board {letter or 'of the lesson'} at "
                           f"{os.path.relpath (path, ROOT)}") from error
    try:
        drawing = Drawing (bench)
        whole, closeup, measured = drawing.page (letter)
        details = []
        for index, (caption, crop, columns) in enumerate (drawing.print_regions ()):
            if letter:
                caption = f"Board {letter} · {caption}"
            svg = drawing.print_svg (caption, crop, columns, f"detail{letter}{index}", "bench" + letter)
            details.append (figure (svg, caption, "detail"))
        return {
            "bench": (figure (whole, bench.title, "bench", letter) +
                      '<div class="print-details" markdown="0">' + "".join (details) + "</div>"),
            "closeup": figure (closeup,
                               f"{'Board ' + letter + ' · ' if letter else ''}{bench.title}",
                               "opening", letter),
            "steps": steps (bench, previous (lesson["number"], letter), lesson["number"], letter,
                            f"Board {letter} · {bench.sketch}" if letter else lesson["title"]),
            "connections": connections (bench),
            "sketch": f'```cpp title="{name}.ino" linenums="1"\n{sketch.rstrip ()}\n```',
            "measure": measurements (bench, measured),
            "probe": traces (bench, [meter.probe_svg (drawing, index, "probe" + letter)
                                     for index in range (len (bench.scope_probes))]),
        }
    except ValueError as error:
        raise PluginError (f"{lesson['slug']}: {error}") from error


# Global "Lesson 56" references display as E01, including the page's own
# number. Written lesson references become links. Code, headings and links
# with their own label are left alone.
REFERENCE = re.compile (r"\b(?:(Lesson) (\d{1,2})|(Lessons) (\d{1,2})"
                        r"((?:, \d{1,2})*(?:,? and \d{1,2}|–\d{1,2}| to \d{1,2})?))\b(?!\s*\])")
E_REFERENCE = re.compile (r"\bE(\d{2})\b")


def link_lessons (markdown, page):
    here = os.path.dirname (page.file.src_uri)
    own = page.meta.get ("lesson")
    out, fenced = [], False

    def linked (text, number):
        if not 1 <= number <= len (LESSONS):
            return text
        lesson = LESSONS[number - 1]
        shown = visible_number (lesson) if lesson["track"] == "electricity" else text
        if number == own or not written (lesson):
            return shown
        target = f"lessons/{lesson['slug']}/index.md"
        return f"[{shown}]({os.path.relpath (target, here or '.')})"

    def replace (match):
        word, first = match.group (1, 2) if match.group (1) else match.group (3, 4)
        more = re.findall (r"(,? and |, |–| to )(\d{1,2})", match.group (5) or "")
        return linked (f"{word} {first}", int (first)) + \
            "".join (between + linked (number, int (number)) for between, number in more)

    def replace_e (match):
        index = int (match.group (1))
        return linked (match.group (), ELECTRICITY_LESSONS[index - 1]["number"]) \
            if 1 <= index <= len (ELECTRICITY_LESSONS) else match.group ()

    for line in markdown.split ("\n"):
        if line.lstrip ().startswith (("```", "~~~")):
            fenced = not fenced
        if fenced or line.lstrip ().startswith (("#", "<", "|---")):
            out.append (line)
            continue
        # Leave inline code and existing inline or reference-style links as they are.
        pieces = re.split (r"(`[^`]*`|\[[^\]]*\](?:\([^)]*\)|\[[^\]]*\])|\[E\d{2}\])", line)
        for index in range (0, len (pieces), 2):
            pieces[index] = re.sub (E_REFERENCE, replace_e, pieces[index])
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
    if path not in CIRCUITS:
        try:
            CIRCUITS[path] = load (path)
        except Exception as error:
            raise PluginError (f"{path}: {error}") from error
    return CIRCUITS[path]


# The build a board carries on from: the same board in the lesson before,
# when that is written. After a one-board lesson Board A carries on from
# its only board, and Board B starts on a Mega of its own; a one-board
# lesson after a two-board one carries on from Board A. Returns the lesson,
# its board's letter and its bench, or None.
def previous (number, letter=""):
    if number < 2 or LESSONS[number - 1].get ("fresh_start") or \
            not written (LESSONS[number - 2]):
        return None
    lesson = LESSONS[number - 2]
    if lesson["track"] != LESSONS[number - 1]["track"]:
        return None
    boards = load_circuit (lesson)
    for mine, theirs in ((letter, letter), ("A", ""), ("", "A")):
        if letter == mine and theirs in boards:
            return lesson, theirs, boards[theirs]
    return None


# The drawing's own width, in drawing units, lets the page size it so its
# labels stay legible when enlarged; the opening and overview fit a phone.
# A lesson's own drawings name their board, so its steps can light up the
# part each one adds.
def figure (svg, caption, kind, board=None):
    width = re.search (r'viewBox="\S+ \S+ (\S+)', svg).group (1)
    whose = f' data-board="{board}"' if board is not None else ""
    return (f'<figure class="bench-figure bench-{kind}" style="--drawing-width: {width}"{whose} '
            f'markdown="0">\n{held (svg)}\n<figcaption>{caption}</figcaption>\n</figure>')


# The comment a drawing stands as until its page is built (on_post_page).
def held (svg):
    digest = hashlib.sha1 (svg.encode ()).hexdigest ()
    DRAWINGS[digest] = svg
    return f"<!-- drawn {digest} -->"


# Each measurement drawn small, side by side, numbered as the table below
# them lists them: what to measure, what to expect, where the probes go and
# when.
def measurements (bench, drawn):
    if not bench.measurements:
        return ""
    figures, rows = [], []
    for index, (taken, svg) in enumerate (zip (bench.measurements, drawn), 1):
        (_, red), (_, black) = bench.probes (index - 1)
        figures.append (
            f'<figure class="meter">\n{held (svg)}\n'
            f'<figcaption>{index}. {escape (taken["label"])}</figcaption>\n</figure>')
        rows.append (f"| {index}. {taken['label']} | {taken['expect']} | {red} | {black} | "
                     f"{taken['when'] or ''} |")
    table = ["| Measurement | Expect | Red probe | Black probe | When |",
             "|---|---|---|---|---|"] + rows
    return ('<div class="meters" markdown="0">\n' + "\n".join (figures) + "\n</div>\n\n" +
            "\n".join (table))


# Each trace to watch on an oscilloscope, drawn small with its probe, and
# numbered as the table below lists them: the channel, where the probe's
# tip and its ground clip go, what to expect and when.
def traces (bench, drawn):
    if not bench.scope_probes:
        return ""
    figures, rows = [], []
    for index, (taken, svg) in enumerate (zip (bench.scope_probes, drawn), 1):
        (_, tip), (_, ground) = bench.probe_points (index - 1)
        figures.append (
            f'<figure class="meter">\n{held (svg)}\n'
            f'<figcaption>{index}. {escape (taken["label"])}</figcaption>\n</figure>')
        rows.append (f"| {index}. {taken['label']} | CH{taken['channel']} | {tip} | {ground} | "
                     f"{taken['expect'] or ''} | {taken['when'] or ''} |")
    table = ["| Trace | Channel | Probe tip | Ground clip | Expect | When |",
             "|---|---|---|---|---|---|"] + rows
    return ('<div class="meters" markdown="0">\n' + "\n".join (figures) + "\n</div>\n\n" +
            "\n".join (table))


# A board's build, step by step, after what to keep from the build it
# carries on from and what to take out of it. When no part or module
# carries over, the learner takes out everything but the Mega's power
# wires, and builds the rest.
def steps (bench, before=None, number=1, letter="", title=None):
    kept, gone, new = continuity (bench, before[2]) if before else ([], [], bench.items)
    head, out = [], []
    if before:
        lesson, letter_before, old = before
        source = f"[{visible_reference (lesson)}](../{lesson['slug']}/index.md)" + \
            (f"'s Board {letter_before}" if letter_before else "")
        if carries (kept):
            # Where anything is taken out, each wire kept is named, since a
            # count can't say which. A chip swapped for another in its holes
            # would meet any wire left on its old pins, so then every one
            # taken out is listed too, however many.
            swapped = any (isinstance (thing, Chip) for kind, thing, _ in gone)
            things = kept_words (old, kept, named=bool (gone))
            still = f"just as {'they are' if len (kept) > 1 else 'it is'}"
            # A few things read as a sentence; more, or any that list what
            # they hold, as a list.
            if len (things) <= 3 and not any (", " in thing for thing in things):
                head = [f"**Keep from {source}:** {join (things)}, {still}."]
            else:
                head = [f"**Keep from {source}**, {still}:",
                        "\n".join (f"- {thing}" for thing in things)]
            if len (gone) > 8 and not swapped:
                head.append (f"**Take out** everything else from {visible_reference (lesson)}.")
            else:
                out = gone
        else:
            power = [wire for kind, wire in kept if old.is_standard (wire)]
            words = power_words (power)
            head = [f"**Take out** everything from {source}"
                    f"{' except ' + join (words) if words else ''}."]
            new = added (bench, before)
        if not new:
            head.append ("Nothing else to add.")
    context = ('<div class="build-context" markdown>\n\n' + "\n\n".join (head) + "\n\n</div>") \
        if head else ""
    return context + ("\n\n" + step_stages (bench, out, new, number, letter, title)
                      if out or new else "")


# The steps in stages, each a part and its wires under a heading that names
# them and the pins they use, so a build reads as a few things to add
# rather than a long list. Each stage is a small table with the same
# columns as the others, so the holes and pins line up all the way down. A
# stage built just this way in an earlier lesson starts folded, pointing
# back to it; everything taken out comes first.
def step_stages (bench, out, new, number, letter, title=None):
    blocks = []
    if out:
        rows = [step_row (f"t{index}", "−", item, label=f"Take-out step {index} done")
                for index, item in enumerate (out, 1)]
        blocks.append (stage_block ("Take out", rows, kind="out"))
    count = 0
    for stage, items in group (new):
        rows = []
        for item in items:
            count += 1
            rows.append (step_row (str (count), str (count), item, bench))
        whole = {identity (bench, kind, thing) for kind, thing, _ in items} == stages (bench)[stage]
        earlier = built_before (bench, stage, number) if whole and \
            not LESSONS[number - 1].get ("fresh_start") else None
        where = (f', as in <a href="../{earlier["slug"]}/">{visible_reference (earlier)}</a>'
                 if earlier else "")
        blocks.append (stage_block (capital (bench.stage_title (stage)), rows,
                                    f"{len (rows)} step{'s' if len (rows) > 1 else ''}{where}",
                                    "again" if earlier else ""))
    content = "\n".join (blocks)
    revision = hashlib.sha256 (content.encode ()).hexdigest ()[:16]
    title = title or (f"Board {letter} · {bench.sketch}" if letter else bench.title)
    return (f'<div class="build-steps" data-board="{letter}" data-title={quoteattr (title)} '
            f'data-revision="{revision}">\n' + content +
            "\n</div>")


def stage_block (title, rows, count="", kind=""):
    tally = f' <span class="stage-count">{count}</span>' if count else ""
    folded = "" if kind == "again" else " open"
    return (f'<details class="stage{" " + kind if kind else ""}"{folded}><summary>'
            f'<span class="stage-title">{title}</span>{tally}</summary>\n'
            f'<table class="steps"><caption class="step-headings">{title}: build steps</caption>\n'
            '<colgroup><col class="step"><col class="what"><col>'
            '<col class="link"><col></colgroup>\n'
            '<thead class="step-headings"><tr><th scope="col">Step</th>'
            '<th scope="col">Part or wire</th><th scope="col">From</th>'
            '<th scope="col">Connection</th><th scope="col">To</th></tr></thead>\n'
            '<tbody>\n' + "\n".join (rows) +
            "\n</tbody></table></details>")


# One step: its tick, what it is, and where it goes, with the thing itself
# drawn between its From and To. A step of this bench lights up its part in
# the drawings (data-item, as drawing.py tags them); one taken out of the
# build before can't.
def step_row (key, mark, item, bench=None, label=None):
    kind, thing, step = item
    lit = f' data-item="{bench.items.index (item)}"' if bench else ""
    points = step_points (bench, item) if bench else []
    labels = step_point_labels (bench, item, points) if bench else []
    action = step_action (item, bench)
    # Saved progress must follow the actual connection, not its row number.
    # Include physical placement: moving a module can leave its sentence
    # ("below the breadboard") unchanged while every wire must be checked again.
    physical = (thing.x, thing.y, thing.angle) if kind == "module" else \
        thing[:3] if kind == "wire" else thing.legs ()
    signature = (kind, physical, step.what, step.places, step.note, step.colors, bool (bench))
    key = hashlib.sha256 (json.dumps (signature, ensure_ascii=False).encode ()).hexdigest ()[:16]
    metadata = (f' data-kind="{kind}" data-action={quoteattr (action)}'
                f' data-points={quoteattr (json.dumps (points))}'
                f' data-point-labels={quoteattr (json.dumps (labels, ensure_ascii=False))}')
    if step.what == "LCD" and bench:
        care = ("Support the overhanging screen at breadboard height, for example on the kit's "
                "box, so it cannot pull its pins out.")
        metadata += f' data-care={quoteattr (care)}'
    places = [f'<span class="place">{escape (where)}{detail (how)}</span>'
              for where, how in step.places]
    if len (places) == 2 and not step.spread:
        ends = (f'<td class="from">{places[0]}</td>'
                f'<td class="link">{picture (kind, thing, step)}</td><td>{places[1]}</td>')
    else:
        ends = f'<td colspan="3"><div class="places">{"".join (places)}</div></td>'
    return (f'<tr data-step="{key}"{lit}{metadata}><td class="step">'
            '<button type="button" class="tick" '
            f'aria-pressed="false" aria-label="{label or f"Step {mark} done"}">{mark}</button></td>'
            f'<td class="what">{capital (step.what)}{detail (step.note)}</td>{ends}</tr>')


# Coordinates come from the same model as the drawing, including the exact
# ground pin and the turned header of a module. Keep them in the order the
# endpoint labels use, which need not be the part's electrical leg order.
# Chips name a whole row of pins at once; highlight every leg in that case.
def step_points (bench, item):
    kind, thing, step = item
    if kind == "module":
        return []
    if kind == "wire":
        points = [bench.xy (end) for end in thing[:2]]
        if step.what == "lead" and bench.style (thing[1]) == "lead":
            points.reverse ()
    else:
        points = []
        for where, detail in step.places:
            for _, hole in thing.legs ():
                place, column = bench.place (("hole", hole))
                if place == where and detail.startswith (column):
                    points.append (bench.hole_xy (hole))
                    break
            else:
                points = [bench.hole_xy (hole) for _, hole in thing.legs ()]
                break
    return [[round (x, 2), round (y, 2)] for x, y in points]


# Crops can exclude the board's printed row letters. Name the actual holes
# at their rings, from the same endpoints that supply the coordinates.
def step_point_labels (bench, item, points):
    kind, thing, _ = item
    ends = thing[:2] if kind == "wire" else \
        [("hole", hole) for _, hole in thing.legs ()] if kind == "part" else []
    labels = []
    for point in points:
        end = next ((end for end in ends if [round (v, 2) for v in bench.xy (end)] == point), None)
        if end is None:
            labels.append ("")
        elif end[0] == "hole" and end[1][0] in "BT":
            labels.append (("top " if end[1][0] == "T" else "bottom ") +
                           end[1][1:].replace ("-", "−"))
        elif end[0] == "hole":
            labels.append (end[1])
        elif end[0] == "pin":
            labels.append (canonical (end[1]))
        else:
            labels.append (bench.module_pin (end[1])[1].name)
    return labels


# A short instruction for the guided view. The table below it remains the
# complete list of legs, colors and placement notes, for reading and print.
def step_action (item, bench=None):
    kind, thing, step = item
    if bench is None:
        places = join ([where + (f" ({how})" if how else "") for where, how in step.places])
        if kind == "wire":
            return f"Disconnect the {step.colors[0]} {step.what} between {places}."
        return f"Take out the {step.what} from {places}."
    if kind == "wire":
        start, end = thing[:2]
        if step.what == "lead" and bench.style (end) == "lead":
            start, end = end, start
        if step.what == "lead":
            return f"Connect {bench.describe (start)} to {bench.describe (end)}."
        color = step.colors[0]
        return f"Connect {bench.describe (start)} to {bench.describe (end)} with the {color} " \
            f"{step.what}."
    if kind == "module":
        return f"Place the {step.what} {step.places[0][0]}."
    holes = [hole_words (hole) for _, hole in thing.legs ()]
    if step.what == "LCD":
        return f"Put the LCD's sixteen pins in {holes[0]} through {holes[-1]}. " \
            "Its screen faces you and hangs past the right end of the breadboard."
    return f"Place the {step.what} in {join ([where for where, _ in step.places])}."


# The steps grouped by stage, in order: (stage, items).
def group (items):
    groups = []
    for item in items:
        if groups and groups[-1][0] is item[2].stage:
            groups[-1][1].append (item)
        else:
            groups.append ((item[2].stage, [item]))
    return groups


# A bench's stages, each with the identities of all its items, which say
# whether another lesson built it just the same way.
STAGES = {}


def stages (bench):
    if id (bench) not in STAGES:
        found = {}
        for kind, thing, step in bench.items:
            found.setdefault (step.stage, set ()).add (identity (bench, kind, thing))
        STAGES[id (bench)] = {stage: frozenset (held) for stage, held in found.items ()}
    return STAGES[id (bench)]


# The first lesson in the same track to build a stage just as this one does,
# on either of its boards, or None.
def built_before (bench, stage, number):
    held = stages (bench)[stage]
    track = LESSONS[number - 1]["track"]
    for lesson in LESSONS[:number - 1]:
        if lesson["track"] == track and written (lesson) and \
                any (held in stages (other).values ()
                                     for other in load_circuit (lesson).values ()):
            return lesson
    return None


def detail (text):
    return f"<small>{escape (text)}</small>" if text else ""


def capital (text):
    return escape (text[:1].upper () + text[1:])


# The thing a step puts in, drawn between the holes it joins: a wire in its
# color, a resistor with its bands, an LED lit in its color, or a part's
# two legs. The color's name goes beneath, for anyone who can't tell them
# apart.
PALETTE = dict (Resistor.TINTS, **WIRES)


def picture (kind, thing, step):
    names = f"<small>{escape (', '.join (step.colors))}</small>" if step.colors else ""
    if kind == "wire":
        lead = " lead" if step.what == "lead" else ""
        color = PALETTE[step.colors[0]]
        return f'<span class="wire{lead}" style="--swatch: {color}"></span>{names}'
    if isinstance (thing, Resistor):
        bands = "".join (f'<span style="--swatch: {PALETTE[color]}"></span>'
                         for color in step.colors)
        return f'<span class="resistor"><span class="bands">{bands}</span></span>{names}'
    if isinstance (thing, Led):
        return f'<span class="led" style="--swatch: {PALETTE[step.colors[0]]}"></span>{names}'
    return '<span class="legs"></span>'


# Whether a build carries on from the one before: some part or module stays
# where it was. Otherwise only the Mega's power wires stay.
def carries (kept):
    return any (kind != "wire" for kind, _ in kept)


# The items a board's steps add to the build it carries on from.
def added (bench, before=None):
    if not before:
        return bench.items
    kept, _, new = continuity (bench, before[2])
    if carries (kept):
        return new
    old = before[2]
    ends = {identity (old, "wire", wire) for kind, wire in kept if old.is_standard (wire)}
    return [item for item in bench.items if identity (bench, item[0], item[1]) not in ends]


# How many jumper wires some items take, of each kind: a jumper between two
# holes or pins, a female-to-male wire to a module's pin; a part's own leads
# take none.
def wire_count (bench, items):
    counts = {"jumper": 0, "female-to-male": 0, "female-to-female": 0}
    for kind, thing, _ in items:
        ends = thing[:2] if kind == "wire" else ()
        if not ends or "lead" in (bench.style (end) for end in ends):
            continue
        males = sum (bench.style (end) == "male" for end in ends)
        counts[("jumper", "female-to-male", "female-to-female")[males]] += 1
    return counts


# A one-board lesson's parts list counts its jumper wires, of each kind the
# build needs: all its build holds, or, saying "more", the ones its steps
# add where it carries on ("5 more jumper wires"). A count that is
# neither, or a kind left out, is an error. (Two-board lessons count their boards in too many
# ways to check.)
def check_wires (lesson, bench, parts):
    listed, more = {}, False
    for item in parts or []:
        more |= bool (re.search (r"\bmore\b", item))
        for number, kind in re.findall (r"\b(\d+) (?:more )?(female-to-male |female-to-female |)"
                                        r"(?:and \d+ )?(?:more )?(?:jumper )?wires?", item):
            kind = kind.strip () or "jumper"
            listed[kind] = listed.get (kind, 0) + int (number)
        for number in re.findall (r"\b\d+ female-to-male and (\d+) (?:jumper )?wires?", item):
            listed["jumper"] = listed.get ("jumper", 0) + int (number)
    if not listed:
        return
    new = wire_count (bench, added (bench, previous (lesson["number"])))
    whole = wire_count (bench, bench.items)
    for kind in new:
        count = listed.get (kind, 0)
        if count == whole[kind] or count == new[kind] and (more or not new[kind]):
            continue
        raise PluginError (f"{lesson['slug']}: its parts list gives {count} {kind} wires, but its "
                           f"build holds {whole[kind]}, and its steps add {new[kind]} (\"{new[kind]} "
                           f"more {'female-to-male ' if kind != 'jumper' else ''}jumper wires\")")


# What this bench keeps from the one before, and the items it takes out
# and adds: parts and modules that stand where they stood, and wires
# between the same two points, are kept.
def continuity (bench, before):
    here = {identity (bench, kind, thing) for kind, thing, _ in bench.items}
    there = {identity (before, kind, thing): (kind, thing) for kind, thing, _ in before.items}
    kept = [there[key] for key in there if key in here]
    gone = [item for item in before.items if identity (before, item[0], item[1]) not in here]
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


# What is kept, in words. A stage kept whole goes by its heading in the
# lesson before, with what else it holds: "the red LED on pin 26 with its
# 220 Ω resistor and 2 wires". Then the other parts, each module with its
# own wires, the Mega's power wires, and the other wires: each named, where
# named asks for it, or else counted. A part that leaves others of its kind
# behind is named by its holes.
def kept_words (bench, kept, named=False):
    held = {id (thing) for _, thing in kept}
    whole = []
    for stage, items in group (bench.items):
        things = [thing for _, thing, _ in items]
        modules = [thing for kind, thing, _ in items if kind == "module"]
        # A module and its own wires already read as one, as below.
        alone = len (modules) == 1 and all (kind != "part" for kind, _, _ in items)
        if all (id (thing) in held for thing in things) and not alone and \
                any (kind != "wire" for kind, _, _ in items):
            whole.append ((stage, items))
    inside = {id (thing) for _, items in whole for _, thing, _ in items}
    kept = [(kind, thing) for kind, thing in kept if id (thing) not in inside]
    modules = {thing.name: thing for kind, thing in kept if kind == "module"}
    kept_parts = [thing for kind, thing in kept if kind == "part"]
    everywhere = [part.name for part in bench.parts]
    staying = [part.name for part in kept_parts]
    staying += [thing.name for _, items in whole for kind, thing, _ in items if kind == "part"]
    parts = [describe (bench, "part", part) if everywhere.count (part.name) > staying.count (part.name)
             else f"the {part.name}" for part in kept_parts]
    wires = [thing for kind, thing in kept if kind == "wire"]
    power = [wire for wire in wires if bench.is_standard (wire)]
    counts = {}
    rest = []
    for wire in wires:
        if wire in power:
            continue
        owners = [end[1].partition (".")[0] for end in wire[:2] if end[0] == "module"]
        if owners and owners[0] in modules:
            counts[owners[0]] = counts.get (owners[0], 0) + 1
        else:
            rest.append (wire)
    things = [stage_words (bench, stage, items) for stage, items in whole]
    things += gather (parts)
    for key, placed in modules.items ():
        count = counts.get (key, 0)
        things.append (f"the {placed.title}" + (f" with its {count} wire{'s' if count > 1 else ''}"
                                                 if count else ""))
    feeds = [canonical (end[1]) for wire in power for end in wire[:2] if end[0] == "pin"]
    if feeds:
        things.append (f"the {' and '.join (sorted (feeds))} wire{'s' if len (feeds) > 1 else ''} "
                       f"from the Mega")
    if len (power) > len (feeds):
        links = len (power) - len (feeds)
        things.append (f"the link{'s' if links > 1 else ''} between the rails")
    if rest and named:
        things += [describe (bench, "wire", wire) for wire in rest]
    elif rest:
        things.append (f"{len (rest)} {'other ' if things else ''}wire{'s' if len (rest) > 1 else ''}")
    return things


# A stage kept whole, by its heading in the lesson that built it and what
# else it holds: "the screen on pins 31–36, with its potentiometer, LCD,
# 220 Ω resistor and 16 wires". A part the heading names goes unsaid.
def stage_words (bench, stage, items):
    title = bench.stage_title (stage)
    names = [thing.name for kind, thing, _ in items if kind == "part"]
    names += [thing.title for kind, thing, _ in items if kind == "module"]
    kinds = [name.split (" at column")[0] for name in names]
    names = [name for name, kind in zip (names, kinds) if not re.search (
        rf"\b({re.escape (kind)}|{re.escape (kind.split ()[-1])}s?)\b", title)]
    count = sum (kind == "wire" for kind, _, _ in items)
    held = gather (names) + ([f"{count} wire{'s' if count > 1 else ''}"] if count else [])
    if not held:
        return title
    whose = "their" if " and its " in title else "its"
    return f"{title}{',' if len (held) > 1 or whose == 'their' else ''} with {whose} {join (held)}"


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
    if kind == "part":
        # Its first and last holes: "(b6–b7)", or, where one is a rail's,
        # "(a46 to the bottom − rail by column 46)".
        legs = thing.legs ()
        ends = [hole_words (hole) for _, hole in (legs[0], legs[-1])]
        if len (legs) == 1:
            return f"the {thing.name} ({ends[0]})"
        between = "–" if all (" " not in end for end in ends) else " to "
        return f"the {thing.name} ({ends[0]}{between}{ends[1]})"
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
    project_arcs = [arc for arc in COURSE if arc.get ("track", "projects") == "projects"]
    for index, arc in enumerate (project_arcs, 1):
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
        if arc.get ("track", "projects") != "projects":
            continue
        rows += [f"\n### {arc['arc']}\n", boards_note (arc) + "| | Lesson | You build |",
                 "|--:|---|---|"]
        for lesson in (lesson for lesson in LESSONS if lesson["arc"] == arc["arc"]):
            title = link (lesson)
            if lesson.get ("project"):
                title = f"★ {title}"
            rows.append (f"| {lesson['number']:02d} | {title} | {lesson['builds']} |")
    return "\n".join (rows)
