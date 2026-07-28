# PDF publication policy

ADK publishes lesson PDFs as printable companions to the HTML lessons. HTML is
the primary accessible format until a PDF passes both automated and human
accessibility review.

## Current guarantee

The current `pdflatex` pipeline provides:

- a title, author, subject, and stable creation date;
- embedded fonts with Unicode mappings where the selected font permits them;
- searchable, copyable text;
- deterministic inputs and `SOURCE_DATE_EPOCH`;
- two TeX passes, bounded output size, and automated Poppler validation;
- a final TeX log without overfull boxes, undefined references, undefined
  citations, or an unresolved rerun request.

It does **not** produce tagged PDF. `pdfinfo` currently reports `Tagged: no`.
Therefore ADK does not claim PDF/UA or WCAG conformance for these files.

## Author requirements

Every lesson source must:

- set `pdftitle`, `pdfauthor`, `pdfsubject`, and `pdflang`;
- use real section headings and lists instead of visual imitations;
- preserve a logical reading order;
- use link text that makes sense without surrounding prose;
- avoid conveying state through color alone;
- repeat table headers and keep tables simple;
- give every meaningful image concise contextual alternative text;
- mark decoration as an artifact;
- explain wiring diagrams in adjacent prose;
- keep code available as text, not only as an image;
- include a hardware-independent HTML route to the same learning outcome.

### Visual language

Every lesson PDF visual that is not a formal electrical schematic uses a
pencil-drawing presentation. This includes physical layouts, orientation
plates, staged build progress, state and timing diagrams, conceptual flows,
charts, troubleshooting illustrations, and other diagrammatic teaching art.
Grayscale, a `pencil` filename, or a rasterized clean vector drawing does not
by itself satisfy the rule; the rendered visual must visibly use the pencil
language while keeping labels and connections legible.

A formal electrical schematic is the only exception. It uses conventional
component and net symbols, is explicitly identified as electrically
authoritative, and may retain precise schematic drafting. A pictorial
breadboard layout, block diagram, pin locator, waveform, or state machine is
not a formal schematic.

Immediately before every visual source construct, authors classify it with one
of these comments:

```tex
% ADK visual: pencil
\includegraphics{assets/example-pencil.png}

% ADK visual: schematic
\begin{circuitikz}
```

`make lessons-check` rejects missing classifications and rejects a plain
`tikzpicture` claiming the schematic exception. Review still inspects the
rendered PDF: the marker records intent and cannot prove that art looks like a
pencil drawing or that a schematic is electrically correct.

Alternative text describes what the learner needs from an image, not every
stroke. A pencil wiring diagram also needs a nearby pin-by-pin connection list.

## Build and inspect

Use only repository targets for released artifacts:

```sh
make lessons
make lessons-check
```

Inspect every generated file:

```sh
for pdf in doc/lessons/*.pdf
do
    pdfinfo "$pdf" |
        grep -E '^(Title|Subject|Author|Tagged|Pages|Encrypted):'
    pdffonts "$pdf"
    pdftotext -layout "$pdf" - >/dev/null
done
```

`make lessons-check` automates nonblank metadata, page and encryption state,
font embedding, nonempty text extraction, final-pass TeX warnings, and
monochrome output. Review `pdffonts` manually as well: each used font should
normally have a Unicode map (`uni: yes`). Also inspect page order, clipping,
contrast, links, text selection, keyboard navigation, reflow or high zoom, and
grayscale printing. Test with at least one screen reader before making an
accessibility claim.

Poppler checks syntax and extraction; it does not establish PDF/UA conformance.
A `Tagged: yes` result also does not prove correct semantics or reading order.

## Reproducibility

`SOURCE_DATE_EPOCH` removes wall-clock variance. Byte-identical output is
expected only with the same sources, assets, engine, fonts, TeX packages, locale,
and build command. Record the Arch package versions with a release:

```sh
pacman -Q poppler texlive-basic texlive-latex \
    texlive-latexextra texlive-latexrecommended texlive-pictures
pdflatex --version | head -1
```

Check a clean rebuild without replacing the first result:

```sh
make lessons
sha256sum doc/lessons/*.pdf > build/adk-pdf-before.sha256
make -B lessons
sha256sum -c build/adk-pdf-before.sha256
```

A mismatch is a release failure until explained. Package upgrades can
legitimately change output; pin the CI image or archive the package-version list
when long-term byte reproduction matters.

## Tagged-PDF migration

Arch provides current LaTeX tagging support through `texlive-latexextra`;
`texlive-luatex` supplies the preferred engine for new tagged documents. Do not
switch the release pipeline merely because a source compiles. Migrate one lesson
first, audit every package it uses, then make the engine change at a documented
commit boundary.

The candidate source begins before `\documentclass` with current LaTeX metadata:

```tex
\DocumentMetadata
{
    lang        = en-US,
    pdfstandard = ua-2,
    tagging     = on
}
```

Use `lualatex`, add `alt={...}` or `artifact` to every
`\includegraphics`, and resolve every tagging warning. Validate the structure
tree with a PDF/UA-aware validator, then perform the manual tests above.
Validator success is necessary, not sufficient: PDF/UA does not assess every
WCAG concern, including whether the author chose usable color and contrast.

ADK will describe a PDF as accessible only after recording:

1. the exact engine and package versions;
2. the requested PDF standard;
3. validator name, version, profile, and passing report;
4. keyboard, zoom/reflow, text-extraction, and screen-reader results;
5. a human review of headings, lists, links, figures, tables, code, and reading
   order.

Until then, release notes must say **tagged-PDF preview**, not **accessible PDF**
or **PDF/UA conforming**.

## References

- [LaTeX tagged-PDF usage guide](https://latex3.github.io/tagging-project/documentation/usage-instructions)
- [LaTeX tagging documentation and package status](https://latex3.github.io/tagging-project/documentation/)
- [Arch TeX Live packages](https://archlinux.org/groups/x86_64/texlive/)
- [Arch Poppler package](https://archlinux.org/packages/extra/x86_64/poppler/)
- [PDF/UA context and limitations](https://pdfa.org/resource/iso-14289-pdfua/)
