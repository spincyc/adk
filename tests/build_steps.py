"""Check the guided build's link between instructions and drawing geometry.

Run with the site's Python: build/venv/bin/python tests/build_steps.py.
These checks need no browser, drawings cache, or generated lesson pages.
"""

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

from bench import Bench  # noqa: E402
from hooks import step_points, step_row, step_stages, steps  # noqa: E402
from drawing import Drawing  # noqa: E402


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
left, top, width, height = Drawing (lab).opening_box ()
assert lab.modules["wave"].reach_box ()[2] < left
x0, y0, x1, y1 = lab.modules["tap"].reach_box ()
assert left <= x0 < x1 <= left + width and top <= y0 < y1 <= top + height
regions = Drawing (screen).print_regions ()
assert all (crop[2] <= 430 for _, crop, columns in regions if columns)
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

print ("Guided build: coordinates, progress identity, board labels and carry-over pass")
