# Determinism and Testing

ADK code should be replayable on a host computer and understandable from a
trace. Hardware I/O occurs at explicit boundaries; behavior code does not read
an invisible clock or make implementation-dependent random choices.

## Time

Time-dependent behavior receives the current time explicitly:

```cpp
behavior.update (now);
```

The spelling above illustrates the target design; no such general behavior API
is implemented yet. Supplying time makes debounce, timeouts, playback, and
timestamp wraparound testable without real delays.

One update samples inputs once, advances state once, and publishes one stable
output snapshot. Button press and release events are planned as non-consuming
snapshots: multiple observers see the same value until the next update.

## Inputs and traces

A deterministic test trace contains:

- the starting configuration;
- the initial state;
- every update timestamp;
- the complete input snapshot at that timestamp;
- the expected state, events, and outputs after the update.

Ambiguous physical input must become an explicit result. For Simon, two or more
active buttons in the same sample are invalid input; scan order never selects a
winner. A held button cannot generate repeated cues because a complete
press-release cycle is required.

## Repeatable sequences

The planned Simon engine uses neutral cue identities rather than colors or pin
numbers. Hardware mapping assigns each cue to a position, button, LED, and
optional tone.

A production sequence may be pseudorandom, but replay is exact:

- the algorithm has a documented version;
- the seed and version are recorded;
- a given version and seed produce the same cues across supported platforms and
  future ADK releases;
- golden-vector tests lock the sequence;
- host tests may inject fixed sequences directly;
- `std::rand()`, wall-clock seeds, boot noise, and implementation-dependent
  standard-library engines do not define observable game behavior.

## Test layers

| Layer | What it proves |
|---|---|
| Pure host test | State transitions, timing boundaries, errors, and replay |
| Fake Arduino test | Exact pin-mode and I/O calls without hardware |
| Mega compile | AVR compatibility and sketch integration |
| Hardware acceptance | Wiring, electrical behavior, and measured evidence |
| Lesson exercise | A learner can predict, observe, diagnose, and explain it |

Compilation is not hardware acceptance. A component page should state which
layers it has actually passed.

## Simon acceptance set

The planned midpoint project will cover:

- legal state transitions;
- exact cue-on and inter-cue timing;
- ignored input during playback;
- correct presses and required releases;
- simultaneous input, mismatch, and timeout;
- restart and sequence growth;
- fixed-capacity maximum length;
- timestamp wraparound;
- golden seeded sequences;
- replay of timestamped input traces.

Table-driven traces will serve both as executable correctness tests and as
examples of proper API use. The project remains marked planned until those
tests and its hardware adapter exist.
