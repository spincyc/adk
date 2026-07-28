# HDMI/shared-fabric next synthetic milestone

Status: bounded research decision; implementation not started
Decision date: 2026-07-28

## Decision

The next HDMI/shared-fabric milestone is a deterministic, host-only
capacity/admission reconciler for one synthetic HDMI route on a shared
household LAN.

The reconciler will decide whether a requested, named HDMI profile can reserve
one directed network path while preserving its configured ordinary-LAN
headroom. It will emit a canonical action trace for admission, rejection, or
fail-closed withdrawal. It will not transport media, discover a real network,
configure a switch, or control an HDMI endpoint.

This milestone closes a bounded gap between the existing HDMI route-state
model and the shared-fabric contract. The route model already demonstrates
stable endpoint identity, source epochs, ordered route transitions, and
explicit blanking. The shared-fabric architecture defines profiles,
per-directed-path reservations, and household capacity, but those decisions
do not yet have one executable deterministic oracle.

## Claim boundary

All topology, traffic, time, and fault observations are supplied fixtures.
The only media identity allowed by the fixture is a generated, unprotected
test pattern. A fixture rate or logical delay is an input to model arithmetic,
not a measurement.

Completion may claim only that the host model:

- makes reproducible admission decisions for its synthetic fixtures;
- preserves modeled household-LAN headroom;
- rejects stale or insufficient evidence;
- orders reservation and withdrawal actions deterministically; and
- fails closed when authority or a required contract becomes invalid.

Completion must not claim HDMI interoperability, HDMI or HDCP compliance,
protected-content handling, physical 4K or 8K transport, physical latency or
jitter, lossless media, image quality, clock recovery, switch QoS, endpoint
behavior, signal integrity, thermal adequacy, EMC performance, or production
readiness. The work includes no HDCP key, protected sample, decryption path,
repeater experiment, or bypass technique.

## Single authority and state boundary

One durable Linux controller remains the only desired-state and admission
authority. The model has no election, peer promotion, or controller
high-availability behavior. The controller supplies:

- a non-wrapping authority term and desired revision;
- one current topology revision;
- one requested route and named profile;
- the route's explicit failure policy;
- existing reservations; and
- the configured ordinary-LAN reserve on every traversed directed link.

The pure reconciler consumes immutable values and returns a decision plus an
ordered action list:

```text
reconcile(
    logicalTick,
    authority,
    topology,
    profileCatalog,
    desiredRoute,
    existingReservations,
    observations)
  -> decision, nextReservations, actions, auditRecord
```

Equal canonical inputs produce byte-identical output. The model cannot read
wall time, enumerate interfaces, query a switch, sample traffic, use random
input, or depend on hash-map iteration order. Every identifier and collection
has a stable serialized order.

Observed state is evidence, never authority. Link speed, path membership,
queue capacity, endpoint capability, and controller reachability are explicit
fixture fields with `Known`, `Unknown`, or `Stale` validity. `Unknown` and
`Stale` cannot satisfy admission.

## Bounded model

The first fixture contains one source receiver, one sink transmitter, one
directed path through at most two switches, one HDMI traffic class, one
management reserve, and one ordinary-LAN reserve. USB traffic is represented
only as an existing named reservation; USB protocol and endpoint behavior are
outside this milestone.

A named HDMI profile declares:

- generated test-pattern identity and unprotected-content requirement;
- synthetic raster label solely for fixture identification;
- sustained, peak, burst, and protocol-overhead rates;
- source and sink capability requirements;
- maximum model delay, jitter, and loss bounds;
- queue and shaping requirements; and
- strict-pin or explicit ordered-fallback policy.

Profile labels such as `4k` or `8k` are forbidden in the initial fixture
because they invite a physical-performance inference. Use a neutral name such
as `hdmi-generated-profile-a`, with every modeled property stated separately.

Admission uses integer arithmetic. For every directed link and constrained
queue:

```text
available =
    knownCapacity
    - managementReserve
    - ordinaryLanReserve
    - existingReservationPeaks

admit only if candidatePeakWithOverhead <= available
```

The implementation must reject arithmetic overflow, duplicate reservation
identities, an absent path element, mismatched topology revisions, an unknown
or downgraded link, inconsistent directional accounting, and a candidate
whose source or sink capability does not satisfy the profile. VLAN or priority
labels never create capacity.

The output is one of:

- `Admitted`: all reservations were atomically modeled and the route may
  proceed to the existing HDMI preparation state machine;
- `Rejected`: no installed state changes, with one stable reason;
- `KeptPrior`: the new request failed but the existing route remains fully
  current and within its pinned contract;
- `Withdraw`: a previously admitted route lost a required contract and must
  enter explicit mute/no-signal handling; or
- `AuthorityFault`: mutation is prohibited because controller authority is
  missing, stale, or inconsistent.

`Admitted` is not `Active`. Endpoint observations and media integrity remain
separate evidence owned by later milestones.

## Transaction and fail-closed behavior

The canonical action order is:

1. validate controller term and desired revision;
2. validate topology and observation freshness;
3. validate source, sink, and profile compatibility;
4. compute each directed-link, queue, and endpoint budget;
5. stage all candidate reservations without changing installed state;
6. either reject the complete candidate or commit all staged reservations;
7. publish the decision and canonical audit record; and
8. only after `Admitted`, issue a symbolic `PrepareHdmiRoute` action to the
   existing route model.

There is no partial admission. A failed calculation leaves every prior
reservation byte-for-byte unchanged.

Controller loss immediately prohibits new admission, fallback selection,
reservation renewal, and route mutation. An already admitted route may remain
modeled only through its previously issued bounded lease while every required
observation remains current and its pinned contract still holds. Lease expiry,
topology change, link downgrade, capacity invalidation, or stale evidence
emits `Withdraw`; the symbolic endpoint action is audio mute followed by HDMI
no-signal. The model never substitutes a profile unless it appears in the
operator's exact ordered fallback list and the current authority explicitly
permits fallback.

An endpoint simulator cannot grant or extend a reservation, advance a source
epoch, elect an authority, infer safety from continued packets, or report a
route active. A stale action from an older authority term, desired revision,
topology revision, route epoch, or reservation identity is rejected without
side effects.

## Deterministic implementation deliverables

One later implementation task may add:

1. a fixed-capacity host library with plain value records for topology,
   profiles, observations, reservations, decisions, actions, and audit data;
2. one pure reconciliation entry point with explicit logical time;
3. a canonical text fixture containing generated-unprotected-media identity
   and no real host or network identifiers;
4. a non-mutating CLI that prints the input fixture, decision, per-link
   arithmetic, ordered actions, and a `synthetic model-only` marker; and
5. deterministic unit and replay tests integrated into the existing
   `hdmi-mesh-check` gate.

The implementation must not add sockets, packet capture, switch APIs, NMOS
services, PTP daemons, kernel changes, FPGA builds, codecs, EDID/HPD electrical
control, or Arduino code.

## Required deterministic tests

The gate must cover:

- exact-fit admission and one-unit-under-capacity rejection;
- ordinary-LAN reserve retained on every directed hop;
- independent forward and reverse budgets;
- the slowest link and an oversubscribed uplink controlling the result;
- existing HDMI and synthetic USB reservations consuming capacity;
- duplicate reservation and integer-overflow rejection;
- unknown, stale, downgraded, and topology-revision-mismatched links;
- strict pin rejecting a lower profile;
- an explicit ordered fallback evaluated in the supplied order only;
- failed candidate admission preserving a valid prior route;
- invalidated prior contract producing ordered mute/no-signal withdrawal;
- controller loss, stale term, stale desired revision, and lease expiry;
- stale reservation and route-epoch actions rejected without side effects;
- endpoint incompatibility and missing endpoint evidence;
- failure before every transaction boundary leaving no partial reservation;
- stable reason codes and stable ordering independent of input insertion
  order; and
- repeated execution and replay producing byte-identical decisions, actions,
  reservation state, and audit bytes.

Tests must assert the `synthetic model-only` and
`generated unprotected media` boundaries in CLI output. Any fixture field
named as latency, loss, capacity, raster, or rate must also be labeled
`modeled`, never `measured`.

## Exit gate

The milestone is complete only when:

- an independently reviewed implementation matches this decision;
- every required deterministic test passes under the normal host gate;
- malformed and stale fixtures fail closed with no state mutation;
- the CLI cannot access hardware or the network and prints its claim boundary;
- documentation distinguishes admission from endpoint and media evidence; and
- no generated output or prose implies physical HDMI, 4K/8K, latency,
  interoperability, compliance, or protected-content success.

Passing this exit gate authorizes only consideration of another bounded
synthetic milestone. It does not authorize purchasing endpoint hardware or
starting a physical HDMI trial.

## Retained promotion blockers

The following remain blockers after this host model succeeds:

- HDMI Adopter access, trademark terms, licensed receiver/transmitter IP, and
  a separately approved HDCP-compliant product path;
- selection and qualification of receiver, transmitter, FPGA/SoC, PHY, NIC,
  switch, clock, power, and enclosure endpoints;
- generated-unprotected-media data-plane implementation with pixel, audio,
  metadata, and clock-recovery oracles;
- measured switch topology, goodput, queueing, loss, reordering, pause
  behavior, PTP behavior, and ordinary-LAN coexistence;
- physical 4K work before any physical 8K work, with instrumented bandwidth,
  latency, jitter, integrity, interoperability, and failure evidence;
- endpoint power, cooling, thermal margin, cable and connector temperature,
  long-duration stability, and fault shutdown;
- signal-integrity, ESD, grounding, emissions, immunity, and complete EMC
  review; and
- simultaneous HDMI, USB, management, timing, and ordinary-LAN qualification
  on the exact shared installation.

No one may close these blockers from a host-model result.

## Relationship to existing research

This decision narrows, and does not replace, the
[dynamic HDMI architecture](HDMI_MESH_ARCHITECTURE.md), the
[HDMI reconciler](HDMI_MESH_RECONCILER.md), the
[HDMI protocol](HDMI_MESH_PROTOCOL.md), and the
[shared USB/HDMI fabric](SHARED_USB_HDMI_FABRIC.md). Those documents retain
the endpoint, media, security, licensing, and deployment contracts. If they
conflict with this milestone's narrower claim boundary, the narrower boundary
controls this milestone.
