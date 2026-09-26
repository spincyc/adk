"""Modules: the servo, the ultrasonic sensor, the LED matrix and the other
boards that sit beside the breadboard on jumper wires, or stand in one of
its rows on their own header.

Each kind is drawn in its own frame, in drawing units (DPI to the inch),
at the real board's size, with its main header along the bottom edge. A
Placed module moves and turns that frame into the scene, and knows where
each of its pins ends up.
"""

import math

from pencil import DPI, WIRES, rounded_rectangle
from route import text_box

SIDES = {"top": (0, -1), "bottom": (0, 1), "left": (-1, 0), "right": (1, 0)}
PITCH = 0.1 * DPI                   # header pins are 0.1 inch apart
STUB = 24                           # how far a male header pin sticks out
HOUSING = 57                        # a jumper's female housing, 14.5 mm

PCB_BLUE  = "#c5d4e6"
PCB_GREEN = "#c8dbbd"
PCB_PURPLE = "#d8c8e2"
METAL     = "#d3d3cf"
GOLD      = "#dcbb62"
COPPER    = "#c98f58"
PLASTIC   = "#3a3a3a"
WHITE     = "#f4f2ec"
LED_RED   = "#f0604f"


class Pin:
    """One pin of a module, in the module's own frame. The direction points
    away from the board, the way a wire leaves it.

    style: male (a header pin; a jumper's female end goes over it), female
    (a socket; a jumper's male end goes in), lead (the part's own wire,
    which goes straight to where it is wired) or screw (a screw terminal).
    """

    def __init__ (self, name, x, y, direction=(0, 1), style="male", note=None, color=None):
        self.name, self.x, self.y = name, x, y
        self.direction = direction
        self.style = style
        self.note = note
        self.color = color

    def label (self):
        return f"{self.name} ({self.note})" if self.note else self.name


class Kind:
    """A module with a row of header pins along its bottom edge."""
    title  = "module"
    pins   = ()
    notes  = {}
    width, height = 80, 80
    style  = "male"
    color  = PCB_BLUE
    flexible = False                # takes any row of pins
    supply = "5V"                   # what its +, VCC or VDD pin takes
    # How a sketch claims the Mega pin wired to each pin it names: "output"
    # for a pin the Mega drives, "input" for one it reads. A pin left out
    # may be either, as a bus's are.
    pin_modes = {}
    inset  = 0                      # how far in from the edge the header sits
    title_side, title_gap = "top", 8

    def __init__ (self, pins=None, label=None, **options):
        if pins is not None and not self.flexible and len (pins) != len (self.pins):
            raise ValueError (f"a {self.title} has {len (self.pins)} pins: {' '.join (self.pins)}")
        self.names = list (pins) if pins is not None else list (self.pins)
        self.label = label
        self.options = options
        if self.flexible:
            self.width = max (self.width, len (self.names) * PITCH + 24)
        self.title = label or self.title

    def header_x (self):
        return self.width / 2

    def header_y (self):
        return self.height

    def header (self):
        x0 = self.header_x () - (len (self.names) - 1) * PITCH / 2
        return [Pin (name, x0 + index * PITCH, self.header_y (), (0, 1), self.style,
                     self.notes.get (name)) for index, name in enumerate (self.names)]

    def extra (self):
        return []

    def all_pins (self):
        return self.header () + self.extra ()

    # The box the body fills, in its own frame.
    def body (self):
        return 0, 0, self.width, self.height

    def draw (self, pencil):
        board (pencil, 0, 0, self.width, self.height, self.color)
        pencil.text (self.width / 2, self.height / 2, self.title, size=8, kind="silk")
        self.draw_header (pencil)

    # The black plastic strip of a male header, and its printed pin names.
    def draw_header (self, pencil, labels=True, size=6.2):
        pins = self.header ()
        if self.style != "male" or not pins:
            return
        left, right = pins[0].x - 5, pins[-1].x + 5
        y = pins[0].y
        pencil.tint ([(left, y - 7), (right, y - 7), (right, y), (left, y)], PLASTIC)
        pencil.rect (left, y - 7, right - left, 7, width=0.7, passes=1)
        if labels:
            pin_labels (pencil, pins, y - 10, size)


# Pieces -------------------------------------------------------------------

def board (pencil, x, y, w, h, color, holes=(), radius=5):
    corners = rounded (x, y, w, h, radius)
    pencil.tint (corners, color)
    pencil.rect (x, y, w, h, width=1.0, radius=radius, layer="top")
    for hx, hy in holes:
        pencil.spot (hx, hy, 4.2, "#fbf9f3")
        pencil.circle (hx, hy, 4.2, width=0.8, layer="top", passes=1)
        pencil.circle (hx, hy, 6.2, width=0.6, tone=0.4, layer="top", passes=1)


def rounded (x, y, w, h, r):
    return rounded_rectangle (x, y, w, h, r) if r else [(x, y), (x + w, y), (x + w, y + h), (x, y + h)]


def ic (pencil, cx, cy, w, h, legs=0, notch=True):
    for index in range (legs):
        lx = cx - w / 2 + (index + 0.5) * w / legs
        for side in (-1, 1):
            pencil.tint ([(lx - 1.2, cy + side * h / 2), (lx + 1.2, cy + side * h / 2),
                          (lx + 1.2, cy + side * (h / 2 + 3)), (lx - 1.2, cy + side * (h / 2 + 3))],
                         METAL)
    pencil.tint (rounded (cx - w / 2, cy - h / 2, w, h, 1.5), PLASTIC)
    pencil.rect (cx - w / 2, cy - h / 2, w, h, width=0.9, radius=1.5, layer="top", passes=1)
    if notch:
        pencil.spot (cx - w / 2 + 3.5, cy + h / 2 - 3.5, 1.2, "#8a8a8a")


def smd (pencil, cx, cy, w=7, h=3.5, color="#6b6257"):
    pencil.tint ([(cx - w / 2, cy - h / 2), (cx + w / 2, cy - h / 2),
                  (cx + w / 2, cy + h / 2), (cx - w / 2, cy + h / 2)], color)
    for side in (-1, 1):
        pencil.tint ([(cx + side * w / 2, cy - h / 2), (cx + side * (w / 2 - 1.8), cy - h / 2),
                      (cx + side * (w / 2 - 1.8), cy + h / 2), (cx + side * w / 2, cy + h / 2)], METAL)


def led_dot (pencil, cx, cy, lit=False, r=3.2):
    pencil.spot (cx, cy, r, LED_RED if lit else "#f0b2aa")
    pencil.circle (cx, cy, r, width=0.7, layer="top", passes=1)
    if lit:
        pencil.spot (cx, cy, r * 2.2, LED_RED, opacity=0.25)


def screw (pencil, cx, cy, r=6.5):
    pencil.spot (cx, cy, r, METAL)
    pencil.circle (cx, cy, r, width=0.9, layer="top", passes=1)
    pencil.line ((cx - r * 0.6, cy + r * 0.3), (cx + r * 0.6, cy - r * 0.3), width=1.2, layer="top",
                 passes=1)


def pin_labels (pencil, pins, y, size=6.2):
    # Short names sit level; longer ones stand on end, as boards print them.
    upright = max (len (pin.name) for pin in pins) <= 2
    for pin in pins:
        if upright:
            pencil.text (pin.x, y + 1, pin.name, size=size + 0.8, kind="silk")
        else:
            pencil.text (pin.x + size * 0.36, y, pin.name, size=size, rotate=-90, anchor="start",
                         kind="silk")


def grid_in_circle (pencil, cx, cy, r, gap, tone=0.35):
    offset = -r + gap / 2
    while offset < r:
        half = math.sqrt (max (0.0, r * r - offset * offset))
        pencil.line ((cx - half, cy + offset), (cx + half, cy + offset), width=0.5, tone=tone,
                     layer="top", passes=1, wobble=0.2)
        pencil.line ((cx + offset, cy - half), (cx + offset, cy + half), width=0.5, tone=tone,
                     layer="top", passes=1, wobble=0.2)
        offset += gap


def level (pencil, x, y, text, size, **options):
    # Printing that stands level, centred on y however the module is
    # turned: turned half round, silk swings over about its baseline.
    turn = sum (pencil.turns) % 360
    rise = size * 0.36 if not 90 < turn <= 270 else -size * 0.36
    pencil.text (x, y + rise, text, size=size, kind="silk", **options)


def hole (pencil, cx, cy, r=3.2, square=False):
    # A plated hole or pad, tinned silver.
    if square:
        pencil.tint ([(cx - r, cy - r), (cx + r, cy - r), (cx + r, cy + r), (cx - r, cy + r)], METAL)
        pencil.rect (cx - r, cy - r, 2 * r, 2 * r, width=0.6, layer="top", passes=1)
    else:
        pencil.spot (cx, cy, r, METAL)
        pencil.circle (cx, cy, r, width=0.6, layer="top", passes=1)
    pencil.spot (cx, cy, r * 0.45, "#8a8a86")


def coil (pencil, x, bottom, top, r, turns, color=METAL):
    # A spring antenna standing on its axis at x, from bottom up to top: each
    # turn a bright stroke across the front and a fainter one behind.
    pitch = (bottom - top) / turns
    for index in range (turns):
        y = bottom - index * pitch
        pencil.line ((x + r, y - pitch / 2), (x - r, y - pitch), width=0.7, tone=0.4, layer="top",
                     passes=1, wobble=0.1)
    for index in range (turns):
        y = bottom - index * pitch
        pencil.wire ([(x - r, y), (x + r, y - pitch / 2)], color, width=1.3, layer="top")


def sma (pencil, x, y, length):
    # A gold SMA socket standing on (x, y), pointing up: a hex nut, then the
    # threaded barrel an antenna screws onto.
    pencil.tint ([(x - 15, y), (x + 15, y), (x + 15, y - 9), (x - 15, y - 9)], GOLD)
    pencil.rect (x - 15, y - 9, 30, 9, width=0.8, layer="top", passes=1)
    for dx in (-5, 5):
        pencil.line ((x + dx, y), (x + dx, y - 9), width=0.5, tone=0.5, layer="top", passes=1)
    pencil.tint ([(x - 12, y - 9), (x + 12, y - 9), (x + 12, y - length), (x - 12, y - length)], GOLD)
    pencil.rect (x - 12, y - length, 24, length - 9, width=0.8, layer="top", passes=1)
    thread = y - 12
    while thread > y - length + 2:
        pencil.line ((x - 12, thread), (x + 12, thread - 1.5), width=0.5, tone=0.45, layer="top",
                     passes=1, wobble=0.05)
        thread -= 3.2


def whip (pencil, x, bottom, top, width):
    # A black rubber antenna's whip, tapering a little to a rounded tip.
    half, tip = width / 2, width / 2 - 2
    outline = [(x - half, bottom), (x + half, bottom), (x + tip, top + tip)]
    outline += [(x + tip * math.cos (math.pi * step / 8),
                 top + tip - tip * math.sin (math.pi * step / 8)) for step in range (1, 8)]
    outline += [(x - tip, top + tip), (x - half, bottom)]
    pencil.tint (outline, "#2e2e2e")
    pencil.polyline (outline, width=1.0, closed=True, layer="top", passes=1)
    pencil.line ((x - half * 0.4, bottom - 4), (x - tip * 0.4, top + tip + 2), width=1.2, tone=0.2,
                 layer="top", passes=1)


def knurled (pencil, x, bottom, top, width):
    # The knurled black nut at the foot of a rubber antenna.
    pencil.tint (rounded (x - width / 2, top, width, bottom - top, 2), "#262626")
    pencil.rect (x - width / 2, top, width, bottom - top, width=0.9, radius=2, layer="top", passes=1)
    for index in range (1, 6):
        lx = x - width / 2 + index * width / 6
        pencil.line ((lx, bottom - 2), (lx, top + 2), width=0.5, tone=0.35, layer="top", passes=1)


def ribbon (pencil, start, end, colors, spread_start, spread_end, width=3.0):
    # Strands side by side, fanning from one spacing to another.
    (x1, y1), (x2, y2) = start, end
    count = len (colors)
    for index, color in enumerate (colors):
        a = x1 + (index - (count - 1) / 2) * spread_start
        b = x2 + (index - (count - 1) / 2) * spread_end
        pencil.wire ([(a, y1), (a, y1 + (y2 - y1) * 0.35), (b, y1 + (y2 - y1) * 0.7), (b, y2)],
                     color, width=width)


# The kinds ----------------------------------------------------------------

class Servo (Kind):
    # SG90: a 22.5 x 12 mm case with mounting ears, and a cable of brown,
    # red and orange ending in a three-way socket.
    title  = "servo"
    pins   = ("−", "+", "signal")
    notes  = {"−": "brown", "+": "red", "signal": "orange"}
    pin_modes = {"signal": "output"}
    width, height = 150, 160
    style  = "female"

    def header_y (self):
        return self.height

    def draw (self, pencil):
        for ex in (8, 120):
            pencil.tint (rounded (ex, 24, 22, 26, 3), "#b4cdec")
            pencil.rect (ex, 24, 22, 26, width=1.0, radius=3, layer="top", passes=1)
            pencil.circle (ex + (7 if ex < 50 else 15), 37, 3.2, width=0.8, layer="top", passes=1)
        pencil.tint (rounded (28, 14, 92, 46, 4), "#8fb3e0")
        pencil.rect (28, 14, 92, 46, width=1.0, radius=4, layer="top")
        pencil.hatch (30, 44, 88, 14, gap=3.2, angle=20, tone=0.25, layer="top")
        pencil.circle (73, 37, 9, width=0.9, layer="top", passes=1)
        pencil.spot (48, 37, 19, "#a9c6ea")
        pencil.circle (48, 37, 19, width=1.2, layer="top")
        # The horn: a white arm on the output shaft.
        arm = [(48 + 36 * math.cos (a) + 5.5 * math.cos (a + math.pi / 2) * s,
                37 + 36 * math.sin (a) + 5.5 * math.sin (a + math.pi / 2) * s)
               for a, s in ((-2.6, 1), (-2.6, -1), (0.55, -1), (0.55, 1))]
        arm = [arm[0], arm[1], arm[3], arm[2]]
        pencil.tint (arm, WHITE)
        pencil.polyline (arm, width=1.1, closed=True, layer="top")
        for t in (-0.75, -0.5, 0.5, 0.75):
            angle = -2.6 if t < 0 else 0.55
            pencil.circle (48 + 36 * abs (t) * math.cos (angle), 37 + 36 * abs (t) * math.sin (angle),
                           1.4, width=0.6, layer="top", passes=1)
        pencil.spot (48, 37, 5, METAL)
        pencil.circle (48, 37, 5, width=0.9, layer="top", passes=1)
        pencil.text (96, 33, "SG90", size=8, kind="silk", weight="bold")
        ribbon (pencil, (110, 58), (75, 108), ["#8a5a36", "#c8423b", "#dc7c2e"], 3.6, 10)
        pencil.tint (rounded (59, 106, 32, 54, 2), "#262626")
        pencil.rect (59, 106, 32, 54, width=1.0, radius=2, layer="top", passes=1)
        for index in range (3):
            pencil.tint ([(63 + 10 * index, 114), (67 + 10 * index, 114), (67 + 10 * index, 124),
                          (63 + 10 * index, 124)], "#8c8c8c")


class Ultrasonic (Kind):
    # HC-SR04: 45 x 20 mm, two 16 mm transducers 26 mm apart.
    title  = "ultrasonic sensor"
    pins   = ("VCC", "Trig", "Echo", "GND")
    pin_modes = {"Trig": "output", "Echo": "input"}
    width, height = 177, 79

    def draw (self, pencil):
        board (pencil, 0, 0, self.width, self.height, self.color,
               holes=[(5, 5), (172, 5), (5, 74), (172, 74)], radius=3)
        for cx, letter in ((38, "T"), (139, "R")):
            pencil.spot (cx, 38, 31, METAL)
            pencil.circle (cx, 38, 31, width=1.0, layer="top")
            pencil.circle (cx, 38, 27, width=0.8, layer="top", passes=1)
            pencil.spot (cx, 38, 24, "#4a4a4a")
            grid_in_circle (pencil, cx, 38, 24, 3.4, tone=0.3)
            pencil.spot (cx - 8, 30, 7, "#ffffff", opacity=0.25)
            pencil.text (cx + (36 if letter == "T" else -36), 74, letter, size=8, kind="silk")
        pencil.tint (rounded (77, 6, 23, 10, 5), METAL)
        pencil.rect (77, 6, 23, 10, width=0.9, radius=5, layer="top", passes=1)
        pencil.text (88.5, 26, "HC-SR04", size=6.5, kind="silk", weight="bold")
        self.draw_header (pencil, size=5.8)


class Matrix (Kind):
    # A MAX7219 module: 32 x 50 mm, a 32 mm square 1088AS matrix of 8 x 8
    # LEDs, the input header at one end and the output at the other.
    title  = "LED matrix"
    pins   = ("VCC", "GND", "DIN", "CS", "CLK")
    title_gap = 32                  # clear of the output header
    pin_modes = {"DIN": "output", "CS": "output", "CLK": "output"}
    width, height = 126, 197

    def draw (self, pencil):
        board (pencil, 0, 0, self.width, self.height, self.color, radius=3)
        for x in (43, 53, 63, 73, 83):
            pencil.line ((x, 0), (x, -STUB), width=2.4, tone=0.5, layer="top", passes=1, wobble=0.1)
        pencil.tint ([(38, 0), (88, 0), (88, 7), (38, 7)], PLASTIC)
        pixels = self.options.get ("pixels") or []
        top = 36
        pencil.tint ([(0, top), (126, top), (126, top + 126), (0, top + 126)], "#3b3b3b")
        pencil.rect (0, top, 126, 126, width=1.0, layer="top")
        for row in range (8):
            for column in range (8):
                x, y = 7.9 + column * 15.75, top + 7.9 + row * 15.75
                lit = row < len (pixels) and column < len (pixels[row]) \
                    and pixels[row][column] in "#1Xx*"
                if lit:
                    pencil.spot (x, y, 9, LED_RED, opacity=0.35)
                pencil.spot (x, y, 5.3, LED_RED if lit else "#e3dfd6")
        pencil.text (63, 27, "MAX7219", size=6.5, kind="silk", weight="bold")
        self.draw_header (pencil, size=5.8)


class Joystick (Kind):
    # KY-023: a thumbstick on a 26 x 34 mm board.
    title  = "joystick"
    pins   = ("GND", "+5V", "VRx", "VRy", "SW")
    pin_modes = {"VRx": "input", "VRy": "input", "SW": "input"}
    width, height = 102, 150

    def draw (self, pencil):
        board (pencil, 0, 0, self.width, self.height, self.color,
               holes=[(8, 8), (94, 8), (8, 104), (94, 104)])
        pencil.tint (rounded (13, 16, 76, 80, 4), "#4a4a4a")
        pencil.rect (13, 16, 76, 80, width=1.2, radius=4, layer="top", passes=1)
        for bx, by, bw, bh in ((2, 42, 11, 28), (37, 96, 28, 10)):
            pencil.tint (rounded (bx, by, bw, bh, 2), "#6f9ad6")
            pencil.rect (bx, by, bw, bh, width=0.8, radius=2, layer="top", passes=1)
        pencil.spot (51, 56, 35, "#2e2e2e")
        pencil.circle (51, 56, 35, width=1.0, layer="top")
        pencil.circle (51, 56, 24, width=0.9, tone=0.5, layer="top", passes=1)
        for index in range (20):
            angle = index * math.pi / 10
            pencil.line ((51 + 26 * math.cos (angle), 56 + 26 * math.sin (angle)),
                         (51 + 33 * math.cos (angle), 56 + 33 * math.sin (angle)),
                         width=0.6, tone=0.35, layer="top", passes=1, wobble=0.2)
        pencil.spot (41, 45, 12, "#ffffff", opacity=0.18)
        self.draw_header (pencil, size=5.8)


class Keypad (Kind):
    # A 4 x 4 membrane keypad, 69 x 77 mm, on an eight-way ribbon that ends
    # in a socket: rows 1-4 then columns 1-4.
    title  = "keypad"
    pins   = ("R1", "R2", "R3", "R4", "C1", "C2", "C3", "C4")
    notes  = {"R1": "row 1", "R2": "row 2", "R3": "row 3", "R4": "row 4",
              "C1": "column 1", "C2": "column 2", "C3": "column 3", "C4": "column 4"}
    width, height = 272, 445
    style  = "female"
    KEYS   = ("123A", "456B", "789C", "*0#D")

    def draw (self, pencil):
        pencil.tint (rounded (0, 0, 272, 303, 8), "#6d7075")
        pencil.rect (0, 0, 272, 303, width=1.0, radius=8, layer="top")
        for row, keys in enumerate (self.KEYS):
            for column, key in enumerate (keys):
                x, y = 15 + column * 64, 22 + row * 70
                tint = "#eaa29a" if key in "ABCD" else "#a9c3e6" if key in "*#" else WHITE
                pencil.tint (rounded (x, y, 50, 54, 6), tint)
                pencil.rect (x, y, 50, 54, width=1.0, radius=6, layer="top", passes=1)
                pencil.text (x + 25, y + 36, key, size=24, kind="silk", weight="bold", tone=0.8)
        pencil.tint ([(96, 303), (176, 303), (176, 393), (96, 393)], "#dedbd2")
        for index in range (9):
            pencil.line ((96 + 10 * index, 303), (96 + 10 * index, 393), width=0.5, tone=0.4,
                         layer="top", passes=1)
        for pin in self.header ():
            pencil.text (pin.x + 2, 386, pin.name, size=6.2, rotate=-90, anchor="start", kind="silk")
        pencil.tint (rounded (94, 393, 84, 52, 2), "#262626")
        pencil.rect (94, 393, 84, 52, width=1.0, radius=2, layer="top", passes=1)


class IrReceiver (Kind):
    # KY-022: a VS1838B receiver on a small board.
    title  = "IR receiver"
    pins   = ("S", "+", "−")
    pin_modes = {"S": "input"}
    width, height = 62, 80

    def draw (self, pencil):
        board (pencil, 0, 0, self.width, self.height, self.color, holes=[(8, 8)])
        pencil.tint (rounded (17, 6, 28, 36, 3), "#333333")
        pencil.rect (17, 6, 28, 36, width=1.1, radius=3, layer="top", passes=1)
        pencil.dome (31, 22, 10, "#4a4a4a")
        led_dot (pencil, 50, 50)
        smd (pencil, 16, 50)
        self.draw_header (pencil)


class Rfid (Kind):
    # RC522: 40 x 60 mm, an antenna coil round most of the board.
    title  = "RFID reader"
    pins   = ("SDA", "SCK", "MOSI", "MISO", "IRQ", "GND", "RST", "3.3V")
    width, height = 157, 236

    def draw (self, pencil):
        board (pencil, 0, 0, self.width, self.height, self.color,
               holes=[(10, 10), (147, 10), (10, 160), (147, 160)])
        for turn in range (4):
            inset = 22 + turn * 6
            pencil.rect (inset, inset - 6, 157 - 2 * inset, 150 - 2 * inset, width=1.0, tone=0.45,
                         radius=12, layer="top", passes=1)
        ic (pencil, 78, 182, 22, 22, notch=True)
        pencil.tint (rounded (108, 176, 24, 10, 5), METAL)
        pencil.rect (108, 176, 24, 10, width=0.8, radius=5, layer="top", passes=1)
        for sx, sy in ((40, 176), (40, 188), (116, 196)):
            smd (pencil, sx, sy)
        pencil.text (78, 84, "RFID-RC522", size=9, kind="silk", weight="bold")
        self.draw_header (pencil, size=5.8)


class Gy521 (Kind):
    # GY-521: an MPU-6050 on a 21 x 16 mm board.
    title  = "GY-521"
    pins   = ("VCC", "GND", "SCL", "SDA", "XDA", "XCL", "AD0", "INT")
    width, height = 83, 63

    def draw (self, pencil):
        board (pencil, 0, 0, self.width, self.height, self.color, holes=[(8, 8), (75, 8)], radius=3)
        ic (pencil, 41.5, 22, 15, 15)
        pencil.line ((20, 36), (20, 22), width=0.8, layer="top", passes=1)
        pencil.line ((20, 36), (32, 36), width=0.8, layer="top", passes=1)
        pencil.text (20, 19, "Y", size=5, kind="silk")
        pencil.text (36, 38, "X", size=5, kind="silk")
        for sx in (56, 64, 70):
            smd (pencil, sx, 20, 5, 2.6)
        pencil.text (62, 33, "GY-521", size=5.2, kind="silk", weight="bold")
        self.draw_header (pencil, size=5.4)


class Rtc (Kind):
    # A DS1307 clock module: its chip, crystal and CR2032 backup cell.
    title  = "clock module"
    pins   = ("GND", "VCC", "SDA", "SCL", "SQW")
    width, height = 120, 100

    def draw (self, pencil):
        board (pencil, 0, 0, self.width, self.height, self.color, holes=[(112, 8)])
        pencil.spot (40, 38, 33, METAL)
        pencil.circle (40, 38, 33, width=1.0, layer="top")
        pencil.circle (40, 38, 27, width=0.7, tone=0.45, layer="top", passes=1)
        pencil.text (40, 36, "CR2032", size=7, kind="silk")
        pencil.text (40, 49, "+", size=10, kind="silk")
        ic (pencil, 94, 26, 22, 15, legs=4)
        pencil.tint (rounded (85, 46, 20, 8, 4), METAL)
        pencil.rect (85, 46, 20, 8, width=0.8, radius=4, layer="top", passes=1)
        pencil.text (94, 66, "DS1307", size=5.5, kind="silk", weight="bold")
        self.draw_header (pencil)


class Relay (Kind):
    # KY-019: a 5 V relay, its three screw terminals at the far end.
    title  = "relay"
    pins   = ("S", "+", "−")
    title_side = "right"            # wires leave its terminals at the top
    pin_modes = {"S": "output"}
    width, height = 102, 134
    TERMINALS = ("NO", "COM", "NC")

    def extra (self):
        return [Pin (name, 31 + 20 * index, 0, (0, -1), "screw")
                for index, name in enumerate (self.TERMINALS)]

    def draw (self, pencil):
        board (pencil, 0, 0, self.width, self.height, self.color, holes=[(8, 125), (94, 125)])
        pencil.tint ([(19, 0), (83, 0), (83, 30), (19, 30)], "#5f93cf")
        pencil.rect (19, 0, 64, 30, width=1.2, layer="top")
        for index, name in enumerate (self.TERMINALS):
            screw (pencil, 31 + 20 * index, 16)
            pencil.tint ([(26 + 20 * index, 0), (36 + 20 * index, 0), (36 + 20 * index, 5),
                          (26 + 20 * index, 5)], "#2a2a2a")
            pencil.text (31 + 20 * index, 38, name, size=5.5, kind="silk")
        pencil.tint (rounded (12, 42, 78, 56, 2), "#6d98d8")
        pencil.rect (12, 42, 78, 56, width=1.0, radius=2, layer="top")
        pencil.text (51, 64, "SONGLE", size=6.5, kind="silk", weight="bold", tone=0.7)
        pencil.text (51, 75, "SRD-05VDC-SL-C", size=4.8, kind="silk", tone=0.7)
        led_dot (pencil, 84, 108)
        ic (pencil, 24, 108, 8, 7, notch=False)
        self.draw_header (pencil)


class Stepper (Kind):
    # A ULN2003 driver board with its 28BYJ-48 motor on a five-colour cable.
    title  = "stepper driver"
    pins   = ("IN1", "IN2", "IN3", "IN4")
    pin_modes = {"IN1": "output", "IN2": "output", "IN3": "output", "IN4": "output"}
    width, height = 170, 290
    POWER = ("−", "+")

    def header_x (self):
        return 50

    def extra (self):
        return [Pin (name, 114 + 10 * index, self.height, (0, 1), "male")
                for index, name in enumerate (self.POWER)]

    def draw (self, pencil):
        # The motor: a 28 mm can on a flange, the shaft off centre.
        pencil.tint (rounded (4, 51, 162, 22, 11), METAL)
        pencil.rect (4, 51, 162, 22, width=1.1, radius=11, layer="top", passes=1)
        for ex in (15, 155):
            pencil.spot (ex, 62, 4, "#fbf9f3")
            pencil.circle (ex, 62, 4, width=0.8, layer="top", passes=1)
        pencil.spot (85, 62, 55, "#cfd2d4")
        pencil.circle (85, 62, 55, width=1.0, layer="top")
        pencil.circle (85, 62, 49, width=0.5, tone=0.3, layer="top")
        pencil.spot (85, 30, 17, "#e2e3e2")
        pencil.circle (85, 30, 17, width=1.0, layer="top", passes=1)
        pencil.spot (85, 30, 5, "#9a9a9a")
        pencil.circle (85, 30, 5, width=0.9, layer="top", passes=1)
        pencil.text (85, 80, "28BYJ-48", size=7, kind="silk", weight="bold")
        pencil.tint (rounded (52, 108, 66, 22, 4), "#7ea3d8")
        pencil.rect (52, 108, 66, 22, width=1.1, radius=4, layer="top", passes=1)
        ribbon (pencil, (85, 128), (85, 168),
                ["#3f6fb5", "#e79fb6", "#d8b32e", "#dc7c2e", "#c8423b"], 4.2, 9)
        # The driver board.
        board (pencil, 16, 160, 138, 130, PCB_GREEN,
               holes=[(24, 168), (146, 168), (24, 282), (146, 282)])
        pencil.tint ([(60, 164), (110, 164), (110, 182), (60, 182)], WHITE)
        pencil.rect (60, 164, 50, 18, width=1.0, layer="top", passes=1)
        ic (pencil, 70, 222, 76, 26, legs=8)
        pencil.text (70, 225, "ULN2003", size=7, kind="silk", tone=0.9, color="#f4f1e8")
        for index, letter in enumerate ("ABCD"):
            led_dot (pencil, 132, 196 + 14 * index, lit=letter in self.options.get ("lit", ""))
            pencil.text (142, 199 + 14 * index, letter, size=6, kind="silk")
        self.draw_header (pencil, size=5.8)
        power = self.extra ()
        left, right = power[0].x - 5, power[-1].x + 5
        pencil.tint ([(left, 283), (right, 283), (right, 290), (left, 290)], PLASTIC)
        for pin in power:
            pencil.text (pin.x, 279, pin.name, size=8, kind="silk")
        pencil.text (119, 268, "5-12V", size=5.5, kind="silk")


class Encoder (Kind):
    # KY-040: a rotary encoder with a knurled shaft, 19 x 32 mm.
    title  = "rotary encoder"
    pins   = ("CLK", "DT", "SW", "+", "GND")
    pin_modes = {"CLK": "input", "DT": "input", "SW": "input"}
    width, height = 75, 126

    def draw (self, pencil):
        board (pencil, 0, 0, self.width, self.height, self.color, holes=[(8, 8), (67, 8)])
        pencil.tint (rounded (13, 20, 49, 49, 2), METAL)
        pencil.rect (13, 20, 49, 49, width=1.0, radius=2, layer="top")
        pencil.spot (37.5, 44.5, 15, "#bdbdb8")
        pencil.circle (37.5, 44.5, 15, width=1.2, layer="top")
        for index in range (24):
            angle = index * math.pi / 12
            pencil.line ((37.5 + 10.5 * math.cos (angle), 44.5 + 10.5 * math.sin (angle)),
                         (37.5 + 14.5 * math.cos (angle), 44.5 + 14.5 * math.sin (angle)),
                         width=0.6, tone=0.5, layer="top", passes=1, wobble=0.15)
        pencil.circle (37.5, 44.5, 9.5, width=0.8, layer="top", passes=1)
        pencil.line ((31, 41), (44, 41), width=1.0, layer="top", passes=1)
        for sx in (20, 37, 54):
            smd (pencil, sx, 80, 7, 3.2)
        self.draw_header (pencil)


class Pir (Kind):
    # HC-SR501: 32 x 24 mm under a 23 mm Fresnel dome.
    title  = "PIR sensor"
    pins   = ("VCC", "OUT", "GND")
    pin_modes = {"OUT": "input"}
    width, height = 126, 96

    def draw (self, pencil):
        board (pencil, 0, 0, self.width, self.height, PCB_GREEN, holes=[(7, 7), (119, 7)])
        pencil.tint (rounded (18, 2, 90, 90, 4), "#ebe8df")
        pencil.rect (18, 2, 90, 90, width=1.0, radius=4, layer="top", passes=1)
        pencil.spot (63, 47, 43, "#f8f6f0")
        pencil.circle (63, 47, 43, width=1.0, layer="top")
        for r in (14, 27):
            pencil.circle (63, 47, r, width=0.6, tone=0.3, layer="top", passes=1)
        for index in range (12):
            angle = index * math.pi / 6
            pencil.line ((63 + 14 * math.cos (angle), 47 + 14 * math.sin (angle)),
                         (63 + 42 * math.cos (angle), 47 + 42 * math.sin (angle)),
                         width=0.6, tone=0.28, layer="top", passes=1, wobble=0.3)
        pencil.spot (50, 34, 14, "#ffffff", opacity=0.5)
        left, right = 48, 78
        pencil.tint ([(left, 89), (right, 89), (right, 96), (left, 96)], PLASTIC)
        for pin in self.header ():
            pencil.text (pin.x + 2, 83, pin.name, size=5.4, rotate=-90, anchor="end", kind="silk")


class Sensor (Kind):
    # One of the 37-in-1 kit's small boards: a sensing part, and for the
    # four-pin ones a comparator with its sensitivity trimmer.
    title  = "sensor"
    pins   = ("S", "+", "−")
    pin_modes = {"S": "input", "OUT": "input", "DO": "input", "AO": "input"}
    width, height = 64, 100
    flexible = True

    def draw (self, pencil):
        w = self.width
        board (pencil, 0, 0, w, self.height, self.color, holes=[(8, 8)])
        emblem = (self.label or "").lower ()
        cx = w / 2
        if any (word in emblem for word in ("sound", "mic", "clap")):
            pencil.spot (cx, 24, 15, METAL)
            pencil.circle (cx, 24, 15, width=1.0, layer="top")
            pencil.spot (cx, 24, 10, "#555555")
            grid_in_circle (pencil, cx, 24, 10, 3, tone=0.3)
        elif any (word in emblem for word in ("18b20", "temperature", "hall", "magnet")):
            pencil.tint ([(cx - 10, 32), (cx - 10, 18), (cx - 7, 12), (cx + 7, 12), (cx + 10, 18),
                          (cx + 10, 32)], PLASTIC)
            pencil.polyline ([(cx - 10, 32), (cx - 10, 18), (cx - 7, 12), (cx + 7, 12),
                              (cx + 10, 18), (cx + 10, 32)], width=1.0, closed=True, layer="top")
        elif any (word in emblem for word in ("tap", "knock", "shock", "vibration")):
            pencil.tint (rounded (cx - 9, 8, 18, 34, 8), METAL)
            pencil.rect (cx - 9, 8, 18, 34, width=1.1, radius=8, layer="top")
            for index in range (6):
                pencil.line ((cx - 6, 13 + index * 4.5), (cx + 6, 15 + index * 4.5), width=0.7,
                             layer="top", passes=1)
        elif any (word in emblem for word in ("obstacle", "avoid", "beam", "line", "track")):
            for dx, tint in ((-9, "#f2f2f2"), (9, "#3a3a3a")):
                pencil.dome (cx + dx, 22, 8, tint)
        elif any (word in emblem for word in ("water", "rain", "level")):
            for index in range (5):
                pencil.line ((cx - 18 + index * 9, 8), (cx - 18 + index * 9, 40), width=1.0,
                             tone=0.5, layer="top", passes=1)
        elif any (word in emblem for word in ("flame", "fire")):
            pencil.dome (cx, 22, 8, "#3a3a3a")
        elif any (word in emblem for word in ("light", "photo")):
            pencil.spot (cx, 24, 10, "#f3e9d2")
            pencil.circle (cx, 24, 10, width=1.1, layer="top")
            pencil.polyline ([(cx - 6, 18), (cx + 6, 18), (cx + 6, 22), (cx - 6, 22), (cx - 6, 26),
                              (cx + 6, 26), (cx + 6, 30)], width=1.0, tone=0.6, layer="top", passes=1)
        else:
            ic (pencil, cx, 24, 20, 14, notch=False)
        if len (self.names) >= 4:
            pencil.tint ([(cx - 20, 48), (cx - 4, 48), (cx - 4, 64), (cx - 20, 64)], "#6f9ad6")
            screw (pencil, cx - 12, 56, 4.5)
            ic (pencil, cx + 12, 56, 13, 12)
        else:
            led_dot (pencil, cx + 14, 56)
            smd (pencil, cx - 10, 56)
        if self.label:
            pencil.text (cx, 72 if len (self.names) >= 4 else 76, self.label, size=5.5, kind="silk")
        self.draw_header (pencil)


class Dht11 (Kind):
    # A DHT11 in its blue grilled case on a three-pin board.
    title  = "DHT11"
    pins   = ("S", "+", "−")
    width, height = 64, 112

    def draw (self, pencil):
        board (pencil, 0, 0, self.width, self.height, self.color, holes=[(56, 104)])
        pencil.tint (rounded (8, 4, 48, 62, 2), "#86aee6")
        pencil.rect (8, 4, 48, 62, width=1.0, radius=2, layer="top")
        for row in range (6):
            for column in range (4):
                x, y = 13 + column * 10.5, 9 + row * 9.5
                pencil.tint ([(x, y), (x + 6, y), (x + 6, y + 5), (x, y + 5)], "#2d4d7a")
        pencil.text (32, 78, "DHT11", size=6, kind="silk", weight="bold")
        self.draw_header (pencil)


class Motor (Kind):
    # A small DC motor with a fan blade on its shaft, and two leads.
    title  = "motor"
    pins   = ("+", "−")
    notes  = {"+": "red lead", "−": "black lead"}
    width, height = 200, 150
    COLORS = {"+": "red", "−": "black"}

    def header (self):
        return [Pin (name, 190, 66 + 18 * index, (1, 0), "lead", self.notes[name],
                     self.COLORS[name]) for index, name in enumerate (self.pins)]

    # What its drawing fills: the blades, can and leads, not the whole frame.
    def body (self):
        return 0, 32, self.width, 112

    def draw (self, pencil):
        for index in range (3):
            angle = math.radians (90 + 120 * index)
            blade = []
            for step in range (13):
                t = step / 12
                along = 8 + 60 * t
                wide = 17 * math.sin (math.pi * min (1.0, t * 1.1)) + 2
                blade.append ((along, wide))
            outline = blade + [(along, -wide * 0.55) for along, wide in reversed (blade)]
            points = [(45 + 0.42 * (a * math.cos (angle) - b * math.sin (angle)),
                       75 + (a * math.sin (angle) + b * math.cos (angle))) for a, b in outline]
            pencil.tint (points, "#dfe6ee")
            pencil.polyline (points, width=1.1, closed=True, layer="top")
        pencil.spot (45, 75, 9, "#c9ccd0")
        pencil.circle (45, 75, 9, width=1.1, layer="top")
        pencil.line ((52, 75), (70, 75), width=2.6, tone=0.5, layer="top", passes=1)
        pencil.tint (rounded (70, 64, 7, 22, 2), METAL)
        pencil.tint (rounded (76, 42, 96, 66, 6), "#cdd0d3")
        pencil.rect (76, 42, 96, 66, width=1.0, radius=6, layer="top")
        pencil.hatch (78, 86, 92, 20, gap=3.2, angle=15, tone=0.22, layer="top")
        pencil.tint (rounded (170, 48, 18, 54, 3), "#ece4d2")
        pencil.rect (170, 48, 18, 54, width=1.1, radius=3, layer="top", passes=1)
        for pin in self.header ():
            pencil.tint ([(186, pin.y - 2.5), (192, pin.y - 2.5), (192, pin.y + 2.5),
                          (186, pin.y + 2.5)], METAL)


class Battery9V (Kind):
    # A 9 V battery on a snap with red and black leads.
    title  = "9 V battery"
    pins   = ("+", "−")
    notes  = {"+": "red lead", "−": "black lead"}
    width, height = 104, 212
    COLORS = {"+": "red", "−": "black"}

    def header (self):
        return [Pin (name, 40 + 24 * index, self.height, (0, 1), "lead", self.notes[name],
                     self.COLORS[name]) for index, name in enumerate (self.pins)]

    def draw (self, pencil):
        pencil.tint (rounded (0, 0, 104, 186, 6), "#4a4a4a")
        pencil.tint ([(0, 40), (104, 40), (104, 120), (0, 120)], "#e9d59a")
        pencil.rect (0, 0, 104, 186, width=1.0, radius=6, layer="top")
        pencil.text (52, 92, "9V", size=30, kind="silk", weight="bold", tone=0.85)
        pencil.tint (rounded (20, 178, 64, 26, 4), "#262626")
        pencil.rect (20, 178, 64, 26, width=1.0, radius=4, layer="top", passes=1)
        pencil.text (40, 172, "+", size=10, kind="silk", color="#f4f1e8")
        pencil.text (64, 172, "−", size=10, kind="silk", color="#f4f1e8")


class Lcd1602 (Kind):
    # The LCD1602 module: 80 x 36 mm, its sixteen pins along the top edge
    # from pin 1 (VSS) 8 mm in from the left, as the screen reads. Plugged
    # into a row with the screen lying off the board's far edge it is turned
    # half round, so it is drawn that way: header along the bottom, pin 16
    # (K) on the left, the screen upside down.
    title  = "LCD"
    pins   = ("K", "A", "D7", "D6", "D5", "D4", "D3", "D2", "D1", "D0", "E", "RW", "RS", "V0",
              "VDD", "VSS")
    notes  = {name: f"pin {16 - index}" for index, name in enumerate (pins)}
    pin_modes = {name: "output" for name in ("RS", "E", "D4", "D5", "D6", "D7")}
    width, height = 315, 142
    inset  = 8
    color  = "#a3cc8e"

    def header_x (self):
        return 209

    def header_y (self):
        return self.height - self.inset

    def draw (self, pencil):
        board (pencil, 0, 0, self.width, self.height, self.color,
               holes=[(10, 10), (305, 10), (10, 132), (305, 132)], radius=3)
        pencil.tint ([(17.5, 23.5), (297.5, 23.5), (297.5, 118.5), (17.5, 118.5)], "#2b2b2b")
        pencil.rect (17.5, 23.5, 280, 95, width=1.0, layer="top")
        pencil.tint ([(30.5, 39.5), (284.5, 39.5), (284.5, 102.5), (30.5, 102.5)], "#5f8fdc")
        pencil.rect (30.5, 39.5, 254, 63, width=0.9, layer="top", passes=1)
        lines = list (self.options.get ("text") or ("", ""))[:2] + ["", ""]
        for row in range (2):
            for column in range (16):
                x = 315 - (46.7 + 14 * column) - 11.6
                y = 142 - (48.35 + 23.4 * row) - 21.9
                pencil.tint ([(x, y), (x + 11.6, y), (x + 11.6, y + 21.9), (x, y + 21.9)], "#9fbdf0",
                             opacity=0.45)
                text = lines[row]
                if column < len (text) and text[column] != " ":
                    pencil.text (x + 5.8, y + 5, text[column], size=17, rotate=180, kind="mono",
                                 color="#f5f7ff", tone=1.0)
        for pin in self.header ():
            pencil.spot (pin.x, pin.y, 3.6, "#d9c98f")
            pencil.circle (pin.x, pin.y, 1.6, width=0.8, layer="top", passes=1)
            pencil.text (pin.x + 1.8, pin.y - 6, pin.name, size=5.2, rotate=-90, anchor="start",
                         kind="silk")


# The radios ---------------------------------------------------------------

class FmRadio (Kind):
    # CJMCU-470: an Si4703 FM receiver on a purple 22 x 29 mm board, with a
    # TPA6111A2 amplifier and a 3.5 mm headphone jack at the far end, whose
    # socket stands 3 mm out of the side. Seen from above with the header
    # at the bottom, pin 1 (GPIO2, the square pad) is on the left and 3.3V
    # on the right, as the back of the board prints them.
    title  = "FM radio"
    pins   = ("GPIO2", "GPIO1", "RST", "SEN", "SCLK", "SDIO", "GND", "3.3V")
    width, height = 99, 114
    color  = PCB_PURPLE
    LEFT   = 12                     # the jack's socket, out past the board's left edge

    def header_x (self):
        return self.LEFT + 43.5

    def draw (self, pencil):
        x0 = self.LEFT
        pencil.tint (rounded (0, 16, 14, 24, 2), "#262626")
        pencil.rect (0, 16, 14, 24, width=0.9, radius=2, layer="top", passes=1)
        board (pencil, x0, 0, 87, 114, self.color, radius=3)
        # The jack, its spring contact showing on top.
        pencil.tint (rounded (x0 + 1, 3, 46, 50, 2), PLASTIC)
        pencil.rect (x0 + 1, 3, 46, 50, width=1.0, radius=2, layer="top")
        pencil.tint (rounded (x0 + 5, 21, 18, 12, 2), METAL)
        pencil.rect (x0 + 5, 21, 18, 12, width=0.7, radius=2, layer="top", passes=1)
        for cy in (15, 41):
            smd (pencil, x0 + 53, cy, 13, 10, "#3d3d3d")
            level (pencil, x0 + 53, cy, "47", 4.5, color=WHITE)
        ic (pencil, x0 + 72, 31, 18, 13, legs=4)
        level (pencil, x0 + 67, 57, "Si470x", 6.5, weight="bold")
        ic (pencil, x0 + 27, 73, 12, 12)
        pencil.tint (rounded (x0 + 56, 67, 24, 8, 4), METAL)
        pencil.rect (x0 + 56, 67, 24, 8, width=0.8, radius=4, layer="top", passes=1)
        for sx, sy in ((x0 + 10, 64), (x0 + 10, 72), (x0 + 44, 71)):
            smd (pencil, sx, sy, 7, 3.5)
        self.draw_header (pencil, size=5.4)


class RfReceiver (Kind):
    # RX470C-V01: a 433 MHz superheterodyne receiver on a green 30 x 9.5 mm
    # board, a right-angle header near one end of a long edge. Seen from
    # above with the header at the bottom, pin 1 (VCC, the square pad) is
    # on the left. The two DATA pins are joined on the board; the second is
    # DATA2, as the bench names the second of a pair. The ANT and GND pads
    # are at the far end, a spring antenna standing up from ANT.
    title  = "radio receiver"
    pins   = ("VCC", "DATA", "DATA2", "GND")
    pin_modes = {"DATA": "input", "DATA2": "input"}
    width, height = 118, 106
    color  = PCB_GREEN
    TOP    = 68.5                   # the board's top edge, below the spring

    def header_x (self):
        return 98

    def draw (self, pencil):
        top = self.TOP
        board (pencil, 0, top, 118, self.height - top, self.color, radius=2)
        for sy in (5, 12, 19):
            smd (pencil, 12, top + sy, 8, 4)
        ic (pencil, 44, top + 18.5, 38, 13, legs=8)
        pencil.tint (rounded (70, top + 0.8, 45, 12.6, 6), METAL)
        pencil.rect (70, top + 0.8, 45, 12.6, width=0.9, radius=6, layer="top", passes=1)
        level (pencil, 92.5, top + 7.1, "6.7458", 5, tone=0.6)
        hole (pencil, 5, top + 31)
        hole (pencil, 14, top + 31)
        pencil.line ((5, top + 31), (5, top - 1), width=1.3, tone=0.6, layer="top", passes=1)
        pencil.line ((5, top - 1), (9, top - 3), width=1.3, tone=0.6, layer="top", passes=1)
        coil (pencil, 9, top - 3, 1, 7, 12)
        self.draw_header (pencil, size=4.5)


class RfTransmitter (Kind):
    # WL102-341: a 433 MHz transmitter on a green 17.7 x 13 mm board, its
    # right-angle header on one long edge and a 13.56 MHz crystal across
    # the other. Seen from above with the header at the bottom, EN (the
    # square pad) is on the left and − on the right: the back prints them
    # − + DAT EN. OUT, in the top left corner, takes the spring antenna.
    title  = "radio transmitter"
    pins   = ("EN", "DAT", "+", "−")
    pin_modes = {"DAT": "output"}
    supply = "3.3V"
    width, height = 70, 121
    color  = PCB_GREEN
    TOP    = 70                     # the board's top edge, below the spring

    def header_x (self):
        return 36

    def draw (self, pencil):
        top = self.TOP
        board (pencil, 0, top, 70, self.height - top, self.color, radius=2)
        pencil.tint (rounded (19, top + 2, 44, 15, 7), METAL)
        pencil.rect (19, top + 2, 44, 15, width=0.9, radius=7, layer="top", passes=1)
        level (pencil, 41, top + 9.5, "13.560", 6, tone=0.65)
        ic (pencil, 43, top + 25, 16, 9, legs=4)
        for sx, sy in ((9, top + 20), (9, top + 28), (62, top + 26)):
            smd (pencil, sx, sy, 6, 3)
        hole (pencil, 7, top + 6)
        pencil.line ((7, top + 6), (7, top - 2), width=1.3, tone=0.6, layer="top", passes=1)
        pencil.line ((7, top - 2), (10, top - 4), width=1.3, tone=0.6, layer="top", passes=1)
        coil (pencil, 10, top - 4, 1, 7, 12)
        self.draw_header (pencil, size=5.4)


class LoraModem (Kind):
    # REYAX RYLR896: a LoRa modem on a blue 17 x 25 mm board, its radio
    # under a can labelled RYLR890, a gold spring antenna standing on its
    # top corner. Lying flat with the header at the bottom, pin 1 (VDD) is
    # on the left, as the datasheet numbers them; the back of the board,
    # seen from behind, prints them the other way: GND ... VDD.
    title  = "LoRa modem"
    pins   = ("VDD", "NRST", "RXD", "TXD", "NC", "GND")
    pin_modes = {"RXD": "output", "TXD": "input"}
    supply = "3.3V"
    width, height = 67, 168
    color  = PCB_BLUE
    TOP    = 70                     # the board's top edge, below the spring

    def draw (self, pencil):
        top = self.TOP
        board (pencil, 0, top, 67, self.height - top, "#9fb7d9", radius=2)
        pencil.line ((55, top + 6), (55, top - 3), width=1.2, tone=0.6, layer="top", passes=1)
        hole (pencil, 55, top + 6, 2.6)
        coil (pencil, 55, top - 3, 1, 10, 13, GOLD)
        pencil.tint (rounded (3, top + 2, 48, 38, 2), METAL)
        pencil.rect (3, top + 2, 48, 38, width=1.0, radius=2, layer="top")
        level (pencil, 27, top + 15, "REYAX", 7, weight="bold")
        level (pencil, 27, top + 26, "RYLR890", 6)
        for index in range (7):
            x, y = 23 + index * 3.6, top + 47 + index * 3.6
            for side in (-1, 1):
                pencil.line ((x, top + 58 + side * 11), (x, top + 58 + side * 14), width=0.8, tone=0.5,
                             layer="top", passes=1)
                pencil.line ((34 + side * 11, y), (34 + side * 14, y), width=0.8, tone=0.5, layer="top",
                             passes=1)
        ic (pencil, 34, top + 58, 22, 22)
        for sx, sy in ((9, top + 50), (9, top + 58), (59, top + 50)):
            smd (pencil, sx, sy, 6, 3)
        self.draw_header (pencil, size=5.2)


class LoraModule (Kind):
    # Ebyte E32-433T20D: a LoRa module on a green 21 x 36 mm board under a
    # shield, an SMA socket standing 12.5 mm out of the far end. Its rubber
    # antenna is 110 mm long; it is drawn upright and cut to a quarter of
    # that, so the module fits beside the bench. Seen from above with the
    # header at the bottom, pin 1 (M0) is on the left, 3.5 mm in from the
    # edge. The board prints no names on its pins.
    title  = "LoRa module"
    pins   = ("M0", "M1", "RXD", "TXD", "AUX", "VCC", "GND")
    pin_modes = {"RXD": "output", "TXD": "input", "AUX": "input"}
    width, height = 83, 301
    color  = PCB_GREEN
    TOP    = 159                    # the board's top edge, below the antenna

    def header_x (self):
        return 43.8

    def draw (self, pencil):
        top = self.TOP
        whip (pencil, 61.4, top - 90, 0, 22)
        pencil.tint (rounded (47.4, top - 92, 28, 20, 5), "#2e2e2e")
        pencil.rect (47.4, top - 92, 28, 20, width=0.9, radius=5, layer="top", passes=1)
        knurled (pencil, 61.4, top - 49, top - 73, 32)
        sma (pencil, 61.4, top, 49)
        board (pencil, 0, top, 83, self.height - top, self.color, radius=2)
        for sx in (51, 72):
            pencil.tint ([(sx - 3, top + 1), (sx + 3, top + 1), (sx + 3, top + 12), (sx - 3, top + 12)],
                         GOLD)
        for index, hx in enumerate ((40.5, 28.7, 16.5)):
            hole (pencil, hx, top + 12, square=index == 2)
        pencil.tint ([(3, top + 24), (80, top + 24), (80, top + 120), (3, top + 120)], METAL)
        pencil.rect (3, top + 24, 77, 96, width=1.0, layer="top")
        pencil.text (35, top + 72, "E32-433T20D", size=9, rotate=-90, kind="silk", weight="bold")
        pencil.text (52, top + 72, "EBYTE", size=6.5, rotate=-90, kind="silk", weight="bold")
        self.draw_header (pencil, size=5.4)


class MeshBoard (Kind):
    # Heltec WiFi LoRa 32 V3: an ESP32-S3 and an SX1262 radio on a white
    # 50.2 x 25.5 mm board under a 0.96 inch OLED, with 18 pins along each
    # long edge. Its LoRa antenna hangs off a U.FL socket at the far end on
    # a short lead to an SMA socket; the antenna turns up at its knuckle.
    # The bottom header is J2 and the top one J3, each counted from the
    # USB-C end on the left. Where a name comes twice, the second takes a 2
    # (Ve2, GND2, as the bench names the Mega's second GND.power2), and the
    # second 3V3 is 3V3b, since 3V32 would read as a number.
    title  = "Meshtastic board"
    pins   = ("GND", "5V", "Ve", "Ve2", "44", "43", "RST", "0", "36", "35", "34", "33", "47", "48",
              "26", "21", "20", "19")
    J3     = ("GND2", "3V3", "3V3b", "37", "46", "45", "42", "41", "40", "39", "38", "1", "2", "3",
              "4", "5", "6", "7")
    width, height = 303, 160
    inset  = 5.5                    # the pins stand in holes 1.4 mm in from each edge
    color  = WHITE
    TOP    = 60                     # the board's top edge, below the antenna
    LEFT   = 3                      # the USB socket, out past the board's left edge

    def header_x (self):
        return self.LEFT + 10 + 8.5 * PITCH

    def header_y (self):
        return self.height - self.inset

    def extra (self):
        return [Pin (name, self.LEFT + 10 + index * PITCH, self.TOP + self.inset, (0, -1), "male")
                for index, name in enumerate (self.J3)]

    def draw (self, pencil):
        x0, top, bottom = self.LEFT, self.TOP, self.height
        outline = [(x0, top), (x0 + 187, top), (x0 + 197.6, top + 18), (x0 + 197.6, bottom - 18),
                   (x0 + 187, bottom), (x0, bottom)]
        pencil.tint (outline, self.color)
        pencil.polyline (outline, width=1.0, closed=True, layer="top")
        # The LoRa antenna: a lead from the U.FL socket to an SMA socket, the
        # antenna's nut, then its knuckle turning it up.
        y = top + 50
        pencil.wire ([(x0 + 189, y), (x0 + 206, y + 5), (x0 + 224, y)], "#555555", width=2.4,
                     layer="top", smooth=True)
        pencil.begin (x0 + 224, y, 90)
        sma (pencil, 0, 0, 34)
        knurled (pencil, 0, -34, -54, 30)
        pencil.end ()
        whip (pencil, x0 + 288, y - 12, 0, 22)
        pencil.tint (rounded (x0 + 277, y - 16, 22, 30, 6), "#2e2e2e")
        pencil.rect (x0 + 277, y - 16, 22, 30, width=0.9, radius=6, layer="top", passes=1)
        pencil.tint (rounded (x0 + 183, y - 5.5, 11, 11, 1), METAL)
        pencil.rect (x0 + 183, y - 5.5, 11, 11, width=0.7, radius=1, layer="top", passes=1)
        pencil.circle (x0 + 188.5, y, 2.6, width=0.6, layer="top", passes=1)
        pencil.tint (rounded (x0 + 184, top + 20, 9, 12, 0), PLASTIC)
        pencil.text (x0 + 188.5, top + 26, "V3", size=5, rotate=-90, kind="silk", color=WHITE)
        # The USB-C socket, the PRG and RST buttons, the 2.4 GHz spring and
        # the charge LED.
        pencil.tint (rounded (x0 - 3, top + 35, 32, 30, 4), METAL)
        pencil.rect (x0 - 3, top + 35, 32, 30, width=1.0, radius=4, layer="top")
        pencil.tint (rounded (x0 - 1, top + 44, 6, 12, 2), "#6a6a66")
        for name, by in (("PRG", top + 21), ("RST", top + 79)):
            pencil.tint (rounded (x0 + 12, by - 7, 14, 14, 1.5), METAL)
            pencil.rect (x0 + 12, by - 7, 14, 14, width=0.8, radius=1.5, layer="top", passes=1)
            pencil.spot (x0 + 19, by, 3.8, PLASTIC)
            level (pencil, x0 + 34, by, name, 4.6)
        pencil.begin (x0 + 40, top + 50, 90)
        coil (pencil, 0, 7, -7, 4, 5, COPPER)
        pencil.end ()
        led_dot (pencil, x0 + 44, top + 64, r=2.4)
        pencil.text (x0 + 53, top + 50, "WiFi LoRa 32 V3", size=5, rotate=-90, kind="silk")
        # The OLED on its clear frame.
        pencil.tint (rounded (x0 + 58, top + 15, 129, 69, 0), "#dfe7ea")
        pencil.rect (x0 + 58, top + 15, 129, 69, width=0.9, layer="top", passes=1)
        pencil.tint (rounded (x0 + 67, top + 15, 107, 69, 0), "#262a2e")
        pencil.rect (x0 + 67, top + 15, 107, 69, width=1.0, layer="top")
        pencil.rect (x0 + 76, top + 24, 89, 45, width=0.5, tone=0.3, layer="top", passes=1)
        # The screen reads the right way up however the board is turned:
        # turned half round, its lines turn half round about its middle.
        flip = 90 < sum (pencil.turns) % 360 <= 270
        for row, line in enumerate (list (self.options.get ("text") or ())[:4]):
            x, y = x0 + 78, top + 33 + row * 11
            if flip:
                x, y = 2 * (x0 + 120.5) - x, 2 * (top + 46.5) - y
            pencil.text (x, y, line, size=8, anchor="start", kind="mono", color="#dce8ff", tone=1.0,
                         rotate=180 if flip else 0)
        # Both headers' holes, and their names: J2's under the OLED, J3's above.
        for pin in self.header () + self.extra ():
            pencil.spot (pin.x, pin.y, 3.4, "#d9c98f")
            pencil.circle (pin.x, pin.y, 1.6, width=0.8, layer="top", passes=1)
        for pin in self.header ():
            level (pencil, pin.x, top + 87.5, pin.name, 4.8)
        for pin in self.extra ():
            level (pencil, pin.x, top + 12, pin.name, 3.9)


KINDS = {
    "lcd": Lcd1602, "servo": Servo, "ultrasonic": Ultrasonic, "matrix": Matrix, "joystick": Joystick,
    "keypad": Keypad, "ir_receiver": IrReceiver, "rfid": Rfid, "gy521": Gy521, "rtc": Rtc,
    "relay": Relay, "stepper": Stepper, "encoder": Encoder, "pir": Pir, "sensor": Sensor,
    "dht11": Dht11, "motor": Motor, "battery9v": Battery9V, "fm_radio": FmRadio,
    "rf_receiver": RfReceiver, "rf_transmitter": RfTransmitter, "lora_modem": LoraModem,
    "lora_module": LoraModule, "mesh_board": MeshBoard, "module": Kind,
}

# Other names a wire may use for a pin, when the module has no pin by that
# name: GND for −, S for OUT, VCC for + or VDD. A voltage names a supply pin
# only of a module that runs on it: 5V finds the servo's +, but no pin of
# the LoRa modem, whose VDD takes 3.3 V.
GROUND  = {"−", "-", "GND", "G"}
SIGNAL  = {"S", "SIG", "SIGNAL", "OUT"}
SUPPLY  = {"+", "VCC", "VDD"}
VOLTAGE = {"5V": {"5V", "+5V"}, "3.3V": {"3.3V", "3V3", "+3.3V"}}


def make (kind, pins=None, label=None, **options):
    if kind not in KINDS:
        raise ValueError (f"no module kind {kind!r}; there are {', '.join (sorted (KINDS))}")
    return KINDS[kind] (pins, label, **options)


class Placed:
    """A module in the scene: its frame's origin moved to (x, y) and turned
    by angle (a quarter turn at a time). reach is how far its male pins
    stick out: to a jumper's housing off the board, or into the holes when
    it stands in the breadboard."""

    COS = {0: 1, 90: 0, 180: -1, 270: 0}
    SIN = {0: 0, 90: 1, 180: 0, 270: -1}

    def __init__ (self, kind, name, x, y, angle, reach=STUB):
        self.kind, self.name = kind, name
        self.title = kind.title
        self.x, self.y, self.angle = x, y, angle % 360
        self.reach = reach
        self.wired = set ()

    def scene (self, x, y):
        c, s = self.COS[self.angle], self.SIN[self.angle]
        return self.x + x * c - y * s, self.y + x * s + y * c

    def turn (self, dx, dy):
        c, s = self.COS[self.angle], self.SIN[self.angle]
        return dx * c - dy * s, dx * s + dy * c

    def pins (self):
        return self.kind.all_pins ()

    def pin (self, name):
        pins = self.pins ()
        for match in (lambda p: p.name == name,
                      lambda p: p.name.casefold () == name.replace ("-", "−").casefold ()):
            found = [p for p in pins if match (p)]
            if found:
                return found[0]
        asked = name.upper ()
        volts = next ((volts for volts, names in VOLTAGE.items () if asked in names), None)
        if volts:
            group = VOLTAGE[volts] | (SUPPLY if self.kind.supply == volts else set ())
        else:
            group = next ((group for group in (GROUND, SIGNAL, SUPPLY) if asked in group), set ())
            if group is SUPPLY:
                group = SUPPLY | VOLTAGE["5V"] | VOLTAGE["3.3V"]
        found = [p for p in pins if p.name.upper () in group]
        if len (found) == 1:
            return found[0]
        supply = [p for p in pins if p.name.upper () in SUPPLY]
        if volts and supply and self.kind.supply != volts:
            raise ValueError (f"the {self.title}'s {supply[0].name} takes {self.kind.supply}, not "
                              f"{volts}")
        raise ValueError (f"the {self.title} has no pin {name!r}; its pins are "
                          f"{', '.join (p.name for p in pins)}")

    # Where a wire meets a pin, and which way it leaves.
    def anchor (self, pin):
        root = self.scene (pin.x, pin.y)
        direction = self.turn (*pin.direction)
        return root, direction

    def box (self):
        x0, y0, w, h = self.kind.body ()
        points = [self.scene (x, y)
                  for x, y in ((x0, y0), (x0 + w, y0), (x0, y0 + h), (x0 + w, y0 + h))]
        return (min (p[0] for p in points), min (p[1] for p in points),
                max (p[0] for p in points), max (p[1] for p in points))

    # Everything drawn, pins and a jumper's housing included.
    def reach_box (self):
        x0, y0, x1, y1 = self.box ()
        for pin in self.pins ():
            (rx, ry), (dx, dy) = self.anchor (pin)
            out = HOUSING if pin.style == "male" and self.reach == STUB else 10
            x0, x1 = min (x0, rx + dx * out), max (x1, rx + dx * out)
            y0, y1 = min (y0, ry + dy * out), max (y1, ry + dy * out)
        return x0, y0, x1, y1

    def draw (self, pencil, title=True, size=10):
        pencil.begin (self.x, self.y, self.angle)
        for pin in self.pins ():
            if pin.style == "male" and self.reach:
                dx, dy = pin.direction
                pencil.line ((pin.x, pin.y), (pin.x + dx * self.reach, pin.y + dy * self.reach),
                             width=2.4, tone=0.55, layer="top", passes=1, wobble=0.1)
            elif pin.style == "lead" and pin.name not in self.wired:
                dx, dy = pin.direction
                pencil.wire ([(pin.x, pin.y), (pin.x + dx * 14, pin.y + dy * 14)], WIRES[pin.color],
                             width=2.4)
        self.kind.draw (pencil)
        pencil.end ()
        if title:
            x, y, anchor = self.title_spot ()
            pencil.text (x, y, self.title, size=size, anchor=anchor)

    # Where its name goes: on the side away from the main header, unless
    # the kind has wires or pins there.
    def title_spot (self):
        x0, y0, x1, y1 = self.box ()
        dx, dy = self.turn (*SIDES[self.kind.title_side])
        gap = self.kind.title_gap
        if dy < 0:
            return (x0 + x1) / 2, y0 - gap, "middle"
        if dy > 0:
            return (x0 + x1) / 2, y1 + gap + 9, "middle"
        if dx > 0:
            return x1 + gap, (y0 + y1) / 2 + 3.5, "start"
        return x0 - gap, (y0 + y1) / 2 + 3.5, "end"

    # The box its name fills.
    def title_box (self):
        x, y, anchor = self.title_spot ()
        return text_box (x, y, self.title, 10, anchor)
