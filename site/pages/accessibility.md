---
title: Accessibility
description: Accessibility requirements for the ADK website, lessons, diagrams, code, and printable PDFs.
---

# Accessibility

ADK documentation is designed to remain useful with a keyboard, screen reader,
small display, high zoom, reduced color perception, or monochrome printer. The
website targets WCAG 2.2 Level AA. Automated checks help prevent regressions,
but they do not replace testing by people using the material.

## Page structure and navigation

- Use semantic HTML and one descriptive `h1` per page. Heading levels must not
  skip merely to obtain a visual style.
- Provide a skip link and identifiable header, navigation, main, and footer
  landmarks. Mark the current navigation item with text and `aria-current`.
- All functions must work with a keyboard. Focus order must follow reading
  order, focus must remain visible, and no component may trap focus.
- Links must describe their destination without surrounding context. Avoid
  repeated labels such as “click here” and “read more.”
- Pointer targets must meet the WCAG 2.2 AA minimum of 24 by 24 CSS pixels;
  prefer at least 44 by 44 pixels for primary controls.
- Do not place essential instructions or content behind hover, animation, or a
  time limit. Respect `prefers-reduced-motion`.

## Color and visual presentation

- Normal text must have a contrast ratio of at least 4.5:1. Large text must
  have at least 3:1 contrast. Focus indicators, controls, and meaningful
  diagram boundaries must have at least 3:1 contrast against adjacent colors.
- Color must never be the only carrier of meaning. Add labels, numbers,
  positions, line styles, patterns, or shapes. Simon cues, wire identities,
  warnings, and component states must remain distinct in grayscale.
- Text must remain readable when user font spacing is increased. Do not encode
  prose or source code in raster images.

## Mobile and reflow

- Pages must remain usable at 320 CSS pixels wide and at 400 percent zoom
  without page-level horizontal scrolling.
- Code listings, wide tables, and schematics may use a clearly bounded,
  keyboard-accessible local scrolling region when they cannot reflow safely.
- Navigation controls must expose their name and expanded state. Content must
  not require hover, precise dragging, or a particular screen orientation.
- Test representative pages at 320, 375, 768, and 1280 CSS pixels, including
  portrait and landscape layouts.

## Diagrams and images

- Decorative images use empty alternative text. Informative images receive
  concise alternative text describing their purpose, not their appearance
  alone.
- A complex schematic, timing graph, state graph, or wiring diagram requires a
  short alternative and an adjacent long text description. The description
  must preserve every connection, pin, component value, direction, state,
  timing relationship, and safety boundary needed to complete the lesson.
- Pencil drawings provide orientation only. Their captions must direct the
  learner to the exact schematic and state that the drawing is not
  authoritative wiring guidance.
- Color-coded wires must also be identified by endpoint and label. Diagrams
  must remain legible when enlarged and printed in grayscale.

## Source code

- Present code as real text in `pre` and `code` elements. Syntax color is
  supplemental; tokens must remain distinguishable without it.
- A listing identifies its language, purpose, prerequisites, expected result,
  and canonical source file. Line numbers must not corrupt copied code or
  screen-reader output.
- Copy controls must be keyboard operable and announce success without moving
  focus. On narrow displays, wrapping or a local scrolling region must not
  cause the whole page to overflow.

## Lessons, downloads, and PDFs

- The target is for each HTML lesson to be the complete canonical accessible
  lesson, so a PDF is never the only form of lesson content.
- PDF links state their purpose, format, page count, and approximate size, for
  example: “Download printable Lesson 004 PDF (12 pages, 2.1 MB).”
- A published accessible PDF must have tagged semantic structure, correct
  reading order, document title and language, bookmarks, meaningful link text,
  image alternatives, Unicode text extraction, and embedded fonts. Until a PDF
  meets those requirements, label it as a print edition and link prominently
  to the accessible HTML edition.
- Printed output must not clip code, tables, diagrams, labels, or URLs. Hide
  website navigation and interactive-only controls; avoid orphaned headings
  and unnecessary blank pages. Validate one grayscale paper copy for each new
  lesson layout.

### First-publication status

The current HTML pages provide searchable summaries, prerequisites, safety
boundaries, wiring, and download navigation. The print-edition PDFs still
contain some exercises, evidence tables, and diagrams that have not yet been
transcribed into equivalent HTML. The PDFs have useful document metadata and
extractable text, but they are not yet tagged PDFs. Full HTML lesson parity,
PDF tagging, and manual assistive-technology testing remain release-roadmap
work; this first publication does not claim WCAG conformance.

## Verification

Every site release should include:

1. A static build with broken links, missing fragments, and missing assets
   treated as failures.
2. Automated WCAG checks on representative lesson, component, project, and
   reference pages at desktop and mobile sizes.
3. Keyboard-only testing of navigation, menus, copy controls, and downloads.
4. Screen-reader review of reading order, code, tables, and diagram
   descriptions using Orca or NVDA with a supported browser.
5. Checks at 200 and 400 percent zoom, in forced-colors or high-contrast mode,
   with reduced motion, and in grayscale print preview.
6. PDF metadata, text extraction, tagging, reading-order, and print checks.

Report accessibility problems through the repository issue tracker with the
page, browser or reader, expected behavior, and observed behavior. Safety
problems should be marked clearly and handled before ordinary presentation
issues.
