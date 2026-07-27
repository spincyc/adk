# Shared USB and HDMI fabric

Status: research architecture; no hardware or performance claim.

## Product constraint

USB and HDMI routes share the home's ordinary switched Ethernet network with
the household LAN. A dedicated media fabric is not a supported assumption.
Logical separation, scheduling, and admission control make the shared network
usable; they do not create capacity or remove a weak link.

```text
utility closet                                      occupied room

computer                                            monitor and peripherals
    |                                                      |
    +-- Computer Attachment Unit --+    +-- Console Attachment Unit --+
                                   |    |
                             shared switched LAN
                                   |
                         ordinary Linux controller
```

The controller runs on a normal Linux computer. It carries inventory, policy,
route plans, fencing epochs, and audit records. USB transactions and HDMI media
flow directly between attachment units; the controller is never in their data
path. The first controller is singular and durable. High availability is
deferred, and loss of the controller fails route changes closed.

## Link hierarchy

One fabric may contain 1, 2.5, 10, 25, and 100 Gb/s Ethernet links. Link speed
is a measured property of each path, not a property inferred from an endpoint.

| Link | Intended use |
|---|---|
| 1 Gb/s | Controller, management, telemetry, and low-rate household clients |
| 2.5 Gb/s | Household access and development endpoints |
| 10 Gb/s | Baseline CAU/PAU access, including PoE++ where supported |
| 25 Gb/s | Higher-rate gateway access or aggregation |
| 100 Gb/s | Closet core and aggregation when concurrent media requires it |

Every admitted route is bounded by the slowest link and oversubscribed uplink
on its complete path. A 100 Gb/s core does not make a 10 Gb/s access link
faster. Likewise, VLANs and priority markings do not reserve throughput by
themselves.

The baseline USB Peripheral Attachment Unit has one 10GBASE-T PoE++ connection
and four independently powered USB 3 Type-A topology roots. The baseline USB
Computer Attachment Unit has one 10GBASE-T PoE++ connection and one USB 3
Type-B computer port. Auxiliary DC remains a development and overload fallback.
Actual PoE class, usable peripheral power, conversion loss, cable temperature,
and gateway thermal limits remain selection and bench-test items.

HDMI endpoints may require 25 or 100 Gb/s access and separate DC power. Their
chosen media representation, pixel format, compression, redundancy, and rate
determine the requirement. The design must not imply that PoE++ or 10 Gb/s can
power or carry an arbitrary 8K HDMI route.

## Logical separation and queues

The physical network is shared; logical traffic domains are distinct:

| Domain | Examples | Treatment |
|---|---|---|
| Management | controller API, enrollment, route epochs | authenticated, low bandwidth, protected queue |
| Timing | PTP event and general messages | tightly policed priority queue |
| USB control | enumeration, reset, interrupt completions | latency-sensitive, bounded queue |
| USB payload | bulk and isochronous transfers | admitted and shaped per route |
| HDMI control | EDID, hot-plug state, route health | protected low-rate queue |
| HDMI media | video, audio, metadata | admitted and shaped; multicast when useful |
| Household LAN | ordinary client traffic | guaranteed a configured residual share |

IEEE 802.1Q VLANs provide logical broadcast-domain separation. VLAN identity is
not an authorization decision and is not a confidentiality mechanism.
Attachment units authenticate peers and authorize each route independently.

IEEE 802.1 priority queues and IP differentiated-services markings may select
forwarding behavior. Markings are rewritten or policed at trusted ingress so a
household client cannot obtain media priority by setting a tag. Strict priority
without rate limits is forbidden because it can starve management and ordinary
LAN traffic. Credit-based shaping or time-aware scheduling may be evaluated
where every switch on the path implements the required behavior. Mixed-switch
paths fall back to measured, conservatively admitted service.

This follows the aggregation and boundary-conditioning model in
[RFC 2475](https://datatracker.ietf.org/doc/html/rfc2475). IEEE describes
[802.1Qav](https://www.ieee802.org/1/pages/802.1av.html) as separating
time-sensitive and ordinary traffic through priority and credit-based queues,
and [802.1Qbv](https://www.ieee802.org/1/pages/802.1bv.html) as scheduling
queue transmission from a shared time base. Neither is claimed implemented.

## Capacity admission

The controller admits a proposed route only from a current topology snapshot
and a conservative, named traffic profile. Profiles are first-class route
contracts rather than hidden implementation choices. A profile declares:

- source and destination access-link rates;
- each traversed switch and uplink;
- sustained, peak, and burst rates;
- maximum tolerable loss, delay, and jitter;
- USB transfer-class support and buffering limits, or HDMI media format,
  encoding, chroma, bit depth, frame rate, and compression mode;
- expected end-to-end latency and startup interval;
- queue and shaping requirements;
- multicast replication points and receiver count;
- existing admitted routes and a household reserve;
- protocol, framing, encryption, and redundancy overhead;
- measured headroom for topology discovery and control traffic.

The system may define several explicit profiles for the same route type. A
lower-bandwidth profile may trade resolution, frame rate, compression, USB
throughput, buffering, or latency only where the corresponding data-plane
contract permits it. Names describe the actual contract, such as
`Hdmi4k60LowLatency`, `Hdmi4k30Compressed`, or `UsbBulkBuffered`; names such as
`Auto`, `Best`, and `Normal` are insufficient.

An operator selects one of two admission policies:

- **Strict pin:** admit exactly the requested profile or reject the route.
- **Ordered fallback:** try the requested profile followed by the operator's
  explicit ordered list of acceptable profiles.

The controller never invents, reorders, or extends the fallback list. Before
applying a fallback, it reports the proposed profile and reason. Whether that
requires fresh confirmation or may proceed automatically is an explicit route
policy. The route record and audit journal preserve requested, considered,
admitted, and applied profile identities.

Admission is per directed path. Full-duplex links have independent directional
budgets. A route is rejected when any link, switch queue, replication point,
power budget, or endpoint cannot satisfy its declared envelope.

Each admitted profile creates reservations on every traversed link, queue, and
multicast replication egress. A fallback releases the superseded reservation
and atomically acquires the new profile's reservation before its data plane is
applied. Concurrent reconciliation cannot spend the same capacity twice.
Partial reservation failure leaves the previous profile intact or the route
disconnected according to its failure policy.

Admission is not a promise based on nominal port labels. Before promotion, the
system measures goodput, queue delay, loss, reordering, pause behavior, and
thermal stability on the installed path. Unknown unmanaged switches, link-rate
changes, or an unrecognized topology invalidate the affected admission.

The initial implementation should use static, conservative reservations.
Automatic reclamation from observed under-use is deferred because a quiet USB
or HDMI stream may burst later. Reconciliation may degrade or disconnect a
route only according to its explicit policy. It never silently changes media
quality, latency, buffering, USB capability, or any other profile property.

## Time and media clocks

PTP supplies a common observation and scheduling time. It does not replace the
USB bus clock or HDMI pixel/audio clocks, and it does not make asynchronous
oscillators identical.

Attachment units should use NIC hardware timestamping where the selected NIC,
driver, and switch path support it. Linux `ptp4l` can use a PTP Hardware Clock,
and `phc2sys` can discipline the Linux system clock from that clock; see the
[Linux PTP `ptp4l` documentation](https://www.linuxptp.org/documentation/ptp4l/)
and [`phc2sys` documentation](https://www.linuxptp.org/documentation/phc2sys/).
Boundary- or transparent-clock-aware switches are preferred but not assumed.

Each data plane still needs explicit clock recovery and buffering:

- USB preserves ordering and endpoint semantics while absorbing bounded
  network variation.
- HDMI receives and interprets the source link, transports timestamped media,
  and generates a fresh sink-side HDMI clock and link.
- Audio/video clock drift is measured and corrected by the selected media
  design, not hidden by queue growth.
- Loss of time lock is visible and may block a route whose tested contract
  depends on it.

PTP offset, path delay, servo state, clock class, and holdover age are recorded.
No precision claim is made until the exact NICs, switches, topology, and load
have been measured.

## Multicast

One-to-one USB routes remain unicast and exclusive. HDMI may use multicast for
one source routed to several sinks when content authorization, endpoint
compatibility, and capacity allow it.

Multicast replication belongs at the nearest capable switch, with explicit
group membership and source filtering. Unknown multicast is not flooded across
the household LAN. Receiver joins and leaves trigger capacity reconciliation.
The controller accounts for bandwidth after each replication point; it never
charges only the source link.

Multicast can reduce duplicated upstream traffic, but it does not eliminate
per-egress capacity. RFC 2475 notes that dynamic multicast membership makes
resource use and quantitative guarantees harder, so multicast routes retain
stricter headroom and membership limits.

## Congestion and failure containment

Congestion is a route fault, not permission to consume every queue. The fabric:

- meters each endpoint at trusted ingress;
- bounds queues to avoid persistent buffer growth;
- reserves management reachability and household capacity;
- isolates broadcast, multicast, and discovery traffic;
- detects link downgrade, loss, excessive delay, and reordering;
- rejects new routes before degrading admitted routes;
- marks a route degraded before any policy-directed disconnect;
- treats a switch reboot or topology change as invalidating affected paths;
- never lets stale data from an old route epoch reach a new attachment session.

Ethernet pause and priority-flow-control behavior must be measured and scoped.
Fabric-wide pause propagation is not accepted as flow control for USB or HDMI.
Endpoints need bounded application buffers and explicit overflow behavior.

USB route movement remains a physical unplug/replug model. A cold move
disconnects the old computer, removes remote VBUS, verifies discharge, advances
the topology epoch, restores protected power, rediscovers the topology, and
then presents a fresh attachment to the new computer. HDMI route changes
similarly expose explicit link loss and fresh sink-side reconstruction.

### Operational failure policy

Each route carries a named production failure policy. It controls real
endpoint, link, clock, capacity, power, and compatibility failures. Supported
policy primitives are:

- keep the currently applied profile while it remains within contract;
- retry the same profile for a bounded count and interval;
- try only the remaining operator-approved fallback profiles in order;
- disconnect and remain faulted;
- restore a previously proven profile when its reservation is still valid;
- require operator acknowledgement before reconnection.

A policy specifies which failure classes permit each action. For example,
capacity loss may allow an approved lower-bandwidth HDMI profile, while USB
topology mismatch may require immediate disconnect. A profile that is already
violating its declared envelope is not reported as healthy merely because
packets continue to flow.

Every transition is externally visible before or at application. Endpoints
show requested, applied, degraded-by-policy, and faulted states distinctly.
They display the actual applied profile using a local display or an unambiguous
profile code plus labeled reference. A generic green link LED is not sufficient
evidence of the active mode.

### Deterministic test fault injection

Test fault injection is a separate facility and is never accepted through the
production route API. Host tests use a deterministic model with an explicit
seed or trace, supplied timestamps, and named injection points. The model can
inject:

- link loss, downgrade, delay, jitter, loss, reordering, and queue exhaustion;
- reservation conflict and stale topology;
- endpoint rejection, restart, thermal, PoE, and USB VBUS faults;
- PTP unlock, clock step, excessive drift, and holdover expiry;
- multicast join/leave races and replication failure;
- profile apply failure before and after each reconciliation boundary;
- stale epoch messages and controller interruption.

Each trace records requested profile, allowed fallbacks, operational failure
policy, fault, timestamps, decisions, reservations, endpoint commands, applied
profile, visible evidence, and final route state. Replaying a trace must produce
the same decisions and audit bytes. Test injection cannot be compiled or
configured into a release endpoint without a separately gated diagnostic
build, and a diagnostic endpoint cannot join a production mesh.

## Security

Management and data-plane peers use provisioned device identities and mutual
authentication. Route authority comes only from the controller's current,
durable epoch. Endpoints fail closed during controller loss and never elect a
replacement.

Network policy includes:

- deny-by-default inter-VLAN forwarding;
- narrow controller and endpoint access-control lists;
- authenticated, integrity-protected control messages;
- encrypted data paths where the measured latency and bandwidth permit;
- replay rejection and non-reusable route epochs;
- secure boot and signed endpoint updates as hardware selection requirements;
- no trust based solely on switch port, MAC address, VLAN, or physical room;
- rate limits for discovery, control, and malformed traffic;
- auditable operator identity for every route and power operation.

HDCP handling must remain within licensed, compliant receiver/repeater/
transmitter behavior. Encryption of the Ethernet transport does not grant
permission to decrypt, copy, or redistribute protected HDMI content.

## Observability

The Linux controller exposes a CLI and machine-readable API from the same
state. It records desired, planned, applied, and observed route state
separately. Required evidence includes:

- physical and negotiated link speed for every hop;
- switch path, VLAN, queue, and traffic-class mapping;
- reserved, measured, and peak bandwidth per link and route;
- queue occupancy, drops, explicit congestion, and packet reordering;
- latency and jitter histograms rather than averages alone;
- requested profile, ordered allowed fallbacks, applied profile, and reason for
  every rejected or changed profile;
- reserved versus available capacity for every traversed link, queue, and
  multicast egress;
- PTP offset, path delay, lock state, and clock discontinuities;
- multicast membership and replication accounting;
- endpoint temperature, PoE class, input power, and each USB port's VBUS
  voltage, current, overcurrent, and fault state;
- USB topology epoch, attachment session, enumeration, resets, and stalls;
- HDMI link mode, media format, buffer level, clock recovery, and sink state;
- controller reachability and durable audit-journal health.

Each appliance also needs circuit-visible evidence independent of the
controller: power-safe, link, admitted-route, active-route, applied-profile,
policy-approved fallback, and fault indicators or named electrical test
points. A local display presents the actual applied profile; a headless unit
uses a deterministic, labeled code that can identify every supported profile.
Indicators distinguish resource acquisition from safe electrical state.

## Staged home deployment

1. **Management proof:** run the singular controller on a normal Linux box;
   enroll two endpoint simulators on the existing LAN; prove authentication,
   epochs, audit, and fail-closed controller loss without media.
2. **Isolated logical domains:** configure management, timing, USB, HDMI, and
   household VLAN/queue policy on the existing managed switches. Verify that
   ordinary LAN traffic retains its configured share.
3. **Measured 10 Gb/s path:** install one CAU and one PAU path using auxiliary
   DC first. Record cable qualification, link stability, goodput, latency,
   loss, queue behavior, and thermals under concurrent household load.
   Exercise strict profile rejection and each explicitly allowed fallback;
   verify the endpoints and controller always agree on the applied profile.
4. **Protected USB power:** validate one USB Type-A root's current limit,
   overcurrent latch, discharge, cold move, and physical topology recreation.
   Expand to four independent roots only after aggregate power admission works.
5. **PoE++ operation:** qualify the exact PSE, cable, connector, PD converter,
   enclosure, ambient range, and USB load. Auxiliary DC remains until the
   operating envelope is recorded.
6. **Transparent USB corpus:** progress from HID through controlled bulk,
   storage, composite, hub, and isochronous devices. Windows and Linux use
   native drivers; no custom host driver is permitted.
7. **HDMI development route:** begin with a bounded format and one source/sink.
   Measure end-to-end delay, recovered clocks, packet loss response, and
   concurrent USB/household behavior before increasing rate or resolution.
8. **Aggregation upgrade:** add 25 or 100 Gb/s links only where measured route
   demand and oversubscription require them. Re-run every failure and
   congestion test after each topology change.
9. **Room console:** coordinate USB and HDMI as one `ConsoleRoute`, while
   preserving independent route epochs, admission, fault reporting, and
   recovery.

Every stage retains an ordinary household workload during stress tests. A
successful quiet-network demonstration is not shared-fabric acceptance.

## Deferred work

- exact CAU, PAU, HDMI endpoint, PHY, FPGA/SoC, switch, PSE, and PD selection;
- measured envelopes for each USB transfer type and HDMI representation;
- transparent USB topology reconstruction hardware;
- HDMI compression and protected-content implementation;
- controller high availability;
- wireless transport;
- USB-C and USB Power Delivery;
- automatic capacity reclamation and quality adaptation;
- physical, thermal, interoperability, security, and long-duration acceptance.

This document defines the intended architecture. It does not claim that a
transparent USB bridge, 8K HDMI bridge, PoE++ 10 Gb/s endpoint, deterministic
shared LAN, or production controller has been implemented.

## Primary references

- [IEEE 802.1Q: Virtual LANs](https://www.ieee802.org/1/pages/802.1Q.html)
- [IEEE 802.1Qav: forwarding and queuing for time-sensitive streams](https://www.ieee802.org/1/pages/802.1av.html)
- [IEEE 802.1Qbv: enhancements for scheduled traffic](https://www.ieee802.org/1/pages/802.1bv.html)
- [IEEE 802.1 Time-Sensitive Networking task group](https://www.ieee802.org/1/pages/tsn.html)
- [IETF RFC 2475: Differentiated Services architecture](https://datatracker.ietf.org/doc/html/rfc2475)
- [Linux PTP `ptp4l`](https://www.linuxptp.org/documentation/ptp4l/)
- [Linux PTP `phc2sys`](https://www.linuxptp.org/documentation/phc2sys/)
- [IEEE 802.3bt public task-force material](https://www.ieee802.org/3/bt/public/)
