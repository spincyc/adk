# Lesson 013 delivery plan: DHT11 climate adapter

Status: executable delivery plan; implementation and hardware acceptance open.

This plan promotes the existing `ClimateSensor`, `ClimateSample`, validation,
and recorded-source model into a physical Mega 2560 lesson. It does not change
those public contracts. The new adapter translates one selected DHT11 protocol
into that model; it does not make `ClimateSensor` depend on Arduino timing or
claim that every temperature/humidity sensor uses the same transport.

## Delivery boundary

Land lesson 013 after the existing climate-model boundary and before the LCD
work in lesson 014:

```text
ClimateSample model
    -> timed one-wire transport
        -> Dht11Sensor adapter
            -> narrative Mega example
                -> HTML reference + printable experiment
```

The boundary consists of:

- `src/dht11_sensor.h` and `src/dht11_sensor.cpp`;
- a small timed, bidirectional digital-line seam if measurement proves it
  cannot remain private to the adapter;
- deterministic protocol and lifecycle tests;
- `examples/Lesson013Dht11Climate/Lesson013Dht11Climate.ino`;
- `site/pages/lessons/013.md`;
- `docs/lessons/013/main.tex` and
  `docs/lessons/assets/013-dht11-climate-pencil.png`;
- build registration, firmware-size evidence, and an open bench card.

Do not add an external DHT library, heap allocation, exceptions, hidden clock,
blocking `delay()`, LCD code, record formatting, or environmental-station
policy. A short, bounded microsecond transaction may occupy the processor; its
duration and interrupt policy must be measured and documented. Scheduling
between transactions remains nonblocking and uses the caller's `TimePoint`.

## Reference circuit

Use the bare four-pin ASAIR DHT11 when its identity can be verified. A kit
module is acceptable only after its schematic identifies any onboard pull-up.

| Circuit item | Mega 2560 connection | Contract |
|---|---|---|
| DHT11 VDD | `5V` | One board supply; never feed `Vin` |
| DHT11 DATA | `D22` | Adapter owns one exclusive digital pin claim |
| DHT11 NC | unconnected | Do not ground or drive |
| DHT11 GND | `GND` | Common reference |
| DATA pull-up | VDD to DATA | Exact value selected from the identified part/module documentation |
| RGB red | `D5` through 330 ohm | PWM status channel |
| RGB green | `D6` through 330 ohm | PWM status channel |
| RGB blue | `D7` through 330 ohm | PWM status channel |
| RGB common cathode | `GND` | Indicator safe state is off |
| `TP13` | DHT11 DATA at sensor pin | Protocol observation point |

The delivery must record the exact sensor or module part number before choosing
the pull-up and before making any range, accuracy, cable-length, or power-up
claim. Do not stack an external pull-up blindly on a module that already has
one. Wiring is checked with USB power removed.

`D22` is deliberately outside the Mega's external-interrupt pin set. The first
implementation measures edges by bounded polling so the lesson does not imply
that DHT11 requires an interrupt resource. The adapter must claim `D22` once
and change only its electrical direction during the transaction. It must never
drive the pulled-up line high:

```text
request     output low
release     input/high impedance
receive     input/high impedance
shutdown    input/high impedance
```

Resource-acquisition evidence and electrical safe-state evidence remain
separate. The RGB waiting pattern shows successful initialization. A meter or
logic analyzer at `TP13` verifies that shutdown releases the data line.

## Adapter contract

The public header stays declarative. Constants that depend on the selected
datasheet live in the implementation or a protocol configuration value used by
tests.

```cpp
struct Dht11Sensor final : ClimateSensor
{
    Dht11Sensor  (ResourceRegistry& resources,
                  PinId            dataPin) noexcept;
    ~Dht11Sensor () noexcept override;

    Dht11Sensor            (const Dht11Sensor&) = delete;
    Dht11Sensor& operator= (const Dht11Sensor&) = delete;
    Dht11Sensor            (Dht11Sensor&&)      = delete;
    Dht11Sensor& operator= (Dht11Sensor&&)      = delete;

    Status        initialize () noexcept override;
    void          shutdown   () noexcept override;
    Status        update     (TimePoint now) noexcept override;
    ClimateSample sample     (TimePoint now,
                              Duration  staleAfter) const noexcept override;

    PinId dataPin     () const noexcept;
    bool  initialized () const noexcept;
};
```

Construction is inert. `initialize()` validates D22's digital input/output
capability, claims it, leaves it high impedance, resets the snapshot to
`Unavailable`, and is idempotent. It does not claim the sensor responded.
Failure makes no hardware write and leaks no claim.

`shutdown()` first releases the wire to high impedance, clears the snapshot to
`Unavailable`, releases the claim, and is idempotent. Destruction calls
`shutdown()`. No callback occurs during cleanup.

`update(now)` has these observable outcomes:

| Condition | Return | New snapshot |
|---|---|---|
| Adapter is not initialized | `NotInitialized` | unchanged |
| Minimum acquisition interval has not elapsed | `Ok` | unchanged |
| Complete frame, checksum and ranges valid | `Ok` | new `Valid` sample stamped with `now` |
| Sensor response or bit edge exceeds its bound | `HardwareFailure` | `TransportTimeout` |
| Forty bits arrive but checksum differs | `HardwareFailure` | `ChecksumFailure` |
| Decoded value violates selected limits | `InvalidArgument` | corresponding range state |

The first update after initialization is eligible only after the selected
part's documented power-up stabilization interval. That eligibility must be
represented using caller-supplied time, not `delay()`. Early calls are
successful no-ops and cannot manufacture a timestamp.

`sample(now, staleAfter)` applies the existing freshness rule without touching
hardware. A transport fault is never relabeled stale. A successful later frame
recovers normally. Numeric fields in a fault snapshot are diagnostic only;
consumers must branch on `state`.

## Protocol engine and timing

Split the implementation into two testable mechanics:

1. a bounded line transaction records forty high-pulse widths and explicit
   transport failure;
2. a pure decoder converts those widths and five bytes into a
   `ClimateSample`.

The production transport uses microsecond resolution. Millisecond
`TimePoint` remains the scheduling and observation timestamp; it is not
precise enough to decode DHT11 pulses. Introduce the smallest injectable timing
seam needed for host replay. Do not put `micros()` in `ClimateSensor`, and do
not expose raw Arduino functions in the public climate model.

The initially coded timing windows must be named, conservative bounds derived
from the selected DHT11 manual. The commonly published ASAIR transaction is
the verification starting point, not an unreviewed magic-number license:

- MCU request low for at least 18 ms, then release the line;
- sensor acknowledgement is approximately 80 microseconds low and
  80 microseconds high;
- each of 40 bits begins with an approximately 50 microsecond low;
- the following high pulse is approximately 26--28 microseconds for zero and
  approximately 70 microseconds for one;
- the five bytes are humidity integer, humidity decimal, temperature integer,
  temperature decimal, and checksum;
- checksum is the low eight bits of the sum of the first four bytes.

Every wait has a finite deadline. Sampling loops cannot wait forever for an
edge. The threshold separating zero from one is configured between the
documented windows and tested at both neighboring counts. Interrupt masking,
if measurement shows it is required, is confined to the shortest receive
window and restored on every exit path. Its effect on `millis()`, PWM status,
and other timers must be measured before promotion.

Before implementation, resolve one protocol ambiguity against the exact sensor
revision: published DHT11 manuals differ in how decimal/sign bytes are
described. Save the selected manual in the acceptance references, encode its
actual byte interpretation, and add its sample frames as tests. Never infer
DHT22 signed/fractional behavior or advertise DHT22 compatibility.

## Deterministic tests and fault injection

Host tests use a fixed-capacity scripted line/timer fake. A script contains
direction changes, levels, pulse durations, and injected operation failures;
production and tests exercise the same transaction and decoder.

Required cases:

- invalid and unsupported pin, busy pin, and resource-capacity exhaustion;
- inert construction, initialize success, repeated initialize, failure
  rollback, repeated shutdown, destruction while active, and resource reuse;
- no hardware operation before a successful claim;
- exact first-acquisition and minimum-interval boundaries;
- interval scheduling across 32-bit millisecond rollover;
- valid zero and one pulses at every accepted boundary;
- pulse immediately outside every acknowledgement and bit window;
- timeout waiting for initial response, each transition, and final bit;
- all-zero and all-one payloads, checksum carry, and checksum mismatch;
- selected manual's integer/decimal/sign rules;
- temperature and humidity range boundaries and one step outside each;
- last-known-good, fault replacement, stale boundary, and fault recovery;
- high-impedance release after success and after every injected transport exit;
- identical operation, status, and sample traces on two replays;
- injected pin-mode, read, timing, and release failures where the platform seam
  can report them.

Test injection is compile-time host infrastructure, not a learner-facing
backdoor in the Mega sketch. The lesson's physical fault experiments are
reversible: disconnect DATA for timeout and corrupt a scripted host frame for
checksum failure. Do not teach a learner to short DATA, VDD, or an output.

## Narrative Mega example

The canonical example uses only `Dht11Sensor` and the already-supported
`RgbLed`. Objects appear in dependency order:

```cpp
adk::Runtime     runtime;
adk::Dht11Sensor climateSensor (runtime.resources (), 22);
adk::RgbLed       statusLed     (runtime.resources (),
                                 {5, 330},
                                 {6, 330},
                                 {7, 330});
```

`setup()` reads as acquire, configure, start:

```text
initialize sensor
    -> initialize evidence LED
        -> show waiting-for-first-frame
```

If RGB acquisition fails after sensor acquisition, setup shuts the sensor down
before halting. If sensor acquisition fails, the built-in LED on D13 may show
only the initialization failure fallback; the lesson must say that this does
not prove DATA is electrically safe.

`loop()` reads as observe, decide, actuate:

```cpp
const adk::TimePoint now = adk::TimePoint (millis ());

observeClimate (now);
const adk::ClimateSample observation = decideClimateState (now);
showClimateState (observation.state, now);
```

Use those same verbs in the HTML, PDF, and diagram. No Serial output is
required and no `delay()` appears. Indicator timing advances from `now` and
does not trigger extra sensor reads.

The visible state vocabulary is:

| State | RGB evidence |
|---|---|
| Resource acquired, no frame yet | slow blue pulse |
| Fresh valid frame | steady green |
| Transport timeout | two red pulses, repeating |
| Checksum failure | three red pulses, repeating |
| Decoded range failure | three red pulses followed by amber |
| Valid sample stale by lesson policy | steady amber |
| Halted after setup failure | D13 repeating fast pulse if available |

Patterns must also be distinguishable by count or cadence in grayscale and by
learners who cannot distinguish color. `TP13` remains the primary protocol
evidence: predict request/response, observe the pulse train, then interpret
whether transport—not physical accuracy—worked.

## HTML and PDF division

`site/pages/lessons/013.md` is the searchable operational reference. It
contains:

- the exact API and lifecycle/status tables;
- selected-part limits and protocol-window table with source links;
- pin-by-pin wiring and pull-up decision;
- RGB pattern key and `TP13` interpretation;
- links to header, source, host tests, example, PDF, Mega pinout, and bench
  card;
- CLI commands: focused host test, sanitizer gate, Mega compile, size check,
  PDF build/check, site check, and optional serial monitor only as supporting
  evidence;
- an explicit banner: **host verified; hardware acceptance open** until the
  bench card is complete.

The rich PDF is a complementary printable lab, not copied HTML. It includes:

- a pencil-style orientation drawing of the Mega, DHT11 viewed from the front,
  pull-up, RGB resistors, common ground, and labeled `TP13`;
- adjacent prose and a pin-by-pin table so the image is not the only wiring
  source;
- a protocol timing sketch with request, acknowledgement, zero, and one;
- predict/observe/interpret worksheets for acquisition, first valid frame,
  disconnected DATA, recovery, staleness, shutdown, reset, and power removal;
- a fault-diagnosis flow separating resource failure, transport failure,
  checksum failure, decoded range failure, and stale policy;
- short exercises to classify recorded frames and choose freshness policy;
- a bench acceptance record with instruments, sensor identity, supply, pull-up,
  observed timings, shutdown level/direction, and deviations;
- complete metadata, meaningful link text, code as text, grayscale-legible
  symbols, logical reading order, and image alternative text per
  `docs/PDF_POLICY.md`.

The drawing must not imply measured pulse widths. Label nominal/source-derived
timing separately from blank fields for observed minimum and maximum.

## Build, size, and acceptance gates

Register lesson 013 only when its files exist:

```text
LESSONS  += 013
EXAMPLES += Lesson013Dht11Climate
```

Add focused host and sanitizer targets, the example to Arduino compilation,
the PDF source/asset dependency, the HTML route, and link checks. The public
header must compile alone. `Adk.h`, `library.properties`, package smoke tests,
and API/support tables change in the integration commit, not in the component
commit.

Use an initial firmware budget of 16,384 flash bytes and 768 static-RAM bytes.
This is a ceiling, not evidence. After `make size` measures the canonical Mega
build, add its exact toolchain-qualified row to `docs/size_baseline.tsv`; then
`make size-check` must compare against the measured baseline. Any timing seam
or virtual dispatch cost remains only if the map and size report justify it.

Non-hardware promotion gates:

1. style and standalone-header checks;
2. ordinary and sanitizer host tests, including deterministic replay;
3. Mega 2560 compilation with warnings treated as errors where supported;
4. Arduino lint and package-consumer smoke test;
5. measured size report within the stated ceiling;
6. reproducible PDF build, extraction/font checks, strict site/link checks;
7. documentation status consistently says hardware acceptance is open.

Deferred bench acceptance must later record, without fabrication:

- exact DHT11/module manufacturer, part number, revision, and primary manual;
- unpowered continuity and pull-up measurement;
- Mega supply voltage and instrument models;
- power-up wait and acquisition cadence;
- request, acknowledgement, zero, and one timing ranges at `TP13`;
- valid, disconnected-DATA, recovery, stale, reset, shutdown, and power-off
  observations;
- D22 high impedance after every shutdown/failure path;
- RGB pattern interpretation without Serial;
- flash/RAM result from the released toolchain;
- every deviation between the selected manual and the measured specimen.

Until that record exists, the only valid status is **host verified; Mega
compiled; hardware acceptance open**.

## Primary references

- [ASAIR DHT11 product page](https://www.aosong.com/en/Products/info.aspx?itemid=2257)
  identifies the manufacturer and is the starting point for obtaining the
  exact revision's manual. Because its download availability may change, the
  accepted sensor manual and checksum must be recorded with the bench card.
- [Arduino Mega 2560 Rev3 hardware page](https://docs.arduino.cc/hardware/mega-2560)
  supplies the official board resources, pinout, schematic, and board
  datasheet.
- [Arduino Mega 2560 Rev3 datasheet](https://docs.arduino.cc/resources/datasheets/A000067-datasheet.pdf)
  supplies the board-level voltage, clock, memory, and I/O context.
- [Microchip ATmega2560 product page](https://www.microchip.com/en-us/product/atmega2560)
  links the current complete MCU datasheet and errata used to verify GPIO and
  timing implementation constraints.

The DHT11 timing values above remain provisional until checked against the
primary manual for the exact physical sensor. A reseller-hosted copy may aid
discovery but cannot replace that recorded source at promotion.
