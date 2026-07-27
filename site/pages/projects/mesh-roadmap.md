---
title: "Transparent USB and HDMI mesh roadmap"
---

# Transparent USB and HDMI mesh roadmap

This is a research roadmap, not a supported ADK interface or a hardware
performance claim. The controller models and host tests exercise decisions and
failure paths; no transparent USB attachment unit, HDMI endpoint, PoE power
stage, or full-rate shared fabric has passed physical acceptance.

## Product destination

One ordinary Linux controller manages routes while USB, HDMI, management, and
household traffic share the existing switched Ethernet network. The controller
is not in either payload path. It is initially one durable authority;
replication and automatic failover are deferred.

```text
utility closet                                      remote room

Windows/Linux computer                              USB peripheral or real hub
    |                                                    |
    +-- USB -- Computer Attachment Unit -- Ethernet -- Peripheral Attachment Unit
    |
    +-- HDMI -- source endpoint ----------- Ethernet -- sink endpoint -- monitor
```

The USB computer requires no driver, service, installer, or mesh-specific
configuration. A `ComputerAttachmentUnit` (`Cau`) consumes one physical
computer USB port and presents exactly the topology rooted at one
`PeripheralAttachmentUnit` (`Pau`) port. It adds no synthetic hub. A
user-provided hub and all of its descendants form one atomic topology root.

The baseline Pau has four independently routed, protected USB 3 Type-A roots.
The baseline Cau has one fixed-role USB 3 Type-B computer port. Both target one
10GBASE-T PoE++ connection plus auxiliary development power. PoE operation,
power budgets, thermal behavior, and transparent device compatibility remain
measurement gates.

The HDMI mesh terminates the source link, transports interpreted video, audio,
timing, and metadata, then reconstructs a fresh sink-side HDMI link. It is not
raw electrical forwarding. EDID, hot plug, link training, clocks, buffering,
format compatibility, and licensed HDCP handling are endpoint responsibilities.

Read the canonical [transparent USB product contract](https://github.com/spincyc/adk/blob/main/docs/research/USB_TRANSPARENT_PRODUCT.md),
[USB endpoint hardware plan](https://github.com/spincyc/adk/blob/main/docs/research/USB_MESH_ENDPOINT_HARDWARE.md),
[USB PoE power plan](https://github.com/spincyc/adk/blob/main/docs/research/USB_MESH_POE_POWER.md),
[HDMI mesh architecture](https://github.com/spincyc/adk/blob/main/docs/research/HDMI_MESH_ARCHITECTURE.md),
and [HDMI endpoint hardware study](https://github.com/spincyc/adk/blob/main/docs/research/HDMI_MESH_ENDPOINT_HARDWARE.md).

## Routes, profiles, and failures

Every route is admitted against observed endpoint, power, bandwidth, latency,
jitter, and path capacity. Ordinary household traffic retains configured
headroom. A rejected replacement leaves a healthy active route unchanged.

Profiles make tradeoffs explicit:

- `PinnedProfile` applies exactly one named profile or fails.
- `AllowedProfiles` chooses only from an operator-approved ordered set.
- `BestAvailableWithinBounds` chooses within explicit operator limits.

Lower-bandwidth and higher-latency HDMI modes are valid named profiles. USB
profiles may select tested scheduling and buffering behavior, but cannot relax
timing required by the computer or peripheral. Both endpoint units show the
actual applied profile and route state locally; color alone is never the only
evidence.

An active pinned HDMI route that loses its contract blanks and mutes without
silently changing profile. A transparent USB contract failure appears as a
physical disconnect: the Pau removes supplied VBUS, the durable topology epoch
advances, and stale transfers are rejected. After a profile-specific stable
recovery interval, automatic recovery appears as a fresh plug and native
enumeration.

Production failure policies are separate from disabled-by-default deterministic
fault injection. Lab injection is bounded, authorized, audited, visibly marked
`TEST`, and cannot override a real electrical or network fault.

The shared policy model is executable host research; see
[shared route profiles](https://github.com/spincyc/adk/tree/main/research/route_profiles)
and the complete [shared-fabric contract](https://github.com/spincyc/adk/blob/main/docs/research/SHARED_USB_HDMI_FABRIC.md).

## Physical stages

| Stage | USB | HDMI and shared fabric | Status |
|---|---|---|---|
| 0 | Deterministic routing, fencing, profile, and failure models | Deterministic route and endpoint models | Host research exists; integration continues |
| 1 | Linux USB/IP on two Linux machines for discovery and measurement | One receiver/transmitter platform in a local loop at a controlled format | Planned; does not satisfy product transparency |
| 2 | First Cau/PAU hardware for controlled HID and bulk devices | Two endpoints across one managed switch, starting below 8K | Hardware selection and purchase gates open |
| 3 | Exact hub-tree reconstruction, protected four-port Pau, broader device corpus | Explicit native and compressed profiles, reconstruction, recovery, and fan-out | Research |
| 4 | Windows/Linux transparency across composite, storage, reset, suspend, and selected isochronous cases | Full-8K evaluation, shared-network admission, clocks, security, and licensed-content validation | Long-term research |
| 5 | Qualified PoE++, thermals, signal integrity, compatibility, and enclosure | Production endpoint, deployment, and interoperability study | Deferred |

Linux USB/IP is a useful prototype, not the product: it terminates at a Linux
virtual host controller and cannot prove physical transparency to an
unmodified Windows or Linux computer. Likewise, nominal USB, HDMI, or Ethernet
rates do not prove payload throughput, timing, or compatibility.

## Current boundary

- Executable controller and profile models have deterministic host tests.
- CLI, reconciliation, identity, security, observability, and hardware studies
  are research artifacts.
- Physical USB and HDMI payload tests are deferred.
- No endpoint hardware has been purchased, qualified, or bench accepted.
- Controller high availability, USB-C, USB Power Delivery, and a production
  endpoint platform remain deferred.
- The Mega 2560 may provide buttons and independent visible evidence, but it
  carries neither USB nor HDMI payloads.

Detailed work is indexed by the
[USB mesh architecture](https://github.com/spincyc/adk/blob/main/docs/research/USB3_MESH_ARCHITECTURE.md),
[USB data-plane study](https://github.com/spincyc/adk/blob/main/docs/research/USB3_MESH_DATA_PLANE.md),
[HDMI data-plane study](https://github.com/spincyc/adk/blob/main/docs/research/HDMI_MESH_DATA_PLANE.md),
and the
[USB](https://github.com/spincyc/adk/blob/main/docs/research/USB3_MESH_TEST_PLAN.md)
and
[HDMI](https://github.com/spincyc/adk/blob/main/docs/research/HDMI_MESH_TEST_PLAN.md)
test plans.
