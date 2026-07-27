# HDMI mesh observability

Status: design guidance; physical and interoperability acceptance deferred  
Scope: dynamically routed HDMI receiver and transmitter endpoints over a
switched media network

## Evidence contract

Each source-side HDMI receiver and destination-side HDMI transmitter is a
separately named endpoint. Either may join, leave, fail, or be reassigned
without renumbering the other. A route is identified by immutable endpoint
identities and the authoritative controller's monotonically increasing route
epoch, never by an IP address, switch port, connector label, or transient HDMI
link state.

The initial design uses one durable authoritative controller running as a
service on an ordinary Linux computer. It never carries media. Controller high
availability, consensus, and automatic failover are deferred. Endpoint agents
report observations but never grant routes or advance epochs.

An active route requires the controller, receiver, media transport, and
transmitter to agree on:

```text
source identity -> destination identity -> source epoch
    -> sink reservation -> media description
```

The media description includes video format, audio format, clock domain,
metadata set, protection state, and transport profile. A stale epoch, missing
reporter, contradictory format, or expired observation clears `Active` and
produces `Degraded`, `Fault`, or `Unassigned`. The system never guesses that a
picture is correct because a sink displays something.

Keep five proof planes distinct:

1. control proof: the controller authorized one current source epoch;
2. HDMI input proof: receiver HPD, DDC/EDID, training, and decode are healthy;
3. network proof: the expected media flows arrive with acceptable timing;
4. reconstruction proof: the transmitter generated and trained the declared
   HDMI output;
5. content proof: known video, audio, timing, and metadata survived the tested
   path.

No plane proves another. Input lock does not prove packet delivery. Packet
delivery does not prove output training. Output lock does not prove correct
pixels or audio. A route record does not prove that any cable, clock, sink, or
source is healthy.

## Endpoint observations

### Receiver endpoint

Record these observations independently:

- connector presence and debounced HPD state;
- DDC transaction health and the exact EDID identity/hash presented upstream;
- SCDC state and TMDS or FRL training state, negotiated lanes, and rate;
- recovered pixel or stream clock, measured rate, lock age, and discontinuities;
- active width, height, total timing, frame rate, scan mode, sampling, depth,
  colorimetry, quantization, and HDR metadata presence;
- audio sample rate, channels, format, clock lock, and discontinuities;
- InfoFrame and metadata validation failures;
- input frame sequence and CRC over a documented unprotected pixel domain;
- media packetization state, sequence errors, and transmit timestamp.

`HPD high`, `DDC complete`, `trained`, and `media valid` are separate states.
The EDID that the mesh presents to a source is a route policy result. Its hash
and generation belong in the route trace so a format change cannot masquerade
as an unrelated cable event.

### Network path

For every video, audio, and ancillary flow, record:

- flow identity, source epoch, transport profile, multicast or unicast address,
  and expected media description;
- packets and bytes sent/received, sequence gaps, duplicates, reorder depth,
  late packets, malformed packets, and discarded stale-epoch packets;
- PTP or other media-clock lock, offset, uncertainty, grandmaster identity,
  and domain;
- sender timestamp, receiver timestamp, measured path delay, jitter, and
  receive-buffer occupancy;
- switch ingress/egress rate, queue drops, congestion marks where available,
  and path changes;
- FEC corrections, uncorrectable blocks, redundant-path disagreement, and
  protection-switch events when those features exist.

Metrics must identify the measurement clock. Wall-clock timestamps help an
operator correlate events but do not establish media ordering.

### Transmitter endpoint

Record these observations independently:

- sink presence, HPD state, DDC transaction health, and native sink EDID hash;
- selected source epoch and the media description accepted from the network;
- receive-buffer minimum, maximum, current occupancy, underflow, and overflow;
- reconstructed pixel and audio clocks, lock age, adjustments, and
  discontinuities;
- generated video timing, sampling, depth, colorimetry, quantization, HDR, and
  InfoFrames;
- TMDS or FRL training state, negotiated lanes and rate, and retraining count;
- output frame sequence and CRC over the same documented unprotected pixel
  domain used at the receiver;
- audio samples presented, mute state, discontinuities, and generated test-tone
  sequence;
- local test-pattern, border/overlay, and fault-substitution state.

The transmitter reports `Active` only when it holds the current epoch, receives
fresh media, reconstructs the declared clocks and formats, and has a trained
sink link. If policy permits an active route without an attached sink, label
that condition `Armed`, not `Active`.

Its local display names requested and applied profile, link rate, latency
class, route state, and exact fault using text or symbols independent of color.
A pinned-profile contract loss blanks and mutes rather than silently degrading.

Protected-content handling belongs only in a licensed HDMI/HDCP repeater
design. Do not log keys, decrypted payloads, authentication secrets, or
protected pixel CRCs. Research evidence uses generated, public-domain, or
otherwise authorized unprotected material.

## State and route indication

Use the same state names in endpoint firmware, controller events, the CLI, and
the local panel.

| State | Required meaning |
|---|---|
| `Unassigned` | endpoint is known but has no current route |
| `Requested` | route is authorized but no media claim is made |
| `Negotiating` | EDID, input training, flow setup, or output training is active |
| `Active` | all required reporters agree on one fresh epoch and description |
| `Degraded` | media may continue, but one bounded health requirement failed |
| `Fault` | route correctness cannot be claimed; fault substitution is active |
| `Stale` | last agreement exceeded its freshness deadline |

Dynamic reassignment is break-before-make. The prior transmitter clears
`Active`, stops accepting the old epoch, and acknowledges release before the
new transmitter may accept the advanced epoch. Late packets and confirmations
from an earlier epoch are counted and discarded.

## Mega 2560 evidence panel

The Mega may operate a bounded local observation panel over a checksummed,
versioned register snapshot from the FPGA or SoC. It does not authorize routes,
process media, participate in DDC or training, generate media clocks, or hold
HDCP material.

Each endpoint panel provides:

| Indicator | Receiver meaning | Transmitter meaning |
|---|---|---|
| `Presence` | source connector and HPD observed | sink connector and HPD observed |
| `Hdmi` | input trained and decoded | output trained to sink |
| `Network` | controller session and media path fresh | controller session and media path fresh |
| `Clock` | recovered input and media clocks valid | media and generated output clocks valid |
| `Route` | current route state and epoch suffix | current route state and epoch suffix |
| `Activity` | bounded frame/audio pulse | bounded frame/audio pulse |
| `Fault` | local, route, timing, thermal, or format fault | local, route, buffer, timing, or format fault |

The route indication uses color plus cadence and a printed legend:

| State | Indication |
|---|---|
| `Unassigned` | blue, one short pulse |
| `Requested` or `Negotiating` | amber pulse |
| `Active` | steady green only after complete fresh agreement |
| `Degraded` | alternating amber and red |
| `Fault` | steady red |
| `Stale` or identity conflict | three red pulses, pause |

A small display may show stable short endpoint IDs, route state, low-order
epoch digits, video format, link mode, clock state, and a bounded reason code.
It must not show credentials, protected-content details, or private payload
data.

The panel includes a local test-pattern button. The FPGA or SoC, not the Mega,
generates a documented pattern, frame counter, moving marker, audio channel
identification sequence, and optional visible source-epoch suffix. Local pattern
selection is an explicit maintenance state and never silently replaces routed
content.

Named low-speed test points may expose:

- `TP-PPS`: disciplined one-pulse-per-second reference;
- `TP-FRAME`: one bounded pulse per video frame;
- `TP-AUDIO`: divided audio-frame cadence;
- `TP-BUFFER`: receive-buffer threshold alarm;
- `TP-FAULT`: latched endpoint fault;
- `TP-HEARTBEAT`: fresh agent snapshot indication;
- `TP-GND`: measurement reference.

HDMI TMDS/FRL lanes, high-speed reference clocks, and Ethernet serial lanes are
not learner probe points. Use appropriate compliance instruments and fixtures.
The panel and test points remain outside the high-speed media path.

Panel resource acquisition and media safe state are separate proofs. A startup
self-test sweeps the indicators, then reports whether panel resources were
acquired. Loss of snapshot freshness clears green and shows `Stale`.
Panel shutdown makes its own pins safe; it does not claim that HDMI output,
source HPD, sink power, or media processing stopped.

## Known-content verification

Generated evidence must be deterministic and identified by pattern version,
seed, frame number, media description, and source epoch.

The minimum video sequence includes:

- color bars and legal-range markers;
- single-pixel grids, alternating pixels, and sharp text;
- gradients for depth, quantization, colorimetry, and HDR checks;
- moving frame and epoch markers for freeze, reorder, and latency detection;
- noisy and incompressible frames for worst-case transport stress;
- repeated solid frames to expose stale-frame substitution.

The minimum audio sequence includes per-channel identity tones or codes,
silence, full-scale-bounded samples, channel-walk order, and an audio/video
alignment marker. Audio remains within documented safe listening levels at the
actual sink.

CRC comparisons name their exact domain. A receiver-side active-pixel CRC may
be compared directly with a transmitter-side pre-encoding CRC only when both
use identical sampling, depth, range, color representation, metadata policy,
and frame boundaries. Compression, color conversion, scaling, DSC, dithering,
or sink processing requires an appropriate decoded comparison and objective
error metrics instead of a false bit-exact claim.

Measure latency at declared boundaries:

```text
HDMI receiver frame boundary
    -> network ingress
        -> network egress
            -> HDMI transmitter frame boundary
                -> measured photons or audio output
```

Report transport latency, endpoint processing latency, and glass-to-glass
latency separately. CRC agreement proves tested data integrity, not timing.
Latency agreement proves timing for the tested trace, not pixel integrity.

## Events, metrics, and CLI

Each reporter emits versioned, structured events with:

```text
observed-at  event-sequence  endpoint-id  peer-id  source-epoch
state        reason-code     media-description-hash
reporter     freshness       correlation-id
```

At minimum, record endpoint discovery/restart, route request/release, EDID
generation, HPD change, DDC failure, training transition, format change, clock
lock/loss, flow activation, stale epoch, packet error, buffer threshold, output
training, pattern selection, CRC result, audio discontinuity, and fault
substitution. Preserve reporter order; do not rewrite event time to make
distributed traces appear sequential.

Required bounded metrics include:

- known, online, active, degraded, stale, and faulted endpoints by role;
- route requests, activations, releases, moves, conflicts, stale epochs,
  deadlines, and policy denials;
- HPD changes, DDC errors, input/output training attempts and failures, and
  retraining duration;
- negotiated TMDS/FRL modes, video formats, and audio formats using bounded
  enumerations;
- clock-lock age, offset, uncertainty, and discontinuities;
- packets, sequence gaps, late/reordered packets, FEC results, and switch drops;
- buffer occupancy, underflow, overflow, frozen frames, CRC mismatches, and
  audio discontinuities;
- frame, packet, route-transition, and glass-to-glass latency histograms;
- endpoint temperature, power alarms, panel snapshot age, dropped events, and
  journal capacity.

Do not use endpoint IDs, route IDs, EDID contents, IP addresses, filenames, or
other unbounded strings as metric labels. Detailed identities remain in
bounded CLI snapshots and event traces.

All inspection and evidence capture is CLI-driven:

```text
make hdmi-mesh-endpoints
make hdmi-mesh-routes
make hdmi-mesh-route HDMI_SOURCE=<stable-id> HDMI_DESTINATION=<stable-id>
make hdmi-mesh-trace HDMI_ROUTE=<route-id>
make hdmi-mesh-links
make hdmi-mesh-clocks
make hdmi-mesh-crc HDMI_ROUTE=<route-id>
make hdmi-mesh-latency HDMI_ROUTE=<route-id>
make hdmi-mesh-metrics
make hdmi-mesh-watch
make hdmi-mesh-evidence HDMI_RUN=<run-id>
```

Human output never relies on terminal color. Machine output uses a versioned
schema. `watch` prints ordered changes instead of redrawing an ambiguous
dashboard. `evidence` makes a bounded, timestamped, redacted archive and
manifest without changing a route.

Route mutations use plan/apply. A plan names the expected prior epoch, proposed
epoch, receiver, transmitter, EDID policy, media description, flows, detach
requirements, deadlines, and policy result. Apply retains the same correlation
ID. A stale plan fails closed.

## Predict, observe, interpret

| Step | Predict | Observe | Interpret |
|---|---|---|---|
| discovery | one stable endpoint, no active route | endpoint list, panel `Presence`, heartbeat | identity and local connector evidence agree |
| input negotiation | declared EDID leads to one input mode | EDID hash, DDC trace, HPD and training states | receiver negotiated the expected input |
| assignment | old destination releases before new epoch activates | route trace and both panels | exclusivity and fencing were maintained |
| network activation | every expected flow advances in the same epoch | packet sequences, clocks, buffer trend | declared flows reached the transmitter |
| output training | sink link trains in the declared mode | DDC, HPD, TMDS/FRL state, panel `Hdmi` | reconstructed link exists |
| content trial | generated video/audio arrives unchanged or within declared bounds | CRC/error metrics, frame/audio markers | tested content contract passed |
| timing trial | markers advance without freeze and within budget | latency histogram, clocks, frame pulses | tested timing contract passed |
| network loss | freshness expires and green clears | deadline event, stale packets, panel pattern | stale media was not reported active |
| shutdown | panel pins become safe independently of HDMI state | shutdown sequence and low-speed test points | panel ownership and media state are distinct |

Scale tests add and remove unrelated receiver and transmitter endpoints while
known-content routes remain active. Record discovery convergence, route churn,
control-plane latency, tail media latency, multicast state, event loss, and any
unrequested epoch or EDID change.

The fabric is the same managed network used by USB, controller traffic,
telemetry, and the ordinary household LAN. Evidence records admission headroom,
traffic class, cross traffic, and whether a profile was pinned, drawn from an
ordered allowed set, or selected within explicit bounds.

Disabled-by-default deterministic fault injection is separate from production
failure policy. Test evidence records authorization, bounded duration, injected
observation, local `TEST` indication, and proof that a simultaneous real fault
took precedence.

## Deferred physical acceptance

This document specifies evidence and records no bench result. Hardware and
interoperability acceptance remain open until a signed run names:

- endpoint boards, FPGA/SoC images, HDMI IP and license status, controller,
  switch, optics/cables, topology, VLAN/QoS/PTP configuration, and tool
  versions;
- stable endpoint IDs, connector labels, every source epoch, EDID hashes,
  negotiated HDMI modes, media descriptions, and sink/source models;
- panel wiring, resistor values, low-speed test points, instruments, and
  observed panel states;
- generated pattern versions and seeds, CRC domains, objective error results,
  audio channel evidence, latency boundaries, histograms, packet errors,
  buffer behavior, and clock measurements;
- endpoint restart, source/sink disconnect, route move, congestion, clock loss,
  controller loss, thermal/power alarm, shutdown, and power-removal results;
- deviations, reviewer, and date.

Until then the design is **research/model guidance; physical, HDMI compliance,
media-quality, latency, scale, and interoperability acceptance open**. “8K,”
“lossless,” “frame accurate,” “low latency,” “seamless,” and “full HDMI” are
measured, configuration-specific outcomes, never inferred from nominal link
rates or a green panel indicator.
