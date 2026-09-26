"""Wires routed round everything on the bench, and labels set clear of it
all.

A wire runs on a grid half the breadboard's pitch, so on the board it keeps
to the streets between rows and columns of holes, where it covers none. It
turns on rounded corners, goes round parts, modules and labels, crosses
another wire only square on, and never runs along one. A* finds the
cheapest way, where length, turns, holes passed over and crossings all
cost something, so wires come out short, straight and few-cornered.

The wires share the board by negotiation: each is routed as if the others
might move, sharing a stretch of grid costs more every round, and the
rounds go on until no two wires share any, so the first wire routed can't
take the way a later one needed.

Labels are placed after the wires, each at the first of its spots that
covers no other label, wire, part, used hole or view edge, or failing that
further out on a fine leader line.
"""

import heapq
import math

STEP = 5                            # the grid, in drawing units: half the hole pitch
SQRT2 = math.sqrt (2)
DIRECTIONS = [(1, 0), (1, 1), (0, 1), (-1, 1), (-1, 0), (-1, -1), (0, -1), (1, -1)]
TURN = [0, 7, 18, 70]               # by eighths of a turn; a U-turn is never made

HOLE     = 16                       # passing over a free hole
NEAR     = 50                       # brushing a used hole's jumper end or lead
CROSS    = 55                       # crossing another wire
PARALLEL = 90                       # running alongside another wire, touching it
SLANT    = 3.5                      # a slant step costs as much as the two square ones it saves
DIAGONAL = 5                        # and more across the board, between the holes
CLEAR    = 3.2                      # how far a wire's centre keeps from anything
KEEP_OFF = 40                       # crossing a board the wire doesn't plug into
SHARE    = 40                       # sharing grid with another wire, times the round's pressure
HEURISTIC = 1.2                     # A* leans on its estimate a little, for speed


def node (point):
    return round (point[0] / STEP), round (point[1] / STEP)


def spot (node):
    return node[0] * STEP, node[1] * STEP


class Router:
    """The grid, what blocks it, what it costs to pass, and the wires on it."""

    def __init__ (self, bounds, board):
        x0, y0, x1, y1 = bounds
        self.limits = (math.floor (x0 / STEP), math.floor (y0 / STEP),
                       math.ceil (x1 / STEP), math.ceil (y1 / STEP))
        self.board = board
        self.blocked = set ()
        self.extra = {}
        self.holes = {}                 # node -> True when something is in it
        self.near = {}                  # node -> the used holes beside it
        self.occupied = {}              # node -> {wire: [(axis, straight)]} of wires there
        self.cells = {}                 # square a wire crosses on the slant -> wires
        self.history = {}               # node -> how often wires have fought over it
        self.paths = {}                 # wire -> its grid path
        self.statics = {}               # node -> its cost to any wire, None where blocked
        self.outward = []               # (box, directions) a wire may only leave a box by
        self.closes = []                # boxes where wires may run side by side

    # What is where ------------------------------------------------------

    def _nodes (self, box, grow):
        x0, y0, x1, y1 = box
        for i in range (math.ceil ((x0 - grow) / STEP), math.floor ((x1 + grow) / STEP) + 1):
            for j in range (math.ceil ((y0 - grow) / STEP), math.floor ((y1 + grow) / STEP) + 1):
                yield i, j

    def block (self, shape, grow=CLEAR):
        for n in self._nodes (bounds_of (shape), grow):
            if distance_to (shape, spot (n)) <= grow:
                self.blocked.add (n)

    def cost (self, shape, amount, grow=0.0):
        for n in self._nodes (bounds_of (shape), grow):
            if distance_to (shape, spot (n)) <= grow:
                self.extra[n] = self.extra.get (n, 0) + amount

    # Inside the box, wires move only in these directions: out of a
    # header's strip, never along it.
    def one_way (self, box, directions):
        self.outward.append ((tuple (v / STEP for v in box), set (directions)))

    def close (self, box):
        self.closes.append (tuple (v / STEP for v in box))

    # A cost for running along a box's edge, within width of it either side.
    def fringe (self, box, amount, width):
        x0, y0, x1, y1 = box
        for n in self._nodes (box, width):
            x, y = spot (n)
            outside = distance_to (("rect", *box), (x, y))
            inside = min (x - x0, x1 - x, y - y0, y1 - y)
            if outside <= width if outside > 0 else inside <= width:
                self.extra[n] = self.extra.get (n, 0) + amount

    def hole (self, point, used, near=True):
        n = node (point)
        self.holes[n] = used or self.holes.get (n, False)
        if used and near:
            for di, dj in DIRECTIONS[::2]:
                self.near.setdefault ((n[0] + di, n[1] + dj), set ()).add (n)

    # Finding a way ------------------------------------------------------

    def route (self, points, first=None, allow=(), keep_off=None, wire=None, pressure=None):
        """The cheapest way through the points in turn (the first and last are
        a wire's ends, the others waypoints), as grid nodes, and its cost.
        first limits the directions a wire may leave its start in; allow
        names nodes it may use though they are blocked, such as its own ends;
        keep_off is a box the wire would rather go round, such as a board it
        doesn't plug into. With a pressure, other wires' grid may be shared,
        at a price; without, never."""
        self.keep_off, self.wire, self.pressure = keep_off, wire, pressure
        allow = set (allow) | {points[0], points[-1]}
        own = {points[0], points[-1]}
        path, heading, total = [points[0]], None, 0.0
        last = len (points) - 2
        for index, (a, b) in enumerate (zip (points, points[1:])):
            # Slants only at the wire's own ends, never at a waypoint.
            ends = (index == 0, index == last)
            # Between waypoints in line, the wire runs straight when it can,
            # so waypoints set a wire's corners exactly.
            leg = self._straight (a, b, heading, allow, own) if last else None
            if leg:
                heading = direction (leg[0], leg[1])
                path += leg[1:]
                continue
            leg, heading, cost = self._search (a, b, heading, first if index == 0 else None, allow,
                                               own, ends, margin=40)
            if leg is None:
                leg, heading, cost = self._search (a, b, heading, first if index == 0 else None,
                                                   allow, own, ends, margin=None)
            if leg is None:
                raise ValueError (f"no way from {spot (a)} to {spot (b)}")
            path += leg[1:]
            total += cost
        return path, total

    # The straight way from a to b, when they are in line, it would not
    # turn back on itself, and nothing blocks it or runs along it.
    def _straight (self, a, b, heading, allow, own):
        if a == b or a[0] != b[0] and a[1] != b[1]:
            return None
        e = direction (a, b)
        if heading is not None and (e - heading) % 8 == 4:
            return None
        steps = max (abs (b[0] - a[0]), abs (b[1] - a[1]))
        di, dj = DIRECTIONS[e]
        nodes = [(a[0] + di * k, a[1] + dj * k) for k in range (steps + 1)]
        for n in nodes[1:]:
            if n in allow or n in own:
                continue
            if n in self.blocked or self.holes.get (n):
                return None
            if any (other != self.wire and any (axis == e % 4 or not straight
                                                for axis, straight in entries)
                    for other, entries in self.occupied.get (n, {}).items ()):
                return None
        return nodes

    # What passing a node costs whatever the wire, or None where it can't
    # go: worked out once per node.
    def _static (self, n):
        found = self.statics.get (n, False)
        if found is not False:
            return found
        if n in self.blocked or self.holes.get (n):
            cost = None
        else:
            cost = self.extra.get (n, 0) + (HOLE if n in self.holes else 0) + \
                NEAR * len (self.near.get (n, ()))
        self.statics[n] = cost
        return cost

    # A node's cost for this wire, whose own ends are no obstacle to it.
    def _own_cost (self, n, own, allow):
        if n in self.blocked and n not in allow:
            return None
        if self.holes.get (n) and n not in own and n not in allow:
            return None
        return self.extra.get (n, 0) + (HOLE if n in self.holes and n not in own else 0) + \
            NEAR * len (self.near.get (n, set ()) - own)

    def _search (self, start, goal, heading, first, allow, own, ends, margin):
        li0, lj0, li1, lj1 = self.limits
        if margin is not None:
            li0 = max (li0, min (start[0], goal[0]) - margin)
            lj0 = max (lj0, min (start[1], goal[1]) - margin)
            li1 = min (li1, max (start[0], goal[0]) + margin)
            lj1 = min (lj1, max (start[1], goal[1]) + margin)
        gi, gj = goal
        si, sj = start
        wire, pressure, history = self.wire, self.pressure, self.history
        # Other wires' grid, by node: [(axis, straight)].
        occupied = {}
        for n, by in self.occupied.items ():
            others = [entry for other, entries in by.items () if other != wire for entry in entries]
            if others:
                occupied[n] = others
        cells = {cell for cell, wires in self.cells.items () if wires - {wire}}
        off = self.keep_off and tuple (v / STEP for v in self.keep_off)
        # Nodes whose cost differs for this wire: its ends, what it may cross,
        # and the nodes beside its own holes.
        special = {}
        for n in set (allow) | own:
            special[n] = self._own_cost (n, own, allow)
            for di, dj in DIRECTIONS[::2]:
                beside = (n[0] + di, n[1] + dj)
                special[beside] = self._own_cost (beside, own, allow)
        static = self._static
        bx0, by0, bx1, by1 = (v / STEP for v in self.board)
        sharing = SHARE * (pressure or 0)
        crossing = CROSS * min (1, pressure) if pressure is not None else CROSS
        weight = HEURISTIC
        outward = self.outward

        best = {(start, heading): 0.0}
        came = {}
        queue = [(0.0, 0.0, start, heading)]
        push, pop = heapq.heappush, heapq.heappop
        while queue:
            _, g, here, d = pop (queue)
            if here == goal:
                path = [here]
                key = (here, d)
                while key in came:
                    key = came[key]
                    path.append (key[0])
                return path[::-1], d, g
            if g > best.get ((here, d), math.inf):
                continue
            hi, hj = here
            crowded = here != start and here in occupied
            near_start = ends[0] and abs (hi - si) <= 2 and abs (hj - sj) <= 2
            for e in (first if here == start and first else range (8)):
                if d is None:
                    turn = 0
                else:
                    turn = (e - d) % 8
                    turn = min (turn, 8 - turn)
                    if turn == 4 or (crowded and turn and pressure is None):
                        continue
                di, dj = DIRECTIONS[e]
                ti, tj = hi + di, hj + dj
                if not (li0 <= ti <= li1 and lj0 <= tj <= lj1):
                    continue
                there = (ti, tj)
                if outward and any (b[0] <= ti <= b[2] and b[1] <= tj <= b[3] and e not in ways
                                    for b, ways in outward):
                    continue
                base = special[there] if there in special else static (there)
                if base is None:
                    continue
                cost = TURN[turn] + base
                if off and off[0] < ti < off[2] and off[1] < tj < off[3]:
                    cost += KEEP_OFF
                if crowded and turn:
                    cost += sharing * (1 + history.get (here, 0))
                taken = occupied.get (there)
                if taken:
                    axis = e % 4
                    if there == goal or any (a == axis or not straight for a, straight in taken):
                        if pressure is None:
                            continue
                        cost += sharing * (1 + history.get (there, 0))
                    else:
                        # Crossings start cheap, so each wire first takes the
                        # way it wants and the rounds sort out who gives way.
                        cost += crossing + sharing * history.get (there, 0)
                if e % 2:
                    # Slants only fan a wire out of its pin or into its hole.
                    if not (near_start or ends[1] and abs (ti - gi) <= 2 and abs (tj - gj) <= 2):
                        continue
                    cost += STEP * SQRT2 + SLANT
                    if (min (hi, ti), min (hj, tj)) in cells:
                        cost += CROSS
                    if bx0 < ti < bx1 and by0 < tj < by1:
                        cost += DIAGONAL
                    sides = ((ti, hj), (hi, tj))
                else:
                    cost += STEP
                    sides = ((ti - dj, tj + di), (ti + dj, tj - di))
                if occupied:
                    axis = e % 4
                    near = PARALLEL
                    if any (b[0] <= ti <= b[2] and b[1] <= tj <= b[3] for b in self.closes):
                        near = PARALLEL / 10
                    for side in sides:
                        entries = occupied.get (side)
                        if entries and any (a == axis for a, _ in entries):
                            cost += near
                total = g + cost
                key = (there, e)
                if total < best.get (key, math.inf):
                    best[key] = total
                    came[key] = (here, d)
                    dx, dy = abs (ti - gi), abs (tj - gj)
                    guess = (dx + dy - 0.5858 * min (dx, dy)) * STEP * weight
                    push (queue, (total + guess, total, there, e))
        return None, heading, 0.0

    # Taking a way -------------------------------------------------------

    def claim (self, path, wire):
        """Mark a routed wire's nodes, so later wires cross it square on."""
        self.paths[wire] = path
        for index, n in enumerate (path):
            before = direction (path[index - 1], n) if index else None
            after = direction (n, path[index + 1]) if index + 1 < len (path) else None
            axes = {d % 4 for d in (before, after) if d is not None}
            straight = before is None or after is None or before == after
            self.occupied.setdefault (n, {}).setdefault (wire, []).extend (
                (axis, straight) for axis in axes)
        for a, b in zip (path, path[1:]):
            if a[0] != b[0] and a[1] != b[1]:
                self.cells.setdefault ((min (a[0], b[0]), min (a[1], b[1])), set ()).add (wire)

    def unclaim (self, wire):
        for n in self.paths.pop (wire, ()):
            self.occupied.get (n, {}).pop (wire, None)
        for wires in self.cells.values ():
            wires.discard (wire)

    # Nodes two wires share other than by crossing square on, and the nodes
    # where they cross.
    def clashes (self):
        shared, crossed = [], []
        for n, by in self.occupied.items ():
            if len (by) < 2:
                continue
            entries = [(wire, axis, straight) for wire, pairs in by.items ()
                       for axis, straight in pairs]
            if any (not straight for _, _, straight in entries) or \
                    any (a[0] != b[0] and a[1] == b[1] for a in entries for b in entries):
                shared.append (n)
            else:
                crossed.append (n)
        return shared, crossed

    # What the wires cost as they lie, without the rounds' pressure: the sum
    # _search would give each, so rounds can be compared.
    def price (self):
        total = 0.0
        for wire, path in self.paths.items ():
            own = {path[0], path[-1]}
            heading = None
            for here, there in zip (path, path[1:]):
                e = direction (here, there)
                if heading is not None:
                    total += TURN[min ((e - heading) % 8, (heading - e) % 8)]
                heading = e
                total += (STEP * SQRT2 + SLANT if e % 2 else STEP) + self.extra.get (there, 0)
                if there in self.holes and there not in own:
                    total += HOLE
                total += NEAR * len (self.near.get (there, set ()) - own)
                if any (other != wire for other in self.occupied.get (there, {})):
                    total += CROSS
        return total

def direction (a, b):
    di, dj = b[0] - a[0], b[1] - a[1]
    di, dj = (di > 0) - (di < 0), (dj > 0) - (dj < 0)
    return DIRECTIONS.index ((di, dj)) if (di, dj) != (0, 0) else None


# A grid path as the points a wire is drawn through: its corners.
def corners (path):
    points = [spot (path[0])]
    for a, b, c in zip (path, path[1:], path[2:]):
        if direction (a, b) != direction (b, c):
            points.append (spot (b))
    points.append (spot (path[-1]))
    return points


# Shapes -------------------------------------------------------------------
#
# ("rect", x0, y0, x1, y1), ("circle", cx, cy, r) and ("segment", ax, ay, bx,
# by, half width), in drawing units.

def bounds_of (shape):
    kind = shape[0]
    if kind == "rect":
        return shape[1:5]
    if kind == "circle":
        _, cx, cy, r = shape
        return cx - r, cy - r, cx + r, cy + r
    _, ax, ay, bx, by, half = shape
    return min (ax, bx) - half, min (ay, by) - half, max (ax, bx) + half, max (ay, by) + half


# How far a point is outside a shape, 0 inside it.
def distance_to (shape, point):
    x, y = point
    kind = shape[0]
    if kind == "rect":
        _, x0, y0, x1, y1 = shape
        dx = max (x0 - x, 0, x - x1)
        dy = max (y0 - y, 0, y - y1)
        return math.hypot (dx, dy)
    if kind == "circle":
        _, cx, cy, r = shape
        return max (0.0, math.hypot (x - cx, y - cy) - r)
    _, ax, ay, bx, by, half = shape
    return max (0.0, segment_distance ((ax, ay), (bx, by), point) - half)


def segment_distance (a, b, p):
    dx, dy = b[0] - a[0], b[1] - a[1]
    length = dx * dx + dy * dy
    t = 0.0 if not length else max (0.0, min (1.0, ((p[0] - a[0]) * dx + (p[1] - a[1]) * dy) / length))
    return math.hypot (p[0] - a[0] - t * dx, p[1] - a[1] - t * dy)


# Whether a segment passes within reach of a box.
def segment_meets_box (a, b, box, reach):
    x0, y0, x1, y1 = box[0] - reach, box[1] - reach, box[2] + reach, box[3] + reach
    if max (a[0], b[0]) < x0 or min (a[0], b[0]) > x1 or max (a[1], b[1]) < y0 \
            or min (a[1], b[1]) > y1:
        return False
    dx, dy = b[0] - a[0], b[1] - a[1]
    low, high = 0.0, 1.0
    for p, q in ((-dx, a[0] - x0), (dx, x1 - a[0]), (-dy, a[1] - y0), (dy, y1 - a[1])):
        if abs (p) < 1e-9:
            if q < 0:
                return False
        else:
            t = q / p
            if p < 0:
                low = max (low, t)
            else:
                high = min (high, t)
            if low > high:
                return False
    return True


def shape_crosses_segment (shape, a, b):
    if shape[0] == "segment":
        _, ax, ay, bx, by, half = shape
        return segments_meet ((ax, ay), (bx, by), a, b, half)
    return segment_meets_box (a, b, bounds_of (shape), 0)


def segments_meet (p, q, a, b, reach):
    steps = max (2, int (math.dist (a, b) / 2))
    return any (segment_distance (p, q, (a[0] + (b[0] - a[0]) * k / steps,
                                         a[1] + (b[1] - a[1]) * k / steps)) < reach + 0.5
                for k in range (steps + 1))


def shape_meets_box (shape, box, reach=0.0):
    kind = shape[0]
    if kind == "segment":
        _, ax, ay, bx, by, half = shape
        return segment_meets_box ((ax, ay), (bx, by), box, half + reach)
    sx0, sy0, sx1, sy1 = bounds_of (shape)
    if sx1 + reach < box[0] or sx0 - reach > box[2] or sy1 + reach < box[1] or sy0 - reach > box[3]:
        return False
    if kind == "rect":
        return True
    _, cx, cy, r = shape
    nx, ny = min (max (cx, box[0]), box[2]), min (max (cy, box[1]), box[3])
    return math.hypot (cx - nx, cy - ny) < r + reach


# Labels -------------------------------------------------------------------

HARD = 10000


def text_box (x, y, text, size, anchor="middle"):
    width = text_width (text, size)
    left = {"start": x, "end": x - width}.get (anchor, x - width / 2)
    return left - 1, y - size * 0.8, left + width + 1, y + size * 0.26


def text_width (text, size):
    wide = sum (1 for c in text if c in "mwMWΩ%")
    narrow = sum (1 for c in text if c in " .,:;'ilIj1()−-")
    return (len (text) - wide - narrow) * size * 0.58 + wide * size * 0.82 + narrow * size * 0.32 + 1


class Placer:
    """Everything labels keep clear of, and the labels placed so far."""

    def __init__ (self, view=None):
        self.view = view                # (x0, y0, x1, y1) a label must stay inside
        self.soft_view = None           # a label outside it costs a little
        self.labels = []                # placed boxes
        self.shapes = []                # (shape, cost) of things to keep clear of
        self.points = {}                # (x, y) -> cost of covering it: holes, print

    def avoid (self, shape, cost=HARD):
        self.shapes.append ((shape, cost))

    def point (self, xy, cost):
        self.points[(round (xy[0]), round (xy[1]))] = max (cost, self.points.get (
            (round (xy[0]), round (xy[1])), 0))

    def taken (self, box):
        self.labels.append (box)

    def score (self, box, mine=()):
        cost = 0.0
        if self.view:
            vx0, vy0, vx1, vy1 = self.view
            if box[0] < vx0 + 2 or box[2] > vx1 - 2 or box[1] < vy0 + 2 or box[3] > vy1 - 2:
                cost += HARD
        if self.soft_view:
            vx0, vy0, vx1, vy1 = self.soft_view
            if box[0] < vx0 or box[2] > vx1 or box[1] < vy0 or box[3] > vy1:
                cost += 30
        for other in self.labels:
            if box[0] < other[2] + 1.5 and other[0] < box[2] + 1.5 and box[1] < other[3] + 1 \
                    and other[1] < box[3] + 1:
                cost += HARD
        for shape, amount in self.shapes:
            if shape not in mine and shape_meets_box (shape, box, 0.8):
                cost += amount
        for (x, y), amount in self.points.items ():
            if box[0] - 1.5 < x < box[2] + 1.5 and box[1] - 1.5 < y < box[3] + 1.5:
                cost += amount
        return cost

    # The best of the spots, each (x, y, anchor, cost), or with a leader
    # to a point (x, y, anchor, cost, point), which must not cross a wire
    # or a part on its way, but may cross its own part. A label may touch
    # the holes of its own part's legs, mine, which it names.
    def place (self, text, size, spots, own=(), mine=()):
        best = None
        for spot in spots:
            x, y, anchor, extra = spot[:4]
            box = text_box (x, y, text, size, anchor)
            total = self.score (box, mine) + extra
            if len (spot) > 4 and spot[4] and (best is None or total < best[0]):
                total += self.leader_cost (box, spot[4], own)
            if best is None or total < best[0]:
                best = (total, x, y, anchor, box)
        return best

    def leader_cost (self, box, to, own=()):
        start = (min (max (to[0], box[0]), box[2]), min (max (to[1], box[1]), box[3]))
        if math.dist (start, to) < 6:
            return 0
        tip = (to[0] + (start[0] - to[0]) * 3 / math.dist (start, to),
               to[1] + (start[1] - to[1]) * 3 / math.dist (start, to))
        cost = 0
        for shape, amount in self.shapes:
            if amount >= HARD and shape not in own and shape_crosses_segment (shape, start, tip):
                cost += 500
        for other in self.labels:
            if segment_meets_box (start, tip, other, 0.5):
                cost += 500
        return cost
