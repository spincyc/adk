# HDMI mesh reconciler

> **Controller decision:** one durable authoritative controller running on an
> ordinary Linux computer owns desired
> state in the first mesh. High-availability replication, consensus, and
> automatic failover are deferred. Controller loss fails closed for mutations.

Status: architecture guidance  
Research date: 2026-07-27  
Scope: deterministic routing between HDMI receiver and transmitter endpoints

## Boundary

The mesh does not forward an HDMI electrical link through Ethernet. A source
endpoint terminates HDMI, validates the received link, and separates video,
audio, timing, and metadata into network essences. A sink endpoint receives
those essences and creates a new HDMI link for the display.

The controller changes routes and never carries media. FPGA or adaptive-SoC endpoint agents perform
HDMI, media, packet, and timing work. A Mega 2560 may provide buttons, lamps,
and read-only health indication; it never handles pixels, PTP, EDID, HDCP
keys, or route authority.

Phase one accepts owned, unencrypted test material. HDCP is a separate licensed
repeater and key-management problem and is not part of this reconciler.

## Mesh model

Treat source receiver ports and sink transmitter ports as independently
addressable endpoints:

```text
desired routes + capabilities + reservations + observed endpoint state
    -> validate and schedule
        -> prepare media path
            -> establish timing and buffer
                -> switch sink at a frame boundary
                    -> retire the old path
```

A sink port selects at most one source. Phase one is one-to-one. The architecture
permits phase-three fan-out through multicast when admission policy reserves the
complete group. Unlike general
USB, HDMI-over-IP can prepare a new media path before retiring the old one.
The sink still drives only one reconstructed presentation at a time.

Stable identities do not depend on addresses or connector discovery order:

| Object | Stable identity | Mutable observations |
|---|---|---|
| Node | controller-issued `NodeId` | address, boot ID, health, version |
| Source port | `NodeId` plus `SourcePortId` | HDMI lock, format, EDID, media flows |
| Sink port | `NodeId` plus `SinkPortId` | display presence, EDID, TX lock, format |
| Route | controller-issued `RouteId` | source, sink, state, epoch, deadlines |
| Endpoint process | node ID plus random `BootId` | session and report sequence |

Cable position, IP address, multicast address, MAC address, HDMI connector
number, and EDID serial string are observations rather than authority keys.

## Desired and observed state

The durable desired snapshot contains:

- enrolled nodes and administratively enabled ports;
- source-to-sink route requests and source fan-out sets;
- presentation policy, including allowed formats and latency class;
- source-facing EDID policy and its digest;
- route priority, request sequence, and policy identity;
- the non-wrapping epoch for each source port;
- endpoint drain, maintenance, and quarantine intent;
- bandwidth, multicast, processing, and sink reservations.

Each endpoint reports independently:

- its boot ID, capability revision, health, and report sequence;
- cable presence, HPD state, RX/TX lock, and negotiated HDMI format;
- EDID digest and the source-facing EDID version it presents;
- PTP lock, flow membership, RTP continuity, and buffer bounds;
- video, audio, and metadata readiness;
- mute, blank, test-pattern, and transmitter-enable state;
- installed source epoch, route ID, and pending operation.

Observed state is evidence, never authority. Missing evidence is `Unknown`, not
healthy or detached. A route is `Active` only when controller, source agent,
and sink agent agree on identities, source epoch, format, timing, media
continuity, and active presentation. HDMI carrier or TX lock alone is not
proof that the selected pixels are arriving.

The reconciler is a pure function:

```text
reconcile(desiredRevision, desired, observed, reservations, logicalTick)
    -> ordered action batch + next logical state + audit events
```

Equal inputs and logical ticks produce byte-identical results. Wall time,
network arrival order, hash-table order, and worker count cannot choose a
route or alter action order.

## Source epochs and fencing

One durable controller allocates each source port's monotonically increasing
`SourceEpoch`. Every change to its route membership, presentation policy,
source-facing EDID, format contract, or administrative availability advances
that epoch before endpoint mutation begins.

Every command names:

```text
authority term, desired revision, source epoch, route ID, source port,
sink port, source and sink boot IDs, operation ID, logical deadline
```

Endpoint agents persist the highest accepted authority term and source epoch
before changing media or HDMI state. They reject older terms, older epochs,
conflicting operations at the same epoch, and commands for an old boot ID.
An idempotent retry reuses its operation ID and returns the recorded result.

Fan-out does not require interrupting unchanged sinks. When membership changes
without changing the media or EDID contract, existing sinks acknowledge the
higher source epoch while continuing the identical flow. They cannot report
`Active` for the new desired revision until that acknowledgement is durable.
When format or EDID policy changes, every dependent route enters preparation
or safe blanking under the new epoch.

Epochs never wrap, decrement, or reset after restoration. Exhaustion is a
terminal administrative fault. A signature or mutually authenticated session
proves command origin; the epoch proves freshness. Both are required.

## Route states

Each route exposes one state:

```text
Unassigned
Requested
WaitingForSource
WaitingForSink
Reserving
PreparingSource
PreparingSink
Training
Buffering
ReadyToSwitch
Switching
Active
Retiring
Degraded
Fault
```

The route record retains the observations that justified each transition.
`Degraded` never renders green and never silently becomes `Active`.

Every sink also has an explicit output state:

```text
Disabled
SafeBlank
LocalTestPattern
Presenting
FaultBlank
```

`SafeBlank` means video is a documented neutral frame, audio is muted, stale
metadata is suppressed, and no old-source frame can escape. Disabling an HDMI
transmitter is not always the best safe state because it may cause slow display
reacquisition. Policy chooses transmitter-disabled or stable-timing blank, but
never an unlabelled freeze of previous content.

## Admission and reservations

Validation precedes endpoint action. A route must reserve:

- one sink transmitter and its format-conversion resources;
- source packetizer and multicast capacity;
- network ingress, egress, and oversubscription budget along the selected path;
- multicast table and flow identifiers;
- receiver reorder or protection buffers;
- PTP/timing capacity;
- audio and metadata channels;
- codec capacity when compression is requested.

Reserve against measured worst-case wire rate plus configured headroom, not
average content rate. A redundant A/B route reserves the full flow on both
fabrics. A compressed route reserves its declared constant-rate envelope and
fails closed on an encoder overrun.

USB, HDMI, controller, telemetry, and ordinary household traffic share one
managed switched network. Logical traffic classes protect control and
interactive flows while admission preserves configured ordinary-LAN headroom.
A named media profile states format, codec, peak rate, latency, jitter,
buffering, and quality bounds.

Sink reservations have identities distinct from source epochs, expiry ticks,
and states. They
are prepared before route mutation, committed only for the matching operation,
and released idempotently. Expiry cannot authorize presentation. A stale
reservation cannot be renewed into another epoch.

For phase three and later, the controller validates the complete proposed
fan-out and EDID consequence
before publishing desired state. If capacity is unavailable, the request
remains queued or is explicitly rejected; the controller does not degrade
format, chroma, depth, frame rate, audio, or redundancy silently. Route policy
may pin one profile, name an ordered allowed set, or permit best-within-bounds
selection. Failure to admit a candidate leaves an existing healthy route
unchanged.

## EDID and HPD ordering

EDID is policy input, not a transparent side channel. For each source, the
controller chooses one documented mode:

- **fixed profile:** advertise an administrator-selected mesh format;
- **intersection:** advertise only modes supported by every admitted sink;
- **normalized:** receive an allowed source format and convert separately for
  sinks with reserved processing capacity.

A sink join or departure does not automatically rewrite the source EDID. Such
churn could force every display path to retrain. The controller first computes
and records the proposed EDID digest, validates every dependent route, and asks
for explicit policy approval when the active source format would become
invalid.

When source-facing EDID must change, ordering is:

1. advance and persist the source epoch and desired EDID digest;
2. reserve capacity for every surviving desired route;
3. command affected sinks to mute and enter safe blank or local test pattern;
4. stop publishing the old presentation as active;
5. update the source endpoint's EDID memory and verify its readback digest;
6. issue the policy-defined HPD low pulse, then restore HPD;
7. wait for source retraining and read the actual negotiated format;
8. revalidate that format against reservations and every desired sink;
9. prepare, train, and buffer each route;
10. switch sinks at deterministic frame boundaries.

HPD is never pulsed merely to retry an unrelated network fault. The agent owns
minimum low and settling intervals as explicit logical deadlines derived from
the validated HDMI implementation. It reports actual transitions and does not
claim that software intent proves electrical behavior.

Sink EDID changes are inventory events. They invalidate affected reservations
and may cause safe blanking, but they do not acquire authority to rewrite
source EDID or select a substitute format.

## New assignment and move

For a new sink assignment:

1. validate identity, authorization, health, capabilities, and desired epoch;
2. allocate and commit reservations;
3. place the sink in mute plus safe blank or local test pattern;
4. configure network flow reception and timing;
5. configure the HDMI transmitter for the admitted format;
6. train the sink link while media output remains blank and audio muted;
7. join flows, establish PTP lock, and fill the elastic buffer;
8. prove advancing video identity, audio readiness, metadata consistency, and
   buffer bounds;
9. switch video on a named frame boundary, then release audio mute on its
   corresponding media boundary;
10. publish `Active` only after source, sink, and controller agree.

For a move from source A to source B, the sink may prejoin B while continuing
to present A. It validates and buffers B, then changes selection on a frame
boundary. It retires A only after the B presentation is confirmed. If B cannot
become ready, A remains active when policy and its current epoch still permit
it.

A sink capability or EDID change during preparation cancels the switch,
releases the new reservation, and keeps or restores the old valid
presentation. It never triggers an unapproved fallback mode.

## Rollback

Rollback is a new, explicit reconciler action, not history reversal. Epochs are
never rolled back.

Before the switch boundary, failure may preserve the old route if it is still
authorized, current, and healthy. The controller tears down the candidate,
releases its reservations, and records `KeptPriorPresentation`.

After the switch boundary, the controller may create a new higher-epoch plan
to reconstruct the prior source. It must repeat admission, preparation,
buffering, and switching. If prior-source validity cannot be proved, the sink
enters `FaultBlank`; it does not display a stale frame or assert `Active`.

EDID/HPD changes cannot be undone by restoring an old digest under an old
epoch. Restoring a former EDID is another approved higher-epoch transaction
and may cause another visible retraining interval.

## Deterministic scheduling and fairness

Each sink has one serialized switching lane. Each source has one serialized
epoch/EDID lane. Independent preparations may run concurrently if their sink,
source-control, processing, and bandwidth reservations do not conflict.

Bounded queues use this deterministic order:

1. fault blanking, stale-flow fencing, and reservation cleanup;
2. operator-requested release or endpoint drain;
3. recovery of an authorized prior presentation;
4. approved route changes by descending policy priority;
5. oldest immutable request sequence;
6. ascending route ID as final tie-break.

Priority uses a small configured domain. Reissuing a request cannot reset its
age. Weighted-deficit scheduling across policy groups prevents a sustained
high-priority fan-out workload from starving other admitted groups. A large
fan-out consumes measured deficit proportional to its reserved work.

The scheduler caps concurrent endpoint training, multicast joins, codec
configuration, and buffer fills. Queue saturation rejects new work explicitly;
it never drops accepted work. Changing worker count changes completion time,
not selection, actions, or audit output.

## Failure and restart

| Failure | Required result |
|---|---|
| Source HDMI unlocks | Mute dependent audio, safe-blank sinks, retain diagnostic test pattern only when labelled |
| Sink disappears | Stop its transmitter, release its route resources, preserve other fan-out routes |
| PTP unlocks | Safe blank before buffer timing becomes ambiguous |
| RTP loss or buffer limit exceeded | Apply declared concealment bound, then safe blank and fault |
| Audio loses alignment | Mute audio; keep video only if policy explicitly permits degraded video |
| Metadata disagrees with format | Suppress stale metadata and safe blank until a coherent set is ready |
| Candidate preparation fails | Keep authorized old route; otherwise fault blank |
| Switch acknowledgement is lost | Retry the same operation ID and inspect persisted endpoint result |
| Controller restarts | Restore durable epochs and desired state, advance authority term, observe before mutating |
| Endpoint agent restarts | Reject old-boot commands, safe blank local output, inventory and reconcile |
| Network partition | No new mutation; endpoints reject stale authority and apply bounded local hold policy |
| Reservation backend fails | Stop mutation before endpoint action |
| Conflicting observations | Fence the route, safe blank the sink, and reconcile from fresh reports |

Controller loss does not necessarily require an already healthy presentation
to disappear immediately. An endpoint may continue the exact installed route
for a bounded, configured hold interval without accepting changes. On expiry
or loss of any required health proof, it mutes and safe-blanks. This is
availability behavior, not authority: the endpoint cannot create, move, or
renew a route.

On controller restart:

1. recover the complete durable desired snapshot, epochs, reservations, and
   operation results;
2. advance and persist the single-controller authority term;
3. mark endpoint observations unknown;
4. collect authenticated inventories with new or current boot IDs;
5. reconcile actual flows, HDMI state, and reservations;
6. fence stale candidates and safe-blank ambiguous sinks;
7. resume mutation only after the recovery snapshot is internally consistent.

If persistence cannot prove the latest epoch or reservation state, mutation
remains disabled. Future HA may replace this authority boundary, but phase one
contains no leader election, quorum, or automatic failover.

## Observation and audit

Append one durable event for every desired edit, admission result, reservation,
scheduling choice, command, acknowledgement, endpoint report, HPD transition,
EDID digest, timeout, switch boundary, rollback, and fault. Include:

```text
authority term, desired revision, source epoch, route and endpoint identities,
boot IDs, operation ID, logical tick, format digest, reservation IDs,
old and new state, reason, previous-record digest
```

Expose separate evidence for:

- controller desire and source epoch;
- source HDMI lock and negotiated-format digest;
- network flow identity, sequence continuity, PTP, and reservations;
- sink buffer bounds, output state, TX lock, and displayed frame identity.

A visible endpoint panel should distinguish requested, preparing, active,
degraded, safe blank, and fault. Green requires full agreement and advancing
media evidence. A generated border, source identifier, and frame counter prove
pixels; LEDs for RX lock, PTP lock, buffer health, and TX lock prove distinct
subsystems. Named frame-start, HPD, and alarm test points complement the panel.
Serial and network logs remain supporting evidence only.

The panel also names requested and applied profile, link rate, latency class,
and exact failed constraint using text or symbols independent of color. Loss of
an active pinned profile immediately blanks video and mutes audio while
retaining the pin for recovery.

Deterministic fault injection is a separate disabled-by-default laboratory
input with distinct authorization, bounded duration, an unmistakable `TEST`
display, and separate audit records. Real faults dominate injected observations.

## Deterministic proof plan

The host model replays:

- route requests in every arrival permutation;
- one-to-many fan-out joins and independent sink departures;
- simultaneous moves sharing a source, sink, link, or processing resource;
- fixed, intersection, and normalized EDID policies;
- HPD ordering, repeated reports, deadline boundaries, and lost replies;
- candidate failure before and after the switch boundary;
- source unlock, sink removal, PTP loss, RTP loss, and buffer faults;
- source and sink capability changes during every route state;
- stale terms, source epochs, boot IDs, operation IDs, and reservations;
- controller and endpoint restart before and after every durable transition;
- queue exhaustion and fair progress under sustained competing demand;
- tick, sequence, epoch, and capacity boundaries;
- identical traces with one worker and many workers.

Required properties:

- one sink never presents two sources;
- multicast fan-out never exceeds a committed reservation;
- stale observations never create `Active`;
- audio is muted and video is safe-blanked before an invalid presentation;
- EDID is verified before HPD restoration;
- a switch occurs only after candidate readiness and at a named boundary;
- rollback never decrements or reuses an epoch;
- every request completes, is rejected, is cancelled, or reaches a recorded
  fault;
- equal inputs produce equal actions and audit records;
- restart never converts unknown presentation state into active state.

Physical HDMI compatibility, signal integrity, negotiated FRL/TMDS behavior,
display reacquisition, visual continuity, audio alignment, throughput, latency,
and actual EDID/HPD timing remain separate bench evidence.

## Delivery boundaries

1. Build a host-only identity, capability, reservation, and route model.
2. Add the pure reconciler, source epochs, and bounded fair scheduler.
3. Simulate media readiness, EDID/HPD, frame boundaries, and endpoint restart.
4. Add durable single-controller persistence and idempotent agent commands.
5. Loop a generated raster through one source and one sink endpoint.
6. Route 1080p one-to-one over one switch and exercise dynamic sink moves.
7. Add and prove deliberate fan-out.
8. Add fixed-profile EDID first; add intersection and normalization later.
9. Scale formats and link rates only after counters and reservations agree.
10. Add controller HA only after single-authority recovery is correct and
   measured.

## Open decisions

These do not block the deterministic host model:

- fixed-profile EDID contents and allowed initial video/audio format;
- bounded local hold interval during controller loss;
- maximum nodes, sources, sinks, routes, fan-out, and queue depth;
- admission headroom and fabric topology representation;
- initial blank-frame color and diagnostic overlay convention;
- whether phase one keeps video when only audio faults;
- exact endpoint command protocol and durable storage backend;
- first FPGA platform and licensed HDMI IP.

Start with one unencrypted generated 1080p source, one sink, fixed EDID,
uncompressed media, one authoritative controller, and no automatic format
fallback. That narrow system proves identity, admission, fencing, switching,
safe blanking, and restart before 8K bandwidth complicates the evidence.
