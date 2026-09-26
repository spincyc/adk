"""The bench: an Arduino Mega 2560 beside a breadboard, with parts standing in
real holes, modules beside the board, and jumper wires from real header pins.
This is the circuit itself; drawing.py draws it.

A lesson describes its build once, in circuit.py beside its page:

    bench = Bench ("An LED on pin 26", columns=(1, 20))
    bench.wire ("26", "j6")
    bench.resistor ("220 Ω", "g6", "e6")
    bench.led ("red", anode="b6", cathode="b7")
    bench.wire ("a7", "B-7")

Each part stands in its breadboard home, the holes it has in every lesson
(docs/kit.md lists them), so one build carries on into the next. The
Mega's power is wired by the bench itself, the same way every time: once
circuit.py has run, finish () brings GND from the outer pin at the end of
the long header into B-3 and 5V from the outer pin at its top into T+3,
when anything uses the rails, and joins a rail pair at the far end (B-60 to
T-60, T+61 to B+61) when a part uses the other rail and nothing else feeds
it. With a power module 5V stays off the rails. Wiring the Mega's GND or 5V
to a rail, or using those two pins, is an error.

Parts stand in the breadboard: resistor, led, button, rgb_led, buzzer,
potentiometer, photoresistor, thermistor, tilt_switch, chip (a DIP chip
across the middle gap), digit and four_digits (seven-segment displays), lcd,
header_module (a module standing in one row) and power_module, whose two
jumpers set each pair of rails to "5V", "3.3V" or "off":
power_module ("right", top="5V", bottom="off"). screen () builds the
course's LCD at its home: its contrast knob in e5-e7, the LCD's pins in
a9-a24, power, and pins 31 to 36, wired the same in every lesson.

Modules sit beside the board, placed in inches in the drawing's own
coordinates, where the Mega's top-left corner is (0.35, 0.6) and the
breadboard's is (5.15, 0.55); a wire reaches a module pin as "name.PIN", as
in bench.wire ("44", "servo.signal"). note () adds a small annotation with
an arrow. closeup (first, last) picks the columns the close-up shows, for a
build too wide to show whole.

A wire runs from a Mega pin, a hole or a module pin to another. The drawing
routes it round parts, modules and labels on its own; via=[...] makes it
pass through waypoints on the way, each a hole or an (x, y) point in inches,
and waypoints in line with each other set its corners exactly:
bench.wire ("36", "e18", via=["j14"]). A hole-to-hole jumper in one row or
one column lies straight when nothing is in its way.

The Mega's pins go by number ("26", "A0") or name. GND and 5V take the free
pin of that kind nearest the wire's other end; these names pick one:

    GND.top                 the GND beside pin 13 on the top header
    5V.power, 3.3V, VIN     on the power header
    GND.power, GND.power2   the power header's two GNDs, left then right
    5V.long                 the inner 5V at the top of the long double header
    GND.long                the inner GND at its bottom end
    5V.long2, GND.long2     their outer partners, kept for the rails

measure (label, red=..., black=..., expect=..., when=...) records a reading
to take with a multimeter set to DC volts: each probe on a hole (main or
rail) that holds or shares a strip with something in the build, on a Mega
pin number such as "26" (the probe goes in a free hole of the strip that
pin's wire lands in), or on "GND" or "5V" (a free hole of the − or + rail
nearest the other probe).

finish () then checks the circuit could work: no Mega pin's wire reaching
nothing, no pin joined straight to GND, 5V or 3.3V, and no part with two
legs in one strip. load () runs a lesson's circuit.py, one board's or two,
and finishes them.

From that one description come the pencil drawings, the build steps and the
table of connections; signal_pins () and pin_modes () let tests/pins.py
hold the sketch to the pins the build wires, and to what each one does.
Geometry is in inches on the real 0.1-inch grid: holes, header pins and leg
spacing are where they really are.
"""

import math
import re
from typing import NamedTuple

from modules import Placed, make as make_module
from parts import (Button, Buzzer, Chip, Display, HeaderModule, Led, PowerModule, Potentiometer,
                   Resistor, RgbLed, TwoLegs, bands_for)
from pencil import DPI

MEGA_WIDTH, MEGA_HEIGHT = 4.0, 2.1
BOARD_HEIGHT = 2.2
GAP = 0.8                           # between the Mega and the breadboard
MARGIN = 0.35

SIGNAL_COLORS = ["yellow", "green", "blue", "orange", "purple", "white", "brown", "grey"]
# Each home pin's wire keeps one color in every lesson, chosen so the pins
# that share a lesson differ where they can: an LED's wire in its LED's
# color (orange for red), the RGB LED's in its channel's.
PIN_COLORS = {
    "22": "purple", "23": "grey", "24": "brown", "25": "white",
    "26": "orange", "27": "yellow", "28": "green", "29": "blue", "30": "white",
    "31": "brown", "32": "grey", "33": "yellow", "34": "green", "35": "blue", "36": "purple",
    "37": "yellow", "38": "green", "39": "blue", "40": "orange", "41": "purple", "42": "white",
    "43": "brown", "44": "orange", "45": "yellow", "46": "orange", "47": "green", "48": "blue", "49": "purple",
    "50": "purple", "51": "white", "52": "brown", "53": "grey",
    "2": "white", "3": "orange", "4": "yellow", "5": "orange", "6": "green", "7": "blue",
    "8": "blue", "9": "white", "10": "grey", "11": "green", "12": "white", "14": "grey",
    "15": "purple", "16": "orange", "17": "white", "18": "white", "19": "grey", "20": "green",
    "21": "blue",
    "A0": "brown", "A1": "purple", "A2": "yellow", "A3": "yellow", "A4": "white", "A5": "green",
    "A8": "green", "A9": "blue", "A10": "purple", "A11": "white",
    "A12": "purple", "A13": "white", "A14": "brown", "A15": "grey",
}
ROWS = {"j": 0.55, "i": 0.65, "h": 0.75, "g": 0.85, "f": 0.95,
        "e": 1.25, "d": 1.35, "c": 1.45, "b": 1.55, "a": 1.65,
        "T+": 0.15, "T-": 0.25, "B+": 1.95, "B-": 2.05}
FACING = {"down": 0, "left": 90, "up": 180, "right": 270}

# Chips' pins by datasheet name, pin 1 first.
HC595 = ["Q1", "Q2", "Q3", "Q4", "Q5", "Q6", "Q7", "GND",
         "Q7'", "MR", "SH_CP", "ST_CP", "OE", "DS", "Q0", "VCC"]
# L293D pins 1-16, from its datasheet: the lower half drives one motor, the
# upper half (pins 9-15) another.
L293D = ["1,2EN", "1A", "1Y", "GND", "GND", "2Y", "2A", "VCC2",
         "3,4EN", "3A", "3Y", "GND", "GND", "4Y", "4A", "VCC1"]
CHIPS = {"74HC595": HC595, "L293D": L293D}

# The kit's common-cathode displays, pin 1 at the bottom left seen from the
# front, counting anticlockwise: the 5161AS digit and the 5461AS four digits.
DIGIT = ["e", "d", "common", "c", "dp", "b", "a", "common", "f", "g"]
FOUR_DIGITS = ["e", "d", "dp", "c", "g", "D4", "b", "D3", "D2", "f", "a", "D1"]


# The Mega's header pins by name, each at (x, y) inches from the board's
# bottom-left corner, USB socket on the left, on its header: the top one,
# the bottom one (power, then the analog pins) or the double header down the
# right-hand end, in its inner or outer column. block counts the separate
# strips of the top and bottom headers from the left. Pins that repeat (GND,
# 5V) get numbered names; canonical () turns them back.
class MegaPin (NamedTuple):
    x: float
    y: float
    header: str
    block: int = 0
    outer: bool = False


def mega_pins ():
    pins = {}
    top = ["SCL", "SDA", "AREF", "GND"] + [str (n) for n in range (13, 7, -1)]
    for index, name in enumerate (top):
        pins[name] = MegaPin (0.84 + 0.1 * index, 2.0, "top", 0)
    for index, n in enumerate (range (7, -1, -1)):
        pins[str (n)] = MegaPin (1.90 + 0.1 * index, 2.0, "top", 1)
    for index, n in enumerate (range (14, 22)):
        pins[str (n)] = MegaPin (2.80 + 0.1 * index, 2.0, "top", 2)
    for index, name in enumerate (["IOREF", "RESET", "3.3V", "5V", "GND2", "GND3", "VIN"]):
        pins[name] = MegaPin (1.04 + 0.1 * index, 0.1, "bottom", 0)
    for n in range (8):
        pins[f"A{n}"] = MegaPin (1.84 + 0.1 * n, 0.1, "bottom", 1)
    for n in range (8, 16):
        pins[f"A{n}"] = MegaPin (2.74 + 0.1 * (n - 8), 0.1, "bottom", 2)
    rows = [("5V2", "5V3")] + [(str (n), str (n + 1)) for n in range (22, 53, 2)]
    rows += [("GND4", "GND5")]
    for index, (even, odd) in enumerate (rows):
        pins[even] = MegaPin (3.70, 1.95 - 0.1 * index, "double")
        pins[odd] = MegaPin (3.80, 1.95 - 0.1 * index, "double", outer=True)
    return pins


MEGA_PINS = mega_pins ()

ALIASES = {
    "GND": ["GND", "GND2", "GND3", "GND4", "GND5"],
    "5V":  ["5V", "5V2", "5V3"],
}

# The pins that feed the rails, kept for them: the outer GND at the end of
# the long header and the outer 5V at its top.
RAIL_PINS = {"GND5": "B-3", "5V3": "T+3"}

# Names for particular power pins, as the docstring lists them.
PIN_NAMES = {
    "GND.top": "GND", "5V.power": "5V", "GND.power": "GND2", "GND.power2": "GND3",
    "5V.long": "5V2", "5V.long2": "5V3", "GND.long": "GND4", "GND.long2": "GND5",
}


class Bench:
    def __init__ (self, title, columns=(1, 30), seed=1, sketch=None):
        self.title = title
        self.sketch = sketch            # a board's own sketch, in a two-board lesson
        self.board = ""                 # its letter there: "A", "B"
        self.first, self.last = columns
        self.seed = seed
        self.taken = set ()
        self.parts = []
        self.modules = {}
        self.notes = []
        self.wires = []
        self.items = []                 # (kind, thing, step) in build order
        self.used = {}
        self.gap = GAP
        self.label_size = 10            # the drawing's, which parts' labels read
        self.closeup_range = None
        self._last = None
        self._finished = False
        self._powering = False
        self.measurements = []

    def _step (self, text):
        kind, thing = self._last
        self.items.append ((kind, thing, text))

    # Where things are ---------------------------------------------------

    def board_width (self):
        return 0.5 + (self.last - self.first) * 0.1

    def mega_origin (self):
        return MARGIN, MARGIN + 0.25

    def board_origin (self):
        mx, my = self.mega_origin ()
        return mx + MEGA_WIDTH + self.gap, my + (MEGA_HEIGHT - BOARD_HEIGHT) / 2

    def board_box (self):
        bx, by = self.board_origin ()
        return bx * DPI, by * DPI, (bx + self.board_width ()) * DPI, (by + BOARD_HEIGHT) * DPI

    def mega_box (self):
        mx, my = self.mega_origin ()
        return mx * DPI, my * DPI, (mx + MEGA_WIDTH) * DPI, (my + MEGA_HEIGHT) * DPI

    def size (self):
        bx, by = self.board_origin ()
        return (bx + self.board_width () + MARGIN) * DPI, (by + BOARD_HEIGHT + MARGIN) * DPI

    def pin_xy (self, name):
        mx, my = self.mega_origin ()
        pin = MEGA_PINS[name]
        return (mx + pin.x) * DPI, (my + MEGA_HEIGHT - pin.y) * DPI

    def hole_xy (self, hole):
        bx, by = self.board_origin ()
        _, column, row = parse_hole (hole)
        if not self.first <= column <= self.last:
            raise ValueError (f"{hole} is outside the drawn columns {self.first}-{self.last}")
        return (bx + 0.25 + (column - self.first) * 0.1) * DPI, (by + ROWS[row]) * DPI

    def strip_of (self, hole):
        kind, column, row = parse_hole (hole)
        if kind == "rail":
            return f"rail {row}"
        return f"column {column} {'a-e' if row in 'abcde' else 'f-j'}"

    # Parts on the breadboard -------------------------------------------

    def resistor (self, value, a, b):
        self._add (Resistor (value, a, b))
        self._step (f"The {value} resistor ({', '.join (bands_for (value))}) "
                           f"from {a} to {b}.")
        return self

    def led (self, color, anode, cathode):
        self._add (Led (color, anode, cathode))
        self._step (f"The {color} LED: long leg in {anode}, short leg in {cathode}.")
        return self

    def button (self, column):
        part = Button (column)
        self._add (part)
        self._step (f"A push button across the middle gap, legs in "
                           f"{', '.join (part.holes ())}.")
        return self

    def rgb_led (self, red, common, green, blue):
        self._add (RgbLed (red, common, green, blue))
        self._step (f"The RGB LED: red leg in {red}, the longest leg (common, −) in {common}, "
                           f"green in {green}, blue in {blue}.")
        return self

    def buzzer (self, positive, negative, kind="active"):
        _, c1, r1 = parse_hole (positive)
        _, c2, r2 = parse_hole (negative)
        along_a_row = r1 == r2 and abs (c1 - c2) == 3
        across_the_gap = c1 == c2 and {r1, r2} == {"e", "f"}
        if not (along_a_row or across_the_gap):
            raise ValueError ("a 12 mm buzzer's legs are 0.3 inch apart: three columns apart in "
                              "one row, or across the middle gap in rows e and f")
        self._add (Buzzer (positive, negative, kind))
        # A new active buzzer's + leg is also the longer; follow the mark on either.
        mark = "+ mark and longer leg" if kind == "active" else "+ mark"
        self._step (f"The {kind} buzzer, its {mark} in {positive}, the other leg in "
                    f"{negative}.")
        return self

    def potentiometer (self, left, wiper, right, value="10 kΩ"):
        spots = [parse_hole (hole) for hole in (left, wiper, right)]
        steps = {spots[1][1] - spots[0][1], spots[2][1] - spots[1][1]}
        if len ({row for _, _, row in spots}) != 1 or len (steps) != 1 or steps - {1, 2}:
            raise ValueError ("a potentiometer's three legs stand in one row, evenly spaced")
        self._add (Potentiometer (left, wiper, right, value))
        self._step (f"The {value} potentiometer, legs in {left}, {wiper} and {right}: the "
                           f"middle one is the wiper.")
        return self

    def photoresistor (self, a, b):
        return self._two_legs ("photoresistor", a, b)

    def thermistor (self, a, b):
        return self._two_legs ("thermistor", a, b)

    def tilt_switch (self, a, b):
        return self._two_legs ("tilt switch", a, b)

    def _two_legs (self, kind, a, b):
        self._add (TwoLegs (kind, a, b))
        self._step (f"The {kind}, legs in {a} and {b}, either way round.")
        return self

    # A DIP chip across the middle gap: pin 1 in e<first>, pins running
    # right along row e and back along row f (or d and h for a 0.6 inch
    # package).
    def chip (self, name, pins=None, first=1, span=3):
        pins = list (pins or CHIPS[name])
        holes, rows = self._straddle (len (pins), first, span)
        self._add (Chip (name, pins, holes))
        half = len (pins) // 2
        self._step (
            f"The {name} across the middle gap, its notch to the left: pin 1 ({pins[0]}) in "
            f"{holes[0]}, pins 1–{half} along row {rows[0]} to {holes[half - 1]}, and pins "
            f"{half + 1}–{len (pins)} back along row {rows[1]} from {holes[half]} to pin "
            f"{len (pins)} ({pins[-1]}) in {holes[-1]}.")
        return self

    def digit (self, first, shows=None):
        return self._display ("digit display", DIGIT, first, 50, 1, shows)

    def four_digits (self, first, shows=None):
        return self._display ("four-digit display", FOUR_DIGITS, first, 198, 4, shows)

    def _display (self, name, pins, first, width, digits, shows):
        holes, rows = self._straddle (len (pins), first, 6)
        labels = [segment_label (pin) for pin in pins]
        self._add (Display (name, pins, labels, holes, width, digits, shows))
        half = len (pins) // 2
        self._step (
            f"The {name} across the middle gap, decimal point{'s' if digits > 1 else ''} at the "
            f"bottom: pins 1–{half} in {holes[0]}–{holes[half - 1]} (pin 1, {labels[0]}, on the "
            f"left) and pins {half + 1}–{len (pins)} in {holes[half]}–{holes[-1]}.")
        return self

    def _straddle (self, count, first, span):
        if count % 2:
            raise ValueError ("a DIP package has an even number of pins")
        rows = {3: ("e", "f"), 6: ("d", "h")}[span]
        half = count // 2
        holes = [f"{rows[0]}{first + index}" for index in range (half)]
        holes += [f"{rows[1]}{first + half - 1 - index}" for index in range (half)]
        return holes, rows

    # The LCD1602 plugged into one row. In row j its screen lies off the top
    # edge, turned half round, so pin 16 (K) is in the first column; in row
    # a it lies off the bottom edge, reading the right way up.
    def lcd (self, first, row="j", text=None):
        part = self._header_part ("lcd", None, first, row, "LCD", None, {"text": text})
        pins = ", ".join (f"{pin.name} in {hole}" for pin, hole in part.pairs ())
        edge = "bottom" if part.turned else "top"
        self._step (
            f"The LCD, face up, its screen lying over the {edge} edge of the board"
            f"{'' if part.turned else ' (upside down from where you sit)'} and its 16 pins in "
            f"{part.holes[0]}–{part.holes[-1]}: {pins}.")
        return self

    # The course's screen, at its home and wired the same in every lesson so
    # it can stay on the breadboard from one to the next. From column first
    # (5): the contrast knob, its legs in row e, and the LCD four columns
    # on, in row a, its screen over the bottom edge. The knob's left leg
    # takes GND from the bottom − rail; three short jumpers join its legs to
    # VSS, V0 and VDD; VSS, VDD and RW reach up to the top rails, so VDD
    # takes 5V from the top + rail and the top − rail is GND through VSS.
    # The signal wires from pins 31 to 36 rise beside the Mega and cross
    # over the board in lanes, 31 lowest, each coming straight down into its
    # pin's column; the backlight's 220 Ω resistor stands across the gap at
    # the far end. lanes lifts the signal wires' lanes by that many steps,
    # and risers gives where the first rises and how far apart they stand,
    # in inches, to make room for other wires from the header.
    def screen (self, first=5, text=None, lanes=0, risers=(4.45, 0.1)):
        knob, lcd = first, first + 4
        column = lambda offset: lcd + offset
        self.potentiometer (f"e{knob}", f"e{knob + 1}", f"e{knob + 2}")
        self.lcd (lcd, row="a", text=text)
        self.wire (f"a{knob}", self._rail ("B-", knob))
        self.wire (f"b{knob}", f"b{lcd}", color="black")
        self.wire (f"c{knob + 1}", f"c{column (2)}", color="brown")
        self.wire (f"d{knob + 2}", f"d{column (1)}", color="red")
        self.wire (f"e{lcd}", self._rail ("T-", lcd))
        self.wire (f"e{column (1)}", self._rail ("T+", column (1)))
        self.wire (f"e{column (4)}", self._rail ("T-", column (4)))
        # The six signal wires rise beside the Mega and cross over the board
        # in lanes, 31 lowest, so each comes straight down into its column
        # and none crosses another over the board.
        # Each leaves the header at its own height, the inner pins' wires
        # between their neighbors' plugs.
        targets = [(31, 3, 0), (32, 5, -0.05), (33, 10, 0), (34, 11, -0.05), (35, 12, 0),
                   (36, 13, -0.05)]
        mx, my = self.mega_origin ()
        _, by = self.board_origin ()
        for index, (pin, offset, jog) in enumerate (targets):
            x, _ = self.hole_xy (f"e{column (offset)}")
            height = my + MEGA_HEIGHT - MEGA_PINS[str (pin)].y + jog
            lane = by - 0.12 - 0.1 * (index + lanes)
            riser = risers[0] + risers[1] * index
            self.wire (str (pin), f"e{column (offset)}",
                       via=[(mx + MEGA_WIDTH - 0.1, height), (riser, height), (riser, lane),
                            (x / DPI - 0.05, lane)])
        self.resistor ("220 Ω", f"e{column (14)}", f"f{column (14)}")
        self.wire (f"j{column (14)}", self._rail ("T+", column (14)))
        self.wire (f"e{column (15)}", self._rail ("T-", column (15)))
        return self

    # The first free hole of a rail at or after a column.
    def _rail (self, rail, column):
        while not rail_column (column) or f"{rail}{column}" in self.used:
            column += 1
        return f"{rail}{column}"

    # A module standing in one row on its own header, pins listed left to
    # right as they meet the columns.
    def header_module (self, kind, pins=None, first=1, row="j", name=None, label=None, **options):
        part = self._header_part (kind, pins, first, row, name, label, options)
        pins = ", ".join (f"{pin.name} in {hole}" for pin, hole in part.pairs ())
        edge = "bottom" if part.turned else "top"
        self._step (f"The {part.name}, standing in row {row} with its board toward the "
                           f"{edge} edge: {pins}.")
        return self

    def _header_part (self, kind, pins, first, row, name, label, options):
        if row not in "abcdefghij" or len (row) != 1:
            raise ValueError (f"a header stands in one of rows a-j, not {row!r}")
        turned = row in "abcde"
        made = make_module (kind, list (reversed (pins)) if pins and turned else pins, label, **options)
        holes = [f"{row}{first + index}" for index in range (len (made.names))]
        part = HeaderModule (made, holes, turned, name or made.title)
        self._add (part)
        return part

    # The breadboard power supply across both pairs of rails at one end,
    # each side's jumper on "5V", "3.3V" or "off": wires to the rails take
    # their power from it.
    def power_module (self, end="left", top="5V", bottom="5V"):
        if end not in ("left", "right"):
            raise ValueError ("the power module goes on the left or right end")
        columns = (3, 4) if end == "left" else (60, 61)
        if not (self.first <= columns[0] and columns[1] <= self.last):
            raise ValueError (f"the power module needs columns {columns[0]}-{columns[1]} drawn")
        if end == "left":
            # Its jack hangs over the end, so the board moves over for it.
            self.gap = GAP + 1.3
        part = PowerModule (end, columns, top, bottom)
        self._add (part)
        setting = lambda value: {"5V": "5 V", "3.3V": "3.3 V", "off": "off"}[value]
        jumpers = f"both jumpers on {setting (top)}" if top == bottom else \
            f"the top jumper on {setting (top)} and the bottom one {setting (bottom)}"
        jumpers = jumpers.replace ("on off", "off")
        holes = ", ".join (unbroken (f"{rail}{columns[0]}") for rail in ("T+", "T-", "B+"))
        self._step (
            f"The power module on the {end} end of the board, its pins in both pairs of rails "
            f"({holes}, {unbroken (f'B-{columns[0]}')} and the holes beside them), {jumpers}.")
        return self

    # Modules beside the board -------------------------------------------

    # A module placed with its top-left corner at (x, y) inches, its pins
    # facing the board unless facing says otherwise (down, up, left, right).
    def module (self, kind, name=None, at=(0.0, 0.0), pins=None, label=None, facing=None,
                **options):
        name = name or kind
        if name in self.modules or "." in name:
            raise ValueError (f"a module needs a new name without a dot, not {name!r}")
        made = make_module (kind, pins, label, **options)
        angle = FACING[facing or self._facing (at, made)]
        placed = Placed (made, name, 0, 0, angle)
        w, h = made.width, made.height
        turned_w, turned_h = (h, w) if angle in (90, 270) else (w, h)
        rx, ry = placed.turn (w / 2, h / 2)
        placed.x, placed.y = at[0] * DPI + turned_w / 2 - rx, at[1] * DPI + turned_h / 2 - ry
        placed.title = self._unique (made.title)
        # It must lie clear of the Mega, the breadboard and the other
        # modules, jumper housings and all.
        mx, my = self.mega_origin ()
        things = {"the Mega": ((mx - 0.25) * DPI, my * DPI, (mx + MEGA_WIDTH) * DPI,
                               (my + MEGA_HEIGHT) * DPI),
                  "the breadboard": self.board_box ()}
        things.update ({f"the {other.title}": other.reach_box () for other in self.modules.values ()})
        for thing, box in things.items ():
            if overlaps (placed.reach_box (), grow (box, 8)):
                raise ValueError (f"the {placed.title} at {at} overlaps {thing}")
        self.modules[name] = placed
        self._last = ("module", placed)
        self._step (f"The {placed.title}, {self._whereabouts (placed.box ())}.")
        return self

    def _facing (self, at, made):
        bx, by = self.board_origin ()
        x, y = at
        if y + made.height / DPI <= by + 0.2:
            return "down"
        if y >= by + BOARD_HEIGHT - 0.2:
            return "up"
        if x >= bx + self.board_width () - 0.2:
            return "left"
        return "down"

    def _whereabouts (self, box):
        x0, y0, x1, y1 = (v / DPI for v in box)
        bx, by = self.board_origin ()
        over = "the Mega" if (x0 + x1) / 2 < bx - self.gap / 2 else "the breadboard"
        if y1 <= by + 0.1:
            return f"above {over}"
        if y0 >= by + BOARD_HEIGHT - 0.1:
            return f"below {over}"
        if x0 >= bx + self.board_width () - 0.1:
            return "to the right of the breadboard"
        return "beside the board"

    # The standard power hook-up, the same in every lesson so it never
    # drifts: the Mega's GND (the pair at the end of the long header, nearest
    # the board) into the bottom − rail's first hole, B-3, and its 5V (the
    # pair at the top of the long header) into the top + rail's first hole,
    # T+3, whenever the rails are used. A rail that nothing else powers is
    # linked to its partner at the board's far end: GND from B-60 to T-60,
    # 5V from T+61 to B+61. A power module, at the far end, feeds the rails
    # itself, and the Mega's GND still joins them at B-3. The site calls this
    # once circuit.py has run; circuits never wire the Mega's power to a rail.
    # Then it checks the circuit could work, and places the probes.
    def finish (self):
        if self._finished:
            return self
        self._finished = True
        self._power ()
        self._check ()
        for index in range (len (self.measurements)):
            self.probes (index)
        return self

    def _power (self):
        module = next ((part for part in self.parts if isinstance (part, PowerModule)), None)
        # The rails something uses; the power module's own pins don't count.
        own = {hole for _, hole in module.legs ()} if module else set ()
        used = {parse_hole (hole)[2] for hole in self.used
                if parse_hole (hole)[0] == "rail" and hole not in own}
        if not used:
            return
        power = [self._power_wire (("GND.long2", "GND.long"), "B-3")]
        # The Mega's 5V feeds the top rails unless the power module does:
        # with the module's top jumper off, the screen and sensors run from
        # the Mega, as their signals do, and the module feeds only the
        # bottom rails, for motors.
        if (not module and used & {"T+", "B+"}) or \
                (module and module.top == "off" and "T+" in used):
            power.append (self._power_wire (("5V.long2", "5V.long"), "T+3"))
        links = []
        for rail, source, a, b in (("T-", "GND", "B-60", "T-60"), ("B+", "5V", "T+61", "B+61"),
                                   ("T+", "5V", None, None), ("B-", "GND", None, None)):
            if rail not in used or self._powered (rail, source) \
                    or (source == "5V" and self._powered (rail, "3.3V")):
                continue
            if module or not a:
                side = "top" if rail[0] == "T" else "bottom"
                raise ValueError (f"nothing feeds the {side} {rail[1]} rail: " +
                                  (f"set the power module's {side} jumper" if module else
                                   "wire it from a part that does"))
            self.last = max (self.last, 63)
            links.append (self._power_wire (None, a, b))
        # Power first in the build steps, the links after it.
        rest = [item for item in self.items if item not in power + links]
        self.items = power + links + rest

    # What no circuit may do: leave a Mega pin's wire reaching nothing, as a
    # wire in the wrong hole does; join a pin straight to GND, 5V or 3.3V,
    # which shorts it; or stand a part with two of its legs in one strip,
    # which joins them.
    def _check (self):
        for net in self.nets ():
            pins = {m[4:] for m in net if m.startswith ("pin ")} - POWER_PINS
            if not pins:
                continue
            named = " and ".join (f"pin {p}" if numbered (p) else p
                                  for p in sorted (pins, key=pin_order))
            for source in ("GND", "5V", "3.3V"):
                if self._carries (net, source):
                    raise ValueError (f"{named} would be shorted to {source}")
            if not any (": " in member for member in net):
                one = len (pins) == 1
                raise ValueError (f"{named} reach{'es' if one else ''} nothing: no part's leg or "
                                  f"module's pin shares a strip with {'its' if one else 'their'} "
                                  f"wire")
        names = self._names ()
        for part in self.parts:
            # Legs joined inside the part, as a button's are, may share a strip.
            group = {leg: leg for leg, _ in part.legs ()}
            for a, b in part.inside ():
                old, new = group[b], group[a]
                group = {leg: new if was == old else was for leg, was in group.items ()}
            strips = {}
            for leg, hole in part.legs ():
                strips.setdefault (self.strip_of (hole), []).append ((leg, hole))
            for strip, legs in strips.items ():
                apart = [(leg, hole) for leg, hole in legs if group[leg] != group[legs[0][0]]]
                if apart:
                    (leg, hole), (other, where) = legs[0], apart[0]
                    raise ValueError (f"the {names[id (part)]}'s {leg} in {hole} and {other} in "
                                      f"{where} share {strip}, which joins them")

    # How the sketch must claim each Mega pin, where the circuit shows it:
    # as an output when the pin drives an LED, a buzzer or a module's input,
    # as an input when it reads a button, a knob or a sensor. A resistor
    # passes the question on to what is beyond it, short of GND, 5V and the
    # other pins. {pin: (mode, the parts that say so)}, for each pin whose
    # parts all agree.
    def pin_modes (self):
        nets = self.nets ()
        where = {member: index for index, net in enumerate (nets) for member in net}
        names = self._names ()
        said, through = {}, []
        for part in self.parts:
            legs = [f"{names[id (part)]}: {leg}" for leg, _ in part.legs ()]
            if isinstance (part, Resistor):
                through.append ({where[leg] for leg in legs})
            for leg, mode in zip (legs, part.modes ()):
                if mode:
                    said[leg] = (mode, f"the {names[id (part)]}")
        for placed in self.modules.values ():
            for pin in placed.pins ():
                mode = placed.kind.pin_modes.get (pin.name)
                if mode:
                    said[f"{placed.title}: {pin.label ()}"] = (mode,
                                                               f"the {placed.title}'s {pin.name}")
        pins = [{m[4:] for m in net if m.startswith ("pin ")} - POWER_PINS for net in nets]
        stops = {index for index, net in enumerate (nets)
                 if any (self._carries (net, source) for source in ("GND", "5V", "3.3V"))}
        found = {}
        for start, own in enumerate (pins):
            if len (own) != 1:
                continue
            seen, queue = {start}, [start]
            while queue:
                here = queue.pop ()
                for ends in through:
                    for there in ends if here in ends else ():
                        if there not in seen and there not in stops and not pins[there]:
                            seen.add (there)
                            queue.append (there)
            reached = [said[m] for index in sorted (seen) for m in sorted (nets[index])
                       if m in said]
            modes = {mode for mode, _ in reached}
            if len (modes) == 1:
                found[min (own)] = (modes.pop (), sorted ({thing for _, thing in reached}))
        return found

    def _power_wire (self, pins, hole, end=None):
        self._powering = True
        try:
            if pins:
                pin = next (p for p in pins if PIN_NAMES[p] not in self.taken)
                self.wire (pin, hole)
            else:
                self.wire (hole, end)
        finally:
            self._powering = False
        return self.items[-1]

    # Whether a wire is one the bench adds itself: a power feed or a link.
    def is_standard (self, wire):
        holes = {name for kind, name in wire[:2] if kind == "hole"}
        pins = {name for kind, name in wire[:2] if kind == "pin"}
        return bool (pins & set (RAIL_PINS)) and holes <= set (RAIL_PINS.values ()) \
            or holes in ({"B-60", "T-60"}, {"T+61", "B+61"})

    # What a wire's end is fixed to, in terms that hold from one lesson's
    # bench to the next: a module's pin by the module's title and the pin.
    def end_key (self, end):
        kind, where = end
        if kind == "module":
            placed, pin = self.module_pin (where)
            return ("module", placed.title, pin.name)
        return end

    # Whether a rail is joined to a Mega pin or a power module leg of this
    # kind: "GND", "5V" or, from a power module, "3.3V".
    def _powered (self, rail, source):
        for net in self.nets ():
            if f"rail {rail}" in net:
                return self._carries (net, source)
        return False

    # Whether a net is fed with GND, 5V or 3.3V: by the Mega's pin, or by a
    # power module's leg; a module that only uses it doesn't count.
    def _carries (self, net, source):
        leg = {"GND": "GND", "5V": "5 V", "3.3V": "3.3 V"}[source]
        feeds = {m for m in self._sources () if m.endswith (": " + leg)}
        return f"pin {source}" in net or bool (feeds & net)

    # A reading to take with a multimeter on DC volts, between the red
    # probe's point and the black probe's, expected to be about expect, and
    # when to take it.
    def measure (self, label, red, black, expect, when=None):
        self.measurements.append (dict (label=label, red=red, black=black, expect=expect,
                                        when=when))
        return self

    # Where each probe of a measurement touches: (hole, words for it).
    def probes (self, index):
        taken = self.measurements[index]
        red = self._probe (taken["red"], None)
        black = self._probe (taken["black"], red[0])
        if taken["red"] in ("GND", "5V"):
            red = self._probe (taken["red"], black[0])
        return red, black

    def _probe (self, point, near):
        if point in ("GND", "5V"):
            rail = self._rail_of (point)
            covered = self._covered ()
            holes = [h for h in self.holes ()
                     if h.startswith (rail) and h not in self.used and h not in covered]
            if not holes:
                raise ValueError (f"a probe on {point} needs a free hole in a {point} rail")
            x = self.hole_xy (near)[0] if near else 0
            hole = min (holes, key=lambda h: abs (self.hole_xy (h)[0] - x))
            return hole, f"{point}, at {self.describe (('hole', hole))}"
        if point in MEGA_PINS or point in PIN_NAMES:
            name = PIN_NAMES.get (point, point)
            landed = [e for s, f, _, _ in self.wires for p, e in ((s, f), (f, s))
                      if p == ("pin", name) and e[0] == "hole"]
            if not landed:
                raise ValueError (f"a probe on pin {point} needs its wire to land in a hole")
            hole = landed[0][1]
            strip = [h for h in self.holes () if self.strip_of (h) == self.strip_of (hole)
                     and h not in self.used]
            if strip:
                near = self.hole_xy (landed[0][1])
                hole = min (strip, key=lambda h: math.dist (self.hole_xy (h), near))
            name = canonical (name)
            return hole, f"{'pin ' if numbered (name) else ''}{name}, at {hole}"
        self.hole_xy (point)
        strip = self.strip_of (point)
        if not any (self.strip_of (h) == strip for h in self.used):
            raise ValueError (f"a probe at {point} touches nothing in the build")
        if point in self._covered ():
            raise ValueError (f"a probe at {point} can't reach it under a part")
        # A wire's end fills its hole, so the probe goes in a free one beside it.
        if self.used.get (point) == "a wire":
            covered = self._covered ()
            free = [h for h in self.holes ()
                    if self.strip_of (h) == strip and h not in self.used and h not in covered]
            if not free:
                raise ValueError (f"a probe at {point} needs a free hole in its strip")
            near = self.hole_xy (point)
            point = min (free, key=lambda h: math.dist (self.hole_xy (h), near))
        what = self.used.get (point)
        if what and what != "a wire":
            return point, f"{self.describe (('hole', point))}, on {what}'s leg"
        if not what and parse_hole (point)[0] == "main":
            legs = [hole for hole, owner in self.used.items ()
                    if owner != "a wire" and self.strip_of (hole) == strip]
            if legs:
                spot = self.hole_xy (point)
                near = min (legs, key=lambda hole: math.dist (self.hole_xy (hole), spot))
                return point, f"{point}, in the same strip as {self.used[near]}'s leg in {near}"
        return point, self.describe (("hole", point))

    # The rail a GND or 5V probe goes in: one the build has joined to it.
    def _rail_of (self, point):
        for rail in (("B-", "T-") if point == "GND" else ("T+", "B+")):
            for net in self.nets ():
                if f"rail {rail}" in net and self._carries (net, point):
                    return rail
        raise ValueError (f"no rail carries {point} for a probe")

    # A small annotation: text set off from what it points at (a hole, a pin,
    # "module.PIN" or (x, y) inches) by offset inches, with an arrow.
    def note (self, text, at, offset=(-0.45, -0.35)):
        self.notes.append ((text, at, offset))
        return self

    # Wires ---------------------------------------------------------------

    def wire (self, start, end, color=None, via=None):
        # Resolve a hole before a shared pin name, so GND or 5V can be the
        # header pin nearest the hole.
        if start in ALIASES:
            end_end = self._resolve (end, None)
            start_end = self._resolve (start, self.xy (end_end))
        else:
            start_end = self._resolve (start, None)
            end_end = self._resolve (end, self.xy (start_end))
        leads = [end for end in (start_end, end_end) if self.style (end) == "lead"]
        if len (leads) == 2:
            raise ValueError ("two leads need a wire or a hole between them")
        if color is None:
            color = self.module_pin (leads[0][1])[1].color if leads else \
                self._color ((start_end, end_end))
        for point in via or ():
            self.point_xy (point)
        if not self._powering and {start_end[0], end_end[0]} == {"pin", "hole"}:
            pin = start_end[1] if start_end[0] == "pin" else end_end[1]
            hole = end_end[1] if end_end[0] == "hole" else start_end[1]
            if canonical (pin) in ("GND", "5V") and parse_hole (hole)[0] == "rail":
                raise ValueError (f"the Mega's {canonical (pin)} reaches the rails by itself "
                                  f"(GND at B-3, 5V at T+3); leave out the wire to {hole}")
        wire = (start_end, end_end, color, list (via or ()))
        self.wires.append (wire)
        self._last = ("wire", wire)
        self._step (self._wire_step (start_end, end_end, color))
        return self

    def _wire_step (self, start, end, color):
        for lead, other in ((start, end), (end, start)):
            if self.style (lead) == "lead":
                placed, pin = self.module_pin (lead[1])
                return f"The {placed.title}'s {pin.note} into {self.describe (other)}."
        males = sum (self.style (end) == "male" for end in (start, end))
        jumper = ("", "female-to-male ", "female-to-female ")[males]
        # The article agrees with the first word after it: the color.
        article = "An" if color[0] in "aeiou" else "A"
        return f"{article} {color} {jumper}wire from {self.describe (start)} to {self.describe (end)}."

    # Black to ground, red to power, and any other wire a color of its own
    # that never changes from lesson to lesson: a pin's wire by its number,
    # a jumper by its holes.
    def _color (self, ends):
        kinds = {self._polarity (end) for end in ends}
        if "ground" in kinds:
            return "black"
        if "power" in kinds:
            return "red"
        pins = [canonical (name) for kind, name in ends if kind == "pin"]
        if pins and pins[0] in PIN_COLORS:
            return PIN_COLORS[pins[0]]
        if pins and numbered (pins[0]):
            return SIGNAL_COLORS[int (pins[0].lstrip ("A")) % len (SIGNAL_COLORS)]
        key = "".join (sorted (name for _, name in ends))
        return SIGNAL_COLORS[sum (key.encode ()) % len (SIGNAL_COLORS)]

    def _polarity (self, end):
        kind, name = end
        if kind == "pin":
            label = canonical (name)
            return "ground" if label == "GND" else "power" if label in ("5V", "3.3V") else None
        if kind == "hole":
            return "ground" if name.startswith (("T-", "B-")) else \
                "power" if name.startswith (("T+", "B+")) else None
        pin = self.module_pin (name)[1].name.upper ()
        if pin in ("−", "-", "GND", "G", "VSS"):
            return "ground"
        if pin in ("+", "VCC", "5V", "+5V", "3.3V", "VDD"):
            return "power"
        return None

    # A pin name, a module's pin or a hole. A shared name (GND, 5V) picks the
    # free header pin nearest the wire's other end.
    def _resolve (self, name, toward):
        if name in ALIASES:
            free = [p for p in ALIASES[name] if p not in self.taken
                    and (self._powering or p not in RAIL_PINS)]
            if not free:
                raise ValueError (f"no free {name} pin")
            if toward:
                free.sort (key=lambda p: math.dist (self.pin_xy (p), toward))
            self.taken.add (free[0])
            return ("pin", free[0])
        name = PIN_NAMES.get (name, name)
        if name in RAIL_PINS and not self._powering:
            raise ValueError (f"{canonical (name)} {name[-1]} feeds the rail at {RAIL_PINS[name]}; "
                              f"use another {canonical (name)} pin")
        if name in MEGA_PINS:
            if name in self.taken:
                raise ValueError (f"pin {name} already has a wire")
            self.taken.add (name)
            return ("pin", name)
        module, _, pin = name.partition (".")
        if pin and module in self.modules:
            placed = self.modules[module]
            found = placed.pin (pin)
            key = f"{module}.{found.name}"
            if key in self.taken:
                raise ValueError (f"{key} already has a wire")
            self.taken.add (key)
            placed.wired.add (found.name)
            return ("module", key)
        self.hole_xy (name)
        self._claim ([name], "a wire")
        return ("hole", name)

    def module_pin (self, key):
        module, _, pin = key.partition (".")
        placed = self.modules[module]
        return placed, placed.pin (pin)

    def style (self, end):
        return self.module_pin (end[1])[1].style if end[0] == "module" else end[0]

    # Where to find a pin, a hole or a module's pin, in words.
    def describe (self, end):
        kind, name = end
        if kind == "module":
            placed, pin = self.module_pin (name)
            if pin.style == "lead":
                return f"the {placed.title}'s {pin.note}"
            noun = {"male": "pin", "female": "socket", "screw": "terminal"}[pin.style]
            owner = f"the {placed.title}{' plug' if pin.style == 'female' else ''}"
            note = f" ({pin.note})" if pin.note else ""
            return f"{owner}'s {pin.name} {noun}{note}"
        if kind == "hole":
            _, column, row = parse_hole (name)
            if row in ROWS and len (row) == 2:
                side = "top" if row[0] == "T" else "bottom"
                sign = "+" if row[1] == "+" else "−"
                return f"the {side} {sign} rail ({unbroken (name)})"
            return name
        label = canonical (name)
        pin = MEGA_PINS[name]
        if numbered (label):
            return f"pin {label}"
        if pin.header == "double":
            where = f"at the {'end' if pin.y < 1 else 'top'} of the long header"
            return f"the {'outer' if pin.outer else 'inner'} {label} pin {where}"
        if pin.header == "top":
            return f"the {label} pin beside pin 13" if label == "GND" else \
                f"the {label} pin at the left end of the top header"
        # The power header has two GND pins, and one of each other.
        return f"{'a' if label == 'GND' else 'the'} {label} pin on the power header"

    # Where a point is: a hole, a Mega pin, "module.PIN" or (x, y) inches.
    def point_xy (self, at):
        if isinstance (at, tuple):
            return at[0] * DPI, at[1] * DPI
        if at in MEGA_PINS:
            return self.pin_xy (at)
        module, _, pin = at.partition (".")
        if pin and module in self.modules:
            placed = self.modules[module]
            return placed.anchor (placed.pin (pin))[0]
        return self.hole_xy (at)

    def xy (self, end):
        kind, name = end
        if kind == "pin":
            return self.pin_xy (name)
        if kind == "hole":
            return self.hole_xy (name)
        placed, pin = self.module_pin (name)
        return placed.anchor (pin)[0]

    # Holes ----------------------------------------------------------------

    def _add (self, part):
        holes = [hole for _, hole in part.legs ()]
        for hole in holes:
            self.hole_xy (hole)
        part.name = self._unique (part.name, modules_only=True)
        covered = self._under (part.footprint (self)) - set (holes)
        clash = sorted (covered & set (self.used))
        if clash:
            raise ValueError (f"the {part.name} would cover {', '.join (clash)}")
        self._claim (holes, f"the {part.name}")
        self.parts.append (part)
        self._last = ("part", part)

    def _claim (self, holes, what):
        for hole in holes:
            if hole in self.used:
                raise ValueError (f"{hole} already holds {self.used[hole]}")
            xy = self.hole_xy (hole)
            for part in self.parts:
                if inside_shapes (part.footprint (self), xy):
                    raise ValueError (f"{hole} is under the {part.name}")
        for hole in holes:
            self.used[hole] = what

    def holes (self):
        for column in range (self.first, self.last + 1):
            for row in "abcdefghij":
                yield f"{row}{column}"
            if rail_column (column):
                for rail in ("T+", "T-", "B+", "B-"):
                    yield f"{rail}{column}"

    # Holes a probe can't reach, under a part's body or a module lying on the
    # board. The kit's jumpers are flexible and arch clear of the holes.
    def _covered (self):
        covered = set ()
        for part in self.parts:
            covered |= self._under (part.footprint (self))
        for placed in self.modules.values ():
            covered |= self._under ([("rect", *placed.box ())])
        return covered - set (self.used)

    def _under (self, shapes):
        return {hole for hole in self.holes ()
                if shapes and inside_shapes (shapes, self.hole_xy (hole))}

    # A name no module has yet; a part may share its name with other parts,
    # which the connections tell apart by their holes.
    def _unique (self, name, modules_only=False):
        names = {m.title for m in self.modules.values ()}
        if not modules_only:
            names |= {part.name for part in self.parts}
        base, count = name, 2
        while name in names:
            name, count = f"{base} {count}", count + 1
        return name

    def _names (self):
        counts = {}
        for part in self.parts:
            counts[part.name] = counts.get (part.name, 0) + 1
        names = {}
        for part in self.parts:
            holes = [hole for _, hole in part.legs ()]
            where = f"{holes[0]}–{holes[-1]}" if len (holes) == 2 else f"at {holes[0]}"
            names[id (part)] = f"{part.name} ({where})" if counts[part.name] > 1 else part.name
        return names

    # Connections --------------------------------------------------------

    def _node (self, end):
        kind, name = end
        if kind == "pin":
            return "pin " + pin_id (name)
        if kind == "module":
            placed, pin = self.module_pin (name)
            return f"{placed.title}: {pin.label ()}"
        return self.strip_of (name)

    def nets (self):
        parent = {}

        def find (node):
            parent.setdefault (node, node)
            while parent[node] != node:
                parent[node] = parent[parent[node]]
                node = parent[node]
            return node

        def join (a, b):
            parent[find (a)] = find (b)

        names = self._names ()
        for start, end, _, _ in self.wires:
            join (self._node (start), self._node (end))
        for part in self.parts:
            for leg, hole in part.legs ():
                join (f"{names[id (part)]}: {leg}", self.strip_of (hole))
            for a, b in part.inside ():
                join (f"{names[id (part)]}: {a}", f"{names[id (part)]}: {b}")

        groups = {}
        for member in list (parent):
            groups.setdefault (find (member), set ()).add (member)
        return list (groups.values ())

    # Legs that feed power, such as the power module's 5 V, listed with the
    # Mega's pins.
    def _sources (self):
        names = self._names ()
        return {f"{names[id (part)]}: {leg}" for part in self.parts for leg in part.sources}

    # The circuit point by point: every junction that joins two or more
    # things, in the order current meets them walking out from each pin.
    def connections (self):
        sources = self._sources ()
        nets = [net for net in self.nets ()
                if len ({m for m in net if not m.startswith (("column ", "rail "))}) >= 2]
        part_of = lambda member: member.split (": ")[0]
        fed = lambda net: any (m.startswith ("pin ") or m in sources for m in net)
        order = []
        queue = sorted ((net for net in nets if fed (net)),
                        key=lambda net: (any (m in ("pin GND", "pin 5V") or m in sources for m in net),
                                         min ([pin_order (m[4:]) for m in net if m.startswith ("pin ")]
                                              or [(3, "")])))
        while queue:
            net = queue.pop (0)
            if net in order:
                continue
            order.append (net)
            parts = {part_of (m) for m in net if ": " in m}
            following = [other for other in nets if other not in order and other not in queue
                         and parts & {part_of (m) for m in other if ": " in m}]
            queue = following + queue
        order += [net for net in nets if net not in order]

        rows = []
        for net in order:
            pins = sorted ({m[4:] for m in net if m.startswith ("pin ")}, key=pin_order)
            pins = [f"pin {p}" if numbered (p) else p for p in pins]
            pins += sorted (m.replace (": ", " ") for m in net if m in sources)
            legs = sorted (m for m in net if not m.startswith (("pin ", "column ", "rail "))
                           and m not in sources)
            rows.append ((pins, legs))
        return rows

    # The Mega pins the build wires, other than its power pins.
    def signal_pins (self):
        ends = [end for start, finish, _, _ in self.wires for end in (start, finish)]
        return {pin_id (name) for kind, name in ends if kind == "pin"} - POWER_PINS

    # Show these columns in the close-up, for a build too wide to show whole.
    def closeup (self, first, last):
        self.closeup_range = (first, last)
        return self


# Run a lesson's circuit.py and finish its builds. A one-board lesson's
# circuit makes one Bench, bench; a two-board lesson's makes a Bench for
# each board, each naming its own sketch, and lists them in order as
# boards = {"A": ..., "B": ...}. Returns {letter: Bench}, where a one-board
# lesson's board is "".
def load (path):
    scope = {"Bench": Bench, "HC595": HC595, "L293D": L293D}
    exec (compile (open (path, encoding="utf-8").read (), path, "exec"), scope)
    if ("bench" in scope) == ("boards" in scope):
        raise ValueError ('a circuit makes one Bench called bench, or one for each board in '
                          'boards = {"A": ..., "B": ...}')
    if "bench" in scope:
        boards = {"": scope["bench"]}
        if scope["bench"].sketch:
            raise ValueError ("a one-board lesson's sketch is named after the lesson: leave out "
                              "sketch=")
    else:
        boards = dict (scope["boards"])
        if list (boards) != [chr (ord ("A") + index) for index in range (max (2, len (boards)))]:
            raise ValueError ('a two-board lesson lists its boards in order: boards = {"A": ..., '
                              '"B": ...}')
        names = [bench.sketch for bench in boards.values ()]
        if not all (names) or len (set (names)) != len (names):
            raise ValueError ("each board names its own sketch, as Bench (..., sketch=\"Dial\") "
                              "for examples/LessonNNName/Dial/Dial.ino")
    for letter, bench in boards.items ():
        bench.board = letter
        bench.finish ()
    return boards


# The folder a lesson's example is in: examples/Lesson13HelloLcd for
# docs/lessons/13-hello-lcd. A one-board lesson's sketch is in it, named
# the same; a two-board lesson's are in folders of their own inside it.
def example (slug):
    number, _, words = slug.partition ("-")
    return f"Lesson{number}" + "".join (word.capitalize () for word in words.split ("-"))


def parse_hole (hole):
    match = re.fullmatch (r"([a-j])(\d{1,2})", hole)
    if match and 1 <= int (match.group (2)) <= 63:
        return "main", int (match.group (2)), match.group (1)
    match = re.fullmatch (r"([TB][+-])(\d{1,2})", hole)
    if match:
        column = int (match.group (2))
        if not rail_column (column):
            raise ValueError (f"rail {match.group (1)} has no hole at column {column}")
        return "rail", column, match.group (1)
    raise ValueError (f"{hole!r} is neither a Mega pin, a module pin nor a breadboard hole")


def rail_column (column):
    # The rails' holes come in fives with a gap, from column 3 to 61.
    return 3 <= column <= 61 and (column - 3) % 6 != 5


def canonical (name):
    return re.sub (r"^(GND|5V)\d$", r"\1", name)


# The pin a name means in the circuit: GND2 is GND, and the SDA and SCL pins
# by AREF are pins 20 and 21 by other names.
def pin_id (name):
    name = canonical (name)
    return {"SDA": "20", "SCL": "21"}.get (name, name)


POWER_PINS = {"GND", "5V", "3.3V", "VIN"}


# A hole's name that a line of text never breaks at its hyphen: B-3, with a
# word joiner after the hyphen.
def unbroken (hole):
    return hole.replace ("-", "-\u2060")


# Pins called by number, "pin 26" or "pin A0", rather than by name.
def numbered (label):
    return re.fullmatch (r"\d+|A\d+", label) is not None


def segment_label (pin):
    if pin == "common":
        return "common (−)"
    if pin == "dp":
        return "decimal point"
    if pin.startswith ("D"):
        return f"digit {pin[1:]}"
    return f"segment {pin}"


def pin_order (name):
    if name.isdigit ():
        return (0, int (name))
    if re.fullmatch (r"A\d+", name):
        return (1, int (name[1:]))
    return (2, name)


# Geometry ---------------------------------------------------------------------

def inside_shapes (shapes, point):
    x, y = point
    for shape in shapes:
        if shape[0] == "rect" and shape[1] + 0.5 < x < shape[3] - 0.5 \
                and shape[2] + 0.5 < y < shape[4] - 0.5:
            return True
        if shape[0] == "circle" and math.hypot (x - shape[1], y - shape[2]) < shape[3]:
            return True
    return False


def grow (box, margin):
    return box[0] - margin, box[1] - margin, box[2] + margin, box[3] + margin


def overlaps (a, b):
    return a[0] < b[2] and b[0] < a[2] and a[1] < b[3] and b[1] < a[3]


