# Components

Status meanings:

- **Host verified** — strict deterministic tests pass.
- **Hardware experimental** — Mega build passes; bench card remains open.
- **Planned** — contract or curriculum placement only.

| Layer | Type | Status | Owns |
|---|---|---|---|
| Core | `Runtime`, `ResourceClaim`, `TimePoint`, `Status` | Host verified | Fixed registry and explicit time |
| Endpoint | `DigitalOutput` | Hardware experimental | One output pin |
| Endpoint | `DigitalInput` | Hardware experimental | One input pin |
| Component | `MonoLed` | Hardware experimental | One `DigitalOutput` |
| Component | `Button` | Hardware experimental | One `DigitalInput` |
| Behavior | `ReactionTimer` | Hardware experimental | No hardware; observes Button and time |
| Endpoint | `PwmOutput` | Hardware experimental | One PWM pin; shared default timer |
| Component | `RgbLed` | Hardware experimental | Three `PwmOutput` endpoints |
| Component | `PiezoSounder` | Hardware experimental | One pin and Timer2 |
| Behavior | `Simon` | Host verified | No hardware; observes input snapshots and time |
| Later layers | Analog, buses, displays, sensors, actuators | Planned | See catalog |

Composition is preferred: a Button has an input; it is not a specialized pin.
Behavior engines expose output intent rather than hiding hardware callbacks.
`Simon` therefore owns neither buttons nor cue devices: an adapter translates
one complete button observation into `SimonInput`, then maps `SimonSnapshot`
onto LEDs, RGB feedback, and sound.

- [Exact API](api-supported.md)
- [Full component catalog](docs/COMPONENTS.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Deterministic testing](docs/TESTING.md)
