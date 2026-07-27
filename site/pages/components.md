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
| Endpoint | `AnalogInput` | Hardware experimental | One analog-capable pin |
| Value behavior | `LinearCalibration`, `MovingAverage`, `Deadband` | Host verified | Fixed-size sample state |
| Behavior | `NightLight` | Host verified | No hardware; observes normalized samples |
| Component | `ShiftRegisterOutput` | Hardware experimental | Three digital outputs |
| Component | `SevenSegmentDisplay` | Hardware experimental | One shift-register output |
| Behavior | `TrafficJunction` | Host verified | No hardware; observes request, health, and time |
| Contract | `ClimateSensor`, `ClimateSample` | Host verified | No hardware; validated timestamped climate observations |
| Component | `Dht11Sensor` | Hardware experimental | One bidirectional data pin |
| Component | `CharacterDisplay` | Hardware experimental | Six digital output endpoints |
| Behavior | `EnvironmentalStation` | Host verified | No hardware; observes climate, controls, and time |
| Component | `Keypad`, `MatrixKeypad` | Hardware experimental | Four row outputs and three column inputs |
| Behavior | `BoundedServo` | Host verified | No hardware; maps bounded calibrated intent |
| Value codec | `ServoConfigurationRecord` | Host verified | Fixed versioned record; no storage transport |
| Endpoint | `ServoOutput` | Hardware experimental | D44/OC5C, Timer5, and a power-admission gate |
| Behavior | `AccessTrainer` | Host verified | No hardware; observes key, health, and time snapshots |
| Endpoint | `PulseInput` | Hardware experimental | One pulse-observation pin |
| Component | `UltrasonicRanger` | Hardware experimental | Trigger output plus pulse input |
| Behavior | `MotorIntent` | Host verified | No hardware; observes requests, faults, and time |
| Behavior | `RoverController` | Host verified | No hardware; observes range, route, and time |
| Later layers | Buses, sensors, actuators | Planned | See catalog |

Composition is preferred: a Button has an input; it is not a specialized pin.
Behavior engines expose output intent rather than hiding hardware callbacks.
`Simon` therefore owns neither buttons nor cue devices: an adapter translates
one complete button observation into `SimonInput`, then maps `SimonSnapshot`
onto LEDs, RGB feedback, and sound.

`NightLight` follows the same boundary. Its Mega adapter owns the sensor, lamp,
and diagnostic LED, while the engine only accepts a complete observation and
returns output intent.

The same composition continues through lessons 010–021. A
`SevenSegmentDisplay` owns its serialized output; `TrafficJunction` instead
owns no pins and returns a complete legal signal pattern for its Mega adapter.
`Dht11Sensor` owns one bidirectional data pin while presenting validated,
timestamped climate samples through the transport-neutral `ClimateSensor`
contract. `BoundedServo` produces calibrated pulse intent while `ServoOutput`
owns the Timer5 endpoint; the external-power gate admits commands but is not
evidence that load power is present or disconnected.

`AccessTrainer` keeps the lesson 018 circuit inert: it returns visible
soft-latch and audit intent without driving a servo or storing credentials.
`UltrasonicRanger` separates pulse timing from distance validity.
`MotorIntent` and `RoverController` remain hardware-neutral policy engines;
the lesson 021 Mega stage presents requested and applied motion on LEDs, not
motors.

- [Exact API](api-supported.md)
- [Full component catalog](docs/COMPONENTS.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Deterministic testing](docs/TESTING.md)
