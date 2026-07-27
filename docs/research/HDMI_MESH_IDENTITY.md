# HDMI mesh identity and inventory

> **Controller decision:** phase one has one durable authoritative controller
> running as a service on an ordinary Linux computer. It never carries media.
> Replication, consensus, leader election, and automatic controller failover
> are deferred.

Status: design guidance  
Research date: 2026-07-27  
Scope: dynamically reconfigurable HDMI sources and sinks over switched media

## Decision

The mesh identifies appliances, physical HDMI ports, attached source and sink
devices, EDID policy, media senders and receivers, and routes separately. A
route names stable controller-issued identities. Endpoint agents resolve those
identities to current HDMI links and network flows immediately before a
transaction.

Connector labels, IP addresses, NMOS resource IDs, MAC addresses, HDMI Vendor
Specific InfoFrames, EDID strings, and HDCP receiver IDs are observations or
protocol coordinates. None is durable mesh identity or authorization by
itself.

Any enrolled receiver port may ingest a compatible source. Any enrolled
transmitter port may drive a compatible sink. Routes may be created, moved, or
released while the mesh remains live; fan-out joins in phase three. Dynamic routing never
changes which physical attachment or capability revision a plan approved.

## Identity hierarchy

| Object | Stable identity | Incarnation or revision | Current observation |
|---|---|---|---|
| Controller | provisioned controller ID and public-key fingerprint | durable controller incarnation | address, health, store revision |
| Media node | enrolled node ID and key fingerprint | boot ID and agent incarnation | addresses, firmware, clock and fabric health |
| Receiver port | node ID plus controller-issued RX port ID | hardware incarnation | connector map, RX lock, negotiated link |
| Source device | controller-issued source ID | attachment instance | source descriptors, timing, audio, metadata |
| Source presentation | controller-issued presentation ID | capability revision | current video/audio/metadata flows |
| Transmitter port | node ID plus controller-issued TX port ID | hardware incarnation | connector map, TX state, negotiated link |
| Sink device | controller-issued sink ID | attachment instance | EDID digest, link and display observations |
| EDID profile | controller-issued profile ID | immutable content revision | selected policy and generated EDID digest |
| Route | controller-issued route ID | route revision and source ownership epoch | selected ports, flows, timing, state |

IDs are opaque fixed-size values with a type prefix and checksum, such as
`node-...`, `rxp-...`, `src-...`, `txp-...`, `sink-...`, and `edid-...`.
Friendly aliases are mutable metadata. An alias must resolve to exactly one
authorized identity before planning.

### Appliance roles

A media node may expose receiver ports, transmitter ports, or both. Roles are
capabilities of an enrolled node, not permanent machine classes. A bidirectional
appliance therefore keeps one node identity while each physical connector has
one explicit role for its current hardware incarnation. Changing a connector
role fences its routes and advances that port's incarnation.

The Mega 2560 operator panel is a separate control accessory. It may display a
node alias and route health, but it never becomes a media-node identity,
attests HDMI capabilities, stores HDCP material, or authorizes a route.

## Node and port enrollment

Each media node owns a non-exportable key generated or installed during
enrollment. The controller records the key fingerprint, appliance model,
permitted site, operator, policy labels, enrollment time, and expected
firmware trust policy. Mutual authentication proves key possession on every
control session.

Enrollment is a local ceremony:

1. quarantine the unknown node;
2. compare its fingerprint out of band;
3. assign its site, roles, and policy;
4. map and label each physical HDMI and network connector;
5. record receiver, transmitter, clock, codec, and fabric capabilities;
6. run an unprotected generated-pattern self-test;
7. approve the resulting inventory digest and audit record.

A receiver or transmitter port ID belongs to one node and one documented
physical connector. It is not derived from an FPGA channel number, Linux
device path, NMOS ID, or front-panel alias. Its enrollment record includes:

- chassis and connector labels;
- immutable node-local port number;
- direction and permitted role changes;
- HDMI receiver or transmitter IP and PHY family;
- maximum TMDS and FRL modes;
- supported pixel encodings, depths, DSC behavior, audio, and metadata;
- EDID, DDC, HPD, SCDC, CEC, and test-pattern capabilities;
- media packetization, codec, PTP, and network-port relationships;
- HDCP capability state, without keys or protected secrets.

Replacing an HDMI PHY, FPGA image, connector module, or licensed IP block may
change the hardware incarnation even when the chassis remains. Routes do not
survive an incarnation change until the controller revalidates capabilities
and observations.

## Source identity

An HDMI source is an enrolled physical or virtual producer observed on one
receiver port. HDMI does not provide a universally trustworthy globally unique
source identity. Product strings, InfoFrames, CEC addresses, EDID interactions,
and HDCP identifiers can be missing, duplicated, mutable, private, or
unavailable.

Enrollment creates a source ID and records:

- authoritative receiver port and mobility policy;
- operator or asset label;
- observed descriptor and metadata digest;
- permitted video, audio, HDR, VRR, and DSC modes;
- permitted protected-content policy;
- first- and last-seen observations;
- confidence, replacement lineage, and quarantine state.

The receiver agent creates a fresh attachment-instance ID when HPD/link loss,
node restart, receiver reset, or another discontinuity breaks proven
continuity. A route is valid only for the enrolled source and current
attachment instance.

Sources without a trustworthy device-specific identity are port bound:

```text
enrolled source
    = receiver port ID
    + approved observation digest
    + current attachment instance
```

Moving one to another receiver port is an unknown observation until an
operator approves a source move or new enrollment. Matching format and product
strings do not prove that it is the same source.

### Source presentations

A source device may expose changing presentations, such as a laptop changing
timing, HDR state, audio layout, or VRR behavior. The stable source ID names
the producer; a presentation ID names a routable media contract. Its immutable
capability revision records:

- raster, scan, frame-rate family, sampling, and component depth;
- active and transport timing;
- audio format and channel map;
- carried InfoFrame and ancillary sets;
- HDR and colorimetry interpretation;
- DSC state and decoded network representation;
- fixed-rate, genlocked, asynchronous, or VRR timing policy;
- generated sender and flow identities.

A live mode change advances the presentation capability revision. The
controller either plans an authorized compatible transition or faults the
affected routes. It never silently preserves a plan approved for a different
format.

## Sink and transmitter identity

A sink is an enrolled display, processor, recorder, or test instrument
observed on one transmitter port. Its record includes:

- authoritative transmitter port and mobility policy;
- EDID block digest and parsed capability digest;
- supported link modes, formats, audio, HDR, VRR, and DSC;
- required timing and synchronization behavior;
- protected-content capability state;
- operator label, confidence, lineage, and quarantine state.

The transmitter agent creates a new sink attachment instance after HPD loss,
node restart, transmitter reset, or ambiguous reconnection. EDID equality does
not preserve the old attachment instance. Many identical displays legitimately
publish identical EDID data.

A transmitter port is the owned egress resource. A sink attachment is what is
currently connected to it. This distinction lets a destination connector keep
its identity across display replacement while ensuring the replacement cannot
inherit policy merely by copying an EDID.

## EDID identity and policy

Raw sink EDID is signed inventory evidence, not a routing policy. The
controller stores its digest and parsed interpretation, including every
extension block and parse warning. Unknown, malformed, contradictory, or
changed EDID quarantines capability-dependent route changes.

An EDID profile is a controller-managed immutable policy object:

```text
EdidProfileRevision
    profileId
    revision
    allowedVideoModes
    allowedAudioModes
    colorAndHdrPolicy
    vrrAndDscPolicy
    sourceQuirks
    generatedBlockDigest
```

Changing profile content creates a new revision; IDs and revisions are never
rewritten in place. A route records the exact EDID profile revision presented
to its source. The profile may be:

- a validated intersection of selected sinks;
- a conservative site profile;
- a fixed test profile;
- an operator-approved transform of one sink's capabilities.

An alias such as `conference-room-8k` may point to the newest approved revision
for new plans. Existing routes remain bound to the revision they activated
until explicitly replanned.

## HDCP capability without secrets

Phase-one research routes accept only owned, authorized, unprotected material.
HDCP implementation is a separately licensed product boundary.

Inventory may report non-secret capability and compliance state:

- `Unsupported`, `ResearchDisabled`, `LicensedUnavailable`, `LicensedReady`,
  `Authenticated`, `Repeater`, or `Fault`;
- supported protocol versions and repeater role;
- secure-element or licensed-IP readiness as a boolean attestation;
- authentication generation, topology count, and reason code where policy
  permits;
- compliance profile revision and certificate expiry metadata.

Inventory and audit logs must not contain device private keys, session keys,
decrypted protected content, key-selection vectors, secret provisioning
material, or diagnostics that permit their reconstruction. HDCP receiver IDs
and topology data are protocol evidence, not mesh identity. Key provisioning,
revocation data, and secure processing remain outside the Mega and ordinary
mesh database.

A protected route can be planned only by a later licensed policy module that
validates the complete repeater topology. An unprotected route cannot be
silently upgraded to protected operation.

## Capability model

Capabilities are typed, bounded claims with provenance:

```text
CapabilitySet
    objectId
    hardwareIncarnation
    capabilityRevision
    observedAt
    expiresAt
    staticCapabilities
    negotiatedCapabilities
    policyRestrictions
    evidenceDigest
```

Static claims describe installed hardware and licensed features. Negotiated
claims describe the current cable, peer, and link. Policy restrictions are
controller decisions. The usable set is their intersection; these categories
remain separate in diagnostics.

Video capability names exact raster, rate family, sampling, depth, colorimetry,
HDR, DSC, VRR, and pixel-clock or FRL constraints. Audio names sample rate,
sample width, coding, channels, and mapping. Network capability names media
transport, payload format, codec profile, maximum flow rate, PTP profile, and
redundancy support.

Terms such as `8K`, `HDMI 2.1`, `HDR`, `48G`, and `100G` are discovery labels,
not sufficient compatibility predicates.

## Signed inventory

Each node publishes signed, revisioned snapshots:

```text
HdmiInventorySnapshot
    nodeId
    bootId
    revision
    observedAt
    expiresAt
    receiverPorts[]
    sourceAttachments[]
    transmitterPorts[]
    sinkAttachments[]
    clockAndNetworkState
    previousDigest
    digest
    signature
```

Revisions increase within one boot. A reboot creates a new boot ID and requires
reconciliation; it never restores an active route from a local cache. Full
snapshots establish a base. Deltas are accepted only when their base revision
and digest match.

Every attachment observation carries its stable port ID, current attachment
instance, enrolled source or sink ID when known, observation digest, negotiated
link mode, health, and transient hardware coordinates. Unknown or ambiguous
attachments remain quarantined.

## Route binding

A route binds:

- one source presentation and current source attachment;
- one receiver port and hardware incarnation;
- one or more transmitter ports and current sink attachments;
- exact source, sink, and EDID capability revisions;
- network sender, flow, and receiver identities;
- timing, codec, bandwidth, and protection policy;
- node boot IDs and inventory revisions;
- route revision and monotonically increasing source ownership epoch.

Phase-three fan-out is one source presentation feeding multiple independently reserved
transmitter ports. It does not duplicate the source identity. Many-to-one is a
selection among presentations; one transmitter port drives at most one active
presentation.

Any changed attachment, incarnation, capability revision, EDID profile,
protection state, expired inventory, or ambiguous identity invalidates the
plan. Make-before-break media joining may prepare a compatible new flow, but
the transmitter has one authoritative active presentation at the switching
boundary.

The one durable controller is the only writer of route revisions and source
ownership epochs. Controller loss blocks mutation immediately. An exact
unchanged route may continue only until its pre-authorized bounded lease
expires, then every affected transmitter mutes. Endpoints never elect a
controller or infer authority from current media traffic.

## Aliases, groups, and selectors

Aliases improve operation:

```text
stage-left-laptop  -> src-...
wall-center        -> sink-...
room-a-displays    -> {sink-..., sink-...}
standard-8k-test   -> edid-...@revision
```

Aliases do not authorize routes. Policy evaluates immutable identities and
trusted labels after resolution. An alias change cannot alter an approved
plan. A selector matching zero or multiple objects is an error unless the
operation explicitly requests a set.

Placement groups may select any compatible healthy transmitter, but the plan
records the exact chosen node, port, sink attachment, capability revisions,
bandwidth reservation, and reason. Automatic replacement is a new audited
route transaction.

## Replacement, adoption, and retirement

Replacement creates a new identity by default:

1. quarantine and enroll the replacement node or device;
2. verify physical ports and capability evidence;
3. fence old routes and reconcile active media;
4. copy only explicitly approved policy and aliases;
5. record `replaces` and `replacedBy` lineage;
6. retire the old key or identity without reusing its ID.

Adoption preserves a stable port identity only when an administrator proves
that the same enrolled node and physical connector continue after an approved
repair. A new key, connector map, HDMI PHY, or ambiguous source/sink observation
requires a new incarnation or identity. Historical audit records are never
rewritten.

## Required deterministic cases

The host model should replay:

- node address change with stable authenticated identity;
- source and sink disconnect/reconnect with new attachment instances;
- two identical sources or sinks with matching descriptors or EDID;
- receiver or transmitter hardware replacement;
- node reboot during plan, join, switch, active media, and release;
- source mode change during route activation;
- EDID change, malformed extension, and profile revision race;
- alias rename during an in-flight plan;
- multicast fan-out with one incompatible or missing sink;
- bandwidth, PTP, codec, and link-mode incompatibility;
- stale capability or inventory revision;
- protection-state change and forbidden protected route;
- duplicate node identity and contradictory port observations;
- controller restart and stale-incarnation rejection;
- deterministic enrollment, replacement, retirement, and audit replay.

Each trace contains injected time, identities, signed inventory, policy,
reservations, endpoint confirmations, and media observations. Replay must
select the same objects, route revisions, ownership epochs, state transitions,
reason codes, and audit digest.

## Phase-one boundary

Phase one uses:

- one durable authoritative controller;
- unprotected generated or owned test media;
- stable node and physical-port enrollment;
- immutable EDID profile revisions;
- exact, revisioned media capability records;
- dynamic one-to-one routing, with one-to-many reserved for phase three;
- explicit CLI plans and deterministic replay;
- endpoint-visible lock, clock, buffer, route, and fault evidence.

It does not implement controller HA, seamless controller failover, protected
content, secret provisioning, universal source recognition, or transparent
format conversion. Those are later milestones with separate acceptance gates.

## Open design decisions

- Which node hardware root of trust is required beyond the laboratory?
- Which source and sink classes may be adopted after a physical port move?
- Should phase one derive EDID only from fixed profiles, or also support a
  validated intersection of selected sinks?
- Which HDMI timing changes permit an in-place presentation revision rather
  than a complete detach and reacquire?
- Which NMOS identity mappings are persisted, and which are regenerated as
  transient protocol coordinates?
- What capability lease duration balances fast fault detection against HDMI
  training and EDID churn?
- Which product milestone first requires licensed protected-content support?
- What controller replication model is appropriate after single-controller
  restart and fencing are proven?
