---
title: "Lesson 037 — Contact dynamics"
---

# Lesson 037 — contact dynamics

> **NONCANONICAL DESIGN DRAFT — NOT PUBLISHED.** This review file does not
> create a lesson route, PDF download, Mega example, schematic, or support
> claim.

Status: **host-independent draft; exact-specimen gates open**

Energy class: **E0 pure behavior; no powered specimen**

This lesson turns a supplied timestamped level trace into qualified contact
evidence:

`raw level → qualified contact → attack event → release event`

It uses copied `Level`, timestamp, and status values. It does not authorize a
retail module, capsule, contact, piezo element, Mega pin assignment, LED panel,
or powered circuit. Exact-specimen identity, primary electrical evidence, an
authoritative schematic, current accounting, a canonical Mega example, and
physical acceptance remain open.

## What the behavior decides

`ContactDynamics` distinguishes:

- raw activity from continuously qualified activity;
- one accepted attack from chatter around it;
- a qualified release and pulse width;
- a second attack suppressed during refractory time;
- a contact that remains qualified active through the stuck boundary; and
- source or timing faults that must not manufacture events.

A qualified contact is not a force, acceleration, impact-energy, orientation,
damage, or safety measurement. Fixture names describe waveform shapes, not
retail sensors.

## Supplied sample contract

Each `ContactSample` carries one observation time, one copied level, and one
status. The configuration selects the active level and nonzero qualification,
release, refractory, and stuck-active durations.

The snapshot keeps raw and qualified state, one-update attack and release
pulses, pulse width, refractory time remaining, saturating accepted and
suppressed counts, disposition, quality, and status. Re-reading a snapshot
does not consume it.

An attack candidate must remain continuously active through `qualify`. The
event time is the update that proves the interval, not the first raw edge. A
release likewise needs continuous inactivity. One held contact cannot create
repeated attacks.

Refractory begins at an accepted attack. Completion before its exact boundary
is suppressed; completion exactly at the boundary is accepted. The
stuck-active clock also starts at accepted attack.

## Polling limitations

The behavior knows only the samples supplied to it. A pulse that begins and
ends between polls is invisible, and sparse polling cannot reconstruct its
edges. The accepted timestamp is therefore qualification evidence, not a
claim about exact mechanical transition time.

Natural 32-bit rollover is valid. An apparent jump at or beyond the unsigned
half-range is a timing fault. An identical same-time sample is idempotent;
changed evidence at the same time is invalid.

An adapter can also pass a malformed enum value, for example
`static_cast<Level>(7)`. The behavior reports `InvalidArgument` with
`SourceFault`, creates no event, and remains faulted until `reset()`.

## Predict, observe, interpret

Write the predicted snapshot before replaying each trace. Observe every
snapshot row after each timestamp, then interpret only what that evidence
supports.

1. **Short pulse:** keep active time below qualification. Predict no attack
   and no count change.
2. **Chatter train:** alternate levels, then hold active. Predict candidate
   restarts and exactly one accepted attack after continuous qualification.
3. **Refractory pair:** qualify, release, and qualify again before refractory
   expiry. Predict one accepted and one suppressed count. Repeat exactly at
   expiry and predict acceptance.
4. **Held contact:** remain qualified active through the stuck boundary, then
   release. Predict no repeated attack, temporary stuck quality, one pulse
   width, and return to current `Valid` quality.
5. **Source fault:** supply a non-Ok status. Predict no event and require
   `reset()` before qualification resumes.
6. **Replay:** retain configuration, timestamps, levels, statuses, and every
   named snapshot field. Predict complete field-for-field equality from the
   same trace, including `Status::error()`.

These experiments establish deterministic policy for the exact supplied
samples. They do not establish physical edge capture, specimen identity,
electrical safety, a Mega circuit, or bench performance.

## Replay one exact trace

Use this active-low configuration:

| Setting | Value |
|---|---:|
| `qualify` | 30 ms |
| `release` | 20 ms |
| `refractory` | 80 ms |
| `stuckActive` | 120 ms |

Initialize once, then submit these all-Ok samples in order:

| Time | Level | Predict |
|---:|---|---|
| 0 ms | High | idle, Valid, counts 0/0 |
| 10 ms | Low | active candidate starts |
| 20 ms | High | chatter breaks the candidate; no attack |
| 30 ms | Low | candidate restarts |
| 60 ms | Low | accepted attack; counts 1/0 |
| 70 ms | High | release candidate; still qualified |
| 90 ms | High | qualified release; pulse width 30 ms |
| 100 ms | Low | second candidate starts during refractory |
| 130 ms | Low | suppressed attack; counts 1/1 |

Save the complete returned status and snapshot after every row. Reset or
construct a fresh component, replay the same rows, and compare every named
field, including `Status::error()`. Do not compare raw C++ object bytes:
padding has no canonical meaning. Do not compare only event rows.

For the held-contact trace, reset and submit `(0, High)`, `(10, Low)`,
`(40, Low)`, `(160, Low)`, `(170, High)`, and `(190, High)`, all Ok. Predict
acceptance at 40 ms, `StuckActive` at 160 ms, and release with a 150 ms pulse
width at 190 ms.

For the malformed-level trace, reset and submit
`{TimePoint(200), static_cast<Level>(7), StatusCode::Ok}`. Predict
`InvalidArgument`, `SourceFault`, no attack or release, and continued fault on
a later well-formed sample until another reset.

## Safety and deferred work

Keep this edition host-independent. Do not connect or power an unknown
contact, glass capsule, tilt switch, tap/shock module, microphone board, or
bare piezo. A listing name does not establish polarity, pin order, voltage
range, pull-up rail, or safe current.

A later exact-specimen gate must add primary sources, pin-by-pin wiring,
current review, an electrically authoritative formal schematic, a canonical
Mega example, visible non-Serial evidence, safe-state measurements, and a
recorded human bench result.

## Draft references

- [Lessons 037–039 design record](../../LESSONS_037_039_PERCUSSION_PLAN.md)
- [Testing contract](../../../TESTING.md)
- [PDF policy](../../../PDF_POLICY.md)
- [Safety model](../../../SAFETY_MODEL.md)

There is deliberately no canonical Lesson 037 route or downloadable PDF.
