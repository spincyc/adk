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
(docs/kit.md lists them), so one build carries on into the next. A part
with a home goes in with its home_* call, which lays the part and its fixed
wires there, the same in every lesson: bench.home_led ("26", "red") builds
all four lines above. The home_* calls are home_led, home_button,
home_buzzer, home_rgb_led, home_divider, home_knob, home_encoder,
home_servo, home_modem, home_fm_radio, home_rf_receiver and
home_rf_transmitter on and beside the breadboard; home_matrix,
home_joystick, home_gy521, home_rtc, home_dht11, home_ds18b20,
home_ultrasonic, home_motor, home_relay, home_keypad, home_stepper,
home_rfid, home_pir, home_tap and home_beam for modules; and screen (). A
call keeps its home when the screen is added or removed. The buzzer alone
uses a second home beside the four-digit display's wiring. via, as in
wire (), routes a wire from the Mega round the lesson's other parts. The
Mega's power is wired by the bench itself, the same way every time: once
circuit.py has run, finish () brings GND from the outer pin at the end of
the long header into B-3 and 5V from the outer pin at its top into T+3,
when anything uses the rails, and joins a rail pair just before the LCD
(B-41 to T-41, T+42 to B+42) when a part uses the other rail and nothing else feeds
it. With the power module, which lies beside the far end and feeds the
bottom rails from B+42 and B-42 (power_module ("5V") or ("3.3V")), the
Mega's 5V feeds only the top rails. Wiring the Mega's GND or 5V to a rail,
or using those two pins, is an error.

Parts stand in the breadboard: resistor, led, button, rgb_led, buzzer,
potentiometer, photoresistor, thermistor, tilt_switch, chip (a DIP chip
across the middle gap), digit and four_digits (seven-segment displays), lcd
and header_module (a module standing in one row). screen () builds the
course's LCD at its home: its contrast knob across the gap in f43, d44 and
f45, the LCD's pins in a47-a62, power, and pins 31 to 36, wired the same in
every lesson.

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

The build steps come in stages, a part and its wires under one heading:
each home_* call is one, and stage (name) names the steps after it, up to
the next home_* call or stage (). Other steps are named for their first
part other than a resistor, and a module placed among them starts another.

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

import functools
import hashlib
import inspect
import math
import re
from typing import NamedTuple

from modules import STUB, Placed, PowerModule, make as make_module
from parts import (Button, Buzzer, Chip, Diode, Display, HeaderModule, Led, Potentiometer, Resistor,
                   RgbLed, Transistor, TwoLegs, bands_for)
from pencil import DPI

MEGA_WIDTH, MEGA_HEIGHT = 4.0, 2.1
BOARD_HEIGHT = 2.2
GAP = 0.9                           # between the Mega and the breadboard
END = 0.15                          # from a board's end to its first column, as the kit's
MARGIN = 0.35

# The kit's breadboard jumpers come in black, red, orange, yellow, green,
# blue and white; its female-to-male wires are a ribbon of ten colors, those
# and brown, purple and grey. A wire must be one the kit has, and red and
# black are kept for 5V and GND.
JUMPERS = {"black", "red", "orange", "yellow", "green", "blue", "white"}
RIBBON = JUMPERS | {"brown", "purple", "grey"}
SIGNAL_COLORS = ["yellow", "green", "blue", "orange", "white"]
# Each home pin's wire keeps one color in every lesson, chosen so the pins
# that share a lesson differ where they can: an LED's wire in its LED's
# color (orange for red), the RGB LED's in its channel's.
PIN_COLORS = {
    "22": "green", "23": "blue", "24": "yellow", "25": "white",
    "26": "orange", "27": "yellow", "28": "green", "29": "blue", "30": "white",
    "31": "white", "32": "orange", "33": "yellow", "34": "green", "35": "blue", "36": "white",
    "37": "yellow", "38": "green", "39": "blue", "40": "orange", "41": "green", "42": "white",
    "43": "yellow", "44": "orange", "45": "yellow", "46": "orange", "47": "green", "48": "blue",
    "49": "yellow", "50": "orange", "51": "white", "52": "green", "53": "blue",
    "2": "white", "3": "orange", "4": "yellow", "5": "orange", "6": "green", "7": "blue",
    "8": "blue", "9": "white", "10": "orange", "11": "green", "12": "white", "14": "yellow",
    "15": "green", "16": "orange", "17": "white", "18": "white", "19": "blue", "20": "green",
    "21": "blue",
    "A0": "blue", "A1": "green", "A2": "yellow", "A3": "yellow", "A4": "white", "A5": "green",
    "A8": "green", "A9": "blue", "A10": "yellow", "A11": "white",
    "A12": "orange", "A13": "white", "A14": "yellow", "A15": "green",
}
# The holes of the parts that have a breadboard home, by pin: an LED's column
# (its resistor's and long leg's) and a button's left column.
LED_HOMES = {"26": 6, "27": 12, "28": 18, "29": 24, "30": 30, "3": 38}
BUTTON_HOMES = {"22": 2, "23": 8, "24": 14, "25": 20}
# Each pair of rails reads − then + from top to bottom, as the kit's 830-hole
# board is printed: − at the top edge, + at the bottom one.
ROWS = {"j": 0.55, "i": 0.65, "h": 0.75, "g": 0.85, "f": 0.95,
        "e": 1.25, "d": 1.35, "c": 1.45, "b": 1.55, "a": 1.65,
        "T-": 0.15, "T+": 0.25, "B-": 1.95, "B+": 2.05}
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


# One build step, as a row of the steps table: what goes in, with a note on
# it; the colors it is picked out by (a wire's or an LED's, a resistor's
# bands); and its places, each a hole or pin with what tells it apart. Two
# places are the step's From and To; more, or spread ones, fill both.
class Step (NamedTuple):
    what: str
    places: tuple
    note: str = ""
    colors: tuple = ()
    spread: bool = False
    stage: object = None


# A stage of the build: a part and the wires that go with it, as a home_*
# call lays them, or the steps after stage () in circuit.py. Its name is
# the part's; the pins it wires are added to it (stage_title ()). Steps in
# no stage make one of their own, named for their first part other than a
# resistor, and each module placed there starts another.
class Stage:
    def __init__ (self, name=None):
        self.name = name


# A home_* call's steps make one stage, named for its part: "the {color}
# LED" names home_led ("26", "red")'s "the red LED".
def staged (name):
    def wrap (build):
        signature = inspect.signature (build)

        @functools.wraps (build)
        def staging (self, *args, **options):
            asked = signature.bind (self, *args, **options)
            asked.apply_defaults ()
            self._stage = Stage (name.format (**asked.arguments))
            try:
                return build (self, *args, **options)
            finally:
                self._stage = None
        return staging
    return wrap


# The header pins where Arduino's Mega 2560 Rev3 board file puts them, in
# inches from the board's corner by the USB socket: the shield headers as on
# the Uno, with the 0.16 inch step between pins 8 and 7, and the power
# header after its one unconnected pin.
def mega_pins ():
    pins = {}
    top = ["SCL", "SDA", "AREF", "GND"] + [str (n) for n in range (13, 7, -1)]
    for index, name in enumerate (top):
        pins[name] = MegaPin (0.74 + 0.1 * index, 2.0, "top", 0)
    for index, n in enumerate (range (7, -1, -1)):
        pins[str (n)] = MegaPin (1.80 + 0.1 * index, 2.0, "top", 1)
    for index, n in enumerate (range (14, 22)):
        pins[str (n)] = MegaPin (2.70 + 0.1 * index, 2.0, "top", 2)
    for index, name in enumerate (["IOREF", "RESET", "3.3V", "5V", "GND2", "GND3", "VIN"]):
        pins[name] = MegaPin (1.20 + 0.1 * index, 0.1, "bottom", 0)
    for n in range (8):
        pins[f"A{n}"] = MegaPin (2.00 + 0.1 * n, 0.1, "bottom", 1)
    for n in range (8, 16):
        pins[f"A{n}"] = MegaPin (2.90 + 0.1 * (n - 8), 0.1, "bottom", 2)
    rows = [("5V2", "5V3")] + [(str (n), str (n + 1)) for n in range (22, 53, 2)]
    rows += [("GND4", "GND5")]
    for index, (even, odd) in enumerate (rows):
        pins[even] = MegaPin (3.70, 2.00 - 0.1 * index, "double")
        pins[odd] = MegaPin (3.80, 2.00 - 0.1 * index, "double", outer=True)
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
        self.source = None              # its circuit.py's digest and letter, which load () gives
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
        self.screened = False           # the course's screen is on the board
        self._stage = None              # the stage steps join now

    def _step (self, step):
        kind, thing = self._last
        self._stage = self._stage or Stage ()
        self.items.append ((kind, thing, step._replace (stage=self._stage)))

    # The steps that follow, up to the next home_* call or stage (), are
    # one stage of the build, named for what they build: "the 74HC595".
    def stage (self, name):
        self._stage = Stage (name)
        return self

    # A stage's title: its name, or its first part's, and the pins it wires.
    def stage_title (self, stage):
        items = [(kind, thing) for kind, thing, step in self.items if step.stage is stage]
        name = stage.name
        if name is None:
            things = [thing for kind, thing in items if kind != "wire"]
            main = next ((thing for thing in things if not isinstance (thing, Resistor)),
                         things[0] if things else None)
            name = f"the {getattr (main, 'title', None) or main.name}" if main else "wires"
        pins = {canonical (end[1]) for kind, wire in items if kind == "wire" for end in wire[:2]
                if end[0] == "pin" and numbered (canonical (end[1]))}
        return name + (f" on {pin_words (pins)}" if pins else "")

    # A leg's hole in a step, with which leg it is: in a rail, after the
    # column beside it.
    def _at (self, hole, leg=""):
        where, column = self.place (("hole", hole))
        return where, ", ".join (words for words in (column, leg) if words)

    # Where things are ---------------------------------------------------

    def board_width (self):
        return 2 * END + (self.last - self.first) * 0.1

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
        return (bx + END + (column - self.first) * 0.1) * DPI, (by + ROWS[row]) * DPI

    def strip_of (self, hole):
        kind, column, row = parse_hole (hole)
        if kind == "rail":
            return f"rail {row}"
        return f"column {column} {'a-e' if row in 'abcde' else 'f-j'}"

    # Parts on the breadboard -------------------------------------------

    def resistor (self, value, a, b):
        self._add (Resistor (value, a, b))
        self._step (Step (f"{value} resistor", (self._at (a), self._at (b)),
                          colors=tuple (bands_for (value))))
        return self

    def led (self, color, anode, cathode):
        self._add (Led (color, anode, cathode))
        self._step (Step ("LED", (self._at (anode, "long leg"), self._at (cathode, "short leg")),
                          colors=(color,)))
        return self

    def diode (self, anode, cathode):
        self._add (Diode (anode, cathode))
        self._step (Step ("1N4007 diode", (self._at (anode, "unbanded end"),
                                           self._at (cathode, "banded end"))))
        return self

    def transistor (self, emitter, base, collector):
        self._add (Transistor (emitter, base, collector))
        self._step (Step ("S8050 transistor", (self._at (emitter, "E, emitter"),
                                               self._at (base, "B, base"),
                                               self._at (collector, "C, collector")),
                          "marked flat face toward you; check S8050 and its E–B–C pin order"))
        return self

    def button (self, column):
        part = Button (column)
        self._add (part)
        self._step (Step ("push button", tuple (self._at (hole) for hole in part.holes ()),
                          "across the middle gap"))
        return self

    def rgb_led (self, red, common, green, blue):
        self._add (RgbLed (red, common, green, blue))
        self._step (Step ("RGB LED", (self._at (red, "red leg"),
                                      self._at (common, "the longest leg (common, −)"),
                                      self._at (green, "green leg"), self._at (blue, "blue leg"))))
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
        self._step (Step (f"{kind} buzzer", (self._at (positive, mark), self._at (negative))))
        return self

    # The kit's potentiometer has its legs in a triangle, the wiper on its
    # own 0.4 inch from the other two, so it fits a breadboard only across
    # the middle gap: its outer legs two columns apart in row e or f, the
    # wiper between them on the other side of the gap, in row g or d.
    def potentiometer (self, left, wiper, right, value="10 kΩ"):
        spots = [parse_hole (hole) for hole in (left, wiper, right)]
        columns = [column for _, column, _ in spots]
        rows = [row for _, _, row in spots]
        steps = {columns[1] - columns[0], columns[2] - columns[1]}
        if not (rows[0] == rows[2] and (rows[0], rows[1]) in (("e", "g"), ("f", "d"))
                and steps == {1}):
            raise ValueError ("a potentiometer's outer legs stand two columns apart in row e or "
                              "f, its wiper between them across the gap in row g or d")
        self._add (Potentiometer (left, wiper, right, value))
        self._step (Step (f"{value} potentiometer", (self._at (left, "outer leg"),
                                                     self._at (wiper, "wiper, on its own"),
                                                     self._at (right, "outer leg")),
                          "across the middle gap"))
        return self

    def photoresistor (self, a, b):
        return self._two_legs ("photoresistor", a, b)

    def thermistor (self, a, b):
        return self._two_legs ("thermistor", a, b)

    def tilt_switch (self, a, b):
        return self._two_legs ("tilt switch", a, b)

    def _two_legs (self, kind, a, b):
        self._add (TwoLegs (kind, a, b))
        # The tilt switch reads its tilt, so it stands straight up.
        upright = "standing upright, " if kind == "tilt switch" else ""
        self._step (Step (kind, (self._at (a), self._at (b)), f"{upright}either way round"))
        return self

    # A DIP chip across the middle gap: pin 1 in e<first>, pins running
    # right along row e and back along row f (or d and h for a 0.6 inch
    # package).
    def chip (self, name, pins=None, first=1, span=3):
        pins = list (pins or CHIPS[name])
        holes, _ = self._straddle (len (pins), first, span)
        self._add (Chip (name, pins, holes))
        half = len (pins) // 2
        sides = ((f"{holes[0]}–{holes[half - 1]}", f"pins 1 ({pins[0]}) to {half}"),
                 (f"{holes[half]}–{holes[-1]}", f"pins {half + 1} to {len (pins)} ({pins[-1]})"))
        self._step (Step (name, sides, "across the middle gap, its notch to the left", spread=True))
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
        sides = ((f"{holes[0]}–{holes[half - 1]}", f"pins 1 ({labels[0]}) to {half}"),
                 (f"{holes[half]}–{holes[-1]}", f"pins {half + 1} to {len (pins)}"))
        self._step (Step (name, sides, f"across the middle gap, decimal point"
                                       f"{'s' if digits > 1 else ''} at the bottom", spread=True))
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
        edge = "bottom" if part.turned else "top"
        self._step (Step ("LCD", tuple ((hole, pin.name) for pin, hole in part.pairs ()),
                          f"face up, its screen lying over the {edge} edge of the board"
                          f"{'' if part.turned else ', upside down from where you sit'}",
                          spread=True))
        return self

    # The course's screen, at its home and wired the same in every lesson so
    # it can stay on the breadboard from one to the next. From column first
    # (43): the contrast knob across the middle gap, its outer legs in row f
    # with a jumper from each up to a top rail and its wiper in row d, and
    # the LCD four columns on, in row a, its screen over the bottom edge. A
    # jumper takes the wiper's column to V0, and column first carries GND
    # from the bottom − rail to VSS; VSS, VDD and RW reach up to the top
    # rails, so VDD takes 5V from the top + rail and the top − rail is GND
    # through VSS. The signal wires from pins 31 to 36 rise beside the Mega
    # and cross over the board in lanes, 31 lowest, each coming straight
    # down into its pin's column; the backlight's 220 Ω resistor stands
    # across the gap at the far end. lanes lifts the signal wires' lanes by
    # that many steps, and risers gives where the first rises and how far
    # apart they stand, in inches, to make room for other wires from the
    # header.
    @staged ("the screen")
    def screen (self, first=43, text=None, lanes=0, risers=(4.45, 0.1)):
        self.screened = True
        self.last = max (self.last, 63)
        knob, lcd = first, first + 4
        column = lambda offset: lcd + offset
        self.potentiometer (f"f{knob}", f"d{knob + 1}", f"f{knob + 2}")
        self.lcd (lcd, row="a", text=text)
        self.wire (f"a{knob}", self._rail ("B-", knob))
        self.wire (f"b{knob}", f"b{lcd}", color="black")
        self.wire (f"c{knob + 1}", f"c{column (2)}", color="yellow")
        self.wire (f"j{knob}", self._rail ("T-", knob))
        self.wire (f"j{knob + 2}", self._rail ("T+", knob + 2))
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

    # The first free hole of a rail at or after a column, or, past the
    # rail's last hole, the nearest before it.
    def _rail (self, rail, column):
        for step in (1, -1):
            spot = column
            while 1 <= spot <= 63:
                if rail_column (spot) and f"{rail}{spot}" not in self.used:
                    return f"{rail}{spot}"
                spot += step
        raise ValueError (f"no free hole in the {rail} rail near column {column}")

    # Parts at their homes -----------------------------------------------
    #
    # A part the course uses again and again has a home: the same holes, or
    # the same place beside the board, in every lesson that uses it
    # (docs/kit.md#breadboard-homes), so a build carries on from one lesson
    # to the next. Each of these lays a part at its home with its fixed
    # wires. The holes never change; via, as in wire (), routes a wire from
    # the Mega round the lesson's other parts. The screen occupies the far
    # end, leaving these homes unchanged when it is added or removed.

    # An LED on 26 to 30, or the dimmable one on 3: the pin into j, 220 Ω
    # from g across the gap to e, the long leg in b, the short leg in b of
    # the next column, and a black jumper from a there to the − rail.
    @staged ("the {color} LED")
    def home_led (self, pin, color, via=None):
        column = LED_HOMES[pin]
        self.wire (pin, f"j{column}", via=via)
        self.resistor ("220 Ω", f"g{column}", f"e{column}")
        self.led (color, anode=f"b{column}", cathode=f"b{column + 1}")
        self.wire (f"a{column + 1}", f"B-{column + 1}")
        return self

    # A button on 22 to 25 across the gap, the pin into j of its left
    # column and a black jumper from a of its right column to the − rail.
    @staged ("the button")
    def home_button (self, pin, via=None):
        column = BUTTON_HOMES[pin]
        self.wire (pin, f"j{column}", via=via)
        self.button (column)
        self.wire (f"a{column + 2}", f"B-{column + 2}")
        return self

    # The active buzzer on 12, or the passive one on 10, across the gap in
    # column 33: + in f, − in e. A passive buzzer takes its pin into j and
    # 220 Ω from a to GND. An active one takes 5 V through a low-side S8050
    # driver, whose base on column 30 gets pin 12 through 1 kΩ. Beside the four-digit
    # display it stands in column 35, clear of the wires that rise over the
    # gap in column 32 and the segment circuit that occupies column 33.
    @staged ("the {kind} buzzer")
    def home_buzzer (self, kind, via=None):
        column = 35 if self._has ("four-digit display") else 33
        self.buzzer (f"f{column}", f"e{column}", kind=kind)
        if kind == "active":
            self.transistor ("a29", "a30", "a31")
            self.wire ("12", "a32", via=via)
            self.resistor ("1 kΩ", "c32", "c30")
            self.resistor ("10 kΩ", "b30", "B-30")
            self.wire ("b29", "B-29")
            self.wire ("b31", f"a{column}", color="black")
            self.wire (f"h{column}", f"T+{column}")
            self.diode (f"c{column}", f"c{column + 3}")
            self.wire (f"a{column + 3}", f"j{column}", color="red")
        else:
            self.wire ("10", f"j{column}", via=via)
            self.resistor ("220 Ω", f"a{column}", f"B-{column}")
        return self

    # The RGB LED on 5, 6 and 7: its red, green and blue legs in a6, a9 and
    # a11, its common leg in the − rail beside the red, and 220 Ω across the
    # gap above each colored leg, its pin into j. A button on 23 covers e9:
    # green_resistor=13 moves just that resistor, with a jumper back to b9,
    # leaving the LED and button at their homes. via routes the pin wires.
    @staged ("the RGB LED")
    def home_rgb_led (self, via=(None, None, None), green_resistor=9):
        columns = (6, green_resistor, 11)
        for pin, column, route in zip (("5", "6", "7"), columns, via):
            self.wire (pin, f"j{column}", via=route)
        for column in columns:
            self.resistor ("220 Ω", f"g{column}", f"e{column}")
        if green_resistor != 9:
            self.wire (f"b{green_resistor}", "b9", color="green")
        return self.rgb_led (red="a6", common="B-7", green="a9", blue="a11")

    # A light or temperature divider in column 37, between the buzzer and
    # the knob: 5 V from the top + rail, the sensor across the gap, A1 or
    # A2 into c37, and 10 kΩ from a37 down to the bottom − rail. Keeping
    # the resistor in its column leaves the knob's wiper clear.
    @staged ("the {sensor}")
    def home_divider (self, sensor, via=None):
        self.wire ("j37", "T+37")
        if sensor == "photoresistor":
            self.photoresistor ("f37", "e37")
        else:
            self.thermistor ("f37", "e37")
        self.wire ({"photoresistor": "A1", "thermistor": "A2"}[sensor], "c37", via=via)
        return self.resistor ("10 kΩ", "a37", "B-37")

    # The knob on A0, across the middle gap in columns 39 to 41: its outer
    # legs in f39 and f41, jumpers from j to the top − and + rails, its
    # wiper in d40, and A0 into a40. It keeps these holes beside the screen.
    @staged ("the knob")
    def home_knob (self, via=None):
        self.potentiometer ("f39", "d40", "f41")
        self.wire ("j39", "T-39")
        self.wire ("j41", "T+41")
        return self.wire ("A0", "a40", via=via)

    # The rotary encoder stands in row a, columns 15 to 19, its knob
    # toward you. GND and + reach the top rails by 18 and 19, beyond the
    # GY-521's board but before the radio receiver's, so the encoder stays
    # here when either is added. Pins 22, 19 and 18 reach SW, DT and CLK.
    @staged ("the rotary encoder")
    def home_encoder (self):
        self.header_module ("encoder", pins=("GND", "+", "SW", "DT", "CLK"), first=15,
                            row="a")
        self.wire ("e15", "T-18")
        self.wire ("e16", "T+19")
        for hole, pin in (("e17", "22"), ("e18", "19"), ("e19", "18")):
            self.wire (pin, hole)
        return self

    # The servo below the board and the modem, its plug under columns 34
    # to 36: + into B+35 and − into B-36, fed by the power module, and its
    # signal from pin 44. The same space stays free with or without a modem.
    @staged ("the servo")
    def home_servo (self, via=None):
        self.module ("servo", at=(8.05, 5.35), facing="up")
        self.wire ("servo.+", "B+35")
        self.wire ("servo.−", "B-36")
        return self.wire ("44", "servo.signal", via=via)

    # The LoRa modem below the board under columns 24 to 29, aerial down:
    # its GND into B-24, TX3 (pin 14) into j28 and down through 1 kΩ and
    # 2 kΩ to the − rail, the modem's RXD into c28 between them, and its
    # TXD into f26 beside RX3 (pin 15) in j26. Beside the ultrasonic
    # sensor, which has 14 and 15, it takes Serial2 instead: TX2 (16) and
    # RX2 (17). Its VDD takes 3.3 V from power: the Mega's 3.3V pin, or a
    # rail the power module sets to 3.3 V. tx, rx, txd and supply route the
    # wires from the two pins, TXD and power.
    @staged ("the LoRa modem")
    def home_modem (self, tx=None, rx=None, txd=None, power="3.3V", supply=None):
        send, hear = ("16", "17") if "14" in self.taken else ("14", "15")
        self.module ("lora_modem", "modem", at=(7.615, 3.45), facing="up")
        self.wire ("modem.GND", "B-24")
        self.wire (power, "modem.VDD", via=supply)
        self.wire (send, "j28", via=tx)
        self.resistor ("1 kΩ", "g28", "e28")
        self.resistor ("2 kΩ", "a28", "B-28")
        self.wire ("modem.RXD", "c28", color="brown")
        self.wire ("modem.TXD", "f26", color="purple", via=txd)
        return self.wire (hear, "j26", via=rx)

    # The FM radio standing in row j, columns 27 to 34, its board over the
    # top rails: pins 42, 41 and 40 into RST, SCLK and SDIO, the Mega's
    # 3.3V into its 3.3V column, a black jumper from its GND column to the − rail, and 1 kΩ
    # from RST to 3.3 V along row h.
    @staged ("the FM radio")
    def home_fm_radio (self):
        self.header_module ("fm_radio", first=27, row="j")
        self.wire ("42", "f29")
        self.wire ("41", "f31")
        self.wire ("40", "f32")
        self.wire ("3.3V", "f34")
        self.wire ("f33", "B-33")
        return self.resistor ("1 kΩ", "h29", "h34")

    # The 433 MHz receiver standing in row j, columns 30 to 33, its board
    # over the top rails: 5 V from the power header into VCC's column, pin
    # 43 into DATA's, and a black jumper from its GND column to the − rail.
    @staged ("the radio receiver")
    def home_rf_receiver (self):
        self.header_module ("rf_receiver", first=30, row="j")
        self.wire ("5V.power", "f30")
        self.wire ("43", "f31")
        return self.wire ("f33", "B-33")

    # The 433 MHz transmitter standing in row j, columns 38 to 41: its DAT
    # the middle of a divider, pin 46 into f36, 1 kΩ along row h to DAT's
    # column and 2 kΩ across the gap and down to the − rail; the Mega's
    # 3.3V into its + column, and a black jumper from its − column.
    @staged ("the radio transmitter")
    def home_rf_transmitter (self):
        self.header_module ("rf_transmitter", first=38, row="j")
        self.wire ("46", "f36")
        self.resistor ("1 kΩ", "h36", "h39")
        self.resistor ("2 kΩ", "g39", "e39")
        self.wire ("a39", "B-39")
        self.wire ("3.3V", "f40")
        return self.wire ("f41", "B-41")

    # Modules at their homes beside the board.

    # The LED matrix below the gap between the Mega and the breadboard,
    # input pins up: CLK from 48, CS from 49, DIN from 47, GND from the −
    # rail at ground, and VCC from the inner 5V pin, or vcc. options, such
    # as pixels, are the matrix's own.
    @staged ("the LED matrix")
    def home_matrix (self, vcc="5V.long", ground="B-5", **options):
        self.module ("matrix", at=(4.22, 3.9), facing="up", **options)
        self.wire ("48", "matrix.CLK")
        self.wire ("49", "matrix.CS")
        self.wire ("47", "matrix.DIN")
        self.wire (ground, "matrix.GND")
        return self.wire (vcc, "matrix.VCC")

    # The joystick below the Mega, under A3 and A4: VRx and VRy on them, +5V
    # from the power header's 5V, GND from the inner GND at the end of the
    # long header, and its switch on 22.
    @staged ("the joystick")
    def home_joystick (self):
        self.module ("joystick", at=(2.08, 3.9), facing="up")
        self.wire ("A3", "joystick.VRx")
        self.wire ("A4", "joystick.VRy")
        self.wire ("5V.power", "joystick.+5V")
        self.wire ("GND.long", "joystick.GND")
        return self.wire ("22", "joystick.SW")

    # The GY-521 standing in row j, columns 9 to 16, its board over the top
    # rails: VCC from the top + rail, GND down across the gap to the − rail,
    # and SCL and SDA from 21 and 20 through the BSS138 level shifter above
    # the gap beside the Mega. Its low side takes 3.3 V through column 5,
    # which also leaves a tap for another 3.3 V module.
    @staged ("the GY-521")
    def home_gy521 (self):
        self.header_module ("gy521", first=9, row="j")
        self.wire ("T+7", "i9")
        self.wire ("f10", "e10", color="black")
        self.wire ("a10", "B-10")
        self.module ("i2c_level_shifter", "levels", at=(5.0, -1.3), facing="down",
                     detail="headers fitted; LV/A pins toward the Mega, HV/B pins toward the board")
        self.wire ("3.3V", "a5")
        self.wire ("b5", "levels.LV", color="red")
        self.wire ("T+5", "levels.HV")
        self.wire ("B-12", "levels.GND")
        self.wire ("20", "levels.B1")
        self.wire ("21", "levels.B2")
        self.wire ("levels.A1", "h12", color="green")
        return self.wire ("levels.A2", "g11", color="blue")

    # The DS1307 clock above the board, facing right: GND and VCC from the
    # top rails in columns 13 and 15, SDA and SCL from 20 and 21 over the
    # top. sda and scl route those two round anything else up there.
    @staged ("the clock module")
    def home_rtc (self, sda=None, scl=None):
        self.module ("rtc", at=(6.5, -1.4), facing="right")
        self.wire ("rtc.GND", "T-13")
        self.wire ("rtc.VCC", "T+15")
        self.wire ("20", "rtc.SDA", via=sda or [(3.75, -1.6), (8.5, -1.6), (8.5, -0.8)])
        return self.wire ("21", "rtc.SCL", via=scl or [(3.85, -1.5), (8.4, -1.5), (8.4, -0.9)])

    # The DHT11 above the board: S from 16, + and − from the top rails in
    # columns 36 and 37.
    @staged ("the DHT11")
    def home_dht11 (self):
        self.module ("dht11", "dht", at=(8.58, -1.27))
        self.wire ("16", "dht.S")
        self.wire ("dht.+", "T+36")
        return self.wire ("dht.−", "T-37")

    # The 18B20 temperature module above the board: S from 17, + and − from
    # the top rails in columns 27 and 28.
    @staged ("the 18B20")
    def home_ds18b20 (self):
        self.module ("sensor", "probe", at=(7.68, -1.15), label="18B20", pins=("G", "R", "Y"))
        self.wire ("17", "probe.S")
        self.wire ("probe.+", "T+27")
        return self.wire ("probe.−", "T-25")

    # The ultrasonic sensor above the Mega, facing away: Trig and Echo from
    # 14 and 15, VCC from the inner 5V pin, GND to the top − rail.
    @staged ("the ultrasonic sensor")
    def home_ultrasonic (self):
        self.module ("ultrasonic", name="sensor", at=(2.715, -1.2))
        self.wire ("14", "sensor.Trig")
        self.wire ("15", "sensor.Echo")
        self.wire ("sensor.VCC", "5V.long")
        return self.wire ("sensor.GND", "T-5")

    # The DC motor above the board, its leads down into j14 and j17, the
    # L293D's outputs.
    @staged ("the motor")
    def home_motor (self):
        self.module ("motor", name="motor", at=(4.7, -1.25), facing="down")
        self.wire ("motor.−", "j14", via=[(6.7, -0.41)])
        return self.wire ("motor.+", "j17", via=[(7.0, -0.59)])

    # The relay above the Mega: S from 11, + from the inner 5V pin and −
    # from the inner GND, both over the top.
    @staged ("the relay")
    def home_relay (self):
        self.module ("relay", at=(5.05, -0.75), facing="left")
        self.wire ("11", "relay.S", via=[(1.70, -0.35)])
        self.wire ("5V.long", "relay.+", via=[(4.25, 0.65), (4.25, -0.25)])
        return self.wire ("GND.long", "relay.−", via=[(4.35, 2.35), (4.35, -0.15)])

    # The keypad above the Mega, its eight wires R1 to C4 from 22 to 29,
    # each rising past the header in a lane of its own.
    @staged ("the keypad")
    def home_keypad (self):
        self.module ("keypad", "keypad", at=(3.04, -4.95))
        turns = (0.5, 0.45, 0.4, 0.35, None, 0.4, 0.45, 0.5)
        for index, name in enumerate (("R1", "R2", "R3", "R4", "C1", "C2", "C3", "C4")):
            riser, height = 4.25 + 0.05 * index, 0.75 + 0.05 * index
            socket = 4.05 + 0.1 * index
            via = [(riser, height)]
            if turns[index]:
                via += [(riser, turns[index]), (socket, turns[index])]
            self.wire (str (22 + index), f"keypad.{name}", via=via)
        return self

    # The stepper's driver below the Mega, its IN1 to IN4 from A8 to A11,
    # each stepping down and across to its pin, A11 highest, so the four
    # cross square on in a tidy staircase; and its + and − from the bottom
    # rails, unless the lesson powers it some other way.
    @staged ("the stepper driver")
    def home_stepper (self, powered=True):
        self.module ("stepper", at=(3.1, 4.5), facing="up")
        self.wire ("A8", "stepper.IN1", via=[(3.25, 3.5), (4.45, 3.5)])
        self.wire ("A9", "stepper.IN2", via=[(3.35, 3.3), (4.35, 3.3)])
        self.wire ("A10", "stepper.IN3", via=[(3.45, 3.1), (4.25, 3.1)])
        self.wire ("A11", "stepper.IN4", via=[(3.55, 2.9), (4.15, 2.9)])
        if powered:
            self.wire ("stepper.+", "B+5", via=[(3.56, 3.65), (5.8, 3.65)])
            self.wire ("stepper.−", "B-6", via=[(3.66, 3.75), (5.9, 3.75)])
        return self

    # The RFID reader below the Mega: 3.3V from the Mega's 3.3V pin, GND
    # from the inner GND. The four Mega outputs reach SDA, SCK, MOSI and
    # RST through 1 kΩ / 2 kΩ dividers in columns 18, 20, 22 and 24;
    # MISO, the reader's 3.3 V output, reaches pin 50 directly.
    @staged ("the RFID reader")
    def home_rfid (self):
        self.module ("rfid", at=(1.65, 4.17), facing="up")
        self.wire ("3.3V", "rfid.3.3V", via=[(1.76, 2.8), (2.1, 2.8)])
        self.wire ("GND.long", "rfid.GND", via=[(4.25, 2.35), (4.25, 3.0), (2.3, 3.0)])
        for pin, signal, column, ground in (("53", "SDA", 18, 18), ("52", "SCK", 20, 21),
                                             ("51", "MOSI", 22, 22), ("45", "RST", 24, 25)):
            self.wire (pin, f"j{column}")
            self.resistor ("1 kΩ", f"g{column}", f"e{column}")
            self.resistor ("2 kΩ", f"a{column}", f"B-{ground}")
            self.wire (f"c{column}", f"rfid.{signal}")
        self.wire ("50", "rfid.MISO", via=[(4.65, 3.4), (2.5, 3.4)])
        return self

    # The PIR sensor below the Mega: OUT on A12, VCC and GND from the power
    # header.
    @staged ("the PIR sensor")
    def home_pir (self):
        self.module ("pir", name="pir", at=(1.26, 3.8), facing="up")
        self.wire ("A12", "pir.OUT", via=[(3.65, 3.05), (1.89, 3.05)])
        self.wire ("5V.power", "pir.VCC", via=[(1.85, 2.9), (1.79, 2.9)])
        return self.wire ("GND.power", "pir.GND")

    # The tap sensor below the Mega's left end: S on A12, + and − from the
    # power header.
    @staged ("the tap sensor")
    def home_tap (self):
        self.module ("sensor", name="tap", at=(0.43, 3.62), pins=["S", "+", "−"],
                     label="tap sensor", facing="up")
        self.wire ("A12", "tap.S", via=[(3.66, 2.95), (0.85, 2.95)])
        self.wire ("5V.power", "tap.+", via=[(1.86, 2.9), (0.75, 2.9)])
        return self.wire ("GND.power", "tap.−", via=[(1.96, 2.85), (0.65, 2.85)])

    # The beam-break sensor below the board: S on A15, + and − from the
    # bottom rails in columns 36 and 37.
    @staged ("the beam-break sensor")
    def home_beam (self):
        self.module ("sensor", name="beam", at=(8.58, 3.45), label="beam-break sensor",
                     pins=("−", "+", "S"), facing="up")
        self.wire ("A15", "beam.S")
        self.wire ("beam.+", "B+36")
        return self.wire ("beam.−", "B-37")

    # Whether a part of this name is on the bench.
    def _has (self, name):
        return any (part.name == name for part in self.parts)

    # A module standing in one row on its own header, pins listed left to
    # right as they meet the columns.
    def header_module (self, kind, pins=None, first=1, row="j", name=None, label=None, **options):
        part = self._header_part (kind, pins, first, row, name, label, options)
        edge = "bottom" if part.turned else "top"
        self._step (Step (part.name, tuple ((hole, pin.name) for pin, hole in part.pairs ()),
                          f"standing in row {row}, its board toward the {edge} edge",
                          spread=True))
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
    # The breadboard power module, for motors and servos, or 3.3 V for LoRa
    # modems. The pins underneath it fit the kit's breadboard the right way
    # round only at the end nearest the Mega, which the Mega's own wires
    # need; turned round for the far end, its supply would meet the Mega's
    # GND. So it lies beside the far end instead, both its jumpers off so
    # those pins carry nothing, and two wires from the header in its middle
    # feed the bottom rails: supply ("5V" or "3.3V") into B+42, GND into
    # B-42. Its board sits above the LCD's overhang. The Mega's 5V feeds
    # the top rails, for the screen and sensors.
    @staged ("the power module")
    def power_module (self, supply="5V"):
        if supply not in ("5V", "3.3V"):
            raise ValueError (f"the power module gives 5V or 3.3V, not {supply!r}")
        self.last = max (self.last, 63)
        bx, by = self.board_origin ()
        self.module ("power_module", "power", at=(bx + self.board_width () + 0.3, by - 0.3),
                     facing="left",
                     detail="not plugged in: both its jumpers off, each parked on one pin")
        self.wire (f"power.{supply}", "B+42")
        return self.wire ("power.GND", "B-42")

    # Modules beside the board -------------------------------------------

    # A module placed with its top-left corner at (x, y) inches, its pins
    # facing the board unless facing says otherwise (down, up, left, right).
    # detail is anything more its build step needs to say.
    def module (self, kind, name=None, at=(0.0, 0.0), pins=None, label=None, facing=None,
                detail="", **options):
        name = name or kind
        if name in self.modules or "." in name:
            raise ValueError (f"a module needs a new name without a dot, not {name!r}")
        made = make_module (kind, pins, label, **options)
        angle = FACING[facing or self._facing (at, made)]
        placed = Placed (made, name, 0, 0, angle, reach=STUB if made.reach is None else made.reach)
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
        if self._stage and self._stage.name is None:
            self._stage = None
        self._step (Step (placed.title, ((self._whereabouts (placed.box ()), ""),), detail,
                          spread=True))
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

    # Where a module lies, in words: above or below the Mega, the breadboard
    # or the gap between them, as the kit page puts each one's home.
    def _whereabouts (self, box):
        x0, y0, x1, y1 = (v / DPI for v in box)
        bx, by = self.board_origin ()
        mx, _ = self.mega_origin ()
        middle = (x0 + x1) / 2
        over = "the gap between the Mega and the breadboard" \
            if mx + MEGA_WIDTH - 0.15 <= middle <= bx + 0.15 else \
            "the Mega" if middle < bx - self.gap / 2 else "the breadboard"
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
    # 5V from T+61 to B+61. The power module, beside the far end, feeds the
    # bottom rails itself, and the Mega's GND still joins them at B-3. The
    # site calls this once circuit.py has run; circuits never wire the Mega's
    # power to a rail. Then it checks the circuit could work, and places the
    # probes.
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
        module = any (isinstance (placed.kind, PowerModule) for placed in self.modules.values ())
        used = {parse_hole (hole)[2] for hole in self.used if parse_hole (hole)[0] == "rail"}
        if not used:
            return
        self._stage = Stage ("power to the rails")
        power = [self._power_wire (("GND.long2", "GND.long"), "B-3")]
        # The Mega's 5V feeds the top rails, and the bottom ones too unless
        # the power module does: the screen and sensors run from the Mega,
        # as their signals do, and the module feeds only the bottom rails,
        # for motors.
        if "T+" in used or ("B+" in used and not module):
            power.append (self._power_wire (("5V.long2", "5V.long"), "T+3"))
        links = []
        # The links stay just left of the LCD, clear of the power module's
        # feed at B-42. They keep their holes when a screen is added.
        for rail, source, a, b in (("T-", "GND", "B-41", "T-41"),
                                   ("B+", "5V", "T+42", "B+42"),
                                   ("T+", "5V", None, None), ("B-", "GND", None, None)):
            if rail not in used or self._powered (rail, source) \
                    or (source == "5V" and self._powered (rail, "3.3V")):
                continue
            if not a:
                side = "top" if rail[0] == "T" else "bottom"
                raise ValueError (f"nothing feeds the {side} {rail[1]} rail: wire it from a part "
                                  f"that does")
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
            or holes in ({"B-41", "T-41"}, {"T+42", "B+42"})

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

    # Whether a net is fed with GND, 5V or 3.3V: by the Mega's pin, or by the
    # power module's; a module that only uses it doesn't count.
    def _carries (self, net, source):
        feeds = {m for m, gives in self._sources ().items () if gives == source}
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
        if not leads:
            males = sum (self.style (end) == "male" for end in (start_end, end_end))
            kit = JUMPERS if males == 0 else RIBBON
            if color not in kit:
                raise ValueError (f"the {self.describe (start_end)} to {self.describe (end_end)} "
                                  f"wire can't be {color}: the kit's "
                                  f"{'jumpers' if males == 0 else 'female-ended wires'} come in "
                                  f"{', '.join (sorted (kit))}")
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
                return Step ("lead", (self.place (lead), self.place (other)), colors=(color,))
        males = sum (self.style (end) == "male" for end in (start, end))
        jumper = ("", "female-to-male", "female-to-female")[males]
        return Step ("wire", (self.place (start), self.place (end)), jumper, (color,))

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
        if pin in ("+", "VCC", "5V", "+5V", "3.3V", "VDD", "R"):
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

    # Where a wire's end goes, as a build step gives it: the pin, hole or
    # module pin to look for, and what tells it apart.
    def place (self, end):
        kind, name = end
        if kind == "module":
            placed, pin = self.module_pin (name)
            if pin.style == "lead":
                return placed.title, pin.note
            noun = {"male": "pin", "female": "socket", "screw": "terminal"}[pin.style]
            owner = f"{placed.title}{' plug' if pin.style == 'female' else ''}"
            return f"{owner}'s {pin.name} {noun}", pin.note or ""
        if kind == "hole":
            return rail_words (name) if parse_hole (name)[0] == "rail" else (name, "")
        label = canonical (name)
        pin = MEGA_PINS[name]
        if numbered (label):
            return f"pin {label}", ""
        if pin.header == "double":
            return label, (f"{'outer' if pin.outer else 'inner'} pin at the "
                           f"{'end' if pin.y < 1 else 'top'} of the long header")
        if pin.header == "top":
            return label, "beside pin 13" if label == "GND" else "at the left end of the top header"
        return label, "on the power header"

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
            return hole_words (name)
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
            holes = [hole_words (hole) for _, hole in part.legs ()]
            between = "–" if all (" " not in hole for hole in holes) else " to "
            where = f"{holes[0]}{between}{holes[-1]}" if len (holes) == 2 else f"at {holes[0]}"
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

    # Module pins that feed power, such as the power module's 5V, listed with
    # the Mega's pins: {net member: "GND", "5V" or "3.3V"}.
    def _sources (self):
        return {f"{placed.title}: {pin.label ()}": placed.kind.sources[pin.name]
                for placed in self.modules.values () for pin in placed.pins ()
                if pin.name in placed.kind.sources}

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
    text = open (path, encoding="utf-8").read ()
    exec (compile (text, path, "exec"), scope)
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
                              "for examples/lessons/NNN-name/Dial/Dial.ino")
    digest = hashlib.sha256 (text.encode ()).hexdigest ()[:16]
    for letter, bench in boards.items ():
        bench.board = letter
        bench.source = digest + letter
        bench.finish ()
    return boards


# The folder a lesson's example is in, under examples: lessons/013-hello-lcd
# for docs/lessons/013-hello-lcd. A one-board lesson's sketch is in it, named
# the same; a two-board lesson's are in folders of their own inside it.
def example (slug):
    return f"lessons/{slug}"


# Called for every hole a drawing or a probe looks at, so each name is read
# once.
@functools.lru_cache (maxsize=None)
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


# A rail's hole in words, as a page gives it: "bottom − rail", "by column
# 3". Never by its name here, B-3, whose B reads as row b and whose − as a
# minus. Every hole along a rail joins the rest; the column only keeps a
# wire short, as the drawing shows it.
def rail_words (hole):
    _, column, rail = parse_hole (hole)
    side = "top" if rail[0] == "T" else "bottom"
    return f"{side} {'+' if rail[1] == '+' else '−'} rail", f"by column {column}"


# Any hole in running text: "j6", or "the bottom − rail by column 3".
def hole_words (hole):
    return "the " + " ".join (rail_words (hole)) if parse_hole (hole)[0] == "rail" else hole


# Pins in a few words, in order and runs of three or more closed up:
# "pin 26", "pins 22 and 23", "pins 31–36 and A0".
def pin_words (pins):
    order = sorted (pins, key=lambda pin: (pin.startswith ("A"), int (pin.lstrip ("A"))))
    runs = []
    for pin in order:
        last = runs[-1][-1] if runs else ""
        if last and pin.startswith ("A") == last.startswith ("A") and \
                int (pin.lstrip ("A")) == int (last.lstrip ("A")) + 1:
            runs[-1].append (pin)
        else:
            runs.append ([pin])
    words = [f"{run[0]}–{run[-1]}" if len (run) > 2 else ", ".join (run) for run in runs]
    words = ", ".join (words).rsplit (", ", 1)
    return ("pins " if len (order) > 1 else "pin ") + " and ".join (words)


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
