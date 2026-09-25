"""Modules: the servo, the ultrasonic sensor, the LED matrix and the other
boards that sit beside the breadboard on jumper wires, or stand in one of
its rows on their own header.

Each kind is drawn in its own frame, in drawing units (DPI to the inch),
at the real board's size, with its main header along the bottom edge. A
Placed module moves and turns that frame into the scene, and knows where
each of its pins ends up.
"""

import math

from pencil import DPI, rounded_rectangle

SIDES = {"top": (0, -1), "bottom": (0, 1), "left": (-1, 0), "right": (1, 0)}
PITCH = 0.1 * DPI                   # header pins are 0.1 inch apart
STUB = 24                           # how far a male header pin sticks out
HOUSING = 57                        # a jumper's female housing, 14.5 mm

PCB_BLUE  = "#c5d4e6"
PCB_GREEN = "#c8dbbd"
METAL     = "#d3d3cf"
PLASTIC   = "#3a3a3a"
WHITE     = "#f4f2ec"
LED_RED   = "#f0604f"
LEADS     = {"red": "#be4c44", "black": "#3a3a3a"}


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
            pencil.text (pin.x, 85, pin.name, size=4.6, kind="silk")


class Sensor (Kind):
    # One of the 37-in-1 kit's small boards: a sensing part, and for the
    # four-pin ones a comparator with its sensitivity trimmer.
    title  = "sensor"
    pins   = ("S", "+", "−")
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
            pencil.text (cx, 76, self.label, size=5.5, kind="silk")
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


KINDS = {
    "lcd": Lcd1602, "servo": Servo, "ultrasonic": Ultrasonic, "matrix": Matrix, "joystick": Joystick,
    "keypad": Keypad, "ir_receiver": IrReceiver, "rfid": Rfid, "gy521": Gy521, "rtc": Rtc,
    "relay": Relay, "stepper": Stepper, "encoder": Encoder, "pir": Pir, "sensor": Sensor,
    "dht11": Dht11, "motor": Motor, "battery9v": Battery9V, "module": Kind,
}

# Other names a wire may use for a pin, when the module has no pin by that
# name: GND for −, VCC for +, and so on.
SAME = [{"−", "-", "GND", "G"}, {"+", "VCC", "5V", "+5V", "VDD"}, {"S", "SIG", "SIGNAL", "OUT"}]


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
        for group in SAME:
            if name.upper () in {g.upper () for g in group}:
                found = [p for p in pins if p.name.upper () in {g.upper () for g in group}]
                if len (found) == 1:
                    return found[0]
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

    def draw (self, pencil, title=True):
        pencil.begin (self.x, self.y, self.angle)
        for pin in self.pins ():
            if pin.style == "male" and self.reach:
                dx, dy = pin.direction
                pencil.line ((pin.x, pin.y), (pin.x + dx * self.reach, pin.y + dy * self.reach),
                             width=2.4, tone=0.55, layer="top", passes=1, wobble=0.1)
            elif pin.style == "lead" and pin.name not in self.wired:
                dx, dy = pin.direction
                pencil.wire ([(pin.x, pin.y), (pin.x + dx * 14, pin.y + dy * 14)], LEADS[pin.color],
                             width=2.4)
        self.kind.draw (pencil)
        pencil.end ()
        if title:
            x, y, anchor = self.title_spot ()
            pencil.text (x, y, self.title, size=10, anchor=anchor)

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
        width = len (self.title) * 5.4
        left = {"start": x, "end": x - width}.get (anchor, x - width / 2)
        return left, y - 9, left + width, y + 3
