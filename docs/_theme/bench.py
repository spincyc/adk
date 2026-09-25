"""The bench: an Arduino Mega 2560 beside a breadboard, with parts standing in
real holes and jumper wires from real header pins.

A lesson describes its build once, in circuit.py beside its page:

    bench = Bench ("An LED on pin 26", columns=(1, 30))
    bench.wire ("26", "a10")
    bench.resistor ("220 Ω", "b10", "b14")
    bench.led ("red", anode="c14", cathode="c15")
    bench.wire ("a15", "B-15")
    bench.wire ("GND", "B-9")

From that one description come the pencil drawings, the build steps and the
table of connections, and signal_pins () lets the site check the sketch
declares exactly the pins the build wires. Geometry is in inches on the real
0.1-inch grid: holes, header pins and leg spacing are where they really are.
"""

import re

from pencil import Pencil

DPI = 100                           # drawing units per inch
MEGA_WIDTH, MEGA_HEIGHT = 4.0, 2.1
BOARD_HEIGHT = 2.2
GAP = 0.8                           # between the Mega and the breadboard
MARGIN = 0.35

WIRE_COLORS = {
    "red": "#c8423b", "black": "#3b3b3b", "blue": "#3f6fb5", "green": "#4c8a4a",
    "yellow": "#d8b32e", "orange": "#dc7c2e", "white": "#ebe7dc", "purple": "#7d5aa6",
    "brown": "#8a5a36", "grey": "#9a9a9a",
}
SIGNAL_COLORS = ["yellow", "green", "blue", "orange", "purple", "white", "brown", "grey"]
ROWS = {"j": 0.55, "i": 0.65, "h": 0.75, "g": 0.85, "f": 0.95,
        "e": 1.25, "d": 1.35, "c": 1.45, "b": 1.55, "a": 1.65,
        "T+": 0.15, "T-": 0.25, "B+": 1.95, "B-": 2.05}


# The Mega's header pins as name -> (x, y) in inches from the bottom-left
# corner, USB socket on the left. Pins that repeat (GND, 5V) get numbered
# names; canonical () turns them back.
def mega_pins ():
    pins = {}
    top = ["SCL", "SDA", "AREF", "GND"] + [str (n) for n in range (13, 7, -1)]
    for index, name in enumerate (top):
        pins[name] = (0.84 + 0.1 * index, 2.0)
    for index, n in enumerate (range (7, -1, -1)):
        pins[str (n)] = (1.90 + 0.1 * index, 2.0)
    for index, n in enumerate (range (14, 22)):
        pins[str (n)] = (2.80 + 0.1 * index, 2.0)
    for index, name in enumerate (["IOREF", "RESET", "3.3V", "5V", "GND2", "GND3", "VIN"]):
        pins[name] = (1.04 + 0.1 * index, 0.1)
    for n in range (8):
        pins[f"A{n}"] = (1.84 + 0.1 * n, 0.1)
    for n in range (8, 16):
        pins[f"A{n}"] = (2.74 + 0.1 * (n - 8), 0.1)
    rows = [("5V2", "5V3")] + [(str (n), str (n + 1)) for n in range (22, 53, 2)]
    rows += [("GND4", "GND5")]
    for index, (even, odd) in enumerate (rows):
        pins[even] = (3.70, 1.95 - 0.1 * index)
        pins[odd] = (3.80, 1.95 - 0.1 * index)
    return pins


ALIASES = {
    "GND": ["GND", "GND2", "GND3", "GND4", "GND5"],
    "5V":  ["5V", "5V2", "5V3"],
}


class Bench:
    def __init__ (self, title, columns=(1, 30), seed=1):
        self.title = title
        self.first, self.last = columns
        self.seed = seed
        self.pins = mega_pins ()
        self.taken = set ()
        self.parts = []
        self.wires = []
        self.steps = []
        self.palette = iter (SIGNAL_COLORS * 8)

    # Where things are ---------------------------------------------------

    def board_width (self):
        return 0.5 + (self.last - self.first) * 0.1

    def mega_origin (self):
        return MARGIN, MARGIN + 0.25

    def board_origin (self):
        mx, my = self.mega_origin ()
        return mx + MEGA_WIDTH + GAP, my + (MEGA_HEIGHT - BOARD_HEIGHT) / 2

    def size (self):
        bx, by = self.board_origin ()
        return (bx + self.board_width () + MARGIN) * DPI, (by + BOARD_HEIGHT + MARGIN) * DPI

    def pin_xy (self, name):
        mx, my = self.mega_origin ()
        x, y = self.pins[name]
        return (mx + x) * DPI, (my + MEGA_HEIGHT - y) * DPI

    def hole_xy (self, hole):
        bx, by = self.board_origin ()
        _, column, row = parse_hole (hole)
        if not self.first <= column <= self.last:
            raise ValueError (f"{hole} is outside the drawn columns {self.first}-{self.last}")
        return (bx + 0.25 + (column - self.first) * 0.1) * DPI, (by + ROWS[row]) * DPI

    def strip_of (self, hole):
        kind, column, row = parse_hole (hole)
        if kind == "rail":
            return f"rail {row}"
        return f"column {column} {'a-e' if row in 'abcde' else 'f-j'}"

    # Building -----------------------------------------------------------

    def wire (self, start, end, color=None):
        # Resolve a hole before a shared pin name, so GND or 5V can be the
        # header pin nearest the hole.
        if start in ALIASES:
            end_end = self._resolve (end, None)
            start_end = self._resolve (start, self._xy (end_end))
        else:
            start_end = self._resolve (start, None)
            end_end = self._resolve (end, self._xy (start_end))
        if color is None:
            color = self._color ({start, end})
        self.wires.append ((start_end, end_end, color))
        self.steps.append (f"A {color} wire from {self.describe (start_end)} "
                           f"to {self.describe (end_end)}.")
        return self

    def resistor (self, value, a, b):
        self.parts.append (Resistor (value, a, b))
        self.steps.append (f"The {value} resistor ({' – '.join (bands_for (value)[:3])}) from {a} to {b}.")
        return self

    def led (self, color, anode, cathode):
        self.parts.append (Led (color, anode, cathode))
        self.steps.append (f"The {color} LED: long leg in {anode}, short leg in {cathode}.")
        return self

    def button (self, column):
        part = Button (column)
        self.parts.append (part)
        self.steps.append (f"A push button across the middle gap, legs in "
                           f"{', '.join (part.holes ())}.")
        return self

    def _color (self, names):
        if names & {"GND"} or any (n.startswith (("B-", "T-")) for n in names):
            return "black"
        if names & {"5V", "3.3V"} or any (n.startswith (("B+", "T+")) for n in names):
            return "red"
        return next (self.palette)

    # A pin name or a hole. A shared name (GND, 5V) picks the free header pin
    # nearest the wire's other end.
    def _resolve (self, name, toward):
        if name in ALIASES:
            free = [p for p in ALIASES[name] if p not in self.taken]
            if not free:
                raise ValueError (f"no free {name} pin")
            if toward:
                free.sort (key=lambda p: distance (self.pin_xy (p), toward))
            self.taken.add (free[0])
            return ("pin", free[0])
        if name in self.pins:
            if name in self.taken:
                raise ValueError (f"pin {name} already has a wire")
            self.taken.add (name)
            return ("pin", name)
        parse_hole (name)
        return ("hole", name)

    # Where to find a pin or a hole, in words.
    def describe (self, end):
        kind, name = end
        if kind == "hole":
            _, column, row = parse_hole (name)
            if row in ROWS and len (row) == 2:
                side = "top" if row[0] == "T" else "bottom"
                sign = "+" if row[1] == "+" else "−"
                return f"the {side} {sign} rail ({name})"
            return name
        label = canonical (name)
        x, y = self.pins[name]
        if label[0].isdigit () or label[0] == "A":
            return f"pin {label}"
        if 0.11 < y < 1.99:
            where = "at the end of the long header" if y < 1 else "at the top of the long header"
        elif y > 1.99:
            where = "beside pin 13"
        else:
            where = "on the power header"
        return f"a {label} pin {where}"

    def _xy (self, end):
        kind, name = end
        return self.pin_xy (name) if kind == "pin" else self.hole_xy (name)

    # Connections --------------------------------------------------------

    def nets (self):
        parent = {}

        def find (node):
            parent.setdefault (node, node)
            while parent[node] != node:
                parent[node] = parent[parent[node]]
                node = parent[node]
            return node

        def join (a, b):
            parent[find (a)] = find (b)

        def node (end):
            kind, name = end
            return "pin " + canonical (name) if kind == "pin" else self.strip_of (name)

        for start, end, _ in self.wires:
            join (node (start), node (end))
        for part in self.parts:
            for leg, hole in part.legs ():
                join (f"{part.name}: {leg}", self.strip_of (hole))
            for a, b in part.inside ():
                join (f"{part.name}: {a}", f"{part.name}: {b}")

        groups = {}
        for member in list (parent):
            groups.setdefault (find (member), set ()).add (member)
        return list (groups.values ())

    # The circuit point by point: every junction that joins two or more
    # things, in the order current meets them walking out from each pin.
    def connections (self):
        nets = [net for net in self.nets ()
                if len ([m for m in net if not m.startswith (("column ", "rail "))]) >= 2]
        part_of = lambda member: member.split (": ")[0]
        order = []
        queue = sorted ((net for net in nets if any (m.startswith ("pin ") for m in net)),
                        key=lambda net: (any (m in ("pin GND", "pin 5V") for m in net),
                                         min (pin_order (m[4:]) for m in net if m.startswith ("pin "))))
        while queue:
            net = queue.pop (0)
            if net in order:
                continue
            order.append (net)
            parts = {part_of (m) for m in net if ": " in m}
            following = [other for other in nets if other not in order and other not in queue
                         and parts & {part_of (m) for m in other if ": " in m}]
            queue = following + queue
        order += [net for net in nets if net not in order]

        rows = []
        for net in order:
            pins = sorted ({m[4:] for m in net if m.startswith ("pin ")}, key=pin_order)
            legs = sorted (m for m in net if not m.startswith (("pin ", "column ", "rail ")))
            rows.append (([f"pin {p}" if p[0].isdigit () or p[0] == "A" else p for p in pins], legs))
        return rows

    def signal_pins (self):
        return {canonical (name) for (kind, name), _, _ in self.wires if kind == "pin"
                if canonical (name) not in ("GND", "5V", "3.3V", "VIN")} | \
               {canonical (name) for _, (kind, name), _ in self.wires if kind == "pin"
                if canonical (name) not in ("GND", "5V", "3.3V", "VIN")}

    # Drawing ------------------------------------------------------------

    # The whole bench, or a close-up of the breadboard where the parts are.
    def svg (self, view="bench", prefix="bench"):
        pencil = Pencil (self.seed, prefix)
        self._draw_mega (pencil)
        self._draw_board (pencil, detail=self._closeup_columns () if view == "closeup" else None)
        for part in self.parts:
            part.draw (pencil, self)
        for start, end, color in self.wires:
            self._draw_wire (pencil, start, end, color)
        width, height = self.size ()
        box = (0, 0, width, height) if view == "bench" else self._closeup_box ()
        return pencil.svg (box, self.title if view == "bench" else f"{self.title}: close-up")

    def _closeup_columns (self):
        columns = []
        for part in self.parts:
            columns += [parse_hole (hole)[1] for _, hole in part.legs ()]
        for start, end, _ in self.wires:
            columns += [parse_hole (name)[1] for kind, name in (start, end) if kind == "hole"]
        return max (self.first, min (columns) - 5), min (self.last, max (columns) + 5)

    # The close-up keeps the columns and rows the build uses, with room for
    # standing parts, labels and the column numbers.
    def _closeup_box (self):
        low, high = self._closeup_columns ()
        left, _ = self.hole_xy (f"a{low}")
        right, _ = self.hole_xy (f"a{high}")
        _, by = self.board_origin ()
        ys = []
        for part in self.parts:
            ys += [self.hole_xy (hole)[1] for _, hole in part.legs ()]
        for start, end, _ in self.wires:
            ys += [self.hole_xy (name)[1] for kind, name in (start, end) if kind == "hole"]
        top = max (by * DPI - 30, min (ys) - 0.55 * DPI)
        bottom = min ((by + BOARD_HEIGHT) * DPI + 15, max (ys) + 0.35 * DPI)
        # Keep the nearer row of column numbers in view.
        top = min (top, (by + 0.34) * DPI) if min (ys) < (by + 1.1) * DPI else top
        bottom = max (bottom, (by + 1.88) * DPI) if max (ys) > (by + 1.1) * DPI else bottom
        return (left - 28, top, right - left + 50, bottom - top)

    def _draw_wire (self, pencil, start, end, color):
        a, b = self._xy (start), self._xy (end)
        lift = 0.3 * DPI
        if start[0] == "pin" or end[0] == "pin":
            if end[0] == "pin":
                start, end, a, b = end, start, b, a
            route = [a, self._leave (start[1], a, lift), (b[0] - lift * 0.5, b[1] - lift), b]
        else:
            # A short jumper between holes bows out to one side.
            dx, dy = b[0] - a[0], b[1] - a[1]
            length = max (1.0, (dx * dx + dy * dy) ** 0.5)
            bow = min (0.12 * DPI, length * 0.35)
            middle = ((a[0] + b[0]) / 2 + dy / length * bow, (a[1] + b[1]) / 2 - dx / length * bow)
            route = [a, middle, b]
        pencil.wire (route, WIRE_COLORS[color])
        for x, y in (a, b):
            pencil.pin_end (x, y)
        if start[0] == "pin" and end[0] == "hole":
            label = canonical (start[1])
            label = f"pin {label}" if label[0].isdigit () or label[0] == "A" else label
            x, y = route[2]
            pencil.label (x - 6, y - 6, label)

    # A point a little way out from a header, so a wire leaves it cleanly.
    def _leave (self, name, xy, lift):
        _, y = self.pins[name]
        if y > 1.99:
            return xy[0], xy[1] - lift
        if y < 0.11:
            return xy[0], xy[1] + lift
        return xy[0] + lift * 1.4, xy[1]

    def _draw_mega (self, pencil):
        mx, my = self.mega_origin ()
        x, y, w, h = mx * DPI, my * DPI, MEGA_WIDTH * DPI, MEGA_HEIGHT * DPI
        pencil.paper_fill ([(x, y), (x + w, y), (x + w, y + h), (x, y + h)], "#d9e3e6")
        pencil.rect (x, y, w, h, width=1.7, radius=10)
        pencil.hatch (x + 6, y + 6, w - 12, h - 12, gap=6, angle=35, tone=0.14)
        for hx, hy in ((0.55, 0.1), (0.6, 2.0), (3.3, 0.1), (3.95, 1.55)):
            pencil.circle (x + hx * DPI, y + (MEGA_HEIGHT - hy) * DPI, 5, width=1.0, passes=1)

        # The USB socket and the power jack stick out of the left edge.
        pencil.rect (x - 0.25 * DPI, y + 0.28 * DPI, 0.85 * DPI, 0.5 * DPI, width=1.4)
        pencil.hatch (x - 0.25 * DPI, y + 0.28 * DPI, 0.85 * DPI, 0.5 * DPI, gap=2.6, angle=90,
                      tone=0.35)
        pencil.text (x + 0.17 * DPI, y + 0.2 * DPI, "USB", size=12, kind="note")
        pencil.rect (x - 0.12 * DPI, y + 1.55 * DPI, 0.7 * DPI, 0.42 * DPI, width=1.4, radius=4)
        pencil.hatch (x - 0.12 * DPI, y + 1.55 * DPI, 0.7 * DPI, 0.42 * DPI, gap=2.2, angle=0,
                      tone=0.5)

        # The microcontroller sits turned 45 degrees, as on the real board.
        cx, cy, r = x + 2.4 * DPI, y + 1.05 * DPI, 0.36 * DPI
        diamond = [(cx, cy - r), (cx + r, cy), (cx, cy + r), (cx - r, cy)]
        pencil.fill (diamond, tone=0.62)
        pencil.polyline (diamond, width=1.3, closed=True)
        pencil.text (x + 1.25 * DPI, y + 1.12 * DPI, "ARDUINO", size=15, weight="bold",
                     tone=0.7, kind="silk")
        pencil.text (x + 1.25 * DPI, y + 1.3 * DPI, "MEGA 2560", size=15, weight="bold",
                     tone=0.7, kind="silk")

        # The built-in LED, L, on pin 13.
        lx, ly = x + 1.2 * DPI, y + 0.45 * DPI
        pencil.rect (lx - 4, ly - 2.5, 8, 5, width=0.9, passes=1)
        pencil.text (lx + 10, ly + 4, "L", size=9, anchor="start", kind="silk")

        self._draw_headers (pencil)

    def _draw_headers (self, pencil):
        groups = {}
        for name, (px, py) in self.pins.items ():
            key = "top" if py > 1.99 else "bottom" if py < 0.11 else "side"
            gap_key = (key, round (px, 1) if key == "side" else block_of (px, key))
            groups.setdefault (gap_key, []).append (name)
        for key, names in groups.items ():
            points = [self.pin_xy (n) for n in names]
            left = min (p[0] for p in points) - 5.5
            top = min (p[1] for p in points) - 5.5
            right = max (p[0] for p in points) + 5.5
            bottom = max (p[1] for p in points) + 5.5
            if key[0] == "side":
                continue
            pencil.fill ([(left, top), (right, top), (right, bottom), (left, bottom)], tone=0.78)
            pencil.rect (left, top, right - left, bottom - top, width=1.1, passes=1)
        side = [self.pin_xy (n) for n, (px, py) in self.pins.items () if 0.11 < py < 1.99]
        left = min (p[0] for p in side) - 5.5
        top = min (p[1] for p in side) - 5.5
        right = max (p[0] for p in side) + 5.5
        bottom = max (p[1] for p in side) + 5.5
        pencil.fill ([(left, top), (right, top), (right, bottom), (left, bottom)], tone=0.78)
        pencil.rect (left, top, right - left, bottom - top, width=1.1, passes=1)

        for name, (px, py) in self.pins.items ():
            hx, hy = self.pin_xy (name)
            pencil.socket (hx, hy)
            label = canonical (name)
            if py > 1.99:
                pencil.text (hx + 3.5, hy + 11, label, size=9, rotate=-90, anchor="end",
                             kind="silk")
            elif py < 0.11:
                pencil.text (hx + 3.5, hy - 11, label, size=9, rotate=-90, anchor="start",
                             kind="silk")
            elif px < 3.75:
                pencil.text (hx - 9, hy + 3.5, label, size=9, anchor="end", kind="silk")
            else:
                pencil.text (hx + 9, hy + 3.5, label, size=9, anchor="start", kind="silk")

    def _draw_board (self, pencil, detail=None):
        bx, by = self.board_origin ()
        x, y = bx * DPI, by * DPI
        w, h = self.board_width () * DPI, BOARD_HEIGHT * DPI
        torn_left, torn_right = self.first > 1, self.last < 63
        outline = board_outline (x, y, w, h, torn_left, torn_right, pencil.random)
        pencil.paper_fill (outline, "#ffffff")
        pencil.polyline (outline, width=1.5, closed=True)
        # The channel down the middle, where chips straddle.
        pencil.fill ([(x + 6, y + 1.05 * DPI), (x + w - 6, y + 1.05 * DPI),
                      (x + w - 6, y + 1.15 * DPI), (x + 6, y + 1.15 * DPI)], tone=0.1)
        for rail, offset in (("T+", 0.06), ("T-", 0.34), ("B+", 1.86), ("B-", 2.14)):
            color = "#c8423b" if rail.endswith ("+") else "#3f6fb5"
            pencil.stripe ((x + 14, y + offset * DPI), (x + w - 14, y + offset * DPI), color)
            sign = "+" if rail.endswith ("+") else "−"
            hy = (by + ROWS[rail]) * DPI
            if not torn_left:
                pencil.text (x + 9, hy + 4, sign, size=12, kind="silk")
            if not torn_right:
                pencil.text (x + w - 9, hy + 4, sign, size=12, kind="silk")
        for column in range (self.first, self.last + 1):
            for row in "abcdefghij":
                pencil.hole (*self.hole_xy (f"{row}{column}"))
            if rail_column (column):
                for rail in ("T+", "T-", "B+", "B-"):
                    pencil.hole (*self.hole_xy (f"{rail}{column}"))
            numbered = detail and detail[0] <= column <= detail[1]
            if column % 5 == 0 or column == 1 or numbered:
                hx, _ = self.hole_xy (f"a{column}")
                size = 6 if numbered and column % 5 else 8
                pencil.text (hx, y + 0.44 * DPI, str (column), size=size, tone=0.75, kind="silk")
                pencil.text (hx, y + 1.82 * DPI, str (column), size=size, tone=0.75, kind="silk")
        for row in "abcdefghij":
            hx, hy = self.hole_xy (f"{row}{self.first}")
            pencil.text (hx - 11, hy + 3, row, size=7, tone=0.75, kind="silk")
        if detail:
            # A ruler down the close-up's left edge, so the row letters sit
            # clear of the holes.
            hx, _ = self.hole_xy (f"a{detail[0]}")
            left = hx - 28
            pencil.ruler (left, y - 30, 20, h + 60, [(row, self.hole_xy (f"{row}{detail[0]}")[1])
                                                    for row in "abcdefghij"])


class Resistor:
    TINTS = {"black": "#333333", "brown": "#8a5a36", "red": "#c8423b", "orange": "#dc7c2e",
             "yellow": "#d8b32e", "green": "#4c8a4a", "blue": "#3f6fb5", "violet": "#7d5aa6",
             "grey": "#9a9a9a", "white": "#f4f1e8", "gold": "#b8963a"}

    def __init__ (self, value, a, b):
        self.value, self.a, self.b = value, a, b
        self.name = f"{value} resistor"

    def legs (self):
        return [("one end", self.a), ("other end", self.b)]

    def inside (self):
        return []

    def draw (self, pencil, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (self.a), bench.hole_xy (self.b)
        cx, cy = (x1 + x2) / 2, (y1 + y2) / 2 - 7
        length, height = 27, 10
        pencil.lead ((x1, y1), (cx - length / 2, cy))
        pencil.lead ((cx + length / 2, cy), (x2, y2))
        pencil.body ((cx - length / 2, cy - height / 2, length, height), "#e9d7ae", radius=4.5)
        for index, band in enumerate (bands_for (self.value)):
            bx = cx - length / 2 + 5 + index * 4.6 + (3 if index == 3 else 0)
            pencil.band (bx, cy - height / 2, 2.5, height, self.TINTS[band])
        pencil.rect (cx - length / 2, cy - height / 2, length, height, width=1.0, radius=4.5,
                     layer="top")
        pencil.text (cx, cy - 11, self.value, size=14, kind="note")


class Led:
    TINTS = {"red": "#ea8b84", "yellow": "#f1d97e", "green": "#94cc90", "blue": "#93b5e6",
             "white": "#f7f5ee"}

    def __init__ (self, color, anode, cathode):
        self.color, self.anode, self.cathode = color, anode, cathode
        self.name = f"{color} LED"

    def legs (self):
        return [("long leg (+)", self.anode), ("short leg (−)", self.cathode)]

    def inside (self):
        return []

    def draw (self, pencil, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (self.anode), bench.hole_xy (self.cathode)
        cx, cy = (x1 + x2) / 2, min (y1, y2) - 20
        pencil.lead ((x1, y1), (cx - 4, cy + 8))
        pencil.lead ((x2, y2), (cx + 4, cy + 8))
        pencil.dome (cx, cy, 11, self.TINTS[self.color])
        # The flat edge of the rim is on the short leg's side.
        side = 1 if x2 > x1 else -1
        pencil.line ((cx + side * 11.5, cy - 6), (cx + side * 11.5, cy + 6), width=1.6,
                     layer="top")
        pencil.text (x1 - 6, y1 + 16, "+", size=13, kind="note")
        pencil.text (cx, cy - 17, f"{self.color} LED", size=14, kind="note")


class Button:
    # A 6 mm push button across the middle gap, its legs in e and f of two
    # columns two apart. The two legs in each column are joined inside;
    # pressing joins the columns.
    def __init__ (self, column):
        self.column = column
        self.name = f"button at column {column}"

    def holes (self):
        c = self.column
        return [f"f{c}", f"f{c + 2}", f"e{c}", f"e{c + 2}"]

    def legs (self):
        c = self.column
        return [("left leg", f"e{c}"), ("left leg ", f"f{c}"),
                ("right leg", f"e{c + 2}"), ("right leg ", f"f{c + 2}")]

    def inside (self):
        return [("left leg", "left leg "), ("right leg", "right leg ")]

    def draw (self, pencil, bench):
        (x1, y1), (x2, y2) = bench.hole_xy (f"f{self.column}"), bench.hole_xy (f"e{self.column + 2}")
        pencil.body ((x1 - 5, y1 - 5, x2 - x1 + 10, y2 - y1 + 10), "#dedad0", radius=2)
        pencil.rect (x1 - 5, y1 - 5, x2 - x1 + 10, y2 - y1 + 10, width=1.3, layer="top")
        pencil.dome ((x1 + x2) / 2, (y1 + y2) / 2, 7, "#6b6b6b")


def board_outline (x, y, w, h, torn_left, torn_right, random):
    def edge (x0, top_to_bottom):
        points = []
        steps = 14
        for index in range (steps + 1):
            t = index / steps
            jag = random.uniform (-5, 5) if 0 < index < steps else 0
            points.append ((x0 + jag, y + h * (t if top_to_bottom else 1 - t)))
        return points

    right = edge (x + w, True) if torn_right else [(x + w, y), (x + w, y + h)]
    left = edge (x, False) if torn_left else [(x, y + h), (x, y)]
    return [(x, y)] + right + left[:-1] if not torn_left else right + left


def block_of (px, key):
    # Which physical header block a top or bottom pin sits in.
    if key == "top":
        return 0 if px < 1.8 else 1 if px < 2.7 else 2
    return 0 if px < 1.8 else 1 if px < 2.65 else 2


def parse_hole (hole):
    match = re.fullmatch (r"([a-j])(\d{1,2})", hole)
    if match and 1 <= int (match.group (2)) <= 63:
        return "main", int (match.group (2)), match.group (1)
    match = re.fullmatch (r"([TB][+-])(\d{1,2})", hole)
    if match:
        column = int (match.group (2))
        if not rail_column (column):
            raise ValueError (f"rail {match.group (1)} has no hole at column {column}")
        return "rail", column, match.group (1)
    raise ValueError (f"{hole!r} is neither a Mega pin nor a breadboard hole")


def rail_column (column):
    # The rails' holes come in fives with a gap, from column 3 to 61.
    return 3 <= column <= 61 and (column - 3) % 6 != 5


def canonical (name):
    return re.sub (r"^(GND|5V)\d$", r"\1", name)


def distance (a, b):
    return ((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2) ** 0.5


def pin_order (name):
    if name.isdigit ():
        return (0, int (name))
    if re.fullmatch (r"A\d+", name):
        return (1, int (name[1:]))
    return (2, name)


def bands_for (value):
    # "220 Ω" is red, red, brown and a gold tolerance band.
    names = ["black", "brown", "red", "orange", "yellow", "green", "blue", "violet", "grey",
             "white"]
    match = re.fullmatch (r"([\d.]+)\s*(k|M)?\s*Ω", value)
    ohms = float (match.group (1)) * {None: 1, "k": 1e3, "M": 1e6}[match.group (2)]
    digits = f"{ohms:.0f}"
    return [names[int (digits[0])], names[int (digits[1])], names[len (digits) - 2], "gold"]
