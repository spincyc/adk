# HDMI mesh control protocol

> **Controller decision:** phase one uses one durable authoritative controller
> running as a service on an ordinary Linux computer. It never carries media.
> Controller replication, leader election, quorum, and automatic failover are
> deferred. Linux control endpoints run the receiver and transmitter agents.

Status: research contract; not a supported ADK interface.

This protocol dynamically connects admitted HDMI sources to compatible HDMI
sinks through receiver, network, and transmitter appliances. The controller
manages identity, EDID policy, HPD, reservations, fencing, and convergence. It
does not define pixel, audio, metadata, clock-recovery, or forward-error-
correction framing. Those are data-plane responsibilities.

An HDMI route terminates the source link at a receiver and constructs a new
sink link at a transmitter. It is not transparent wire forwarding. HDCP
termination or repeater behavior requires a separately licensed and audited
implementation and is outside the phase-one protocol.

## Terms and stable identities

| Term | Stable identity | Meaning |
|---|---|---|
| Node | `nodeId` | One authenticated Linux control endpoint |
| Source | `sourceId` | One physical or virtual HDMI source connector |
| Sink | `sinkId` | One physical or virtual HDMI sink connector |
| Receiver | `receiverId` | One source-side HDMI receiver resource |
| Transmitter | `transmitterId` | One sink-side HDMI transmitter resource |
| Route | `routeId` | One desired source-to-sink presentation |
| Controller incarnation | `term` | Monotonic generation advanced after durable controller recovery |
| Source fence | `sourceEpoch` | Non-wrapping authority generation owned by one source |
| Sink reservation | `sinkReservationId` | Expiring claim on one sink and its path resources |

Identifiers are opaque, globally unique byte strings. An IP address, hostname,
connector label, PCI address, EDID serial number, display name, or enumeration
order is metadata, not identity. A node persists `nodeId` across restart.
Replacing a connector appliance, receiver, or transmitter creates new
identities unless an administrator explicitly adopts the replacement.

A node advertisement binds sources to receivers and sinks to transmitters. It
includes stable identities, an `advertisementRevision`, health generation,
connector presence, and capabilities. Capabilities include:

- TMDS and FRL modes, lane rates, color formats, bit depths, and pixel limits;
- audio formats, channel limits, and sample rates;
- HDR, variable-refresh, DSC, and metadata support;
- receiver and transmitter data-plane transports and measured capacity;
- EDID storage limits, HPD control, DDC behavior, and link-training support;
- protection state, without exporting secrets or claiming unsupported HDCP.

Discovery locates candidates. Only authenticated admission to `meshId` grants
permission to advertise or receive commands.

## Authority and fencing

One durable controller is the only desired-state author for a controller
`term`. On restart it restores durable state, advances `term`, and reconciles
before authorizing new output. Nodes reject a term lower than the greatest term
they have durably accepted. No endpoint elects itself, infers authority from a
network session, or continues an unfinished reconfiguration after its lease
expires.

Every source owns one monotonic, non-wrapping `sourceEpoch`. The controller advances it before
a route move, EDID-policy change, fan-out-set change, or other operation that
can alter what the source observes. Sink reservations are independent capacity
records; they prevent two routes from driving one sink but never grant source
authority.

Every state-changing message carries:

```text
protocolVersion
meshId
term
sourceEpoch
sinkReservationId
operationId
routeId
sourceId
sinkId
receiverId
transmitterId
desiredRevision
```

Participants reject lower terms or source epochs, accept an exact duplicate
idempotently, and reject conflicting reuse of an operation identity. Data-plane
sessions carry `meshId`, `sourceId`, `sinkId`, `sourceEpoch`, and the applicable
`sinkReservationId`; receivers and
transmitters discard stale media even if an old network connection survives.
Terms, epochs, revisions, and sequence numbers never wrap or reset silently.
Exhaustion is an administrative fault.

## Desired and observed state

Desired state is a complete revisioned snapshot:

```text
DesiredMesh
    desiredRevision
    term
    routes[]

DesiredRoute
    routeId
    sourceId
    receiverId
    sinkId
    transmitterId
    sourceEpoch
    sinkReservationId
    edidPolicy
    mediaPolicy
    profilePolicy
    failurePolicy
    leasePolicy
```

`edidPolicy` identifies a canonical EDID record and whether it came from the
sink, an intersection, or an administrator-defined profile. `mediaPolicy`
bounds video, audio, metadata, bandwidth, latency, and protection requirements.
Absence from a newer complete snapshot means disconnected. A delta names its
base revision and is rejected if the receiver lacks that base.

Each participant reports observations independently:

```text
ObservedRoute
    endpointId
    observationRevision
    acceptedTerm
    acceptedSourceEpoch
    sinkReservationId
    desiredRevision
    routeId
    peerId
    phase
    connectorPresent
    hpdAsserted
    edidDigest
    sourceLink
    sinkLink
    mediaHealth
    lastStatus
```

Phases are `Idle`, `Reserved`, `EdidReady`, `SourceTraining`,
`TransportReady`, `SinkTraining`, `Presenting`, `Muting`, `Draining`, and
`Faulted`. The controller derives convergence from matching desired and
observed records. Message delivery, HPD assertion, or link lock alone is not
proof that a route is presenting. Wall-clock time is diagnostic metadata and
never establishes ordering.

## Sink reservation

A sink has at most one active transmitter route. Before changing the source,
the controller durably creates a separately named reservation for the exact
`sinkId`, `transmitterId`, `routeId`, term, source epoch, path, and bounded
capacity. A reservation has its own stable identity and monotonic deadline and
is visible in observed state. It expires to a muted, non-presenting state.

A transmitter cannot present media merely because it holds a reservation.
Activation additionally requires the matching desired revision and fenced
operation. Source fan-out, if later enabled, creates one sink reservation per
destination while retaining one authoritative epoch for the complete fan-out
set.

## EDID, HPD, and route transaction

EDID and HPD are part of the transaction because they alter source behavior.
Phase one is one-to-one and uses break-before-make with this deterministic
sequence. Source fan-out remains an architectural capability scheduled for
phase three:

1. Validate admitted identities, connector presence, capability compatibility,
   data-plane capacity, policy, and current topology revision.
2. Read the sink EDID through the selected transmitter. Parse, normalize, and
   store its bytes and digest without yet changing source HPD.
3. Derive the route EDID deterministically from `edidPolicy`. Reject a policy
   whose advertised modes cannot be carried and reconstructed.
4. Allocate the source's next `sourceEpoch`. Durably record the complete desired
   snapshot, sink reservation, canonical EDID bytes, and audit record.
5. Ask receiver and transmitter to `PrepareRoute`. Preparation allocates
   bounded resources, programs the receiver's inactive EDID bank, and holds the
   transmitter muted. It does not assert source HPD or emit active media.
6. If replacing a route, mute and drain the old transmitter, deassert source
   HPD when the EDID or fan-out contract changes, then fence and detach both old
   data-plane endpoints.
7. Activate the receiver EDID bank. Assert source HPD for the new epoch only
   after both preparations and old-route fencing are observed.
8. Observe source DDC access and receiver link training. A bounded timeout
   leaves the route muted and faulted; it does not imply that a delayed endpoint
   command may still activate.
9. Establish the fenced media transport, configure the transmitter from
   observed stream timing and policy, and train the sink link while muted.
10. Unmute only when receiver, transport, and transmitter observations agree on
    route, identities, term, epoch, EDID digest, and compatible media state.

Changing EDID policy or the set of sinks visible to a source is a new source
epoch and normally requires an explicit HPD cycle. HPD pulses have configured
minimum-low and settle intervals expressed as monotonic deadlines. Repeated
commands never generate extra pulses. DDC reads are observations, not ordering
authority.

A failure before durable epoch allocation leaves the prior desired route
unchanged. A failure after allocation leaves affected transmitters muted and
the new route detached or faulted. The controller never restores an older epoch
automatically. A different outcome requires a new operation and epoch.

## Messages

| Message | Direction | Purpose |
|---|---|---|
| `AdvertiseEndpoint` | node to controller | Publish stable resources, connectors, and capabilities |
| `PublishDesired` | controller to nodes | Supply a complete desired snapshot or based delta |
| `ReadSinkEdid` | controller to transmitter | Acquire sink EDID without changing presentation |
| `ReportSinkEdid` | transmitter to controller | Return canonical bytes, digest, and read status |
| `PrepareRoute` | controller to endpoints | Reserve resources and stage an inactive configuration |
| `Prepared` / `Rejected` | endpoint to controller | Report deterministic preparation result |
| `SetSourceHpd` | controller to receiver | Apply a fenced HPD state and pulse schedule |
| `MuteRoute` | controller to transmitter | Force visible output to the defined safe presentation |
| `DrainRoute` | controller to endpoints | Stop new media and finish bounded in-flight work |
| `DetachRoute` | controller to endpoints | Fence and remove the old route |
| `ActivateRoute` | controller to endpoints | Authorize link training for a prepared route |
| `PresentRoute` | controller to transmitter | Unmute one fully converged route |
| `ReportObserved` | node to controller | Publish local phase, fences, links, and media health |
| `Heartbeat` | bidirectional | Renew an exact lease without changing desired state |
| `RequestSnapshot` | either direction | Recover a missing desired-state base |

Replies echo the operation identity, term, epoch, route, and endpoint
identities. Stable status codes drive behavior; optional diagnostic text is for
operators only. Canonical wire encoding excludes floating point and unordered
maps so signatures, audit hashes, and deterministic replay agree.

## Idempotency, ordering, and deadlines

`operationId` is globally unique and retained beyond the maximum command,
reconnect, and HPD transaction lifetime. Nodes durably retain their accepted
term, greatest epoch for every source they serve, current desired revision,
HPD state, EDID digest, and terminal results of recent operations.

Messages may be duplicated, delayed, lost, or reordered. Correctness depends
only on identity, term, epoch, desired revision, operation identity, and
explicit state prerequisites. Connection order and arrival time grant no
authority. All waits use explicit monotonic deadlines. A timeout is an
observation, not proof that a remote side effect did not happen.

## Profiles, failure policy, and local evidence

USB, HDMI, control, telemetry, and ordinary household traffic use one shared
managed network. A named profile states the video, audio, metadata, codec,
peak-rate, latency, jitter, buffering, and quality contract. Route policy may
pin one profile, allow an ordered profile set, or choose the best profile within
explicit bounds. The controller never changes profile silently, and a failed
candidate plan leaves an existing working route unchanged.

Each endpoint displays requested and applied profile, link rate, latency class,
route state, and exact fault in text or symbols that do not depend on color.
Loss of an active pinned profile's contract immediately blanks video and mutes
audio while retaining the pin for recovery. Production failure response is
versioned route policy.

Deterministic fault injection is a separate, disabled-by-default laboratory
facility. It requires separate authorization, a bounded duration, an
unmistakable local `TEST` indication, and distinct audit events. Real hardware
or network faults always take precedence.

## Disconnect and restart

The phase-one lease policy fails closed. Controller loss blocks every mutation
immediately. Only an exact, unchanged installed route may continue under its
already issued bounded lease:

- a receiver deasserts source HPD after lease expiry unless an explicitly
  configured diagnostic policy requires HPD low from the outset;
- a transmitter mutes on lease expiry and disables
  presentation after its bounded drain interval;
- an endpoint accepts only a matching lease renewal or a higher-term recovery
  command; an old route cannot be revived.

After node restart, HDMI outputs remain muted and source HPD remains deasserted.
The node first restores durable fences, advertises its actual connector and
link observations, obtains the authoritative snapshot, and reconciles. Cached
EDID may be reported but is not made source-visible until authorized.

After controller restart, the controller restores desired state and audit
records, advances `term`, and gathers observations. It either reauthorizes the
same desired route under the new term or creates a new route transaction.
Automatic controller failover is not implemented. During controller
unavailability, endpoints do not invent, change, expand, or renew routes or
promote cached state.

## Scale and topology changes

Nodes may host any mix of receiver and transmitter resources. Snapshots and
observations are paginated and watched from explicit revisions. Compaction
returns `SnapshotRequired`; it never skips unseen history. Health and
connector changes do not consume a source epoch unless desired routing or the
source-visible EDID contract changes.

The controller may schedule independent sources concurrently. Operations that
share a receiver, transmitter, sink, data-plane capacity pool, or source
fan-out set are serialized by stable resource identity. Future controller
sharding may partition by `sourceId`, but exactly one authority may allocate an
epoch for a source.

## Security and audit boundary

Nodes mutually authenticate and authorize commands against stable identities
and roles. EDID bytes, metadata, and media descriptions are untrusted input and
are parsed with fixed limits. Control messages never carry HDCP keys or
decrypted protected media.

Every requested change records the actor, canonical request, prior and new
desired revisions, term, source epoch, EDID digest, reservations, endpoint
observations, result, and previous-record hash. Audit persistence failure
prevents a state change from becoming authoritative.

## Versioning

The envelope carries major and minor protocol versions and negotiated feature
bits. A major mismatch rejects the session. Unknown required features reject
the affected operation. Stored desired, observed, EDID, and audit records carry
schema versions and use explicit deterministic migrations. A node completes
durable migration before enabling HPD or presentation and cannot downgrade
past stored semantics.

## Deterministic verification

A reference simulator accepts topology, canonical EDID fixtures, initial
durable records, desired operations, explicit monotonic times, link outcomes,
and an ordered fault script. It emits canonical desired snapshots,
reservations, HPD transitions, observations, and audit records. Replaying the
same input produces byte-identical output.

Tests cover duplicate and reordered commands, lost replies, stale epochs,
controller and endpoint restart, partition, connector removal, EDID read and
parse failure, incompatible modes, HPD timing, source training timeout, sink
training timeout, transport loss, delayed old media, resource contention,
simultaneous route changes, epoch exhaustion, malformed snapshots, and audit
failure. In every failure trace, an unauthorized transmitter remains muted and
a stale receiver cannot assert HPD.

## Deferred questions

1. Which unprotected video, audio, metadata, and timing modes define the
   phase-one interoperability profile?
2. Which phase-three fan-out profile should be validated first: fixed EDID or
   administrator-approved capability intersection?
3. Which receiver-to-transmitter data-plane transport meets the required
   latency, clock, bandwidth, and packet-loss behavior?
4. What safe presentation should a sink show while muted: no signal, a fixed
   locally generated diagnostic frame, or both as a policy?
5. Which hardware exposes deterministic EDID-bank switching and HPD control
   through a supported Linux interface?
