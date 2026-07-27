---
title: "USB 3 network matrix — phase 1"
---

# USB 3 network matrix — phase 1

Phase 1 is an executable Linux USB/IP control-plane baseline. It discovers
exportable devices, offers a side-effect-free plan for each mutation, and
records one exclusive device lease per host after a successful attachment. It
does not yet demonstrate USB 3 throughput or broad device compatibility.

The first mesh milestone uses one durable authoritative controller and Linux
destination systems running USB/IP clients. Controller replication, consensus,
automatic failover, and transparent attachment to unmodified hosts are
deferred. Controller loss fails closed; endpoint agents do not elect or promote
an authority.

The Mega 2560 is not in the USB data path. A later circuit may provide route
selection and visible lease status, while Linux endpoints terminate USB and
carry requests and completions across the switched network.

Start and validate the local, non-mutating tooling entirely through Make:

```sh
make usb-matrix-setup
make usb-matrix-check
make usb-matrix-doctor
make usb-matrix-status
```

`doctor` checks commands without loading a module or touching a device.
`usb-matrix-log` prints the temporary ledger and explicitly warns that it is
not an authoritative audit journal.

## Predict

Start with two Arch Linux machines on an isolated wired network and one
non-sensitive test device. Before changing either endpoint, predict:

- which bus ID the device node will export;
- which host will hold the only active lease;
- which `usbip` port will appear after attachment;
- that a failed attach will leave no lease record.

The plan targets print commands without changing kernel modules, USB bindings,
attachments, or the lease ledger:

```sh
make usb-local
make usb-remote USB_DEVICE_NODE=device-a
make usb-export-plan USB_BUS_ID=2-1
make usb-assign-plan \
    USB_DEVICE_NODE=device-a \
    USB_BUS_ID=2-1 \
    USB_HOST_NODE=host-a
```

Review node names and the generated command sequence before using an execution
target.

## Observe

Execution requires explicit targets and sufficient operator-provided privilege.
The controller never inserts `sudo` or invokes a shell:

```sh
make usb-export USB_BUS_ID=2-1
make usb-assign \
    USB_DEVICE_NODE=device-a \
    USB_BUS_ID=2-1 \
    USB_HOST_NODE=host-a
make usb-ports
```

Observe the imported device through normal Linux enumeration, the active
virtual-host-controller port through `make usb-ports`, and the JSON lease at
`build/usb-matrix/leases.json`. These are separate observations: a lease record
states routing intent, while enumeration and a device-specific operation
provide dataplane evidence.

Release the route only after identifying the actual USB/IP port:

```sh
make usb-release-plan USB_HOST_NODE=host-a USB_PORT=0
make usb-release      USB_HOST_NODE=host-a USB_PORT=0
```

## Interpret

- A successful attach followed by enumeration supports one working route; it
  does not establish “full USB 3.”
- A rejected second assignment demonstrates controller exclusivity; it is not
  distributed consensus between independent controllers.
- A failed attach must create no lease. A failed detach must preserve the lease
  for investigation.
- Bulk, control, interrupt, isochronous, hub, reset, suspend, and reconnect
  behavior require separate measured cases.
- Nominal USB and Ethernet rates are not payload-throughput evidence.

The adapter has sixteen deterministic host tests covering command recording,
lease failures, hostile identifiers, and command timeouts. They invoke no
kernel or network operation. Its JSON ledger is intentionally temporary,
single-process experimental state: it has no locking or transactional
coordination with another controller. Real USB/IP endpoint, switch, throughput,
latency, loss, recovery, power, and device-corpus testing remain deferred.

## Deterministic route model

The hardware-neutral `adk::usbmatrix::MatrixController` models a partial
one-to-one device/host route. It requires detach confirmation before attaching
a replacement route, advances a monotonically fenced epoch, ignores stale
acknowledgments, and faults closed on a missed deadline or lost endpoint.
Saving and loading state never restores an active claim after restart.

`requestRoute()`, `detachRoute()`, `confirmDetached()`, `confirmAttached()`,
`reportEndpointLost()`, `clearFault()`, `update()`, and `snapshot()` define the
transition vocabulary. An `AuditLog` supplies a verifiable hash chain for the
decision record. Strict native tests cover this authoritative model. The
Python ledger is deliberately incompatible with it; a durable controller
process bridge, locking, transactional persistence, and deployment hardening
remain explicit integration work.

## Boundaries and references

Do not expose USB/IP to the Internet or attach unknown peripherals. USB data
routing does not grant permission to switch device power, and a remote
peripheral has the authority of a local one.

- [Architecture and staged test matrix](../docs/research/USB3_NETWORK_MATRIX.md)
- [Complete command-line operator contract](../docs/research/USB3_MATRIX_CLI.md)
- [Dynamic mesh architecture](../docs/research/USB3_MESH_ARCHITECTURE.md)
- [Stable identity and inventory](../docs/research/USB3_MESH_IDENTITY.md)
- [Fenced control protocol](../docs/research/USB3_MESH_PROTOCOL.md)
- [Deterministic reconciler](../docs/research/USB3_MESH_RECONCILER.md)
- [Mesh operator CLI](../docs/research/USB3_MESH_CLI.md)
- [Linux USB/IP data plane](../docs/research/USB3_MESH_DATA_PLANE.md)
- [Mesh security model](../docs/research/USB3_MESH_SECURITY.md)
- [Scaling model](../docs/research/USB3_MESH_SCALING.md)
- [Observability and circuit evidence](../docs/research/USB3_MESH_OBSERVABILITY.md)
- [Deterministic and physical test plan](../docs/research/USB3_MESH_TEST_PLAN.md)
- [Shared electrical, security, and licensing constraints](../docs/NETWORK_MATRIX_CONSTRAINTS.md)
- [Route-model source](https://github.com/spincyc/adk/tree/main/research/usb_matrix)
- [Linux USB/IP protocol](https://docs.kernel.org/usb/usbip_protocol.html)
- [USB 3.2 specifications and compliance material](https://www.usb.org/usb-32)
