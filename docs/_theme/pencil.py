"""A pencil for SVG, in the manner of a careful engineer's sketch: fine,
near-straight graphite lines with only a hint of hand, light hatching for
shade, a little restrained colour, and small printed labels that keep a
halo of paper so they read over anything. The same seed always draws the
same picture, so a drawing only changes when its circuit does.
"""

import math
import random
from xml.sax.saxutils import escape

GRAPHITE = "#2b2b2b"
PAPER    = "#fbf9f3"
DPI      = 100                      # drawing units per inch
WOBBLE   = 0.15                     # how far a hand strays, in drawing units


class Pencil:
    def __init__ (self, seed, prefix="pencil"):
        self.random = random.Random (seed)
        self.prefix = prefix
        self.layers = {name: [] for name in ("paper", "crisp", "shade", "ink", "wire", "top", "text")}
        self.turns = []
        self.mono = False

    def id (self, name):
        return f"{self.prefix}-{name}"

    # Strokes ------------------------------------------------------------
    #
    # One pass for most lines; major outlines ask for two.

    def line (self, a, b, width=0.8, tone=0.85, layer="ink", passes=1, wobble=WOBBLE):
        for _ in range (passes):
            self._stroke ([a, b], width, tone, layer, wobble, closed=False)

    def polyline (self, points, width=0.8, tone=0.85, layer="ink", closed=False, passes=1,
                  wobble=WOBBLE):
        for _ in range (passes):
            self._stroke (points, width, tone, layer, wobble, closed)

    def rect (self, x, y, w, h, width=0.8, tone=0.85, layer="ink", passes=1, radius=0):
        points = rounded_rectangle (x, y, w, h, radius) if radius else \
            [(x, y), (x + w, y), (x + w, y + h), (x, y + h)]
        self.polyline (points, width, tone, layer, closed=True, passes=passes)

    def circle (self, cx, cy, r, width=0.8, tone=0.85, layer="ink", passes=1):
        for _ in range (passes):
            start = self.random.uniform (0, 2 * math.pi)
            squash = self.random.uniform (-0.01, 0.01) * r
            steps = max (16, int (r * 1.6))
            points = []
            for index in range (steps + 1):
                angle = start + 2 * math.pi * index / steps
                radius = r + squash * math.cos (2 * angle) + self.random.uniform (-0.05, 0.05)
                points.append ((cx + radius * math.cos (angle), cy + radius * math.sin (angle)))
            self._path (straight (points), width * self.random.uniform (0.92, 1.08),
                        tone * self.random.uniform (0.88, 1.0), layer)

    # Parallel strokes across a rectangle, like shading with a sharp pencil.
    def hatch (self, x, y, w, h, gap=3.0, angle=45, tone=0.25, width=0.4, layer="shade"):
        radians = math.radians (angle)
        dx, dy = math.cos (radians), math.sin (radians)
        nx, ny = -dy, dx
        corners = [(x, y), (x + w, y), (x + w, y + h), (x, y + h)]
        offsets = [cx * nx + cy * ny for cx, cy in corners]
        offset = min (offsets) + gap / 2
        while offset < max (offsets):
            segment = clip_line (corners, (nx * offset, ny * offset), (dx, dy))
            if segment:
                self._stroke (list (segment), width, tone * self.random.uniform (0.8, 1.05),
                              layer, wobble=0.08, closed=False)
            offset += gap * self.random.uniform (0.92, 1.08)

    # Solid areas --------------------------------------------------------

    def fill (self, points, tone=0.15, layer="shade"):
        self.layers[layer].append (
            f'<path d="{straight (points)} Z" fill="{GRAPHITE}" fill-opacity="{tone:.2f}"/>')

    # A flat area of colour under the drawing, like a light wash.
    def paper_fill (self, points, color, layer="paper"):
        self.layers[layer].append (f'<path d="{straight (points)} Z" fill="{color}"/>')

    def disc (self, cx, cy, r, tone=0.8, layer="ink"):
        self.layers[layer].append (
            f'<circle cx="{cx:.1f}" cy="{cy:.1f}" r="{r:.2f}" fill="{GRAPHITE}" '
            f'fill-opacity="{tone:.2f}"/>')

    # Flat, opaque colour, for something that hides what is under it.
    def solid (self, points, color, layer="top"):
        self.layers[layer].append (f'<path d="{straight (points)} Z" fill="{color}"/>')

    # A wash of colour over any shape, or over a circle.
    def tint (self, points, color, layer="top", opacity=1.0):
        self.layers[layer].append (
            f'<path d="{straight (points)} Z" fill="{color}" fill-opacity="{opacity:.2f}" '
            f'filter="url(#{self.id ("grain")})"/>')

    def spot (self, cx, cy, r, color, layer="top", opacity=1.0):
        self.layers[layer].append (
            f'<circle cx="{cx:.1f}" cy="{cy:.1f}" r="{r:.2f}" fill="{color}" '
            f'fill-opacity="{opacity:.2f}" filter="url(#{self.id ("grain")})"/>')

    # A part drawn in its own frame, moved and turned in quarter turns. Text
    # inside keeps reading the right way up.
    def begin (self, x, y, angle=0, scale=1):
        zoom = f" scale({scale})" if scale != 1 else ""
        for layer in self.layers.values ():
            layer.append (f'<g transform="translate({x:.1f} {y:.1f}) rotate({angle}){zoom}">')
        self.turns.append (angle)

    def end (self):
        for layer in self.layers.values ():
            layer.append ("</g>")
        self.turns.pop ()

    # Bench details ------------------------------------------------------

    def hole (self, x, y):
        self.layers["crisp"].append (
            f'<rect x="{x - 1.8:.1f}" y="{y - 1.8:.1f}" width="3.6" height="3.6" rx="0.6" '
            f'fill="{GRAPHITE}" fill-opacity="0.42"/>')

    def socket (self, x, y):
        self.layers["top"].append (
            f'<rect x="{x - 2.2:.1f}" y="{y - 2.2:.1f}" width="4.4" height="4.4" fill="#141414" '
            f'fill-opacity="0.9"/>')

    def stripe (self, a, b, color):
        self.layers["crisp"].append (
            f'<path d="M {a[0]:.1f} {a[1]:.1f} L {b[0]:.1f} {b[1]:.1f}" stroke="{color}" '
            f'stroke-width="1.1" stroke-opacity="0.7"/>')

    # A component lead: a thin bright wire with a fine edge.
    def lead (self, a, b):
        self.line (a, b, width=1.5, tone=0.55, layer="top", wobble=0.08)
        self.line (a, b, width=0.7, tone=0.0, layer="top", wobble=0.0)
        self.layers["top"].append (
            f'<path d="M {a[0]:.1f} {a[1]:.1f} L {b[0]:.1f} {b[1]:.1f}" stroke="#e4e2dc" '
            f'stroke-width="0.6"/>')

    # A part's body, coloured in, under its pencil outline.
    def body (self, box, color, radius=0):
        x, y, w, h = box
        self.layers["top"].append (
            f'<rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{h:.1f}" rx="{radius}" '
            f'fill="{color}" filter="url(#{self.id ("grain")})"/>')

    def band (self, x, y, w, h, color):
        self.layers["top"].append (
            f'<rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{h:.1f}" fill="{color}"/>')

    # A clear lens seen from above, tinted, with a highlight. A list of
    # colours tints it with all of them, as an RGB LED looks.
    def dome (self, cx, cy, r, color):
        gradient = self.id (f"dome{len (self.layers['top'])}")
        if isinstance (color, (list, tuple)):
            stops = "".join (f'<stop offset="{index / (len (color) - 1):.2f}" stop-color="{tint}"/>'
                             for index, tint in enumerate (color))
            self.layers["top"].append (
                f'<linearGradient id="{gradient}r" x1="0.1" y1="0.1" x2="0.9" y2="0.9">{stops}'
                f'</linearGradient>'
                f'<circle cx="{cx:.1f}" cy="{cy:.1f}" r="{r:.1f}" fill="url(#{gradient}r)" '
                f'fill-opacity="0.75"/>')
            color, middle, rim = "#ffffff", ' stop-opacity="0.15"', "0.05"
        else:
            middle, rim = "", "0.95"
        self.layers["top"].append (
            f'<radialGradient id="{gradient}" cx="0.4" cy="0.38" r="0.7">'
            f'<stop offset="0" stop-color="#ffffff" stop-opacity="0.7"/>'
            f'<stop offset="0.45" stop-color="{color}"{middle}/>'
            f'<stop offset="1" stop-color="{color}" stop-opacity="{rim}"/></radialGradient>'
            f'<circle cx="{cx:.1f}" cy="{cy:.1f}" r="{r:.1f}" fill="url(#{gradient})"/>')
        self.circle (cx, cy, r, width=0.9, layer="top")
        # A crescent of light on the lens.
        self.layers["top"].append (
            f'<path d="M {cx - r * 0.62:.1f} {cy - r * 0.1:.1f} A {r * 0.66:.1f} {r * 0.66:.1f} 0 0 1 '
            f'{cx - r * 0.05:.1f} {cy - r * 0.64:.1f}" fill="none" stroke="#ffffff" '
            f'stroke-opacity="0.8" stroke-width="{max (0.8, r * 0.1):.1f}" stroke-linecap="round"/>')

    # A jumper's male end: its black housing and the tip of its pin.
    def pin_end (self, x, y):
        self.layers["top"].append (
            f'<rect x="{x - 3.3:.1f}" y="{y - 3.3:.1f}" width="6.6" height="6.6" rx="0.8" '
            f'fill="#222222" stroke="#000000" stroke-opacity="0.5" stroke-width="0.4"/>'
            f'<circle cx="{x:.1f}" cy="{y:.1f}" r="1.0" fill="#c9c9c4"/>')

    # The black housing on a jumper's female end, pushed over a header pin
    # from a to b.
    def housing (self, a, b, width=6.4):
        (x1, y1), (x2, y2) = a, b
        length = max (1.0, math.dist (a, b))
        nx, ny = -(y2 - y1) / length * width / 2, (x2 - x1) / length * width / 2
        corners = [(x1 + nx, y1 + ny), (x2 + nx, y2 + ny), (x2 - nx, y2 - ny), (x1 - nx, y1 - ny)]
        self.layers["top"].append (
            f'<path d="{straight (corners)} Z" fill="#222222" stroke="#000000" stroke-opacity="0.5" '
            f'stroke-width="0.4"/>'
            f'<path d="M {x1 + nx * 0.45:.1f} {y1 + ny * 0.45:.1f} L {x2 + nx * 0.45:.1f} '
            f'{y2 + ny * 0.45:.1f}" stroke="#ffffff" stroke-opacity="0.2" stroke-width="0.8"/>')

    # The tinned end of a lead, or of a wire stripped for a screw terminal.
    def tip (self, x, y, r=1.6):
        self.layers["top"].append (
            f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r:.1f}" fill="#c4c1b8" stroke="{GRAPHITE}" '
            f'stroke-opacity="0.6" stroke-width="0.5"/>')

    # A fine arrow along a gentle curve, for notes.
    def arrow (self, points, tone=0.75):
        self._path (curve (points), 0.7, tone, "text")
        (x1, y1), (x2, y2) = points[-2], points[-1]
        angle = math.atan2 (y2 - y1, x2 - x1)
        head = [(x2 + 5 * math.cos (angle + math.pi + side * 0.4),
                 y2 + 5 * math.sin (angle + math.pi + side * 0.4)) for side in (-1, 1)]
        self.layers["text"].append (
            f'<path d="{straight ([head[0], (x2, y2), head[1]])} Z" fill="{GRAPHITE}" '
            f'fill-opacity="{tone:.2f}"/>')

    # A jumper wire: coloured, with a fine graphite edge and a soft highlight.
    # Its bends are rounded, and its runs between them straight.
    def wire (self, points, color, width=3.6, radius=7, layer="wire", smooth=False, path=None):
        path = path or (curve (points) if smooth else
                        rounded_path (points, radius) if len (points) > 2 else straight (points))
        self.layers[layer].append (
            f'<path d="{path}" fill="none" stroke="{GRAPHITE}" stroke-opacity="0.6" '
            f'stroke-width="{width + 1.1:.1f}" stroke-linecap="round" stroke-linejoin="round"/>'
            f'<path d="{path}" fill="none" stroke="{color}" stroke-width="{width:.1f}" '
            f'stroke-linecap="round" stroke-linejoin="round"/>'
            f'<path d="{path}" fill="none" stroke="#ffffff" stroke-opacity="0.35" '
            f'stroke-width="{width / 3.6:.1f}" stroke-linecap="round" '
            f'transform="translate(-0.5 -0.6)"/>')

    # A small printed label that keeps a halo of paper, with a fine leader
    # line to what it names when that is not right beside it.
    # It sits on a little patch of paper, so the holes and lines under it
    # never show through its letters.
    def label (self, x, y, content, size=10, anchor="middle", to=None, width=None, patch=None):
        width = width or len (content) * size * 0.5
        left = {"start": x, "end": x - width}.get (anchor, x - width / 2)
        # The paper under it, from patch's left to its right when given.
        under = patch or (left - 1.2, left + width + 1.2)
        if to:
            sx = min (max (to[0], left - 1.5), left + width + 1.5)
            sy = y - size * 0.35 if abs (to[0] - sx) > 1 else (y + 2.5 if to[1] > y else y - size - 0.5)
            self._path (straight ([(sx, sy), to]), 0.45, 0.7, "text")
            self.layers["text"].append (
                f'<circle cx="{to[0]:.1f}" cy="{to[1]:.1f}" r="0.9" fill="{GRAPHITE}" '
                f'fill-opacity="0.75"/>')
        self.layers["text"].append (
            f'<rect x="{under[0]:.1f}" y="{y - size * 0.78:.1f}" width="{under[1] - under[0]:.1f}" '
            f'height="{size * 1.02:.1f}" rx="{size * 0.25:.1f}" fill="{PAPER}" fill-opacity="0.92"/>')
        self.text (x, y, content, size=size, anchor=anchor, kind="label")

    # Words: "label" names things on the drawing and stays level, "silk" is
    # the printing on a board and turns with it but never reads upside
    # down, and "mono" is a screen's characters, which turn with it. A halo
    # of paper, or of the board's colour, keeps words readable over lines.
    def text (self, x, y, content, size=10, anchor="middle", tone=0.9, weight="normal",
              rotate=0, kind="label", color=GRAPHITE, halo=None, layer="text", italic=False):
        kind = "label" if kind == "note" else kind
        if kind == "label" and halo is None:
            halo = PAPER
        turn = sum (self.turns)
        if turn and kind == "label":
            rotate -= turn
        elif turn and kind == "silk" and 90 < (rotate + turn) % 360 <= 270:
            rotate += 180
            anchor = {"start": "end", "end": "start"}.get (anchor, anchor)
        self.mono = self.mono or kind == "mono"
        transform = f' transform="rotate({rotate} {x:.1f} {y:.1f})"' if rotate % 360 else ""
        outline = (f' stroke="{halo}" stroke-width="{min (2.6, size * 0.3):.1f}" '
                   f'stroke-linejoin="round" paint-order="stroke"') if halo else ""
        family = "mono" if kind == "mono" else "silk"
        slant = ' font-style="italic"' if italic else ""
        self.layers[layer].append (
            f'<text class="{family}" x="{x:.1f}" y="{y:.1f}" font-size="{size}" '
            f'text-anchor="{anchor}" font-weight="{weight}"{slant} fill="{color}" '
            f'fill-opacity="{tone:.2f}"{outline}{transform}>{escape (content)}</text>')

    # The picture --------------------------------------------------------

    def svg (self, box, title):
        x, y, w, h = box
        pencil, grain, paper = self.id ("pencil"), self.id ("grain"), self.id ("paper")
        defs = (
            f'<defs>'
            f'<filter id="{pencil}" x="-2%" y="-2%" width="104%" height="104%">'
            f'<feTurbulence type="fractalNoise" baseFrequency="0.9" numOctaves="2" seed="7" result="n"/>'
            f'<feDisplacementMap in="SourceGraphic" in2="n" scale="0.5" xChannelSelector="R" yChannelSelector="G" result="shaky"/>'
            f'<feColorMatrix in="n" type="matrix" values="0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -0.6 1.25" result="speckle"/>'
            f'<feComposite in="shaky" in2="speckle" operator="in"/></filter>'
            f'<filter id="{grain}" x="-2%" y="-2%" width="104%" height="104%">'
            f'<feTurbulence type="fractalNoise" baseFrequency="1.4" numOctaves="1" seed="3" result="n"/>'
            f'<feColorMatrix in="n" type="matrix" values="0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -0.4 1.2" result="speckle"/>'
            f'<feComposite in="SourceGraphic" in2="speckle" operator="in"/></filter>'
            f'<filter id="{paper}" x="0" y="0" width="100%" height="100%">'
            f'<feTurbulence type="fractalNoise" baseFrequency="0.04 0.7" numOctaves="3" seed="11"/>'
            f'<feColorMatrix type="matrix" values="0 0 0 0 0.35 0 0 0 0 0.33 0 0 0 0 0.3 0 0 0 0.05 0"/></filter>'
            f'</defs>')
        layers = self.layers
        body = (
            f'<rect x="{x:.0f}" y="{y:.0f}" width="{w:.0f}" height="{h:.0f}" fill="{PAPER}"/>'
            f'<rect x="{x:.0f}" y="{y:.0f}" width="{w:.0f}" height="{h:.0f}" filter="url(#{paper})"/>'
            + "".join (layers["paper"] + layers["crisp"])
            + f'<g filter="url(#{pencil})">' + "".join (layers["shade"] + layers["ink"]) + "</g>"
            + "".join (layers["wire"])
            + f'<g filter="url(#{pencil})">' + "".join (layers["top"]) + "</g>"
            + "".join (layers["text"]))
        return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="{x:.0f} {y:.0f} {w:.0f} {h:.0f}" '
                f'role="img" aria-labelledby="{self.id ("title")}" class="pencil-drawing">'
                f'<title id="{self.id ("title")}">{escape (title)}</title>'
                f'<style>.silk{{font-family:"Atkinson Hyperlegible Next",sans-serif}}'
                + ('.mono{font-family:"JetBrains Mono",monospace}' if self.mono else "")
                + '</style>'
                f'{defs}{body}</svg>')

    def _stroke (self, points, width, tone, layer, wobble, closed):
        jittered = [(x + self.random.uniform (-wobble, wobble),
                     y + self.random.uniform (-wobble, wobble)) for x, y in points]
        if closed:
            jittered.append (jittered[0])
        # Long runs bend a little along their length, as a hand would.
        detailed = []
        for a, b in zip (jittered, jittered[1:]):
            pieces = max (1, int (math.dist (a, b) / 60))
            for piece in range (pieces):
                t = piece / pieces
                bend = self.random.uniform (-wobble, wobble) * 0.5 if piece else 0
                detailed.append ((a[0] + (b[0] - a[0]) * t + bend, a[1] + (b[1] - a[1]) * t + bend))
        detailed.append (jittered[-1])
        self._path (straight (detailed), width * self.random.uniform (0.92, 1.08),
                    tone * self.random.uniform (0.88, 1.0), layer)

    def _path (self, d, width, tone, layer):
        self.layers[layer].append (
            f'<path d="{d}" fill="none" stroke="{GRAPHITE}" stroke-opacity="{tone:.2f}" '
            f'stroke-width="{width:.2f}" stroke-linecap="round" stroke-linejoin="round"/>')


def straight (points):
    return "M " + " L ".join (f"{x:.1f} {y:.1f}" for x, y in points)


# A Catmull-Rom curve through the points, as cubic Béziers.
def curve (points):
    if len (points) == 2:
        return straight (points)
    padded = [points[0]] + list (points) + [points[-1]]
    d = f"M {points[0][0]:.1f} {points[0][1]:.1f}"
    for index in range (1, len (padded) - 2):
        p0, p1, p2, p3 = padded[index - 1:index + 3]
        c1 = (p1[0] + (p2[0] - p0[0]) / 6, p1[1] + (p2[1] - p0[1]) / 6)
        c2 = (p2[0] - (p3[0] - p1[0]) / 6, p2[1] - (p3[1] - p1[1]) / 6)
        d += f" C {c1[0]:.1f} {c1[1]:.1f} {c2[0]:.1f} {c2[1]:.1f} {p2[0]:.1f} {p2[1]:.1f}"
    return d


# Straight runs between the points, each corner turned on a radius.
def rounded_path (points, radius=16):
    kept = [points[0]]
    for point in points[1:]:
        if math.dist (point, kept[-1]) > 2:
            kept.append (point)
    if math.dist (points[-1], kept[-1]) > 0:
        kept[-1] = points[-1]
    d = f"M {kept[0][0]:.1f} {kept[0][1]:.1f}"
    for a, p, b in zip (kept, kept[1:], kept[2:]):
        da, db = math.dist (a, p), math.dist (p, b)
        r = min (radius, da * 0.5, db * 0.5)
        p1 = (p[0] + (a[0] - p[0]) * r / da, p[1] + (a[1] - p[1]) * r / da)
        p2 = (p[0] + (b[0] - p[0]) * r / db, p[1] + (b[1] - p[1]) * r / db)
        d += (f" L {p1[0]:.1f} {p1[1]:.1f} Q {p[0]:.1f} {p[1]:.1f} {p2[0]:.1f} {p2[1]:.1f}")
    return d + f" L {kept[-1][0]:.1f} {kept[-1][1]:.1f}"


def rounded_rectangle (x, y, w, h, r):
    points = []
    for cx, cy, start in ((x + w - r, y + r, -90), (x + w - r, y + h - r, 0),
                          (x + r, y + h - r, 90), (x + r, y + r, 180)):
        for step in range (5):
            angle = math.radians (start + step * 22.5)
            points.append ((cx + r * math.cos (angle), cy + r * math.sin (angle)))
    return points


# Where the line through point along direction crosses a convex polygon.
def clip_line (polygon, point, direction):
    hits = []
    for (x1, y1), (x2, y2) in zip (polygon, polygon[1:] + polygon[:1]):
        ex, ey = x2 - x1, y2 - y1
        denominator = direction[0] * ey - direction[1] * ex
        if abs (denominator) < 1e-9:
            continue
        t = ((x1 - point[0]) * ey - (y1 - point[1]) * ex) / denominator
        u = ((x1 - point[0]) * direction[1] - (y1 - point[1]) * direction[0]) / denominator
        if 0 <= u <= 1:
            hits.append ((point[0] + direction[0] * t, point[1] + direction[1] * t))
    if len (hits) < 2:
        return None
    hits.sort ()
    return hits[0], hits[-1]
