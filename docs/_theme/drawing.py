"""The pencil drawings of a bench: the whole bench, a close-up of the
breadboard, and each measurement with its meter, all from the circuit a
lesson's circuit.py describes (bench.py).

    drawing = Drawing (bench)
    drawing.svg ("bench")           the Mega, the breadboard, every part and wire
    drawing.svg ("opening")         the visible result, including off-board screens
    drawing.svg ("closeup")         the breadboard round the parts, larger, as the
                                    home page shows Lesson 1's
    drawing.measure_svg (0)         the first measurement's probes and meter

Every wire is routed once for all of them (route.py): round parts, modules
and labels, the jumpers that can lie straight first and the rest in rounds
of negotiation until no two share a stretch of grid. Labels go where they
cover nothing they shouldn't, and a label with no room is an error.

Routes, and the finished drawings of a circuit read from a lesson's
circuit.py, are kept in the folder ADK_DRAWINGS names, the Makefile's
build/drawings, so a rebuild draws again only what has changed. A site
build draws every lesson at once before it starts, a process to a core
(draw_all).
"""

import concurrent.futures
import functools
import hashlib
import inspect
import json
import logging
import math
import os
import re
import shutil
import sys

import meter

from bench import (BOARD_HEIGHT, END, MARGIN, MEGA_HEIGHT, MEGA_PINS, MEGA_WIDTH, ROWS, canonical,
                   identity, load, numbered, parse_hole, rail_column)
from modules import HOUSING, Lcd1602, Matrix, Motor, Servo, Stepper
from parts import HeaderModule, Label, Led, Resistor, back_to_front, spots_round
from pencil import DPI, WIRES, Pencil, leader_start
from route import (HARD, STEP, Placer, Router, bounds_of, corners, direction, node,
                   segment_distance, segment_meets_box, shape_crosses_segment, text_box,
                   text_width)

MEGA_TINT = "#dfe6e8"
CLOSEUP_COLUMNS = 16                # the narrowest close-up, so parts keep one scale
ROUNDS = 6                          # of negotiation between the wires
# How far out a label crowded from beside its part on the whole bench may go,
# on its leader: as far as the margin round the board.
FAR = (16, 24, 34, 46, 60, 80, 100, 125, 150)
# What a label costs over a probe's hole, which it would hide from a learner
# looking for it: more than a leader across a wire or two.
PROBED = 1000

# Directions a wire may leave a header in: the way its pins point, or a
# little either side, to fan out from its neighbors.
LEAVING = {"top": (6, 5, 7), "bottom": (2, 1, 3), "double": (0, 7, 1)}

# The modules that move, which the opening shows beside a screen.
MOVING = (Servo, Motor, Stepper)
# A row of like parts' one name, as "4 × 220 Ω".
GROUPED = re.compile (r"\d+ × ")

# Where routes and drawings are kept, if anywhere, and this version of the
# engine: a digest of the theme's Python.
KEPT = os.environ.get ("ADK_DRAWINGS")
THEME = os.path.dirname (os.path.abspath (__file__))
ENGINE = hashlib.sha256 (b"".join (open (os.path.join (THEME, name), "rb").read ()
                                   for name in sorted (os.listdir (THEME))
                                   if name.endswith (".py"))).hexdigest ()[:16]
# The drawings this process has made, read or been handed, by key.
DRAWN = {}


# What this version of the engine kept under a name, or None.
def recall (name):
    if not KEPT:
        return None
    try:
        with open (os.path.join (KEPT, ENGINE, name), encoding="utf-8") as file:
            return file.read ()
    except OSError:
        return None


# Keep text under a name. Each version of the engine keeps its own folder,
# and the first thing it keeps clears the other versions' away. A file is
# written whole and then renamed, so a build drawing in parallel never
# reads half of one.
def keep (name, text):
    if not KEPT:
        return
    folder = os.path.join (KEPT, ENGINE)
    if not os.path.isdir (folder):
        for old in set (os.listdir (KEPT) if os.path.isdir (KEPT) else ()) - {ENGINE}:
            shutil.rmtree (os.path.join (KEPT, old), ignore_errors=True)
        os.makedirs (folder, exist_ok=True)
    part = os.path.join (folder, f"{name}.{os.getpid ()}")
    with open (part, "w", encoding="utf-8") as file:
        file.write (text)
    os.replace (part, os.path.join (folder, name))


# A drawing is kept by the circuit it draws (the bench's source: its
# circuit.py's digest and its board's letter) and what was asked of it. A
# bench made any other way, as the tests make theirs, is drawn every time.
def kept (draw):
    signature = inspect.signature (draw)

    @functools.wraps (draw)
    def drawing (self, *args, **options):
        if self.bench.source is None:
            return draw (self, *args, **options)
        asked = signature.bind (self, *args, **options)
        asked.apply_defaults ()
        wanted = (self.bench.source, draw.__name__, list (asked.arguments.values ())[1:])
        key = hashlib.sha256 (repr (wanted).encode ()).hexdigest () + ".svg"
        # Only a finished drawing goes in DRAWN: one that raised must not
        # leave a None behind for the pages to find in its place.
        if key not in DRAWN:
            text = recall (key)
            if text is None:
                text = draw (self, *args, **options)
                keep (key, text)
            DRAWN[key] = text
        return DRAWN[key]
    return drawing


# Drawing is nearly all of a site build's time, so the build draws every
# lesson at once before it starts, a process to a core, and keeps what they
# drew in DRAWN for the pages to find. Each circuit comes with what the
# lesson before hands on (bench.load), which colors the wires it keeps. A
# circuit that fails is left to its page, which says why; so is
# everything, where processes can't be had. The biggest circuits, the
# two-board lessons, start first so none is left to finish alone. The
# processes import this module from the theme's folder, which MkDocs takes
# off the path once the hooks are loaded.
def draw_all (circuits):
    DRAWN.clear ()
    biggest = sorted (circuits, key=lambda circuit: os.path.getsize (circuit[0]), reverse=True)
    for drawn in _each (_draw_circuit, biggest):
        DRAWN.update (drawn)


# A lesson's colors hang on the routes of every lesson before it, so the
# build routes every circuit at once first, a process to a core, and keeps
# the routes, before it reads them in order (hooks.load_all).
def route_all (paths):
    if KEPT:
        for _ in _each (_route_circuit, sorted (paths, key=os.path.getsize, reverse=True)):
            pass


def _each (work, jobs):
    if THEME not in sys.path:
        sys.path.insert (0, THEME)
    try:
        with concurrent.futures.ProcessPoolExecutor () as pool:
            yield from pool.map (work, jobs)
    except (OSError, NotImplementedError, concurrent.futures.process.BrokenProcessPool) as error:
        logging.getLogger ("mkdocs").info (f"Taking the lessons one at a time: {error}")


def _draw_circuit (circuit):
    path, before = circuit
    DRAWN.clear ()
    try:
        for letter, bench in load (path, before).items ():
            Drawing (bench).page (letter)
    except Exception:  # noqa: BLE001 - the page draws it again, and says what failed
        pass
    return dict (DRAWN)


def _route_circuit (path):
    try:
        load (path)
    except Exception:  # noqa: BLE001 - its page reads it again, and says what failed
        pass


class Drawing:
    """A finished bench's drawings, which share one routing of its wires."""

    def __init__ (self, bench):
        self.bench = bench
        self._routes = None
        self._paths = None
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
        for plan, supply in zip (plans, bench.supplies ()):
            plan["supply"] = supply
        paths = self._recall (plans)
        if paths is None:
            paths = self._negotiate_all (plans)
            self._remember (plans, paths)
        paths = self._carry (plans, paths)
        self._paths = paths
        routes = []
        for index, plan in enumerate (plans):
            drawn = corners (paths[index])
            drawn[0], drawn[-1] = plan["a"], plan["b"]
            routes.append ((plan["start"], plan["end"], drawn))
        self._routes = routes
        return routes

    def _negotiate_all (self, plans):
        router = self._router ()
        router.supplies = {index: plan["supply"] for index, plan in enumerate (plans)}
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

    # Each wire's way across the grid, by what makes it the same wire in
    # the next lesson (bench.load), for that lesson to keep.
    def ways (self):
        self._layout ()
        return {identity (self.bench, "wire", wire): self._paths[index]
                for index, wire in enumerate (self.bench.wires)}

    # A wire kept from the lesson before keeps its way there, where that is
    # still clear and shares no grid with the others', so a build changes
    # as little on paper as on the bench. Every lesson is routed on its own
    # first, all at once (route_all), and only then are kept ways put back,
    # in course order; what that gives is kept too.
    def _carry (self, plans, paths):
        bench = self.bench
        if not bench.ways:
            return paths
        name = f"{self._fingerprint (plans)}-{ways_digest (bench.ways)}.json"
        try:
            kept = json.loads (recall (name) or "")
            return {int (index): [tuple (n) for n in path] for index, path in kept.items ()}
        except ValueError:
            pass
        router = self._router ()
        for index, path in paths.items ():
            router.claim (path, index)
        paths = dict (paths)
        ways = {}
        for index, plan in enumerate (plans):
            way = [tuple (n) for n in bench.ways.get (identity (bench, "wire", bench.wires[index]),
                                                      ())]
            if way and way[0] != plan["points"][0]:
                way.reverse ()
            if way and way[0] == plan["points"][0] and way[-1] == plan["points"][-1] and \
                    router.free (way, plan["allow"]):
                ways[index] = way
        # One kept way can stand in another's new one: put them back until
        # none more will go. A way that would reach further out than
        # anything else drawn went round something no longer there.
        x0, y0, x1, y1 = (v / STEP for v in self._extent ())
        moved = True
        while moved:
            moved = False
            for index, way in ways.items ():
                if paths[index] == way:
                    continue
                others = [n for other, path in paths.items () if other != index for n in path]
                if min (n[0] for n in way) < min ([x0] + [n[0] for n in others]) or \
                        max (n[0] for n in way) > max ([x1] + [n[0] for n in others]) or \
                        min (n[1] for n in way) < min ([y0] + [n[1] for n in others]) or \
                        max (n[1] for n in way) > max ([y1] + [n[1] for n in others]):
                    continue
                router.unclaim (index)
                router.claim (way, index)
                if any (index in router.occupied[n] for n in router.clashes ()[0]):
                    router.unclaim (index)
                    router.claim (paths[index], index)
                else:
                    paths[index], moved = way, True
        keep (name, json.dumps (paths))
        return paths

    # Routes worked out before are kept (keep) by everything the router is
    # given, so a rebuild only routes what has changed.
    def _fingerprint (self, plans):
        bench = self.bench
        text = repr ((bench.first, bench.last, bench.gap, sorted (bench.used), sorted (bench.taken),
                      [(p["points"], p["first"], sorted (p["allow"]), p["keep_off"], p["straight"],
                        p["along"], p["supply"]) for p in plans],
                      [(type (part).__name__, part.legs (), part.shapes (bench), part.blocks,
                        [(label.text, label.spots[0]) for label in part.labels (bench)[:1]])
                       for part in bench.parts],
                      [(m.reach_box (), m.title_box ()) for m in bench.modules.values ()]))
        return hashlib.sha256 (text.encode ()).hexdigest ()

    def _recall (self, plans):
        try:
            paths = json.loads (recall (self._fingerprint (plans) + ".json") or "")
        except ValueError:
            return None
        return {int (index): [tuple (n) for n in path] for index, path in paths.items ()}

    def _remember (self, plans, paths):
        keep (self._fingerprint (plans) + ".json", json.dumps (paths))

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
        # A wire would rather not lie along a rail strip, where it would
        # seem to join the rail, unless it goes to one.
        along = not any (kind == "hole" and parse_hole (name)[0] == "rail"
                         for kind, name in (start, end))
        plan = dict (start=start, end=end, a=a, b=b, first=first, allow=allow_a | allow_b,
                     points=points,
                     keep_off=bench.board_box () if "hole" not in (start[0], end[0]) else None,
                     along=along, straight=None)
        if start[0] == end[0] == "hole" and not via and self._straight (start[1], end[1]):
            plan["straight"] = straight_nodes (node (a), node (b))
        return plan

    def _negotiate (self, router, plan, index, pressure):
        bench = self.bench
        try:
            path, _ = router.route (plan["points"], plan["first"], plan["allow"], plan["keep_off"],
                                    index, pressure, plan["along"])
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
        # A jumper's plug in a hole, which other wires pass clear of; a
        # part's own lead ends in a hole with no plug.
        for start, end, _, _ in bench.wires:
            if "lead" not in (bench.style (start), bench.style (end)):
                for kind, name in (start, end):
                    if kind == "hole":
                        router.plug (bench.hole_xy (name))
        # Along a rail strip, among the rail's holes and over its stripe, a
        # wire would read as joined to the rail (_plan says which may).
        bx0, by0, bx1, by1 = bench.board_box ()
        _, by = bench.board_origin ()
        router.along ((bx0, by0, bx1, (by + 0.45) * DPI))
        router.along ((bx0, (by + 1.75) * DPI, bx1, by1))
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
            if placed.kind.blocks:
                router.block (("rect", *placed.reach_box ()))
            else:
                router.cost (("rect", *placed.reach_box ()), 5)
                # A wire may cross a module that lies flat, but not over a
                # pin with no wire on it, as if it went into it.
                for pin in placed.pins ():
                    if pin.name not in placed.wired:
                        router.block (("circle", *placed.anchor (pin)[0], 2))
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
            if placed.kind.blocks and segment_meets_box ((x1, y1), (x2, y2), placed.reach_box (), 2):
                return False
        return True

    # Drawing ------------------------------------------------------------

    # Everything a lesson's page shows of a board, by the names the page
    # gives its drawings: the bench, the opening, and each measurement.
    def page (self, letter=""):
        return (self.svg ("bench", "bench" + letter), self.svg ("opening", "opening" + letter),
                [self.measure_svg (index, "measure" + letter)
                 for index in range (len (self.bench.measurements))])

    # The whole bench, or a close-up of the breadboard where the parts are.
    @kept
    def svg (self, view="bench", prefix="bench"):
        bench = self.bench
        routes = self._layout ()
        pencil = Pencil (bench.seed, prefix)
        detail = self._closeup_columns () if view == "closeup" else None
        # Labels are set smaller in the close-up, which the page shows larger.
        bench.label_size = 6.4 if view == "closeup" else 10
        rough = self._closeup_box () if view == "closeup" else None
        placed = self._place_labels (routes, rough, detail) if rough else self._bench_labels ()
        box = self._view_box (rough, placed, detail) if rough else self._canvas (routes, placed)
        if view == "opening":
            box = self.opening_box ()
            # It keeps only the labels it shows whole, with what they name.
            placed = [label for label in placed if shown (box, label)]
        self._draw_mega (pencil)
        self._draw_board (pencil, detail)
        # Each part, module and wire is tagged with its build step's place in
        # the bench's items, which the steps table names too.
        keys = {id (thing): index for index, (_, thing, _) in enumerate (bench.items)}
        wires = {frozenset (wire[:2]): keys[id (wire)] for wire in bench.wires}

        def draw (thing, key):
            pencil.item (key)
            thing ()
            pencil.done ()

        for part in back_to_front (bench.parts, bench):
            draw (lambda: part.draw (pencil, bench), keys[id (part)])
        # A module lying flat, as the power module does, goes under the
        # wires that cross it; the others over them.
        for module in bench.modules.values ():
            if not module.kind.blocks:
                draw (lambda: module.draw (pencil), keys[id (module)])
        for start, end, points in routes:
            draw (lambda: self._draw_wire (pencil, start, end, points),
                  wires[frozenset ((start, end))])
        for module in bench.modules.values ():
            if module.kind.blocks:
                draw (lambda: module.draw (pencil), keys[id (module)])
        for text, x, y, anchor, size, to, kind in placed:
            if kind == "note":
                pencil.text (x, y, text, size=size, anchor=anchor, kind="label", italic=True)
                pencil.arrow (to)
            else:
                width = text_width (text, size) - 1
                pencil.label (x, y, text, size=size, anchor=anchor, to=to, width=width,
                              patch=self._patch (x, y, width, size, anchor))
        return pencil.svg (box, bench.title if view == "bench" else f"{bench.title}: close-up")

    # The whole bench's labels, placed once for all its drawings: the bench,
    # the opening, and the printed details, which keep their row letters
    # clear of them.
    def _bench_labels (self):
        labels = json.loads (self._labels ())
        self._placed_boxes = [tuple (box) for box in labels["boxes"]]
        return [tuple (label) for label in labels["placed"]]

    @kept
    def _labels (self):
        self.bench.label_size = 10
        placed = self._place_labels (self._layout (), None, None)
        return json.dumps ({"placed": placed, "boxes": self._placed_boxes})

    # The opening shows the actual parts: a screen and whatever moves, or
    # the parts on the board and the modules that belong with them, each
    # with its name. Wiring detail comes later.
    def opening_box (self):
        bench = self.bench
        modules = list (bench.modules.values ())
        modules += [part.placed (bench) for part in bench.parts if isinstance (part, HeaderModule)]
        screens = [module for module in modules if isinstance (module.kind, (Lcd1602, Matrix))]
        # A screen is the visible result, and so is a servo's arm or a fan
        # turning. Framing a distant button with them would turn a readable
        # picture into a tall strip of empty wiring.
        if screens:
            shown = screens + [module for module in modules if isinstance (module.kind, MOVING)]
            boxes = [box for module in shown for box in (module.reach_box (), module.title_box ())]
        else:
            boxes = [bounds_of (shape) for part in bench.parts for shape in part.shapes (bench)]
            boxes += [box for module in self._results () for box in module]
        if not boxes:
            boxes = [bench.board_box ()]
        left, top = min (box[0] for box in boxes) - 30, min (box[1] for box in boxes) - 30
        right, bottom = max (box[2] for box in boxes) + 30, max (box[3] for box in boxes) + 30
        # A part's name that the frame would cut in half, or leave outside
        # at the end of its leader, is framed in whole rather than left out:
        # the coil a lesson is about is never shown unnamed.
        if not screens:
            names = {label.text for part in bench.parts for label in part.labels (bench)}
            cut = (left, top, right, bottom)
            for text, x, y, anchor, size, to, kind in self._bench_labels ():
                x0, y0, x1, y1 = box = text_box (x, y, text, size, anchor)
                if kind == "label" and (text in names or GROUPED.match (text)) and \
                        (boxes_meet (box, cut) or to and boxes_meet ((*to, *to), cut)):
                    left, top = min (left, x0 - 8), min (top, y0 - 8)
                    right, bottom = max (right, x1 + 8), max (bottom, y1 + 8)
        width = max (160, right - left)
        height = max (110, bottom - top)
        return ((left + right - width) / 2, (top + bottom - height) / 2, width, height)

    # The modules off the board that are part of what the lesson builds,
    # each as its box and the corners of the wires running close by it:
    # those the Mega drives or reads, and those standing by the board. Not a
    # supply, such as the power module, nor an instrument standing away from
    # the board, such as the signal generator; and not their names, which
    # would widen the picture for a word.
    def _results (self):
        bench = self.bench
        bx0, by0, bx1, by1 = bench.board_box ()
        board = (bx0 - DPI, by0 - DPI, bx1 + DPI, by1 + DPI)
        driven = {end[1].split (".")[0] for wire in bench.wires if "pin" in (wire[0][0], wire[1][0])
                  for end in wire[:2] if end[0] == "module"}
        results = []
        for name, module in bench.modules.items ():
            x0, y0, x1, y1 = box = module.reach_box ()
            if module.kind.sources or name not in driven and not boxes_meet (box, board):
                continue
            corners = [(x - 4, y - 4, x + 4, y + 4) for _, _, points in self._layout ()
                       for x, y in points if x0 - 80 < x < x1 + 80 and y0 - 80 < y < y1 + 80]
            results.append ([box] + corners)
        return results

    # Paper cannot scroll or open the guided view. Divide the occupied
    # breadboard into readable, overlapping ranges, and give each off-board
    # module its own view. All views retain the finished drawing's orientation.
    def print_regions (self):
        bench = self.bench
        regions = []
        left, top, right, bottom = bench.mega_box ()
        regions.append (("Mega · check the pin numbers", (left - 12, top - 12,
                         right - left + 24, bottom - top + 24), None))
        standard = {hole for start, end, _, _ in bench.wires if bench.is_standard ((start, end))
                    for kind, hole in (start, end) if kind == "hole"}
        columns = sorted ({parse_hole (hole)[1] for hole in bench.used if hole not in standard})
        groups = []
        for column in columns:
            if not groups or column - groups[-1][-1] > 6:
                groups.append ([])
            groups[-1].append (column)
        for group in groups:
            low, high = max (bench.first, group[0] - 2), min (bench.last, group[-1] + 2)
            first = low
            while first <= high:
                last = min (high, first + 33)
                x, _ = bench.hole_xy (f"a{first}")
                right, _ = bench.hole_xy (f"a{last}")
                _, top, _, bottom = bench.board_box ()
                regions.append ((f"Breadboard · columns {first}–{last}",
                                 (x - 36, top - 35, right - x + 72, bottom - top + 70),
                                 (first, last)))
                if last == high:
                    break
                first = last - 1
        modules = list (bench.modules.values ())
        modules += [part.placed (bench) for part in bench.parts if isinstance (part, HeaderModule)]
        for module in modules:
            points = [module.anchor (pin)[0] for pin in module.pins ()]
            if not points:
                continue
            left, top = min (p[0] for p in points), min (p[1] for p in points)
            right, bottom = max (p[0] for p in points), max (p[1] for p in points)
            regions.append ((module.title + " · connections",
                             (left - 38, top - 38, right - left + 76, bottom - top + 76), None))
        return [(caption, self._whole_names (crop), columns) for caption, crop, columns in regions]

    # A printed detail shows the bench's own labels, so its frame would cut
    # a name at its edge to "1N4007 dio" or "50 transistor". It grows to
    # take in whole every name it cuts, and any those bring in.
    def _whole_names (self, crop):
        x, y, width, height = crop
        left, top, right, bottom = x, y, x + width, y + height
        names = json.loads (self._labels ())["boxes"]
        grown = True
        while grown:
            grown = False
            for x0, y0, x1, y1 in names:
                whole = left <= x0 and x1 <= right and top <= y0 and y1 <= bottom
                if not whole and boxes_meet ((x0, y0, x1, y1), (left, top, right, bottom)):
                    left, top = min (left, x0 - 4), min (top, y0 - 4)
                    right, bottom = max (right, x1 + 4), max (bottom, y1 + 4)
                    grown = True
        return left, top, right - left, bottom - top

    # Reuse the overview's scene instead of copying thousands of SVG nodes
    # for each printed crop. Add row names at a crop's own edge;
    # the wiring itself is exactly the same scene, at a readable scale.
    # Keep the existing column numbers: the label placer reserved their
    # space, whereas extra numbers could cover a wire's pin label.
    def print_svg (self, caption, box, columns, prefix, source):
        pencil = Pencil (self.bench.seed, prefix)
        pencil.layers["paper"].append (f'<use href="#{source}-scene"/>')
        if columns:
            # A frame grown to hold a name whole starts further along: its
            # row letters stand by the first column it shows.
            first = min (columns[0], self._column_at (box[0] + 36))
            original, _ = self.bench.hole_xy (f"a{self.bench.first}")
            if not box[0] <= original - 11 <= box[0] + box[2]:
                self._row_letters (pencil, first)
        return pencil.svg (box, caption)

    # Row letters for a printed detail that starts partway along the board:
    # in a column between the holes left of its first, where they cover the
    # fewest of the bench's labels and wires. A letter that would still
    # cover a label goes in another such column, or is left out.
    def _row_letters (self, pencil, first):
        bench = self.bench
        left, _ = bench.hole_xy (f"a{first}")
        labels = json.loads (self._labels ())["boxes"]
        runs = [(a, b) for _, _, points in self._layout () for a, b in zip (points, points[1:])]
        rows = [(row, bench.hole_xy (f"{row}{first}")[1] + 2) for row in "abcdefghij"]

        def cost (x, y):
            box = (x - 3, y - 5.6, x + 3, y + 1.8)
            return (100 * any (boxes_meet (box, label) for label in labels)
                    + 10 * any (segment_meets_box (a, b, box, 1.8) for a, b in runs))

        columns = [left - 15, left - 25, left - 5]
        best = min (columns, key=lambda x: sum (cost (x, y) for _, y in rows))
        for row, y in rows:
            x = min ([best] + columns, key=lambda x: cost (x, y))
            if cost (x, y) < 100:
                pencil.text (x, y, row, size=7, kind="silk", halo="#ffffff")

    # A measurement: the breadboard round its two probe points, the meter
    # below the board reading what is expected, and its leads rising to the
    # probes. Labels are left to the caption and the table.
    @kept
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
        for part in back_to_front (bench.parts, bench):
            part.draw (pencil, bench)
        for module in bench.modules.values ():
            if not module.kind.blocks:
                module.draw (pencil)
        for start, end, route in routes:
            self._draw_wire (pencil, start, end, route)
        for module in bench.modules.values ():
            if module.kind.blocks:
                module.draw (pencil)
        meter.draw (pencil, mx, my, taken["expect"])
        for color, side in meter.probe_sides (bench, routes, points).items ():
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
        column = round ((x / DPI - bx - END) / 0.1) + bench.first
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
        placer = Labels ()
        if rough:
            _, by = bench.board_origin ()
            placer.view = (rough[0] - 30, by * DPI - 40, rough[2] + 30, (by + BOARD_HEIGHT) * DPI + 25)
            placer.soft_view = rough
        # Each thing a leader may cross is counted once, however many of its
        # shapes it crosses: a wire's run and the end it plugs in by, a part's
        # body and its legs.
        placer.avoid (("rect", *bench.mega_box ()), owner="Mega")
        for name, placed in bench.modules.items ():
            placer.avoid (("rect", *placed.reach_box ()), owner=name)
            placer.body (("rect", *placed.box ()), name)
            placer.taken (placed.title_box ())
        owners = {}
        for part in bench.parts:
            for shape in part.shapes (bench):
                placer.avoid (shape, owner=id (part))
            for shape in part.bodies (bench):
                placer.body (shape, id (part))
            owners.update ((hole, id (part)) for _, hole in part.legs ())
        # The holes a meter's or a scope's probes go in, which the learner
        # looks for on the bench.
        probes = [hole for index in range (len (bench.measurements))
                  for hole, _ in bench.probes (index)]
        probes += [hole for index in range (len (bench.scope_probes))
                   for hole, _ in bench.probe_points (index)]
        for hole in probes:
            placer.avoid (("circle", *bench.hole_xy (hole), 3.4), PROBED)
        for index, (start, end, points) in enumerate (routes):
            for a, b in zip (points, points[1:]):
                placer.avoid (("segment", a[0], a[1], b[0], b[1], 2.4), owner=index)
            owners.update ((name, index) for kind, name in (start, end) if kind in ("hole", "pin"))
        for hole in bench.holes ():
            xy = bench.hole_xy (hole)
            if hole in bench.used:
                placer.avoid (("circle", xy[0], xy[1], 3.4), owner=owners.get (hole, hole))
            else:
                placer.point (xy, 3)
        for name in bench.taken:
            if name in MEGA_PINS:
                placer.avoid (("circle", *bench.pin_xy (name), 3.4), owner=owners.get (name, name))
        for x, y, w in self._print_marks (detail):
            placer.avoid (("rect", x - w / 2, y - 6, x + w / 2, y), 150)
        # A label across the board's edge, or the Mega's, reads as struck
        # through.
        for x0, y0, x1, y1 in (bench.board_box (), bench.mega_box ()):
            for edge in ((x0, y0, x1, y0), (x0, y1, x1, y1), (x0, y0, x0, y1), (x1, y0, x1, y1)):
                placer.avoid (("segment", *edge, 0.6), 600)
        for box in self._silk_boxes ():
            placer.avoid (("rect", *box), 400)
        for x, y, w in self._board_signs ():
            placer.taken ((x - w / 2, y - 7, x + w / 2, y + 1))

        placed, self._placed_boxes = [], []

        named = []

        # A label that can only cover something else is an error, never an
        # overlap drawn without a word, nor a part left without its name.
        def clear (text, best):
            if best[0] >= HARD:
                raise ValueError (f"the label {text!r} has no room of its own: move what crowds "
                                  f"it{', or show other columns in the close-up' if rough else ''}")

        # A label, and the leader back from it to what it names, which later
        # labels keep off and later leaders would rather not cross.
        def take (text, x, y, anchor, size, to, box):
            placer.taken (box)
            placed.append ((text, x, y, anchor, size, to, "label"))
            self._placed_boxes.append (box)
            if to:
                placer.lead (leader_from (box, to), to)

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
            # A leg's own label, such as an RGB LED's R, may touch its hole.
            mine = [("circle", *label.leg, 3.4)] if label.leg else ()
            # The whole bench names a part on the breadboard beside it where
            # the name fits, clear of a probe's hole, and only failing that
            # further out, on a leader back to it, once the wires are named
            # (crowded); the close-up, and a part off the board, weigh both
            # at once.
            together = rough or part_box (own)
            spots = list (label.spots) + (unmistaken (text, aimed (label, ring (label.at, text,
                                                                              size * scale)),
                                                      size * scale)
                                          if label.at and together else [])
            best = placer.place (text, size * scale, spots, own, mine)
            if label.at and not together and best[0] >= PROBED:
                crowded.append ((label, own, mine))
                return
            settle (label, best, spots)

        crowded = []

        # Where each part's label points, by its words: a label that stands
        # nearer another part of the same name than its own seems to name
        # that one, as a 2 kΩ beside another modem's 2 kΩ would.
        namesakes = [(label.text, label.at) for part in bench.parts
                     for label in part.labels (bench)[:1] if label.at]

        def unmistaken (text, spots, scale):
            kept = []
            for spot in spots:
                x, y, anchor, cost, to = spot
                box = text_box (x, y, text, scale, anchor)
                near = gap_to (box, to)
                cost += 1000 * any (gap_to (box, at) < near for other, at in namesakes
                                    if other == text and math.dist (at, to) > 1)
                kept.append ((x, y, anchor, cost, to))
            return kept

        # A row of like parts' label points to the one nearest each spot.
        def aimed (label, spots):
            if not label.targets:
                return spots
            return [(x, y, anchor, cost, min (label.targets, key=lambda at: math.dist (at, (x, y))))
                    for x, y, anchor, cost, _ in spots]

        def settle (label, best, spots):
            text = label.text
            clear (text, best)
            cost, x, y, anchor, box = best
            to = None
            # A leader only when the label stands away from what it names.
            if label.at and (x, y, anchor) not in [s[:3] for s in label.spots]:
                aim = next (s[4] for s in spots if s[:3] == (x, y, anchor))
                to = aim if gap_to (box, aim) > 6 else None
            take (text, x, y, anchor, size * label.size, to, box)

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
            legs = [("circle", *bench.hole_xy (hole), 3.4) for part in group
                    for _, hole in part.legs ()]
            put (Label (text, spots, ((box[0] + box[2]) / 2, (box[1] + box[3]) / 2),
                        targets=[part.geometry (bench) for part in group]), shapes + legs)
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
        # A ground or supply wire's name would rather not lie along the rail
        # of the other sign, where "GND" beside a + reads as one.
        _, by = bench.board_origin ()
        rails = {sign: [(board[0], (by + low) * DPI, board[2], (by + high) * DPI)
                        for low, high in bands]
                 for sign, bands in (("+", ((0.21, 0.36), (2.01, 2.2))),
                                     ("−", ((0.04, 0.19), (1.84, 1.99))))}
        # Where each wire ends in a hole: a wire's name right beside another
        # wire's end would seem to name that one, worse than a leader across
        # a wire would.
        ends = [(points[index], (start, end)) for start, end, points in routes
                for index, (kind, _) in ((0, start), (-1, end)) if kind == "hole"]
        for start, end, points in routes:
            # Named by the end that matters: the hole, or the Mega for a module.
            if start[0] != "pin" or not seen ((points[-1] if end[0] == "hole" else points[0])[0]):
                continue
            # Its leader may cross its own wire, and its own wire's ends.
            own = [("segment", a[0], a[1], b[0], b[1], 2.4) for a, b in zip (points, points[1:])]
            own += [("circle", *bench.hole_xy (name), 3.4) for kind, name in (start, end)
                    if kind == "hole"]
            text = canonical (start[1])
            wrong = rails["+"] if text == "GND" else rails["−"] if text in ("5V", "3.3V") else ()
            others = [(x - 6, y - 6, x + 6, y + 6) for (x, y), wire in ends if wire != (start, end)]
            text = f"pin {text}" if numbered (text) else text
            spots = along (points if end[0] != "hole" else points[::-1], size * 0.9)
            spots = [(x, y, anchor, cost + 100 * any (boxes_meet (box, rail) for rail in wrong)
                      + 150 * any (boxes_meet (box, other) for other in others), to)
                     for x, y, anchor, cost, to in spots
                     for box in [text_box (x, y, text, size * 0.9, anchor)]]
            best = placer.place (text, size * 0.9, spots, own)
            if best:
                clear (text, best)
                _, x, y, anchor, box = best
                to = next (spot[4] for spot in spots if spot[:3] == (x, y, anchor))
                take (text, x, y, anchor, size * 0.9, to, box)
        # A crowded name goes further out, on a leader, where it can: round
        # its part in fine steps, centred on each point or, on the side away
        # from the part, starting or ending there, so a long name finds a gap
        # too. A gap is worth a little reach, but not a leader across another
        # part or a wire.
        for label, own, mine in crowded:
            scaled = size * label.size
            spots = list (label.spots) + unmistaken (label.text,
                                                     aimed (label, wide_ring (label.at, scaled)),
                                                     scaled)
            settle (label, placer.place (label.text, scaled, spots, own, mine), spots)
        for text, at, offset in bench.notes:
            tx, ty = self._point (at)
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

    # Where a note points: a module's named spot, such as "servo.horn", or a
    # point as the bench takes one.
    def _point (self, at):
        module, _, name = at.partition (".") if isinstance (at, str) else ("", "", "")
        if name and module in self.bench.modules:
            spot = self.bench.modules[module].spot (name)
            if spot:
                return spot
        return self.bench.point_xy (at)

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
        # Its mounting holes, the chip, the L LED and the headers where
        # Arduino's board file puts them, in inches from the corner by the
        # USB socket.
        for hx, hy in ((0.55, 0.1), (0.6, 2.0), (2.6, 1.4), (2.6, 0.3), (3.55, 2.0), (3.8, 0.1)):
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
        cx, cy, r = x + 2.0 * DPI, y + (MEGA_HEIGHT - 1.12) * DPI, 0.36 * DPI
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
        pencil.text (x + 1.0 * DPI, y + 1.12 * DPI, "ARDUINO", size=12, weight="bold",
                     tone=0.5, kind="silk")
        pencil.text (x + 1.0 * DPI, y + 1.28 * DPI, "MEGA 2560", size=12, weight="bold",
                     tone=0.5, kind="silk")

        # The built-in LED, L, on pin 13.
        lx, ly = x + 1.1 * DPI, y + (MEGA_HEIGHT - 1.66) * DPI
        pencil.rect (lx - 3.5, ly - 2, 7, 4, width=0.6)
        pencil.text (lx + 9, ly + 3, "L", size=7, anchor="start", kind="silk", tone=0.7)

        self._draw_headers (pencil)

    def _draw_headers (self, pencil):
        bench = self.bench
        groups = {}
        for name, pin in MEGA_PINS.items ():
            groups.setdefault ((pin.header, pin.block), []).append (name)
        # The power header's first position has no pin: a socket with no name.
        mx, my = bench.mega_origin ()
        spare = ((mx + 1.10) * DPI, (my + MEGA_HEIGHT - 0.1) * DPI)
        for key, names in groups.items ():
            points = [bench.pin_xy (n) for n in names] + ([spare] if key == ("bottom", 0) else [])
            left = min (p[0] for p in points) - 5
            top = min (p[1] for p in points) - 5
            right = max (p[0] for p in points) + 5
            bottom = max (p[1] for p in points) + 5
            pencil.fill ([(left, top), (right, top), (right, bottom), (left, bottom)], tone=0.72)
            pencil.rect (left, top, right - left, bottom - top, width=0.6)

        pencil.socket (*spare)
        for name, pin in MEGA_PINS.items ():
            hx, hy = bench.pin_xy (name)
            pencil.socket (hx, hy)
            label = canonical (name)
            weight, tone = ("bold", 0.95) if name in bench.taken else ("normal", 0.7)
            common = dict (size=7, kind="silk", halo=MEGA_TINT, weight=weight, tone=tone)
            if pin.header == "top":
                pencil.text (hx + 2.4, hy + 8, label, rotate=-90, anchor="end", **common)
            elif pin.header == "bottom":
                pencil.text (hx + 2.4, hy - 7, label, rotate=-90, anchor="start", **common)
        # The double header's names both sit on its inner side, away from
        # the wires that leave it to the right: the left pin's name, then the
        # right pin's. The last row's GND stands a little high, and the bottom
        # header's names start a little low, so A14 and A15 read whole.
        last = min (pin.y for pin in MEGA_PINS.values () if pin.header == "double")
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
            y = hy + (2.5 if pin.y > last else -1)
            for label, pin, offset in pair:
                weight, tone = ("bold", 0.95) if pin in bench.taken else ("normal", 0.7)
                pencil.text (hx - 7 - offset, y, label, size=7, anchor="end", kind="silk",
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
        for rail, offset in (("T-", 0.06), ("T+", 0.34), ("B-", 1.86), ("B+", 2.14)):
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


class Labels (Placer):
    """The labels placed so far and what they keep clear of, minding each
    leader as well: one that runs under another label, or along another
    leader, leaves unclear which label names what, which is worse than
    crossing a wire or two."""

    def __init__ (self):
        super ().__init__ ()
        self.leaders = []
        self.owners = []                # what each shape belongs to, as avoid () was told
        self.bodies = []                # parts' and modules' bodies, with their owners

    def avoid (self, shape, cost=HARD, owner=None):
        super ().avoid (shape, cost)
        self.owners.append (("shape", len (self.owners)) if owner is None else owner)

    # A leader drawn from start to the point it names, which later labels
    # keep off.
    def lead (self, start, to):
        if math.dist (start, to) > 3:
            self.leaders.append ((start, to))
            self.avoid (("segment", *start, *to, 1.2), owner=("leader", len (self.leaders)))

    # A part's or module's body, which a leader crossing it would seem to
    # point into: worse than crossing a wire.
    def body (self, shape, owner):
        self.bodies.append ((shape, owner))

    # A leader runs from the label's side to just short of its point. Each
    # wire, part or leader it crosses costs, once however many of its shapes
    # it crosses, and each body more; a leader under another label, or along
    # another leader, leaves unclear which label names what.
    def leader_cost (self, box, to, own=()):
        start = leader_from (box, to)
        length = math.dist (start, to)
        if length < 6:
            return 0
        tip = (to[0] + (start[0] - to[0]) * 3 / length, to[1] + (start[1] - to[1]) * 3 / length)
        crossed = {owner for (shape, amount), owner in zip (self.shapes, self.owners)
                   if amount >= HARD and shape not in own
                   and shape_crosses_segment (shape, start, tip)}
        bodies = {owner for shape, owner in self.bodies
                  if shape not in own and shape_crosses_segment (shape, start, tip)}
        cost = 500 * len (crossed) + 1500 * len (bodies)
        cost += 2000 * sum (segment_meets_box (start, to, other, 2) for other in self.labels)
        return cost + 2000 * sum (alongside (start, to, *leader) for leader in self.leaders)


# Where the leader from a label's box to a point leaves the label, as the
# pencil draws it.
def leader_from (box, to):
    size = (box[3] - box[1]) / 1.06
    return leader_start (box[0] + 1, box[2] - box[0] - 3, box[3] - size * 0.26, size, to)


# Whether the segment from a to b runs along the one from p to q, rather
# than just crossing it.
def alongside (a, b, p, q):
    steps = max (2, int (math.dist (a, b) / 2))
    near = sum (segment_distance (p, q, (a[0] + (b[0] - a[0]) * k / steps,
                                         a[1] + (b[1] - a[1]) * k / steps)) < 2.5
                for k in range (steps + 1))
    return near > 3


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


# The ways a bench keeps from the lesson before, as one short digest.
def ways_digest (ways):
    text = repr (sorted ((sorted (key[1]), path) for key, path in ways.items ()))
    return hashlib.sha256 (text.encode ()).hexdigest ()[:16]


def boxes_meet (a, b):
    return a[0] < b[2] and b[0] < a[2] and a[1] < b[3] and b[1] < a[3]


# How far a point lies outside a box.
def gap_to (box, point):
    return math.hypot (max (box[0] - point[0], 0, point[0] - box[2]),
                       max (box[1] - point[1], 0, point[1] - box[3]))


# Whether a view, (x, y, width, height), shows a placed label whole, and
# the point its leader or arrow names.
def shown (view, label):
    text, x, y, anchor, size, to, kind = label
    x0, y0, x1, y1 = view[0], view[1], view[0] + view[2], view[1] + view[3]
    left, top, right, bottom = text_box (x, y, text, size, anchor)
    point = (to[-1] if kind == "note" else to) if to else (x, y)
    return (x0 <= left and right <= x1 and y0 <= top and bottom <= y1
            and x0 <= point[0] <= x1 and y0 <= point[1] <= y1)


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
    for out in range (10, int (min (total - 6, 400)), 5):
        (px, py), (dx, dy) = point_along (points, out)
        cost = out * 0.35
        # Beside the wire, or further out on a longer leader when crowded.
        for reach, extra in ((1, 0), (2.4, 30), (4, 60), (7, 90), (10, 120), (15, 150), (21, 180)):
            if abs (dx) >= abs (dy):
                spots += [(px, py - 4.5 * reach - size * 0.26, "middle", cost + extra, (px, py)),
                          (px, py + 4.5 * reach + size * 0.8, "middle", cost + extra + 1, (px, py))]
            else:
                spots += [(px + 5 * reach, py + size * 0.3, "start", cost + extra, (px, py)),
                          (px - 5 * reach, py + size * 0.3, "end", cost + extra + 1, (px, py))]
    return spots


# Spots further out round a point, each with a leader back to it, for a
# label crowded out of its own. Each step out costs: a label further from its
# part than it need be is the harder to match to it, as much as one whose
# leader crosses a wire on the way.
def ring (at, text, size, reaches=(16, 24, 34, 46, 60)):
    spots = []
    for reach in reaches:
        for step in range (16):
            angle = step * math.pi / 8
            x, y = at[0] + math.cos (angle) * reach * 1.4, at[1] + math.sin (angle) * reach
            spots.append ((x, y + size * 0.3, "middle", 40 + reach * 10, at))
    return spots


# More spots round a point for a name the ring can't place: in finer steps
# and further out, each with the name centred on it or, on the side away
# from the point, starting or ending there.
def wide_ring (at, size):
    spots = []
    for reach in FAR + (175, 200, 250):
        for step in range (24):
            angle = step * math.pi / 12
            x, y = at[0] + math.cos (angle) * reach * 1.4, at[1] + math.sin (angle) * reach
            cost = 40 + reach * 10
            spots.append ((x, y + size * 0.3, "middle", cost, at))
            if math.cos (angle) > -0.3:
                spots.append ((x, y + size * 0.3, "start", cost + 3, at))
            if math.cos (angle) < 0.3:
                spots.append ((x, y + size * 0.3, "end", cost + 3, at))
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
