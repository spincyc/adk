# Host-only USB mesh model

This isolated research model exercises the first dynamic mesh ownership rules.
It does not transport USB traffic or configure a kernel.

The model has one durable controller authority. Source and destination nodes
carry enrolled incarnations and inventory revisions. A source may expose many
devices; a destination may expose many stable import slots. Each device lease
and each slot is exclusive. Moves are deterministic, fenced, and
break-before-make.

The executable model uses logical source devices and destination slots because
it tests controller fencing rather than USB signaling. Linux USB/IP is a useful
transport prototype only. The product target remains the transparent physical
CAU/PAU topology in `docs/research/USB_TRANSPARENT_PRODUCT.md`: unmodified
Windows and Linux computers, no installed host driver, and an exact rooted
physical topology reconstructed across the shared switched network.

Persistence stores only per-device fencing epochs. Restart advances every
loaded epoch and marks routes faulted; it never reconstructs an attachment from
remembered state. Endpoint inventory must be enrolled and observed again.
Controller high availability remains deferred.

Run the host-only checks with:

```sh
make usb-mesh-check
```

The production design remains in `docs/research/USB3_MESH_*.md`. This model is
not a supported ADK library interface.
