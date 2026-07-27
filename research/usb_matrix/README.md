# USB matrix lease controller

> **Prototype only:** this directory models Linux USB/IP leases. Its source,
> destination, virtual-host, bus-ID, and port vocabulary is not the transparent
> product model. The product contract is
> `docs/research/USB_TRANSPARENT_PRODUCT.md`: physical one-port Caus, four-root
> Paus, exact real topology, native Windows/Linux USB, and no Cau software.

This host-only prototype is the authoritative route state machine for the
USB/IP experiment. It owns logical leases, not USB transfers. A transport
adapter executes detach and attach commands, then confirms their result with
the controller's current epoch.

## Target: dynamic endpoint mesh

The intended system is not a fixed source-to-destination matrix. It is a
scalable mesh of independently enrolled endpoints:

- a **source endpoint** exports one or more physical USB devices;
- a **destination endpoint** provides one or more virtual host-controller
  ports;
- a **route** assigns one exported device to one destination port;
- a **controller** reconciles desired routes with endpoint-reported state.

Every source, destination, device, and destination port needs a stable logical
identity independent of its current address, USB bus ID, or USB/IP port
number. Endpoints may join, leave, or change addresses without changing those
identities. An authorized operator may create, move, or release any compatible
route at runtime. Reconfiguration remains break-before-make: the old
destination must acknowledge detachment before a new destination may attach
the device.

The mesh is scalable in endpoint count, but an individual physical USB device
still has exactly one active destination. Mesh means arbitrary dynamic
pairing, not multicast or simultaneous host ownership. Source and destination
roles may coexist on one node, but their identities, permissions, capacity,
and health are reported separately.

The current fixed-capacity controller proves per-device routing invariants
only. It does not yet implement distributed endpoint discovery, durable
inventory, destination-port allocation, controller failover, or a network
protocol. Those are required before the prototype can claim mesh operation.

## Transaction narrative

```text
requestRoute(host, device)
    -> Attaching
    -> confirmAttached(host, device, epoch)
    -> Active

requestRoute(newHost, activeDevice)
    -> Detaching
    -> confirmDetached(device, epoch)
    -> Attaching
    -> confirmAttached(newHost, device, epoch)
    -> Active
```

Only one active host is stored for a device. Reassignment cannot enter
`Attaching` until the old host confirms detachment. Every new transaction
advances the device epoch; late confirmations return `StaleEpoch` without
changing state.

Endpoint loss enters `Fault`, forgets active and pending hosts, and requires
`clearFault()` before another request. Loading persistent state advances every
stored epoch and restores any formerly active transaction as `Fault`. Restart
therefore never asserts that a remote USB attachment still exists.

## Audit and persistence

`AuditLog` is a fixed-capacity ordered journal. Every entry includes logical
time, device, host, epoch, action, state, the preceding hash, and its own
deterministic FNV-1a hash. `verify()` detects accidental or unsophisticated
record modification. It is not a cryptographic signature; a deployed
controller must append records to authenticated durable storage.

`save()` writes versioned controller state. `load()` validates into temporary
storage before replacing live state. The outer Linux service is responsible
for the durable-write transaction:

1. write a new file in the same directory;
2. flush the file;
3. atomically rename it over the prior snapshot;
4. flush the parent directory;
5. retain the append-only audit journal separately.

## USB/IP adapter contract

The Python USB/IP CLI remains the operating-system adapter. It should consume
this model through a small process or serialization boundary:

- ask the controller for the next required action;
- run commands without a shell;
- report success or failure with `device` and `epoch`;
- detach before attach on every reassignment;
- reject results whose epoch is no longer current;
- persist state after each accepted transition;
- default to a printed plan until the operator supplies `--execute`.

The adapter must not independently allocate lease generations. One persisted
controller is authoritative. A production service adds a single-writer lock,
mutual authentication, authorization policy, deadlines, and a cryptographic
append-only log before it can coordinate multiple machines.

## Host gates

```sh
make usb-matrix-controller-test
make usb-matrix-security-test
make usb-matrix-test
```

The controller and independent security suites compile as separate strict
C++17 binaries with exceptions and RTTI disabled. `usb-matrix-test` runs both
of them and the Python dry-run USB/IP adapter suites. No Arduino IDE or USB
hardware is required.

These passing gates establish the route model and adapter command planning.
They do not establish an integrated authoritative service: the Python
phase-one adapter still uses its explicitly temporary ledger. A later process
bridge must make this controller the sole epoch and lease authority before an
executed USB/IP route can claim end-to-end fencing.
