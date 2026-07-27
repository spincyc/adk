# USB 3 mesh observability

> **Product boundary:** [the transparent product contract](USB_TRANSPARENT_PRODUCT.md)
> is authoritative. Product evidence comes from physical Cau/Pau units that
> reconstruct an exact topology for an unmodified Windows or Linux computer.
> Virtual-host evidence is prototype-only.

> **Controller decision:** initial evidence assumes one durable authoritative
> controller. Controller replication, consensus, and automatic HA failover are
> deferred; controller loss is observed as a fail-closed condition.

Status: design guidance; physical acceptance deferred  
Scope: dynamically reconfigurable host-facing and device-facing USB endpoint
appliances on the shared switched network

## Evidence contract

Product evidence names a one-port Cau, its `ComputerPort`, a four-root Pau,
one `PeripheralPort`, the complete real `TopologyRoot`, and its
`TopologyEpoch`. Source/destination and virtual-host terms below apply only to
the USB/IP prototype.

Every source-side physical USB device and every destination-side virtual host
is a separately named endpoint. Either may join, leave, fail, or be reassigned
without renumbering the other. A route is identified by immutable endpoint
identities plus the controller's monotonically increasing lease epoch, never by
a transient USB bus ID, virtual-host port, IP address, or panel position.

The controller publishes one coherent snapshot:

```text
device endpoint -> device ownership epoch -> destination endpoint -> transaction state
```

Endpoint agents report observations; they do not decide ownership. An active
route requires agreement among the authoritative controller, the source
endpoint, and the destination endpoint on the same identities and epoch.
Missing, contradictory, late, or stale evidence produces `Degraded`, `Fault`,
or `Unassigned`, never a guessed green state.

The system keeps three proof planes distinct:

1. logical proof: the controller authorized exactly one current lease;
2. transport proof: source and destination agents confirmed that epoch;
3. circuit proof: the endpoint panels and named test points show local
   presence, power, link, and fault state.

No single plane proves the other two. Enumeration does not prove payload
integrity. VBUS does not prove assignment. A controller record does not prove
that a cable, device, network path, or destination driver is healthy.

## Endpoint panel

The canonical display modes are `Startup`, `Normal`, `Night`, `Attention`,
`Fault`, `Test`, `ControllerLost`, and `Maintenance`. Route state, mode, and
brightness remain independent. `Night` cannot hide a fault; `Test` cannot
override a real fault; `ControllerLost` cannot remain green or mutate a route.
A deterministic self-test exercises every indicator without changing USB or
VBUS state.

Each endpoint appliance reserves a locally driven panel that remains useful
when its network service or Serial link is unavailable. Labels use the same
terms as the CLI and event journal.

| Indicator | Source-side device endpoint | Destination-side host endpoint |
|---|---|---|
| `Presence` | physical attach detected | virtual controller available |
| `Power` | measured protected VBUS present | host-facing interface powered |
| `Network` | authenticated controller session healthy | authenticated controller session healthy |
| `Route` | current lease state and epoch suffix | current lease state and epoch suffix |
| `Activity` | bounded pulse for observed USB work | bounded pulse for observed USB work |
| `Fault` | local power, USB, agent, or consistency fault | local virtual-host, agent, or consistency fault |

`Power` comes from voltage-good or protected-load-switch telemetry, not from
the commanded enable value. `Presence` comes from the local USB controller.
`Network` requires a fresh authenticated heartbeat. `Activity` is rate-limited
so a busy device remains visibly alive without exposing payload content or
holding an LED permanently on.

The route indication is color plus cadence, with a text label or adjacent
legend so color is never the only information:

| State | Route indication |
|---|---|
| `Unassigned` | blue, one short pulse |
| `Requested` or `Detaching` | amber, slow pulse |
| `Attaching` or `Enumerating` | amber, fast pulse |
| `Active` | steady green only after three-plane epoch agreement |
| `Degraded` | alternating amber and red |
| `Fault` | steady red; no active-route claim |
| endpoint identity conflict | three red pulses, pause |

A small display may add the endpoint's stable short ID, peer short ID, state,
and low-order epoch digits. It must not display credentials, raw serial
numbers, filenames, keystrokes, camera content, or storage data.

The Mega 2560 may render this panel from bounded, checksummed snapshots. It
does not authorize routes, carry USB traffic, retain credentials, or turn an
old snapshot green after its freshness deadline. Loss of panel communication
shows `Degraded`; it does not invent a detach or power-cycle operation.

## Named electrical test points

Every source appliance exposes documented, protected measurement points:

- `TP-VBUS`: device-port VBUS relative to `TP-GND`;
- `TP-PWR-EN`: protected load-switch command;
- `TP-PWR-GOOD`: independently sensed VBUS-good indication;
- `TP-USB-PRESENT`: local controller's attach observation;
- `TP-HEARTBEAT`: bounded endpoint-agent heartbeat.

Every destination appliance exposes `TP-GND`, a virtual-controller-ready
signal, an agent heartbeat, and any real host-facing power-good signal. Do not
label a software-only state as an electrical test point.

USB SuperSpeed pairs are not learner probe points. Their signal integrity is
verified with suitable compliance equipment and fixtures, not a breadboard,
Mega pin, ordinary logic probe, or exposed wire. The panel is galvanically and
logically outside the SuperSpeed data path.

Resource acquisition and safe state are separate observations. A startup
self-test sweeps all indicators and then reports whether the panel resources
were acquired. Shutdown extinguishes activity and active-route indications,
shows a bounded shutdown pattern, makes panel output pins safe, and releases
them. It does not imply that device VBUS was removed. `TP-VBUS` independently
proves the power state.

## CLI observation surface

All inspection and evidence capture is available through Make targets; an IDE
is never required. The mesh controller should expose these stable operations:

```text
make usb-mesh-endpoints
make usb-mesh-routes
make usb-mesh-route USB_SOURCE=<stable-id> USB_DESTINATION=<stable-id>
make usb-mesh-trace USB_ROUTE=<route-id>
make usb-mesh-metrics
make usb-mesh-watch
make usb-mesh-evidence USB_RUN=<run-id>
```

`endpoints` lists stable identity, role, capabilities, last observation,
session identity, local USB identity, panel health, and quarantine state.
`routes` gives a point-in-time controller snapshot. `route` reconstructs the
history of one lease epoch. `trace` follows the controller and both endpoint
agents by route ID and epoch. `watch` prints changes rather than redrawing an
ambiguous dashboard. `evidence` creates a bounded, timestamped, redacted
archive and manifest without changing a route.

Machine-readable output uses a versioned schema and canonical field names.
Human output never relies on terminal color. Every row carries:

```text
observed-at  event-sequence  source-id  destination-id  epoch
state        reason-code     reporter    freshness       correlation-id
```

Wall-clock time aids operators; the per-reporter sequence and device ownership epoch
establish ordering. Controller receipt time and endpoint observation time are
both retained. Clock-offset and uncertainty estimates are explicit rather
than silently merging distributed clocks.

Mutation remains plan/apply. The plan names the expected old epoch, proposed
new epoch, both endpoints, required detach, deadlines, and policy result.
Apply emits the same correlation ID. Concurrent or stale plans fail closed and
direct the operator to fresh status.

## Events and traces

Each state transition emits one structured event. At minimum record:

- endpoint discovery, disappearance, restart, and identity conflict;
- authenticated session establishment, expiry, and rejection;
- route request, policy decision, detach, attach, enumeration, activation,
  release, cancellation, deadline, and fault;
- per-source-device ownership epoch, controller incarnation, and correlation ID;
- local USB speed and negotiated network link speed;
- protected power-good, overcurrent, undervoltage, and commanded power change;
- stale confirmation, duplicate assignment, audit discontinuity, and state
  reconciliation;
- operator identity for mutations, without recording secrets.

Payloads, HID reports, storage contents, camera/audio data, credentials, and
complete sensitive descriptors are not logged. Device inventory uses a stable,
salted administrative identity plus the minimum descriptors needed for
policy. Logs are append-only, bounded locally, forwarded to authenticated
durable storage, and explicitly report drops or storage exhaustion.

A trace joins events by route ID, epoch, and correlation ID:

```text
controller Requested e42
source     Detaching e42
destination Detached e42
source     Exported e42
destination Enumerating e42
controller Active e42
```

An absent step remains visible as a deadline or missing reporter. Reordered
arrival never rewrites the original reporter sequence.

## Metrics

Metrics describe populations and behavior; they do not replace per-route
events. Labels use bounded values such as endpoint role, state, reason code,
transfer class, and negotiated speed. Never label metrics with raw device
serials, bus IDs, IP addresses, route IDs, or user-controlled strings.

Required counters and gauges include:

- known, online, quarantined, degraded, and faulted endpoints by role;
- requested, transitioning, active, and faulted routes;
- route requests, successful activations, releases, conflicts, stale epochs,
  deadline failures, reconciliation failures, and policy denials;
- current lease epoch per endpoint only in the bounded CLI snapshot, not as an
  unbounded metrics label;
- bytes and completed operations by transfer class;
- cancellation, retry, stall, reset, late isochronous packet, underrun,
  overrun, and disconnect counts;
- endpoint heartbeat age, event-journal backlog, dropped events, audit storage
  use, and panel communication age;
- protected-port overcurrent and undervoltage counts.

Histograms cover route detach, enumeration, and activation duration; control,
bulk, interrupt, and isochronous latency; network round trip; and endpoint
clock uncertainty. Report percentiles only from retained histogram data.
Throughput, latency, loss, and jitter remain separate observations.

Health checks distinguish process liveness, controller authority, durable
journal availability, endpoint freshness, USB readiness, and data-path
readiness. A single aggregate `healthy` value is presentation, not diagnosis.

## Predict, observe, interpret

Every mesh exercise records one row per endpoint and epoch:

| Step | Predict | Observe | Interpret |
|---|---|---|---|
| discovery | one new stable endpoint, no route | `endpoints`, `Presence`, heartbeat | identity and local detection agree |
| assignment | old route detaches before new attach | plan, trace, amber cadence | transaction order and epoch are current |
| activation | both endpoints confirm one epoch | steady green on both panels, active snapshot | logical and transport agreement exists |
| payload trial | known generated data is unchanged | hashes, rate, latency, activity pulses | tested workload passed; not universal compatibility |
| network loss | heartbeat expires and green clears | trace deadline, degraded/fault pattern | stale state was not presented as active |
| reassignment | prior destination loses route first | two endpoint traces and panels | exclusivity was maintained |
| shutdown | panel pins become safe independently of VBUS | shutdown pattern, pin measurement, `TP-VBUS` | resource release and power state are distinct |

For a scale trial, add endpoints while unrelated active routes carry generated
bulk traffic. Record discovery convergence, route churn, control-plane latency,
data-plane tail latency, event drops, and whether any unrelated epoch changes.
Test endpoint restart, duplicate identity, controller restart, link partition,
switch congestion, journal exhaustion, and simultaneous non-conflicting route
requests with deterministic fault injection before physical testing.

## Deferred physical acceptance

This document specifies required evidence; it records no bench result. Hardware
acceptance remains open until a signed run names:

- endpoint hardware, controller and kernel versions, USB/IP version, switch,
  optics/cables, VLAN, MTU, and network topology;
- source and destination stable IDs, physical port labels, device inventory,
  transfer classes, negotiated speeds, and every lease epoch;
- panel wiring, resistor values, test-point references, instruments, measured
  VBUS/power-good behavior, and indicator observations;
- generated payload hashes, latency/jitter histograms, throughput, drops,
  disconnects, resets, detach/reassignment behavior, and fault injections;
- safe shutdown and power-removal observations, deviations, reviewer, and
  date.

Until then the mesh is **model/host verified; physical and interoperability
acceptance open**. “Full USB 3,” seamless migration, arbitrary-device
compatibility, and scale limits are measured outcomes, never inferred from a
green route indicator or nominal link speed.
