---
title: "Noncanonical draft — Lesson 038 acoustic envelope"
---

# Noncanonical design draft — Lesson 038 acoustic envelope

> **Not published or supported.** This file is retained under
> `docs/design/drafts/` for architecture review only. It is not a canonical
> lesson page, downloadable artifact, Mega example, electrical schematic,
> support claim, or physical acceptance record.

Energy class: **E0 copied observations; physical module gated**  
Safety boundary: **relative evidence only; no recording, SPL, or surveillance**

This lesson turns supplied ADC and optional threshold traces into one bounded
event:

`supplied trace → quiet baseline → event window → unitless intensity`

`AcousticEnvelope` is pure behavior. It owns no endpoint, pin, interrupt,
sampling timer, ADC reference, heap, callback, or hidden clock. A physical
microphone module remains outside scope until its exact revision, AO/DO roles,
rails, bias/amplifier, output topology, and authoritative schematic are
qualified.

The pure core and focused host tests are implemented. The endpoint-owning Mega
adapter, exact specimen, electrical schematic, board example, size evidence,
and physical evidence remain future gated work. This design draft does not
promote the implemented core into a published lesson.

## Baseline and amplitude

The first healthy, non-clipped sample establishes the baseline and begins
calibration. During calibration and quiet, the integer estimate follows:

`baseline += (raw - baseline) / 2^baselineShift`

Signed division truncates toward zero. The baseline freezes during an event and
refractory interval. `amplitude = abs(raw - baseline)`.

Attack requires amplitude at or above a configured offset. Release requires
amplitude at or below a smaller offset, so noisy samples near one boundary do
not chatter the event open and closed.

No event is qualified before calibration completes. A rail sample cannot
establish the first baseline; clipping during calibration faults and requires
reset.

## One bounded event window

Attack opens a window and retains the greatest supplied amplitude. The window
closes at the earlier of:

- continuous quiet for `quietToClose`; or
- exact `eventWindow` expiry.

Refractory starts on completion and prevents another event until its exact
boundary. Event flags describe one update only.

At completion, the component maps retained peak above attack into an exact
integer from 0 to 1000 using the configured rail margin and current baseline
headroom. That value is **unitless within one configuration**. It is not dB
SPL, loudness, frequency content, force, damage, or a value that can be
compared across modules, gain settings, rooms, or baselines as a physical
scale.

## Snapshot evidence

| Field | Meaning |
|---|---|
| `raw`, `baseline`, `amplitude` | supplied ADC value and derived offset |
| `peakAmplitude` | greatest supplied amplitude in the open window |
| `eventStarted`, `eventCompleted` | one-update transition evidence |
| `eventStartedAt`, `eventDuration` | bounded interval over supplied times |
| `relativeIntensity` | completed-event value, 0–1000 and unitless |
| `rawThresholdActive` | copied optional comparator evidence |
| `phase`, `quality`, `status` | whether copied values are qualified |

Before a completed event, intensity, start, and duration have canonical zero
values. Status and quality must always travel with copied evidence.

## Clipping and threshold disagreement

`raw <= railMargin` is low clipping; `raw >= 1023 - railMargin` is high
clipping. Clipping cannot open or extend an event. A failed analog source is
not interpreted. If threshold input is configured, it must also be healthy.

After calibration, the optional digital threshold is compared with the analog
authority: active should agree with amplitude at or above attack. One mismatch
is visible but does not fault. Continuous mismatch for `quietToClose` becomes
`ThresholdDisagreement`. The comparator never replaces the analog envelope for
event timing or intensity; threshold-only specimens are unsupported.

Fault recovery requires `reset()` and a fresh calibration interval. The
behavior never silently recalibrates.

## Exact legal phase, quality, and status tuples

| Phase | Quality | Status | Event-field rule |
|---|---|---|---|
| `Calibrating` | `Unqualified` | Ok | event fields canonical zero |
| `Quiet` | `ValidQuiet` | Ok | event fields canonical zero |
| `EventOpen` | `ValidEvent` | Ok | live start/peak; completion and intensity false/zero |
| `Refractory` | `ValidEvent` | Ok | completion update publishes nonzero duration |
| `Refractory` | `ValidQuiet` | Ok | completion snapshot cleared; new attack blocked |
| `Fault` | `ClippedLow` or `ClippedHigh` | `InvalidArgument` | event and intensity fields cleared |
| `Fault` | `ThresholdDisagreement` | `HardwareFailure` | event and intensity fields cleared |
| `Fault` | `SourceFault` | exact non-Ok endpoint status | event and intensity fields cleared |
| `Fault` | `SourceFault` | `HardwareFailure` | configured threshold presence mismatch; event and intensity fields cleared |
| `Fault` | `SourceFault` | `InvalidArgument` | malformed runtime threshold enum; event and intensity fields cleared |
| `Fault` | `TimingFault` | `InvalidArgument` | last completed evidence retained |
| `Fault` | `Unqualified` | `InvalidArgument` | raw above 1023 or invalid runtime headroom; event and intensity fields cleared |

No other tuple is legal. Exact source status means the non-Ok analog status,
or the configured threshold status after a healthy analog sample. A malformed
runtime threshold `Level` is instead
`Fault/SourceFault/InvalidArgument`. A supplied raw value above 1023 and
invalid runtime headroom are both specifically
`Fault/Unqualified/InvalidArgument`; unlike a timing fault, it clears event
and intensity fields and requires reset and recalibration.

Invalid construction policy is not a runtime tuple: `initialize()` returns
`InvalidArgument`, leaves the behavior uninitialized, and does not mutate the
snapshot into `Fault`.

## Transition order

After the initialized-state check, each update applies this validation
precedence:

1. timestamp validity;
2. malformed configured runtime threshold enum;
3. analog status;
4. configured-threshold presence agreement;
5. configured threshold status;
6. raw range `0..1023`;
7. clipping;
8. calibration;
9. sustained threshold disagreement;
10. event close;
11. event open; and
12. baseline update.

Identical same-time frames are idempotent. Changed same-time evidence,
backward apparent time, and jumps at least the unsigned half-range fault
without partial mutation. Natural 32-bit rollover remains valid.

## Predict, observe, interpret

1. Establish quiet at midrail; predict zero amplitude on the first healthy
   sample and no event before the exact calibration boundary.
2. Supply slow quiet drift; predict the baseline’s exact integer steps.
3. Supply attack minus one, exact attack, and attack plus one; predict which
   frames can open a window.
4. Supply equal positive and negative excursions; predict equal amplitude.
5. Retain a larger peak, then close once by continuous quiet and once by
   maximum-window expiry.
6. Exercise exact low/high clipping boundaries; predict fault and canonical
   zero event evidence.
7. Compare one threshold mismatch with sustained disagreement.
8. Place a narrow pulse wholly between supplied timestamps; predict that the
   trace contains no evidence of it.

## Sampling and privacy boundary

This is a polled model. Its update interval is part of the evidence. Activity
entirely between samples may be missed. Supplied points do not establish
bandwidth, frequency response, alias rejection, onset between samples, or
continuous capture.

The component stores bounded state and completed relative evidence only. It
records no waveform, audio, or speech and performs no speech recognition,
surveillance, medical inference, impact grading, or safety-alarm function.
No SPL claim is permitted.

Use host traces until one exact amplified microphone module is electrically
qualified. Never ask for a loud or startling stimulus. Stop for an unidentified
part, out-of-rail output, unexpected reset, unstable rail, heat, odor,
excessive current, or specimen/schematic disagreement.

## Implemented core evidence and design sources

Focused host fixtures cover calibration, integer baseline steps, attack and
release edges, positive/negative peaks, intensity endpoints, quiet and
maximum-window close, refractory, clipping, source faults, malformed runtime
threshold evidence, raw range, threshold agreement, time edges, lifecycle, and
byte-identical replay. These are not endpoint-adapter, Mega, microphone, ADC
bandwidth, privacy, acoustic, SPL, or physical acceptance evidence.

- Implemented pure-core contract: `src/acoustic_envelope.h`; it is not a
  published lesson or physical support claim
- Future gated work: endpoint-owning adapter, transactional acquisition,
  exact-specimen Mega example and schematic, size evidence, and bench record
- Governing design record:
  `docs/design/LESSONS_037_039_PERCUSSION_PLAN.md`
- Governing policies: `docs/PDF_POLICY.md`, `docs/TESTING.md`, and
  `docs/SAFETY_MODEL.md`

This draft intentionally has no previous/next navigation, download link,
canonical sketch link, or publication claim.
