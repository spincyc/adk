# Contributing

ADK grows in hierarchy order: establish a small endpoint, prove its ownership
and lifecycle, teach it, and only then compose it into a larger component or
project. A change is complete only when its code, tests, hardware exercise, and
documentation agree.

## Component completeness gate

A new component should pass these gates in order:

1. Document the physical behavior, electrical limits, and safe inactive state.
2. Identify every pin, bus, timer, interrupt, and other resource it claims.
3. Place it in the endpoint, circuit-component, or behavior layer.
4. Define a small deterministic interface and its ownership invariants.
5. Implement transactional `initialize()` and idempotent
   `shutdown() noexcept`; construction leaves a valid inert object.
6. Keep headers declarative and move implementation out of line.
7. Add host fakes and ample deterministic tests, including failures, boundary
   cases, cleanup, and examples of correct use.
8. Add and compile a Mega 2560 sketch.
9. Add a rich lesson with prerequisites, safety checks, pencil orientation
   art, an exact schematic, prediction, measurement, diagnosis, assessment,
   and acceptance evidence.
10. Run `make check`, `make format-check`, `make arduino`, and
    `make lessons-check`.
11. Commit the component before anything that depends on it.

Do not land a composite interface before its underlying endpoint and semantic
component contracts are complete. Prefer composition. Use inheritance only for
a genuine substitutable capability whose runtime cost is justified.

## Code conventions

Use four-space indentation, normalized CamelCase acronyms such as `RgbLed`, and
`struct` rather than `class`. Braces occupy their own lines except for compact
namespace declarations. Braces are mandatory for conditionals and loops.
Align related declarations and opening parentheses where it improves scanning.

ADK does not use exceptions, RTTI-dependent designs, or heap allocation
internally. Destructors remain `noexcept`. Do not hide time, randomness, event
consumption, or resource ownership in global state.

The complete mechanical rules live in
[the project style guide](docs/STYLE.md). Architecture and lifecycle
contracts live in [the architecture guide](docs/ARCHITECTURE.md).

## Safety and scope

Lessons must use appropriately rated parts and explicit stop conditions.
High-current loads, motors, relays, servos, and radios require documented power,
isolation, and legal-use boundaries.

ADK may teach receive-only radio observation in a lawful lab and may implement
an inert show-cue simulator, continuity simulation, logging, fault injection,
and emergency-stop behavior. It must not clone or transmit a pyrotechnic launch
protocol, implement an ignition circuit, or bypass a certified controller's
safety interlocks.

Contributions are provided under the repository's
[MIT License](license.md).
