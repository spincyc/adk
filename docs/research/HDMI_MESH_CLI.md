# HDMI mesh CLI contract

> **Controller decision:** the intended first implementation uses one durable
> authoritative controller as a service on an ordinary Linux computer. It
> never carries media. Replication, quorum, leader election, and
> automatic controller failover are deferred. Mutations fail closed while the
> controller is unavailable.

Status: proposed research control-plane contract  
Scope: dynamically routed, unprotected HDMI essences across authenticated
media endpoints on the shared managed network

This document specifies an operator vocabulary and command shape. It does not
claim that `adk-hdmi-mesh`, its Make targets, an HDMI endpoint, or an 8K media
plane is implemented. The initial laboratory profile accepts generated,
public-domain, or otherwise authorized unencrypted media only.

Each source-facing appliance terminates an HDMI link, interprets negotiated
video, audio, timing, and metadata, then publishes network media flows. Each
destination-facing appliance receives compatible flows and constructs a new
HDMI link for its sink. The design does not forward raw TMDS or FRL symbols
through Ethernet.

The Mega 2560 may provide buttons, indicators, and read-only health evidence.
It never carries media, manages EDID, holds protected-content keys, allocates
routes, or becomes controller authority.

## Mesh vocabulary

Use controller-issued stable identifiers:

```text
node:stage-left
input:camera-a
output:wall-center
profile:wall-8k60
route:camera-to-wall
```

A node is an authenticated media appliance. An input endpoint terminates one
source-side HDMI connection and publishes its interpreted essences. An output
endpoint reconstructs one sink-side HDMI connection. A route is desired media
intent between one input and one or more outputs. An EDID profile is a
versioned capability policy presented to a source; it is not a raw,
unreviewed sink blob.

Hostnames, addresses, HDMI connector labels, multicast groups, RTP ports, and
FPGA lane assignments are mutable inventory attributes, never identities.
Replacing an appliance or endpoint creates a new incarnation. Old
observations cannot activate a new incarnation.

One output belongs to at most one active route. Phase one is one-to-one.
Phase-three fan-out may allow an input to feed multiple outputs when admission
control reserves the complete sender, receiver, switch, clock, and multicast
capacity. A route never becomes active merely because the controller records
intent.

## Command shape

The proposed executable is `adk-hdmi-mesh`. Read-only commands run directly.
Every mutation uses an immutable, expiring plan:

```sh
adk-hdmi-mesh <mutation> ... --plan
adk-hdmi-mesh plan show plan:01J...
adk-hdmi-mesh apply     plan:01J...
```

Planning validates actor policy, identities, endpoint incarnations,
capabilities, EDID policy, bandwidth, multicast resources, clock domain,
format conversion, latency budget, and current source epochs. Applying
uses that exact plan once and never silently replans changed state.

All commands default to concise human output and support stable records through
`--output json`. Scripts use identifiers and JSON fields rather than display
names or terminal columns. Successful mutations print their audit event,
source epoch, and resulting desired state.

Equivalent future Make targets may wrap the executable:

```sh
make hdmi-mesh-inventory
make hdmi-mesh-route-plan ROUTE=route:camera-to-wall
make hdmi-mesh-route      PLAN=plan:01J...
make hdmi-mesh-status
make hdmi-mesh-watch
```

These targets are guidance, not implemented repository interfaces.

## Inventory and enrollment

Discovery observes hardware without authorizing it:

```sh
adk-hdmi-mesh node list
adk-hdmi-mesh endpoint discover --node node:stage-left
adk-hdmi-mesh endpoint list --role input
adk-hdmi-mesh endpoint list --role output --healthy
adk-hdmi-mesh endpoint show input:camera-a --output json
```

An input inventory includes connector presence, RX lock, source identity when
available, negotiated HDMI mode, FRL or TMDS state, video format, audio,
InfoFrames, timing, encryption state, packetizer modes, clock health, and
measured media counters.

An output inventory includes connector presence, HPD, sink EDID digest,
supported modes, transmitter capability, audio, metadata, buffer limits,
clock health, and output-link state. Discovery must show unsupported,
ambiguous, protected, or malformed observations explicitly.

Enrollment is a planned mutation:

```sh
adk-hdmi-mesh endpoint enroll input:camera-a \
    --node node:stage-left \
    --connector hdmi-rx-1 \
    --plan

adk-hdmi-mesh apply plan:01J...
```

Enrollment binds an identity to an authenticated node, connector, and endpoint
incarnation. Cable changes update observations, not identity. Hardware
replacement requires an explicit adoption or new enrollment plan.

## Capability inspection

Operators inspect compatibility before requesting a route:

```sh
adk-hdmi-mesh capability show input:camera-a
adk-hdmi-mesh capability show output:wall-center
adk-hdmi-mesh capability compare \
    input:camera-a \
    output:wall-center

adk-hdmi-mesh format list --input input:camera-a
adk-hdmi-mesh format explain format:8k60-rgb-8
```

Capability records name exact limits: raster, scan mode, frame rate, chroma,
component depth, fixed or variable timing, audio layouts, HDR and other
metadata, DSC handling, transport mode, clock requirements, latency class, and
required network reservation. “HDMI 2.1” and “8K” are not sufficient
capability claims.

A comparison reports one of:

```text
native
compatible through named conversion
blocked by policy
unsupported
unknown
```

Conversion is never implicit. A plan names every scaler, frame synchronizer,
chroma or depth conversion, codec, audio map, metadata rule, and added latency.
Unknown capability blocks apply.

## EDID profiles

EDID is route policy, not incidental connector data. Preserve the raw observed
sink EDID as evidence, then derive a reviewed, immutable profile:

```sh
adk-hdmi-mesh edid observe output:wall-center
adk-hdmi-mesh edid profile list
adk-hdmi-mesh edid profile show profile:wall-8k60
adk-hdmi-mesh edid profile derive \
    --from-output output:wall-center \
    --name profile:wall-8k60 \
    --plan

adk-hdmi-mesh apply plan:01J...
```

A profile records its source digest, admitted modes, preferred timing, audio,
HDR metadata policy, VRR policy, DSC policy, version, review reason, and
author. Editing creates a new profile version.

Bind a profile to an input with a plan:

```sh
adk-hdmi-mesh edid assign input:camera-a \
    --profile profile:wall-8k60 \
    --plan
```

The plan states whether applying the profile asserts HPD, retrains the source,
interrupts existing routes, or changes the published flows. It cannot promise
that a source will select the preferred mode. Observed negotiation remains
separate from requested EDID policy.

For one input feeding several outputs, use an explicit reviewed profile or a
controller-computed intersection saved as a new immutable profile. Never
silently copy whichever sink connected most recently.

## Create a route

Name the media intent before transport details:

```sh
adk-hdmi-mesh route create route:camera-to-wall \
    --input input:camera-a \
    --output output:wall-center \
    --edid-profile profile:wall-8k60 \
    --latency-budget 2ms \
    --plan

adk-hdmi-mesh plan show plan:01J...
adk-hdmi-mesh apply     plan:01J...
```

The plan reads as an ordered transaction:

```text
authorize actor, input, output, and profile
verify endpoint incarnations and current source epochs
reserve endpoint, switch, multicast, bandwidth, and clock resources
stage video, audio, and metadata receivers
acquire packet and timestamp lock
construct and verify the destination HDMI link
activate at the named boundary
publish active only after controller and endpoints agree
```

`active` requires agreement on endpoint incarnations, source epoch, the
separately named sink reservation, media flow identifiers, format, clock state,
and output link. A lit display alone does not prove that the requested source
or metadata arrived.

Phase three adds fan-out through a planned route update:

```sh
adk-hdmi-mesh route add-output route:camera-to-wall \
    --output output:lobby \
    --plan
```

Admission accounts for the complete route set. Multicast does not make
receiver, switch-table, uplink, PTP, or source capacity free.

## Move and reconfigure

Both sides are dynamically reconfigurable:

```sh
adk-hdmi-mesh route move route:camera-to-wall \
    --to-input input:camera-b \
    --plan

adk-hdmi-mesh route move route:camera-to-wall \
    --from-output output:wall-center \
    --to-output output:wall-right \
    --plan

adk-hdmi-mesh route move route:camera-to-wall \
    --to-input input:camera-b \
    --from-output output:wall-center \
    --to-output output:wall-right \
    --plan
```

The plan chooses and reports a transition policy:

- `cut`: stop the old output, then start the new route;
- `boundary`: switch at a verified frame boundary;
- `prejoin`: stage compatible new flows before the boundary, then release old
  reservations after output verification.

`prejoin` is make-before-break media switching, not duplicate output ownership.
The same output still has one route authority. If compatibility, bandwidth,
clock, or verification fails, the route remains on its previous verified state
when that state is still authoritative; otherwise it becomes explicitly
inactive or faulted. The controller never invents a rollback after losing
knowledge of endpoint state.

A change to format, EDID profile, latency, audio map, metadata policy, codec,
or path is the same kind of versioned planned mutation:

```sh
adk-hdmi-mesh route configure route:camera-to-wall \
    --edid-profile profile:wall-4k60 \
    --plan
```

The profile policy is explicit:

```sh
adk-hdmi-mesh route configure route:camera-to-wall \
    --pin-profile profile:wall-4k60 \
    --on-contract-loss blank-mute \
    --plan
```

An alternative policy may name an ordered allowed set or bounded automatic
selection. The plan shows requested and selected profiles, peak bandwidth,
latency, jitter, codec, and ordinary-LAN headroom. It never silently lowers a
format. A rejected candidate leaves the existing working route unchanged.

Deterministic fault injection is a separate disabled-by-default laboratory
command group. It requires test authorization, a bounded duration, a named test
route, an endpoint `TEST` display, and separate audit events. Real faults
override injected observations.

## Release and remove

Release stops media and frees reservations while preserving the named route:

```sh
adk-hdmi-mesh route release route:camera-to-wall --plan
adk-hdmi-mesh apply         plan:01J...
```

Remove deletes an already released route record:

```sh
adk-hdmi-mesh route remove route:camera-to-wall --plan
adk-hdmi-mesh apply        plan:01J...
```

Release completes only after outputs report the planned inactive source epoch
and reservations are reconciled. Endpoint loss, ambiguous output state, audit
failure, or controller restart leaves a visible fault and blocks conflicting
assignment until reconciliation.

## Status, watch, and evidence

Status separates desired, staged, and observed state:

```sh
adk-hdmi-mesh status
adk-hdmi-mesh route show route:camera-to-wall
adk-hdmi-mesh status --node node:stage-left --output json
```

Watch emits a snapshot followed by ordered events:

```sh
adk-hdmi-mesh watch
adk-hdmi-mesh watch --route route:camera-to-wall --since-event 1842
adk-hdmi-mesh watch --output json
```

Each event includes its ID, controller incarnation, source epoch, endpoint
incarnations, logical time, transition, reason, plan digest, and audit digest.
Reconnect either resumes exactly or requires a fresh snapshot. Watch is
read-only and cannot confirm or apply a plan.

Concise route states are:

```text
inactive -> reserving -> staging -> locking -> switching -> active
                                     \-> fault
```

`degraded` supplements `active` when media remains present but a declared
service level is not met. Policy may propose a plan; it never silently reroutes
or changes format.

Capture a bounded evidence bundle:

```sh
adk-hdmi-mesh evidence capture route:camera-to-wall \
    --duration 10s \
    --output build/hdmi-evidence/camera-to-wall

adk-hdmi-mesh evidence verify \
    build/hdmi-evidence/camera-to-wall/manifest.json
```

The bundle records repository and endpoint revisions, plan and audit digests,
capabilities, EDID/profile digests, negotiated modes, SDP or equivalent flow
descriptions, source epochs, PTP state, RTP sequence errors, packet loss
and reorder, buffer bounds, video CRC or test-pattern counter, audio state,
metadata state, link alarms, switch counters, and timestamps. It does not
capture protected payloads.

## Circuit-native observability

Each physical endpoint provides evidence without Serial or the CLI:

- blue: enrolled and inactive;
- pulsing amber: plan in progress;
- green: controller and both media edges agree on the active source epoch;
- red: fault or quarantined state;
- separate RX lock, PTP lock, buffer alarm, and TX lock indicators;
- a local generated test-pattern button;
- an output border or frame counter generated after network receive;
- named test points for frame start, PTP pulse, buffer alarm, and output
  enable.

Its local display names the requested and applied profile, route state, link
rate, latency class, and exact failure. Text or symbols carry the meaning
without depending on color.

Panel state proves control-plane agreement. RX/TX lock and test points prove
specific electrical or timing observations. A visible test pattern and
advancing frame counter prove more than link lock, but neither proves every
pixel, audio sample, or metadata item. The evidence bundle provides the
complementary counters and hashes.

For each experiment:

1. predict the endpoint indicators, test-point timing, negotiated format, and
   counter changes;
2. observe them at named connectors or test points;
3. interpret which route, timing, payload, and safe-state claims they support.

Resource acquisition and inactive output are separate checks. Serial logs are
optional supporting evidence only.

## Authority, failure, and security

One durable controller allocates plan IDs and monotonically increasing,
non-wrapping source epochs. Endpoint agents accept only authenticated commands for their
identity and current incarnation. They report observations but never promote
themselves, allocate routes, or extend authority locally.

Controller loss permits read-only local evidence but blocks plan creation and
apply. Endpoints retain only the controller-issued state allowed by policy;
only the exact unchanged installed route may run until its bounded lease
expires, when the transmitter mutes. They do not renew, change, or expand it and
do not elect a replacement. Restart reconciles durable desired state,
endpoint observations, reservations, and the audit journal before another
mutation.

Default state is no route. Management, HDMI media, USB, timing, telemetry, and
ordinary LAN traffic share one managed fabric with logical security domains and
traffic classes. Unknown endpoints, malformed EDID, protected
input, incompatible formats, stale plans, incarnation changes, clock loss,
bandwidth exhaustion, packet loss, buffer fault, thermal alarm, and audit
failure are explicit fault cases.

Do not decrypt, capture, bridge, or reconstruct protected media outside a
licensed HDMI/HDCP product. A later licensed repeater path needs separate
architecture, key custody, compliance, and legal review; it is not enabled by
a CLI flag.

## Initial implementation boundary

The proposed first executable slice should use synthetic raster/audio/metadata records
and fake endpoint adapters. It proves:

- stable identity and endpoint incarnation handling;
- immutable plan/apply behavior and stale-plan rejection;
- EDID profile versioning and capability intersection;
- exclusive output ownership and a one-to-one phase-one route;
- deterministic route create, move, configure, release, and restart traces;
- bounded evidence manifests and tamper-evident audit references;
- single-controller fail-closed behavior.

Later adapters may target reduced-format software media, FPGA test patterns,
1080p packet transport, and eventually validated 8K hardware. Each boundary
must remain named honestly. Simulation, compilation, link lock, and a visible
image are different evidence levels.
