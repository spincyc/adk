# Firmware size budgets

ADK treats flash, static RAM, stack, and object size as interface constraints.
The reference target is the Arduino Mega 2560 (`arduino:avr:mega`): 253,952
bytes of application flash and 8,192 bytes of SRAM. A successful link is not a
size acceptance result.

Budgets below are first-pass ceilings. They are intentionally generous until a
reproducible baseline exists. Once measured, a ceiling may be reduced without
an API change; increasing one requires a reviewed budget change and evidence.

## Global limits

| Resource | Supported ceiling | Reserved headroom |
|---|---:|---:|
| Linked application flash | 196,608 B | 57,344 B for bootloader boundary, growth, and user code |
| Static SRAM (`.data` + `.bss`) | 4,096 B | 4,096 B for stack and transient locals |
| Estimated peak stack | 1,536 B | 2,560 B beyond static data and estimated stack |
| Largest supported object | 128 B | Prevents accidental state aggregation |
| Largest project-owned fixed buffer | 512 B | Requires a named purpose and boundary tests |

No supported library path may allocate dynamically. `new`, `delete`, `malloc`,
`calloc`, `realloc`, owning standard containers, exceptions, RTTI, and hidden
function-local static state are prohibited. A non-owning view may refer to
caller storage. Fixed-capacity storage must expose exhaustion as a `Result`.

Virtual dispatch is prohibited in endpoints, components, and deterministic
engines. Platform seams use compile-time binding or an explicit table of
non-owning function pointers. A virtual interface requires measurements against
the alternative, a lifetime proof, and an approved change to this document.

## Incremental library budgets

Measure each row as the linked delta between a minimal sketch that exercises
the public behavior and the same sketch without that layer. Compiler, core,
FQBN, flags, and sketch structure must be identical. Dead, unreferenced code
does not count as delivered firmware.

| Layer or component | Flash delta | Static SRAM delta | `sizeof` ceiling |
|---|---:|---:|---:|
| Core values: `Status`, `Result`, time | 768 B | 8 B | 8 B each |
| Mega profile and resource registry | 1,536 B | 192 B | 64 B registry |
| `DigitalOutput` | 768 B | 8 B | 16 B |
| `MonoLed` | 512 B | 4 B | 24 B |
| `DigitalInput` | 896 B | 8 B | 16 B |
| `Button` | 1,536 B | 16 B | 48 B |
| Scheduler and blink behavior | 1,280 B | 32 B | 64 B |
| `PwmOutput` and `RgbLed` | 1,792 B | 16 B | 64 B |
| `AnalogInput` and scalar sensors | 1,280 B | 16 B | 40 B |
| Tone endpoint and piezo component | 2,048 B | 32 B | 48 B |
| Bus owner plus one device adapter | 2,560 B | 64 B | 96 B |
| Deterministic engine, excluding buffers | 4,096 B | 96 B | 128 B |

One instance is measured unless the component contract says otherwise. Also
measure two instances: the second instance may add state, but should not add a
second copy of code or constant tables.

Object-file size is a diagnostic, not a substitute for linked size. For each
layer, the sum of `.text`, `.rodata`, `.data`, and `.bss` in its AVR objects
must remain below twice its linked-delta ceiling. A larger object usually means
dead code, an oversized table, or a misplaced implementation and requires an
explanation.

## Project budgets

Project limits are absolute linked totals, including the Arduino core and ADK.
Static SRAM is the linker-reported `.data` plus `.bss`; stack is reviewed
separately from maximum call depth and local storage.

| Integrating project | Flash | Static SRAM | Estimated stack |
|---|---:|---:|---:|
| Reaction timer | 12 KiB | 512 B | 384 B |
| Light-and-color instrument | 16 KiB | 768 B | 512 B |
| Simon | 20 KiB | 1,024 B | 512 B |
| Traffic controller or lock simulator | 24 KiB | 1,024 B | 640 B |
| Environmental station or data logger | 40 KiB | 2,048 B | 768 B |
| Operator panel or rover dashboard | 56 KiB | 2,560 B | 1,024 B |
| Greenhouse controller | 64 KiB | 3,072 B | 1,024 B |
| Inert show-cue simulator | 80 KiB | 3,584 B | 1,280 B |

A lesson example should be much smaller than its nearest project ceiling.
Debug text and serial diagnostics count; acceptance measurements use the
documented release configuration, not a hand-edited quiet variant.

## Measurement contract

The build system will provide these stable targets:

```text
make size          # build every supported AVR example and write reports
make size-check    # compare reports with committed baselines and ceilings
make size-update   # refresh baselines; never run implicitly
```

`make size` writes:

```text
build/size/avr.tsv
build/size/objects.tsv
build/size/types.tsv
```

`avr.tsv` contains, in order:

```text
target  fqbn  compiler  flash_bytes  data_bytes  bss_bytes  static_ram_bytes
```

`objects.tsv` records section sizes for every ADK AVR object. `types.tsv`
records `sizeof` and alignment for every public owning type using an AVR-built
measurement program. Output is sorted, tab-separated, locale-independent, and
contains byte counts rather than formatted percentages.

The implementation may locate the ELF through Arduino CLI build properties,
then uses the AVR tools supplied by the Arch Arduino toolchain:

```sh
avr-size -A build/arduino/<target>/<target>.ino.elf
avr-nm --print-size --size-sort --radix=d build/arduino/<target>/<target>.ino.elf
```

Section accounting is:

```text
flash      = .text + .data
static RAM = .data + .bss
```

Record `arduino-cli version`, the resolved FQBN, AVR compiler version, and core
version beside each report. Clean and rebuild before an accepted measurement.
Do not compare Arduino CLI summary strings, host objects, stripped archives, or
ELFs produced by different toolchains.

Committed baselines live in `docs/size_baseline.tsv`. Every baseline row names
the source commit and toolchain. The first implementation of a component adds
its baseline and must satisfy the ceiling in this document.

## CI policy

`make size-check` fails when:

- any global, layer, object, type, or project ceiling is exceeded;
- flash or static SRAM grows by more than 64 B and more than 2% from baseline;
- a public owning type grows at all without an updated baseline;
- a report, supported target, compiler identity, or baseline row is missing;
- forbidden heap, exception, RTTI, or virtual-dispatch symbols appear.

Changes below both the 64-byte and 2% thresholds are reported but do not fail.
A reduction updates the baseline in the same component or project commit so the
budget ratchets downward. A justified increase includes the before/after
reports, cause, alternatives considered, and the narrow budget adjustment.
Never approve a baseline update whose only explanation is “the build passes.”

CI should scan linked symbols and disassembly for allocation and exception
runtime entry points, including operator `new`/`delete`, `malloc` family,
`__cxa_throw`, `__cxa_allocate_exception`, `__cxa_pure_virtual`, and type-info
symbols. The compile flags remain `-fno-exceptions -fno-rtti`; symbol scanning
guards prebuilt dependencies and accidental platform calls.

Stack estimates are review evidence until AVR stack instrumentation is added.
Projects using interrupts must include interrupt frames, nested-call policy,
and every automatic buffer in that estimate. Hardware acceptance should fill
unused SRAM with a sentinel and record the observed high-water mark before a
project is called size-verified.

