# Dynamic HDMI endpoint mesh

Status: architecture research, not a supported ADK interface  
Research date: 2026-07-27

## Decision

Build independently enrolled HDMI receiver and transmitter endpoints around an
ordinary switched IP fabric. Any authorized receiver may feed any compatible
transmitter, and one receiver may deliberately feed several transmitters. The
controller selects and supervises routes; it never forwards media.

The mesh terminates HDMI at both edges:

```text
HDMI source
    |
receiver endpoint: terminate TMDS/FRL, decode video/audio/metadata
    |
direct packet media flows across the switched fabric
    |
transmitter endpoint: buffer, schedule, encode, train a new HDMI link
    |
HDMI sink
```

This is interpretation, forwarding, and reconstruction, not extension of the
electrical HDMI waveform. The receiver exposes timed video, audio, and metadata
flows. The transmitter creates a fresh HDMI link with its own DDC, hot-plug,
TMDS/FRL training, clock, and error state. AMD's receiver subsystem demonstrates
the first boundary by converting HDMI into native/AXI video and audio streams;
its transmitter subsystem demonstrates the inverse boundary by encoding video
and audio streams as TMDS or FRL for a sink.[^amd-rx][^amd-tx]

Phase one accepts owned, unencrypted test content only. HDCP repeater behavior,
key custody, licensing, compliance, and protected-media transport are separate
product requirements and are not experimental shortcuts.

## Mesh vocabulary

| Object | Meaning |
|---|---|
| `ReceiverEndpoint` | One enrolled HDMI input and its media packetizer |
| `TransmitterEndpoint` | One enrolled HDMI output and its media receiver |
| `SourcePort` | Stable identity of a receiver-facing HDMI connector |
| `SinkPort` | Stable identity of a transmitter-facing HDMI connector |
| `SourceSignal` | Currently decoded source format and timed essences |
| `SinkCapability` | Validated EDID plus implementation and policy limits |
| `MediaRoute` | Authorized association from one source port to one sink port |
| `RouteGroup` | Explicit fan-out of one source signal to several media routes |
| `SourceEpoch` | Durable, non-wrapping fence for one source port's presentation authority |
| `SinkReservation` | Separately named, expiring claim on one sink port and its path resources |

“Source” and “sink” describe the HDMI media direction. A node may contain both
receiver and transmitter endpoints. Hardware roles do not change merely because
software changes a route.

## Stable identity and inventory

Names must survive IP changes, cable order, reboots, and discovery order:

```text
MeshId
NodeId
PortId
EndpointId
```

An admitted node receives a stable `NodeId`. Each physical connector receives a
stable `PortId` bound to the node and hardware inventory record. A replaceable
processing image may have its own `EndpointId`, but an update does not silently
create a new connector. Display make, model, serial, EDID hash, source format,
MAC address, hostname, and IP address are observations, never authority.

Each endpoint advertisement includes:

- receiver, transmitter, or combined capability;
- HDMI generation, TMDS/FRL rates, pixel formats, depths, DSC handling, audio,
  InfoFrame, HDR, and variable-timing support;
- network link rates, packet transports, media-clock profiles, buffer bounds,
  and fan-out limits;
- immutable hardware identity, software compatibility, health, and alarms;
- security zone and route-admission labels.

Discovery locates endpoints. Enrollment authenticates them. Neither operation
creates a route.

## Authority and dynamic state

Phase one uses one durable authoritative controller running as a service on an
ordinary Linux computer. It never terminates HDMI or carries media. Replication,
consensus, leader election, and automatic failover are deferred. Protocol
records reserve a controller term so high availability can be added without
changing endpoint identity or route semantics.

The controller persists:

- admitted nodes, ports, certificates, and policy;
- complete revisioned desired-route snapshots;
- the next non-wrapping source epoch for every source port;
- named sink reservations and their bounded expiry;
- capability and EDID policy selected for every route;
- an append-only, hash-linked audit record.

Endpoints persist their accepted controller term, latest source epoch, accepted
sink reservations, recent operation results, and safe restart state. They never promote
themselves, infer authority from a surviving connection, or accept an older
epoch. Controller loss blocks every mutation immediately. Only an exact,
unchanged installed route may run through its already issued bounded lease;
the phase-one policy mutes it on expiry and presents a visible fault.

Desired and observed state are separate. A delivered command is not evidence
that video is locked.

```text
DesiredRoute
    routeId
    sourcePortId
    sinkPortId
    controllerTerm
    sourceEpoch
    sinkReservationId
    transportProfile
    formatPolicy
    edidPolicy

ObservedRoute
    routeId
    localPortId
    peerPortId
    controllerTerm
    sourceEpoch
    sinkReservationId
    phase
    sourceFormat
    outputFormat
    mediaClockState
    lastError
```

Every control message and media-flow description carries the mesh, route,
controller term, source epoch, and sink reservation identity. Stale control
commands are rejected. Media receivers discard packets belonging to an
inactive source epoch or uncommitted sink reservation.
Counters never wrap into a valid earlier value; exhaustion is an administrative
fault.

## Routing and scale

Each sink port has zero or one active source route. The phase-one implementation
is one source to one sink. The architecture permits deliberate source fan-out,
but implementation and validation begin in phase three. Fan-out should normally
use switched multicast or replicated sender flows, not payload relay through
the controller.

The control plane scales with enrolled ports and state changes. The media plane
scales with active flow bandwidth:

```text
R1 -----------\
R2 ------------+---- switched media fabric ---- T1
R3 ------------+---- authorized direct flows -- T2
                \--- multicast fan-out -------- T3
```

Admission control evaluates every video, audio, metadata, and redundancy flow
against endpoint links, switch paths, multicast replication, QoS reservations,
clock domains, and measured headroom. A route is rejected before affecting the
current output when its capacity or compatibility cannot be proven.

USB, HDMI, control, telemetry, and ordinary household traffic share one managed
switched network. Logical traffic classes and VLANs may isolate policy and
queues, but are not separate physical fabrics. Admission preserves configured
ordinary-LAN headroom and evaluates a named media profile with explicit format,
peak rate, latency, jitter, buffering, codec, and quality bounds.

A route policy is either pinned to one profile, limited to an ordered allowed
set, or permitted to select the best profile within explicit bounds. A failed
proposal never disrupts an existing working route. No endpoint silently lowers
resolution, refresh, depth, chroma, audio, redundancy, or quality.

Use AMWA NMOS semantics where practical rather than inventing discovery and
connection behavior. IS-04 models Nodes, Devices, Sources, Flows, Senders, and
Receivers; IS-05 stages and activates connections.[^is04][^is05] The mesh
controller adds the durable fencing, EDID policy, HDMI link state, and audit
rules required by this project. IPMX is the interoperability direction for
pro-AV media over IP and HDMI-related metadata.[^ipmx]

## EDID, hot-plug, and format policy

The source-side receiver acts as an HDMI sink and therefore presents an EDID.
The destination-side transmitter reads the real display EDID over DDC. DDC is
local to each HDMI cable; it is not electrically bridged across the network.
AMD documents DDC as the I2C-based negotiation path between HDMI source and
sink, and its receiver exposes configurable EDID storage.[^amd-ddc][^amd-rx]

The controller maintains a validated `SinkCapability` observation and chooses
one explicit source-facing policy:

1. **Fixed mesh profile:** present a known, conservative EDID independent of
   destinations. This is the deterministic phase-one default.
2. **Selected-sink proxy:** synthesize a source EDID from one intended sink and
   endpoint/network limits.
3. **Route-group intersection:** advertise only formats supported by every sink
   in a declared fan-out group.
4. **Normalize:** accept a supported source format and perform an explicitly
   declared conversion at the transmitter or media processor.

Never concatenate arbitrary EDIDs or silently advertise the union of several
sinks. A union can make the source choose a format that no selected sink can
display. EDID changes are versioned desired state and may require an intentional
source-facing HPD pulse. They are not incidental consequences of endpoint
discovery.

HPD has two independent meanings:

- the transmitter observes the real sink's HPD and DDC state;
- the receiver controls the HPD presented to the real source.

The controller correlates them but does not wire them together. Route loss does
not automatically flap source HPD. The selected policy decides whether to keep
the source trained, request renegotiation, or report unavailable.

## Route transaction

Dynamic reconfiguration is staged and idempotent:

1. Resolve stable source and sink identities at one inventory revision.
2. Authorize the actor and route tuple.
3. Validate source signal, sink capability, conversion policy, media clock,
   endpoint resources, and network capacity.
4. Advance and durably record the source port's next epoch, then prepare a
   separately named sink reservation.
5. Prepare packet sender, receiver, multicast state, buffers, and clocks without
   changing the visible output.
6. Train the destination HDMI link against the real sink. If policy requires a
   new source format, stage the new EDID and deliberately sequence source HPD.
7. At the declared activation time, mute or test-pattern the old output, fence
   the old epoch and reservation, activate the new media flow, and wait for valid video,
   audio, metadata, buffer, and clock observations.
8. Publish `Active` only after both endpoints report the same route, term,
   source epoch, and sink reservation and the transmitter reports stable output.
9. On failure, hold a documented safe presentation: muted audio plus a local
   test pattern or black output with a visible fault. Never revive an older
   generation automatically.

A source can remain trained while several transmitters independently change
routes. A failed destination preparation must not disturb other fan-out
members. A requested simultaneous “salvo” prepares all routes first and uses a
common scheduled activation time; partial activation is reported explicitly.

## Endpoint phase machines

Receiver endpoint phases:

```text
Offline
  -> SourceAbsent
  -> SourceTraining
  -> SourceLocked
  -> PacketPrepared
  -> Streaming
  -> Quiescing
  -> Faulted
```

Transmitter endpoint phases:

```text
Offline
  -> SinkAbsent
  -> SinkProbing
  -> OutputTraining
  -> MediaPrepared
  -> OutputLocked
  -> Active
  -> Muted
  -> Faulted
```

Controller route phases:

```text
Unassigned
  -> Validating
  -> Preparing
  -> Prepared
  -> Activating
  -> Active
  -> Draining
  -> Unassigned
```

Every transition has a monotonic deadline and stable status code. A timeout is
an observation, not proof that the remote action did not occur. Duplicate
operations return their stored result without repeating HPD pulses, link
training, multicast changes, or media activation.

## Media timing and reconstruction

The receiver derives source timing, timestamps each essence against the mesh
media clock, and packetizes video, audio, and metadata separately. The
transmitter uses a bounded elastic buffer and a disciplined output clock to
construct a new raster and HDMI stream. It reports any frame repeat/drop,
sample-rate conversion, metadata substitution, or format conversion; none is
silent.

Fixed-rate, genlocked signals should be the first target. Variable refresh,
quick media switching, asynchronous sources, seamless switching, DSC decode or
pass-through, HDR metadata transformation, and multi-output frame alignment are
separate declared capabilities. The transmitter subsystem must own DDC, HPD,
InfoFrames, TMDS/FRL selection, FEC, and link training; AMD's transmitter
feature list illustrates these as endpoint responsibilities.[^amd-features]

The direct data plane carries:

- timed active video;
- audio essences and channel descriptions;
- HDMI InfoFrames and other admitted metadata;
- clock and stream descriptions;
- integrity, sequence, and optional redundant-path information.

Control commands and audit traffic never share the real-time media queue.

## Security and protected content

All nodes mutually authenticate. Route authorization covers actor, source,
sink, time, format, fan-out, and security zone. Media and control planes are
separated by policy and network segmentation. Endpoint software is signed;
secrets and protected-media keys, if a future licensed product introduces
them, never pass through a Mega.

The laboratory mesh does not accept HDCP-protected input. A commercial design
would be an HDMI/HDCP repeater system with licensed receiver and transmitter
functions, topology reporting, revocation, secure keys, and a protected
network-media design. AMD exposes HDCP only as an optional licensed subsystem,
which is evidence that this is a distinct gated feature, not transparent
forwarding.[^amd-hdcp]

## Observability

Correctness must be visible beside execution. Every endpoint supplies:

- source/sink cable presence and HDMI training phase;
- negotiated TMDS/FRL rate, raster, chroma, depth, audio, HDR, and DSC state;
- route identity, controller term, generation, and lease;
- packet count, loss, reorder, late arrival, and integrity counters;
- media-clock lock and offset, buffer low/high watermarks, underrun/overflow;
- output link lock and retraining count;
- temperature, power, fan, and transceiver alarms;
- video CRC or deterministic test-pattern signature.

Circuit-native evidence remains available without Serial:

- green: output locked and frame signature advancing;
- amber: endpoint controlled but route or media not locked;
- red: persistent format, timing, thermal, resource, or authority fault;
- locate indicator distinct from health;
- local test-pattern button and generated border/overlay;
- named frame-start, clock/PPS, buffer-alarm, HPD, and DDC test points.

The controller offers a CLI for inventory, compatibility explanation, route
prepare/activate/release, EDID inspection, observed-state watch, counter
snapshot, test-pattern selection, fault injection, and audit replay. Every
operation has a noninteractive form and stable machine-readable output.

Each endpoint also presents the requested profile, actually applied profile,
latency class, link rate, and failure state on a local display. Color may
reinforce status but never carries the only meaning. A pinned-profile contract
failure blanks video, mutes audio, retains the pin, and reports the failed
constraint until the exact profile can be admitted and retrained.

Production failure policy is versioned route configuration. Deterministic
software fault injection is a separate, disabled-by-default laboratory mode
with distinct authorization, a bounded duration, an unmistakable `TEST`
indicator, and audit records. Real faults always override injected observations.

The Arduino Mega 2560 may own buttons, LEDs, a small display, watchdog/reset
lines, and read-only health registers. It does not parse HDMI, packetize video,
run NMOS, decide route authority, maintain media time, handle keys, or forward
payload. Its display reports controller-approved desired state separately from
endpoint-observed active state.

## Deterministic verification

A reference simulator accepts stable inventories, capability records, desired
route snapshots, explicit timestamps, and an ordered fault script. It emits
canonical endpoint observations and audit records. Replaying the same input
must produce byte-identical output.

Required cases include:

- every compatible and incompatible format boundary;
- duplicate, delayed, lost, and reordered control messages;
- stale controller terms and source epochs;
- simultaneous route requests for one sink;
- source fan-out with one destination failing;
- EDID change and HPD sequencing;
- source loss, sink loss, cable bounce, and link-training timeout;
- endpoint and controller restart;
- partition, lease expiry, and delayed old media packets;
- PTP loss, clock drift, buffer underflow/overflow, and packet loss;
- multicast admission failure and oversubscribed links;
- audio/video/metadata disagreement;
- test-pattern and muted-output fallbacks;
- audit-storage failure and generation exhaustion.

Hardware work starts with generated low-resolution, unencrypted patterns and
one receiver/transmitter pair. It advances only after host and FPGA simulation,
rate accounting, electrical review, licensed-IP/tool review, and recorded
bench evidence. No document may infer 8K, latency, signal integrity, or
interoperability from compilation.

## Delivery phases

| Phase | Scope | Exit evidence |
|---|---|---|
| 0 | Simulator, stable identity, desired/observed records, fenced transactions | Deterministic fault replay |
| 1 | One generated source, one receiver, one transmitter, one-to-one route, fixed EDID, unprotected low-resolution video | Pixel/audio/metadata oracle and visible fault paths |
| 2 | Several endpoints on one managed switch, CLI dynamic routes, direct flows | Reconfiguration and capacity tests |
| 3 | Explicit source fan-out, scheduled salvo, PTP, network fault injection | Alignment, isolation, and recovery traces |
| 4 | Uncompressed 8K operating point on suitably rated FPGA/SoC and fabric | Measured rate, latency, integrity, thermal, and power evidence |
| 5 | Optional JPEG XS or other declared compression profile | Quality, latency, and bounded-rate evidence |
| 6 | Production study: redundancy, controller HA, licensing, HDCP, compliance | Separate approved product requirements |

Controller HA is intentionally not a prerequisite for phases 0--5. Endpoint
state and fencing must nevertheless remain compatible with a later single
logical authority implemented by replicated processes.

## Deferred questions

- Which fixed phase-one EDID and raster provide the best deterministic oracle?
- Is phase-one media transport raw RTP/ST 2110, IPMX-qualified behavior, or a
  deliberately smaller internal profile with an NMOS-compatible object model?
- Which FPGA/SoC board supplies both licensed HDMI endpoints and sufficient
  network bandwidth without committing the architecture to one vendor?
- What maximum route-change interruption is acceptable before seamless
  switching becomes a separate capability?
- Which metadata is preserved byte-for-byte, normalized, or regenerated?
- Does the first fan-out milestone require frame-aligned outputs or only
  independently locked outputs?

These choices affect implementation and purchasing, but none changes the core
mesh: stable enrolled endpoints, one durable route authority, direct fenced
media flows, local HDMI termination, and explicit reconstruction.

## Primary references

[^amd-rx]: AMD, [HDMI 2.1 Receiver Subsystem PG351](https://docs.amd.com/r/en-US/pg351-v-hdmi-rxss1/HDMI-2.1-Receiver).
[^amd-tx]: AMD, [HDMI 2.1 Transmitter Subsystem PG350](https://docs.amd.com/r/en-US/pg350-v-hdmi-txss1/HDMI-2.1-Transmitter).
[^amd-features]: AMD, [HDMI 2.1 Transmitter features](https://docs.amd.com/r/en-US/2026.1/pg350-v-hdmi-txss1/Features?contentId=WEKOP0cTS7GGpwyULsuo9w).
[^amd-ddc]: AMD, [HDMI transmitter Data Display Channel interface](https://docs.amd.com/r/en-US/2026.1/pg350-v-hdmi-txss1/Data-Display-Channel-Interface?contentId=1vu81n9AxSDoith05Ar81A).
[^amd-hdcp]: AMD, [HDMI receiver configuration and HDCP licensing gate](https://docs.amd.com/r/en-US/pg351-v-hdmi-rxss1/Top-Level-Tab).
[^is04]: AMWA, [NMOS IS-04 discovery and registration](https://specs.amwa.tv/is-04/releases/v1.3.3/docs/Overview.html).
[^is05]: AMWA, [NMOS IS-05 device connection management](https://specs.amwa.tv/is-05/v1.1/docs/Overview.html).
[^ipmx]: AIMS Alliance, [IPMX specifications](https://ipmx.io/specifications/).
