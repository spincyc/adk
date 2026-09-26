"""The pencil drawings of a bench: the whole bench, a close-up of the
breadboard, and each measurement with its meter, all from the circuit a
lesson's circuit.py describes (bench.py).

    drawing = Drawing (bench)
    drawing.svg ("bench")           the Mega, the breadboard, every part and wire
    drawing.svg ("closeup")         the breadboard round the parts, larger
    drawing.measure_svg (0)         the first measurement's probes and meter

Every wire is routed once for all of them (route.py): round parts, modules
and labels, the jumpers that can lie straight first and the rest in rounds
of negotiation until no two share a stretch of grid. Labels go where they
cover nothing they shouldn't, and a label with no room is an error. Routes
are kept in the folder ADK_ROUTES names, the Makefile's build/routes.
"""

import hashlib
import json
import math
import os
import shutil

import meter

from bench import (BOARD_HEIGHT, MARGIN, MEGA_HEIGHT, MEGA_PINS, MEGA_WIDTH, ROWS, canonical,
                   numbered, parse_hole, rail_column)
from modules import HOUSING
from parts import HeaderModule, Label, Led, Resistor, spots_round
from pencil import DPI, WIRES, Pencil
from route import (HARD, Placer, Router, bounds_of, corners, direction, node, segment_distance,
                   segment_meets_box, text_box, text_width)

MEGA_TINT = "#dfe6e8"
CLOSEUP_COLUMNS = 16                # the narrowest close-up, so parts keep one scale
ROUNDS = 6                          # of negotiation between the wires

# Directions a wire may leave a header in: the way its pins point, or a
# little either side, to fan out from its neighbors.
LEAVING = {"top": (6, 5, 7), "bottom": (2, 1, 3), "double": (0, 7, 1)}

# Where routes are kept, if anywhere, and this version of the engine: a
# digest of the theme's Python.
ROUTES = os.environ.get ("ADK_ROUTES")
THEME = os.path.dirname (os.path.abspath (__file__))
ENGINE = hashlib.sha256 (b"".join (open (os.path.join (THEME, name), "rb").read ()
                                   for name in sorted (os.listdir (THEME))
                                   if name.endswith (".py"))).hexdigest ()[:16]


class Drawing:
    """A finished bench's drawings, which share one routing of its wires."""

    def __init__ (self, bench):
        self.bench = bench
        self._routes = None
        self._placed_boxes = []

    # Routing -------------------------------------------------------------

    # Every wire's path, worked out once for both drawings. Jumpers that lie
    # straight are fixed first; the rest are routed in rounds, the nearer
    # ends first, each round round the others' last, until no two share any
    # stretch of grid. Failing that, they are routed strictly, one at a time.
    def _layout (self):
        bench = self.bench
        if self._routes is not None:
            return self._routes
        bench.label_size = 10
        plans = [self._plan (*wire) for wire in bench.wires]
        paths = self._recall (plans)
        if paths is None:
            paths = self._negotiate_all (plans)
            self._remember (plans, paths)
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

    # Routes worked out before are kept in the folder the environment's
    # ADK_ROUTES names (the Makefile's build/routes), by everything the
    # router is given, so a rebuild only routes what has changed. Each
    # version of the drawing engine keeps its own folder, and the first
    # routes it keeps clear the other versions' away.
    def _fingerprint (self, plans):
        bench = self.bench
        text = repr ((bench.first, bench.last, bench.gap, sorted (bench.used), sorted (bench.taken),
                      [(p["points"], p["first"], sorted (p["allow"]), p["keep_off"], p["straight"])
                       for p in plans],
                      [(type (part).__name__, part.legs (), part.shapes (bench), part.blocks,
                        [(label.text, label.spots[0]) for label in part.labels (bench)[:1]])
                       for part in bench.parts],
                      [(m.reach_box (), m.title_box ()) for m in bench.modules.values ()]))
        return hashlib.sha256 (text.encode ()).hexdigest ()

    def _recall (self, plans):
        if not ROUTES:
            return None
        try:
            with open (os.path.join (ROUTES, ENGINE, self._fingerprint (plans) + ".json")) as file:
                kept = json.load (file)
        except (OSError, ValueError):
            return None
        return {int (index): [tuple (n) for n in path] for index, path in kept.items ()}

    def _remember (self, plans, paths):
        if not ROUTES:
            return
        folder = os.path.join (ROUTES, ENGINE)
        if not os.path.isdir (folder):
            for old in set (os.listdir (ROUTES) if os.path.isdir (ROUTES) else ()) - {ENGINE}:
                shutil.rmtree (os.path.join (ROUTES, old), ignore_errors=True)
            os.makedirs (folder, exist_ok=True)
        with open (os.path.join (folder, self._fingerprint (plans) + ".json"), "w") as file:
            json.dump ({index: path for index, path in paths.items ()}, file)

    # How a wire is to be routed: its ends in routing order, where each is
    # drawn, the grid points it passes, and its path when it lies straight.
    def _plan (self, start, end, color, via):
        bench = self.bench
        if self._oriented (start, end) != (start, end):
            start, end, via = end, start, list (reversed (via))
        a, first, allow_a = self._terminal (start)
        b, _, allow_b = self._terminal (end)
        # Into a module's pin straight along the way the pin points.
        approach = []
        if end[0] == "module":
            (x, y), out = bench.module_pin (end[1])[0].anchor (bench.module_pin (end[1])[1])
            approach = [node ((b[0] + out[0] * 10, b[1] + out[1] * 10))]
        # Wires run on a grid of 0.05 inch. Two waypoints that fall on one
        # spot of it, or a waypoint that sends the wire back the way it
        # came, would turn it back on itself.
        stops = [node (bench.point_xy (point)) for point in via]
        wire = f"the wire from {bench.describe (start)} to {bench.describe (end)}"
        for index in range (1, len (via)):
            if stops[index] == stops[index - 1] and via[index] != via[index - 1]:
                raise ValueError (f"{wire} passes {via[index - 1]} and {via[index]}, which fall on "
                                  f"one point of the 0.05 inch grid: move one, or leave one out")
        points = [node (a)] + stops + approach + [node (b)]
        for p, q, r in zip (points, points[1:], points[2:]):
            if p != q != r and (p[0] == q[0] == r[0] or p[1] == q[1] == r[1]) and \
                    (direction (p, q) - direction (q, r)) % 8 == 4:
                raise ValueError (f"{wire} turns back on itself at a waypoint: check its via "
                                  f"points")
        plan = dict (start=start, end=end, a=a, b=b, first=first, allow=allow_a | allow_b,
                     points=points,
                     keep_off=bench.board_box () if "hole" not in (start[0], end[0]) else None,
                     straight=None)
        if start[0] == end[0] == "hole" and not via and self._straight (start[1], end[1]):
            plan["straight"] = straight_nodes (node (a), node (b))
        return plan

    def _negotiate (self, router, plan, index, pressure):
        bench = self.bench
        try:
            path, _ = router.route (plan["points"], plan["first"], plan["allow"], plan["keep_off"],
                                    index, pressure)
        except ValueError as error:
            raise ValueError (f"the wire from {bench.describe (plan['start'])} to "
                              f"{bench.describe (plan['end'])} can't get through ({error}): move "
                              f"what is in its way, or give it via points") from error
        return path

    def _reach (self, index):
        bench = self.bench
        start, end, _, _ = bench.wires[index]
        (x1, y1), (x2, y2) = bench.xy (start), bench.xy (end)
        return abs (x1 - x2) + abs (y1 - y2)

    def _jumper (self, index):
        start, end, _, via = self.bench.wires[index]
        return start[0] == end[0] == "hole" and not via

    def _router (self):
        bench = self.bench
        x0, y0, x1, y1 = self._extent ()
        router = Router ((x0 - 80, y0 - 120, x1 + 80, y1 + 120), bench.board_box ())
        # The Mega's middle is solid; wires only leave its header strips.
        mx0, my0, mx1, my1 = bench.mega_box ()
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
        for box in (bench.mega_box (), bench.board_box ()):
            router.fringe (box, 4, 5)
        for name in MEGA_PINS:
            router.hole (bench.pin_xy (name), True, near=False)
        for hole in bench.holes ():
            router.hole (bench.hole_xy (hole), hole in bench.used)
        # The board's printing, which wires would rather not hide.
        for x, y, w in self._print_marks ():
            router.cost (("rect", x - w / 2, y - 6, x + w / 2, y), 4)
        for x, y, w in self._board_signs ():
            router.cost (("rect", x - w / 2, y - 6, x + w / 2, y), 12, grow=1)
        bx0, by0, bx1, _ = bench.board_box ()
        for offset in (0.06, 0.34, 1.86, 2.14):
            y = by0 + offset * DPI
            router.cost (("rect", bx0 + 14, y - 1, bx1 - 14, y + 1), 3)
        for part in bench.parts:
            for shape in part.shapes (bench):
                if part.blocks:
                    router.block (shape)
                else:
                    router.cost (shape, 5)
            # Leave room where each part's label would best go.
            for label in part.labels (bench)[:1]:
                x, y, anchor, _ = label.spots[0]
                router.cost (("rect", *text_box (x, y, label.text, 10 * label.size, anchor)), 10)
        for placed in bench.modules.values ():
            router.block (("rect", *placed.reach_box ()))
            router.block (("rect", *placed.title_box ()))
        return router

    # Everything drawn but wires and labels, for sizing the grid.
    def _extent (self):
        bench = self.bench
        boxes = [bench.mega_box (), bench.board_box ()]
        boxes += [placed.reach_box () for placed in bench.modules.values ()]
        boxes += [placed.title_box () for placed in bench.modules.values ()]
        boxes += [part.box (bench) for part in bench.parts if part.box (bench)]
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
        bench = self.bench
        kind, name = end
        if kind == "pin":
            return bench.pin_xy (name), LEAVING[MEGA_PINS[name].header], set ()
        if kind == "hole":
            return bench.hole_xy (name), None, set ()
        placed, pin = bench.module_pin (name)
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
        bench = self.bench
        (x1, y1), (x2, y2) = bench.hole_xy (a), bench.hole_xy (b)
        if abs (x1 - x2) > 0.5 and abs (y1 - y2) > 0.5:
            return False
        for hole in bench.holes ():
            if hole in (a, b) or hole not in bench.used:
                continue
            x, y = bench.hole_xy (hole)
            if min (x1, x2) - 0.5 <= x <= max (x1, x2) + 0.5 and \
                    min (y1, y2) - 0.5 <= y <= max (y1, y2) + 0.5:
                return False
        for part in bench.parts:
            if not part.blocks:
                continue
            for shape in part.shapes (bench):
                if shape_crosses (shape, (x1, y1), (x2, y2)):
                    return False
        for placed in bench.modules.values ():
            if segment_meets_box ((x1, y1), (x2, y2), placed.reach_box (), 2):
                return False
        return True

    # Drawing ------------------------------------------------------------

    # The whole bench, or a close-up of the breadboard where the parts are.
    def svg (self, view="bench", prefix="bench"):
        bench = self.bench
        routes = self._layout ()
        pencil = Pencil (bench.seed, prefix)
        detail = self._closeup_columns () if view == "closeup" else None
        # Labels are set smaller in the close-up, which the page shows larger.
        bench.label_size = 6.4 if detail else 10
        rough = self._closeup_box () if detail else None
        placed = self._place_labels (routes, rough, detail)
        box = self._view_box (rough, placed, detail) if detail else self._canvas (routes, placed)
        self._draw_mega (pencil)
        self._draw_board (pencil, detail)
        for part in bench.parts:
            part.draw (pencil, bench)
        for start, end, points in routes:
            self._draw_wire (pencil, start, end, points)
        for module in bench.modules.values ():
            module.draw (pencil)
        for text, x, y, anchor, size, to, kind in placed:
            if kind == "note":
                pencil.text (x, y, text, size=size, anchor=anchor, kind="label", italic=True)
                pencil.arrow (to)
            else:
                width = text_width (text, size) - 1
                pencil.label (x, y, text, size=size, anchor=anchor, to=to, width=width,
                              patch=self._patch (x, y, width, size, anchor))
        return pencil.svg (box, bench.title if view == "bench" else f"{bench.title}: close-up")

    # A measurement: the breadboard round its two probe points, the meter
    # below the board reading what is expected, and its leads rising to the
    # probes. Labels are left to the caption and the table.
    def measure_svg (self, index, prefix="measure"):
        bench = self.bench
        routes = self._layout ()
        taken = bench.measurements[index]
        (red, _), (black, _) = bench.probes (index)
        points = {"red": bench.hole_xy (red), "black": bench.hole_xy (black)}
        xs = [x for x, _ in points.values ()]
        bx0, by0, bx1, by1 = bench.board_box ()
        width, height = meter.WIDTH * meter.SCALE, meter.HEIGHT * meter.SCALE
        # The meter below the board and to the left of the probes, its leads
        # sweeping up and right into them.
        mx, my = min (xs) - width - 50, by1 + 26
        left, right = mx - 8, max (xs) + 36
        # Below a module hanging under the board there, such as the screen,
        # rather than over it.
        hanging = [placed.box () for placed in bench.modules.values ()]
        hanging += [part.box (bench) for part in bench.parts if part.box (bench)]
        for x0, y0, x1, y1 in hanging:
            if x0 < mx + width and x1 > mx and y0 < my + height and y1 > by1:
                my = max (my, y1 + 12)
        top = min (y for _, y in points.values ()) - 30
        # Take in the whole body of any part the top edge would cut through,
        # such as an LED or a buzzer, as far as the top of the board.
        for part in bench.parts:
            for shape in part.footprint (bench):
                if shape[0] == "rect":
                    x0, y0, x1, y1 = shape[1:5]
                else:
                    cx, cy, r = shape[1:4]
                    x0, y0, x1, y1 = cx - r, cy - r, cx + r, cy + r
                if x0 < right and x1 > left and y0 < top < y1:
                    top = max (by0 - 4, min (top, y0 - 8))
        box = (left, top, right - left, my + height + 8 - top)
        bench.label_size = 6.4
        detail = (self._column_at (left), self._column_at (right))
        pencil = Pencil (bench.seed, f"{prefix}{index}")
        self._draw_mega (pencil)
        self._draw_board (pencil, detail)
        for part in bench.parts:
            part.draw (pencil, bench)
        for start, end, route in routes:
            self._draw_wire (pencil, start, end, route)
        for module in bench.modules.values ():
            module.draw (pencil)
        meter.draw (pencil, mx, my, taken["expect"])
        # The lower jack's lead, the black, runs outside the red one.
        for color, side in (("red", -1), ("black", 1)):
            meter.lead (pencil, meter.jack (mx, my, color), points[color], color, side)
        return pencil.svg (box, f"{bench.title}: {taken['label']}")

    # The drawing grows to hold every module, overhanging part, wire and label.
    def _canvas (self, routes, placed):
        bench = self.bench
        width, height = bench.size ()
        x0, y0, x1, y1 = 0, 0, width, height
        pad = MARGIN * DPI * 0.7
        boxes = [placed.reach_box () for placed in bench.modules.values ()]
        boxes += [placed.title_box () for placed in bench.modules.values ()]
        boxes += [part.placed (bench).title_box () for part in bench.parts
                  if isinstance (part, HeaderModule)]
        boxes += [part.box (bench) for part in bench.parts if part.box (bench)]
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

    # The columns the build uses, leaving out the Mega's power feeds and the
    # rail links, which are the same in every lesson.
    def _closeup_columns (self):
        bench = self.bench
        if bench.closeup_range:
            return bench.closeup_range
        standard = {hole for start, end, _, _ in bench.wires if bench.is_standard ((start, end))
                    for kind, hole in (start, end) if kind == "hole"}
        columns = [parse_hole (hole)[1] for hole in bench.used if hole not in standard]
        for part in bench.parts:
            for shape in part.footprint (bench):
                if shape[0] == "rect" and not part.box (bench):
                    columns += [self._column_at (shape[1]), self._column_at (shape[3])]
        if not columns:
            return bench.first, min (bench.last, bench.first + CLOSEUP_COLUMNS - 1)
        low, high = max (bench.first, min (columns) - 3), min (bench.last, max (columns) + 3)
        # At least sixteen columns wide, so parts keep one scale from lesson
        # to lesson.
        while high - low + 1 < CLOSEUP_COLUMNS and (low > bench.first or high < bench.last):
            if high < bench.last:
                high += 1
            if high - low + 1 < CLOSEUP_COLUMNS and low > bench.first:
                low -= 1
        return low, high

    def _column_at (self, x):
        bench = self.bench
        bx, _ = bench.board_origin ()
        column = round ((x / DPI - bx - 0.25) / 0.1) + bench.first
        return min (bench.last, max (bench.first, column))

    # The close-up before its labels: the columns it shows, and the rows the
    # build uses with room for every part standing in them.
    def _closeup_box (self):
        bench = self.bench
        low, high = self._closeup_columns ()
        left, _ = bench.hole_xy (f"a{low}")
        right, _ = bench.hole_xy (f"a{high}")
        _, by = bench.board_origin ()
        ceiling, floor = by * DPI - 40, (by + BOARD_HEIGHT) * DPI + 25
        ys = [bench.hole_xy (hole)[1] for hole in bench.used] or [(by + 1.1) * DPI]
        for part in bench.parts:
            if any (low <= parse_hole (hole)[1] <= high for _, hole in part.legs ()):
                for shape in part.shapes (bench):
                    x0, y0, x1, y1 = bounds_of (shape)
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
        _, by = self.bench.board_origin ()
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
        bench = self.bench
        size = bench.label_size
        placer = Placer ()
        if rough:
            _, by = bench.board_origin ()
            placer.view = (rough[0] - 30, by * DPI - 40, rough[2] + 30, (by + BOARD_HEIGHT) * DPI + 25)
            placer.soft_view = rough
        placer.avoid (("rect", *bench.mega_box ()))
        for placed in bench.modules.values ():
            placer.avoid (("rect", *placed.reach_box ()))
            placer.taken (placed.title_box ())
        for part in bench.parts:
            for shape in part.shapes (bench):
                placer.avoid (shape)
        for _, _, points in routes:
            for a, b in zip (points, points[1:]):
                placer.avoid (("segment", a[0], a[1], b[0], b[1], 2.4))
        for hole in bench.holes ():
            xy = bench.hole_xy (hole)
            if hole in bench.used:
                placer.avoid (("circle", xy[0], xy[1], 3.4))
            else:
                placer.point (xy, 3)
        for name in bench.taken:
            if name in MEGA_PINS:
                placer.avoid (("circle", *bench.pin_xy (name), 3.4))
        for x, y, w in self._print_marks (detail):
            placer.avoid (("rect", x - w / 2, y - 6, x + w / 2, y), 150)
        for x0, y0, x1, y1 in (bench.board_box (), bench.mega_box ()):
            for edge in ((x0, y0, x1, y0), (x0, y1, x1, y1), (x0, y0, x0, y1), (x1, y0, x1, y1)):
                placer.avoid (("segment", *edge, 0.6), 60)
        for box in self._silk_boxes ():
            placer.avoid (("rect", *box), 400)
        for x, y, w in self._board_signs ():
            placer.taken ((x - w / 2, y - 7, x + w / 2, y + 1))

        placed, self._placed_boxes = [], []

        named = []

        # A label that can only cover something else is an error, never an
        # overlap drawn without a word.
        def clear (text, best):
            if best[0] >= HARD:
                raise ValueError (f"the label {text!r} has no room of its own: move what crowds "
                                  f"it{', or show other columns in the close-up' if rough else ''}")

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
            # A leg's own label, such as an RGB LED's R, may touch its hole;
            # the whole bench leaves those to the close-up.
            if label.leg and not rough:
                return
            mine = [("circle", *label.leg, 3.4)] if label.leg else ()
            best = placer.place (text, size * scale, spots, own, mine)
            if best is None or beside and best[0] >= HARD:
                return
            clear (text, best)
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
        board = bench.board_box ()
        # Whether a part lies off the board, as a module standing in a row does.
        part_box = lambda shapes: any (bounds_of (shape)[3] > board[3]
                                       or bounds_of (shape)[1] < board[1]
                                       for shape in shapes if shape[0] == "rect")
        # A row of like resistors in the same two rows is named once, as a
        # parts list names them: "4 × 220 Ω", beside the last of them.
        rows = {}
        resistors = [part for part in bench.parts
                     if isinstance (part, Resistor) and seen (part.geometry (bench)[0])]
        for part in sorted (resistors, key=lambda part: part.geometry (bench)[0]):
            key = (part.value, parse_hole (part.a)[2], parse_hole (part.b)[2])
            groups = rows.setdefault (key, [[]])
            # A gap of more than six columns starts another row.
            if groups[-1] and part.geometry (bench)[0] - groups[-1][-1].geometry (bench)[0] > 65:
                groups.append ([])
            groups[-1].append (part)
        rows = [(key[0], group) for key, groups in rows.items () for group in groups]
        grouped = {id (part) for _, group in rows if len (group) > 1 for part in group}
        for value, group in rows:
            if len (group) < 2:
                continue
            shapes = [shape for part in group for shape in part.shapes (bench)]
            bounds = [bounds_of (shape) for shape in shapes]
            box = (min (b[0] for b in bounds), min (b[1] for b in bounds),
                   max (b[2] for b in bounds), max (b[3] for b in bounds))
            last = max (group, key=lambda part: part.geometry (bench)[0])
            text = f"{len (group)} × {value}"
            spots = spots_round (box, text, size, "right")[:8] + last.labels (bench)[0].spots
            put (Label (text, spots, last.geometry (bench)), shapes)
        for part in bench.parts:
            # The whole bench leaves an LED's color to speak for it; the
            # close-up names each one.
            if not rough and isinstance (part, Led):
                continue
            own = part.shapes (bench) + [("circle", *bench.hole_xy (hole), 3.4)
                                        for _, hole in part.legs ()]
            for label in part.labels (bench) if id (part) not in grouped else ():
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
                clear (text, best)
                _, x, y, anchor, box = best
                to = next (spot[4] for spot in spots if spot[:3] == (x, y, anchor))
                placer.taken (box)
                placed.append ((text, x, y, anchor, size * 0.9, to, "label"))
                self._placed_boxes.append (box)
        for text, at, offset in bench.notes:
            tx, ty = bench.point_xy (at)
            if rough and not (rough[0] < tx < rough[2] and rough[1] < ty < rough[3]):
                continue
            note_size = size * 0.95
            spots = [(tx + offset[0] * fx * reach * DPI, ty + offset[1] * fy * reach * DPI, "middle",
                      (reach - 1) * 20 + (fx < 0) * 3 + (fy < 0) * 3)
                     for reach in (1, 1.4, 1.8) for fx in (1, -1) for fy in (1, -1)]
            best = placer.place (text, note_size, spots)
            clear (text, best)
            _, x, y, anchor, box = best
            placer.taken (box)
            placed.append ((text, x, y, anchor, note_size, arrow_to (box, (tx, ty), note_size), "note"))
            self._placed_boxes.append (box)
        return placed

    # A label's paper, widened to cover whole any hole it would cut in half.
    def _patch (self, x, y, width, size, anchor):
        bench = self.bench
        left = {"start": x, "end": x - width}.get (anchor, x - width / 2) - 1.2
        right = left + width + 2.4
        top, bottom = y - size * 0.78, y + size * 0.24
        for hole in bench.holes ():
            hx, hy = bench.hole_xy (hole)
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
        for name, pin in MEGA_PINS.items ():
            hx, hy = self.bench.pin_xy (name)
            length = len (canonical (name)) * 4.2 + 2
            if pin.header == "top":
                boxes.append ((hx - 4, hy + 5, hx + 4, hy + 8 + length))
            elif pin.header == "bottom":
                boxes.append ((hx - 4, hy - 8 - length, hx + 4, hy - 5))
            elif not pin.outer:
                boxes.append ((hx - 36, hy - 5, hx - 5, hy + 5))
        return boxes

    # Where the board's own printing is: column numbers, which labels
    # would rather not hide.
    def _print_marks (self, detail=None):
        bench = self.bench
        marks = []
        _, by = bench.board_origin ()
        for column in range (bench.first, bench.last + 1):
            listed = detail and detail[0] <= column <= detail[1]
            if column % 5 == 0 or column == 1 or listed:
                hx, _ = bench.hole_xy (f"a{column}")
                w = len (str (column)) * 4.5 + 2
                marks += [(hx, (by + 0.44) * DPI + 1, w), (hx, (by + 1.82) * DPI + 1, w)]
        return marks

    # The row letters and rail signs, which labels never cover.
    def _board_signs (self):
        bench = self.bench
        signs = []
        bx, by = bench.board_origin ()
        x = bx * DPI
        if bench.first == 1:
            for rail in ("T+", "T-", "B+", "B-"):
                signs.append ((x + 8, (by + ROWS[rail]) * DPI + 3.5, 7))
        for row in "abcdefghij":
            hx, hy = bench.hole_xy (f"{row}{bench.first}")
            signs.append ((hx - 11, hy + 2.3, 5))
        return signs

    # Pictures --------------------------------------------------------------

    def _draw_wire (self, pencil, start, end, route):
        bench = self.bench
        lead = next ((e for e in (start, end) if bench.style (e) == "lead"), None)
        color = next (c for s, e, c, _ in bench.wires if {s, e} == {start, end})
        pencil.wire (route, WIRES[color], width=2.4 if lead else 3.6)
        for which, point in ((start, route[0]), (end, route[-1])):
            style = bench.style (which)
            if which[0] == "module":
                placed, pin = bench.module_pin (which[1])
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
        mx, my = self.bench.mega_origin ()
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
        bench = self.bench
        groups = {}
        for name, pin in MEGA_PINS.items ():
            groups.setdefault ((pin.header, pin.block), []).append (name)
        for names in groups.values ():
            points = [bench.pin_xy (n) for n in names]
            left = min (p[0] for p in points) - 5
            top = min (p[1] for p in points) - 5
            right = max (p[0] for p in points) + 5
            bottom = max (p[1] for p in points) + 5
            pencil.fill ([(left, top), (right, top), (right, bottom), (left, bottom)], tone=0.72)
            pencil.rect (left, top, right - left, bottom - top, width=0.6)

        for name, pin in MEGA_PINS.items ():
            hx, hy = bench.pin_xy (name)
            pencil.socket (hx, hy)
            label = canonical (name)
            weight, tone = ("bold", 0.95) if name in bench.taken else ("normal", 0.7)
            common = dict (size=7, kind="silk", halo=MEGA_TINT, weight=weight, tone=tone)
            if pin.header == "top":
                pencil.text (hx + 2.4, hy + 8, label, rotate=-90, anchor="end", **common)
            elif pin.header == "bottom":
                pencil.text (hx + 2.4, hy - 8, label, rotate=-90, anchor="start", **common)
        # The double header's names both sit on its inner side, away from
        # the wires that leave it to the right: the left pin's name, then the
        # right pin's.
        for name, pin in MEGA_PINS.items ():
            if pin.header != "double" or pin.outer:
                continue
            odd = next (n for n, other in MEGA_PINS.items ()
                        if other.header == "double" and other.outer and other.y == pin.y)
            hx, hy = bench.pin_xy (name)
            pair = [(canonical (odd), odd, 0),
                    (canonical (name), name, len (canonical (odd)) * 3.9 + 3.5)]
            if canonical (odd) == canonical (name):
                # Both 5V, or both GND: one name for the row.
                pair = [(canonical (name), name if name in bench.taken else odd, 0)]
            for label, pin, offset in pair:
                weight, tone = ("bold", 0.95) if pin in bench.taken else ("normal", 0.7)
                pencil.text (hx - 7 - offset, hy + 2.5, label, size=7, anchor="end", kind="silk",
                             halo=MEGA_TINT, weight=weight, tone=tone)

    def _draw_board (self, pencil, detail=None):
        bench = self.bench
        bx, by = bench.board_origin ()
        x, y = bx * DPI, by * DPI
        w, h = bench.board_width () * DPI, BOARD_HEIGHT * DPI
        torn_left, torn_right = bench.first > 1, bench.last < 63
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
        for column in range (bench.first, bench.last + 1):
            for row in "abcdefghij":
                pencil.hole (*bench.hole_xy (f"{row}{column}"))
            if rail_column (column):
                for rail in ("T+", "T-", "B+", "B-"):
                    pencil.hole (*bench.hole_xy (f"{rail}{column}"))
            listed = detail and detail[0] <= column <= detail[1]
            if column % 5 == 0 or column == 1 or listed:
                hx, _ = bench.hole_xy (f"a{column}")
                size = 5.5 if listed and column % 5 else 7
                pencil.text (hx, y + 0.44 * DPI, str (column), size=size, tone=0.7, **board)
                pencil.text (hx, y + 1.82 * DPI, str (column), size=size, tone=0.7, **board)
        # Row letters in the board's margins, as printed; in a close-up that
        # starts partway along, between the first two columns in view.
        edges = [(bench.first, -11)] + ([] if torn_right else [(bench.last, 11)])
        if detail and detail[0] > bench.first:
            edges.append ((detail[0], -5))
        for column, offset in edges:
            for row in "abcdefghij":
                hx, hy = bench.hole_xy (f"{row}{column}")
                pencil.text (hx + offset, hy + 2.3, row, size=6.5, tone=0.7, **board)


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


# Whether the line from a to b passes over a shape, away from its ends.
def shape_crosses (shape, a, b):
    length = max (1e-9, math.dist (a, b))
    for step in range (1, int (length / 2)):
        t = step * 2 / length
        p = (a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t)
        x0, y0, x1, y1 = bounds_of (shape)
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


# Spots for a wire's label beside it, from its labeled end outwards: above
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
