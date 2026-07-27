# HDMI mesh security

> **Controller decision:** the initial mesh has one durable authoritative
> controller running as a service on an ordinary Linux computer and never
> carrying media. High availability, replication, consensus, and automatic
> controller failover are deferred. Controller loss fails closed.

Status: threat model for a research architecture  
Scope: dynamically route authorized HDMI source endpoints to authorized HDMI
destination endpoints across a switched network

## Security conclusion

An HDMI mesh terminates a source link, transports interpreted media and
sideband state, and reconstructs a new sink link. It is therefore a collection
of HDMI receiver and transmitter endpoints, not a transparent cable. Dynamic
routing makes the controller, endpoint firmware, network transport, EDID,
metadata, audio, CEC, and any content-protection subsystem part of the security
boundary.

The research path supports generated or explicitly authorized unprotected
content in an isolated lab. Handling protected content is a separate licensed
product boundary. A prototype without mutually authenticated endpoints,
exclusive fenced routes, protected control traffic, and independent audit is
not a deployable matrix.

## Terms and invariants

A **source endpoint** terminates one or more HDMI source links and produces
bounded media, audio, metadata, and observed sideband records. A **destination
endpoint** reconstructs HDMI links for sinks. A node may implement both roles.
The **durable controller** authorizes desired routes. The **media plane**
carries only streams authorized for its source, destination set, format, and
ownership epoch.

The mesh preserves these invariants:

- every physical HDMI port has one enrolled identity and one role per ownership
  epoch;
- only the durable controller creates, changes, or revokes routes;
- a newer ownership epoch permanently fences older authority for that port;
- endpoints accept control and media only from authenticated peers named by
  the current route;
- uncertain authority, expired authorization, or conflicting state disables
  output or selects an explicit local test pattern;
- EDID, HPD, DDC, CEC, metadata, and content-protection state never confer
  route authority;
- phase one is one-to-one; phase-three fan-out is explicit policy, never an
  accidental multicast subscription;
- a route reports active only after source, network, destination, timing,
  format, and output observations agree;
- the Mega 2560 panel proposes changes and displays evidence; it holds no mesh
  authority, media, or content-protection key.

## Threat model

Protect against:

- a hostile or compromised source sending malformed timing, audio, metadata,
  EDID-facing behavior, or link-training sequences;
- a hostile sink using crafted EDID, DDC, CEC, or hot-plug behavior to attack
  an endpoint or influence other routes;
- an endpoint forging inventory, capabilities, route state, media health, or
  content-protection state;
- an operator viewing, recording, routing, or duplicating content without
  authorization;
- a network attacker reading, modifying, replaying, delaying, redirecting, or
  subscribing to control or media traffic;
- a stale controller command or endpoint retaining a route after reassignment;
- metadata injection, downgrade, format confusion, route churn, resource
  exhaustion, and audit deletion;
- theft, extraction, cloning, or misuse of endpoint credentials or licensed
  content-protection keys.

Initial scope does not promise continued service after controller compromise,
Byzantine consensus, protection from physical implants, or safe parsing of
arbitrary proprietary extensions. These require a separate assurance program.

## Identity, authentication, and enrollment

Give each controller, endpoint appliance, physical port, firmware image, and
administrative operator a stable identity. Do not authenticate by IP address,
MAC address, switch port, EDID, source name, sink name, or HDMI serial fields.

- Enroll a port through a documented physical-presence procedure.
- Store deployable node keys in a hardware-backed keystore.
- Mutually authenticate and encrypt control-plane sessions.
- Authenticate, encrypt, and replay-protect media-plane sessions unless the
  physical deployment supplies an independently reviewed equivalent boundary.
- Bind every session to node and port identities, roles, protocol version,
  policy revision, route identifier, and ownership epoch.
- Verify signed firmware and prevent rollback below the allowed security
  version.
- Treat recovery, replacement, reprovisioning, and factory reset as new
  enrollment events.
- Support credential expiry, rotation, and prompt revocation without changing
  unrelated routes.

Manual short-lived certificates may be acceptable in the isolated lab.
Shared credentials, trust on first use, and plaintext media are lab-only and
must be stated in every resulting test record.

## Authorization and content policy

Authorization is default-deny and evaluates the complete request:

```text
operator, source port, destination ports, content label, capture permission,
fan-out permission, format limits, audio policy, metadata policy,
CEC policy, protection state, duration, bandwidth class, policy revision
```

Separate permissions to discover, preview, propose, approve, activate,
duplicate, capture, record, move, revoke, reset, and alter source or sink
capabilities. A signed route grant names all endpoint and port identities,
allowed streams, format bounds, ownership epochs, expiry, and approved fan-out.
It cannot authorize a different source, destination, or media mode.

Content labels travel as controller-signed policy records, not as trusted HDMI
metadata. Generated test patterns, public test media, private unprotected
media, and licensed protected media remain distinct policy classes. A capture
or monitoring output requires its own destination authorization. Thumbnails,
frame hashes, audio meters, and diagnostic snapshots are content access, not
harmless telemetry.

## HDMI sideband and metadata isolation

Treat every HDMI-derived field as hostile input. Parse it in a bounded,
least-privileged process or hardware block and convert it to a strict internal
schema before control-plane use.

- Bound EDID length, extension count, nesting, retry rate, and supported data
  blocks; reject malformed checksums and impossible combinations.
- Generate the source-facing EDID from controller policy and the chosen
  destination capability set. Never forward arbitrary sink bytes directly.
- Rate-limit and debounce HPD. A sink cannot repeatedly retrain unrelated
  sources or routes.
- Isolate each DDC transaction by physical port and source epoch.
- Disable CEC by default. If enabled, filter explicit message types and logical
  addresses per route; never bridge CEC as a mesh-wide broadcast bus.
- Validate AVI, audio, HDR, vendor-specific, variable-refresh, and compression
  metadata against the negotiated media format.
- Reject unknown or unsupported metadata by policy rather than copying it
  blindly.
- Keep sink identities and capability records private to authorized operators.

Fuzz EDID, DDC, CEC, infoframe, link-training, and internal-schema parsers.
Compatibility with ordinary displays is not security evidence.

## Control-plane and media-plane isolation

Use separate logical security domains and traffic classes for management,
media, timing, observability, USB, and ordinary LAN traffic on the one shared
managed network. A media packet never carries controller authority, and a media
receiver cannot create a route by joining multicast state.

- Give endpoint services only the port, queue, register, and credential access
  required for their role.
- Admit media flows only after controller authorization and endpoint agreement.
- Enforce source-specific multicast, destination allowlists, bandwidth
  reservations, and bounded queue use at endpoints and the network fabric.
- Authenticate stream descriptions and bind payload format, timing domain, and
  encryption keys to the current route.
- Use independent credentials and key rotation for control and media.
- Keep PTP or equivalent timing input outside route authority; loss or spoofing
  of time produces a declared timing fault, not a route change.
- Rate-limit route churn, capability probes, HPD actions, preview generation,
  and diagnostic captures.
- Prevent one endpoint's malformed or excess stream from exhausting unrelated
  routes.

## Route changes, fencing, and failure

The controller commits desired state before endpoints activate it. Each source
port has a monotonically increasing, non-wrapping ownership epoch in the
controller's durable store. Fan-out destinations share that source epoch and
hold separately named, expiring sink reservations. Epoch exhaustion is fatal;
it never wraps.

1. Authorize the complete route and reserve media, timing, and endpoint
   capacity.
2. Issue signed prepare records to the named source and destinations.
3. Disable or retain the old output according to an explicit transition
   policy; never show frames from an unauthorized new source.
4. Fence revoked destinations and receive durable acknowledgements.
5. Persist the new source epoch and sink reservations before activation.
6. Install matching media keys, stream descriptions, and policy records.
7. Activate only after authenticated peer, epoch, timing, and format agreement.
8. Reconstruct fresh HDMI links and observe enumeration and media lock.
9. Record desired state separately from every observed plane.

Every step has a deadline, idempotency key, reason code, and deterministic
recovery rule. Failure leaves affected outputs disabled with HDMI no-signal.
An unmistakable local test pattern requires a separately authorized
maintenance mode. Automatic restoration is a new authorized epoch, never reuse
of stale authority.

Only the durable controller issues route grants. Endpoints do not elect a
leader, compare wall clocks, use last-writer-wins, or invent leases during a
partition. On controller loss, mutation stops immediately and only an exact,
unchanged installed route may continue for its pre-authorized bounded lease;
it cannot change, expand fan-out, renew
themselves, or change content policy. Expiry or ambiguity fails closed.

## HDCP and protected content

The research mesh does not handle HDCP-protected content. Do not strip,
bypass, emulate, downgrade, record, or expose decrypted protected media.

A future protected-content product requires:

- current HDMI Adopter and HDCP licensing, approved repeater behavior, and
  compliance testing for every supported mode;
- licensed receiver, repeater, and transmitter implementations;
- hardware-backed storage and use of per-device keys with no software export;
- secure boot, signed updates, rollback protection, debug-port controls,
  tamper response, revocation-list handling, and controlled manufacturing;
- separation between decrypted media buffers and general-purpose software,
  diagnostics, previews, capture paths, and the Mega control plane;
- zeroization of transient secrets and protected buffers on reset, fault,
  reassignment, revocation, and decommissioning;
- vendor and legal review of topology, fan-out, recording, remote transport,
  field service, logging, and incident response.

Possession of HDMI receiver/transmitter IP is not proof of HDCP authorization.
Protected-content support cannot be promoted by a Make target, successful link,
or ordinary functional test.

## Privacy and capture

HDMI may contain private screens, camera feeds, conversations, credentials,
notifications, location data, and regulated content. Minimize collection.

- Keep payload capture off by default and require a distinct, short-lived
  authorization plus a visible local indication.
- Prefer counters, synthetic patterns, and locally computed health summaries
  over frames or audio.
- Redact source titles, sink names, EDID serial fields, thumbnails, and media
  fingerprints from ordinary logs.
- Encrypt approved captures, limit access, record exports, enforce retention,
  and verify deletion.
- Disable audio monitoring independently of video.
- Give operators clear evidence of every preview, recording, multicast
  destination, and diagnostic snapshot.

Local law, contracts, workplace policy, copyright, and consent determine
whether capture is permitted. Technical reachability is never permission.

## Audit and incident evidence

Record desired and observed state separately. Each audit event includes:

- authenticated actor, approver, controller identity, policy revision, and
  idempotency identifier;
- source and destination endpoint and port identities;
- content label, permitted operations, fan-out set, ownership epochs, and
  route expiry;
- capability digest, selected format, audio/metadata/CEC policy, protection
  state, and media encryption identifier;
- prepare, fence, activate, lock, unlock, preview, capture, record, revoke,
  reset, expiry, and fault events;
- endpoint acknowledgements, deadlines, observed output state, and
  reconciliation result.

Send append-only records to an independent authenticated sink. Local hash
chains detect some edits but cannot prevent truncation by a compromised node.
Do not log payloads, decrypted content, media keys, HDCP secrets, full EDID
records, or unnecessary personal fields. Protect the audit service from media
load and define access, export, retention, and deletion policy.

## Lab boundary and deployment gate

| Property | Phase-one lab | Deployable mesh |
|---|---|---|
| Content | Generated or approved unprotected media | Explicit content policy and legal review |
| Network | Shared managed household LAN with lab-only traffic profiles | Segmented, monitored, capacity-managed shared LAN |
| Authority | One durable controller | Hardened single controller; HA still deferred |
| Identity | Manually provisioned short-lived credentials | Hardware-backed lifecycle and revocation |
| Endpoints | Known evaluation hardware | Hardened boot, updates, isolation, and parser review |
| Media | Authenticated experiment or stated isolation exception | Encrypted, authenticated, replay-protected |
| Sideband | CEC off; bounded EDID profile | Policy-filtered, fuzzed, compatibility-qualified |
| HDCP | Unsupported | Separately licensed and independently reviewed |
| Capture | Disabled except explicit test | Authorized, indicated, audited, and retained by policy |
| Audit | Local experimental evidence | Independent append-only evidence |
| Failure | Manual inspection and reconciliation | Bounded, deterministic, fail-closed recovery |

Deployment requires adversarial testing, operational procedures, compliance
work, independent security review, and physical validation for the intended
hardware and environment. Host simulation and FPGA loopback establish neither
signal integrity nor deployment security.

## Required deterministic evidence

Before the security architecture advances, tests must cover:

- every authorized source/destination combination and rejection of forbidden
  routes, fan-out, previews, captures, and format changes;
- simultaneous moves, stale grants, duplicate replay, expiry, and epoch
  exhaustion;
- controller restart before and after every transaction boundary;
- endpoint crash, rollback, clone, credential expiry, and revocation;
- delayed, reordered, duplicated, dropped, modified, and forged messages;
- malformed EDID, DDC, CEC, infoframes, stream descriptions, and format
  transitions;
- PTP loss, timing spoof, media-key mismatch, packet loss, buffer failure, and
  bandwidth exhaustion;
- capture-policy violation, audit-sink failure, full storage, and clock change;
- deterministic reconciliation to exactly the authorized route set or disabled
  outputs.

Physical HDMI link tests, HDCP compliance, electrical validation, codec
quality, latency, and switch behavior remain separate acceptance gates. Never
report simulated evidence as physical or compliance verification.
