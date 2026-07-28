---
title: "Noncanonical draft — Lesson 039 percussion sequencer"
---

# Noncanonical design draft — Lesson 039 percussion sequencer

> **NONCANONICAL DESIGN DRAFT — NOT PUBLISHED.** This review file does not
> create a lesson route, PDF download, Mega example, schematic, physical
> acceptance record, or support claim.

Status: **host-independent design draft; publication and physical gates open**

Energy class: **E0 project engine and supplied four-lane traces**  
Safety boundary: **logical grouping and presentation intents only**

Lesson 039 composes qualified contact and acoustic records without owning
hardware:

`qualified contacts → bounded group → one acoustic association → quantized hits → playback frame`

The four surfaces are logical lanes in supplied host traces. They are not a
claim that four physical contact specimens, a microphone module, or a
percussion instrument has been identified or tested. Relative intensity is not
sound pressure, loudness, force, damage, speech content, or a safety measure.

## Group and associate

The first consumable contact attack starts recording and freezes the recording
tempo and grid epoch. Up to one attack from each of four surfaces enters one
bounded simultaneous group. An attack exactly at the simultaneous-window
boundary belongs to that group.

Only one closed group may wait for association. New attacks during that wait
set `hitSuppressed`; they are not queued. The first eligible completed acoustic
window whose inclusive interval contains the first attack supplies one
relative intensity to every member. At the association timeout boundary,
timeout wins and finalizes the group with intensity zero.

The project input is a deliberately narrow evidence transfer:

| Evidence | Meaning |
|---|---|
| `attackMask` | bits 0–3 are copied qualified attacks |
| `surfaceStatus[4]` | exact upstream status for each logical lane |
| `acousticStatus` | exact upstream acoustic-source status |
| `acousticCompletion.present` | whether one copied completion is supplied |
| completion start, duration, intensity | bounded interval and relative intensity |

The sequencer does not ingest or reinterpret component snapshots. The adapter
publishes qualified attack bits and optional eligible completion data while
preserving source statuses. The first failing lane is attributed as
`Surface0`…`Surface3`; acoustic, timing, tempo, and malformed project evidence
have distinct `PercussionFaultSource` values. Fault clears frame and tone
intents immediately while retaining finalized hits for diagnosis.

## Quantized pattern

The pattern contains 4–16 sixteenth-note steps at 30–240 BPM:

`stepPeriodMs = floor(60000 / tempoBpm / 4)`

Each group time is rounded to the nearest step relative to the recording epoch;
an exact half-step rounds forward. Group members are stored in ascending
surface order. Different surfaces may share a step. Repeating one surface at
the same step is suppressed.

Fixed storage holds 32 finalized hits. Capacity is atomic after duplicates are
removed: if the complete new group cannot fit, no member is inserted and no
ordinal advances. Existing hits never move or change.

`hit(index)` returns a copied `Result<PercussionHit>` for an index below
`hitCount`, and `InvalidArgument` otherwise. Each copied hit carries
`AcousticCompletion` or `AssociationTimeout` provenance. `lastAssociation`
is transient finalization evidence, not durable per-hit provenance.

## Playback

Play toggles Recording and Playing. Empty playback is an idempotent no-op.
Each start publishes step zero immediately; each restart begins again at step
zero.

Playback position derives from supplied elapsed time, not update count. If an
update crosses several boundaries, the engine skips directly to the frame
current now. It never emits missed light or tone cues late.

Each frame contains:

| Value | Narrow intent |
|---|---|
| `surfaceMask` | logical surface LEDs active for the current frame |
| `intensity[4]` | retained relative intensity for each current hit |
| `frequencyHz` | fixed lane cue: 262, 330, 392, or 523 Hz |
| `toneDuration` | smaller of 60 ms and half a step |
| `heartbeat` | changes once per beat, including silent pattern regions |

The strongest current hit selects the tone; equal intensity selects the lowest
surface. A frame with no hit publishes frequency and duration zero. That is a
**model-level silence intent**, not evidence that a physical piezo, speaker,
board, or room is silent.

## Control precedence

Clear dominates play and attacks at the same valid timestamp. It erases hits,
epochs, frame, and ordinals. Invalid evidence still outranks clear, so a
control cannot conceal a fault.

The deterministic update order is:

1. configuration, time, project evidence, and tempo validation;
2. clear;
3. association timeout, then eligible completion;
4. play toggle;
5. attacks, group closure, and capacity;
6. tempo application and playback boundary.

Same-time identity compares semantic fields: time, attack mask, all four
surface statuses, acoustic status, completion presence/start/duration/intensity,
tempo, play, and clear. It never compares padding or an upstream object image.
An identical record is idempotent. Any changed semantic field, apparent
half-range time, or backward non-wrapping time faults without mutation.
Natural unsigned rollover remains valid.

## Replay worksheet

1. Replay one qualified attack with one matching acoustic completion. Predict
   one quantized hit with the supplied relative intensity.
2. Group surfaces zero and three. Predict ascending surface storage.
3. Put an attack exactly at the group boundary, then one tick later. Predict
   inclusion first and suppression second.
4. Omit acoustic completion. Predict timeout intensity zero without claiming
   a physical sound was quiet or inaudible.
5. Test one tick before, exactly at, and after a half-step quantization tie.
6. Start playback and inspect step zero immediately.
7. Jump across several playback boundaries. Verify no missed LED or tone intent
   is emitted late.
8. Clear with simultaneous play and attack. Predict empty Recording unless
   invalid evidence wins.
9. Inject a lane status failure. Predict its exact status and surface
   attribution, retained finalized hits, and zero cue intents.
10. Shut down after finalization. Predict cleared pending, transient, and frame
    state plus `NotInitialized`, while hit count, indexed hit copies, and next
    ordinal remain inspectable and survive reinitialization.

Seed-free host fixtures cover lane combinations and permutations, grouping and
association provenance, quantization ties, tempo endpoints, atomic capacity,
clear dominance, sparse playback, rollover, attributed failures, lifecycle
retention, and byte-identical traces. A versioned fieldwise golden includes
every snapshot scalar, fault and association enum, every frame field and four
frame intensities, and every indexed copied hit with provenance. It catches
semantic field regressions without depending on padding bytes.

## Draft and physical boundary

This hardware-independent draft supplies no Mega sketch, electrical schematic,
pin authorization, specimen wiring, or physical playback claim. Exact
contacts, microphone, potentiometer, display, and passive piezo require primary
sources, electrical qualification, current budgets, an authoritative
schematic, and recorded human bench acceptance before any powered build.

Review this draft against the Lessons 037–039 design record,
`docs/TESTING.md`, and `docs/SAFETY_MODEL.md`. There is deliberately no
canonical Lesson 039 route, downloadable PDF, sketch link, or lesson
navigation. Promotion requires relocation into canonical paths and every
applicable component, lesson, publication, and physical-evidence gate.
