# USB 3 matrix lab CLI

> **Prototype boundary:** this CLI drives a Linux USB/IP lab adapter only.
> [The transparent product contract](USB_TRANSPARENT_PRODUCT.md) requires
> physical Cau/Pau units, unmodified Windows and Linux computers, exact
> topology reconstruction, atomic real-hub movement, `ColdMove`, and the shared
> switched network. No result from this CLI is product conformance.

Status: prototype operator contract  
Scope: Linux USB/IP on a controlled, wired lab segment of the shared network

The Make interface is the supported control surface. It terminates USB at two
Linux machines and routes USB requests over IP; it does not forward raw
SuperSpeed symbols. The Mega 2560 may later display route state, but it never
carries USB traffic or authorizes a route.

The two-node commands below are the prototype adapter, not the final topology.
The target is a dynamic endpoint mesh: any enrolled source endpoint can export
its eligible devices, any enrolled destination endpoint can offer virtual
host-controller ports, and an authorized route can be created, moved, or
released at runtime. A stable endpoint identity must not depend on a hostname,
address, bus ID, or temporary USB/IP port number.

## Safety model

Read-only discovery runs directly. Every mutation has a plan target and an
explicit apply target. The Python controller passes argument arrays directly
to subprocesses, never invokes a shell, and never inserts `sudo`. Arrange
privilege outside the tool, inspect the printed command, then run the named
apply target from an appropriately privileged session.

Do not expose TCP port 3240 to the Internet. Use an isolated management and
data VLAN, known test devices, and disposable test hosts. A remote USB device
has the authority of a local peripheral. Start with non-sensitive bulk test
media; do not route keyboards, cameras, security tokens, personal storage, or
production devices.

An assignment is an exclusive lease. A device cannot belong to two
destinations. A destination may eventually own multiple devices up to its
declared port, power, bandwidth, policy, and driver capacity; phase one
intentionally supports only a smaller subset. Reassignment means detach,
attach, and normal enumeration; it is not seamless migration. A failed
transaction remains unassigned and visible as a fault.

## Mesh command direction

The future CLI keeps the same preview-before-apply rule while operating on
stable identities:

```sh
make usb-mesh-endpoints
make usb-mesh-devices SOURCE=source-a
make usb-mesh-destinations
make usb-mesh-route-plan DEVICE=camera-a DESTINATION=workstation-b PORT=auto
make usb-mesh-route      DEVICE=camera-a DESTINATION=workstation-b PORT=auto
make usb-mesh-move-plan  DEVICE=camera-a DESTINATION=workstation-c PORT=auto
make usb-mesh-move       DEVICE=camera-a DESTINATION=workstation-c PORT=auto
make usb-mesh-release-plan DEVICE=camera-a
make usb-mesh-release      DEVICE=camera-a
make usb-mesh-watch
```

These targets are guidance, not implemented commands. The authoritative
controller must validate identity, compatibility, authorization, capacity,
and the current fenced epoch before emitting any endpoint action. Each
endpoint reports observed state; it does not decide global ownership.

## Stock Arch setup

The only USB/IP package is the official repository package `usbip`. Install it
deliberately on both endpoint machines:

```sh
sudo pacman -Syu --needed usbip
```

The package command is documentation, not something the matrix controller runs
implicitly. The endpoint kernel must provide `usbip-host` on the device node
and `vhci-hcd` on the importing host.

## Command narrative

Set names once per shell or pass them to each Make invocation:

```sh
export USB_DEVICE_NODE=usb-device-a.lab
export USB_HOST_NODE=usb-host-a.lab
export USB_BUS_ID=2-1
export USB_PORT=0
```

Then work in the same order as the route state machine:

```sh
# Setup and prove prerequisites.
make usb-matrix-setup
make usb-matrix-doctor
make usb-matrix-check

# Observe local devices, remote exports, and imported ports.
make usb-matrix-discover
make usb-remote USB_DEVICE_NODE="$USB_DEVICE_NODE"
make usb-matrix-status

# Preview every mutation before applying it.
make usb-export-plan USB_BUS_ID="$USB_BUS_ID"
make usb-export      USB_BUS_ID="$USB_BUS_ID"
make usb-assign-plan USB_DEVICE_NODE="$USB_DEVICE_NODE" \
    USB_BUS_ID="$USB_BUS_ID" USB_HOST_NODE="$USB_HOST_NODE"
make usb-assign      USB_DEVICE_NODE="$USB_DEVICE_NODE" \
    USB_BUS_ID="$USB_BUS_ID" USB_HOST_NODE="$USB_HOST_NODE"

# Observe enumeration and the exclusive lease generation.
make usb-matrix-status
make usb-matrix-log

# Detach before unbinding or moving the device.
make usb-release-plan USB_HOST_NODE="$USB_HOST_NODE" USB_PORT="$USB_PORT"
make usb-release      USB_HOST_NODE="$USB_HOST_NODE" USB_PORT="$USB_PORT"
```

`usb-matrix-dry-run` previews one assignment without changing modules,
bindings, attachments, or the lease ledger. `usb-export-plan`,
`usb-assign-plan`, and `usb-release-plan` are the focused dry-run forms.
`usb-matrix-log` currently prints the temporary ledger and explicitly reports
that the authoritative journal has not been connected.

The concise discovery aliases remain useful:

| Target | Observation |
|---|---|
| `usb-local` | devices physically visible on this endpoint |
| `usb-remote` | devices exported by `USB_DEVICE_NODE` |
| `usb-ports` | USB/IP ports imported on this host |
| `usb-matrix-status` | temporary adapter ledger and its non-authoritative label |
| `usb-matrix-doctor` | presence of Python and the stock `usbip` command |

The adapter state defaults to `build/usb-matrix/`. Override its paths only to
preserve a deliberate experiment record. It is not the authoritative fenced
controller, a durable database, or a distributed consensus service. Phase one
permits exactly one operator and one adapter process. Never run two controllers
against the same endpoints.

## What each observation proves

Predict that a newly assigned device will pass through detached, attaching,
enumerating, then active states. Observe three independent surfaces:

1. `usbip port` proves that the virtual host controller has an imported port;
2. the lease ledger proves which generation the controller authorized;
3. the endpoint panel or named VBUS test point proves physical presence or
   protected power independently of the logical route.

None proves the other two. A green panel indication requires endpoint and host
agreement on the same generation. USB enumeration does not prove payload
correctness, and a powered connector does not prove an active lease.

For a bulk-storage experiment, record a known generated payload hash before
export, after a routed read, and after an orderly detach. Record throughput and
tail latency separately. Do not infer USB 3 performance from the nominal link
speed or from one successful enumeration.

## Failure handling

Commands have a ten-second bound. A timeout, malformed ledger, duplicate
lease, invalid identifier, missing tool, failed module operation, or failed
USB/IP command returns nonzero. Identifiers are validated before subprocess
construction.

The temporary Python ledger does not yet provide durable monotonic generations,
an interprocess transaction lock, an authoritative audit journal, or rollback
after attachment succeeds but ledger persistence fails. The host-only C++
controller models fenced epochs, stale confirmations, fail-closed restart, and
deterministic audit events, but the operating-system adapter is not connected
to it yet. Until that bridge exists, apply targets are controlled experiments,
not a deployable matrix service. After any failure, inspect `usbip port` on the
host and the physical device state before another command.

Do not power-cycle storage as ordinary recovery. Detach it from the importing
host, confirm the port disappeared, then unbind it at the device endpoint.
Network loss, endpoint restart, or ambiguous ownership stops the experiment
until both endpoints and the ledger agree that the device is unassigned.

## Repeatable evidence

Preserve these artifacts for each run:

- exact repository revision and `usbip --version`;
- kernel version and module state on both endpoints;
- switch model, port speeds, VLAN, MTU, and endpoint addresses;
- physical port, bus ID, descriptor inventory, and lease generations;
- plan output, temporary ledger snapshots, import-port snapshots, and kernel
  logs;
- generated payload hashes, throughput, latency, detach, cable-loss, and
  daemon-restart observations.

CLI unit tests use a fake command runner. They prove command construction,
validation, basic exclusive-lease behavior, and fail-closed command errors
without loading a module or touching USB hardware. Controller tests separately
prove epoch and state-machine behavior. The durable bridge, real USB/IP
compatibility, and performance remain deferred evidence.
