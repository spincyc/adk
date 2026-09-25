"""The bench: an Arduino Mega 2560 beside a breadboard, with parts standing in
real holes, modules beside the board, and jumper wires from real header pins.

A lesson describes its build once, in circuit.py beside its page:

    bench = Bench ("An LED on pin 26", columns=(1, 30))
    bench.wire ("26", "a1")
    bench.resistor ("220 Ω", "b1", "b5")
    bench.led ("red", anode="c5", cathode="c6")
    bench.wire ("a6", "B-6")
    bench.wire ("GND", "B-3")

Builds start at column 1, the end nearest the Mega, and place parts left to
right in the order current meets them; next_column () hands out the first
free column.

Parts stand in the breadboard: resistor, led, button, rgb_led, buzzer,
potentiometer, photoresistor, thermistor, tilt_switch, chip (a DIP chip
across the middle gap), digit and four_digits (seven-segment displays), lcd,
header_module (a module standing in one row) and power_module. Modules sit
beside the board, placed in inches in the drawing's own coordinates, where
the Mega's top-left corner is (0.35, 0.6) and the breadboard's is (5.15,
0.55); a wire reaches a module pin as "name.PIN", as in
bench.wire ("44", "servo.signal"). note () adds a small annotation with an
arrow. closeup (first, last) picks the columns the close-up shows, for a
build too wide to show whole.

From that one description come the pencil drawings, the build steps and the
table of connections, and signal_pins () lets the site check the sketch
declares exactly the pins the build wires. Geometry is in inches on the real
0.1-inch grid: holes, header pins and leg spacing are where they really are.
"""

import heapq
import math
import re

from modules import HOUSING, Placed, make as make_module
from parts import (Button, Buzzer, Chip, Display, HeaderModule, Led, PowerModule, Potentiometer,
                   Resistor, RgbLed, TwoLegs, bands_for)
from pencil import DPI, Pencil

MEGA_WIDTH, MEGA_HEIGHT = 4.0, 2.1
BOARD_HEIGHT = 2.2
GAP = 0.8                           # between the Mega and the breadboard
MARGIN = 0.35
MEGA_TINT = "#dfe6e8"
CLOSEUP_COLUMNS = 16                # the narrowest close-up, so parts keep one scale

WIRE_COLORS = {
    "red": "#be4c44", "black": "#3a3a3a", "blue": "#4a72ad", "green": "#56874f",
    "yellow": "#d3ae3f", "orange": "#cf8243", "white": "#ece8de", "purple": "#7a63a0",
    "brown": "#86613f", "grey": "#9a9a98",
}
SIGNAL_COLORS = ["yellow", "green", "blue", "orange", "purple", "white", "brown", "grey"]
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
        self.steps = []
        self.used = {}
        self.gap = GAP
        self.palette = iter (SIGNAL_COLORS * 8)

    # Where things are ---------------------------------------------------

    def board_width (self):
        return 0.5 + (self.last - self.first) * 0.1

    def mega_origin (self):
        return MARGIN, MARGIN + 0.25

    def board_origin (self):
        mx, my = self.mega_origin ()
        return mx + MEGA_WIDTH + self.gap, my + (MEGA_HEIGHT - BOARD_HEIGHT) / 2

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
        self.steps.append (f"The {value} resistor ({', '.join (bands_for (value))}) "
                           f"from {a} to {b}.")
        return self

    def led (self, color, anode, cathode):
        self._add (Led (color, anode, cathode))
        self.steps.append (f"The {color} LED: long leg in {anode}, short leg in {cathode}.")
        return self

    def button (self, column):
        part = Button (column)
        self._add (part)
        self.steps.append (f"A push button across the middle gap, legs in "
                           f"{', '.join (part.holes ())}.")
        return self

    def rgb_led (self, red, common, green, blue):
        self._add (RgbLed (red, common, green, blue))
        self.steps.append (f"The RGB LED: red leg in {red}, the longest leg (common, −) in {common}, "
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
        self.steps.append (f"The {kind} buzzer, its + mark and longer leg in {positive}, the other "
                           f"leg in {negative}.")
        return self

    def potentiometer (self, left, wiper, right, value="10 kΩ"):
        spots = [parse_hole (hole) for hole in (left, wiper, right)]
        steps = {spots[1][1] - spots[0][1], spots[2][1] - spots[1][1]}
        if len ({row for _, _, row in spots}) != 1 or len (steps) != 1 or steps - {1, 2}:
            raise ValueError ("a potentiometer's three legs stand in one row, evenly spaced")
        self._add (Potentiometer (left, wiper, right, value))
        self.steps.append (f"The {value} potentiometer, legs in {left}, {wiper} and {right}: the "
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
        self.steps.append (f"The {kind}, legs in {a} and {b}, either way round.")
        return self

    # A DIP chip across the middle gap: pin 1 in e<first>, pins running
    # right along row e and back along row f (or d and h for a 0.6 inch
    # package).
    def chip (self, name, pins=None, first=1, span=3):
        pins = list (pins or CHIPS[name])
        holes, rows = self._straddle (len (pins), first, span)
        self._add (Chip (name, pins, holes))
        half = len (pins) // 2
        self.steps.append (
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
        self.steps.append (
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
        self.steps.append (
            f"The LCD, face up, its screen lying over the {edge} edge of the board"
            f"{'' if part.turned else ' (upside down from where you sit)'} and its 16 pins in "
            f"{part.holes[0]}–{part.holes[-1]}: {pins}.")
        return self

    # A module standing in one row on its own header, pins listed left to
    # right as they meet the columns.
    def header_module (self, kind, pins=None, first=1, row="j", name=None, label=None, **options):
        part = self._header_part (kind, pins, first, row, name, label, options)
        pins = ", ".join (f"{pin.name} in {hole}" for pin, hole in part.pairs ())
        edge = "bottom" if part.turned else "top"
        self.steps.append (f"The {part.name}, standing in row {row} with its board toward the "
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
    # both sides set to 5 V: wires to the rails take their power from it.
    def power_module (self, end="left"):
        if end not in ("left", "right"):
            raise ValueError ("the power module goes on the left or right end")
        columns = (3, 4) if end == "left" else (60, 61)
        if not (self.first <= columns[0] and columns[1] <= self.last):
            raise ValueError (f"the power module needs columns {columns[0]}-{columns[1]} drawn")
        if end == "left":
            # Its jack hangs over the end, so the board moves over for it.
            self.gap = GAP + 1.3
        part = PowerModule (end, columns)
        self._add (part)
        self.steps.append (
            f"The power module on the {end} end of the board, its pins in both pairs of rails "
            f"(T+{columns[0]}, T-{columns[0]}, B+{columns[0]}, B-{columns[0]} and the holes beside "
            f"them), both jumpers set to 5 V.")
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
        bx, by = self.board_origin ()
        things = {"the Mega": ((mx - 0.25) * DPI, my * DPI, (mx + MEGA_WIDTH) * DPI,
                               (my + MEGA_HEIGHT) * DPI),
                  "the breadboard": (bx * DPI, by * DPI, (bx + self.board_width ()) * DPI,
                                     (by + BOARD_HEIGHT) * DPI)}
        things.update ({f"the {other.title}": other.reach_box () for other in self.modules.values ()})
        for thing, box in things.items ():
            if overlaps (placed.reach_box (), grow (box, 8)):
                raise ValueError (f"the {placed.title} at {at} overlaps {thing}")
        self.modules[name] = placed
        self.steps.append (f"The {placed.title}, {self._whereabouts (placed.box ())}.")
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

    # A small annotation: text set off from what it points at (a hole, a pin,
    # "module.PIN" or (x, y) inches) by offset inches, with an arrow.
    def note (self, text, at, offset=(-0.45, -0.35)):
        self.notes.append ((text, at, offset))
        return self

    # Wires ---------------------------------------------------------------

    def wire (self, start, end, color=None):
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
        self.wires.append ((start_end, end_end, color))
        self.steps.append (self._wire_step (start_end, end_end, color))
        return self

    def _wire_step (self, start, end, color):
        for lead, other in ((start, end), (end, start)):
            if self._style (lead) == "lead":
                placed, pin = self._module_pin (lead[1])
                return f"The {placed.title}'s {pin.note} into {self.describe (other)}."
        males = sum (self._style (end) == "male" for end in (start, end))
        jumper = ("", "female-to-male ", "female-to-female ")[males]
        article = "An" if (jumper or color)[0] in "aeiou" else "A"
        return f"{article} {color} {jumper}wire from {self.describe (start)} to {self.describe (end)}."

    def _color (self, ends):
        kinds = {self._polarity (end) for end in ends}
        if "ground" in kinds:
            return "black"
        if "power" in kinds:
            return "red"
        return next (self.palette)

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
            free = [p for p in ALIASES[name] if p not in self.taken]
            if not free:
                raise ValueError (f"no free {name} pin")
            if toward:
                free.sort (key=lambda p: distance (self.pin_xy (p), toward))
            self.taken.add (free[0])
            return ("pin", free[0])
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
        for start, end, _ in self.wires:
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
                if len ([m for m in net if not m.startswith (("column ", "rail "))]) >= 2]
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
        return {canonical (name) for (kind, name), _, _ in self.wires if kind == "pin"
                if canonical (name) not in ("GND", "5V", "3.3V", "VIN")} | \
               {canonical (name) for _, (kind, name), _ in self.wires if kind == "pin"
                if canonical (name) not in ("GND", "5V", "3.3V", "VIN")}

    # Drawing ------------------------------------------------------------

    # The whole bench, or a close-up of the breadboard where the parts are.
    def svg (self, view="bench", prefix="bench"):
        pencil = Pencil (self.seed, prefix)
        detail = self._closeup_columns () if view == "closeup" else None
        # Labels are set smaller in the close-up, which the page shows larger.
        self.label_size = 6.4 if detail else 10
        self.view = self._closeup_box () if detail else None
        self.labels, self._tags = [], []
        self._draw_mega (pencil)
        self._draw_board (pencil, detail)
        for part in self.parts:
            part.draw (pencil, self)
        routes = self._routes ()
        for (start, end, color), route in zip (self.wires, routes):
            self._draw_wire (pencil, start, end, color, route)
        for placed in self.modules.values ():
            placed.draw (pencil)
        for text, at, offset in self.notes:
            self._draw_note (pencil, text, at, offset)
        box = self._canvas (routes) if view == "bench" else self._closeup_box ()
        return pencil.svg (box, self.title if view == "bench" else f"{self.title}: close-up")

    # The drawing grows to hold every module, overhanging part, wire and note.
    def _canvas (self, routes):
        width, height = self.size ()
        x0, y0, x1, y1 = 0, 0, width, height
        pad = MARGIN * DPI * 0.7
        boxes = [placed.reach_box () for placed in self.modules.values ()]
        boxes += [placed.title_box () for placed in self.modules.values ()]
        boxes += [part.placed (self).title_box () for part in self.parts
                  if isinstance (part, HeaderModule)]
        boxes += [part.box (self) for part in self.parts if part.box (self)]
        boxes += [(x - 4, y - 4, x + 4, y + 4) for route in routes for x, y in route]
        boxes += [(x - w / 2, y - 12, x + w / 2, y + 4) for x, y, w in self.labels]
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

    def _closeup_columns (self):
        if getattr (self, "closeup_range", None):
            return self.closeup_range
        columns = [parse_hole (hole)[1] for hole in self.used]
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

    # The close-up keeps the columns and rows the build uses, with room for
    # standing parts, labels and the column numbers, and is never so small
    # that it zooms in too far.
    def _closeup_box (self):
        low, high = self._closeup_columns ()
        left, _ = self.hole_xy (f"a{low}")
        right, _ = self.hole_xy (f"a{high}")
        _, by = self.board_origin ()
        ys = [self.hole_xy (hole)[1] for hole in self.used] or [(by + 1.1) * DPI]
        top = max (by * DPI - 30, min (ys) - 0.55 * DPI)
        bottom = min ((by + BOARD_HEIGHT) * DPI + 15, max (ys) + 0.35 * DPI)
        # Keep the nearer row of column numbers in view.
        top = min (top, (by + 0.34) * DPI) if min (ys) < (by + 1.1) * DPI else top
        bottom = max (bottom, (by + 1.88) * DPI) if max (ys) > (by + 1.1) * DPI else bottom
        left -= 32 if low == 1 else 14
        right += 32 if high == 63 else 14
        width = right - left
        least = 0.62 * width
        if bottom - top < least:
            grow = (least - (bottom - top)) / 2
            ceiling, floor = by * DPI - 40, (by + BOARD_HEIGHT) * DPI + 25
            top, bottom = top - grow, bottom + grow
            if top < ceiling:
                top, bottom = ceiling, bottom + ceiling - top
            if bottom > floor:
                top, bottom = max (ceiling, top - (bottom - floor)), floor
        return (left, top, width, bottom - top)

    # Routing ---------------------------------------------------------------

    def _routes (self):
        obstacles = self._obstacles ()
        self.channels = {"above": 0, "below": 0}
        # Tall bodies on the board, such as a display or a buzzer: wires lie
        # over the board, but go round these.
        self.bodies = []
        self.clear_of = obstacles[1:]
        for part in self.parts:
            if part.box (self):
                continue
            for shape in part.footprint (self):
                if shape[0] == "rect":
                    self.bodies.append (shape[1:])
                else:
                    _, cx, cy, r = shape
                    self.bodies.append ((cx - r, cy - r, cx + r, cy + r))
        return [self._route (start, end, obstacles, index)
                for index, (start, end, _) in enumerate (self.wires)]

    def _obstacles (self):
        mx, my = self.mega_origin ()
        boxes = [((mx - 0.25) * DPI, my * DPI, (mx + MEGA_WIDTH) * DPI, (my + MEGA_HEIGHT) * DPI)]
        boxes += [placed.box () for placed in self.modules.values ()]
        boxes += [part.box (self) for part in self.parts if part.box (self) and part.blocks]
        return boxes

    # A wire's path: from its first end (a Mega pin when it has one), out
    # the way its pin points, round anything in the way, and into its other
    # end.
    def _route (self, start, end, obstacles, index):
        if end[0] == "pin" and start[0] != "pin" or start[0] == "hole" and end[0] == "module":
            start, end = end, start
        a, b = self._xy (start), self._xy (end)
        lift = 0.3 * DPI
        if start[0] == "hole" and end[0] == "hole":
            # A short jumper between holes bows out to one side.
            dx, dy = b[0] - a[0], b[1] - a[1]
            length = max (1.0, (dx * dx + dy * dy) ** 0.5)
            bow = min (0.09 * DPI, length * 0.3)
            middle = ((a[0] + b[0]) / 2 + dy / length * bow, (a[1] + b[1]) / 2 - dx / length * bow)
            return [a, middle, b]
        first, out_a = self._terminal (start)
        last, out_b = self._terminal (end)
        exit_a = self._leave (start[1], first, lift) if start[0] == "pin" else \
            (first[0] + out_a[0] * lift, first[1] + out_a[1] * lift)
        head = [a] + ([first] if first != a else []) + [exit_a]
        tail = [last] + ([b] if b != last else [])
        if out_b:
            middle = [(last[0] + out_b[0] * lift, last[1] + out_b[1] * lift)]
        else:
            middle = self._approach (exit_a, end[1], start[0] == "pin" and out_a == (1, 0))
        # Round anything in the way, a segment at a time; a wire that doesn't
        # plug into the breadboard goes round that too.
        margin = 10 + 5 * (index % 3)
        boxes = obstacles + [grow (body, 3 - margin) for body in self.bodies]
        if end[0] != "hole":
            bx, by = self.board_origin ()
            boxes.append ((bx * DPI, by * DPI, (bx + self.board_width ()) * DPI,
                           (by + BOARD_HEIGHT) * DPI))
        path = head[-1:] + middle
        routed = [path[0]]
        for p, q in zip (path, path[1:]):
            routed += detour (p, q, boxes, margin) + [q]
        return head[:-1] + routed + tail

    # From where a wire leaves its first end to just short of a hole. Wires
    # from the side come across the gap, along a rail into a rail's hole.
    # Wires from above or below travel in a lane beside the board, each in
    # its own, and drop into the half of the board on their side; for the
    # other half, or a hole with something standing over it, they come in
    # from the gap at the Mega's end.
    def _approach (self, exit, name, sideways):
        hole = self.hole_xy (name)
        bx, by = self.board_origin ()
        left, top, bottom = bx * DPI, by * DPI, (by + BOARD_HEIGHT) * DPI
        lift = 0.3 * DPI
        kind, _, row = parse_hole (name)
        half = "above" if row in "fghij" or row.startswith ("T") else "below"
        spots = [(hole[0] - lift, hole[1])] if kind == "rail" else []
        spots += [(hole[0] - lift * 0.5, hole[1] - lift), (hole[0] - lift * 0.5, hole[1] + lift),
                  (hole[0] - lift, hole[1]), (hole[0] + lift * 0.5, hole[1] - lift)]
        if sideways or top < exit[1] < bottom:
            return [self._clear_spot (spots, hole)]
        side = "above" if exit[1] <= top else "below"
        lane = self.channels[side]
        self.channels[side] += 1
        sign = -1 if side == "above" else 1
        y = (min if side == "above" else max) (exit[1], top - 16 if side == "above" else bottom + 16)
        y += sign * 6 * lane
        drop = (hole[0] - 6, (max if sign < 0 else min) (y, hole[1] + sign * lift))
        if (side == half or exit[0] > left) and self._clear_spot ([drop], hole, None):
            return [(exit[0], y), (hole[0] - 12, y), drop]
        gap = left - 0.2 * DPI - 5 * lane
        spots.sort (key=lambda spot: spot[1] * -sign)
        return [(exit[0], y), (gap, y), self._clear_spot (spots, hole)]

    # The first spot from which a wire drops into the hole clear of any tall
    # body, and of anything lying off the board's edge.
    def _clear_spot (self, spots, hole, fallback=True):
        boxes = self.bodies + self.clear_of
        for spot in spots:
            if not any (within (spot, grow (box, 3)) for box in boxes) and \
                    all (misses (spot, hole, grow (box, 1)) for box in boxes):
                return spot
        return spots[0] if fallback else None

    def _terminal (self, end):
        kind, name = end
        if kind == "pin":
            _, y = self.pins[name]
            return self.pin_xy (name), (0, -1) if y > 1.99 else (0, 1) if y < 0.11 else (1, 0)
        if kind == "hole":
            return self.hole_xy (name), None
        placed, pin = self._module_pin (name)
        (x, y), out = placed.anchor (pin)
        reach = {"male": HOUSING, "female": 4, "screw": 0, "lead": 0}[pin.style]
        return (x + out[0] * reach, y + out[1] * reach), out

    # A point a little way out from a header, so a wire leaves it cleanly.
    def _leave (self, name, xy, lift):
        _, y = self.pins[name]
        if y > 1.99:
            return xy[0], xy[1] - lift
        if y < 0.11:
            return xy[0], xy[1] + lift
        return xy[0] + lift * 1.4, xy[1]

    # Pictures --------------------------------------------------------------

    def _draw_wire (self, pencil, start, end, color, route):
        if end[0] == "pin" and start[0] != "pin" or start[0] == "hole" and end[0] == "module":
            start, end = end, start
        lead = next ((e for e in (start, end) if self._style (e) == "lead"), None)
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
        if start[0] == "pin":
            label = canonical (start[1])
            label = f"pin {label}" if numbered (label) else label
            self._label_wire (pencil, route if end[0] != "hole" else route[::-1], label)

    # A part's label, at this view's label size, which wire labels keep
    # clear of. A label that repeats one just beside it (a row of 220 Ω
    # resistors) is left out; one that would overlap another moves up a
    # line, or is left out when optional.
    def tag (self, pencil, x, y, text, size=1.0, to=None, optional=False):
        size = self.label_size * size
        width = len (text) * size * 0.52 + 2
        if any (t == text and abs (x - lx) < 40 and abs (y - ly) < 30
                for lx, ly, w, t in self._tags):
            return
        clash = lambda y: any (abs (x - lx) < (width + w) / 2 + 1 and abs (y - ly) < size + 1
                               for lx, ly, w in self.labels)
        for lift in (0, 1, 2):
            if not clash (y - lift * (size + 1)):
                y -= lift * (size + 1)
                break
        else:
            if optional:
                return
        pencil.label (x, y, text, size=size, to=to)
        self.labels.append ((x, y, width))
        self._tags.append ((x, y, width, text))

    # A wire's name beside it, near the end that matters: the hole, or the
    # Mega when it goes to a module. It sits clear of other labels, with a
    # fine leader to the wire.
    def _label_wire (self, pencil, route, text):
        size = self.label_size * 0.9
        width = len (text) * size * 0.52 + 2
        spots = []
        for along in range (26, 150, 12):
            point, (dx, dy) = point_along (route, along)
            normal = (dy, -dx) if dx > 0 else (-dy, dx)
            for side in (1, -1):
                cx = point[0] + normal[0] * side * (width / 2 + 3)
                cy = point[1] + normal[1] * side * (size * 0.8 + 2) + size * 0.35
                spots.append ((self._label_cost (cx, cy, width, size) + along * 0.4, cx, cy, point))
        _, cx, cy, point = min (spots)
        self.labels.append ((cx, cy, width))
        pencil.label (cx, cy, text, size=size, to=point)

    # How badly a label there would sit: over another label, out of view,
    # or over holes in use or holes at all.
    def _label_cost (self, cx, cy, width, size):
        box = (cx - width / 2 - 1, cy - size * 0.8, cx + width / 2 + 1, cy + size * 0.25)
        cost = 0
        for x, y, w in self.labels:
            if abs (cx - x) < (width + w) / 2 + 2 and abs (cy - y) < size + 1:
                cost += 1000
        if self.view:
            vx, vy, vw, vh = self.view
            if box[0] < vx + 2 or box[2] > vx + vw - 2 or box[1] < vy + 2 or box[3] > vy + vh - 2:
                cost += 500
        for hole in self._holes ():
            hx, hy = self.hole_xy (hole)
            if box[0] - 2 < hx < box[2] + 2 and box[1] - 2 < hy < box[3] + 2:
                cost += 40 if hole in self.used else 8
        for part in self.parts:
            for shape in part.footprint (self):
                if shape[0] == "rect" and shape[1] < box[2] and box[0] < shape[3] \
                        and shape[2] < box[3] and box[1] < shape[4]:
                    cost += 60
        return cost

    def _draw_note (self, pencil, text, at, offset):
        tx, ty = self._point (at)
        size = self.label_size * 0.95
        width = len (text) * size * 0.5
        # Where it was asked for, or mirrored round its point, or further out,
        # whichever overlaps the fewest labels.
        spots = [(tx + offset[0] * fx * reach * DPI, ty + offset[1] * fy * reach * DPI)
                 for reach in (1, 1.4) for fx, fy in ((1, 1), (-1, 1), (1, -1), (-1, -1))]
        def cost (spot):
            clashes = sum (abs (spot[0] - x) < (width + w) / 2 + 2 and abs (spot[1] - y) < size + 2
                           for x, y, w in self.labels)
            if self.view:
                vx, vy, vw, vh = self.view
                clashes += 0.5 * (not (vx + 2 < spot[0] - width / 2 and spot[0] + width / 2 < vx + vw - 2
                                       and vy + size < spot[1] < vy + vh - 2))
            return clashes

        lx, ly = min (spots, key=cost)
        pencil.text (lx, ly, text, size=size, kind="label", italic=True)
        self.labels.append ((lx, ly, width))
        sx = lx + (width / 2 + 3 if tx > lx + width / 2 else
                   -width / 2 - 3 if tx < lx - width / 2 else 0)
        sy = ly - size * 0.35 if sx != lx else (ly + 3 if ty > ly else ly - size)
        dx, dy = tx - sx, ty - sy
        length = max (1.0, math.hypot (dx, dy))
        ex, ey = tx - dx / length * 4, ty - dy / length * 4
        bend = 0.18 * length
        mid = ((sx + ex) / 2 - dy / length * bend, (sy + ey) / 2 + dx / length * bend)
        pencil.arrow ([(sx, sy), mid, (ex, ey)])

    def _point (self, at):
        if isinstance (at, tuple):
            return at[0] * DPI, at[1] * DPI
        if at in self.pins:
            return self.pin_xy (at)
        module, _, pin = at.partition (".")
        if pin and module in self.modules:
            placed = self.modules[module]
            return placed.anchor (placed.pin (pin))[0]
        return self.hole_xy (at)

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
        pencil.polyline (outline, width=1.0, closed=True, passes=2)
        # The channel down the middle, where chips straddle.
        pencil.fill ([(x + 6, y + 1.05 * DPI), (x + w - 6, y + 1.05 * DPI),
                      (x + w - 6, y + 1.15 * DPI), (x + 6, y + 1.15 * DPI)], tone=0.07)
        board = dict (kind="silk", halo="#ffffff", layer="crisp")
        for rail, offset in (("T+", 0.06), ("T-", 0.34), ("B+", 1.86), ("B-", 2.14)):
            color = "#c0564e" if rail.endswith ("+") else "#4f74ad"
            pencil.stripe ((x + 14, y + offset * DPI), (x + w - 14, y + offset * DPI), color)
            sign = "+" if rail.endswith ("+") else "−"
            hy = (by + ROWS[rail]) * DPI
            # Printing near the Mega's end, which wire labels keep clear of.
            if not torn_left:
                pencil.text (x + 8, hy + 3.5, sign, size=9, tone=0.7, **board)
                self.labels.append ((x + 8, hy + 3.5, 6))
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
                if offset < 0:
                    self.labels.append ((hx + offset, hy + 2.3, 4))


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


# Geometry for wires ---------------------------------------------------------

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


def within (point, box):
    return box[0] < point[0] < box[2] and box[1] < point[1] < box[3]


# Whether the segment from a to b stays out of the box (Liang-Barsky).
def misses (a, b, box):
    x0, y0, x1, y1 = box[0] + 0.3, box[1] + 0.3, box[2] - 0.3, box[3] - 0.3
    dx, dy = b[0] - a[0], b[1] - a[1]
    low, high = 0.0, 1.0
    for p, q in ((-dx, a[0] - x0), (dx, x1 - a[0]), (-dy, a[1] - y0), (dy, y1 - a[1])):
        if abs (p) < 1e-9:
            if q < 0:
                return True
        else:
            t = q / p
            if p < 0:
                low = max (low, t)
            else:
                high = min (high, t)
            if low > high:
                return True
    return False


def clear_path (points, boxes):
    return all (misses (a, b, box) for a, b in zip (points, points[1:]) for box in boxes)


# The shortest way from start to goal round the boxes, through their
# corners: the points in between.
def detour (start, goal, boxes, margin):
    # Each box grows by the margin, or less where an end is close to it; a
    # box an end is inside is ignored.
    grown = []
    for box in boxes:
        room = margin
        while room > 0 and (within (start, grow (box, room)) or within (goal, grow (box, room))):
            room -= 2
        if room > 0 or not (within (start, box) or within (goal, box)):
            grown.append (grow (box, max (room, 0.5)))
    corners = [(x, y) for box in grown for x in (box[0] - 0.5, box[2] + 0.5)
               for y in (box[1] - 0.5, box[3] + 0.5)]
    nodes = [start, goal] + [c for c in corners if not any (within (c, box) for box in grown)]
    best = {0: 0.0}
    came = {}
    heap = [(0.0, 0)]
    while heap:
        cost, node = heapq.heappop (heap)
        if node == 1:
            break
        if cost > best.get (node, math.inf):
            continue
        for other in range (1, len (nodes)):
            if other == node:
                continue
            if not all (misses (nodes[node], nodes[other], box) for box in grown):
                continue
            total = cost + math.dist (nodes[node], nodes[other]) + 8
            if total < best.get (other, math.inf):
                best[other], came[other] = total, node
                heapq.heappush (heap, (total, other))
    if 1 not in came:
        return []
    path, node = [], came[1]
    while node != 0:
        path.append (nodes[node])
        node = came[node]
    return path[::-1]


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
