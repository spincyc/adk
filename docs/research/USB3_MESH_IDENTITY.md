# USB 3 mesh identity and inventory

> **Product boundary:** [the transparent product contract](USB_TRANSPARENT_PRODUCT.md)
> is authoritative. Interpret legacy source/destination names below as
> prototype vocabulary. Product identity is rooted in `Cau`, `ComputerPort`,
> `Pau`, `PeripheralPort`, complete `TopologyRoot`, and `TopologyEpoch`.

> **Controller decision:** the initial trust domain contains one durable
> authoritative controller. Replication, consensus, leader election, and
> automatic controller failover are deferred.

Status: design guidance  
Research date: 2026-07-27  
Scope: dynamically reconfigurable USB source and destination nodes

## Decision

The mesh identifies nodes, ports, observed attachments, enrolled peripherals,
and destination hosts separately. A route names stable enrolled identities and
the controller resolves them to current transport coordinates immediately
before each operation. Linux bus IDs, USB/IP ports, IP addresses, interface
names, and display aliases are observations, never durable identity or
authorization.

Every source node may export any locally allowed peripheral. Every destination
node may import any allowed peripheral for one of its hosts. Nodes may change
addresses, ports may be re-enumerated, and routes may be reassigned while the
mesh remains live. A peripheral still has one authoritative lease at a time.
Dynamic topology does not mean simultaneous multi-host ownership.

## Identity layers

| Identity | Stable key | Current observation | Authority |
|---|---|---|---|
| Controller | provisioned public-key fingerprint | management address, incarnation, health | issues policy decisions and fenced device ownership epochs |
| Source node | enrolled node-key fingerprint | addresses, health, agent version | attests physical ports and attached devices |
| Source port | source node ID plus provisioned port ID | Linux topology path, bus ID, speed, VBUS state | bounds where a peripheral was observed |
| Peripheral | controller-issued enrollment ID | descriptors, serial, source port, bus ID | unit leased to a destination |
| Destination node | enrolled node-key fingerprint | addresses, capacity, health | attests virtual controllers and imports |
| Destination host | destination node ID plus provisioned host ID | OS instance, virtual-controller slots | receives one or more policy-allowed leases |
| Import slot | destination host ID plus stable slot ID | USB/IP port or virtual-controller coordinate | realizes one active route |
| Route | controller-issued route ID plus epoch | source and destination transport coordinates | binds one peripheral to one host |

IDs are opaque, fixed-size values. Their printed form includes a type prefix
and checksum, for example `src-...`, `port-...`, `usb-...`, and `dst-...`.
Friendly aliases such as `lab-camera` are mutable metadata. An alias lookup
must resolve to exactly one enrolled ID before a plan can be approved.

## Node identity and enrollment

Each source and destination node owns a non-exportable signing key generated
or installed during enrollment. The controller records its public-key
fingerprint, role, permitted site, administrator, enrollment time, and policy
labels. Mutual authentication proves possession on every control session.

Enrollment is an explicit, local ceremony:

1. place the unknown node in quarantine;
2. compare an out-of-band fingerprint at the physical node and controller;
3. assign its role and site;
4. enumerate and label its stable physical ports;
5. grant the least device classes, destinations, and operations needed;
6. record the operator, controller term, and resulting inventory digest.

Reinstalling an operating system does not silently inherit the old node
identity. Restoring a protected node key is recovery; presenting a new key is
replacement and requires a new enrollment or an approved replacement
transaction.

IP address, DNS name, MAC address, switch port, and machine hostname are
routing hints. They may support diagnosis, but none proves node identity.

## Stable source ports

A source port ID belongs to the enrolled node and a documented physical
connector, not to a Linux bus number. Provisioning records:

- chassis label and connector location;
- immutable node-local port number;
- controller or hub topology expected behind that connector;
- supported USB generations and lane count;
- protected-power capabilities and limits;
- whether hubs or composite devices are accepted;
- optional switch and rack metadata for operator display.

The node agent maps the stable port ID to the current kernel topology after
every boot and topology change. An ambiguous map quarantines the port. Moving a
controller card or replacing an internal hub therefore cannot silently move
trust to another front-panel connector.

Linux bus IDs such as `2-1.3` are ephemeral attachment coordinates. They may
appear in a command plan only alongside the source node ID, source port ID,
fresh inventory revision, and device ownership epoch that authorized their use.

## Peripheral identity

USB descriptors provide evidence, not a universally unique identity. Vendor
ID, product ID, class, subclass, protocol, configuration, endpoint layout,
firmware strings, and speed form a descriptor fingerprint. A valid serial
number strengthens identity, but the controller must not assume it is present,
unique, immutable, or honest.

Enrollment creates an opaque peripheral ID and records:

- descriptor fingerprint and the raw descriptor digest;
- normalized serial evidence, when present;
- source node and stable port;
- physical asset label or operator note;
- device class and authorization policy;
- first-seen time, last-seen time, and inventory revision;
- confidence and mobility policy;
- replacement lineage, initially empty.

The source agent reports an attachment-instance ID for every connect event.
It changes after disconnect, reset that destroys identity continuity, or node
restart. A route is valid only for the enrolled peripheral ID and the current
attachment instance reported by the authoritative source node.

### Serial-less and duplicate-serial devices

A serial-less peripheral cannot be identified globally from descriptors alone.
Two units of one model may be indistinguishable to software. The default
identity is therefore **port bound**:

```text
enrolled peripheral
    = source node ID
    + stable physical port ID
    + approved descriptor digest
    + current attachment instance
```

Moving that device to another source port creates an unknown observation and
requires deliberate re-enrollment or an approved move transaction. A physical
asset tag, tamper-resistant hardware identity, or device-specific
cryptographic attestation may permit a mobile policy, but an operator alias or
matching VID/PID does not.

Duplicate, malformed, or mutable serials reduce confidence to the serial-less
rule. A changed descriptor digest quarantines the attachment until policy
explicitly accepts the new version.

## Destination identity

A destination node can serve one or more explicitly enrolled destination
hosts. A host ID denotes the security principal that receives the peripheral,
not merely the Linux machine currently running `vhci-hcd`. Its record includes:

- parent destination node;
- host key or locally attested operating-system instance;
- allowed device classes and peripheral IDs;
- virtual-controller and import-slot capacity;
- reset, detach, and recovery capabilities;
- quarantine and maintenance state.

An import slot is stable controller metadata mapped to the current USB/IP port
or future virtual xHCI coordinate. Reboot may change the operating-system port
number without changing the slot ID. The node must reconcile every observed
import against the controller ledger before accepting another route.

A destination node may be dynamically repurposed, but changing the host
principal is replacement, not an address update. Existing leases are fenced,
detached, and returned to unassigned or fault before the new host can receive
them.

## Inventory records

Inventory is a signed, revisioned observation from one enrolled node:

```text
InventorySnapshot
    nodeId
    bootId
    revision
    observedAt
    expiresAt
    ports[]
    attachments[]
    importSlots[]
    previousDigest
    digest
    signature
```

Revisions increase within one boot ID. A reboot creates a new boot ID and
requires reconciliation; it never resumes an active route merely from a saved
local snapshot. The controller rejects stale, expired, skipped, malformed, or
wrong-node inventory. Full snapshots establish a base; ordered deltas may
reduce traffic only when their base revision and digest match.

An attachment record carries both stable and transient information:

```text
AttachmentObservation
    sourcePortId
    attachmentInstanceId
    peripheralId or Unknown
    descriptorDigest
    serialEvidence
    kernelBusId
    negotiatedSpeed
    powerState
    health
```

Transient coordinates never appear alone in an audit event. The event records
the stable identities, inventory revision, device ownership epoch, and exact transient
coordinates used by the adapter.

## Dynamic route resolution

A route request states intent using stable IDs:

```text
assign peripheral usb-A to destination host host-B
```

The controller then:

1. verifies source node, destination node, peripheral, host, and policy;
2. selects fresh compatible source and destination inventory revisions;
3. resolves the peripheral to one attachment instance and source port;
4. reserves one destination import slot and fabric capacity;
5. advances the peripheral device ownership epoch;
6. emits a plan bound to both node boot IDs and inventory revisions;
7. detaches any old route before authorizing the new attach;
8. accepts confirmations only from the named nodes for the current epoch;
9. marks active only after source, destination, and controller agree.

Any identity change, attachment-instance change, node reboot, stale inventory,
or ambiguous mapping invalidates the plan. Failure leaves the peripheral
unassigned or faulted; it does not reconstruct a route from aliases or current
bus numbers.

Scale comes from distributing observations and transport execution across many
nodes while retaining one authoritative lease decision per peripheral.
Controllers may later use replicated consensus, but one source device's
ownership epoch must have one logical writer. The initial durable controller is
that writer.

## Aliases, groups, and selectors

Aliases make the mesh operable:

```text
usb-camera-stage-left -> usb-4f...
editing-host          -> host-91...
room-a-sources        -> {src-10..., src-22...}
```

They do not grant access. Policy is evaluated against immutable IDs and
trusted labels after resolution. Alias mutation is audited and cannot change
an already approved plan. A selector that matches zero or multiple identities
produces a plan error unless the operation explicitly requests a set.

Groups help scalable placement, such as “any healthy destination in lab A,”
but the resulting plan records the one chosen identity, capacity decision, and
epoch. Automatic placement is a new detach-and-attach transaction, never
simultaneous ownership.

## Replacement and retirement

Replacement is explicit because similar hardware is not the same principal.

For a node replacement:

1. quarantine the new node and enroll its key;
2. map and physically verify its ports;
3. fence and reconcile every old-node lease;
4. transfer selected policy and aliases with an audited approval;
5. retire the old key so it cannot rejoin.

For a peripheral replacement:

1. leave the old peripheral ID retired or missing;
2. enroll the new unit under a new peripheral ID;
3. record `replaces` and `replacedBy` lineage;
4. copy only explicitly approved policy;
5. update aliases after the new identity is accepted.

Never reuse an ID. A repaired unit returning with its old trusted identity
requires a recovery decision; a different unit with matching descriptors is a
replacement.

## Trust and policy invariants

- One peripheral has at most one active or attaching destination lease.
- Only an enrolled source node may attest its physical port and attachment.
- Only an enrolled destination node may attest its import slot and host.
- Every mutation names a controller term, device ownership epoch, node boot IDs, and
  inventory revisions.
- A route cannot outlive the inventory evidence used to create it without
  positive renewal.
- Unknown, ambiguous, changed, or expired identity is quarantined.
- An alias, VID/PID pair, serial string, IP address, bus ID, or USB/IP port
  never authorizes a route by itself.
- Node loss, contradictory observations, or controller ambiguity prevents new
  attachment and drives reconciliation.
- Audit records preserve stable identity, transient coordinates, policy
  decision, operator or automation principal, and result.

## Required deterministic cases

The host model and adapter tests should cover:

- a bus ID changing while stable node, port, and peripheral identity remain;
- a destination USB/IP port changing after reboot;
- two serial-less devices with identical descriptors on different ports;
- duplicate and changing serial strings;
- disconnect and reconnect producing a new attachment instance;
- a device moved to an unauthorized port;
- stale inventory racing a route request;
- source or destination reboot during detach, attach, and active states;
- alias rename during an in-flight plan;
- source-node, destination-node, and peripheral replacement;
- simultaneous placement requests for one peripheral;
- automatic placement with insufficient bandwidth or import slots;
- controller partition and stale-term rejection;
- inventory digest, signature, revision, and expiry failures;
- deterministic replay of enrollment, routing, loss, recovery, and retirement.

Each trace records only injected time, identity records, inventory snapshots,
policy, and confirmations. Replaying it must produce the same selected nodes,
device ownership epochs, state transitions, reason codes, and audit digest.

## Phase-one compatibility

The current USB/IP prototype can adopt this model incrementally:

1. assign controller-issued IDs to the existing source node, source port,
   peripheral, destination node, host, and import slot;
2. add signed or locally authenticated inventory snapshots;
3. bind every adapter command to boot ID, inventory revision, attachment
   instance, and controller epoch;
4. replace the temporary adapter ledger with the authoritative controller;
5. add multi-node discovery and placement without weakening exclusive leases;
6. add replicated control only after single-writer recovery is proven.

Until those steps land, `USB_DEVICE_NODE`, `USB_HOST_NODE`, `USB_BUS_ID`, and
`USB_PORT` are laboratory coordinates. They do not constitute a scalable mesh
identity model.

## Open design decisions

- Which hardware root of trust, if any, is required for production nodes?
- Does the first mesh bind serial-less devices permanently to ports, or permit
  operator-approved moves between pre-enrolled ports?
- Which destination principal is authoritative: node OS, VM, container, or a
  dedicated virtual xHCI function?
- What inventory lease duration balances failure detection against controller
  and node churn?
- Which device classes may use automatic placement?
- Does a source port permit one direct device only, or can policy enroll a hub
  and its complete subtree?
- What replicated-controller model and quorum size are appropriate after the
  single-controller phase?
