"""A pencil for SVG. Every stroke is drawn twice with a little seeded wobble,
shading is hatched, colour goes on like coloured pencil, and the paper has
grain. The same seed always draws the same picture, so a drawing only
changes when its circuit does.
"""

import math
import random
from xml.sax.saxutils import escape

GRAPHITE = "#2b2b2b"
PAPER    = "#fbf9f3"


class Pencil:
    def __init__ (self, seed, prefix="pencil"):
        self.random = random.Random (seed)
        self.prefix = prefix
        self.layers = {name: [] for name in ("paper", "crisp", "shade", "ink", "wire", "top", "text")}

    def id (self, name):
        return f"{self.prefix}-{name}"

    # Strokes ------------------------------------------------------------

    def line (self, a, b, width=1.1, tone=0.85, layer="ink", passes=2, wobble=0.6):
        for _ in range (passes):
            self._stroke ([a, b], width, tone, layer, wobble, closed=False)

    def polyline (self, points, width=1.1, tone=0.85, layer="ink", closed=False, passes=2,
                  wobble=0.6):
        for _ in range (passes):
            self._stroke (points, width, tone, layer, wobble, closed)

    def rect (self, x, y, w, h, width=1.1, tone=0.85, layer="ink", passes=2, radius=0):
        points = rounded_rectangle (x, y, w, h, radius) if radius else \
            [(x, y), (x + w, y), (x + w, y + h), (x, y + h)]
        self.polyline (points, width, tone, layer, closed=True, passes=passes)

    def circle (self, cx, cy, r, width=1.1, tone=0.85, layer="ink", passes=2):
        for _ in range (passes):
            start = self.random.uniform (0, 2 * math.pi)
            squash = self.random.uniform (-0.04, 0.04) * r
            steps = max (14, int (r * 1.5))
            points = []
            for index in range (steps + 2):
                angle = start + 2 * math.pi * index / steps
                radius = r + squash * math.cos (2 * angle) + self.random.uniform (-0.2, 0.2)
                points.append ((cx + radius * math.cos (angle), cy + radius * math.sin (angle)))
            self._path (straight (points), width, tone, layer)

    # Parallel strokes across a rectangle, like shading with a pencil's side.
    def hatch (self, x, y, w, h, gap=3.0, angle=45, tone=0.35, width=0.7, layer="shade"):
        radians = math.radians (angle)
        dx, dy = math.cos (radians), math.sin (radians)
        nx, ny = -dy, dx
        corners = [(x, y), (x + w, y), (x + w, y + h), (x, y + h)]
        offsets = [cx * nx + cy * ny for cx, cy in corners]
        offset = min (offsets) + gap / 2
        while offset < max (offsets):
            segment = clip_line (corners, (nx * offset, ny * offset), (dx, dy))
            if segment:
                self._stroke (list (segment), width, tone * self.random.uniform (0.75, 1.1),
                              layer, wobble=0.3, closed=False)
            offset += gap * self.random.uniform (0.85, 1.15)

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

    # Bench details ------------------------------------------------------

    def hole (self, x, y):
        self.layers["crisp"].append (
            f'<rect x="{x - 2.1:.1f}" y="{y - 2.1:.1f}" width="4.2" height="4.2" rx="0.7" '
            f'fill="{GRAPHITE}" fill-opacity="0.5"/>')

    def socket (self, x, y):
        self.layers["top"].append (
            f'<rect x="{x - 2.5:.1f}" y="{y - 2.5:.1f}" width="5" height="5" fill="#111" '
            f'fill-opacity="0.92"/>')

    def stripe (self, a, b, color):
        self.layers["wire"].append (
            f'<path d="M {a[0]:.1f} {a[1]:.1f} L {b[0]:.1f} {b[1]:.1f}" stroke="{color}" '
            f'stroke-width="1.8" stroke-opacity="0.85" filter="url(#{self.id ("grain")})"/>')

    # A component lead: a thin bright wire.
    def lead (self, a, b):
        self.line (a, b, width=1.3, tone=0.75, layer="top", passes=1, wobble=0.2)
        self.line (a, b, width=0.5, tone=0.25, layer="top", passes=1, wobble=0.2)

    # A part's body, coloured in, under its pencil outline.
    def body (self, box, color, radius=0):
        x, y, w, h = box
        self.layers["top"].append (
            f'<rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{h:.1f}" rx="{radius}" '
            f'fill="{color}" filter="url(#{self.id ("grain")})"/>')

    def band (self, x, y, w, h, color):
        self.layers["top"].append (
            f'<rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{h:.1f}" fill="{color}" '
            f'filter="url(#{self.id ("grain")})"/>')

    # A round, lit-looking cap: an LED, or a button's plunger.
    def dome (self, cx, cy, r, color):
        gradient = self.id (f"dome{len (self.layers['top'])}")
        self.layers["top"].append (
            f'<radialGradient id="{gradient}" cx="0.38" cy="0.35" r="0.75">'
            f'<stop offset="0" stop-color="#ffffff" stop-opacity="0.85"/>'
            f'<stop offset="0.35" stop-color="{color}"/>'
            f'<stop offset="1" stop-color="{color}" stop-opacity="0.9"/></radialGradient>'
            f'<circle cx="{cx:.1f}" cy="{cy:.1f}" r="{r:.1f}" fill="url(#{gradient})" '
            f'filter="url(#{self.id ("grain")})"/>')
        self.circle (cx, cy, r, width=1.25, layer="top")

    # The black plastic sleeve and metal tip at each end of a jumper wire.
    def pin_end (self, x, y):
        self.layers["top"].append (
            f'<rect x="{x - 3.6:.1f}" y="{y - 3.6:.1f}" width="7.2" height="7.2" rx="1" '
            f'fill="#1d1d1d"/><circle cx="{x:.1f}" cy="{y:.1f}" r="1.3" fill="#bdbdbd"/>')

    # A jumper wire in coloured pencil, with a pencilled edge and a highlight.
    def wire (self, points, color, width=6.2):
        path = curve (points)
        grain = self.id ("grain")
        self.layers["wire"].append (
            f'<path d="{path}" fill="none" stroke="{GRAPHITE}" stroke-opacity="0.8" '
            f'stroke-width="{width + 2.4:.1f}" stroke-linecap="round"/>'
            f'<path d="{path}" fill="none" stroke="{color}" stroke-width="{width:.1f}" '
            f'stroke-linecap="round" filter="url(#{grain})"/>'
            f'<path d="{path}" fill="none" stroke="#ffffff" stroke-opacity="0.4" '
            f'stroke-width="{width / 4:.1f}" stroke-linecap="round" transform="translate(-1.2 -1.2)"/>')

    # A handwritten label on a patch of clean paper, so it reads over lines.
    def label (self, x, y, content, size=13):
        width = len (content) * size * 0.42 + 8
        self.layers["text"].append (
            f'<rect x="{x - width / 2:.1f}" y="{y - size * 0.85:.1f}" width="{width:.1f}" '
            f'height="{size * 1.15:.1f}" rx="{size * 0.5:.1f}" fill="{PAPER}" fill-opacity="0.92"/>')
        self.text (x, y, content, size=size, kind="note")

    # A strip of paper down one edge, carrying labels at given heights.
    def ruler (self, x, y, w, h, labels):
        self.layers["text"].append (
            f'<rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{h:.1f}" fill="{PAPER}"/>'
            f'<path d="M {x + w:.1f} {y:.1f} V {y + h:.1f}" stroke="{GRAPHITE}" '
            f'stroke-opacity="0.35" stroke-dasharray="2 3"/>')
        for label, ly in labels:
            self.text (x + w / 2, ly + 3.5, label, size=10, kind="silk", tone=0.85)

    # Words: "note" is handwriting, "silk" is the printing on a board.
    def text (self, x, y, content, size=12, anchor="middle", tone=0.9, weight="normal",
              rotate=0, kind="note"):
        transform = f' transform="rotate({rotate} {x:.1f} {y:.1f})"' if rotate else ""
        self.layers["text"].append (
            f'<text class="{kind}" x="{x:.1f}" y="{y:.1f}" font-size="{size}" '
            f'text-anchor="{anchor}" font-weight="{weight}" fill="{GRAPHITE}" '
            f'fill-opacity="{tone:.2f}"{transform}>{escape (content)}</text>')

    # The picture --------------------------------------------------------

    def svg (self, box, title):
        x, y, w, h = box
        pencil, grain, paper = self.id ("pencil"), self.id ("grain"), self.id ("paper")
        defs = (
            f'<defs>'
            f'<filter id="{pencil}" x="-2%" y="-2%" width="104%" height="104%">'
            f'<feTurbulence type="fractalNoise" baseFrequency="0.85" numOctaves="2" seed="7" result="n"/>'
            f'<feDisplacementMap in="SourceGraphic" in2="n" scale="1.4" xChannelSelector="R" yChannelSelector="G" result="shaky"/>'
            f'<feColorMatrix in="n" type="matrix" values="0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1.0 1.3" result="speckle"/>'
            f'<feComposite in="shaky" in2="speckle" operator="in"/></filter>'
            f'<filter id="{grain}" x="-2%" y="-2%" width="104%" height="104%">'
            f'<feTurbulence type="fractalNoise" baseFrequency="1.3" numOctaves="1" seed="3" result="n"/>'
            f'<feColorMatrix in="n" type="matrix" values="0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -0.8 1.25" result="speckle"/>'
            f'<feComposite in="SourceGraphic" in2="speckle" operator="in"/></filter>'
            f'<filter id="{paper}" x="0" y="0" width="100%" height="100%">'
            f'<feTurbulence type="fractalNoise" baseFrequency="0.04 0.7" numOctaves="3" seed="11"/>'
            f'<feColorMatrix type="matrix" values="0 0 0 0 0.35 0 0 0 0 0.33 0 0 0 0 0.3 0 0 0 0.07 0"/></filter>'
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
                f'<style>.note{{font-family:"Patrick Hand",cursive}}'
                f'.silk{{font-family:"Atkinson Hyperlegible Next",sans-serif}}</style>'
                f'{defs}{body}</svg>')

    def _stroke (self, points, width, tone, layer, wobble, closed):
        jittered = [(x + self.random.uniform (-wobble, wobble),
                     y + self.random.uniform (-wobble, wobble)) for x, y in points]
        if closed:
            jittered.append (jittered[0])
        # Long runs bend a little along their length, as a hand would.
        detailed = []
        for a, b in zip (jittered, jittered[1:]):
            pieces = max (1, int (math.dist (a, b) / 40))
            for piece in range (pieces):
                t = piece / pieces
                bend = self.random.uniform (-wobble, wobble) * 0.6 if piece else 0
                detailed.append ((a[0] + (b[0] - a[0]) * t + bend, a[1] + (b[1] - a[1]) * t + bend))
        detailed.append (jittered[-1])
        self._path (straight (detailed), width * self.random.uniform (0.85, 1.15),
                    tone * self.random.uniform (0.8, 1.0), layer)

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
