"""Check the guided build's link between instructions and drawing geometry.

Run with the site's Python: build/venv/bin/python tests/build_steps.py.
These checks need no browser, drawings cache, or generated lesson pages.
"""

import copy
import json
import re
import sys
from html.parser import HTMLParser
from pathlib import Path

ROOT = Path (__file__).resolve ().parents[1]
# Nothing may be written outside build/, so the theme's modules leave no
# byte-code in docs/_theme.
sys.dont_write_bytecode = True
sys.path.insert (0, str (ROOT / "docs" / "_theme"))

from bench import Bench, identity, load  # noqa: E402
from hooks import (LESSONS, board_caption, carries, continuity, load_all,  # noqa: E402
                   load_circuit, previous, recolored, step_points, step_row, step_stages, steps,
                   written)
from drawing import Drawing, follows, shown  # noqa: E402


class Tags (HTMLParser):
    def __init__ (self, html):
        super ().__init__ ()
        self.tags = []
        self.feed (html)

    def handle_starttag (self, tag, attrs):
        self.tags.append ((tag, dict (attrs)))

    def first (self, name):
        return next (attrs for tag, attrs in self.tags if tag == name)


def row (bench, item, ordinal="1"):
    return Tags (step_row (ordinal, ordinal, item, bench)).first ("tr")


# Replay the items identified by generated instructions, then compare their
# electrical nets. A complete option explicitly clears the actual earlier
# circuit; it must restore power feeds and every kept part's wires too.
def rebuilt (target, html, actual=None):
    tags = Tags (html)
    block = next (attrs for tag, attrs in tags.tags
                  if tag == "div" and attrs.get ("class") == "build-steps")
    assert actual is None or block.get ("data-start") == "empty"
    if actual is not None:
        assert "Remove every part and wire" in html
        assert "including the Mega's power wires and the rail" in html
    items = []
    for tag, attrs in tags.tags:
        if tag != "tr" or "data-step" not in attrs:
            continue
        assert "data-item" in attrs, "a complete build must not remove an assumed old item"
        items.append (target.items[int (attrs["data-item"])])
    return assembled (target, items)


def assembled (target, items):
    made = copy.copy (target)
    made.parts, made.wires, made.modules, made.items = [], [], {}, []
    for item in items:
        kind, thing, _ = item
        made.items.append (item)
        if kind == "part":
            made.parts.append (thing)
        elif kind == "wire":
            made.wires.append (thing)
        else:
            made.modules[thing.name] = thing
    return made


def continued (target, actual, html):
    items = list (actual.items)
    for tag, attrs in Tags (html).tags:
        if tag != "tr" or "data-step" not in attrs:
            continue
        if "data-item" in attrs:
            items.append (target.items[int (attrs["data-item"])])
        else:
            taken = next (item for item in items if Tags (step_row ("", "", item))
                          .first ("tr")["data-step"] == attrs["data-step"])
            items.remove (taken)
    return assembled (target, items)


def same_nets (a, b):
    return {frozenset (net) for net in a.nets ()} == {frozenset (net) for net in b.nets ()}


def empty_steps (lesson, letter, bench):
    html = steps (bench, previous (lesson["number"], letter), lesson["number"], letter)
    if previous (lesson["number"], letter):
        html = html[html.index ('<details class="complete-build"'):]
        assert 'data-build="complete" data-start="empty"' in html
    if previous (lesson["number"], letter) or lesson.get ("fresh_start"):
        assert 'class="stage again"' not in html, lesson["slug"]
    return html


# A route must visit waypoints in order; the same set in reverse is not a
# match. Adjacent duplicate points can arise at a wire's terminal.
assert follows ([(0, 0), (1, 0), (2, 0)], [(0, 0), (0, 0), (2, 0)])
assert not follows ([(0, 0), (1, 0), (2, 0)], [(2, 0), (1, 0)])


# A rail's minus and plus holes are ten drawing units apart. The guided
# view must ring the real minus hole, not just a nearby spot on the board.
screen = Bench ("Screen", columns=(1, 63)).screen ()
wire = next (item for item in screen.items if item[0] == "wire" and
             item[1][:2] == (("hole", "a43"), ("hole", "B-43")))
attrs = row (screen, wire)
assert json.loads (attrs["data-points"]) == [[960, 220], [960, 250]]
assert "bottom − rail by column 43" in attrs["data-action"]
assert "B-43" not in attrs["data-action"]
assert json.loads (attrs["data-point-labels"]) == ["a43", "bottom −43"]

# All sixteen LCD legs are marked at their actual holes. Its instruction
# also carries the support reminder when the article's warning is away.
lcd = next (item for item in screen.items if item[2].what == "LCD")
attrs = row (screen, lcd)
points = json.loads (attrs["data-points"])
assert len (points) == 16
assert points[0] == [1000, 220] and points[-1] == [1150, 220]
assert "a47 through a62" in attrs["data-action"]
assert "breadboard height" in attrs["data-care"]
assert json.loads (attrs["data-point-labels"])[::15] == ["a47", "a62"]

# An opening must include an off-board screen, the visible result, however
# far it stands from the breadboard. Printed details must remain large
# enough to read, and cover every occupied breadboard column.
matrix = Bench ("Matrix", columns=(1, 63)).button (2)
matrix.module ("matrix", at=(6, 4), facing="up")
drawing = Drawing (matrix)
left, top, width, height = drawing.opening_box ()
for module in matrix.modules.values ():
    x0, y0, x1, y1 = module.reach_box ()
    assert left <= x0 < x1 <= left + width
    assert top <= y0 < y1 <= top + height
# Without a screen, it frames the parts on the board and the modules the
# Mega drives or reads, not an instrument standing away from the board.
lab = Bench ("Lab", columns=(1, 14))
lab.module ("sensor", name="wave", at=(1, 4), pins=["OUT", "GND"], label="isolated generator")
lab.module ("sensor", name="tap", at=(3, 4), label="tap sensor")
lab.wire ("wave.OUT", "j6").inductor ("100 mH", "g6", "e6").wire ("a6", "B-6")
lab.wire ("22", "tap.S")
left, top, width, height = box = Drawing (lab).opening_box ()
assert lab.modules["wave"].reach_box ()[2] < left
x0, y0, x1, y1 = lab.modules["tap"].reach_box ()
assert left <= x0 < x1 <= left + width and top <= y0 < y1 <= top + height
# It names every part it frames: the coil a lesson is about keeps its name,
# however far up its leader the name stands.
names = {label.text for part in lab.parts for label in part.labels (lab)}
assert all (shown (box, label) for label in Drawing (lab)._bench_labels () if label[0] in names)
# Beside a screen, it frames what moves, such as a servo's arm, which is
# the visible result as well.
needle = Bench ("Needle", columns=(1, 63)).screen ().module ("servo", at=(6.0, 5.5))
left, top, width, height = Drawing (needle).opening_box ()
x0, y0, x1, y1 = needle.modules["servo"].reach_box ()
assert left <= x0 < x1 <= left + width and top <= y0 < y1 <= top + height
regions = Drawing (screen).print_regions ()
assert all (crop[2] <= 430 for _, crop, columns in regions if columns)
# A printed detail shows each of the bench's names whole, or not at all.
for _, (x, y, width, height), _ in regions:
    for x0, y0, x1, y1 in json.loads (Drawing (screen)._labels ())["boxes"]:
        inside = x <= x0 and x1 <= x + width and y <= y0 and y1 <= y + height
        apart = x1 <= x or x + width <= x0 or y1 <= y or y + height <= y0
        assert inside or apart, "a printed detail cuts a name in half"
assert any (columns and columns[0] <= 43 <= columns[1] for _, _, columns in regions)
assert any (columns and columns[0] <= 62 <= columns[1] for _, _, columns in regions)

# A crop beginning at column 3 can still contain the board's original
# left row letters. Add replacement letters only when those are outside.
colors = Drawing (Bench ("Colors", columns=(1, 63)).led ("red", anode="b5", cathode="b6"))
caption, box, columns = next (r for r in colors.print_regions () if r[2])
assert columns[0] == 3
assert ">a</text>" not in colors.print_svg (caption, box, columns, "detail", "bench")
caption, box, columns = next (r for r in regions if r[2])
assert ">a</text>" in Drawing (screen).print_svg (caption, box, columns, "detail", "bench")

# The power feed uses the outer GND, not another same-named ground pin.
screen.finish ()
feed = screen.items[0]
assert feed[1][0] == ("pin", "GND5")
attrs = row (screen, feed)
assert json.loads (attrs["data-points"])[0] == [round (v, 2) for v in screen.pin_xy ("GND5")]
assert "outer GND pin" in attrs["data-action"]

# Buttons list their legs in physical row order; their electrical model
# groups the joined pairs. Endpoint labels must stay with the right holes.
button = Bench ("Button", columns=(1, 20)).button (2)
assert step_points (button, button.items[0]) == [[round (v, 2) for v in button.hole_xy (hole)]
                                               for hole in ("f2", "f4", "e2", "e4")]

# DIP packages describe two ranges instead of spelling out sixteen holes.
# Every pin is nevertheless a real highlighted point, never a range's centre.
chip = Bench ("Chip", columns=(1, 30)).chip ("74HC595", first=18)
points = step_points (chip, chip.items[0])
assert len (points) == 16
assert set (map (tuple, points)) == {tuple (round (v, 2) for v in chip.hole_xy (hole))
                                    for _, hole in chip.parts[0].legs ()}
assert "e18–e25" in row (chip, chip.items[0])["data-action"]

# A part's own lead is described first, even when circuit.py wrote the hole
# first. Its rings must agree, and the instruction must not add another wire.
motor = Bench ("Leads", columns=(1, 30)).module ("motor", at=(6, -2), facing="down")
motor.wire ("j14", "motor.+")
attrs = row (motor, motor.items[-1])
assert json.loads (attrs["data-points"]) == [[790, -134], [670, 110]]
assert attrs["data-action"] == "Connect the motor's red lead to j14."

# Re-numbering a step does not change its saved identity. Moving its hole,
# changing its colour, or changing whether it is removed must change it.
before = row (screen, wire)
assert before["data-step"] == row (screen, wire, "14")["data-step"]
other = Bench ("Other", columns=(1, 63)).wire ("a42", "B-42", color="black")
assert before["data-step"] != row (other, other.items[0])["data-step"]
removed = Tags (step_row ("t1", "−", wire, label="Take-out step 1 done")).first ("tr")
assert removed["data-step"] != before["data-step"]
assert "data-item" not in removed and json.loads (removed["data-points"]) == []
assert removed["data-action"].startswith ("Disconnect the black wire")

# Moving a module can leave its words unchanged ("below the breadboard").
# It still invalidates completion, since the physical build has changed.
left = Bench ("One", columns=(1, 63)).module ("servo", at=(6.0, 5.5))
right = Bench ("Two", columns=(1, 63)).module ("servo", at=(7.0, 5.5))
assert left.items[0][2].places == right.items[0][2].places
assert row (left, left.items[0])["data-step"] != row (right, right.items[0])["data-step"]
assert json.loads (row (left, left.items[0])["data-points"]) == []

# Board identity is explicit and attribute escaping round-trips quotes.
left.sketch = 'Dial & "counter"'
html = step_stages (left, [], left.items, 1, "A")
block = Tags (html).first ("div")
assert block["data-title"] == 'Board A · Dial & "counter"'
assert block["data-board"] == "A" and block["data-revision"]
same = Tags (step_stages (left, [], left.items, 1, "A")).first ("div")
assert same["data-revision"] == block["data-revision"]
# A caption names its board once, whether or not the title does.
assert board_caption ("A", "Board A, the meter: the LCD") == "Board A, the meter: the LCD"
assert board_caption ("B", "a button on pin 22") == "Board B · a button on pin 22"
assert board_caption ("", "Board games") == "Board games"
changed = Tags (step_stages (right, [], right.items, 1, "A")).first ("div")
assert changed["data-revision"] != block["data-revision"]

# A learner entering the guided build still gets the transition instruction.
old = Bench ("Before", columns=(1, 20)).home_led ("26", "red")
old.finish ()
new = Bench ("After", columns=(1, 20)).home_led ("26", "red").home_button ("22")
new.finish ()
html = steps (new, ({"number": 1, "slug": "001-blink"}, "", old), number=1)
assert 'class="build-context"' in html and "Keep from" in html
assert html.index ('class="build-context"') < html.index ('class="build-steps"')

# A chip swapped for another in the same holes must not meet a wire left
# on its old pins: every wire kept is named by its holes, and every one
# taken out is listed, however many.
def nand (bench, ties):
    bench.chip ("SN74HC00N", pins=["1A", "1B", "1Y", "2A", "2B", "2Y", "GND",
                                   "3Y", "3A", "3B", "4Y", "4A", "4B", "VCC"], first=16)
    bench.wire ("j16", "T+16").wire ("a22", "B-22").capacitor ("100 nF", "g15", "e15")
    for hole, rail in ties:
        bench.wire (hole, rail)
    return bench.finish ()


ties = [("j17", "T-17"), ("j18", "T-18"), ("j20", "T-19"), ("j21", "T-21"), ("a17", "B-17"),
        ("a18", "B-18"), ("a19", "B-19"), ("a21", "B-21"), ("j19", "T-22"), ("j22", "T-23")]
old = nand (Bench ("NAND", columns=(1, 30)), ties)
new = Bench ("Schmitt", columns=(1, 30))
new.chip ("SN74HC14N", pins=["1A", "1Y", "2A", "2Y", "3A", "3Y", "GND",
                             "4Y", "4A", "5Y", "5A", "6Y", "6A", "VCC"], first=16)
new.wire ("j16", "T+16").wire ("a22", "B-22").capacitor ("100 nF", "g15", "e15")
new.wire ("j17", "T-17").wire ("j21", "T-21").finish ()
html = steps (new, ({"number": 75, "slug": "075-set-reset-latch"}, "", old), number=76)
context = html[:html.index ('class="build-steps"')]
for words in ("100 nF capacitor", "j16", "a22", "j17 to the top − rail by column 17", "j21"):
    assert words in context, f"a chip swap names the kept wire {words}"
assert "everything else" not in context, "a chip swap lists what it takes out"
assert html.count ("Take-out step") == 9, "a chip swap lists every wire and part it takes out"

# Each board needs its own drawing, even when two boards have identical
# wiring. Sharing only Board A's figure left Board B without a guided view.
for page in sorted ((ROOT / "docs" / "lessons").glob ("*/index.md")):
    source = page.read_text (encoding="utf-8")
    figures = re.findall (r"<!-- bench(?: ([A-Z]))? -->", source)
    builds = re.findall (r"<!-- steps(?: ([A-Z]))? -->", source)
    assert sorted (figures) == sorted (builds), f"{page.parent.name}: each build needs a drawing"
    for match in re.finditer (r"<!-- bench(?P<board> [A-Z])? -->", source):
        following = "<!-- steps" + (match["board"] or "") + " -->"
        assert source[match.end ():].lstrip ().startswith (following), \
            f"{page.parent.name}: keep each board's drawing and steps together"

# A build carried on keeps its wires' colors: each wire a lesson keeps from
# the build before is drawn the color the keep text names it by, its color
# there. Where no part carries on, only the Mega's power wires stay. Every
# lesson is read as the site reads them; the routing runs a process to a
# core, which import this script again, so this part runs only here.
if __name__ == "__main__":
    load_all ()
    compared = 0
    complete_boards = 0
    for lesson in filter (written, LESSONS):
        for letter, bench in load_circuit (lesson).items ():
            caption = board_caption (letter, bench.title)
            assert not letter or caption.count (f"Board {letter}") == 1, \
                f"{lesson['slug']}{letter}: its caption names its board twice: {caption}"
            before = previous (lesson["number"], letter)
            full = empty_steps (lesson, letter, bench)
            made = rebuilt (bench, full, before[2] if before else None)
            assert len (made.items) == len (bench.items), lesson["slug"]
            assert same_nets (made, bench), f"{lesson['slug']}{letter}: complete build nets"
            complete_boards += 1
            if before is None:
                continue
            kept, _, _ = continuity (bench, before[2])
            if not carries (kept):
                kept = [(kind, thing) for kind, thing in kept if before[2].is_standard (thing)]
            changed = recolored (bench, before[2], kept)
            assert not changed, f"{lesson['slug']}{letter}: kept wires change color: {changed}"
            compared += sum (kind == "wire" for kind, _ in kept)
    assert compared > 500, f"only {compared} kept wires compared"
    assert complete_boards == 92, complete_boards

    # The guided route's actual cross-course predecessors, including an
    # optional logic detour after any of Lessons 16–36, not just each
    # destination's numbered predecessor. Empty starts above also cover
    # standalone E12/E13/E19/E22/E24 and the meter-only E17 entry.
    returns = [(58, 1), (3, 64), (65, 4), (6, 59), (60, 7), (9, 61),
               (61, 10), (12, 62), (63, 13), (15, 67), (67, 16),
               (42, 77), (77, 79), (79, 43), (65, 66), (63, 68)]
    returns += [(number, 74) for number in range (16, 37)]
    returns += [(76, number) for number in range (17, 38)]
    for old_number, number in returns:
        lesson = LESSONS[number - 1]
        actual = next (iter (load_circuit (LESSONS[old_number - 1]).values ()))
        for letter, target in load_circuit (lesson).items ():
            full = empty_steps (lesson, letter, target)
            made = rebuilt (target, full, actual if previous (number, letter) else None)
            assert same_nets (made, target), (old_number, number, letter)

    # Explicit failure case: 3 -> E09 -> E10 -> 4. E10 has a button in
    # the same home as Lesson 4, but fed from 5 V and joined to a base
    # resistor. Full generated steps must restore both pin 22 and ground.
    duel = load_circuit (LESSONS[2])[""]
    diode = load_circuit (LESSONS[63])[""]
    rebuilt_diode = rebuilt (diode, empty_steps (LESSONS[63], "", diode), duel)
    assert same_nets (rebuilt_diode, diode)
    switch = load_circuit (LESSONS[64])[""]
    ordinary = steps (switch, previous (65), 65).split ('<p class="complete-build-link">')[0]
    switch = continued (switch, rebuilt_diode, ordinary)
    assert same_nets (switch, load_circuit (LESSONS[64])[""])
    assert "pin 22" not in set ().union (*switch.nets ())
    mood = load_circuit (LESSONS[3])[""]
    full = empty_steps (LESSONS[3], "", mood)
    made = rebuilt (mood, full, switch)
    assert same_nets (made, mood)
    button_net = next (net for net in made.nets () if "pin 22" in net)
    assert "pin 5V" not in button_net and any ("button" in node for node in button_net)
    assert any ("pin GND" in net and any ("button" in node for node in net)
                for net in made.nets ())
    pin22 = next (index for index, item in enumerate (mood.items)
                  if item[0] == "wire" and ("pin", "22") in item[1][:2])
    missing = re.sub (rf'<tr\b[^>]*data-item="{pin22}".*?</tr>', "", full, flags=re.S)
    assert not same_nets (rebuilt (mood, missing, switch), mood), "missing wire must change nets"

    # Lesson 6 changes the button routes inherited from Lesson 5. Its
    # current via points win, for fresh, inherited and cache-recalled
    # drawings; an unchanged compatible route still carries through.
    simon = load_circuit (LESSONS[5])[""]
    fresh = load (ROOT / "docs" / "lessons" / LESSONS[5]["slug"] / "circuit.py")[""]
    for bench in (fresh, simon):
        for drawing in (Drawing (bench), Drawing (bench)):
            drawing._layout ()
            for index, wire in enumerate (bench.wires):
                assert follows (drawing._paths[index], drawing._plan (*wire)["points"]), \
                    f"Simon wire {index} misses current waypoints"
    drawing = Drawing (simon)
    drawing._layout ()
    compatible = next ((index, wire) for index, wire in enumerate (simon.wires)
                       if identity (simon, "wire", wire) in simon.ways and
                       follows (list (simon.ways[identity (simon, 'wire', wire)]),
                                drawing._plan (*wire)["points"]))
    index, wire = compatible
    assert drawing._paths[index] == simon.ways[identity (simon, "wire", wire)]

    print ("Guided build: coordinates, progress identity, board labels, carry-over and kept "
           "colors, complete route builds and explicit waypoints pass")
