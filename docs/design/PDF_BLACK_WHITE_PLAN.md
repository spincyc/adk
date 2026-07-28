# Black-and-white lesson PDF plan

## Publication contract

Released lesson PDFs use only black, white, and device-independent grayscale.
They do not require color ink, and color never distinguishes a wire, state,
warning, syntax token, or measurement. Pencil images are grayscale before TeX
includes them. Schematics identify signals with labels, line patterns, and
connection tables.

This requirement is stricter than “still understandable when printed in
grayscale.” HTML may retain an accessible color palette; the printable PDF is
deliberately monochrome.

## Current audit

All 12 current TeX lessons define or use non-black colors. Lessons 001, 004,
007, and 008 use muted custom palettes; the others use blue section headings,
green comments, and red safety boxes. Lesson 004 also defines red, green, and
blue wire colors.

Ten of the 12 pencil assets are RGB PNG files:

```text
002-digital-input-pencil.png
003-reaction-timer-pencil.png
005-piezo-pencil.png
006-simon-pencil.png
007-analog-input-pencil.png
008-sampled-signal-pencil.png
010-shift-register-pencil.png
011-timed-traffic-pencil.png
012-traffic-junction-pencil.png
```

Lesson 009 now uses three grayscale plates instead of the rejected single
orientation image:

```text
009-mega-header-locator.png
009-night-light-overview.png
009-night-light-breadboard.png
```

They deliberately separate physical Mega header location, logical
header-to-net mapping, and enlarged breadboard coordinates. This keeps the
labels and endpoints legible at final PDF size while preserving a
cross-checkable net manifest. Assets 001 and 004 are also grayscale PNGs. An
RGB file can happen to contain only gray pixels, so file metadata alone is a
useful gate but not sufficient proof.

## Mechanical enforcement

Implement this as one dependency-ordered documentation boundary:

1. Add a shared TeX style that defines black text, white pages, gray code
   backgrounds, black rules, and patterned warning boxes.
2. Replace lesson-local color definitions with the shared style. Use labels and
   line patterns where a diagram currently names color.
3. Convert source pencil assets to 8-bit grayscale while preserving the
   originals outside the release path only when provenance requires it.
4. Rebuild every PDF through the existing deterministic two-pass pipeline.
5. Extend `lessons-check` with both structural and rendered-output checks.

The structural check rejects RGB or CMYK embedded images:

```sh
pdfimages -list lesson.pdf
```

Every image row must report `gray` or `mono`. The rendered-output check uses
Ghostscript ink coverage:

```sh
gs -q -o - -sDEVICE=inkcov lesson.pdf
```

Every page must report zero cyan, magenta, and yellow coverage. Nonzero black
coverage is expected. Parse the numeric fields rather than matching one
Ghostscript display format, and fail closed when the tool or output is missing.

The gate also rasterizes representative pages at release resolution and checks
that every pixel has equal red, green, and blue channels. This catches colored
vector content that an embedded-image check cannot see. Keep the ink-coverage
test as the authoritative release gate and the pixel test as a diagnostic.

## Human acceptance

Mechanical grayscale is not enough. Review every lesson at normal size and
200% zoom, then print at least one diagram-heavy page and one worksheet page on
a monochrome printer. Confirm:

- headings remain distinct by size and weight;
- warnings remain distinct by border, label, and spacing;
- code comments and keywords remain readable without syntax color;
- every wire and state is identified without hue;
- shaded regions remain separable when a printer compresses light grays;
- pencil strokes, test-point labels, and worksheet rules remain legible.

Record the printer, driver mode, paper size, lesson pages, reviewer, date, and
findings. Do not call the PDF black-and-white compliant until both automated
checks and this print review pass.

## Acceptance commands

The eventual repository target should make this sufficient:

```sh
make lessons
make lessons-check
```

`lessons-check` must fail on any colored source that reaches a generated PDF,
not merely warn. Reproducibility, metadata, font, extraction, link, and
accessibility checks remain in force alongside the monochrome gate.
