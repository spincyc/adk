# Deterministic HDMI mesh control model

This isolated host-only model explores one Linux controller coordinating
receiver and transmitter appliances. It does not transport media, implement an
HDMI electrical interface, or process HDCP.

The controller enrolls endpoint nodes with explicit incarnations and revisions.
It observes stable source and sink identities, reserves each sink exclusively,
and advances a source fencing epoch for every topology change. The first
milestone is one-to-one. The model can also exercise the explicitly later
fan-out milestone without claiming a media implementation.

A route progresses only through:

```text
unassigned
    -> read EDID
    -> assert HPD
    -> train link
    -> active
    -> blank
    -> unassigned
```

An invalid confirmation is rejected. A timed-out stage becomes `fault` and
continues to reserve its sink until an operator clears it. Clearing a fault or
confirming a blank releases the reservation. The caller supplies time, so a
recorded command and confirmation trace replays deterministically.

Build and run:

```sh
make hdmi-mesh-check
```

Inspect the deterministic synthetic fixture without hardware or network access:

```sh
make hdmi-mesh-routes
make hdmi-mesh-route \
    HDMI_SOURCE=input:camera-a HDMI_DESTINATION=output:wall-center
make hdmi-mesh-trace HDMI_ROUTE=route:camera-to-wall
make hdmi-mesh-crc HDMI_ROUTE=route:camera-to-wall
make hdmi-mesh-latency HDMI_ROUTE=route:camera-to-wall
```

Every report begins with `fixture synthetic`. CRC and latency are fixed model
evidence that exercises the inspection contract; they are not measurements.
The commands are read-only and perform no discovery, routing, media, network,
or hardware operation.

The fixed capacities are 16 nodes, 32 sources, 64 sinks, and 64 simultaneous
routes. No operation allocates memory, throws, touches hardware, or performs
network I/O.

## Boundary

This model represents controller authority and desired route state only.
Endpoint agents would perform EDID acquisition, HPD signaling, link training,
blanking, and media-session setup, then return fenced confirmations. A future
adapter must authenticate those agents and reject confirmations from older
controller terms or endpoint incarnations. Controller high availability is
deferred.

An Arduino Mega 2560 is suitable for a physical operator panel and observable
status indicators. It is not an HDMI receiver, transmitter, packetizer, or 8K
media transport.
