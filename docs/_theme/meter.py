"""A handheld multimeter in the pencil's manner: set to DC volts, its
reading on the display, and its red and black leads ending in probes that
touch the exact holes being measured.

The meter is drawn in its own frame, 70 by 112 drawing units, and set down
at SCALE. Like the common DT830 its jacks stand in a column at the right of
its face, 10A at the top, then V, then COM; it lies below and to the left of
what it measures, so its leads leave to the right and the one from the
lower jack stays outside the other.

An oscilloscope's probe is drawn here too (probe_svg): a slim probe with a
hook tip on the point it watches, a band in its channel's color, and a
short ground lead to a clip on the ground point, for each trace a circuit
records with bench.probe ().
"""

import math
import re

from modules import rounded
from pencil import GRAPHITE, WIRES, Pencil

WIDTH, HEIGHT = 70, 112
SCALE = 0.72
HOLSTER = "#e7cf7d"
FACE    = "#3c4046"
SCREEN  = "#c9d2b6"
JACKS   = {"10A": 60, "red": 77, "black": 94}      # down the right of the face
JACK_X  = 57


# What the display shows for an expected reading: its first number, to two
# places, or dashes.
def reading (expect):
    match = re.search (r"-?\d+(?:\.\d+)?", expect or "")
    return f"{float (match.group ()):.2f}" if match else "-.--"


def jack (x, y, color):
    return x + (JACK_X + 4) * SCALE, y + JACKS[color] * SCALE


def draw (pencil, x, y, expect):
    pencil.begin (x, y, 0, SCALE)
    frame (pencil, 0, 0, expect)
    pencil.end ()


def frame (pencil, x, y, expect):
    body = rounded (x, y, WIDTH, HEIGHT, 9)
    pencil.solid (body, HOLSTER)
    pencil.tint (body, HOLSTER)
    pencil.rect (x, y, WIDTH, HEIGHT, width=1.0, radius=9, layer="top")
    face = (x + 5, y + 13, WIDTH - 10, HEIGHT - 18)
    pencil.solid (rounded (*face, 5), FACE)
    pencil.rect (*face, width=0.8, radius=5, layer="top", passes=1)
    # The jacks: 10A, left empty, V for the red lead and COM for the black.
    for key, words in (("10A", "10A"), ("red", "VΩ"), ("black", "COM")):
        jx, jy = x + JACK_X, y + JACKS[key]
        pencil.spot (jx, jy, 4.2, "#1c1c1c")
        pencil.circle (jx, jy, 4.2, width=0.8, layer="top", passes=1)
        pencil.spot (jx, jy, 1.8, WIRES[key] if key in WIRES else "#6a6a6a")
        pencil.text (jx, jy - 6.5, words, size=3.8, kind="silk", color="#f1eee6",
                     tone=0.55 if key == "10A" else 0.9)
    # The display: the reading, and V with the DC mark.
    sx, sy, sw, sh = x + 11, y + 21, WIDTH - 22, 22
    pencil.solid (rounded (sx, sy, sw, sh, 2), SCREEN)
    pencil.rect (sx, sy, sw, sh, width=0.7, radius=2, layer="top", passes=1)
    pencil.text (sx + sw - 13, sy + 16.5, reading (expect), size=12.5, kind="mono", anchor="end",
                 color="#262a22", tone=0.95)
    pencil.text (sx + sw - 7, sy + 11, "V", size=6, kind="silk", color="#262a22", tone=0.95)
    pencil.line ((sx + sw - 10, sy + 14.5), (sx + sw - 4, sy + 14.5), width=0.6, tone=0.9,
                 layer="top", passes=1, wobble=0)
    # The dial, its pointer on DC volts, 20 V range.
    cx, cy, r = x + 28, y + 78, 13
    pencil.spot (cx, cy, r, "#565b62")
    pencil.circle (cx, cy, r, width=0.9, layer="top", passes=1)
    marks = (("OFF", -90), ("V⎓", -150), ("V~", -30), ("Ω", 30), ("A", 150))
    for words, angle in marks:
        a = math.radians (angle)
        pencil.text (cx + (r + 6) * math.cos (a), cy + (r + 6) * math.sin (a) + 2, words, size=4.2,
                     kind="silk", color="#f1eee6", tone=0.9 if words == "V⎓" else 0.55)
    a = math.radians (-150)
    pencil.line ((cx, cy), (cx + (r - 3) * math.cos (a), cy + (r - 3) * math.sin (a)), width=2.4,
                 tone=0.0, layer="top", passes=1, wobble=0)
    pencil.layers["top"].append (
        f'<path d="M {cx:.1f} {cy:.1f} L {cx + (r - 3) * math.cos (a):.1f} '
        f'{cy + (r - 3) * math.sin (a):.1f}" stroke="#f1eee6" stroke-width="1.6" '
        f'stroke-linecap="round"/>')
    pencil.text (cx, y + HEIGHT - 5, "20 V", size=4.4, kind="silk", color="#f1eee6", tone=0.7)


# A strip half wide either side of the line from a to b.
def band (a, b, half):
    length = max (1e-9, math.dist (a, b))
    nx, ny = -(b[1] - a[1]) / length * half, (b[0] - a[0]) / length * half
    return [(a[0] + nx, a[1] + ny), (b[0] + nx, b[1] + ny), (b[0] - nx, b[1] - ny),
            (a[0] - nx, a[1] - ny)]


# A lead from the meter's jack to a probe touching point, the probe lying
# along the lead's last stretch.
def lead (pencil, start, touch, color, side):
    (sx, sy), (tx, ty) = start, touch
    # The probe: a slim handle, a guard, and a metal tip ending at the hole,
    # leaning in from below: the red from the left and the black from the
    # right, so two probes on neighboring holes stand apart.
    angle = math.radians (-90 + 30 * side * -1)
    ux, uy = math.cos (angle), math.sin (angle)
    tip = (tx - ux * 1.5, ty - uy * 1.5)
    shoulder = (tip[0] - ux * 7, tip[1] - uy * 7)
    handle = (shoulder[0] - ux * 20, shoulder[1] - uy * 20)
    end = (handle[0] - ux * 5, handle[1] - uy * 5)
    # One smooth sweep: out of the jack to the right, and into the probe
    # along it.
    # The lower jack's lead turns wider, so it stays outside the other.
    k = max (12.0, math.dist ((sx, sy), end) * 0.45)
    turn = 10 if side < 0 else 22
    path = (f"M {sx:.1f} {sy:.1f} C {sx + turn:.1f} {sy:.1f} {end[0] - ux * k:.1f} "
            f"{end[1] - uy * k:.1f} {end[0]:.1f} {end[1]:.1f}")
    pencil.wire ([], WIRES[color], width=2.2, layer="top", path=path)
    # The handle in the lead's color, and a black guard ring at its tip end.
    guard = ((shoulder[0] - ux, shoulder[1] - uy), (shoulder[0] + ux, shoulder[1] + uy))
    for half, (a, b), tint in ((2.6, (shoulder, handle), WIRES[color]), (3.4, guard, "#222222")):
        points = band (a, b, half)
        pencil.solid (points, tint)
        pencil.polyline (points, width=0.6, closed=True, layer="top", passes=1)
    pencil.line (shoulder, tip, width=1.4, tone=0.55, layer="top", passes=1, wobble=0)
    pencil.layers["top"].append (
        f'<path d="M {shoulder[0]:.1f} {shoulder[1]:.1f} L {tip[0]:.1f} {tip[1]:.1f}" '
        f'stroke="#e4e2dc" stroke-width="0.6"/>')
    pencil.layers["top"].append (
        f'<circle cx="{tx:.1f}" cy="{ty:.1f}" r="1.3" fill="{GRAPHITE}" fill-opacity="0.8"/>')


# An oscilloscope probe --------------------------------------------------

# Each channel's band, as most scopes color their inputs: yellow, then cyan.
CHANNELS = {1: "#d9b43c", 2: "#3fa7c4"}
PROBE   = "#3c3f44"


# A scope probe touching point with its hook tip, leaning in from below at
# its side (-1 from the left, 1 from the right), its cable running down to
# bottom; returns where its ground lead leaves it.
def scope_probe (pencil, touch, channel, side, bottom):
    tx, ty = touch
    angle = math.radians (-90 + 32 * side * -1)
    ux, uy = math.cos (angle), math.sin (angle)
    at = lambda back, across=0.0: (tx - ux * back - uy * across, ty - uy * back + ux * across)
    # The cable, from the probe's back end down out of the view.
    end = at (62)
    path = (f"M {end[0]:.1f} {end[1]:.1f} C {end[0] - ux * 30:.1f} {end[1] - uy * 30:.1f} "
            f"{end[0] - side * 10:.1f} {bottom - 30:.1f} {end[0] - side * 14:.1f} {bottom + 4:.1f}")
    pencil.wire ([], "#2d2f33", width=3.2, layer="top", path=path)
    # The body, its channel's band, the ground collar and the nose.
    for (front, back), half, tint in (((22, 62), 4.6, PROBE), ((48, 54), 4.8, CHANNELS[channel]),
                                      ((17, 22), 3.6, "#c9c9c4"), ((7, 17), 2.6, PROBE)):
        points = band (at (front), at (back), half)
        pencil.solid (points, tint)
        pencil.polyline (points, width=0.6, closed=True, layer="top", passes=1)
    # The hook tip, from the nose to the hole, curling round it.
    pencil.line (at (7), at (1.5), width=1.2, tone=0.55, layer="top", passes=1, wobble=0)
    pencil.layers["top"].append (
        f'<path d="M {at (1.5)[0]:.1f} {at (1.5)[1]:.1f} Q {tx + uy * 3:.1f} {ty - ux * 3:.1f} '
        f'{tx:.1f} {ty:.1f}" fill="none" stroke="#c9c9c4" stroke-width="1.1"/>'
        f'<circle cx="{tx:.1f}" cy="{ty:.1f}" r="1.3" fill="{GRAPHITE}" fill-opacity="0.8"/>')
    # Its channel, beside the band.
    x, y = at (51, side * 13)
    pencil.text (x, y + 2.5, f"CH{channel}", size=6, kind="silk", weight="bold", tone=0.85)
    return at (19)


# The probe's short ground lead from its collar to a crocodile clip, its
# jaws on the ground point.
def ground_clip (pencil, start, touch):
    (sx, sy), (tx, ty) = start, touch
    length = max (1e-9, math.dist (start, touch))
    ux, uy = (tx - sx) / length, (ty - sy) / length
    jaws = (tx - ux * 3, ty - uy * 3)
    back = (tx - ux * 15, ty - uy * 15)
    middle = ((sx + back[0]) / 2 + uy * 14, (sy + back[1]) / 2 - ux * 14)
    path = (f"M {sx:.1f} {sy:.1f} Q {middle[0]:.1f} {middle[1]:.1f} {back[0]:.1f} {back[1]:.1f}")
    pencil.wire ([], WIRES["black"], width=2.0, layer="top", path=path)
    # The clip: a black sleeve, then its two steel jaws, open on the point.
    sleeve = band (back, (tx - ux * 7, ty - uy * 7), 3.2)
    pencil.solid (sleeve, "#222222")
    pencil.polyline (sleeve, width=0.6, closed=True, layer="top", passes=1)
    for spread in (-1, 1):
        tip = (jaws[0] - uy * spread * 1.8, jaws[1] + ux * spread * 1.8)
        pencil.line ((tx - ux * 7 - uy * spread * 1.6, ty - uy * 7 + ux * spread * 1.6), tip,
                     width=1.3, tone=0.6, layer="top", passes=1, wobble=0)
    pencil.layers["top"].append (
        f'<circle cx="{tx:.1f}" cy="{ty:.1f}" r="1.3" fill="{GRAPHITE}" fill-opacity="0.8"/>')


# A trace to watch (bench.probe): the breadboard round its two points, the
# probe leaning in on its tip and its ground lead clipped to the ground
# point, drawn on the bench as drawing's Drawing draws it, and labeled
# only by the page's caption and table.
def probe_svg (drawing, index, prefix="probe"):
    bench = drawing.bench
    routes = drawing._layout ()
    taken = bench.scope_probes[index]
    (tip, _), (ground, _) = bench.probe_points (index)
    points = {"tip": bench.hole_xy (tip), "ground": bench.hole_xy (ground)}
    side = -1 if points["ground"][0] >= points["tip"][0] else 1
    xs = [x for x, _ in points.values ()]
    ys = [y for _, y in points.values ()]
    _, by0, _, _ = bench.board_box ()
    left, right = min (xs) - 100, max (xs) + 80
    top = min (ys) - 30
    bottom = max (ys) + 80
    for part in bench.parts:
        for shape in part.footprint (bench):
            if shape[0] == "rect":
                x0, y0, x1, y1 = shape[1:5]
            else:
                cx, cy, r = shape[1:4]
                x0, y0, x1, y1 = cx - r, cy - r, cx + r, cy + r
            if x0 < right and x1 > left and y0 < top < y1:
                top = max (by0 - 4, min (top, y0 - 8))
    bench.label_size = 6.4
    detail = (drawing._column_at (left), drawing._column_at (right))
    pencil = Pencil (bench.seed, f"{prefix}{index}")
    drawing._draw_mega (pencil)
    drawing._draw_board (pencil, detail)
    for part in bench.parts:
        part.draw (pencil, bench)
    for module in bench.modules.values ():
        if not module.kind.blocks:
            module.draw (pencil)
    for start, end, route in routes:
        drawing._draw_wire (pencil, start, end, route)
    for module in bench.modules.values ():
        if module.kind.blocks:
            module.draw (pencil)
    collar = scope_probe (pencil, points["tip"], taken["channel"], side, bottom)
    ground_clip (pencil, collar, points["ground"])
    return pencil.svg ((left, top, right - left, bottom - top), f"{bench.title}: {taken['label']}")
