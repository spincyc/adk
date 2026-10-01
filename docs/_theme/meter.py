"""A handheld multimeter in the pencil's manner: set to DC volts, its
reading on the display, and its red and black leads ending in probes that
touch the exact holes being measured.

The meter is drawn in its own frame, 70 by 112 drawing units, and set down
at SCALE. Like the common DT830 its jacks stand in a column at the right of
its face, 10A at the top, then V, then COM; it lies below and to the left of
what it measures, so its leads leave to the right. Its two probes lean in
apart, each from its own side, whichever way hides less (probe_sides).

An oscilloscope's probe is drawn here too (probe_svg): a slim probe with a
hook tip on the point it watches, a band in its channel's color, and a
short ground lead to a clip on the ground point, for each trace a circuit
records with bench.probe ().
"""

import math
import re

from modules import rounded
from parts import back_to_front
from pencil import GRAPHITE, WIRES, Pencil
from route import HARD, Placer, bounds_of, distance_to, text_width

WIDTH, HEIGHT = 70, 112
SCALE = 0.72
HOLSTER = "#e7cf7d"
FACE    = "#3c4046"
SCREEN  = "#c9d2b6"
JACKS   = {"10A": 60, "red": 77, "black": 94}      # down the right of the face
JACK_X  = 57
LEAN    = 30                        # a meter probe's tilt from upright, in degrees
PROBE_LENGTH = 34                   # from its tip to where its lead leaves it


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
    # leaning in from below, from the left (side -1) or the right (1).
    angle = math.radians (-90 + LEAN * side * -1)
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


# How a measurement's two probes lean in, {"red": side, "black": side}, each
# -1 from the left or 1 from the right: apart, so they never cross over
# what they measure, the red from the left and the black from the right
# unless the other way round hides less of the build, as when the red's
# point is the right-hand one.
def probe_sides (bench, routes, points):
    def cost (red, black):
        bodies = [lie (points[color], side, LEAN)
                  for color, side in (("red", red), ("black", black))]
        crossed = segments_cross (bodies[0] (1.5), bodies[0] (PROBE_LENGTH),
                                  bodies[1] (1.5), bodies[1] (PROBE_LENGTH))
        return (1000 * crossed + hides (bench, routes, points["red"], red, LEAN, PROBE_LENGTH)
                + hides (bench, routes, points["black"], black, LEAN, PROBE_LENGTH))

    red, black = min (((-1, 1), (1, -1)), key=lambda sides: cost (*sides))
    return {"red": red, "black": black}


# An oscilloscope probe --------------------------------------------------

# Each channel's band, as most scopes color their inputs: yellow, then cyan.
CHANNELS = {1: "#d9b43c", 2: "#3fa7c4"}
PROBE   = "#3c3f44"
LENGTH  = 62                        # from the tip to the probe's back end
TILTS   = (32, 22, 44)              # how far from upright it may lean, best first


# A probe leaning in on touch from below: side -1 from the left, 1 from
# the right, tilt degrees from upright. Returns a function giving a point
# back along it and across it.
def lie (touch, side, tilt):
    tx, ty = touch
    angle = math.radians (-90 - tilt * side)
    ux, uy = math.cos (angle), math.sin (angle)
    return lambda back, across=0.0: (tx - ux * back - uy * across, ty - uy * back + ux * across)


# What lying a probe there would hide, as a cost: most for a part's body or
# a module, then a wire, a used hole, and a little for leaning further.
def hides (bench, routes, touch, side, tilt, length=LENGTH):
    at = lie (touch, side, tilt)
    body = ("segment", *at (7), *at (length + 20), 5.5)
    cost = 3 * TILTS.index (tilt) if tilt in TILTS else 0
    for part in bench.parts:
        cost += 100 * any (shape_meets (body, shape) for shape in part.shapes (bench)[:1])
        cost += 10 * any (shape_meets (body, shape) for shape in part.shapes (bench)[1:])
    for placed in bench.modules.values ():
        cost += 100 * shape_meets (body, ("rect", *placed.box ()))
    for _, _, points in routes:
        cost += 8 * any (segments_cross (a, b, at (7), at (length + 20))
                         for a, b in zip (points, points[1:]))
    for hole in bench.used:
        cost += 4 * (distance_to (body, bench.hole_xy (hole)) < 1)
    return cost


# Whether the segment from a to b crosses the one from c to d.
def segments_cross (a, b, c, d):
    def turn (p, q, r):
        return (q[0] - p[0]) * (r[1] - p[1]) - (q[1] - p[1]) * (r[0] - p[0])
    return turn (a, b, c) * turn (a, b, d) < 0 and turn (c, d, a) * turn (c, d, b) < 0


# Whether a shape comes within a body's half width of its middle line,
# a body being a ("segment", ...) shape.
def shape_meets (body, shape):
    _, ax, ay, bx, by, half = body
    steps = max (2, int (math.dist ((ax, ay), (bx, by)) / 2))
    return any (distance_to (shape, (ax + (bx - ax) * k / steps, ay + (by - ay) * k / steps)) < half
                for k in range (steps + 1))


# A scope probe lying as lie () has it, its hook tip on touch and its cable
# running down to bottom; returns where its ground lead leaves it, and its
# band's middle.
def scope_probe (pencil, touch, channel, side, tilt, bottom):
    tx, ty = touch
    at = lie (touch, side, tilt)
    ux, uy = (at (0)[0] - at (1)[0]), (at (0)[1] - at (1)[1])
    # The cable, from the probe's back end down out of the view.
    end = at (LENGTH)
    path = (f"M {end[0]:.1f} {end[1]:.1f} C {end[0] - ux * 30:.1f} {end[1] - uy * 30:.1f} "
            f"{end[0] - side * 10:.1f} {bottom - 30:.1f} {end[0] - side * 14:.1f} {bottom + 4:.1f}")
    pencil.wire ([], "#2d2f33", width=3.2, layer="top", path=path)
    # The body, its channel's band, the ground collar and the nose.
    for (front, back), half, tint in (((22, LENGTH), 4.6, PROBE),
                                      ((48, 54), 4.8, CHANNELS[channel]),
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
    return at (19), at (51)


# The ground lead's curve from the probe's collar to the clip, bowed to
# whichever side, and by however much, crosses fewest parts' bodies:
# (start, middle, end of the lead, the clip's sleeve end).
def ground_way (bench, start, touch):
    (sx, sy), (tx, ty) = start, touch
    length = max (1e-9, math.dist (start, touch))
    ux, uy = (tx - sx) / length, (ty - sy) / length
    back = (tx - ux * 15, ty - uy * 15)
    bodies = [part.shapes (bench)[0] for part in bench.parts if part.shapes (bench)]

    def way (bow):
        middle = ((sx + back[0]) / 2 + uy * bow, (sy + back[1]) / 2 - ux * bow)
        curve = [((1 - t) ** 2 * sx + 2 * (1 - t) * t * middle[0] + t * t * back[0],
                  (1 - t) ** 2 * sy + 2 * (1 - t) * t * middle[1] + t * t * back[1])
                 for t in (k / 16 for k in range (17))]
        crossed = sum (any (distance_to (body, point) < 2.5 for point in curve) for body in bodies)
        return crossed * 100 + abs (bow), middle

    _, middle = min ((way (bow) for bow in (14, -14, 8, -8, 24, -24, 36, -36)),
                     key=lambda found: found[0])
    return middle, back, (ux, uy)


# The probe's short ground lead from its collar to a crocodile clip, its
# jaws on the ground point, round what it can.
def ground_clip (pencil, bench, start, touch):
    (sx, sy), (tx, ty) = start, touch
    middle, back, (ux, uy) = ground_way (bench, start, touch)
    jaws = (tx - ux * 3, ty - uy * 3)
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
    curve = [((1 - t) ** 2 * sx + 2 * (1 - t) * t * middle[0] + t * t * back[0],
              (1 - t) ** 2 * sy + 2 * (1 - t) * t * middle[1] + t * t * back[1])
             for t in (k / 8 for k in range (9))]
    return curve + [touch]


# The view's frame: the tip and the ground point, the probe as far as its
# band, the part the tip probes and any part beside the tip, whole, and
# room for the probe's label, within the board's height above.
def framing (bench, hole, touches, at):
    tip = touches[0]
    boxes = [(x - 30, y - 30, x + 30, y + 20) for x, y in touches]
    probe = [at (0), at (LENGTH)]
    boxes.append ((min (x for x, _ in probe) - 12, min (y for _, y in probe) - 12,
                   max (x for x, _ in probe) + 12, max (y for _, y in probe) + 12))
    strip = bench.strip_of (hole)
    for part in bench.parts:
        legs = [hole for _, hole in part.legs ()]
        shapes = part.shapes (bench)
        if not shapes:
            continue
        near = any (distance_to (shape, tip) < 22 for shape in shapes)
        probed = any (bench.strip_of (leg) == strip for leg in legs)
        if near or probed:
            for shape in shapes:
                x0, y0, x1, y1 = bounds_of (shape)
                boxes.append ((x0 - 8, y0 - 8, x1 + 8, y1 + 8))
    left = min (box[0] for box in boxes)
    right = max (box[2] for box in boxes)
    _, by0, _, _ = bench.board_box ()
    top = max (by0 - 12, min (box[1] for box in boxes))
    # No narrower than the meter's views, so the board keeps one scale.
    if right - left < 190:
        middle = (left + right) / 2
        left, right = middle - 95, middle + 95
    return left, top, right


# A trace to watch (bench.probe): the breadboard round its two points, the
# probe leaning in on its tip from the side where it hides least, its
# ground lead clipped to the ground point, and its channel named beside
# its band where that covers nothing. Everything else is left to the
# page's caption and table. drawing is the bench's Drawing, which draws
# the board, parts and wires under it as its other views do.
def probe_svg (drawing, index, prefix="probe"):
    bench = drawing.bench
    routes = drawing._layout ()
    taken = bench.scope_probes[index]
    (tip, _), (ground, _) = bench.probe_points (index)
    touches = (bench.hole_xy (tip), bench.hole_xy (ground))
    sides = {"left": (-1,), "right": (1,), None: (-1, 1)}[taken.get ("side")]
    # The ground on the far side breaks a tie: its lead then runs clear of the probe.
    toward = 1 if touches[1][0] < touches[0][0] else -1
    side, tilt = min (((side, tilt) for side in sides for tilt in TILTS),
                      key=lambda lying: (hides (bench, routes, touches[0], *lying),
                                         lying[0] != toward))
    at = lie (touches[0], side, tilt)
    left, top, right = framing (bench, tip, touches, at)
    # The top edge takes in whole any part it would cut through.
    _, by0, _, _ = bench.board_box ()
    for part in bench.parts:
        for shape in part.footprint (bench) + part.shapes (bench)[:1]:
            x0, y0, x1, y1 = bounds_of (shape)
            if x0 < right and x1 > left and y0 < top < y1:
                top = max (by0 - 12, min (top, y0 - 8))
    bottom = max (max (y for _, y in touches), at (LENGTH)[1]) + 30
    bench.label_size = 6.4
    detail = (drawing._column_at (left), drawing._column_at (right))
    pencil = Pencil (bench.seed, f"{prefix}{index}")
    drawing._draw_mega (pencil)
    drawing._draw_board (pencil, detail)
    for part in back_to_front (bench.parts, bench):
        part.draw (pencil, bench)
    for module in bench.modules.values ():
        if not module.kind.blocks:
            module.draw (pencil)
    for start, end, route in routes:
        drawing._draw_wire (pencil, start, end, route)
    for module in bench.modules.values ():
        if module.kind.blocks:
            module.draw (pencil)
    collar, middle = scope_probe (pencil, touches[0], taken["channel"], side, tilt, bottom)
    lead = ground_clip (pencil, bench, collar, touches[1])
    view = (left, top, right, bottom)
    channel (pencil, drawing, routes, view, detail, f"CH{taken['channel']}", at, side, lead)
    return pencil.svg ((left, top, right - left, bottom - top), f"{bench.title}: {taken['label']}")


# The probe's channel, named where it covers nothing: beside its band on
# either side, or a little further along it, inside the view, or failing
# that further out on a leader to the band.
def channel (pencil, drawing, routes, view, detail, text, at, side, lead):
    bench, size = drawing.bench, 7
    placer = Placer (view)
    for part in bench.parts:
        for shape in part.shapes (bench):
            placer.avoid (shape)
    for placed in bench.modules.values ():
        placer.avoid (("rect", *placed.reach_box ()))
    for _, _, points in routes:
        for a, b in zip (points, points[1:]):
            placer.avoid (("segment", *a, *b, 2.4))
    for hole in bench.used:
        placer.avoid (("circle", *bench.hole_xy (hole), 3.4))
    for x, y, w in drawing._print_marks (detail):
        placer.avoid (("rect", x - w / 2, y - 6, x + w / 2, y), 150)
    for x, y, w in drawing._board_signs ():
        placer.taken ((x - w / 2, y - 7, x + w / 2, y + 1))
    x0, y0, x1, y1 = bench.board_box ()
    for edge in ((x0, y0, x1, y0), (x0, y1, x1, y1), (x0, y0, x0, y1), (x1, y0, x1, y1)):
        placer.avoid (("segment", *edge, 0.6), 60)
    placer.avoid (("segment", *at (0), *at (LENGTH + 40), 6.5))
    for a, b in zip (lead, lead[1:]):
        placer.avoid (("segment", *a, *b, 2))
    spots = []
    for along in (51, 40, 30, 60):
        for across in (14, 20, 28):
            for way in (-side, side):
                x, y = at (along, way * across)
                spots.append ((x, y + size * 0.35, "middle",
                               abs (along - 51) + across + (5 if way == side else 0)))
    best = placer.place (text, size, spots)
    point = at (51)
    if best[0] >= HARD:
        far = [(point[0] + dx, point[1] + dy, "middle", 40 + math.hypot (dx, dy), point)
               for dx in range (-80, 81, 10) for dy in range (-60, 41, 10)]
        best = placer.place (text, size, far, own=[("segment", *at (0), *at (LENGTH + 40), 6.5)])
    _, x, y, anchor, box = best
    to = point if math.dist (((box[0] + box[2]) / 2, (box[1] + box[3]) / 2), point) > 32 else None
    pencil.label (x, y, text, size=size, anchor=anchor, to=to, width=text_width (text, size) - 1)
