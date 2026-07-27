# Development contract

This file is the integration contract for people and agents extending ADK. The
supported API is developed from the resource layer upward. Imported preview
code belongs under `legacy/`; it may demonstrate history, but no supported
header, example, lesson, or test may depend on it.

## First-class architecture

ADK has four layers. Dependencies point downward only.

| Layer | Owns | Examples |
|---|---|---|
| Platform | Board operations and capabilities | clock, GPIO driver, Mega 2560 profile |
| Resource | Exclusive or shared hardware claims | pin claim, timer claim, bus owner |
| Endpoint | Electrical configuration and lifetime | `DigitalOutput`, `DigitalInput`, `PwmOutput` |
| Component | Circuit meaning and deterministic behavior | `MonoLed`, `Button`, `RgbLed`, `Simon` |

Components contain endpoints. Endpoints contain claims and refer to a platform
driver. Neither inheritance nor global registration substitutes for ownership.
Use runtime polymorphism only when measured composition cannot express a
substitutable capability within the flash and RAM budget.

### Lifetime

Every exclusive owner is inert after construction, non-copyable, and initially
non-movable. Its common contract is:

```cpp
Result initialize  ();
void   shutdown    () noexcept;
bool   initialized () const;
```

`initialize()` acquires every required resource before exposing active
behavior. Any failure restores the pre-call state and releases claims acquired
by that attempt. Repeated initialization reports success without reconfiguring
hardware. `shutdown()` is idempotent. Destruction calls `shutdown()`.

ADK does not throw, allocate from the heap, depend on RTTI, or hide time in an
internal clock. Its destructors remain safe during exception-driven unwinding
in a caller. Cleanup cannot invoke user callbacks.

Generic output endpoints become high impedance on shutdown. A semantic
component first selects its documented inactive electrical state, then releases
its endpoint. Board capability checks happen before a claim or hardware write.

### Result and resources

`Result` is a small value with an inspectable status; it carries no allocation
or text. Statuses distinguish at least invalid configuration, unsupported
capability, busy resource, and platform failure.

A claim registry has fixed capacity and deterministic lookup. Exclusive claims
cannot overlap. A failed multi-resource acquisition releases the earlier
claims in reverse order. Shared buses have one owner; devices receive an
explicit non-owning relationship whose lifetime cannot exceed the bus owner.

### Determinism

Time enters through `update(TimePoint now)`. Tests supply timestamps and input
samples. Event snapshots remain stable until the next update and are not
consumed by observation. A recorded seed, configuration, timestamps, and
samples reproduce behavior byte for byte. Production and test code use the same
state machine.

## Dependency and commit sequence

Each numbered boundary is a buildable commit or a short, reviewable commit
series. Do not land a consumer before its dependency.

1. Move imported headers, sources, tests, sketches, and lessons to `legacy/`.
   Keep a separate opt-in legacy build; remove legacy includes from the
   supported aggregate header.
2. Add `Status`, `Result`, fixed-width time values, and platform driver
   interfaces.
3. Add Mega 2560 capability data and the fixed-capacity resource registry.
4. Add `DigitalOutput` first, including safe shutdown and a recording host fake.
5. Add `MonoLed`, so every later circuit has a visible diagnostic output.
6. Add `DigitalInput`, raw sampling, pull policy, and host fault injection.
7. Add `Button`, deterministic debounce, edges, release gating, and chords.
8. Build project 1: a deterministic reaction timer.
9. Add nonblocking scheduling and reusable blink behavior.
10. Add `PwmOutput` and `RgbLed`.
11. Add `AnalogInput`, potentiometer, and photoresistor.
12. Build project 2: a deterministic light-and-color instrument.
13. Add tone/timer ownership and a piezo sounder.
14. Add the deterministic Simon engine and project.
15. Continue in three-lesson blocks: two component lessons followed by one
    integrating project.

Candidate later projects include a traffic controller, combination-lock
simulator, environmental station, data logger, operator panel, rover dashboard,
greenhouse controller, and inert show-cue simulator. Each project must reuse
supported components and add no hidden hardware access.

Commit subjects use a terse boundary and purpose, for example:

```text
legacy: isolate preview api
core: add status and time
gpio: add digital output
led: add mono led
project: add reaction timer
```

Never combine a hierarchy change, lesson rewrite, and unrelated build cleanup
in one commit. Generated PDFs may accompany their lesson source. A migration
commit may remove a legacy dependency only after its supported replacement
passes every gate.

## Component deliverables

Every endpoint or component lands with:

- a declarative public header and out-of-line implementation;
- ownership, initialization, failure, shutdown, and timing contracts;
- host fakes plus success, failure, lifetime, and boundary tests;
- a minimal Mega 2560 example and measured flash/RAM size;
- terse HTML reference with links to source, example, tests, and related work;
- a complementary PDF lesson with schematic, pencil orientation drawing,
  experiment, diagnosis, exercises, and acceptance record;
- a hardware checklist that distinguishes compilation from observation.

Every third lesson is an integrating project. Project tests include a replayable
happy path, every state transition, timeout and rollover boundaries, invalid
input, platform failure, restart, and shutdown from each active state.

## Exact acceptance gates

Run these gates in order. Record the commands, tool versions, board, result, and
measured sizes in the change or lesson acceptance record.

### Source gate

- Formatting and repository style checks report no findings.
- Host compilation uses warnings as errors, no exceptions, and no RTTI.
- Public headers compile alone and contain no avoidable implementation.
- Supported source contains no include or symbol from `legacy/`.
- Copy and move traits match the documented ownership contract.

### Correctness gate

- Host tests pass under the ordinary build and an available sanitizer build.
- Initialization success, repeated initialization, every injected failure
  point, repeated shutdown, destruction while active, and reuse after
  destruction are tested.
- Failed initialization produces no leaked claim or hardware operation after
  rollback.
- Deterministic components pass the same trace twice with identical output.
- Timestamp wraparound and fixed-capacity exhaustion are explicit tests.

### Arduino gate

- Every supported example compiles for `arduino:avr:mega`.
- Arduino lint reports no error for the supported library and examples.
- Flash and static RAM do not exceed their documented budgets.
- The example contains no dynamic allocation, blocking delay in reusable
  behavior, or undocumented global ownership.

### Hardware gate

- Wiring is checked unpowered against the schematic.
- Startup, normal operation, initialization failure, shutdown, reset, and power
  removal are observed on a Mega 2560.
- Output shutdown is measured at the pin, not inferred from an LED.
- The acceptance record names instruments, supply, pins, observations, and any
  deviation. Without this record, status is “host verified,” not “hardware
  verified.”

### Publication gate

- PDF builds without missing assets, overflow, or blank required metadata.
- HTML and PDF links, fragments, images, source downloads, and examples pass
  the site checker.
- HTML supplies concise reference and searchable procedures; PDF supplies the
  richer printable experiment. Together they contain every required fact.
- Diagrams remain legible in grayscale and at 200% zoom; keyboard navigation
  and heading order are checked.
- The strict site build and Pages workflow pass before the interface is marked
  supported.

### Release gate

- The worktree is clean and commits follow the dependency order above.
- Full host, Arduino, lesson, site, and lint targets pass from a clean build.
- The version, changelog, API status tables, roadmap, and deferred-work ledger
  agree.
- A fresh consumer project can include the library and build one supported
  example without a repository-relative path.
- The published Pages URL and each current lesson download return success.

## Agent handoff

An agent claims one bounded deliverable and lists the files it may edit before
working. Parallel agents must not edit the same file. Shared indexes, build
fragments, navigation, generated artifacts, and status tables have one named
integrator.

A handoff contains:

1. scope and dependency commit;
2. files changed and files intentionally untouched;
3. contract decisions and unresolved risks;
4. exact validation commands and results;
5. measured flash, RAM, PDF pages, and artifact sizes when applicable;
6. follow-up work, with blockers distinguished from enhancements;
7. the proposed terse commit subject.

Agents do not weaken tests to satisfy an implementation, silently revise a
public contract, commit another agent's partial work, push, publish, or mark
hardware acceptance without physical evidence. If a prerequisite changes, the
agent stops at a clean boundary and notifies the integrator.

The integrator reviews dependency direction, public names, resource rollback,
shutdown behavior, deterministic traces, documentation links, and generated
artifacts before committing. Failed gates remain visible in the deferred-work
ledger with an owner and next action.

## Definition of supported

An interface is supported only when all component deliverables and all
applicable acceptance gates pass. Compiling code is “experimental.” Host-tested
code is “host verified.” Mega-observed code is “hardware verified.” Only a
published, documented, hardware-verified interface is “supported.”
