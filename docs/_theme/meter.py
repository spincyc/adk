"""A handheld multimeter in the pencil's manner: set to DC volts, its
reading on the display, and its red and black leads ending in probes that
touch the exact holes being measured.

The meter is drawn in its own frame, 70 by 112 drawing units, and set down
at SCALE. Like the common DT830 its jacks stand in a column at the right of
its face, 10A at the top, then V, then COM; it lies below and to the left of
what it measures, so its leads leave to the right and the one from the
lower jack stays outside the other.
"""

import math
import re

from modules import rounded
from pencil import GRAPHITE

WIDTH, HEIGHT = 70, 112
SCALE = 0.72
HOLSTER = "#e7cf7d"
FACE    = "#3c4046"
SCREEN  = "#c9d2b6"
LEADS   = {"red": "#be4c44", "black": "#3a3a3a"}
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
        pencil.spot (jx, jy, 1.8, LEADS[key] if key in LEADS else "#6a6a6a")
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
    # right, so two probes on neighbouring holes stand apart.
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
    pencil.wire ([], LEADS[color], width=2.2, layer="top", path=path)
    # The handle in the lead's color, and a black guard ring at its tip end.
    guard = ((shoulder[0] - ux, shoulder[1] - uy), (shoulder[0] + ux, shoulder[1] + uy))
    for half, (a, b), tint in ((2.6, (shoulder, handle), LEADS[color]), (3.4, guard, "#222222")):
        points = band (a, b, half)
        pencil.solid (points, tint)
        pencil.polyline (points, width=0.6, closed=True, layer="top", passes=1)
    pencil.line (shoulder, tip, width=1.4, tone=0.55, layer="top", passes=1, wobble=0)
    pencil.layers["top"].append (
        f'<path d="M {shoulder[0]:.1f} {shoulder[1]:.1f} L {tip[0]:.1f} {tip[1]:.1f}" '
        f'stroke="#e4e2dc" stroke-width="0.6"/>')
    pencil.layers["top"].append (
        f'<circle cx="{tx:.1f}" cy="{ty:.1f}" r="1.3" fill="{GRAPHITE}" fill-opacity="0.8"/>')
