# USB 3 mesh security

> **Product boundary:** [the transparent product contract](USB_TRANSPARENT_PRODUCT.md)
> is authoritative. Security applies to physical Cau/Pau attachment units,
> complete topology roots, protected port power, and the shared household
> network. Linux USB/IP remains a disposable-lab prototype only.

> **Controller decision:** the supported initial mesh has one durable
> authoritative controller. High availability, replication, consensus, and
> automatic leader failover are deferred. Controller loss fails closed.

Status: security requirements for the research architecture  
Scope: dynamically assign any authorized device-facing endpoint to any
authorized host-facing endpoint across a switched network

## Security conclusion

Dynamic routing turns every endpoint and the controller into part of a
distributed security boundary. A scalable mesh cannot trust a node because it
has an expected IP address, USB identifier, or physical label. Nodes,
operators and the controller need cryptographic identities; routes need
short, exclusive, monotonically fenced leases; and endpoint enforcement must
remain fail-closed when the network or control plane disagrees.

The lab prototype does not provide these guarantees. Its USB/IP commands and
temporary ledger are suitable only for one operator, known disposable hosts,
known harmless devices, and a controlled VLAN on the shared household network.
It must not be described as a secure, highly available, or deployable mesh.

## Terms and invariants

Product terms are one-port `Cau`, `ComputerPort`, four-root `Pau`,
`PeripheralPort`, atomic `TopologyRoot`, and `TopologyEpoch`. The generic
device/host endpoint wording below applies to the USB/IP prototype. Product
appliances have fixed electrical roles and never synthesize a hub.

A **device endpoint** owns one or more physical USB devices and exports them. A
**host endpoint** reconstructs attachment for a destination host. An appliance
may implement both roles, but each port has one role for a ownership epoch.
The **durable controller** authorizes desired assignments. The **data plane**
carries USB requests only after both endpoints install the same authorization.

The mesh preserves these invariants:

- one physical device or indivisible hub subtree has at most one host lease;
- one port has one role and one active ownership epoch;
- a newer ownership epoch permanently fences every older ownership epoch;
- an endpoint accepts data only from the peer named by its current lease;
- uncertain ownership, expired authority, or conflicting state means detached;
- reassignment creates observable USB detach and enumeration boundaries;
- device power is separately authorized and never proves route ownership;
- the Mega 2560 panel proposes changes and displays evidence; it has no routing
  authority or long-term mesh credential.

Hosts may own several devices and endpoint appliances may serve several ports.
Those are independent exclusive leases, not shared attachment. Policy may cap
routes per host, node, tenant, class, bandwidth pool, or fault domain.

## Threat model

Protect against:

- a hostile USB peripheral attacking a host driver, impersonating another
  device class, injecting input, or exposing private storage or sensors;
- a compromised host attempting to claim unauthorized devices or retain access
  after reassignment;
- a compromised endpoint forging inventory, replaying a lease, redirecting
  traffic, or observing another tenant's USB stream;
- a network attacker reading, modifying, delaying, replaying, or redirecting
  control and USB traffic;
- a stale, partitioned, or compromised controller issuing conflicting routes;
- an operator making an unsafe bulk reassignment or using the wrong physical
  port;
- lost credentials, rollback to vulnerable software, audit deletion, and
  denial-of-service through enumeration or route churn.

Initial scope does not promise continued service after node compromise,
Byzantine controller consensus, protection from physical hardware implants, or
safe use of arbitrary peripherals. Those require a separate assurance program.

## Identity and mutual authentication

Give every controller, endpoint appliance, and administrative operator a
unique identity. Do not use VID/PID, USB serial strings, MAC addresses, DNS
names, or switch ports as authenticators.

- Bootstrap appliance identity through a documented physical enrollment step.
- Store private keys in a TPM or equivalent hardware-backed keystore for a
  deployable node.
- Mutually authenticate every control-plane and data-plane session.
- Bind the session to the node identity, role, protocol version, and current
  ownership epoch.
- Use fresh nonces and replay-resistant secure transport; reject expired
  certificates and unknown trust roots.
- Rotate credentials without interrupting unrelated leases. Revocation must
  take effect without waiting for a normal certificate lifetime.
- Treat recovery, replacement, and factory reset as new enrollment events.

The lab prototype may use manually provisioned short-lived certificates, but
shared keys, trust-on-first-use, and plaintext USB/IP remain lab-only.

## Authorization

Authorization is default-deny and evaluates the complete tuple:

```text
operator, source device port, observed inventory, destination host port,
tenant, requested duration, bandwidth class, power action, policy revision
```

Separate permissions to discover, propose, approve, assign, revoke, reset, and
change VBUS. High-impact operations support two-person approval and a bounded
change window. An assignment token is signed, names both endpoint identities
and ports, carries one ownership epoch and expiry, and cannot authorize a different
peer or device.

USB descriptors are observations, not identity proof. Inventory combines a
physical port enrollment, descriptor digest, expected class, optional device
attestation, and operator label. A mismatch routes the device to quarantine,
never to the previously authorized host.

## Hostile peripherals and isolation

Remote USB has the authority and attack surface of local USB. Assume every
newly attached peripheral is hostile.

- Begin with disposable hosts, non-sensitive generated data, and deliberately
  selected bulk devices.
- Reject HID, security tokens, personal storage, cameras, microphones, hubs,
  vendor-specific devices, and composite devices until each has an explicit
  policy and compatibility record.
- Use host USB authorization and driver allowlists where available.
- Run endpoint daemons with least privilege, syscall and filesystem
  confinement, bounded memory, and no access to unrelated ports.
- Place tenants and management traffic in separate network security domains.
- Use independent IOMMU groups or dedicated controllers where DMA and driver
  isolation matter; logical port labels alone are insufficient.
- Quarantine descriptor changes, reset storms, excess bandwidth, malformed
  traffic, unexpected classes, and repeated enumeration failures.

A deployable system requires threat review and fuzzing of the tunnel,
controller API, endpoint parser, persistence format, and supported USB classes.
Passing ordinary USB devices is compatibility evidence, not security evidence.

## Lease and reassignment protocol

The controller commits a desired assignment before endpoints activate it.
Each source device has a monotonically increasing ownership epoch allocated by
the authoritative controller's durable store. Epoch rollover is a fatal
administrative condition, not wraparound.

1. Authorize the full assignment tuple; reserve destination and network
   capacity separately from ownership.
2. Issue a signed prepare record to both named endpoints.
3. Quiesce new transfers and bound completion or cancellation.
4. Detach the old host and receive durable acknowledgement for its ownership epoch.
5. Fence the old ownership epoch at the device endpoint.
6. Install the new ownership epoch at both endpoints.
7. Activate the data session only after mutual peer and ownership epoch agreement.
8. Let the destination host enumerate normally.
9. Record observed state independently from desired state.

Every step has a deadline, idempotency key, reason code, and deterministic
recovery rule. A failed move leaves the device unassigned. Automatic restoration
to the old host is forbidden unless it is a new, explicitly authorized
ownership epoch.

## Split brain and partitions

Only the durable controller may issue assignments. Endpoint nodes do not
resolve conflicts by timestamps, node priority, last writer wins, or operator
preference. A future replicated controller must preserve this single logical
authority through consensus.

- Persist the highest accepted ownership epoch before activating a route.
- Reject lower or equal ownership epochs after restart.
- Require a controller-issued ownership fence, not merely a valid signature.
- Stop accepting new transfers when a lease expires or control authority
  becomes ambiguous.
- Make the fail-closed interval explicit for devices whose abrupt detach can
  corrupt data.
- Reconcile desired, device-endpoint, host-endpoint, USB/IP, and protected-power
  state after every restart or partition.

Availability cannot override exclusive ownership. Deployments that need
continued operation during a controller outage require pre-authorized,
time-bounded leases and an analyzed renewal margin; they do not permit local
lease invention.

## Revocation

Revocation targets an operator, controller, endpoint, port, device inventory,
tenant, policy revision, or individual lease. It increments the source
device's ownership epoch and orders detach before any reassignment.

Emergency administrative isolation may disable the network port or protected
device VBUS, but forced power loss can corrupt or damage a peripheral and is
not routine revocation. A physical endpoint power disconnect is independent of
software and is not described as an emergency stop.

Revocation evidence includes request identity, approval, policy decision,
ownership epoch, endpoint acknowledgements, detach observation, timeout, power
action, and final reconciled state.

## Audit

Record desired and observed state separately. Each record contains:

- globally unique operation and idempotency identifiers;
- authenticated actor, approver, controller term, and policy revision;
- endpoint identities, physical ports, inventory digest, and ownership epoch;
- prepare, detach, fence, attach, enumerate, expire, revoke, reset, power, and
  fault events with monotonic and wall-clock timestamps;
- command result, endpoint acknowledgement, timeout, and reconciliation result.

Send append-only records to an independent sink with authenticated ordering and
retention controls. Local hash chaining detects some edits but does not prevent
a compromised node from truncating its history. Avoid sensitive USB payloads
and unnecessary descriptor strings in logs. Access, export, retention, and
deletion policies must address device and user privacy.

## Safe scaling and rollout

Scale by adding independently identified endpoint ports and scheduling
exclusive leases through one logical authority. Do not scale by running
independent ledgers.

1. One controller, one source port, one destination port, synthetic bulk data.
2. Multiple ports on one endpoint, with conflict, restart, and fencing tests.
3. Multiple endpoints on one switch, with network isolation and capacity
   admission.
4. Controller restart, forced partitions, stale commands, and restore testing.
5. One explicitly supported USB class at a time, with hostile-device testing.
6. Canary nodes, signed updates, rollback protection, and automatic quarantine.
7. Multi-tenant deployment only after independent security review.

Bulk changes first produce a complete plan: affected leases, detach order,
capacity, expected enumerations, rollback ownership epochs, and stop conditions.
Rate-limit assignment churn so one faulty operator or node cannot repeatedly
reset an entire mesh.

## Prototype versus deployable system

| Property | Phase-one lab prototype | Deployable mesh requirement |
|---|---|---|
| Network | Isolated wired lab | Segmented, monitored, capacity-managed fabric |
| Operator | One trusted operator | Authenticated roles and approval policy |
| Identity | Manual endpoint configuration | Hardware-backed node identity and lifecycle |
| Transport | USB/IP experiment | Mutually authenticated, encrypted, replay-resistant |
| Authority | Temporary local ledger | Durable single-controller epochs and fencing |
| Failure | Inspect and manually reconcile | Bounded, idempotent, fail-closed reconciliation |
| Peripherals | Known harmless test device | Explicit class/device policy and quarantine |
| Isolation | Disposable test hosts | Least privilege, tenant, driver, DMA, and port isolation |
| Audit | Local experimental evidence | Independent append-only audit and retention |
| Updates | Manual lab maintenance | Signed updates, measured rollout, rollback protection |

No Make target or passing host test promotes the prototype to the right-hand
column. Deployment requires implementation evidence, adversarial tests,
operational procedures, and independent review for the intended environment.

## Required deterministic evidence

Before the architecture advances, tests must cover:

- arbitrary allowed endpoint pairs and rejection of every forbidden pair;
- simultaneous claims, reassignment, stale confirmation, and duplicate replay;
- controller restart before and after every transaction boundary;
- endpoint crash, restart, rollback, clone, certificate expiry, and revocation;
- delayed, reordered, duplicated, dropped, and forged control messages;
- lease expiry during control, bulk, interrupt, and isochronous activity;
- audit sink failure, full storage, malformed persistence, and clock changes;
- descriptor change, hostile class, hub insertion, and physical-port mismatch;
- deterministic reconciliation to one active ownership epoch or no assignment.

Physical testing remains a separate deferred acceptance gate. Simulated and
host tests must never be reported as proof of USB compatibility, electrical
behavior, isolation, or deployment security.
