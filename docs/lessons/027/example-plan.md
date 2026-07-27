# Lesson 027 canonical example plan

Canonical sketch: `examples/Lesson027TelemetryConsole/Lesson027TelemetryConsole.ino`

The sketch uses `TelemetryConsole` and `TelemetryConsoleProject` directly.
Its local adapters implement the public operator, presentation, and record
interfaces; they do not duplicate the console policy.

## E1 circuit

- Mega 2560 powered only by USB.
- `Next` button on D30 and `Acknowledge` button on D31, each wired to GND and
  sampled with the internal pull-up.
- Source-health RGB LED on D5--D7, one 330 ohm resistor per channel.
- Record-acceptance LED on D13; each accepted bounded record toggles it.
- Three compiled-in, owned observation fixtures. No receiver, RF transmitter,
  or live unidentified-radio requirement.

An optional display and bounded sounder are extension exercises. They are not
part of the starter pin or timer claim.

## Object narrative

Declare objects in this order:

1. runtime and resource registry;
2. operator buttons;
3. source-health RGB presentation;
4. bounded record sink and acceptance LED;
5. telemetry console;
6. project coordinator;
7. fixed fixture observation storage.

`setup()` must read as acquire, configure, start. `loop()` must retain these
four visible phases:

```cpp
observeTelemetrySources (now);
decideConsoleState      (now);
presentConsoleState     (now);
recordConsoleEvidence   (now);
```

The sketch uses the canonical interface directly. It does not hide source
order, synthesize missing sources as healthy, or let presentation or record
failure alter observation health.

## Deterministic fixture

Use three configured source slots:

- source 101 temperature;
- source 202 relative humidity;
- source 303 contact state.

The sketch drives starting, one-by-one recovery, healthy, aging/degraded,
stale/fault, and recovery at fixed millisecond boundaries. The buttons add
selection and acknowledgement events. Host tests own exact repeated replay,
fault injection, and golden-record proof.

## Circuit-native evidence

- RGB/cadence: blue starting, green healthy, amber degraded, red fault, dark
  stopped. Cadence must distinguish states without color.
- D13: toggle only after a complete record is accepted by the bounded sink.
- TP-R/G/B and TP-REC: named electrical observation points at the corresponding
  Mega pins.

Resource acquisition and safe shutdown are separate experiments. The
acquisition sequence uses the blue startup pattern. Safe state requires direct
measurement after shutdown: evidence outputs inactive and released, and no
claim retained.

## Required implementation checks

- zero, maximum, and over-capacity source configurations;
- ordering independent of packet arrival;
- duplicate identity and missing-slot rejection;
- health precedence for simultaneous faults;
- next edge, held next, wrap, invalid chord, acknowledgement, and
  reannouncement;
- exact age and heartbeat boundaries across timestamp wrap;
- golden record, smallest buffer, one-byte-short buffer, bounded retry, and
  restart;
- display and storage faults never alter source health;
- shutdown from every health state;
- two byte-identical replays.

The bench record remains blank until a person observes the exact released
sketch on a Mega 2560. The sketch makes no physical multi-room claim.
