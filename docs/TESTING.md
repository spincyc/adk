# Testing

ADK tests behavior before hardware. Every result must be reproducible from the
test name, seed, input trace, board profile, and library revision.

## Test pyramid

Run the cheapest proof first:

1. host unit tests for one contract;
2. host composition tests for a complete circuit;
3. recorded trace replay for timing and state-machine behavior;
4. Mega 2560 compilation and size checks;
5. hardware acceptance using the published circuit and checklist.

Host tests are the primary correctness gate. Hardware checks prove electrical
assumptions and integration; they do not replace deterministic tests.

Compile every public header independently before composition tests:

```sh
make headers-check
```

The gate checks every `src/*.h` under strict C++11 with warnings as errors,
exceptions disabled, and RTTI disabled. The host Arduino fake include path
supplies Arduino declarations when a public header needs them. Both
`make check` and `make quality` include this gate.

## Circuit-native observation

Every design has at least one verification path that does not depend on
Serial: an LED, sounder, display, or named point for a meter or logic probe.
Serial output may add timestamps or detail, but it is optional and cannot be
the only hardware evidence.

Write each hardware check as predict, observe, interpret. Name the expected
signal, its location and timing, the actual observation, and what contract the
evidence supports or contradicts. A surprising result becomes a recorded
diagnostic observation, not a pass/fail guess.

Test resource ownership and safe electrical state independently. Successful
initialization or an explicit status signal establishes acquisition. A pin
level, waveform, or high-impedance measurement establishes safe state.

## Fake hardware layer

Production components depend on narrow hardware interfaces, never directly on
Arduino globals. The host fake implements the same interfaces and records:

- pin mode and level changes;
- reads and their configured sequence;
- clock observations;
- resource claims and releases;
- ordered calls with timestamps;
- injected failures and capability mismatches.

The fake starts with every pin unclaimed and high impedance. Tests configure a
board profile explicitly. An unspecified read is an error, not an accidental
default. A test may inspect the ordered operation log, but should prefer public
state unless call order is part of the contract.

Use a manually advanced monotonic clock. Tests must not sleep, read wall time,
or depend on host scheduling.

## Arrange, act, prove

Each test should make four facts obvious:

1. initial hardware and time;
2. stimulus;
3. public result;
4. relevant hardware effects.

Keep one behavioral reason per test. Table-driven cases may cover many values
when they prove the same rule. Name failures in circuit language, such as
`buttonRejectsBounceBeforeDebounceInterval`.

## Trace and replay

A trace is a compact, ordered sequence of timestamped external observations:

```text
0       pin 22 high
1000    pin 22 low
3500    pin 22 high
```

Times are integer ticks in the component clock type. Traces contain inputs, not
expected implementation calls. Replay advances the fake clock, applies all
observations at that tick, calls the circuit update once, and records public
outputs. Equal-timestamp observations retain file order.

Store short, diagnostic traces as readable fixtures. A failing generated case
must print a replayable trace and seed. Replay format changes require a version
field and a reader for every fixture still in the repository.

## Determinism

- Pass time into `update()`; do not read it implicitly.
- Seed pseudo-random behavior explicitly with the documented stable generator.
- Define ordering for simultaneous inputs.
- Use integer arithmetic for timing and thresholds where practical.
- Never derive correctness from iteration order of an unordered container.
- Compare observable state and ordered effects, not addresses or host timing.

The same trace and seed must produce byte-for-byte equivalent observable output
on repeated host runs.

## Timer wraparound

Arduino tick counters wrap. Duration checks use unsigned subtraction:

```cpp
if (static_cast<Tick>(now - previous) >= interval)
{
    // interval elapsed
}
```

Every timed component needs cases immediately before wrap, at wrap, immediately
after wrap, one tick before its boundary, exactly at the boundary, and one tick
after it. Intervals must stay within the documented unambiguous half-range.
Tests also cover repeated timestamps and large valid advances.

## Fault injection

The fake can fail a selected operation or claim. Cover at least:

- unavailable or already claimed resource;
- unsupported board capability;
- failure at each step of multi-resource initialization;
- invalid configuration;
- repeated `initialize()` and `shutdown()`;
- destruction while initialized;
- callback-driven removal during dispatch;
- noisy, stuck, and simultaneous inputs.

After any initialization failure, every earlier claim is released and hardware
is in its documented safe state. `shutdown()` and destructors must not fail,
throw, allocate, or retain claims.

## Tables and generated properties

Use the C++ standard library and the repository test harness. Do not add a test
dependency when a loop and a table communicate the proof.

Table tests cover truth tables, boundaries, board capabilities, and state
transitions. Deterministic generated tests use a fixed small generator owned by
the test suite. Useful properties include:

- initialization owns either all required resources or none;
- shutdown is idempotent;
- inactive semantic outputs never command an active level;
- accepted button presses alternate with accepted releases;
- replay output depends only on configuration, trace, and seed;
- state remains within the documented domain after every update.

Use fixed seeds in normal runs. A longer opt-in run may enumerate more seeds.
On failure, report the seed, minimal useful prefix, and replay trace. Check in a
small regression case after fixing the defect.

## Hardware acceptance

Each lesson provides a Mega 2560 checklist with:

- exact parts, pins, polarity, resistor values, and power limits;
- wiring inspection with power removed;
- expected safe state before and after object lifetime;
- one nominal behavior measurement;
- boundary or fault behavior;
- a non-Serial diagnostic signal or named electrical test point;
- separate resource-acquisition and safe-state observations;
- prediction, observation, and interpretation for each measurement;
- optional Serial evidence when useful;
- shutdown and reset behavior;
- observed result, board revision, tool versions, and date.

Never automate hazardous loads. Motors, relays, radios, and external supplies
require the lesson's isolation and power checks. Fireworks work is limited to
an inert cue simulator and receive-only lawful observation.

Hardware acceptance is recorded evidence, not a source-code assertion. A
skipped physical check must remain visibly incomplete in the lesson.

## Firmware size budgets

Compile every example for `arduino:avr:mega`. Record flash and static RAM from
the toolchain output. Each example has an explicit budget near its build
metadata; the build fails when it is exceeded.

Budgets include margin for toolchain variation and are tightened only from
measured evidence. Report both absolute bytes and percentage of board capacity.
A change that crosses a budget needs either a size-focused fix or a documented
budget decision. Debug-only host facilities must not enter firmware builds.

Review map or symbol output when growth is surprising. Avoid heap allocation,
RTTI, exceptions, unnecessary virtual dispatch, and duplicate inline code in
the supported library.

## Required gates

Before committing a component boundary:

```sh
make check
make arduino
make lessons-check
make site-check
```

The component is incomplete until its contract tests, failure rollback tests,
wraparound tests when timed, example, hardware checklist, HTML reference, and
lesson PDF are present. Projects add composition traces and deterministic
replay fixtures.

CI runs the same commands from a clean checkout. Local bootstrap uses stock
Arch Linux package names, while both local bootstrap and CI install Arduino AVR
core 1.8.8. GitHub Actions are pinned remotely. Tests must not require network
access after dependencies are installed.

Ordinary `site-check` remains offline. After Pages deployment, the publication
workflow separately verifies the live landing page, newest lesson, PDF
signature, and byte-identical canonical sketch with bounded retries.
