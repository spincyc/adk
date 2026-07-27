# Target API

> **Design target:** the interfaces and contracts on this page are planned.
> They are not implemented library APIs. Use [Current API](api-current.md) for
> code that compiles today.

The target hierarchy replaces global auto-registration with explicit resource
ownership and RAII cleanup while remaining suitable for small AVR firmware.

## Layers

| Layer | Responsibility | Planned examples |
|---|---|---|
| Hardware endpoint | Own one pin, bus, timer, interrupt, or channel claim | `DigitalInput`, `DigitalOutput`, `PwmOutput` |
| Circuit component | Compose endpoints and express physical semantics | `Button`, `MonoLed`, `RgbLed` |
| Behavior or circuit | Coordinate components deterministically | debounce, blink, Simon engine |

Composition is the default. Inheritance is reserved for genuinely substitutable
capabilities where runtime polymorphism earns its code and memory cost.

## Lifecycle contract

The intended common lifecycle vocabulary is:

```cpp
Result initialize  ();
void   shutdown    () noexcept;
bool   initialized () const;
```

The exact declarations and `Result` representation will be introduced with
tests, not assumed by this design page. The contract is:

- construction creates a valid, inert object;
- initialization claims and configures all required resources transactionally;
- failure releases everything claimed by that attempt;
- shutdown is safe to call repeatedly;
- destruction performs shutdown;
- cleanup cannot throw;
- the library does not throw internally, but remains safe when an enclosing
  exception-enabled program unwinds its stack;
- copying is unavailable for exclusive owners;
- moving is not promised until safe ownership transfer is designed and tested.

Generic output endpoints return to high impedance at shutdown. Semantic
components define an explicit inactive state. Safety-sensitive components may
require an explicit shutdown policy rather than accepting a default.

## First endpoint and component

`DigitalInput` is the first planned endpoint. It will own its pin claim and
expose the electrical reading needed for diagnostics.

`Button` will own a `DigitalInput`. Its default circuit uses the Mega 2560
internal pull-up and a switch to ground. The electrical level is therefore
active-low, while the semantic state reports active when pressed.

The planned button behavior includes:

- raw state for wiring, noise, and bounce diagnosis;
- debounced active state;
- pressed and released events;
- one explicit time value supplied to each update;
- non-consuming event snapshots that remain stable for an update cycle;
- a required release before another press is accepted;
- simultaneous presses handled explicitly by the composing behavior rather
  than by scan-order priority.

Names and signatures remain provisional until the endpoint and button commits
land with host tests, a Mega sketch, and their lesson.

## Resource rules

- Exclusive resources cannot be silently shared.
- A multi-resource component acquires everything or nothing.
- Shared buses have one explicit owner; attached devices borrow a defined
  access relationship.
- Pin identity, board capability, and ownership are separate concepts.
- PWM is not described as analog voltage.
- No component silently changes another component's pin configuration.
- Board capability data begins with the Mega 2560 reference profile.

## Promotion into the current API

A planned interface moves into the current reference only after:

1. its public header and out-of-line implementation exist;
2. host tests cover success, failure, lifetime, and deterministic behavior;
3. its example compiles for the Mega 2560;
4. its physical acceptance procedure and rich lesson are published;
5. its resource, initialization, and shutdown behavior are documented.

Until then, examples on this page are explanatory sketches rather than promises
of a particular spelling or ABI.
