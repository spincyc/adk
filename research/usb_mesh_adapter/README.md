# USB mesh action adapter

Status: isolated research prototype; not a supported ADK interface.

This adapter is the narrow boundary between the authoritative mesh model and
platform-specific USB operations. It contains no command strings, shell
execution, privilege escalation, USB/IP assumptions, network discovery, or
controller election.

The controller supplies stable device, source, source-slot, destination, and
destination-slot identities. Hostnames, IP addresses, bus numbers, and kernel
enumeration order remain adapter metadata and cannot authorize an action.

## Contract

`initialize()` loads and validates durable state before planning is allowed.
`plan()` durably records an immutable, monotonically increasing action ID and
fence before any external operation. `apply()` accepts only that action ID.
Repeating an identical plan or the most recently completed apply is
idempotent; reusing an old ID fails.

State is keyed by stable destination and destination-slot identity. Independent
slots may have interleaved plans and applies. Every slot transition begins with
a detach action for the route currently occupying that exact slot. A
cross-slot or cross-route detach fails. An attach at the same fence is allowed
only after detach has been positively verified for that destination slot. A
later transition requires a higher controller term or destination epoch.
Counters never wrap.

The injected `UsbActionBridge` performs one already-authorized operation and
answers a separate observation query. Returning from `detach()` is not proof
that the device disappeared. A production bridge must derive `detached()` and
`attached()` from bounded, platform-specific observations.

Persistence failure is fail-closed:

- a plan-save failure causes no external operation;
- a detach-result save failure leaves the durable plan retryable and the USB
  side detached;
- an attach-result save failure detaches the new route and verifies that
  rollback before reporting failure;
- failed rollback durably enters a fault state and requires operator recovery;
  it never claims an active route.

Restart recovery validates every durable slot. A planned detach is completed
by another verified detach. A planned attach is first forced detached and then
left retryable. An active route must be positively observed or its slot enters
the durable fault state. Malformed state, an unavailable store, or an existing
fault prevents initialization.

Phase one assumes one durable authoritative controller. Endpoints do not elect
or promote a controller, and this adapter contains no high-availability logic.

Linux USB/IP endpoints may translate an authorized action into direct,
argument-vector process execution during transport prototyping. They are not
the product boundary. The final bridge targets the physical CAU/PAU contract
in `docs/research/USB_TRANSPARENT_PRODUCT.md`: unmodified Windows and Linux
computers, exact rooted topology reconstruction, and no installed host driver.
This prototype intentionally supplies only the injected contract and fake.

## Host check

```sh
make usb-mesh-check
```
