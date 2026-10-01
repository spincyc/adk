"""Check that public lesson IDs and navigation stay within their tracks.

Run with build/venv/bin/python tests/navigation_ids.py.
"""

import sys
from pathlib import Path
from types import SimpleNamespace

import yaml

ROOT = Path (__file__).resolve ().parents[1]
# Nothing may be written outside build/, so the theme's modules leave no
# byte-code in docs/_theme.
sys.dont_write_bytecode = True
sys.path.insert (0, str (ROOT / "docs" / "_theme"))

from hooks import (LESSONS, NBSP, arc_title, arcs, check_wires, course_table,  # noqa: E402
                   link_lessons, load_circuit, on_config, on_nav, previous,
                   stage_block, steps, visible_reference)


def nav_paths (items):
    if isinstance (items, str):
        yield items
        return
    for item in items:
        if isinstance (item, str):
            yield item
        elif isinstance (item, dict):
            for children in item.values ():
                yield from nav_paths (children)


assert len (LESSONS) == 79

# An arc of one lesson is named by that lesson alone, not "55–55".
assert arc_title ({"arc": "Extras", "boards": 2}, [LESSONS[54]]) == \
    f"Extras{NBSP}·{NBSP}55{NBSP}·{NBSP}two boards"
assert arc_title ({"arc": "First light"}, LESSONS[0:3]) == f"First light{NBSP}·{NBSP}1–3"

# An admonition's text goes on the indented lines below its title; text on
# the title line makes the page show the raw "!!! warning" instead of a box.
import re  # noqa: E402
for page in (ROOT / "docs").rglob ("*.md"):
    for number, line in enumerate (page.read_text ().splitlines (), 1):
        assert not re.match (r'(!!!|\?\?\?\+?) \w+(?: "[^"]*")? +[^"\s]', line), f"{page}:{number}: {line}"

# In course.yml's one-line entries a comma ends a value unless the value is
# quoted: "builds: A game, with sound" would make a key named "with sound".
for arc in yaml.safe_load ((ROOT / "docs" / "_theme" / "course.yml").read_text ()):
    for lesson in arc["lessons"]:
        assert set (lesson) <= {"slug", "title", "builds", "project", "fresh_start"}, lesson
assert [(lesson["number"], visible_reference (lesson)) for lesson in
        (LESSONS[0], LESSONS[54], LESSONS[55], LESSONS[-1])] == [
            (1, "Lesson 1"), (55, "Lesson 55"), (56, "E01"), (79, "E24")]

config = on_config ({"nav": [{"Course": ["course.md"]},
                             {"Electricity": ["electricity/index.md"]}]})
course = list (nav_paths (config["nav"][0]["Course"]))
electricity = list (nav_paths (config["nav"][1]["Electricity"]))
electricity_lessons = [path for path in electricity if path.startswith ("lessons/")]
assert len (course) == 56 and len (electricity) == 29
assert course[0] == "course.md" and electricity[0] == "electricity/index.md"
assert course[-1] == "lessons/055-reliability-meter/index.md"
assert electricity_lessons[0] == "lessons/056-close-the-loop/index.md"
assert electricity_lessons[-1] == "lessons/079-serial-link/index.md"
assert set (electricity[1:5]) == {
    "electricity/skills.md", "electricity/schematics.md",
    "electricity/diagnose.md", "electricity/challenges.md"}
assert "E01" in str (config["nav"][1]) and "Lesson 56" not in str (config["nav"][1])
assert "E01" not in arcs () and "Electricity" not in course_table ()

pages = [SimpleNamespace (file=SimpleNamespace (src_uri=path),
                         previous_page=None, next_page=None)
         for path in course + electricity]
by_path = {page.file.src_uri: page for page in pages}
on_nav (SimpleNamespace (pages=pages), config, None)
assert by_path["course.md"].previous_page is None
assert by_path["course.md"].next_page is by_path[course[1]]
assert by_path["electricity/index.md"].previous_page is None
assert by_path["electricity/index.md"].next_page is by_path[electricity_lessons[0]]
assert by_path[course[1]].previous_page is by_path["course.md"]
assert by_path[course[-1]].next_page is by_path["course.md"]
assert by_path[electricity_lessons[0]].previous_page is by_path["electricity/index.md"]
assert by_path[electricity_lessons[-1]].next_page is by_path["electricity/index.md"]
assert by_path[electricity_lessons[1]].previous_page is by_path[electricity_lessons[0]]

page = SimpleNamespace (
    file=SimpleNamespace (src_uri="lessons/057-measure-across-and-through/index.md"),
    meta={"lesson": 57})
source = ("E01 and Lesson 58; Lesson 57 is here. [E03](other.md) and `E04`. "
          "[E16 amplifier][amp-guide] and [E17] stay intact.")
linked = link_lessons (source, page)
assert "[E01](../056-close-the-loop/index.md)" in linked
assert "[E03](../058-resist-the-flow/index.md)" in linked
assert "E02 is here" in linked
assert "[E03](other.md)" in linked and "`E04`" in linked
assert "[E16 amplifier][amp-guide]" in linked and "[E17]" in linked
assert "[[" not in linked

assert previous (67) is None  # E12 is a standalone entry after optional E11.
assert previous (68) is None  # E13 starts the separate scope/generator branch.
assert previous (74) is None  # E19 is a standalone entry after the scope/generator modules.
assert previous (77) is None  # E22 re-enters the core path after optional logic.
assert previous (79) is None  # E24 can follow E22 without E23's filter.
for number in (67, 68, 74, 77, 79):
    lesson = LESSONS[number - 1]
    page_path = ROOT / "docs" / "lessons" / lesson["slug"] / "index.md"
    meta = yaml.safe_load (page_path.read_text (encoding="utf-8").split ("---", 2)[1])
    bench = load_circuit (lesson)[""]
    check_wires (lesson, bench, meta["parts"])
    full_build = steps (bench, previous (number), number)
    assert full_build.count ('<tr data-step=') == len (bench.items)
    assert 'class="stage again"' not in full_build

table = stage_block ("the LED", ["<tr><td>one</td></tr>"])
assert '<caption class="step-headings">the LED: build steps</caption>' in table
assert table.index ("<caption") < table.index ("<colgroup") < table.index ("<thead")
assert all (f'<th scope="col">{heading}</th>' in table for heading in
            ("Step", "Part or wire", "From", "Connection", "To"))

print ("Navigation, public lesson IDs, references and stage headings pass")
