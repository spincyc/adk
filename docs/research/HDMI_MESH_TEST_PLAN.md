# HDMI mesh test plan

> **Authority decision:** initial acceptance assumes one durable authoritative
> controller running on an ordinary Linux computer and never carrying media.
> Replication, quorum, leader election, and automatic controller
> failover are deferred. Controller loss fails closed.

> **Content decision:** research uses generated, synthetic, or explicitly
> unprotected media. HDCP authentication, keys, protected-content capture, and
> repeater conformance are excluded.

Status: research test contract  
Scope: dynamically reconfigurable HDMI source and sink endpoint mesh  
Physical evidence: deferred

## Purpose

The mesh connects any authorized HDMI receiver endpoint to any compatible HDMI
transmitter endpoint through switched IP networking. It terminates each source
HDMI link, interprets video, audio, timing, and metadata, transports those
streams, then constructs a new HDMI link at each sink. It does not forward an
electrical HDMI waveform through Ethernet.

Endpoints may join, leave, restart, change capabilities, or be rerouted while
the service runs. Phase one is one-to-one; phase three adds deliberate fan-out
when policy and bandwidth permit. A sink receives at most one source. Route changes produce explicit
blanking, mute, retraining, and lock states; seamless switching is a separate
declared capability, never an assumption.

The authoritative route model is the control-plane oracle. Media correctness
comes from independent synthetic generators, packet monitors, reconstructed
stream captures, and sink-side analyzers. Endpoint status alone cannot prove
that pixels, samples, timestamps, or metadata are correct.

## Testable route contract

Every route names:

- stable controller, receiver endpoint, transmitter endpoint, HDMI connector,
  sender, flow, and receiver identities;
- controller incarnation and monotonically increasing source epoch;
- separately named sink reservation and its monotonic deadline;
- topology generation and fresh authenticated endpoint sessions;
- exact video format, audio format, metadata set, transport profile, latency
  class, redundancy mode, and required bandwidth;
- pinned, ordered-allowed, or bounded-best profile policy and configured
  failure response;
- EDID policy and the capabilities exposed to the source;
- command identity, logical tick, activation time, deadline, and protocol
  version.

Addresses, switch ports, connector labels, serial numbers reported by a source,
and display names are attributes rather than identities. Address reuse or an
endpoint reconnect cannot revive an old route.

A route progresses through:

```text
unassigned
reserved
source-configuring
network-configuring
sink-configuring
training
locking
active
draining
fault
```

The reference model permits progress only when current sessions are authorized,
advertised capabilities are compatible, bandwidth is reserved, the sink is not
owned by another route, and every term, epoch, generation, activation time, and
deadline is current. A failed or ambiguous operation converges to `unassigned`
or `fault`; it never silently restores or adopts a previous route.

## Observable invariants

Check these after every generated event and message delivery:

- one sink belongs to at most one active or activating route;
- fan-out creates independent sink route state without duplicating source-link
  ownership;
- `active` requires agreement among the controller, both current endpoint
  sessions, the media receiver, and the sink-link transmitter;
- stale terms, epochs, topology generations, sessions, and deadlines cannot
  mutate state or operate media hardware;
- an old stream cannot appear at a newly assigned sink;
- accepted changes are durable before success is exposed;
- rejected changes do not mutate routes, epochs, reservations, or audit state;
- duplicate commands are idempotent and cause no second endpoint action;
- every active flow fits its reserved link, queue, and switch-fabric budget;
- admission failure cannot disturb unrelated active routes;
- rejection of a candidate profile cannot disturb the existing working route;
- no profile changes without the exact configured policy and an audit event;
- video, audio, ancillary data, and route identity refer to the same source
  generation;
- timestamps are monotonic within a flow and use its declared clock domain;
- audio-to-video phase remains within the declared bound;
- invalid, missing, or incompatible metadata is explicit, not fabricated;
- underflow, overflow, loss of timing, or link-training failure selects the
  documented blank/mute/fault output;
- endpoint and controller restart never claim pre-restart media lock;
- bounded tables, queues, counters, epochs, and audit storage fail closed;
- a trace replays byte for byte from its version, configuration, seed, initial
  snapshot, logical timestamps, and samples.

## Deterministic harness

Use a manually advanced unsigned logical clock. The harness contains the real
controller and route reconciler plus:

- fake receiver and transmitter endpoint agents;
- independent EDID, HPD, TMDS/FRL-training, and sink-lock models;
- synthetic video, audio, ancillary-data, and packet generators;
- fake PTP/media clocks with controllable offset, drift, step, and loss;
- a bounded switch-fabric model with deterministic queues and drops;
- fake authenticated transport and durable storage;
- an independent route and bandwidth reference model.

The normal test harness does not sleep, read wall time, open a network socket,
touch HDMI hardware, or depend on thread scheduling. Equal-tick events retain
recorded trace order. Every failure emits the seed, smallest useful prefix,
initial snapshot, capabilities, and a versioned replay trace.

Run every checked-in trace twice. Require byte-identical public snapshots,
endpoint actions, EDID responses, HPD events, network reservations, media
packet records, reconstructed stream digests, persistence writes, and audit
entries.

## Model and property tests

Generate endpoint inventories, capability matrices, EDIDs, policies, routes,
formats, clock traces, packet faults, and control events from fixed seeds. Bias
generation toward exact deadlines, capacity boundaries, endpoint reconnects,
format changes, shared sources, competing sinks, counter rollover, and stale
identifiers.

After every step compare the implementation with the independent model:

- membership, route state, ownership, and required next action;
- accepted status or stable rejection reason;
- source epoch, activation time, deadline, and audit action;
- bandwidth reservations and queue occupancy;
- fake endpoint action logs;
- advertised and selected capabilities;
- media sequence, timestamp, format, and digest summaries;
- durable snapshot visibility.

Required metamorphic properties include:

- changing labels or addresses does not change route semantics;
- permuting unrelated endpoint identities only permutes corresponding output;
- inserting duplicate or stale messages does not change current state;
- reordering independent routes preserves each route result;
- adding an unused endpoint does not change active media;
- fan-out preserves the source stream digest at every lossless sink;
- splitting a packet without changing payload or timestamps preserves decoded
  media;
- changing packet delivery within the allowed jitter window preserves output;
- serialize-and-restart preserves durable intent but converts ambiguous work to
  `fault` or `unassigned`;
- lowering a capability or bandwidth limit can reject affected work but cannot
  degrade a route silently.

Use bounded exhaustive exploration for small topologies and fixed short seed
ranges in normal checks. Longer deterministic campaigns supplement rather than
replace exhaustive schedules. Reduce every discovered failure to a checked-in
regression trace.

## Endpoint and routing scenarios

### Dynamic membership

- Join one receiver and one transmitter, activate a route, release it, and
  reconnect each endpoint with a fresh session.
- Replace an endpoint while retaining its address and display label.
- Add and remove a connector without disturbing unrelated routes.
- Reject simultaneous sessions claiming one stable endpoint identity.
- Change an endpoint between receive, transmit, and combined roles.
- Quarantine and restore an endpoint only through an authorized new source
  epoch and sink reservation.
- Remove a source with one sink and with many sinks.
- Remove one sink from a multicast group without perturbing the other sinks.

### Dynamic routes

- Activate every compatible source/sink pair from an empty mesh.
- Move a sink between sources and fan one source out to several sinks.
- Switch two sinks simultaneously as one scheduled salvo.
- Rotate `N` sources across `N` sinks with deterministic transition order.
- Request a new route during configuration, training, locking, and draining.
- Repeat an active request and prove idempotence.
- Release an unassigned route without changing epochs or endpoint actions.
- Cancel immediately before and at every activation deadline.
- Request mutually incompatible output formats from one source.
- Preserve unrelated routes when one route fails admission or activation.

### Messages and fencing

For every request, action, acknowledgement, and media-lock report, test
duplicate, delayed, dropped, reordered, malformed, wrong-version,
wrong-session, wrong-term, wrong-epoch, wrong-route, wrong-endpoint,
wrong-generation, and post-deadline delivery.

Deliver stale success and fault messages after:

- endpoint reconnect or replacement;
- controller restart;
- route move or release;
- connector removal and recreation;
- topology-generation advance;
- ownership-epoch advance;
- a later route has already achieved media lock.

No stale event may blank, mute, retrain, activate, or relabel the current route.

## HDMI sideband and link tests

### EDID and capability policy

- Parse minimal valid EDIDs and every supported extension-block combination.
- Reject truncated blocks, invalid lengths, bad checksums, contradictory modes,
  unsupported timings, and configured size limits transactionally.
- Intersect source, network, and sink capabilities without inventing a mode.
- Generate a deterministic virtual EDID for one sink and for a fan-out group.
- Exercise policies for common-denominator mode, fixed operator-selected mode,
  and explicit incompatibility.
- Change sink EDID while inactive, activating, and active.
- Preserve the prior durable route intent while reporting renegotiation
  requirements explicitly.
- Ensure an EDID cache is keyed by stable identity and generation, not address.

### HPD, DDC, and negotiation

- Debounce or sequence HPD solely through supplied logical time.
- Exercise HPD low before, during, and after EDID access and link training.
- Reject stale DDC results after HPD or topology generation changes.
- Bound repeated HPD pulses and prevent an endpoint fault from creating an
  uncontrolled pulse loop.
- Verify that route release, endpoint loss, and incompatible capability changes
  produce the documented HPD/blank/mute sequence.

### TMDS and FRL training

- Cover every declared TMDS and FRL operating mode and legal fallback.
- Inject negotiation timeout, lane-lock loss, FEC threshold, clock-recovery
  failure, SCDC error, and unsupported-rate reports.
- Exercise failure at every training transition and exact timeout boundary.
- Never report `active` from network packet flow alone when the sink link is
  unlocked.
- Require a policy decision before reducing resolution, sampling, depth, frame
  rate, or HDR capability; no silent fallback is permitted.

## Media correctness

### Synthetic video corpus

Use deterministic, independently generated frames:

- solid primaries, black, white, and near-black/near-white ramps;
- one-pixel checkerboards, alternating lines, zone plates, and moving edges;
- grids, coordinates, frame numbers, route identity, and timestamp overlays;
- gradients at every declared component depth;
- RGB and each supported YCbCr sampling arrangement;
- limited/full-range boundary values and chroma siting patterns;
- sparse bit walkers and pseudorandom frames from recorded seeds;
- UI text, thin lines, cursor motion, and repeated image generations;
- HDR-oriented ramps and metadata changes without making display-quality claims.

For an uncompressed route, compare active pixels and declared blanking treatment
exactly. For a reversible codec, compare decoded values exactly. For a lossy
mode, publish the selected profile and objective thresholds before testing;
record maximum error and content-dependent failures rather than calling the
result lossless or visually lossless from inspection alone.

Test first frame, steady state, final frame, route transitions, sequence
rollover, dropped and duplicated packets, reordered packets, jitter-buffer
underflow/overflow, and clock discontinuities. A fault output must be
distinguishable from valid black video.

### Audio and synchronization

Generate deterministic silence, impulses, tones, channel identifiers, phase
patterns, maximum legal samples, and pseudorandom sample blocks. Verify sample
value, order, channel mapping, sample rate, packet sequence, mute behavior, and
route identity independently of video.

Embed matching events in audio and video to measure phase using media
timestamps, not host wall time. Exercise:

- start, steady state, route switch, mute/unmute, and drain;
- positive and negative clock drift;
- PTP loss, step, offset, grandmaster change, and reacquisition;
- packet delay, loss, duplication, and reordering;
- audio-only and video-only fault;
- sample and frame counter rollover.

Assert the declared phase bound and monotonic correction policy. Never conceal
an audio discontinuity by resetting the measurement epoch.

### Metadata and ancillary data

For every supported InfoFrame or transported metadata unit, test absent,
minimal, maximum-size, unknown, duplicate, changing, malformed, and conflicting
values. Verify association with the correct video frame and source epoch.

At minimum cover declared handling for colorimetry, quantization, HDR static or
dynamic metadata, active format, content type, audio configuration, and
variable-refresh information. Pass-through modes preserve exact bytes;
interpreted modes preserve declared semantics. Unsupported metadata is dropped
with a visible reason, never converted silently.

CEC is a separate routed control plane. Until explicitly designed and tested,
it remains disabled rather than being bridged opportunistically.

## Bandwidth, buffering, and scale

Admission tests calculate traffic from the exact media profile plus packet,
Ethernet, shaping, redundancy, audio, ancillary, and configured engineering
overhead. Exercise configured-capacity minus one unit, exact capacity, and one
unit over capacity for endpoints, links, queues, multicast groups, and the
whole fabric.

Required topologies are staged. Phase one requires the one-to-one rows; the
fan-out and scaling rows begin in phase three:

| Topology | Purpose |
|---|---|
| 1 source × 1 sink | lifecycle and media baseline |
| 2 × 2 | conflicts, swaps, and scheduled salvos |
| 1 × 8 | multicast fan-out and sink independence |
| 8 × 8 | small matrix and oversubscription |
| 32 × 32 | controller and multicast scaling |
| configured maximum − 1, maximum, maximum + 1 | bounded-capacity behavior |

Use deterministic operation counts, memory bounds, reservation totals, packet
counts, and modeled queue occupancy rather than host elapsed time. Model
oversubscription, microbursts, cross traffic, multicast join delay, queue loss,
and A/B path disagreement. Reconciliation and a topology rescan cannot reorder
routes, change epochs, or interrupt unaffected media.

Host scale evidence validates model and control-plane bounds. It does not prove
8K throughput, FPGA timing closure, switch performance, or optical integrity.

## Fault and recovery matrix

Inject each fault in every applicable route state:

- controller loss or restart;
- either endpoint loss, restart, or stale reconnect;
- durable-write and audit-append failure;
- control, media, timing, and discovery partitions independently;
- HDMI input loss, output link loss, and repeated retraining failure;
- PTP loss, excessive offset, and timestamp discontinuity;
- packet loss, duplication, corruption, reordering, and prolonged jitter;
- buffer underflow, overflow, and impossible occupancy report;
- multicast programming failure and unexpected flow arrival;
- capability, EDID, format, or metadata change;
- thermal, power-good, clock, transceiver, and endpoint watchdog fault;
- route, queue, endpoint, audit, counter, and epoch exhaustion;
- malformed, truncated, incompatible, or inconsistent snapshots.

Restart endpoints before and after each modeled hardware action but before its
acknowledgement. Restart the controller before and after every durable write.
Recover partitions in every order and after deadlines. An unexpected stream or
physical link is quarantined or blanked; it is never adopted as active merely
because it decodes.

Controller loss blocks mutation immediately. Verify that only the exact
unchanged installed route runs through its pre-authorized bounded lease and
that every affected transmitter mutes on expiry. It may not renew, change
profile, expand fan-out, or accept another sink reservation.

The production fault presentation uses HDMI no-signal plus a labelled local
LED/display indication. An explicitly selected maintenance mode may use a
local test pattern or border. Resource/control readiness, network readiness,
media lock, and no-signal/mute state are separate evidence.
The local display names requested and applied profile, link rate, latency class,
failure policy, and exact fault without relying on color.

Fault injection is disabled by default and separately authorized. Every
injected scenario is deterministic, time-bounded, audited as test input, and
shows `TEST` on involved endpoints. Repeat every scenario with a simultaneous
real fault and prove the real observation takes precedence.

## Concurrency schedules

Systematically enumerate all interleavings for two sources, two sinks, two
endpoint reconnects, and two competing operator commands to bounded depth.
Include:

- two sources targeting one sink;
- one source fanning out while another request replaces it;
- assignment racing release, endpoint removal, or EDID change;
- activation deadline racing successful media lock;
- stale lock racing a new route;
- persistence failure after one or both endpoint actions;
- multicast join and leave racing packet delivery;
- controller restart between command and acknowledgement;
- pre-restart commands reaching only one endpoint.

At most one route owns a sink. Losing commands return stable reasons and cannot
perform late endpoint actions. Random longer schedules do not substitute for
this bounded exploration.

## Replay artifacts and acceptance output

Each regression trace records:

- schema and implementation versions;
- seed and initial durable snapshot digest;
- endpoint identities, sessions, capabilities, and EDID digests;
- route commands and exact logical timestamps;
- clock, packet, media, and injected-fault events;
- expected route transitions, endpoint actions, and media digests;
- expected audit-chain digest and final public snapshot.

Keep large synthetic frame bodies generated from the recorded algorithm and
seed. Store compact golden digests and small human-readable witness regions
rather than opaque media captures. A replay tool reports the first differing
pixel, sample, metadata byte, packet field, state transition, or audit entry.

## Physical evidence deferred

Physical work begins only after model, controller, endpoint-agent, persistence,
security, and synthetic media gates pass. A skipped bench check remains open.

Later evidence must record:

- exact source, sink, FPGA/SoC endpoint, switch, optics, cables, firmware,
  licensed IP versions, analyzers, and test-pattern equipment;
- negotiated mode, EDID, TMDS/FRL state, packet rate, clock state, and measured
  bandwidth;
- pixel, audio, metadata, latency, jitter, route-switch, and long-run results;
- cold start, reset, hot plug, cable removal, packet fault, endpoint loss,
  switch congestion, thermal alarm, shutdown, and power removal;
- named indicators and test points proving control readiness, media lock,
  fault presentation, and safe blank/mute independently.

Begin at low resolution with a generated unprotected pattern and PCM audio.
Progress one variable at a time through depth, sampling, rate, metadata,
multicast, compression, and finally 8K. Host tests and endpoint status cannot
be relabeled as HDMI, network-throughput, image-quality, or interoperability
evidence.

## Explicit exclusions

This plan does not test or authorize:

- HDCP keys, authentication, decryption, re-encryption, topology, revocation,
  locality checks, or protected-content transport;
- capture or replay of protected, private, or unauthorized media;
- claims of HDMI, HDCP, SMPTE, AMWA, PTP, codec, or safety conformance without
  the corresponding licensed specifications, test suites, equipment, and
  human acceptance;
- Arduino Mega participation in pixels, packets, EDID, link training, media
  clocks, or key handling;
- seamless switching, zero loss, losslessness, HDR fidelity, or 8K operation
  unless its exact declared acceptance evidence is recorded.

HDCP-capable product work is a separate licensed program with independent legal,
security, key-provisioning, repeater, compliance, and physical test plans.
