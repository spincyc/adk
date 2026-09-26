"""Parts that stand in the breadboard: each knows its legs and the holes
they are in, what it joins inside, which holes its body covers, how to draw
itself over those holes, the shapes wires and labels keep clear of, and
where its label may go.

Tall parts are drawn as if seen a little from the front: an LED's dome
sits above its legs, so the holes stay in view.
"""

import math
import re

from modules import Placed, rounded
from pencil import DPI
from route import text_width

RAINBOW = ["#eab0aa", "#efdca6", "#b5d6ad", "#adc4e6"]
METAL   = "#d3d3cf"
LEAD    = 0.9                       # half a lead's width, as wires keep clear of it


class Label:
    """A label a part or wire wants: its text, and the spots it may go,
    best first, each (x, y, anchor, cost). at is what a leader points to
    when the label has to go further out; leg is the hole of the leg it
    names, if it names one, which it may touch."""

    def __init__ (self, text, spots, at=None, size=1.0, leg=None):
        self.text, self.spots, self.at, self.size, self.leg = text, spots, at, size, leg


# Spots round a box for a label of that text: above, then beside, then
# below, each a little way out.
def spots_round (box, text, size, first="above", gap=2.5):
    x0, y0, x1, y1 = box
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    width = text_width (text, size)
    above = [(cx, y0 - gap - size * 0.26, "middle", 0)]
    below = [(cx, y1 + gap + size * 0.8, "middle", 6)]
    right = [(x1 + gap + 1, cy + size * 0.3, "start", 2)]
    left = [(x0 - gap - 1, cy + size * 0.3, "end", 3)]
    sides = {"above": above + right + left + below, "right": right + left + above + below,
             "left": left + right + above + below, "below": below + right + left + above}[first]
    sides = [(x, y, anchor, index * 6) for index, (x, y, anchor, _) in enumerate (sides)]
    # The same again, slid along a little, for a crowded spot.
    slid = []
    for x, y, anchor, cost in sides:
        if anchor == "middle":
            slid += [(x - width / 2 - 2, y, anchor, cost + 5), (x + width / 2 + 2, y, anchor, cost + 5)]
        else:
            slid += [(x, y - size - 1, anchor, cost + 5), (x, y + size + 1, anchor, cost + 5)]
    return sides + slid


class Part:
    name = "part"
    sources = ()                    # legs that feed power, listed like the Mega's pins
    blocks = True                   # wires go round its body
    # How a sketch claims a Mega pin wired to its legs: "output" to drive
    # it, "input" to read it, or None when that isn't the part's to say.
    mode = None

    def legs (self):
        return []

    def modes (self):
        return [self.mode for _ in self.legs ()]

    def inside (self):
        return []

    # Where its body is, as ("rect", x0, y0, x1, y1) or ("circle", cx, cy, r)
    # in drawing units: holes under it can't be used.
    def footprint (self, bench):
        return []

    # Everything drawn, bodies and leads, which wires and labels keep clear
    # of, in the shapes of route.py.
    def shapes (self, bench):
        return [("rect", *s[1:]) if s[0] == "rect" else s for s in self.footprint (bench)]

    # A body off the board, which grows the drawing and wires go round.
    def box (self, bench):
        return None

    def labels (self, bench):
        return []

    def draw (self, pencil, bench):
        pass


def lead_shape (a, b):
    return ("segment", a[0], a[1], b[0], b[1], LEAD)


class Resistor (Part):
    # A quarter-watt resistor as the Elegoo kits ship them: a pale blue 1%
    # metal-film body, fatter at its end caps, lying just above its legs, with
    # five bands: three digits, a multiplier, and the brown tolerance band set
    # apart on the far cap.
    TINTS = {"black": "#343434", "brown": "#7d5238", "red": "#b9483f", "orange": "#cf7a3a",
             "yellow": "#d4b240", "green": "#4f8049", "blue": "#44689f", "violet": "#745a9b",
             "grey": "#999997", "white": "#f1eee6", "gold": "#b39448"}
    BANDS = (-0.68, -0.46, -0.24, -0.02, 0.64)
    LENGTH = 25

    def __init__ (self, value, a, b):
        self.value, self.a, self.b = value, a, b
        self.name = f"{value} resistor"

    def legs (self):
        return [("one end", self.a), ("other end", self.b)]

    # Standing across rows, or across the middle gap, the body lies along
    # its legs, halfway between them; along a row it lies just above them.
    def standing (self, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (self.a), bench.hole_xy (self.b)
        return abs (y2 - y1) > abs (x2 - x1) * 0.2

    def geometry (self, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (self.a), bench.hole_xy (self.b)
        if self.standing (bench):
            return (x1 + x2) / 2, (y1 + y2) / 2
        return (x1 + x2) / 2, (y1 + y2) / 2 - 7

    def shapes (self, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (self.a), bench.hole_xy (self.b)
        cx, cy = self.geometry (bench)
        half = self.LENGTH / 2
        if self.standing (bench):
            span = math.dist ((x1, y1), (x2, y2))
            ux, uy = (x2 - x1) / span, (y2 - y1) / span
            ends = (cx - ux * half, cy - uy * half), (cx + ux * half, cy + uy * half)
            return [("segment", *ends[0], *ends[1], 5.2), lead_shape ((x1, y1), ends[0]),
                    lead_shape (ends[1], (x2, y2))]
        return [("rect", cx - half, cy - 5.2, cx + half, cy + 5.2),
                lead_shape ((x1, y1), (cx - half + 1, cy)), lead_shape ((cx + half - 1, cy), (x2, y2))]

    def labels (self, bench):
        size = bench.label_size
        cx, cy = self.geometry (bench)
        if self.standing (bench):
            (_, y1), (_, y2) = bench.hole_xy (self.a), bench.hole_xy (self.b)
            box = (cx - 5.5, min (y1, y2) + 3, cx + 5.5, max (y1, y2) - 3)
            return [Label (self.value, spots_round (box, self.value, size, "right"), (cx, cy))]
        half = self.LENGTH / 2
        box = (cx - half, cy - 5.5, cx + half, cy + 5.5)
        return [Label (self.value, spots_round (box, self.value, size, "above"), (cx, cy))]

    def draw (self, pencil, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (self.a), bench.hole_xy (self.b)
        length = self.LENGTH
        cx, cy = self.geometry (bench)
        if self.standing (bench):
            span = math.dist ((x1, y1), (x2, y2))
            angle = math.degrees (math.atan2 (y2 - y1, x2 - x1))
            pencil.begin (cx, cy, angle)
            pencil.lead ((-span / 2, 0), (-length / 2 + 1, 0))
            pencil.lead ((length / 2 - 1, 0), (span / 2, 0))
            self._body (pencil, 0, 0, length)
            pencil.end ()
            return
        pencil.lead ((x1, y1), (cx - length / 2 + 1, cy))
        pencil.lead ((cx + length / 2 - 1, cy), (x2, y2))
        self._body (pencil, cx, cy, length)

    def _body (self, pencil, cx, cy, length):
        outline = resistor_outline (cx, cy, length)
        pencil.tint (outline, "#a9c3d8")
        for index, band in enumerate (bands_for (self.value)):
            u = self.BANDS[index]
            half = resistor_half (abs (u)) - 0.3
            bx = cx + u * length / 2
            pencil.band (bx - 1.0, cy - half, 2.0, 2 * half, self.TINTS[band])
        pencil.line ((cx - length * 0.3, cy - 2.4), (cx + length * 0.3, cy - 2.4), width=0.7,
                     tone=0.0, layer="top")
        pencil.layers["top"].append (
            f'<path d="M {cx - length * 0.36:.1f} {cy - 2.2:.1f} '
            f'L {cx + length * 0.36:.1f} {cy - 2.2:.1f}" '
            f'stroke="#ffffff" stroke-opacity="0.45" stroke-width="0.9" stroke-linecap="round"/>')
        pencil.polyline (outline, width=0.8, closed=True, layer="top")


class Led (Part):
    # A 5 mm LED seen from above, leaning back so its legs show: the lens
    # inside its flange, and the flange's flat on the short leg's side.
    mode = "output"
    TINTS = {"red": "#e39089", "yellow": "#ecd68c", "green": "#9ccb96", "blue": "#9ab7e2",
             "white": "#f4f2ea"}
    RADIUS = 11.4

    def __init__ (self, color, anode, cathode):
        self.color, self.anode, self.cathode = color, anode, cathode
        self.name = f"{color} LED"

    def legs (self):
        return [("long leg (+)", self.anode), ("short leg (−)", self.cathode)]

    def dome (self, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (self.anode), bench.hole_xy (self.cathode)
        return (x1 + x2) / 2, min (y1, y2) - 20

    # Each leg rises to the side of the dome nearer it, so they never cross.
    def feet (self, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (self.anode), bench.hole_xy (self.cathode)
        cx, cy = self.dome (bench)
        side = -1 if (x1, y2) <= (x2, y1) else 1
        return [((x1, y1), (cx + side * 3.5, cy + 8)), ((x2, y2), (cx - side * 3.5, cy + 8))]

    def shapes (self, bench):
        cx, cy = self.dome (bench)
        return [("circle", cx, cy, self.RADIUS)] + [lead_shape (a, b) for a, b in self.feet (bench)]

    def labels (self, bench):
        cx, cy = self.dome (bench)
        r = self.RADIUS
        return [Label (self.name, spots_round ((cx - r, cy - r, cx + r, cy + r), self.name,
                                               bench.label_size, "right"), (cx + r, cy))]

    def draw (self, pencil, bench):
        (x1, _), (x2, _) = bench.hole_xy (self.anode), bench.hole_xy (self.cathode)
        cx, cy = self.dome (bench)
        for a, b in self.feet (bench):
            pencil.lead (a, b)
        lens (pencil, cx, cy, self.TINTS[self.color], 1 if x2 > x1 else -1)


class Button (Part):
    # A 6 mm push button across the middle gap, its legs in e and f of two
    # columns two apart. The two legs in each column are joined inside;
    # pressing joins the columns.
    mode = "input"

    def __init__ (self, column):
        self.column = column
        self.name = f"button at column {column}"

    def holes (self):
        c = self.column
        return [f"f{c}", f"f{c + 2}", f"e{c}", f"e{c + 2}"]

    # The legs joined inside share a name, so each pair is one junction.
    def legs (self):
        c = self.column
        return [("left legs", f"e{c}"), ("left legs", f"f{c}"),
                ("right legs", f"e{c + 2}"), ("right legs", f"f{c + 2}")]

    def footprint (self, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (f"f{self.column}"), bench.hole_xy (f"e{self.column + 2}")
        return [("rect", x1 - 5, y1 - 5, x2 + 5, y2 + 5)]

    def draw (self, pencil, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (f"f{self.column}"), bench.hole_xy (f"e{self.column + 2}")
        cx, cy = (x1 + x2) / 2, (y1 + y2) / 2
        for hx, hy in ((x1, y1), (x2, y1), (x1, y2), (x2, y2)):
            inner = cy - 11 if hy < cy else cy + 11
            pencil.tint ([(hx - 1.6, min (hy, inner)), (hx + 1.6, min (hy, inner)),
                          (hx + 1.6, max (hy, inner)), (hx - 1.6, max (hy, inner))], METAL)
        pencil.tint (rounded (cx - 12, cy - 12, 24, 24, 1.5), "#dcd9d1")
        pencil.rect (cx - 12, cy - 12, 24, 24, width=0.9, radius=1.5, layer="top")
        for dx, dy in ((-9, -9), (9, -9), (-9, 9), (9, 9)):
            pencil.circle (cx + dx, cy + dy, 1.1, width=0.5, tone=0.5, layer="top")
        pencil.spot (cx, cy, 7, "#3c3c3c")
        pencil.circle (cx, cy, 7, width=0.8, layer="top")
        pencil.circle (cx, cy, 5.2, width=0.4, tone=0.4, layer="top")


class RgbLed (Part):
    # A 5 mm common-cathode RGB LED: red, common (the longest leg), green and
    # blue, side by side.
    mode = "output"
    RADIUS = 11.4
    LETTERS = ("R", "−", "G", "B")

    def __init__ (self, red, common, green, blue):
        self.holes = [red, common, green, blue]
        self.name = "RGB LED"

    def legs (self):
        return list (zip (("red leg", "common, longest leg (−)", "green leg", "blue leg"),
                          self.holes))

    def dome (self, bench):
        points = [bench.hole_xy (hole) for hole in self.holes]
        return sum (x for x, _ in points) / 4, min (y for _, y in points) - 24

    def feet (self, bench):
        points = [bench.hole_xy (hole) for hole in self.holes]
        cx, cy = self.dome (bench)
        order = sorted (range (4), key=lambda index: points[index])
        tops = {index: (cx + (rank - 1.5) * 3.2, cy + 8) for rank, index in enumerate (order)}
        return [(points[index], tops[index]) for index in range (4)]

    def shapes (self, bench):
        cx, cy = self.dome (bench)
        return [("circle", cx, cy, self.RADIUS)] + [lead_shape (a, b) for a, b in self.feet (bench)]

    def labels (self, bench):
        cx, cy = self.dome (bench)
        r = self.RADIUS
        size = bench.label_size
        labels = [Label (self.name, spots_round ((cx - r, cy - r, cx + r, cy + r), self.name, size,
                                                 "right"), (cx + r, cy))]
        # Each leg's letter just below its hole, or above it when a wire
        # comes in from below.
        for letter, hole in zip (self.LETTERS, self.holes):
            x, y = bench.hole_xy (hole)
            small = size * 0.62
            spots = [(x, y + 4 + small * 0.8, "middle", 0), (x - 4, y + small * 0.3, "end", 3),
                     (x + 4, y + small * 0.3, "start", 3)]
            labels.append (Label (letter, spots, None, 0.62, leg=(x, y)))
        return labels

    def draw (self, pencil, bench):
        cx, cy = self.dome (bench)
        for a, b in self.feet (bench):
            pencil.lead (a, b)
        lens (pencil, cx, cy, RAINBOW, 0)


# A 5 mm lens in its 5.8 mm flange, the flange cut flat on one side (0 for
# none).
def lens (pencil, cx, cy, tint, flat):
    r = 11.4
    ring = []
    for step in range (48):
        angle = step / 48 * 2 * math.pi
        x = cx + r * math.cos (angle)
        if flat:
            x = min (x, cx + 9.6) if flat > 0 else max (x, cx - 9.6)
        ring.append ((x, cy + r * math.sin (angle)))
    pencil.tint (ring, tint[0] if isinstance (tint, list) else tint, opacity=0.55)
    pencil.polyline (ring, width=0.8, closed=True, layer="top")
    pencil.dome (cx, cy, 9.6, tint)


def resistor_half (edge, waist=3.9, cap=4.9):
    if edge <= 0.5:
        return waist
    if edge <= 0.78:
        t = (edge - 0.5) / 0.28
        return waist + (cap - waist) * t * t * (3 - 2 * t)
    return cap * math.sqrt (max (0.0, 1 - ((edge - 0.78) / 0.22) ** 2))


def resistor_outline (cx, cy, length):
    top = []
    for step in range (41):
        u = -1 + step / 20
        top.append ((cx + u * length / 2, cy - resistor_half (abs (u))))
    return top + [(x, 2 * cy - y) for x, y in reversed (top)]


class Buzzer (Part):
    # A 12 mm buzzer, 9.5 mm tall, legs 7.6 mm (0.3 inch) apart under it. The
    # active one is sealed and carries a sticker; the passive one is open
    # underneath, where its green board shows.
    mode = "output"
    RADIUS = 6 / 25.4 * DPI
    RISE = 3

    def __init__ (self, positive, negative, kind):
        if kind not in ("active", "passive"):
            raise ValueError ("a buzzer is active or passive")
        self.positive, self.negative, self.kind = positive, negative, kind
        self.name = f"{kind} buzzer"

    def legs (self):
        return [("+ leg (long)", self.positive), ("− leg", self.negative)]

    def centre (self, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (self.positive), bench.hole_xy (self.negative)
        return (x1 + x2) / 2, (y1 + y2) / 2

    def footprint (self, bench):
        return [("circle", *self.centre (bench), self.RADIUS)]

    def shapes (self, bench):
        mx, my = self.centre (bench)
        return [("circle", mx, my - self.RISE / 2, self.RADIUS + 1.5)]

    def labels (self, bench):
        mx, my = self.centre (bench)
        r, top = self.RADIUS, my - self.RISE
        return [Label (self.name, spots_round ((mx - r, top - r, mx + r, my + r), self.name,
                                               bench.label_size), (mx, top - r))]

    def draw (self, pencil, bench):
        mx, my = self.centre (bench)
        # Seen almost from above, so it covers only the holes it really does.
        r, rise = self.RADIUS, self.RISE
        top = my - rise
        if self.kind == "passive":
            pencil.spot (mx, my, r, "#6fa860")
        pencil.spot (mx, my - (3 if self.kind == "passive" else 0), r, "#2f2f2f")
        pencil.tint ([(mx - r, top), (mx + r, top), (mx + r, my - 3), (mx - r, my - 3)], "#2f2f2f")
        pencil.line ((mx - r, top), (mx - r, my), width=1.0, layer="top")
        pencil.line ((mx + r, top), (mx + r, my), width=1.0, layer="top")
        pencil.circle (mx, my, r, width=1.0, tone=0.6, layer="top", passes=1)
        pencil.spot (mx, top, r, "#454545")
        pencil.circle (mx, top, r, width=1.0, layer="top")
        pencil.circle (mx, top, r - 3, width=0.6, tone=0.4, layer="top", passes=1)
        if self.kind == "active":
            pencil.spot (mx, top, r * 0.62, "#f4f2ea")
            pencil.circle (mx, top, r * 0.62, width=0.7, layer="top", passes=1)
            pencil.text (mx, top - 1, "REMOVE SEAL", size=2.7, kind="silk", weight="bold")
            pencil.text (mx, top + 3.4, "AFTER WASHING", size=2.4, kind="silk")
        else:
            pencil.spot (mx, top, 3.2, "#111111")
        px, _ = bench.hole_xy (self.positive)
        side = 1 if px > mx else -1
        pencil.text (mx + side * r * 0.72, top + 4.5, "+", size=12, kind="silk", weight="bold",
                     color="#f4f1e8")


class Potentiometer (Part):
    # The kit's 10 kΩ knob: three legs in a row, the wiper in the middle, its
    # body standing behind them.
    mode = "input"

    def __init__ (self, left, wiper, right, value):
        self.holes = [left, wiper, right]
        self.value = value
        self.name = "potentiometer"

    def legs (self):
        return list (zip (("left leg", "wiper, middle leg", "right leg"), self.holes))

    def body (self, bench):
        points = [bench.hole_xy (hole) for hole in self.holes]
        cx = sum (x for x, _ in points) / 3
        y = min (y for _, y in points)
        half = max (21, (points[2][0] - points[0][0]) / 2 + 6)
        return cx, y, half

    def footprint (self, bench):
        cx, y, half = self.body (bench)
        return [("rect", cx - half, y - 2 * half - 4, cx + half, y - 4)]

    def shapes (self, bench):
        cx, y, half = self.body (bench)
        legs = [lead_shape (bench.hole_xy (hole), (bench.hole_xy (hole)[0], y - 5))
                for hole in self.holes]
        return [("rect", cx - half, y - 2 * half - 4, cx + half, y - 4)] + legs

    def labels (self, bench):
        cx, y, half = self.body (bench)
        box = (cx - half, y - 2 * half - 4, cx + half, y - 4)
        return [Label (self.value, spots_round (box, self.value, bench.label_size), (cx, box[1]))]

    def draw (self, pencil, bench):
        cx, y, half = self.body (bench)
        for hole in self.holes:
            hx, hy = bench.hole_xy (hole)
            pencil.lead ((hx, hy), (hx, y - 5))
        top = y - 2 * half - 4
        pencil.tint (rounded (cx - half, top, 2 * half, 2 * half, 3), "#7fa6d9")
        pencil.rect (cx - half, top, 2 * half, 2 * half, width=1.0, radius=3, layer="top")
        kx, ky, kr = cx, top + half, half * 0.72
        pencil.spot (kx, ky, kr, "#efeae0")
        pencil.circle (kx, ky, kr, width=1.0, layer="top")
        for index in range (18):
            angle = index * math.pi / 9
            pencil.line ((kx + (kr - 3) * math.cos (angle), ky + (kr - 3) * math.sin (angle)),
                         (kx + kr * math.cos (angle), ky + kr * math.sin (angle)), width=0.5,
                         tone=0.4, layer="top", passes=1, wobble=0.15)
        angle = math.radians (-60)
        pencil.line ((kx, ky), (kx + (kr - 4) * math.cos (angle), ky + (kr - 4) * math.sin (angle)),
                     width=2.0, layer="top", passes=1)


class TwoLegs (Part):
    # A small part on two legs: a photoresistor, a thermistor or a tilt
    # switch. Either way round.
    mode = "input"

    def __init__ (self, kind, a, b):
        self.kind, self.a, self.b = kind, a, b
        self.name = kind

    def legs (self):
        return [("one leg", self.a), ("other leg", self.b)]

    # Standing in one column, across the middle gap, its body sits between
    # its legs; otherwise it leans back above them.
    def standing (self, bench):
        (x1, _), (x2, _) = bench.hole_xy (self.a), bench.hole_xy (self.b)
        return abs (x1 - x2) < 1

    # Its centre, the box its body fills, and where each leg meets it.
    def geometry (self, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (self.a), bench.hole_xy (self.b)
        half_w, half_h, low = {"photoresistor": (10.5, 10.5, 7), "thermistor": (6.5, 9, 3),
                               "tilt switch": (11, 19.5, 10)}[self.kind]
        if self.standing (bench):
            cx, cy = x1, (y1 + y2) / 2
            box = (cx - half_w, cy - half_h, cx + half_w, cy + half_h)
            top, bottom = sorted (((x1, y1), (x2, y2)), key=lambda p: p[1])
            feet = [(top, (cx, cy - half_h + 2)), (bottom, (cx, cy + half_h - 2))]
            return cx, cy, box, feet
        cx, cy = (x1 + x2) / 2, min (y1, y2) - 20
        box = (cx - half_w, cy - half_h - (12 if self.kind == "tilt switch" else 0), cx + half_w,
               cy + half_h - (7.5 if self.kind == "tilt switch" else 0))
        spread = {"photoresistor": 5, "thermistor": 1.5, "tilt switch": 4}[self.kind]
        feet = [((x1, y1), (cx - spread, cy + low)), ((x2, y2), (cx + spread, cy + low))]
        return cx, cy, box, feet

    def shapes (self, bench):
        _, _, box, feet = self.geometry (bench)
        return [("rect", *box)] + [lead_shape (a, b) for a, b in feet]

    def labels (self, bench):
        cx, _, box, _ = self.geometry (bench)
        first = "right" if self.standing (bench) else "above"
        return [Label (self.name, spots_round (box, self.name, bench.label_size, first),
                       (box[2], (box[1] + box[3]) / 2) if first == "right" else (cx, box[1]))]

    def draw (self, pencil, bench):
        cx, cy, _, feet = self.geometry (bench)
        for a, b in feet:
            pencil.lead (a, b)
        if self.kind == "photoresistor":
            pencil.spot (cx, cy, 10.5, "#f3e9d2")
            track = []
            for index in range (7):
                ty = cy - 7 + index * 2.35
                half = math.sqrt (max (0.0, 7.2 ** 2 - (ty - cy) ** 2))
                ends = [(cx - half, ty), (cx + half, ty)]
                track += ends if index % 2 else ends[::-1]
            pencil.tint ([(cx - 10, cy - 5), (cx - 7.5, cy - 5), (cx - 7.5, cy + 5), (cx - 10, cy + 5)],
                         "#b8a47a")
            pencil.tint ([(cx + 7.5, cy - 5), (cx + 10, cy - 5), (cx + 10, cy + 5), (cx + 7.5, cy + 5)],
                         "#b8a47a")
            self._track (pencil, track)
            pencil.circle (cx, cy, 10.5, width=1.0, layer="top")
            pencil.spot (cx - 3, cy - 4, 4, "#ffffff", opacity=0.45)
        elif self.kind == "thermistor":
            bead = [(cx + 6.5 * math.cos (a / 12 * math.pi), cy - 2 + 7.5 * math.sin (a / 12 * math.pi))
                    for a in range (24)]
            bead = [(x, y + (4 if y > cy - 2 else 0) * abs (x - cx) / 7) for x, y in bead]
            pencil.tint (bead, "#2d2d2d")
            pencil.polyline (bead, width=1.1, closed=True, layer="top")
            pencil.spot (cx - 2, cy - 5, 2, "#ffffff", opacity=0.4)
        else:
            top = cy - (4 if self.standing (bench) else 16)
            pencil.tint (rounded (cx - 11, top, 22, 28 - (12 if self.standing (bench) else 0), 4),
                         "#3a3a3a")
            pencil.rect (cx - 11, top, 22, 28 - (12 if self.standing (bench) else 0), width=1.0,
                         radius=4, layer="top")
            pencil.spot (cx, top, 11, "#555555")
            pencil.circle (cx, top, 11, width=1.1, layer="top", passes=1)
            pencil.spot (cx - 5, top + 12, 2.5, "#ffffff", opacity=0.3)

    def _track (self, pencil, track):
        pencil.layers["top"].append (
            '<path d="M ' + " L ".join (f"{x:.1f} {y:.1f}" for x, y in track) +
            '" fill="none" stroke="#c4563a" stroke-width="1.1" stroke-linejoin="round"/>')


class Chip (Part):
    # A DIP chip across the middle gap: pin 1 at the bottom left beside the
    # notch, counting anticlockwise seen from above.
    mode = "output"

    def __init__ (self, name, pins, holes):
        self.name, self.pins, self.holes = name, list (pins), holes

    def legs (self):
        return [(f"{pin} (pin {number})", hole)
                for number, (pin, hole) in enumerate (zip (self.pins, self.holes), 1)]

    # Pins with the same name are one pin inside: the L293D's four GNDs.
    def inside (self):
        legs = self.legs ()
        joins = []
        for index, (leg, _) in enumerate (legs):
            for other, _ in legs[index + 1:]:
                if leg.split (" (pin")[0] == other.split (" (pin")[0]:
                    joins.append ((leg, other))
        return joins

    def corners (self, bench):
        half = len (self.pins) // 2
        (x1, bottom), (x2, _) = bench.hole_xy (self.holes[0]), bench.hole_xy (self.holes[half - 1])
        _, top = bench.hole_xy (self.holes[half])
        return x1, x2, top, bottom

    def outline (self, bench):
        x1, x2, top, bottom = self.corners (bench)
        return x1 - 5.5, top + 2.5, x2 + 5.5, bottom - 2.5

    def shapes (self, bench):
        x1, x2, top, bottom = self.corners (bench)
        return [("rect", x1 - 5.5, top - 2.5, x2 + 5.5, bottom + 2.5)]

    def labels (self, bench):
        x1, x2, top, bottom = self.corners (bench)
        box = (x1 - 5.5, top - 3, x2 + 5.5, bottom + 3)
        return [Label (self.name, spots_round (box, self.name, bench.label_size, gap=4),
                       ((x1 + x2) / 2, top))]

    def draw (self, pencil, bench):
        x1, x2, top, bottom = self.corners (bench)
        left, upper, right, lower = self.outline (bench)
        # The legs' shoulders, splayed out to the holes from under the body.
        for hole in self.holes:
            hx, hy = bench.hole_xy (hole)
            y1, y2 = (hy - 2.2, upper) if hy < upper else (lower, hy + 2.2)
            box = [(hx - 2.1, y1), (hx + 2.1, y1), (hx + 2.1, y2), (hx - 2.1, y2)]
            pencil.tint (box, "#c4c3bd")
            pencil.polyline (box, width=0.4, tone=0.6, closed=True, layer="top")
        pencil.tint (rounded (left, upper, right - left, lower - upper, 1.5), "#2e2e2e")
        pencil.rect (left, upper, right - left, lower - upper, width=1.0, radius=1.5, layer="top")
        middle = (upper + lower) / 2
        notch = [(left, middle - 3.5)] + [(left + 3.5 * math.sin (a / 8 * math.pi),
                                           middle - 3.5 * math.cos (a / 8 * math.pi)) for a in range (9)]
        pencil.tint (notch, "#777777")
        pencil.spot (x1 + 2.2, lower - 2.8, 1.1, "#9a9a9a")
        # Each pin's name printed beside it, the bottom row's on the left of
        # its pin and the top row's on the right, so both fit.
        half = len (self.pins) // 2
        for index, pin in enumerate (self.pins):
            hx, hy = bench.hole_xy (self.holes[index])
            if index < half:
                pencil.text (hx - 0.4, lower - 2.2, pin, size=4, rotate=-90, anchor="start",
                             kind="silk", color="#f4f1e8")
            else:
                pencil.text (hx + 3.6, upper + 2.2, pin, size=4, rotate=-90, anchor="end",
                             kind="silk", color="#f4f1e8")


class Display (Chip):
    # A seven-segment display across the middle gap on 0.6 inch rows: its
    # body covers its own pins, so the drawing shows the face and the
    # decimal points, which sit at the bottom right.
    SEGMENTS = {"a": ((0.1, 0), (0.9, 0)), "b": ((1, 0.05), (1, 0.45)),
                "c": ((1, 0.55), (1, 0.95)), "d": ((0.1, 1), (0.9, 1)),
                "e": ((0, 0.55), (0, 0.95)), "f": ((0, 0.05), (0, 0.45)),
                "g": ((0.1, 0.5), (0.9, 0.5))}
    FONT = {"0": "abcdef", "1": "bc", "2": "abdeg", "3": "abcdg", "4": "bcfg", "5": "acdfg",
            "6": "acdefg", "7": "abc", "8": "abcdefg", "9": "abcdfg", "-": "g", " ": "",
            "A": "abcefg", "b": "cdefg", "C": "adef", "d": "bcdeg", "E": "adefg", "F": "aefg",
            "H": "bcefg", "L": "def", "P": "abefg", "o": "cdeg", "r": "eg", "u": "cde"}

    def __init__ (self, name, pins, labels, holes, width, digits, shows):
        super ().__init__ (name, pins, holes)
        self.names, self.width, self.digits = labels, width, digits
        self.shows = shows or ""

    def legs (self):
        return [(f"{label} (pin {number})", hole)
                for number, (label, hole) in enumerate (zip (self.names, self.holes), 1)]

    def rectangle (self, bench):
        x1, x2, top, bottom = self.corners (bench)
        cx, cy = (x1 + x2) / 2, (top + bottom) / 2
        return cx - self.width / 2, cy - 37.5, cx + self.width / 2, cy + 37.5

    def footprint (self, bench):
        return [("rect", *self.rectangle (bench))]

    # The body, and the printed 1 by pin 1 just below it.
    def shapes (self, bench):
        left, upper, right, lower = self.rectangle (bench)
        x1, _, _, _ = self.corners (bench)
        return [("rect", left, upper, right, lower), ("rect", x1 - 3, lower, x1 + 3, lower + 11)]

    def labels (self, bench):
        left, upper, right, lower = self.rectangle (bench)
        size = bench.label_size
        name = Label (self.name, spots_round ((left, upper, right, lower + 12), self.name, size),
                      ((left + right) / 2, upper))
        return [name]

    def draw (self, pencil, bench):
        left, upper, right, lower = self.rectangle (bench)
        pencil.tint (rounded (left, upper, right - left, lower - upper, 2), "#2b2b2b")
        pencil.rect (left, upper, right - left, lower - upper, width=1.0, radius=2, layer="top")
        pencil.rect (left + 3, upper + 3, right - left - 6, lower - upper - 6, width=0.6, tone=0.35,
                     radius=1.5, layer="top", passes=1)
        lit = self._lit ()
        pitch = (right - left) / self.digits
        for digit in range (self.digits):
            ox, oy = left + pitch * digit + pitch / 2 - 12, upper + 9
            for segment, ((ax, ay), (bx, by)) in self.SEGMENTS.items ():
                self._segment (pencil, ox, oy, ax, ay, bx, by, segment in lit[digit][0])
            self._dot (pencil, ox + 30, oy + 56, lit[digit][1])
        x1, _, _, _ = self.corners (bench)
        pencil.text (x1, lower + 9, "1", size=7, kind="silk", tone=0.7)

    def _lit (self):
        cells = []
        for character in self.shows:
            if character == "." and cells:
                cells[-1] = (cells[-1][0], True)
            else:
                cells.append ((self.FONT.get (character, ""), False))
        cells = cells[:self.digits]
        return cells + [("", False)] * (self.digits - len (cells))

    # A segment: a slanted bar 24 wide and 56 tall per digit, leaning 10°.
    def _segment (self, pencil, ox, oy, ax, ay, bx, by, lit):
        lean = math.tan (math.radians (10))
        point = lambda u, v: (ox + u * 24 + (1 - v) * 56 * lean, oy + v * 56)
        (x1, y1), (x2, y2) = point (ax, ay), point (bx, by)
        length = math.dist ((x1, y1), (x2, y2))
        ux, uy = (x2 - x1) / length, (y2 - y1) / length
        nx, ny = -uy * 3.0, ux * 3.0
        bar = [(x1, y1), (x1 + ux * 2.6 + nx, y1 + uy * 2.6 + ny),
               (x2 - ux * 2.6 + nx, y2 - uy * 2.6 + ny), (x2, y2),
               (x2 - ux * 2.6 - nx, y2 - uy * 2.6 - ny), (x1 + ux * 2.6 - nx, y1 + uy * 2.6 - ny)]
        if lit:
            pencil.tint (bar, "#ff5a48", opacity=0.35)
            pencil.polyline (bar, width=3.0, tone=0.0, layer="top", closed=True, passes=1)
        pencil.tint (bar, "#ff4a3a" if lit else "#dcd8cf", opacity=1.0 if lit else 0.85)

    def _dot (self, pencil, x, y, lit):
        pencil.spot (x, y, 2.8, "#ff4a3a" if lit else "#dcd8cf")


class PowerModule (Part):
    # The breadboard power supply: it plugs into both pairs of rails at one
    # end of the board, its barrel jack and USB socket hanging over the end.
    # Each side has its own jumper: 5 V, 3.3 V or off; its GND is always on.
    blocks = False                  # it is low: wires may lie over it
    LENGTH, OVERHANG = 1.3 * DPI, 0.3 * DPI
    SETTINGS = {"5V": "5 V", "3.3V": "3.3 V", "off": None}

    def __init__ (self, end, columns, top="5V", bottom="5V"):
        for side in (top, bottom):
            if side not in self.SETTINGS:
                raise ValueError (f"a power module's jumper is on 5V, 3.3V or off, not {side!r}")
        self.end, self.columns, self.top, self.bottom = end, columns, top, bottom
        self.name = "power module"
        self.sources = tuple ({self.SETTINGS[s] for s in (top, bottom) if self.SETTINGS[s]}) + ("GND",)

    # A rail pair switched off still has its pins in the + rail, joined to
    # nothing.
    def legs (self):
        legs = []
        for side, setting, plus, minus in (("top", self.top, "T+", "T-"),
                                           ("bottom", self.bottom, "B+", "B-")):
            voltage = self.SETTINGS[setting] or f"{side} + pin, switched off"
            legs += [(voltage, f"{plus}{column}") for column in self.columns]
            legs += [("GND", f"{minus}{column}") for column in self.columns]
        return legs

    # Its outline: sign is -1 when it hangs off the left end.
    def frame (self, bench):
        sign = -1 if self.end == "left" else 1
        xs = [bench.hole_xy (f"T+{column}")[0] for column in self.columns]
        near = (max if sign < 0 else min) (xs) - sign * 18
        far = near + sign * self.LENGTH
        _, top = bench.hole_xy (f"T+{self.columns[0]}")
        _, bottom = bench.hole_xy (f"B-{self.columns[0]}")
        return sign, near, far, top - 13, bottom + 13

    def footprint (self, bench):
        sign, near, far, top, bottom = self.frame (bench)
        return [("rect", min (near, far), top, max (near, far), bottom)]

    def box (self, bench):
        sign, near, far, top, bottom = self.frame (bench)
        outer = far + sign * self.OVERHANG
        return min (near, outer), top, max (near, outer), bottom

    def shapes (self, bench):
        return [("rect", *self.box (bench))]

    def labels (self, bench):
        sign, near, far, top, bottom = self.frame (bench)
        size = bench.label_size
        x = (near + far) / 2 + sign * 30
        spots = [(x, top - 4 - size * 0.26, "middle", 0), (x, bottom + 4 + size * 0.8, "middle", 4)]
        return [Label (self.name, spots, (x, top))]

    # Drawn beneath the wires, which lie over it.
    def draw (self, pencil, bench):
        sign, near, far, top, bottom = self.frame (bench)
        x = lambda d: near + sign * d
        left, right = min (near, far), max (near, far)
        for _, hole in self.legs ():
            hx, hy = bench.hole_xy (hole)
            pencil.tint ([(hx - 2.2, hy - 2.2), (hx + 2.2, hy - 2.2), (hx + 2.2, hy + 2.2),
                          (hx - 2.2, hy + 2.2)], "#1d1d1d", layer="shade")
        pencil.solid (rounded (left, top, right - left, bottom - top, 4), "#d3dde9", layer="crisp")
        pencil.tint (rounded (left, top, right - left, bottom - top, 4), "#c5d4e6", layer="shade")
        pencil.rect (left, top, right - left, bottom - top, width=1.0, radius=4, layer="ink")
        for _, hole in self.legs ():
            hx, hy = bench.hole_xy (hole)
            pencil.tint ([(hx - 3, hy - 3), (hx + 3, hy - 3), (hx + 3, hy + 3), (hx - 3, hy + 3)],
                         "#c9c3b0", layer="shade")
            pencil.rect (hx - 3, hy - 3, 6, 6, width=0.6, tone=0.6, layer="ink", passes=1)
        # Each side's output jumper, three pins in a row: its cap joins the
        # pair by 5V or the pair by 3.3V, or is parked on the end pin, off.
        for setting, y, below in ((self.top, top + 42, True), (self.bottom, bottom - 42, False)):
            for index in range (3):
                pencil.spot (x (22 + index * 10), y, 1.8, "#9a9a9a", layer="shade")
            cap = {"5V": (17, 37), "3.3V": (27, 47), "off": (38, 47)}[setting]
            pencil.tint ([(x (cap[0]), y - 4.5), (x (cap[1]), y - 4.5), (x (cap[1]), y + 4.5),
                          (x (cap[0]), y + 4.5)], "#1f1f1f", layer="shade")
            for word, along, rise in (("5V", 10, 2.2), ("3.3V", 58, 2.2),
                                      ("OFF", 32, 12 if below else -7)):
                chosen = setting == ("off" if word == "OFF" else word)
                pencil.text (x (along), y + rise, word, size=6 if chosen else 5, kind="silk",
                             weight="bold" if chosen else "normal", tone=0.95 if chosen else 0.55)
        cy = (top + bottom) / 2
        # The regulators, switch and LED, the USB socket and the barrel jack.
        for y in (cy - 40, cy + 28):
            pencil.tint ([(x (60), y), (x (80), y), (x (80), y + 12), (x (60), y + 12)], "#2e2e2e",
                         layer="shade")
        pencil.tint ([(x (92), cy - 14), (x (108), cy - 14), (x (108), cy + 2), (x (92), cy + 2)],
                     "#f1efe8", layer="shade")
        pencil.circle (x (100), cy - 6, 5, width=0.9, layer="ink", passes=1)
        pencil.spot (x (100), cy + 14, 3, "#8fd18a", layer="shade")
        usb = sorted ((x (104), x (self.LENGTH + 12)))
        pencil.tint ([(usb[0], cy + 38), (usb[1], cy + 38), (usb[1], cy + 70), (usb[0], cy + 70)],
                     METAL, layer="shade")
        pencil.rect (usb[0], cy + 38, usb[1] - usb[0], 32, width=1.2, layer="ink")
        pencil.hatch (usb[0] + 2, cy + 42, usb[1] - usb[0] - 4, 24, gap=2.4, angle=0, tone=0.3,
                      layer="ink")
        jack = sorted ((x (78), x (self.LENGTH + self.OVERHANG)))
        pencil.tint ([(jack[0], cy - 86), (jack[1], cy - 86), (jack[1], cy - 44), (jack[0], cy - 44)],
                     "#2a2a2a", layer="shade")
        pencil.rect (jack[0], cy - 86, jack[1] - jack[0], 42, width=1.0, layer="ink")
        pencil.spot (x (self.LENGTH + self.OVERHANG - 6), cy - 65, 7, "#555555", layer="shade")
        for rail, hole in (("+", "T+"), ("−", "T-"), ("+", "B+"), ("−", "B-")):
            _, hy = bench.hole_xy (f"{hole}{self.columns[0]}")
            pencil.text (x (46), hy + 4, rail, size=11, kind="silk", weight="bold")
        pencil.text ((near + far) / 2, cy + 2, "MB102", size=8, kind="silk", tone=0.6)


class HeaderModule (Part):
    # A module standing in one row on its own header, its body drawn lying
    # off the nearer edge of the board. Turned, it lies off the bottom edge,
    # and its pins meet the columns in the opposite order.
    GAP = 7                         # from the holes to the edge of its board

    def __init__ (self, kind, holes, turned, name):
        self.kind, self.holes, self.turned = kind, holes, turned
        self.name = name

    # Its pins and their holes, left to right along the row.
    def pairs (self):
        pins = self.kind.header ()
        return list (zip (reversed (pins) if self.turned else pins, self.holes))

    def legs (self):
        return [(pin.label (), hole) for pin, hole in self.pairs ()]

    def modes (self):
        return [self.kind.pin_modes.get (pin.name) for pin, _ in self.pairs ()]

    def placed (self, bench):
        pins = self.kind.header ()
        gap = 0 if self.kind.inset else self.GAP
        hx, hy = bench.hole_xy (self.holes[-1] if self.turned else self.holes[0])
        if self.turned:
            x, y = hx + pins[0].x, hy + pins[0].y + gap
        else:
            x, y = hx - pins[0].x, hy - pins[0].y - gap
        placed = Placed (self.kind, self.name, x, y, 180 if self.turned else 0, reach=gap)
        placed.title = self.name
        return placed

    def footprint (self, bench):
        return [("rect", *self.placed (bench).box ())]

    def box (self, bench):
        return self.placed (bench).box ()

    def shapes (self, bench):
        placed = self.placed (bench)
        return [("rect", *placed.box ()), ("rect", *placed.title_box ())]

    def draw (self, pencil, bench):
        self.placed (bench).draw (pencil, size=bench.label_size)


def bands_for (value):
    # A 1% resistor's five bands: three digits, how many zeros follow, and
    # brown for 1%. "220 Ω" is red, red, black, black, brown.
    names = ["black", "brown", "red", "orange", "yellow", "green", "blue", "violet", "grey",
             "white"]
    match = re.fullmatch (r"([\d.]+)\s*(k|M)?\s*Ω", value)
    ohms = float (match.group (1)) * {None: 1, "k": 1e3, "M": 1e6}[match.group (2)]
    digits = f"{ohms:.0f}"
    if len (digits) < 3:
        raise ValueError (f"{value}: five-band resistors below 100 Ω need a gold multiplier")
    zeros = len (digits) - 3
    return [names[int (digits[0])], names[int (digits[1])], names[int (digits[2])],
            names[zeros], "brown"]

