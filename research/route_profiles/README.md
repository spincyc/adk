# Shared route profiles

This isolated research model gives USB and HDMI routes the same deterministic
policy vocabulary without carrying either payload.

## Contract

- `Pinned` either applies the named profile or fails. It never degrades.
- `OrderedFallback` tries only the declared profiles, in order.
- `BestWithinBounds` chooses the highest quality admissible profile. Equal
  quality resolves by the smaller stable profile ID.
- A selection result records requested and applied identities and whether a
  fallback occurred. Endpoint UI must show the applied profile, not merely the
  request.
- Failed admission can preserve an existing route. Active pinned HDMI contract
  loss can blank and mute while retaining the pin. Transparent USB contract
  loss can disconnect, remove peripheral VBUS, and advance the route epoch.
- Automatic USB recovery requires a continuously healthy, profile-specific
  interval. Recovery is a fresh attachment; no prior USB session is reused.

`ProfileBounds` are operator limits. `PathObservation` is measured fabric
state. A profile must satisfy both. No policy silently modifies a profile.

## Lab fault injection

`FaultScenario` is disabled unless `labModeEnabled` is explicit. Its fixed,
ordered events deterministically alter an observation for a bounded duration.
`InjectedObservation::testActive` is the mandatory visible `TEST` evidence.
A real fault always dominates the injected view.

The injector is test infrastructure, not production failure policy. It cannot
clear an electrical fault, bypass route fencing, or claim that a path is safe.
Endpoint and controller implementations must additionally authorize, audit,
scope, and provide immediate cancellation for a live lab scenario.

## Verify

```sh
make route-profile-check
```

The tests cover pinned refusal, ordered fallback, deterministic best-fit
selection, payload-specific failure decisions, continuous recovery including
32-bit timer rollover, bounded injection, real-fault precedence, duplicates,
and fixed-capacity exhaustion. Physical testing is deferred; this model has no
hardware or payload operations.
