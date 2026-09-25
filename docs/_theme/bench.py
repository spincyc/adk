"""The bench: an Arduino Mega 2560 beside a breadboard, with parts standing in
real holes, modules beside the board, and jumper wires from real header pins.

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

A wire runs from a Mega pin, a hole or a module pin to another. It is
routed round parts, modules and labels on its own; via=[...] makes it pass
through waypoints on the way, each a hole or an (x, y) point in inches, and
waypoints in line with each other set its corners exactly:
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
nearest the other probe). measure_svg (index) draws a small close-up of
the two probe points with the meter below, reading what is expected.

From that one description come the pencil drawings, the build steps and the
table of connections, and signal_pins () lets the site check the sketch
declares exactly the pins the build wires. Geometry is in inches on the real
0.1-inch grid: holes, header pins and leg spacing are where they really are.
"""

import hashlib
import json
import math
import os
import re

import meter

from modules import HOUSING, Placed, make as make_module
from parts import (Button, Buzzer, Chip, Display, HeaderModule, Label, Led, PowerModule,
                   Potentiometer, Resistor, RgbLed, TwoLegs, bands_for, spots_round)
from pencil import DPI, Pencil
from route import (HARD, Placer, Router, corners, node, segment_distance, segment_meets_box,
                   text_box, text_width)

MEGA_WIDTH, MEGA_HEIGHT = 4.0, 2.1
BOARD_HEIGHT = 2.2
GAP = 0.8                           # between the Mega and the breadboard
MARGIN = 0.35
MEGA_TINT = "#dfe6e8"
CLOSEUP_COLUMNS = 16                # the narrowest close-up, so parts keep one scale
ROUNDS = 6                          # of negotiation between the wires

WIRE_COLORS = {
    "red": "#be4c44", "black": "#3a3a3a", "blue": "#4a72ad", "green": "#56874f",
    "yellow": "#d3ae3f", "orange": "#cf8243", "white": "#ece8de", "purple": "#7a63a0",
    "brown": "#86613f", "grey": "#9a9a98",
}
SIGNAL_COLORS = ["yellow", "green", "blue", "orange", "purple", "white", "brown", "grey"]
# Each home pin's wire keeps one color in every lesson, chosen so the pins
# that share a lesson differ where they can: an LED's wire in its LED's
# color (orange for red), the RGB LED's in its channel's.
PIN_COLORS = {
    "22": "purple", "23": "grey", "24": "brown", "25": "white",
    "26": "orange", "27": "yellow", "28": "green", "29": "blue", "30": "white",
    "31": "brown", "32": "grey", "33": "yellow", "34": "green", "35": "blue", "36": "purple",
    "37": "yellow", "38": "green", "39": "blue", "40": "orange", "41": "purple", "42": "white",
    "43": "brown", "44": "orange", "45": "yellow", "47": "green", "48": "blue", "49": "purple",
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

# Directions a wire may leave a header in: the way its pins point, or a
# little either side, to fan out from its neighbours.
LEAVING = {"top": (6, 5, 7), "bottom": (2, 1, 3), "side": (0, 7, 1)}


# The Mega's header pins as name -> (x, y) in inches from the bottom-left
# corner, USB socket on the left. Pins that repeat (GND, 5V) get numbered
# names; canonical () turns them back.
def mega_pins ():
    pins = {}
    top = ["SCL", "SDA", "AREF", "GND"] + [str (n) for n in range (13, 7, -1)]
    for index, name in enumerate (top):
        pins[name] = (0.84 + 0.1 * index, 2.0)
    for index, n in enumerate (range (7, -1, -1)):
        pins[str (n)] = (1.90 + 0.1 * index, 2.0)
    for index, n in enumerate (range (14, 22)):
        pins[str (n)] = (2.80 + 0.1 * index, 2.0)
    for index, name in enumerate (["IOREF", "RESET", "3.3V", "5V", "GND2", "GND3", "VIN"]):
        pins[name] = (1.04 + 0.1 * index, 0.1)
    for n in range (8):
        pins[f"A{n}"] = (1.84 + 0.1 * n, 0.1)
    for n in range (8, 16):
        pins[f"A{n}"] = (2.74 + 0.1 * (n - 8), 0.1)
    rows = [("5V2", "5V3")] + [(str (n), str (n + 1)) for n in range (22, 53, 2)]
    rows += [("GND4", "GND5")]
    for index, (even, odd) in enumerate (rows):
        pins[even] = (3.70, 1.95 - 0.1 * index)
        pins[odd] = (3.80, 1.95 - 0.1 * index)
    return pins


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
    def __init__ (self, title, columns=(1, 30), seed=1):
        self.title = title
        self.first, self.last = columns
        self.seed = seed
        self.pins = mega_pins ()
        self.taken = set ()
        self.parts = []
        self.modules = {}
        self.notes = []
        self.wires = []
        self.items = []                 # (kind, thing, step) in build order
        self.used = {}
        self.gap = GAP
        self.label_size = 10
        self._routes = None
        self._last = None
        self._finished = False
        self._powering = False
        self.measurements = []

    # The build steps, one for each part, module and wire, in build order.
    @property
    def steps (self):
        return [step for _, _, step in self.items]

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
        x, y = self.pins[name]
        return (mx + x) * DPI, (my + MEGA_HEIGHT - y) * DPI

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

    # The first column after everything on the main rows, so a build fills
    # the board from the Mega's end in order.
    def next_column (self, gap=0):
        columns = [parse_hole (hole)[1] for hole in self.used if parse_hole (hole)[0] == "main"]
        return max (columns) + 1 + gap if columns else self.first

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
        self._step (f"The {kind} buzzer, its + mark and longer leg in {positive}, the other "
                           f"leg in {negative}.")
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
        # between their neighbours' plugs.
        targets = [(31, 3, 0), (32, 5, -0.05), (33, 10, 0), (34, 11, -0.05), (35, 12, 0),
                   (36, 13, -0.05)]
        mx, my = self.mega_origin ()
        _, by = self.board_origin ()
        for index, (pin, offset, jog) in enumerate (targets):
            x, _ = self.hole_xy (f"e{column (offset)}")
            height = my + MEGA_HEIGHT - self.pins[str (pin)][1] + jog
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
        self._step (
            f"The power module on the {end} end of the board, its pins in both pairs of rails "
            f"(T+{columns[0]}, T-{columns[0]}, B+{columns[0]}, B-{columns[0]} and the holes beside "
            f"them), {jumpers}.")
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
        self._routes = None
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
    def finish (self):
        if self._finished:
            return self
        self._finished = True
        module = next ((part for part in self.parts if isinstance (part, PowerModule)), None)
        # The rails something uses; the power module's own pins don't count.
        own = {hole for _, hole in module.legs ()} if module else set ()
        used = {parse_hole (hole)[2] for hole in self.used
                if parse_hole (hole)[0] == "rail" and hole not in own}
        if not used:
            for index in range (len (self.measurements)):
                self.probes (index)
            return self
        power = [self._power_wire (("GND.long2", "GND.long"), "B-3")]
        if not module and used & {"T+", "B+"}:
            power.append (self._power_wire (("5V.long2", "5V.long"), "T+3"))
        links = []
        for rail, source, a, b in (("T-", "GND", "B-60", "T-60"), ("B+", "5V", "T+61", "B+61"),
                                   ("T+", "5V", None, None), ("B-", "GND", None, None)):
            if rail not in used or self._powered (rail, source):
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
        for index in range (len (self.measurements)):
            self.probes (index)
        return self

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
    def _standard (self, ends):
        holes = {name for kind, name in ends if kind == "hole"}
        pins = {name for kind, name in ends if kind == "pin"}
        return bool (pins & set (RAIL_PINS)) and holes <= set (RAIL_PINS.values ()) \
            or holes in ({"B-60", "T-60"}, {"T+61", "B+61"})

    # Whether a rail is joined to a Mega pin or a power module leg of this
    # kind: "GND" or "5V".
    def _powered (self, rail, source):
        for net in self.nets ():
            if f"rail {rail}" in net:
                return self._carries (net, source)
        return False

    # Whether a net is fed with GND or 5V: by the Mega's pin, or by a power
    # module's leg; a module that only uses it doesn't count.
    def _carries (self, net, source):
        leg = "GND" if source == "GND" else "5 V"
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
            holes = [h for h in self._holes () if h.startswith (rail) and h not in self.used]
            if not holes:
                raise ValueError (f"a probe on {point} needs a free hole in a {point} rail")
            x = self.hole_xy (near)[0] if near else 0
            hole = min (holes, key=lambda h: abs (self.hole_xy (h)[0] - x))
            return hole, f"{point}, at {self.describe (('hole', hole))}"
        if point in self.pins or point in PIN_NAMES:
            name = PIN_NAMES.get (point, point)
            landed = [e for s, f, _, _ in self.wires for p, e in ((s, f), (f, s))
                      if p == ("pin", name) and e[0] == "hole"]
            if not landed:
                raise ValueError (f"a probe on pin {point} needs its wire to land in a hole")
            hole = landed[0][1]
            strip = [h for h in self._holes () if self.strip_of (h) == self.strip_of (hole)
                     and h not in self.used]
            if strip:
                near = self.hole_xy (landed[0][1])
                hole = min (strip, key=lambda h: distance (self.hole_xy (h), near))
            return hole, f"pin {canonical (name)}, at {hole}"
        self.hole_xy (point)
        strip = self.strip_of (point)
        if not any (self.strip_of (h) == strip for h in self.used):
            raise ValueError (f"a probe at {point} touches nothing in the build")
        what = self.used.get (point)
        if what and what != "a wire":
            return point, f"{self.describe (('hole', point))}, on {what}'s leg"
        if not what and parse_hole (point)[0] == "main":
            legs = [hole for hole, owner in self.used.items ()
                    if owner != "a wire" and self.strip_of (hole) == strip]
            if legs:
                near = min (legs, key=lambda hole: distance (self.hole_xy (hole), self.hole_xy (point)))
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
            start_end = self._resolve (start, self._xy (end_end))
        else:
            start_end = self._resolve (start, None)
            end_end = self._resolve (end, self._xy (start_end))
        leads = [end for end in (start_end, end_end) if self._style (end) == "lead"]
        if len (leads) == 2:
            raise ValueError ("two leads need a wire or a hole between them")
        if color is None:
            color = self._module_pin (leads[0][1])[1].color if leads else \
                self._color ((start_end, end_end))
        for point in via or ():
            self._point (point)
        if not self._powering and {start_end[0], end_end[0]} == {"pin", "hole"}:
            pin = start_end[1] if start_end[0] == "pin" else end_end[1]
            hole = end_end[1] if end_end[0] == "hole" else start_end[1]
            if canonical (pin) in ("GND", "5V") and parse_hole (hole)[0] == "rail":
                raise ValueError (f"the Mega's {canonical (pin)} reaches the rails by itself "
                                  f"(GND at B-3, 5V at T+3); leave out the wire to {hole}")
        wire = (start_end, end_end, color, list (via or ()))
        self.wires.append (wire)
        self._routes = None
        self._last = ("wire", wire)
        self._step (self._wire_step (start_end, end_end, color))
        return self

    def _wire_step (self, start, end, color):
        for lead, other in ((start, end), (end, start)):
            if self._style (lead) == "lead":
                placed, pin = self._module_pin (lead[1])
                return f"The {placed.title}'s {pin.note} into {self.describe (other)}."
        males = sum (self._style (end) == "male" for end in (start, end))
        jumper = ("", "female-to-male ", "female-to-female ")[males]
        # The article agrees with the first word after it: the colour.
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
        pin = self._module_pin (name)[1].name.upper ()
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
                free.sort (key=lambda p: distance (self.pin_xy (p), toward))
            self.taken.add (free[0])
            return ("pin", free[0])
        name = PIN_NAMES.get (name, name)
        if name in RAIL_PINS and not self._powering:
            raise ValueError (f"{canonical (name)} {name[-1]} feeds the rail at {RAIL_PINS[name]}; "
                              f"use another {canonical (name)} pin")
        if name in self.pins:
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

    def _module_pin (self, key):
        module, _, pin = key.partition (".")
        placed = self.modules[module]
        return placed, placed.pin (pin)

    def _style (self, end):
        return self._module_pin (end[1])[1].style if end[0] == "module" else end[0]

    # Where to find a pin, a hole or a module's pin, in words.
    def describe (self, end):
        kind, name = end
        if kind == "module":
            placed, pin = self._module_pin (name)
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
                return f"the {side} {sign} rail ({name})"
            return name
        label = canonical (name)
        x, y = self.pins[name]
        if numbered (label):
            return f"pin {label}"
        if 0.11 < y < 1.99:
            where = "at the end of the long header" if y < 1 else "at the top of the long header"
            return f"the {'outer' if x > 3.75 else 'inner'} {label} pin {where}"
        elif y > 1.99:
            where = "beside pin 13"
        else:
            where = "on the power header"
        return f"a {label} pin {where}"

    def _xy (self, end):
        kind, name = end
        if kind == "pin":
            return self.pin_xy (name)
        if kind == "hole":
            return self.hole_xy (name)
        placed, pin = self._module_pin (name)
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
        self._routes = None
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

    def _holes (self):
        for column in range (self.first, self.last + 1):
            for row in "abcdefghij":
                yield f"{row}{column}"
            if rail_column (column):
                for rail in ("T+", "T-", "B+", "B-"):
                    yield f"{rail}{column}"

    def _under (self, shapes):
        return {hole for hole in self._holes ()
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
            return "pin " + canonical (name)
        if kind == "module":
            placed, pin = self._module_pin (name)
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

    def signal_pins (self):
        ends = [end for start, finish, _, _ in self.wires for end in (start, finish)]
        return {canonical (name) for kind, name in ends if kind == "pin"
                if canonical (name) not in ("GND", "5V", "3.3V", "VIN")}

    # Routing -------------------------------------------------------------

    # Every wire's path, worked out once for both drawings. Jumpers that lie
    # straight are fixed first; the rest are routed in rounds, the nearer
    # ends first, each round round the others' last, until no two share any
    # stretch of grid. Failing that, they are routed strictly, one at a time.
    def _layout (self):
        if self._routes is not None:
            return self._routes
        self.label_size = 10
        plans = [self._plan (*wire) for wire in self.wires]
        paths = self._recall (plans)
        if paths is None:
            paths = self._negotiate_all (plans)
            self._remember (plans, paths)
        self._paths = paths
        routes = []
        for index, plan in enumerate (plans):
            drawn = corners (paths[index])
            drawn[0], drawn[-1] = plan["a"], plan["b"]
            routes.append ((plan["start"], plan["end"], drawn))
        self._routes = routes
        return routes

    def _negotiate_all (self, plans):
        router = self._router ()
        order = sorted (range (len (plans)), key=lambda index: (not self._jumper (index),
                                                                  self._reach (index)))
        paths = {}
        for index in order:
            if plans[index]["straight"]:
                paths[index] = plans[index]["straight"]
                router.claim (paths[index], index)
        pressure, best, stale = 0.5, None, 0
        moving = [index for index in order if not plans[index]["straight"]]
        for _ in range (ROUNDS):
            for index in moving:
                router.unclaim (index)
                paths[index] = self._negotiate (router, plans[index], index, pressure)
                router.claim (paths[index], index)
            shared, crossed = router.clashes ()
            if not shared:
                price = router.price ()
                stale = stale + 1 if best and price >= best[0] else 0
                if best is None or price < best[0]:
                    best = (price, dict (paths))
                if not crossed or stale >= 2:
                    break
            for n in shared:
                router.history[n] = router.history.get (n, 0) + 1
            for n in crossed:
                router.history[n] = router.history.get (n, 0) + 0.5
            pressure *= 2
            # Only the wires in a clash, or crossing, have anything to settle.
            involved = {wire for n in shared + crossed for wire in router.occupied.get (n, {})}
            moving = [index for index in order if index in involved and not plans[index]["straight"]]
        if best:
            paths = best[1]
        else:
            for index in order:
                if not plans[index]["straight"]:
                    router.unclaim (index)
            for index in order:
                if not plans[index]["straight"]:
                    paths[index] = self._negotiate (router, plans[index], index, None)
                    router.claim (paths[index], index)
        return paths

    # Routes worked out before are kept under the build directory, by
    # everything they were worked out from, engine and all, so a rebuild
    # only routes what has changed.
    def _fingerprint (self, plans):
        text = repr ((self.first, self.last, self.gap, sorted (self.used), sorted (self.taken),
                      [(p["points"], p["first"], sorted (p["allow"]), p["keep_off"], p["straight"])
                       for p in plans],
                      [(type (part).__name__, part.legs (), part.shapes (self), part.blocks)
                       for part in self.parts],
                      [(m.reach_box (), m.title_box ()) for m in self.modules.values ()]))
        digest = hashlib.sha256 (text.encode ())
        for name in sorted (os.listdir (THEME)):
            if name.endswith (".py"):
                digest.update (open (os.path.join (THEME, name), "rb").read ())
        return digest.hexdigest ()

    def _recall (self, plans):
        folder = route_cache ()
        if not folder:
            return None
        try:
            with open (os.path.join (folder, self._fingerprint (plans) + ".json")) as file:
                kept = json.load (file)
        except (OSError, ValueError):
            return None
        return {int (index): [tuple (n) for n in path] for index, path in kept.items ()}

    def _remember (self, plans, paths):
        folder = route_cache ()
        if not folder:
            return
        os.makedirs (folder, exist_ok=True)
        with open (os.path.join (folder, self._fingerprint (plans) + ".json"), "w") as file:
            json.dump ({index: path for index, path in paths.items ()}, file)

    # How a wire is to be routed: its ends in routing order, where each is
    # drawn, the grid points it passes, and its path when it lies straight.
    def _plan (self, start, end, color, via):
        if self._oriented (start, end) != (start, end):
            start, end, via = end, start, list (reversed (via))
        a, first, allow_a = self._terminal (start)
        b, _, allow_b = self._terminal (end)
        # Into a module's pin straight along the way the pin points.
        approach = []
        if end[0] == "module":
            (x, y), out = self._module_pin (end[1])[0].anchor (self._module_pin (end[1])[1])
            approach = [node ((b[0] + out[0] * 10, b[1] + out[1] * 10))]
        plan = dict (start=start, end=end, a=a, b=b, first=first, allow=allow_a | allow_b,
                     points=[node (a)] + [node (self._point (point)) for point in via] + approach
                     + [node (b)],
                     keep_off=self.board_box () if "hole" not in (start[0], end[0]) else None,
                     straight=None)
        if start[0] == end[0] == "hole" and not via and self._straight (start[1], end[1]):
            plan["straight"] = straight_nodes (node (a), node (b))
        return plan

    def _negotiate (self, router, plan, index, pressure):
        try:
            path, _ = router.route (plan["points"], plan["first"], plan["allow"], plan["keep_off"],
                                    index, pressure)
        except ValueError as error:
            raise ValueError (f"the wire from {self.describe (plan['start'])} to "
                              f"{self.describe (plan['end'])} can't get through ({error}): move "
                              f"what is in its way, or give it via points") from error
        return path

    def _reach (self, index):
        start, end, _, _ = self.wires[index]
        (x1, y1), (x2, y2) = self._xy (start), self._xy (end)
        return abs (x1 - x2) + abs (y1 - y2)

    def _jumper (self, index):
        start, end, _, via = self.wires[index]
        return start[0] == end[0] == "hole" and not via

    def _router (self):
        x0, y0, x1, y1 = self._extent ()
        router = Router ((x0 - 80, y0 - 120, x1 + 80, y1 + 120), self.board_box ())
        # The Mega's middle is solid; wires only leave its header strips.
        mx0, my0, mx1, my1 = self.mega_box ()
        router.block (("rect", mx0 - 25, my0 + 17, mx1 - 37, my1 - 17), grow=0)
        router.cost (("rect", mx0 - 25, my0, mx1, my1), 14)
        # Out of each header's strip, never along it: right from the double
        # header, up from the top one, down from the power and analog one.
        router.one_way ((mx1 - 37, my0 + 17, mx1, my1 - 17), (0, 1, 7))
        router.one_way ((mx0 - 25, my0, mx1 - 37, my0 + 17), (5, 6, 7))
        router.one_way ((mx0 - 25, my1 - 17, mx1 - 37, my1), (1, 2, 3))
        # Wires leaving the double header run close together until they part.
        router.close ((mx1 - 37, my0, mx1 + 15, my1))
        # Wires keep a little way off the Mega's edge and the board's.
        for box in (self.mega_box (), self.board_box ()):
            router.fringe (box, 4, 5)
        for name in self.pins:
            router.hole (self.pin_xy (name), True, near=False)
        for hole in self._holes ():
            router.hole (self.hole_xy (hole), hole in self.used)
        # The board's printing, which wires would rather not hide.
        for x, y, w in self._print_marks ():
            router.cost (("rect", x - w / 2, y - 6, x + w / 2, y), 4)
        for x, y, w in self._board_signs ():
            router.cost (("rect", x - w / 2, y - 6, x + w / 2, y), 12, grow=1)
        bx0, by0, bx1, _ = self.board_box ()
        for offset in (0.06, 0.34, 1.86, 2.14):
            y = by0 + offset * DPI
            router.cost (("rect", bx0 + 14, y - 1, bx1 - 14, y + 1), 3)
        for part in self.parts:
            for shape in part.shapes (self):
                if part.blocks:
                    router.block (shape)
                else:
                    router.cost (shape, 5)
            # Leave room where each part's label would best go.
            for label in part.labels (self)[:1]:
                x, y, anchor, _ = label.spots[0]
                router.cost (("rect", *text_box (x, y, label.text, 10 * label.size, anchor)), 10)
        for placed in self.modules.values ():
            router.block (("rect", *placed.reach_box ()))
            router.block (("rect", *placed.title_box ()))
        return router

    # Everything drawn but wires and labels, for sizing the grid.
    def _extent (self):
        boxes = [self.mega_box (), self.board_box ()]
        boxes += [placed.reach_box () for placed in self.modules.values ()]
        boxes += [placed.title_box () for placed in self.modules.values ()]
        boxes += [part.box (self) for part in self.parts if part.box (self)]
        return (min (b[0] for b in boxes), min (b[1] for b in boxes),
                max (b[2] for b in boxes), max (b[3] for b in boxes))

    # A wire is routed from its Mega end, or failing that its module end.
    def _oriented (self, start, end):
        if end[0] == "pin" and start[0] != "pin" or start[0] == "hole" and end[0] == "module":
            return end, start
        return start, end

    # Where a wire's end is drawn, the ways it may leave, and grid nodes it
    # may cross though they are blocked.
    def _terminal (self, end):
        kind, name = end
        if kind == "pin":
            _, y = self.pins[name]
            side = "top" if y > 1.99 else "bottom" if y < 0.11 else "side"
            return self.pin_xy (name), LEAVING[side], set ()
        if kind == "hole":
            return self.hole_xy (name), None, set ()
        placed, pin = self._module_pin (name)
        (x, y), out = placed.anchor (pin)
        reach = {"male": HOUSING, "female": 4, "screw": 0, "lead": 0}[pin.style]
        tip = (x + out[0] * reach, y + out[1] * reach)
        heading = {(1, 0): 0, (0, 1): 2, (-1, 0): 4, (0, -1): 6}[(round (out[0]), round (out[1]))]
        start = node (tip)
        allow = {(start[0] + round (out[0]) * step, start[1] + round (out[1]) * step)
                 for step in range (0, 4)}
        return tip, (heading, (heading + 1) % 8, (heading + 7) % 8), allow

    # Whether a jumper between two holes in one row or column can lie flat
    # and straight: nothing but free holes between.
    def _straight (self, a, b):
        (x1, y1), (x2, y2) = self.hole_xy (a), self.hole_xy (b)
        if abs (x1 - x2) > 0.5 and abs (y1 - y2) > 0.5:
            return False
        for hole in self._holes ():
            if hole in (a, b) or hole not in self.used:
                continue
            x, y = self.hole_xy (hole)
            if min (x1, x2) - 0.5 <= x <= max (x1, x2) + 0.5 and \
                    min (y1, y2) - 0.5 <= y <= max (y1, y2) + 0.5:
                return False
        box = (min (x1, x2), min (y1, y2), max (x1, x2), max (y1, y2))
        for part in self.parts:
            if not part.blocks:
                continue
            for shape in part.shapes (self):
                if shape_crosses (shape, (x1, y1), (x2, y2)):
                    return False
        for placed in self.modules.values ():
            if segment_meets_box ((x1, y1), (x2, y2), placed.reach_box (), 2):
                return False
        return bool (box)

    # Drawing ------------------------------------------------------------

    # The whole bench, or a close-up of the breadboard where the parts are.
    def svg (self, view="bench", prefix="bench"):
        routes = self._layout ()
        pencil = Pencil (self.seed, prefix)
        detail = self._closeup_columns () if view == "closeup" else None
        # Labels are set smaller in the close-up, which the page shows larger.
        self.label_size = 6.4 if detail else 10
        rough = self._closeup_box () if detail else None
        placed = self._place_labels (routes, rough, detail)
        box = self._view_box (rough, placed, detail) if detail else self._canvas (routes, placed)
        self._draw_mega (pencil)
        self._draw_board (pencil, detail)
        for part in self.parts:
            part.draw (pencil, self)
        for start, end, points in routes:
            self._draw_wire (pencil, start, end, points)
        for module in self.modules.values ():
            module.draw (pencil)
        for text, x, y, anchor, size, to, kind in placed:
            if kind == "note":
                pencil.text (x, y, text, size=size, anchor=anchor, kind="label", italic=True)
                pencil.arrow (to)
            else:
                width = text_width (text, size) - 1
                pencil.label (x, y, text, size=size, anchor=anchor, to=to, width=width,
                              patch=self._patch (x, y, width, size, anchor))
        return pencil.svg (box, self.title if view == "bench" else f"{self.title}: close-up")

    # A measurement: the breadboard round its two probe points, the meter
    # below the board reading what is expected, and its leads rising to the
    # probes. Labels are left to the caption and the table.
    def measure_svg (self, index, prefix="measure"):
        routes = self._layout ()
        taken = self.measurements[index]
        (red, _), (black, _) = self.probes (index)
        points = {"red": self.hole_xy (red), "black": self.hole_xy (black)}
        xs = [x for x, _ in points.values ()]
        bx0, by0, bx1, by1 = self.board_box ()
        width, height = meter.WIDTH * meter.SCALE, meter.HEIGHT * meter.SCALE
        # The meter below the board and to the left of the probes, its leads
        # sweeping up and right into them.
        mx, my = min (xs) - width - 50, by1 + 26
        left, right = mx - 8, max (xs) + 36
        top = min (y for _, y in points.values ()) - 30
        box = (left, top, right - left, my + height + 8 - top)
        self.label_size = 6.4
        detail = (self._column_at (left), self._column_at (right))
        pencil = Pencil (self.seed, f"{prefix}{index}")
        self._draw_mega (pencil)
        self._draw_board (pencil, detail)
        for part in self.parts:
            part.draw (pencil, self)
        for start, end, route in routes:
            self._draw_wire (pencil, start, end, route)
        for module in self.modules.values ():
            module.draw (pencil)
        meter.draw (pencil, mx, my, taken["expect"])
        # The lower jack's lead, the black, runs outside the red one.
        for color, side in (("red", -1), ("black", 1)):
            meter.lead (pencil, meter.jack (mx, my, color), points[color], color, side)
        return pencil.svg (box, f"{self.title}: {taken['label']}")

    # The drawing grows to hold every module, overhanging part, wire and label.
    def _canvas (self, routes, placed):
        width, height = self.size ()
        x0, y0, x1, y1 = 0, 0, width, height
        pad = MARGIN * DPI * 0.7
        boxes = [placed.reach_box () for placed in self.modules.values ()]
        boxes += [placed.title_box () for placed in self.modules.values ()]
        boxes += [part.placed (self).title_box () for part in self.parts
                  if isinstance (part, HeaderModule)]
        boxes += [part.box (self) for part in self.parts if part.box (self)]
        boxes += [(x - 4, y - 4, x + 4, y + 4) for _, _, points in routes for x, y in points]
        boxes += self._placed_boxes
        for bx0, by0, bx1, by1 in boxes:
            if bx0 < x0 + pad:
                x0 = min (x0, bx0 - pad)
            if by0 < y0 + pad:
                y0 = min (y0, by0 - pad)
            if bx1 > x1 - pad:
                x1 = max (x1, bx1 + pad)
            if by1 > y1 - pad:
                y1 = max (y1, by1 + pad)
        return (x0, y0, x1 - x0, y1 - y0)

    # Show these columns in the close-up, for a build too wide to show whole.
    def closeup (self, first, last):
        self.closeup_range = (first, last)
        return self

    # The columns the build uses, leaving out the Mega's power feeds and the
    # rail links, which are the same in every lesson.
    def _closeup_columns (self):
        if getattr (self, "closeup_range", None):
            return self.closeup_range
        standard = {hole for start, end, _, _ in self.wires
                    if self._standard ((start, end)) for kind, hole in (start, end) if kind == "hole"}
        columns = [parse_hole (hole)[1] for hole in self.used if hole not in standard]
        for part in self.parts:
            for shape in part.footprint (self):
                if shape[0] == "rect" and not part.box (self):
                    columns += [self._column_at (shape[1]), self._column_at (shape[3])]
        if not columns:
            return self.first, min (self.last, self.first + CLOSEUP_COLUMNS - 1)
        low, high = max (self.first, min (columns) - 3), min (self.last, max (columns) + 3)
        # At least sixteen columns wide, so parts keep one scale from lesson
        # to lesson.
        while high - low + 1 < CLOSEUP_COLUMNS and (low > self.first or high < self.last):
            if high < self.last:
                high += 1
            if high - low + 1 < CLOSEUP_COLUMNS and low > self.first:
                low -= 1
        return low, high

    def _column_at (self, x):
        bx, _ = self.board_origin ()
        column = round ((x / DPI - bx - 0.25) / 0.1) + self.first
        return min (self.last, max (self.first, column))

    # The close-up before its labels: the columns it shows, and the rows the
    # build uses with room for every part standing in them.
    def _closeup_box (self):
        low, high = self._closeup_columns ()
        left, _ = self.hole_xy (f"a{low}")
        right, _ = self.hole_xy (f"a{high}")
        _, by = self.board_origin ()
        ceiling, floor = by * DPI - 40, (by + BOARD_HEIGHT) * DPI + 25
        ys = [self.hole_xy (hole)[1] for hole in self.used] or [(by + 1.1) * DPI]
        for part in self.parts:
            if any (low <= parse_hole (hole)[1] <= high for _, hole in part.legs ()):
                for shape in part.shapes (self):
                    x0, y0, x1, y1 = shape_bounds (shape)
                    if x1 > left - 10 and x0 < right + 10:
                        ys += [max (ceiling, y0), min (floor, y1)]
        top, bottom = min (ys) - 8, max (ys) + 8
        # Keep the nearer row of column numbers in view.
        top = min (top, (by + 0.34) * DPI) if min (ys) < (by + 1.1) * DPI else top
        bottom = max (bottom, (by + 1.88) * DPI) if max (ys) > (by + 1.1) * DPI else bottom
        # Starting partway along, the row letters stand where the column
        # before would be, just out of view.
        left -= 32 if low == 1 else 8
        right += 32 if high == 63 else 14
        return (left, max (ceiling, top), right, min (floor, bottom))

    # The close-up with its labels, never so flat that it zooms in too far.
    def _view_box (self, rough, placed, detail):
        left, top, right, bottom = rough
        _, by = self.board_origin ()
        ceiling, floor = by * DPI - 40, (by + BOARD_HEIGHT) * DPI + 25
        for box in self._placed_boxes:
            if box[2] > left and box[0] < right:
                top, bottom = min (top, box[1] - 3), max (bottom, box[3] + 3)
                left, right = min (left, box[0] - 3), max (right, box[2] + 3)
        top, bottom = max (ceiling, top), min (floor, bottom)
        width = right - left
        least = 0.62 * width
        if bottom - top < least:
            more = (least - (bottom - top)) / 2
            top, bottom = top - more, bottom + more
            if top < ceiling:
                top, bottom = ceiling, bottom + ceiling - top
            if bottom > floor:
                top, bottom = max (ceiling, top - (bottom - floor)), floor
        return (left, top, width, bottom - top)

    # Labels -----------------------------------------------------------------

    # Every label, placed: the parts' names and values, each wire's pin by
    # the end that matters (the hole, or the Mega for a module), and the
    # notes. Each goes where it covers nothing it shouldn't.
    def _place_labels (self, routes, rough, detail):
        size = self.label_size
        placer = Placer ()
        if rough:
            _, by = self.board_origin ()
            placer.view = (rough[0] - 30, by * DPI - 40, rough[2] + 30, (by + BOARD_HEIGHT) * DPI + 25)
            placer.soft_view = rough
        placer.avoid (("rect", *self.mega_box ()))
        for placed in self.modules.values ():
            placer.avoid (("rect", *placed.reach_box ()))
            placer.taken (placed.title_box ())
        for part in self.parts:
            for shape in part.shapes (self):
                placer.avoid (shape)
        for _, _, points in routes:
            for a, b in zip (points, points[1:]):
                placer.avoid (("segment", a[0], a[1], b[0], b[1], 2.4))
        for hole in self._holes ():
            xy = self.hole_xy (hole)
            if hole in self.used:
                placer.avoid (("circle", xy[0], xy[1], 3.4))
            else:
                placer.point (xy, 3)
        for name in self.taken:
            if name in self.pins:
                placer.avoid (("circle", *self.pin_xy (name), 3.4))
        for x, y, w in self._print_marks (detail):
            placer.avoid (("rect", x - w / 2, y - 6, x + w / 2, y), 150)
        for x0, y0, x1, y1 in (self.board_box (), self.mega_box ()):
            for edge in ((x0, y0, x1, y0), (x0, y1, x1, y1), (x0, y0, x0, y1), (x1, y0, x1, y1)):
                placer.avoid (("segment", *edge, 0.6), 60)
        for box in self._silk_boxes ():
            placer.avoid (("rect", *box), 400)
        for x, y, w in self._board_signs ():
            placer.taken ((x - w / 2, y - 7, x + w / 2, y + 1))

        placed, self._placed_boxes = [], []

        named = []

        def put (label, own):
            text, scale = label.text, label.size
            # One label serves a row of like parts close together, such as a
            # row of 220 Ω resistors.
            if label.at and label.size == 1.0:
                if any (other == text and abs (at[0] - label.at[0]) < 35
                        and abs (at[1] - label.at[1]) < 20 for other, at in named):
                    named.append ((text, label.at))
                    return
                named.append ((text, label.at))
            spots = list (label.spots)
            # The whole bench names a part on the breadboard only where the
            # name fits beside it; the close-up names them all.
            beside = not rough and not part_box (own)
            if label.at and not beside:
                spots += ring (label.at, text, size * scale)
            best = placer.place (text, size * scale, spots, own)
            if best is None or beside and best[0] >= HARD:
                return
            cost, x, y, anchor, box = best
            to = None
            # A leader only when the label stands away from what it names.
            if label.at and (x, y, anchor) not in [s[:3] for s in label.spots]:
                gap = math.hypot (max (box[0] - label.at[0], 0, label.at[0] - box[2]),
                                  max (box[1] - label.at[1], 0, label.at[1] - box[3]))
                to = label.at if gap > 6 else None
            placer.taken (box)
            placed.append ((text, x, y, anchor, size * scale, to, "label"))
            self._placed_boxes.append (box)

        seen = lambda x: not rough or rough[0] - 4 < x < rough[2] + 4
        board = self.board_box ()
        # Whether a part lies off the board, as a module standing in a row does.
        part_box = lambda shapes: any (shape_bounds (shape)[3] > board[3]
                                       or shape_bounds (shape)[1] < board[1]
                                       for shape in shapes if shape[0] == "rect")
        # A row of like resistors in the same two rows is named once, as a
        # parts list names them: "4 × 220 Ω", beside the last of them.
        rows = {}
        resistors = [part for part in self.parts
                     if isinstance (part, Resistor) and seen (part.geometry (self)[0])]
        for part in sorted (resistors, key=lambda part: part.geometry (self)[0]):
            key = (part.value, parse_hole (part.a)[2], parse_hole (part.b)[2])
            groups = rows.setdefault (key, [[]])
            # A gap of more than six columns starts another row.
            if groups[-1] and part.geometry (self)[0] - groups[-1][-1].geometry (self)[0] > 65:
                groups.append ([])
            groups[-1].append (part)
        rows = [(key[0], group) for key, groups in rows.items () for group in groups]
        grouped = {id (part) for _, group in rows if len (group) > 1 for part in group}
        for value, group in rows:
            if len (group) < 2:
                continue
            shapes = [shape for part in group for shape in part.shapes (self)]
            bounds = [shape_bounds (shape) for shape in shapes]
            box = (min (b[0] for b in bounds), min (b[1] for b in bounds),
                   max (b[2] for b in bounds), max (b[3] for b in bounds))
            last = max (group, key=lambda part: part.geometry (self)[0])
            text = f"{len (group)} × {value}"
            put (Label (text, spots_round (box, text, size, "right")[:8] + last.labels (self)[0].spots,
                        last.geometry (self)), shapes)
        for part in self.parts:
            # The whole bench leaves an LED's color to speak for it; the
            # close-up names each one.
            if not rough and isinstance (part, Led):
                continue
            own = part.shapes (self) + [("circle", *self.hole_xy (hole), 3.4)
                                        for _, hole in part.legs ()]
            for label in part.labels (self) if id (part) not in grouped else ():
                if label.at is None or seen (label.at[0]):
                    put (label, own)
        for start, end, points in routes:
            # Named by the end that matters: the hole, or the Mega for a module.
            if start[0] != "pin" or not seen ((points[-1] if end[0] == "hole" else points[0])[0]):
                continue
            text = canonical (start[1])
            text = f"pin {text}" if numbered (text) else text
            spots = along (points if end[0] != "hole" else points[::-1], size * 0.9)
            best = placer.place (text, size * 0.9, spots)
            if best:
                _, x, y, anchor, box = best
                to = next (spot[4] for spot in spots if spot[:3] == (x, y, anchor))
                placer.taken (box)
                placed.append ((text, x, y, anchor, size * 0.9, to, "label"))
                self._placed_boxes.append (box)
        for text, at, offset in self.notes:
            tx, ty = self._point_xy (at)
            if rough and not (rough[0] < tx < rough[2] and rough[1] < ty < rough[3]):
                continue
            note_size = size * 0.95
            spots = [(tx + offset[0] * fx * reach * DPI, ty + offset[1] * fy * reach * DPI, "middle",
                      (reach - 1) * 20 + (fx < 0) * 3 + (fy < 0) * 3)
                     for reach in (1, 1.4, 1.8) for fx in (1, -1) for fy in (1, -1)]
            best = placer.place (text, note_size, spots)
            _, x, y, anchor, box = best
            placer.taken (box)
            placed.append ((text, x, y, anchor, note_size, arrow_to (box, (tx, ty), note_size), "note"))
            self._placed_boxes.append (box)
        return placed

    # A label's paper, widened to cover whole any hole it would cut in half.
    def _patch (self, x, y, width, size, anchor):
        left = {"start": x, "end": x - width}.get (anchor, x - width / 2) - 1.2
        right = left + width + 2.4
        top, bottom = y - size * 0.78, y + size * 0.24
        for hole in self._holes ():
            hx, hy = self.hole_xy (hole)
            if hy + 1.8 < top or hy - 1.8 > bottom:
                continue
            if hx - 1.8 < left < hx + 1.8:
                left = hx - 2.2
            if hx - 1.8 < right < hx + 1.8:
                right = hx + 2.2
        return left, right

    # The Mega's printed pin names, which labels keep clear of.
    def _silk_boxes (self):
        boxes = []
        for name, (px, py) in self.pins.items ():
            hx, hy = self.pin_xy (name)
            length = len (canonical (name)) * 4.2 + 2
            if py > 1.99:
                boxes.append ((hx - 4, hy + 5, hx + 4, hy + 8 + length))
            elif py < 0.11:
                boxes.append ((hx - 4, hy - 8 - length, hx + 4, hy - 5))
            elif px < 3.75:
                boxes.append ((hx - 36, hy - 5, hx - 5, hy + 5))
        return boxes

    # Where the board's own printing is: column numbers, which labels
    # would rather not hide.
    def _print_marks (self, detail=None):
        marks = []
        _, by = self.board_origin ()
        for column in range (self.first, self.last + 1):
            listed = detail and detail[0] <= column <= detail[1]
            if column % 5 == 0 or column == 1 or listed:
                hx, _ = self.hole_xy (f"a{column}")
                w = len (str (column)) * 4.5 + 2
                marks += [(hx, (by + 0.44) * DPI + 1, w), (hx, (by + 1.82) * DPI + 1, w)]
        return marks

    # The row letters and rail signs, which labels never cover.
    def _board_signs (self):
        signs = []
        bx, by = self.board_origin ()
        x = bx * DPI
        if self.first == 1:
            for rail in ("T+", "T-", "B+", "B-"):
                signs.append ((x + 8, (by + ROWS[rail]) * DPI + 3.5, 7))
        for row in "abcdefghij":
            hx, hy = self.hole_xy (f"{row}{self.first}")
            signs.append ((hx - 11, hy + 2.3, 5))
        return signs

    def _point (self, at):
        if isinstance (at, tuple):
            return at[0] * DPI, at[1] * DPI
        return self._point_xy (at)

    def _point_xy (self, at):
        if isinstance (at, tuple):
            return at[0] * DPI, at[1] * DPI
        if at in self.pins:
            return self.pin_xy (at)
        module, _, pin = at.partition (".")
        if pin and module in self.modules:
            placed = self.modules[module]
            return placed.anchor (placed.pin (pin))[0]
        return self.hole_xy (at)

    # Pictures --------------------------------------------------------------

    def _draw_wire (self, pencil, start, end, route):
        lead = next ((e for e in (start, end) if self._style (e) == "lead"), None)
        color = next (c for s, e, c, _ in self.wires if {s, e} == {start, end})
        pencil.wire (route, WIRE_COLORS[color], width=2.4 if lead else 3.6)
        for which, point in ((start, route[0]), (end, route[-1])):
            style = self._style (which)
            if which[0] == "module":
                placed, pin = self._module_pin (which[1])
                (x, y), (dx, dy) = placed.anchor (pin)
                if style == "male":
                    pencil.housing ((x + dx, y + dy), (x + dx * HOUSING, y + dy * HOUSING))
                elif style == "female":
                    pencil.pin_end (x + dx * 4, y + dy * 4)
                elif style == "screw":
                    pencil.tip (x, y)
            elif lead:
                pencil.tip (*point)
            else:
                pencil.pin_end (*point)

    def _draw_mega (self, pencil):
        mx, my = self.mega_origin ()
        x, y, w, h = mx * DPI, my * DPI, MEGA_WIDTH * DPI, MEGA_HEIGHT * DPI
        outline = [(x, y), (x + w, y), (x + w, y + h), (x, y + h)]
        pencil.paper_fill (outline, MEGA_TINT)
        pencil.hatch (x + 3, y + 3, w - 6, h - 6, gap=5.5, angle=35, tone=0.07)
        pencil.rect (x, y, w, h, width=1.0, radius=7, passes=2)
        for hx, hy in ((0.55, 0.1), (0.6, 2.0), (3.3, 0.1), (3.95, 1.55)):
            cx, cy = x + hx * DPI, y + (MEGA_HEIGHT - hy) * DPI
            pencil.paper_fill ([(cx + 4.5 * math.cos (a / 8 * math.pi),
                                 cy + 4.5 * math.sin (a / 8 * math.pi))
                                for a in range (16)], "#fbf9f3")
            pencil.circle (cx, cy, 4.5, width=0.7)
            pencil.circle (cx, cy, 6.8, width=0.4, tone=0.4)

        # The USB socket and the power jack stick out of the left edge.
        ux, uy, uw, uh = x - 0.25 * DPI, y + 0.28 * DPI, 0.85 * DPI, 0.5 * DPI
        pencil.paper_fill ([(ux, uy), (ux + uw, uy), (ux + uw, uy + uh), (ux, uy + uh)], "#d9d8d3")
        pencil.hatch (ux, uy, uw, uh, gap=2.2, angle=0, tone=0.12)
        pencil.rect (ux, uy, uw, uh, width=0.9)
        pencil.rect (ux + 4, uy + 10, 14, uh - 20, width=0.6, tone=0.6)
        pencil.text (x + 0.2 * DPI, y + 0.2 * DPI, "USB", size=8, kind="silk", tone=0.6)
        jx, jy, jw, jh = x - 0.12 * DPI, y + 1.55 * DPI, 0.7 * DPI, 0.42 * DPI
        pencil.fill ([(jx, jy), (jx + jw, jy), (jx + jw, jy + jh), (jx, jy + jh)], tone=0.68)
        pencil.rect (jx, jy, jw, jh, width=0.9, radius=3)
        pencil.circle (jx + 14, jy + jh / 2, 8, width=0.6, tone=0.5)
        pencil.circle (jx + 14, jy + jh / 2, 2.5, width=0.6, tone=0.5)

        # The microcontroller sits turned 45 degrees, as on the real board: a
        # 100-pin package with a pin-1 dot and its legend.
        cx, cy, r = x + 2.4 * DPI, y + 1.05 * DPI, 0.36 * DPI
        diamond = [(cx, cy - r), (cx + r, cy), (cx, cy + r), (cx - r, cy)]
        for (ax, ay), (bx, by) in zip (diamond, diamond[1:] + diamond[:1]):
            for step in range (1, 25):
                t = step / 25
                px, py = ax + (bx - ax) * t, ay + (by - ay) * t
                ox, oy = (px - cx) / r * 3.2, (py - cy) / r * 3.2
                pencil.line ((px, py), (px + ox, py + oy), width=0.5, tone=0.55, wobble=0.05)
        pencil.fill (diamond, tone=0.7)
        pencil.polyline (diamond, width=0.8, closed=True)
        pencil.disc (cx, cy - r + 7, 1.6, tone=0.35)
        pencil.text (cx, cy + 3, "ATMEGA2560", size=6.5, kind="silk", rotate=-45,
                     color="#e9e6de", tone=0.85)
        pencil.text (x + 1.25 * DPI, y + 1.12 * DPI, "ARDUINO", size=12, weight="bold",
                     tone=0.5, kind="silk")
        pencil.text (x + 1.25 * DPI, y + 1.28 * DPI, "MEGA 2560", size=12, weight="bold",
                     tone=0.5, kind="silk")

        # The built-in LED, L, on pin 13.
        lx, ly = x + 1.2 * DPI, y + 0.45 * DPI
        pencil.rect (lx - 3.5, ly - 2, 7, 4, width=0.6)
        pencil.text (lx + 9, ly + 3, "L", size=7, anchor="start", kind="silk", tone=0.7)

        self._draw_headers (pencil)

    def _draw_headers (self, pencil):
        groups = {}
        for name, (px, py) in self.pins.items ():
            key = "top" if py > 1.99 else "bottom" if py < 0.11 else "side"
            gap_key = (key, "side" if key == "side" else block_of (px, key))
            groups.setdefault (gap_key, []).append (name)
        for names in groups.values ():
            points = [self.pin_xy (n) for n in names]
            left = min (p[0] for p in points) - 5
            top = min (p[1] for p in points) - 5
            right = max (p[0] for p in points) + 5
            bottom = max (p[1] for p in points) + 5
            pencil.fill ([(left, top), (right, top), (right, bottom), (left, bottom)], tone=0.72)
            pencil.rect (left, top, right - left, bottom - top, width=0.6)

        for name, (px, py) in self.pins.items ():
            hx, hy = self.pin_xy (name)
            pencil.socket (hx, hy)
            label = canonical (name)
            weight, tone = ("bold", 0.95) if name in self.taken else ("normal", 0.7)
            common = dict (size=7, kind="silk", halo=MEGA_TINT, weight=weight, tone=tone)
            if py > 1.99:
                pencil.text (hx + 2.4, hy + 8, label, rotate=-90, anchor="end", **common)
            elif py < 0.11:
                pencil.text (hx + 2.4, hy - 8, label, rotate=-90, anchor="start", **common)
        # The double header's names both sit on its inner side, away from
        # the wires that leave it to the right: the left pin's name, then the
        # right pin's.
        for name, (px, py) in self.pins.items ():
            if not 0.11 < py < 1.99 or px > 3.75:
                continue
            odd = next (n for n, (qx, qy) in self.pins.items () if abs (qy - py) < 0.01 and qx > 3.75)
            hx, hy = self.pin_xy (name)
            pair = [(canonical (odd), odd, 0),
                    (canonical (name), name, len (canonical (odd)) * 3.9 + 3.5)]
            if canonical (odd) == canonical (name):
                # Both 5V, or both GND: one name for the row.
                pair = [(canonical (name), name if name in self.taken else odd, 0)]
            for label, pin, offset in pair:
                weight, tone = ("bold", 0.95) if pin in self.taken else ("normal", 0.7)
                pencil.text (hx - 7 - offset, hy + 2.5, label, size=7, anchor="end", kind="silk",
                             halo=MEGA_TINT, weight=weight, tone=tone)

    def _draw_board (self, pencil, detail=None):
        bx, by = self.board_origin ()
        x, y = bx * DPI, by * DPI
        w, h = self.board_width () * DPI, BOARD_HEIGHT * DPI
        torn_left, torn_right = self.first > 1, self.last < 63
        outline = board_outline (x, y, w, h, torn_left, torn_right, pencil.random)
        pencil.paper_fill (outline, "#ffffff")
        pencil.polyline (outline, width=1.0, closed=True, passes=2, layer="crisp")
        # The channel down the middle, where chips straddle.
        pencil.fill ([(x + 6, y + 1.05 * DPI), (x + w - 6, y + 1.05 * DPI),
                      (x + w - 6, y + 1.15 * DPI), (x + 6, y + 1.15 * DPI)], tone=0.07)
        board = dict (kind="silk", halo="#ffffff", layer="crisp")
        for rail, offset in (("T+", 0.06), ("T-", 0.34), ("B+", 1.86), ("B-", 2.14)):
            color = "#c0564e" if rail.endswith ("+") else "#4f74ad"
            pencil.stripe ((x + 14, y + offset * DPI), (x + w - 14, y + offset * DPI), color)
            sign = "+" if rail.endswith ("+") else "−"
            hy = (by + ROWS[rail]) * DPI
            if not torn_left:
                pencil.text (x + 8, hy + 3.5, sign, size=9, tone=0.7, **board)
            if not torn_right:
                pencil.text (x + w - 8, hy + 3.5, sign, size=9, tone=0.7, **board)
        for column in range (self.first, self.last + 1):
            for row in "abcdefghij":
                pencil.hole (*self.hole_xy (f"{row}{column}"))
            if rail_column (column):
                for rail in ("T+", "T-", "B+", "B-"):
                    pencil.hole (*self.hole_xy (f"{rail}{column}"))
            listed = detail and detail[0] <= column <= detail[1]
            if column % 5 == 0 or column == 1 or listed:
                hx, _ = self.hole_xy (f"a{column}")
                size = 5.5 if listed and column % 5 else 7
                pencil.text (hx, y + 0.44 * DPI, str (column), size=size, tone=0.7, **board)
                pencil.text (hx, y + 1.82 * DPI, str (column), size=size, tone=0.7, **board)
        # Row letters in the board's margins, as printed; in a close-up that
        # starts partway along, between the first two columns in view.
        edges = [(self.first, -11)] + ([] if torn_right else [(self.last, 11)])
        if detail and detail[0] > self.first:
            edges.append ((detail[0], -5))
        for column, offset in edges:
            for row in "abcdefghij":
                hx, hy = self.hole_xy (f"{row}{column}")
                pencil.text (hx + offset, hy + 2.3, row, size=6.5, tone=0.7, **board)


THEME = os.path.dirname (os.path.abspath (__file__))


# Where routes are kept: beside the build's Python cache, which the Makefile
# puts in the build directory; nowhere when there is none.
def route_cache ():
    prefix = os.environ.get ("PYTHONPYCACHEPREFIX")
    return os.path.join (os.path.dirname (prefix), "routes") if prefix else None


def board_outline (x, y, w, h, torn_left, torn_right, random):
    def edge (x0, top_to_bottom):
        points = []
        steps = 14
        for index in range (steps + 1):
            t = index / steps
            jag = random.uniform (-4, 4) if 0 < index < steps else 0
            points.append ((x0 + jag, y + h * (t if top_to_bottom else 1 - t)))
        return points

    right = edge (x + w, True) if torn_right else [(x + w, y), (x + w, y + h)]
    left = edge (x, False) if torn_left else [(x, y + h), (x, y)]
    return [(x, y)] + right + left[:-1] if not torn_left else right + left


def block_of (px, key):
    # Which physical header block a top or bottom pin sits in.
    if key == "top":
        return 0 if px < 1.8 else 1 if px < 2.7 else 2
    return 0 if px < 1.8 else 1 if px < 2.65 else 2


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


# The rail hole in this column, or the nearest one after it.
def rail_near (column):
    while not rail_column (column):
        column += 1
    return column


def canonical (name):
    return re.sub (r"^(GND|5V)\d$", r"\1", name)


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


def distance (a, b):
    return ((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2) ** 0.5


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


def shape_bounds (shape):
    if shape[0] == "rect":
        return shape[1:5]
    if shape[0] == "circle":
        _, cx, cy, r = shape
        return cx - r, cy - r, cx + r, cy + r
    _, ax, ay, bx, by, half = shape
    return min (ax, bx) - half, min (ay, by) - half, max (ax, bx) + half, max (ay, by) + half


# Whether the line from a to b passes over a shape, away from its ends.
def shape_crosses (shape, a, b):
    length = max (1e-9, math.dist (a, b))
    for step in range (1, int (length / 2)):
        t = step * 2 / length
        p = (a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t)
        x0, y0, x1, y1 = shape_bounds (shape)
        if not (x0 - 2 < p[0] < x1 + 2 and y0 - 2 < p[1] < y1 + 2):
            continue
        if shape[0] == "rect" or shape[0] == "circle" and \
                math.hypot (p[0] - shape[1], p[1] - shape[2]) < shape[3] + 2:
            return True
        if shape[0] == "segment":
            if segment_distance (shape[1:3], shape[3:5], p) < shape[5] + 2:
                return True
    return False


def straight_nodes (a, b):
    steps = max (abs (b[0] - a[0]), abs (b[1] - a[1]))
    return [(a[0] + (b[0] - a[0]) * k // steps, a[1] + (b[1] - a[1]) * k // steps)
            for k in range (steps + 1)]


def grow (box, margin):
    return box[0] - margin, box[1] - margin, box[2] + margin, box[3] + margin


def overlaps (a, b):
    return a[0] < b[2] and b[0] < a[2] and a[1] < b[3] and b[1] < a[3]


# The point a given distance along a polyline, and the way it runs there.
def point_along (points, along):
    for a, b in zip (points, points[1:]):
        length = math.dist (a, b)
        if length and along <= length:
            t = along / length
            return (a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t), \
                ((b[0] - a[0]) / length, (b[1] - a[1]) / length)
        along -= length
    a, b = points[-2], points[-1]
    length = max (1e-9, math.dist (a, b))
    return b, ((b[0] - a[0]) / length, (b[1] - a[1]) / length)


# Spots for a wire's label beside it, from its labelled end outwards: above
# or below a level run, right or left of an upright one. Each ends with the
# point on the wire its leader goes to.
def along (points, size):
    spots = []
    total = sum (math.dist (a, b) for a, b in zip (points, points[1:]))
    for out in range (10, int (min (total - 6, 200)), 5):
        (px, py), (dx, dy) = point_along (points, out)
        cost = out * 0.35
        # Beside the wire, or further out on a longer leader when crowded.
        for reach, extra in ((1, 0), (2.4, 30), (4, 60), (7, 90), (10, 120)):
            if abs (dx) >= abs (dy):
                spots += [(px, py - 4.5 * reach - size * 0.26, "middle", cost + extra, (px, py)),
                          (px, py + 4.5 * reach + size * 0.8, "middle", cost + extra + 1, (px, py))]
            else:
                spots += [(px + 5 * reach, py + size * 0.3, "start", cost + extra, (px, py)),
                          (px - 5 * reach, py + size * 0.3, "end", cost + extra + 1, (px, py))]
    return spots


# Spots further out round a point, each with a leader back to it, for a
# label crowded out of its own.
def ring (at, text, size):
    spots = []
    for reach in (16, 24, 34, 46, 60):
        for step in range (16):
            angle = step * math.pi / 8
            x, y = at[0] + math.cos (angle) * reach * 1.4, at[1] + math.sin (angle) * reach
            spots.append ((x, y + size * 0.3, "middle", 40 + reach * 1.2, at))
    return spots


# An arrow's curve from a note's box to what it points at.
def arrow_to (box, target, size):
    x0, y0, x1, y1 = box
    lx, ly = (x0 + x1) / 2, (y0 + y1) / 2
    tx, ty = target
    sx = x1 + 3 if tx > x1 else x0 - 3 if tx < x0 else lx
    sy = ly if sx != lx else (y1 + 2 if ty > ly else y0 - 2)
    dx, dy = tx - sx, ty - sy
    length = max (1.0, math.hypot (dx, dy))
    ex, ey = tx - dx / length * 4, ty - dy / length * 4
    bend = 0.18 * length
    mid = ((sx + ex) / 2 - dy / length * bend, (sy + ey) / 2 + dx / length * bend)
    return [(sx, sy), mid, (ex, ey)]
