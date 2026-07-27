# Components

ADK grows from hardware endpoints into circuit components and then into
behaviors and projects. The status column is the authority on whether a name is
usable today:

- **Compatibility** means the interface is implemented and used by the imported
  lessons, but is expected to be replaced by the ownership-oriented API.
- **Planned** means the interface or project is a design target. It must not be
  used as though it were part of the library.

| Component | Layer | Status | Host test | Mega build | Lesson |
|---|---|---:|---:|---:|---:|
| `pin::Base`, `pin::Input`, `pin::Output` | endpoint | Compatibility | Yes | Yes | Indirect |
| `digital::Input`, `digital::InputPullUp`, `digital::Output` | endpoint | Compatibility | Partial | Yes | Indirect |
| `analog::Input`, `analog::Output` | endpoint | Compatibility | Partial | Yes | Indirect |
| `led::Mono` | component | Compatibility | Yes | Yes | 001, 002 |
| `color::Rgb`, `led::Rgb` | value/component | Compatibility | Yes | Yes | 003 |
| `DigitalInput` | endpoint | Planned | No | No | Planned |
| `Button` | component | Planned | No | No | Planned |
| `DigitalOutput`, `MonoLed` | endpoint/component | Planned | No | No | Planned |
| Clock, debounce, and blink behavior | behavior | Planned | No | No | Planned |
| `PwmOutput`, `RgbLed` | endpoint/component | Planned | No | No | Planned |
| Analog controls and sensors | component | Planned | No | No | Planned |
| Sound, servo, displays, buses, and operator controls | component | Planned | No | No | Planned |
| Deterministic Simon | project | Planned | No | No | Planned |

“Mega build” records compilation for the Mega 2560, not a claim that every
combination has passed a physical hardware acceptance test. “Partial” host
coverage means the fake Arduino layer exercises some operations but does not
yet give the endpoint a complete, independent contract test suite.

## Composition rule

A hardware endpoint owns a pin, bus, timer, interrupt, or similar resource. A
circuit component owns or composes its endpoints and gives them physical
meaning. A behavior coordinates components without taking over their electrical
responsibilities.

For example, the planned `Button` owns a `DigitalInput`; it is not a specialized
input pin. This keeps raw electrical diagnostics, debounce, and user-facing
events separate while making ownership unambiguous.

Each new component is complete only when it has:

1. a documented physical and shutdown contract;
2. a small interface and deterministic host tests;
3. a Mega 2560 sketch and hardware acceptance procedure;
4. a pencil orientation drawing and exact schematic;
5. a lesson PDF linking prediction, measurement, diagnosis, and evidence.

See [Current API](api-current.md) for usable symbols and
[Target API](target-api.md) for the intended hierarchy.
