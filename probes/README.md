# Resource probes

Resource probes preserve aggregate firmware evidence that no single lesson
example can represent. They are compile-only fixtures, not hardware acceptance
evidence.

Run the Lessons 052--054 infrared probe with:

```sh
make ir-resource-check
```

The command starts from the canonical Lesson 054 sketch and injects
`ir_translator_maximum_composition.inc`, which adds the promoted Lesson 025
receiver footprint. A fresh Mega 2560 build must measure 21,864 B flash and
3,531 B static SRAM.

The same command compiles `ir_translator_object_size.cpp` with the AVR compiler
and reads its symbol size with `avr-nm`; `InertIrTranslator` must occupy 407 B.
It also compiles the relevant sources with `-fno-lto -fstack-usage`. The
conservative live-stack bound is 888 B:

```text
522 B loop
+ 210 B InertIrTranslator::prepareTranslation
+  89 B KnownIrEmissionPolicy::prepare
+  64 B interrupt reserve
+   3 B return-address allowance
```

The script uses tools discovered beside the compiler recorded by
`arduino-cli`, checks every result exactly, and removes its temporary build
directory.

Run the Lessons 055--057 escape-console probe with:

```sh
make escape-console-resource-check
```

The probe builds each available canonical Mega fixture with LTO and jump
tables disabled and statically reserves outgoing arguments. It retains a link
map and combines compiler stack-usage records with the linked AVR call graph.
Direct calls add the Mega's three-byte return address; tail jumps add none.
Reachable dynamic or unresolved transfers, recursion, and non-static stack
records fail the probe instead of becoming estimates.

The JSON evidence in
`build/evidence/escape-console-resource-probe.json` records the exact commands,
tool versions, flash, static SRAM, object sizes, synchronous stack path, and
every target and hard-gate disposition. Lesson 057 additionally reserves 128 B
for interrupt context and must leave at least 2,048 B of Mega SRAM. Until the
Lesson 057 implementation and its real maximum-composition fixture both
exist, that boundary is explicitly `pending`; use `--require-complete` for a
promotion gate that rejects pending evidence. The Make target enables that
promotion behavior.

A target miss is `review-required`, not a hard-gate exception. The checked-in
`escape_console_resource_reviews.json` may accept only an exact
lesson/metric/observed/target/hard tuple and must cite its controlling design
authority. A changed measurement makes that review stale and fails the probe.
Reviewed target misses permit the command to succeed; hard-ceiling and
residual-SRAM failures are never reviewable.

Run the Lessons 058--060 display-timing probe through the current promoted
boundary with:

```sh
make display-timing-resource-check
```

The current promotion gate requires Lessons 058--060. It compiles each canonical
Mega replay with LTO and jump tables disabled, measures the linked flash and
static SRAM, derives the exact reachable synchronous stack from compiler
records and the AVR call graph, and reads the policy object size from
`display_timing_object_sizes.cpp`. Evidence is written to
`build/evidence/display-timing-resource-probe.json`.

The script accepts `--require-through 058`, `--require-through 059`, and
`--require-through 060` to select a promotion boundary. A required lesson must
have its canonical fixture.
Target misses require an exact, authority-backed entry in
`display_timing_resource_reviews.json`; changed measurements invalidate the
review, and hard-ceiling failures remain non-reviewable.

Run the museum-case probe through Lesson 063 with:

```sh
make museum-case-resource-check
```

The promotion gate compiles the canonical resistive-probe, thermal-radiant,
and complete museum-monitor replays, measures their linked flash and static
SRAM, derives conservative synchronous-stack bounds from compiler records and
the AVR call graph, and reads public value and policy object sizes from
`museum_case_object_sizes.cpp`. The component-only exact fixtures live under
`extras/probes/` so Arduino package lint does not mistake them for public
examples; the Lesson 063 maximum-composition gate deliberately builds its
canonical narrative example and audits each required linked storage symbol.
Evidence is written to
`build/evidence/museum-case-resource-probe.json`. An exact, authority-backed
entry in `museum_case_resource_reviews.json` is required for any target miss;
hard ceilings and residual-SRAM floors remain non-reviewable. Fingerprints are
lesson-scoped so adding a later boundary does not invalidate an unchanged
earlier review.
