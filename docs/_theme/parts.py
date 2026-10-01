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
from route import distance_to, segment_distance, text_width

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

    # Its body without its leads, and its leads alone, which a standing
    # part beside it keeps clear of (aside).
    def bodies (self, bench):
        return [s for s in self.shapes (bench) if not is_lead (s)]

    def leads (self, bench):
        return [s for s in self.shapes (bench) if is_lead (s)]

    # A body off the board, which grows the drawing and wires go round.
    def box (self, bench):
        return None

    def labels (self, bench):
        return []

    def draw (self, pencil, bench):
        pass


# The parts in the order they are drawn: seen a little from the front, a
# part whose legs stand nearer the front edge stands in front, over any
# part behind it that it overlaps, as an LED in row b does over the knob
# whose wiper is in row d. Parts level with each other keep the build's
# order.
def back_to_front (parts, bench):
    def front (part):
        return max ((bench.hole_xy (hole)[1] for _, hole in part.legs ()), default=0)
    return sorted (parts, key=front)


def lead_shape (a, b):
    return ("segment", a[0], a[1], b[0], b[1], LEAD)


def is_lead (shape):
    return shape[0] == "segment" and shape[5] <= LEAD


# Where a standing two-lead part's body stands (Resistor.geometry): halfway
# between its legs, or, where that would lie over a hole in use, or over
# another part's body or legs, half a column aside, wherever along keeps
# the body (about 4 across each way, a disc 8 round) and its bent leads
# clearest of them. Nearer the middle, half a column aside rather than more,
# and the right-hand side win a tie. Other standing two-lead parts don't
# count: each finds its own place.
BOW = 5


def aside (part, bench):
    (x1, y1), (x2, y2) = bench.hole_xy (part.a), bench.hole_xy (part.b)
    span = math.dist ((x1, y1), (x2, y2))
    ux, uy = (x2 - x1) / span, (y2 - y1) / span
    half = part.LENGTH / 2
    held = [bench.hole_xy (hole) for hole in bench.used if hole not in (part.a, part.b)]
    others = [other for other in bench.parts if other is not part
              and not isinstance (other, (Resistor, Capacitor, Inductor, Diode))]
    bodies = [shape for other in others for shape in other.bodies (bench)]
    legs = [shape for other in others for shape in other.leads (bench)]

    def at (along, off):
        return x1 + ux * along - uy * off, y1 + uy * along + ux * off

    def room (along, off):
        ends = at (along - half, off), at (along + half, off)
        pieces = (((x1, y1), ends[0], LEAD), (*ends, 4.0), (ends[1], (x2, y2), LEAD))
        holes = min ((segment_distance (a, b, hole) - reach for a, b, reach in pieces
                      for hole in held), default=math.inf)
        # The body as drawn, a disc or a stadium, against other bodies; the
        # bent leads against other parts' legs.
        if part.RADIUS:
            outline = [(at (along, off), part.RADIUS)]
        else:
            outline = [(at (along + half * k / 4, off), part.GIRTH) for k in range (-4, 5)]
        clear = min ((distance_to (shape, point) - reach for point, reach in outline
                      for shape in bodies), default=math.inf)
        apart = min ((segments_apart ((a, b), (shape[1:3], shape[3:5])) - 2 * LEAD
                      for a, b, _ in pieces[::2] for shape in legs), default=math.inf)
        return holes, clear, apart

    if all (amount > least for amount, least in zip (room (span / 2, 0), (2.5, 1, 1.5))):
        return at (span / 2, 0)
    tries = [(along, off) for off in (-BOW - 1, -BOW, BOW, BOW + 1)
             for along in range (math.ceil (half + 2), math.floor (span - half - 2) + 1)]
    if not tries:
        return at (span / 2, 0)
    along, off = max (tries, key=lambda t: (round (min (room (*t)), 1), -abs (t[0] - span / 2),
                                            -abs (abs (t[1]) - BOW), at (*t)[0]))
    return at (along, off)


# The least distance between two segments, each given by its two ends.
def segments_apart (one, other):
    (a, b), (p, q) = one, other
    if segments_cross (a, b, p, q):
        return 0.0
    return min (segment_distance (a, b, p), segment_distance (a, b, q),
                segment_distance (p, q, a), segment_distance (p, q, b))


def segments_cross (a, b, p, q):
    def side (o, s, t):
        return (s[0] - o[0]) * (t[1] - o[1]) - (s[1] - o[1]) * (t[0] - o[0])
    return side (a, b, p) * side (a, b, q) < 0 and side (p, q, a) * side (p, q, b) < 0


# A standing part's leads from each hole to its body's ends, reach from its
# middle, and then its body (body (), drawn at the origin), turned along it.
def stand (part, pencil, bench, reach, body):
    a, b = bench.hole_xy (part.a), bench.hole_xy (part.b)
    cx, cy = part.geometry (bench)
    angle = math.atan2 (b[1] - a[1], b[0] - a[0])
    ux, uy = math.cos (angle), math.sin (angle)
    pencil.lead (a, (cx - ux * reach, cy - uy * reach))
    pencil.lead ((cx + ux * reach, cy + uy * reach), b)
    pencil.begin (cx, cy, math.degrees (angle))
    body ()
    pencil.end ()


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
    GIRTH, RADIUS = 4.9, None           # its body's half width; no disc

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

    # Standing, its body would lie over any hole between its legs. Where one
    # holds another part's leg or a wire's end, as a30 holds the S8050's
    # base under the active buzzer's 10 kΩ from b30, the body would hide it
    # and seem to plug into it, so the leads bend and the body stands half a
    # column aside, as far along as keeps it clearest of the holes in use,
    # and on the side that does that better.
    def geometry (self, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (self.a), bench.hole_xy (self.b)
        if not self.standing (bench):
            return (x1 + x2) / 2, (y1 + y2) / 2 - 7
        key = (id (bench), len (bench.used))
        if getattr (self, "_placed", (None,))[0] != key:
            self._placed = key, aside (self, bench)
        return self._placed[1]

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
            stand (self, pencil, bench, length / 2 - 1, lambda: self._body (pencil, 0, 0, length))
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


class Capacitor (Part):
    # A small two-lead capacitor: an electrolytic can, a ceramic disc or a
    # film box. On an electrolytic, the stripe marks −; it is drawn on the
    # second leg's side and named in the build step. Unless kind says, a
    # polarized one is electrolytic, and a nonpolar one is a film box from
    # 1 µF up, a ceramic disc below.
    LENGTH = 18
    KINDS = ("electrolytic", "ceramic", "film")
    standing = Resistor.standing
    labels = Resistor.labels

    def __init__ (self, value, a, b, polarized=False, kind=None):
        self.value, self.a, self.b, self.polarized = value, a, b, polarized
        self.name = f"{value} capacitor"
        if kind is None:
            kind = "electrolytic" if polarized else "film" if farads (value) >= 1e-6 else "ceramic"
        if kind not in self.KINDS or (kind == "electrolytic") != polarized:
            raise ValueError (f"a {value} capacitor is {', '.join (self.KINDS)}, and only an "
                              f"electrolytic is polarized: not {kind!r}")
        self.kind = kind
        # A can or disc is round, 8 across each way; a film box 4.5 thick.
        self.RADIUS = None if kind == "film" else 8
        self.GIRTH = 4.5

    def legs (self):
        if self.polarized:
            return [("+ leg", self.a), ("striped − leg", self.b)]
        return [("one leg", self.a), ("other leg", self.b)]

    # Standing across rows, as a resistor does. In one row, upright on legs
    # side by side, it leans back a row, as an LED does, so both holes show
    # in front of it.
    def geometry (self, bench):
        if self.standing (bench):
            return Resistor.geometry (self, bench)
        (x1, y1), (x2, y2) = bench.hole_xy (self.a), bench.hole_xy (self.b)
        return (x1 + x2) / 2, (y1 + y2) / 2 - (8.5 if self.kind == "film" else 10)

    # Where each leg, in one row, meets the body's underside: the left leg
    # first.
    def _feet (self, bench):
        left, right = sorted ((bench.hole_xy (self.a), bench.hole_xy (self.b)))
        cx, cy = self.geometry (bench)
        spread, low = (4, 4.5) if self.kind == "film" else (2.5, 5)
        return [(left, (cx - spread, cy + low)), (right, (cx + spread, cy + low))]

    def shapes (self, bench):
        if self.standing (bench):
            return Resistor.shapes (self, bench)
        cx, cy = self.geometry (bench)
        w, h = (9, 4.5) if self.kind == "film" else (8, 8)
        return [("rect", cx - w, cy - h, cx + w, cy + h)] + [lead_shape (a, b)
                                                          for a, b in self._feet (bench)]

    def draw (self, pencil, bench):
        (x1, _), (x2, _) = bench.hole_xy (self.a), bench.hole_xy (self.b)
        if self.standing (bench):
            stand (self, pencil, bench, self.LENGTH / 2, lambda: self._body (pencil, 0, 0))
            return
        for a, b in self._feet (bench):
            pencil.lead (a, b)
        self._body (pencil, *self.geometry (bench), stripe_side=-1 if x2 < x1 else 1)

    def _body (self, pencil, cx, cy, stripe_side=1):
        if self.kind == "film":
            # A red box with a sheen along it.
            pencil.tint (rounded (cx - 9, cy - 4.5, 18, 9, 1.5), "#c4574d")
            pencil.layers["top"].append (
                f'<path d="M {cx - 6.5:.1f} {cy - 2.4:.1f} L {cx + 6.5:.1f} {cy - 2.4:.1f}" '
                f'stroke="#ffffff" stroke-opacity="0.4" stroke-width="0.9" '
                f'stroke-linecap="round"/>')
            pencil.rect (cx - 9, cy - 4.5, 18, 9, width=0.8, radius=1.5, layer="top")
            return
        color = "#3f4851" if self.polarized else "#d4ab74"
        pencil.spot (cx, cy, 8, color)
        pencil.circle (cx, cy, 8, width=0.8, layer="top")
        if self.polarized:
            pencil.band (cx + (4.5 if stripe_side > 0 else -7), cy - 6, 2.5, 12,
                         "#d8dce0")


# A capacitor's value in farads: "100 nF" is 1e-7.
def farads (value):
    match = re.fullmatch (r"([\d.]+)\s*([pnuµμ]?)F", value)
    if not match:
        raise ValueError (f"a capacitor's value is a number and pF, nF, µF or F, not {value!r}")
    scale = {"p": 1e-12, "n": 1e-9, "u": 1e-6, "µ": 1e-6, "μ": 1e-6, "": 1}
    return float (match.group (1)) * scale[match.group (2)]


class Inductor (Part):
    # A two-lead coil, drawn with its turns visible above the breadboard.
    LENGTH = 24
    GIRTH, RADIUS = 5, None
    standing = Resistor.standing
    geometry = Resistor.geometry
    shapes = Resistor.shapes
    labels = Resistor.labels

    def __init__ (self, value, a, b):
        self.value, self.a, self.b = value, a, b
        self.name = f"{value} inductor"

    def legs (self):
        return [("one lead", self.a), ("other lead", self.b)]

    def draw (self, pencil, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (self.a), bench.hole_xy (self.b)
        cx, cy = self.geometry (bench)
        if self.standing (bench):
            stand (self, pencil, bench, self.LENGTH / 2, lambda: self._body (pencil, 0, 0))
            return
        pencil.lead ((x1, y1), (cx - self.LENGTH / 2, cy))
        pencil.lead ((cx + self.LENGTH / 2, cy), (x2, y2))
        self._body (pencil, cx, cy)

    def _body (self, pencil, cx, cy):
        pencil.tint (rounded (cx - 12, cy - 5, 24, 10, 3), "#b48a5b")
        pencil.rect (cx - 12, cy - 5, 24, 10, width=0.8, radius=3, layer="top")
        for offset in (-7, -3, 1, 5):
            pencil.band (cx + offset, cy - 5, 1.6, 10, "#704f33")


class Diode (Part):
    # The rectifier's band marks its cathode; unlike a resistor its ends
    # must stay distinguishable in both the drawing and the build steps.
    LENGTH = 18
    GIRTH, RADIUS = 4.5, None
    standing = Resistor.standing
    geometry = Resistor.geometry
    shapes = Resistor.shapes
    labels = Resistor.labels

    def __init__ (self, anode, cathode):
        self.a, self.b = anode, cathode
        self.name = self.value = "1N4007 diode"

    def legs (self):
        return [("anode, unbanded end", self.a), ("cathode, banded end", self.b)]

    def draw (self, pencil, bench):
        a, b = bench.hole_xy (self.a), bench.hole_xy (self.b)
        cx, cy = self.geometry (bench)
        angle = math.degrees (math.atan2 (b[1] - a[1], b[0] - a[0]))
        ux, uy = math.cos (math.radians (angle)), math.sin (math.radians (angle))
        pencil.lead (a, (cx - ux * 9, cy - uy * 9))
        pencil.lead ((cx + ux * 9, cy + uy * 9), b)
        pencil.begin (cx, cy, angle)
        pencil.tint (rounded (-9, -4.5, 18, 9, 2), "#353535")
        pencil.rect (-9, -4.5, 18, 9, radius=2, width=0.8)
        pencil.tint ([(4, -4.5), (6.5, -4.5), (6.5, 4.5), (4, 4.5)], "#d4d4cf")
        pencil.end ()


class Transistor (Part):
    # TO-92 S8050, emitter/base/collector with the marked flat face toward
    # the learner. Spread the three leads to adjacent breadboard columns.
    # Seen from above it is a D, its flat face toward the legs' row and its
    # round back away from it, leaning back a little: the holes of the next
    # row show through it, and the leads and wires in them lie over it. It
    # is drawn smaller than its 4.8 mm by 3.8 mm, so that a wire's end in
    # the next row beside it reads as the wire's, not as a fourth leg, and
    # the hole behind its base, where the active buzzer's 10 kΩ goes, shows.
    name = "S8050 transistor"
    FLAT, DEEP, HALF = 4, 10, 6.5       # the flat face's offset from the legs, and the D's size
    SPREAD = 5                          # between the legs where they meet the face

    def __init__ (self, emitter, base, collector):
        self.holes = (emitter, base, collector)

    def legs (self):
        return list (zip (("E, emitter", "B, base", "C, collector"), self.holes))

    def modes (self):
        return [None, "output", None]

    # The D's outline, where its flat face lies, and which way its back
    # leans from the legs: toward the middle gap.
    def outline (self, bench):
        x, y = bench.hole_xy (self.holes[1])
        back = -1 if self.holes[1][0] in "abcde" else 1
        face = y + back * self.FLAT
        arc = [(x + self.HALF * math.cos (math.pi * step / 12),
                face + back * self.DEEP * math.sin (math.pi * step / 12)) for step in range (13)]
        return arc, face, back

    def shapes (self, bench):
        arc, _, _ = self.outline (bench)
        xs, ys = [px for px, _ in arc], [py for _, py in arc]
        return [("rect", min (xs), min (ys), max (xs), max (ys))]

    # Each leg, from its hole to the flat face: spread as far apart there as
    # the face allows, so a lead from the next row, such as the active
    # buzzer's 10 kΩ from behind the base, finds room between two of them.
    def leads (self, bench):
        x, _ = bench.hole_xy (self.holes[1])
        _, face, _ = self.outline (bench)
        return [lead_shape (bench.hole_xy (hole), (x + (index - 1) * self.SPREAD, face))
                for index, hole in enumerate (self.holes)]

    def labels (self, bench):
        _, x0, y0, x1, y1 = self.shapes (bench)[0]
        return [Label (self.name, spots_round ((x0, y0, x1, y1), self.name,
                                               bench.label_size, "below"), ((x0 + x1) / 2, y1))]

    def draw (self, pencil, bench):
        x, _ = bench.hole_xy (self.holes[1])
        arc, face, back = self.outline (bench)
        for _, ax, ay, bx, by, _ in self.leads (bench):
            pencil.lead ((ax, ay), (bx, by))
        pencil.tint (arc, "#353535", opacity=0.6)
        pencil.polyline (arc, width=0.8, closed=True)
        # The flat face, marked, toward the legs' row. The marking keeps a
        # halo of the body's grey, so a lead passing over reads as under it.
        pencil.line ((x - self.HALF, face), (x + self.HALF, face), width=1.3, tone=0.95)
        pencil.text (x, face + back * 0.9, "8050", size=4, kind="silk",
                     color="#f4f1e8", halo="#858585")


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

    # Its lens leans back two rows, or a little less where that leaves more
    # of the holes in use, or a probe's, in view: at its home, the hole
    # behind its long leg where its resistor ends shows whole beside the
    # flange.
    def dome (self, bench):
        key = (id (bench), len (bench.used), len (bench.measurements), len (bench.scope_probes))
        if getattr (self, "_dome", (None,))[0] != key:
            (x1, y1), (x2, y2) = bench.hole_xy (self.anode), bench.hole_xy (self.cathode)
            # The holes a probe is put in by name; one on a pin or a rail
            # finds a hole of its own, away from the parts.
            probed = {point for taken in bench.measurements
                      for point in (taken["red"], taken["black"])}
            probed |= {point for taken in bench.scope_probes
                       for point in (taken["tip"], taken["ground"])}
            held = [bench.hole_xy (hole)
                    for hole in set (bench.used) | (probed & set (bench.holes ()))
                    if hole not in (self.anode, self.cathode)]

            def hidden (lean):
                middle = ((x1 + x2) / 2, min (y1, y2) - lean)
                return sum (max (0.0, self.RADIUS + 1.8 - math.dist (middle, hole))
                            for hole in held)

            lean = min ((17.5, 20), key=hidden)
            self._dome = key, ((x1 + x2) / 2, min (y1, y2) - lean)
        return self._dome[1]

    # Each leg rises to the side of the dome nearer it, so they never cross.
    def feet (self, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (self.anode), bench.hole_xy (self.cathode)
        cx, cy = self.dome (bench)
        side = -1 if (x1, y2) <= (x2, y1) else 1
        return [((x1, y1), (cx + side * 3.5, cy + 8)), ((x2, y2), (cx - side * 3.5, cy + 8))]

    # Each leg as it is drawn, a run of points from its hole up under the
    # dome. The long leg, the anode, has a knee bent outward a little above
    # its hole, as a long leg bent to stand level with the short one, so
    # the two legs tell + from − as the flange's flat does.
    def bends (self, bench):
        (hole, top), short = self.feet (bench)
        out = -1 if hole[0] < short[0][0] else 1
        knee = (hole[0] + (top[0] - hole[0]) * 0.4 + out * 2.6,
                hole[1] + (top[1] - hole[1]) * 0.4)
        return [(hole, knee, top), short]

    def shapes (self, bench):
        cx, cy = self.dome (bench)
        return [("circle", cx, cy, self.RADIUS)] + [lead_shape (a, b)
                                                    for leg in self.bends (bench)
                                                    for a, b in zip (leg, leg[1:])]

    def labels (self, bench):
        cx, cy = self.dome (bench)
        r = self.RADIUS
        return [Label (self.name, spots_round ((cx - r, cy - r, cx + r, cy + r), self.name,
                                               bench.label_size, "right"), (cx + r, cy))]

    def draw (self, pencil, bench):
        (x1, _), (x2, _) = bench.hole_xy (self.anode), bench.hole_xy (self.cathode)
        cx, cy = self.dome (bench)
        for leg in self.bends (bench):
            for a, b in zip (leg, leg[1:]):
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
        # A letter beside each leg's hole: below it, or along a rail beside
        # it, where below would stand on the other rail. When crowded, the
        # letter moves farther away with a leader back to that exact leg.
        for letter, hole in zip (self.LETTERS, self.holes):
            x, y = bench.hole_xy (hole)
            small = size * 0.7
            below, beside = (9, 0) if re.match (r"[TB][+-]", hole) else (0, 3)
            spots = [(x, y + 4 + small * 0.8, "middle", below), (x - 4, y + small * 0.3, "end", beside),
                     (x + 4, y + small * 0.3, "start", beside)]
            labels.append (Label (letter, spots, (x, y), 0.7, leg=(x, y)))
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
    # The flat, drawn firm, as the transistor's flat face is.
    if flat:
        edge = cx + 9.6 * flat
        half = math.sqrt (r * r - 9.6 * 9.6)
        pencil.line ((edge, cy - half), (edge, cy + half), width=1.4, tone=0.95, layer="top")


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

    # Only the active buzzer's + leg is the longer one.
    def legs (self):
        plus = "+ leg (long)" if self.kind == "active" else "+ leg"
        return [(plus, self.positive), ("− leg", self.negative)]

    def center (self, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (self.positive), bench.hole_xy (self.negative)
        return (x1 + x2) / 2, (y1 + y2) / 2

    def footprint (self, bench):
        return [("circle", *self.center (bench), self.RADIUS)]

    def shapes (self, bench):
        mx, my = self.center (bench)
        return [("circle", mx, my - self.RISE / 2, self.RADIUS + 1.5)]

    def labels (self, bench):
        mx, my = self.center (bench)
        r, top = self.RADIUS, my - self.RISE
        return [Label (self.name, spots_round ((mx - r, top - r, mx + r, my + r), self.name,
                                               bench.label_size), (mx, top - r))]

    def draw (self, pencil, bench):
        mx, my = self.center (bench)
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
        # The + mark on the side of the + leg, whichever way the legs lie:
        # dark on the active one's sticker, clear of its words, and light
        # on the passive one's dark top.
        px, py = bench.hole_xy (self.positive)
        length = math.hypot (px - mx, py - my) or 1
        ux, uy = (px - mx) / length, (py - my) / length
        reach, color = (0.46, "#2b2b2b") if self.kind == "active" else (0.62, "#f4f1e8")
        pencil.text (mx + ux * r * reach, top + uy * r * reach + 4, "+", size=11, kind="silk",
                     weight="bold", color=color, tone=1.0)


class Potentiometer (Part):
    # The kit's 10 kΩ knob, a square body with the knob on top. It stands
    # across the middle gap, its body over the gap between its outer legs
    # and the wiper.
    mode = "input"

    def __init__ (self, left, wiper, right, value):
        self.holes = [left, wiper, right]
        self.value = value
        self.name = "potentiometer"
        self.across = left[0] != wiper[0]

    def legs (self):
        middle = "wiper, the leg on its own" if self.across else "wiper, middle leg"
        return list (zip (("left leg", middle, "right leg"), self.holes))

    # The body's box, and where each leg meets it.
    def body (self, bench):
        points = [bench.hole_xy (hole) for hole in self.holes]
        if self.across:
            cx = points[1][0]
            cy = (points[0][1] + points[1][1]) / 2
            half = 17
            box = (cx - half, cy - half, cx + half, cy + half)
            ends = [(x, cy - half if y < cy else cy + half) for x, y in points]
            return box, ends
        cx = sum (x for x, _ in points) / 3
        y = min (y for _, y in points)
        half = max (21, (points[2][0] - points[0][0]) / 2 + 6)
        box = (cx - half, y - 2 * half - 4, cx + half, y - 4)
        return box, [(x, y - 5) for x, _ in points]

    def footprint (self, bench):
        return [("rect",) + self.body (bench)[0]]

    def shapes (self, bench):
        box, ends = self.body (bench)
        return [("rect",) + box] + [lead_shape (bench.hole_xy (hole), end)
                                    for hole, end in zip (self.holes, ends)]

    def labels (self, bench):
        box, _ = self.body (bench)
        return [Label (self.value, spots_round (box, self.value, bench.label_size),
                       ((box[0] + box[2]) / 2, box[1]))]

    def draw (self, pencil, bench):
        (x0, y0, x1, y1), ends = self.body (bench)
        for hole, end in zip (self.holes, ends):
            pencil.lead (bench.hole_xy (hole), end)
        side = x1 - x0
        pencil.tint (rounded (x0, y0, side, side, 3), "#7fa6d9")
        pencil.rect (x0, y0, side, side, width=1.0, radius=3, layer="top")
        kx, ky, kr = (x0 + x1) / 2, (y0 + y1) / 2, side * 0.36
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

    # Its center, the box its body fills, and where each leg meets it.
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
    # A 1% resistor's five bands: three digits, a multiplier (gold for
    # tenths), and brown for 1%. "220 Ω" is red, red, black, black, brown.
    names = ["black", "brown", "red", "orange", "yellow", "green", "blue", "violet", "grey",
             "white"]
    match = re.fullmatch (r"([\d.]+)\s*(k|M)?\s*Ω", value)
    ohms = float (match.group (1)) * {None: 1, "k": 1e3, "M": 1e6}[match.group (2)]
    if 10 <= ohms < 100:
        digits = f"{ohms * 10:.0f}"
        multiplier = "gold"
    else:
        digits = f"{ohms:.0f}"
        if len (digits) < 3:
            raise ValueError (f"{value}: five-band resistors below 10 Ω are unsupported")
        multiplier = names[len (digits) - 3]
    return [names[int (digits[0])], names[int (digits[1])], names[int (digits[2])],
            multiplier, "brown"]
