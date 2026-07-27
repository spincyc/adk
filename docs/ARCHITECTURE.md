# Architecture

## Method

Every component is developed in the same order:

1. establish the physical behavior and safety boundary;
2. name the resource and ownership invariant;
3. design one small interface;
4. exercise it through a host fake;
5. verify it on the Mega 2560;
6. publish a lesson with prediction, evidence, diagnosis, and acceptance proof;
7. compose it into the next circuit without weakening earlier contracts.

## Layers

ADK separates three kinds of object.

The original preview is frozen under `legacy/`. First-class code uses the
vocabulary below without forwarding headers or dual-API ambiguity.

| Layer | Purpose | Target examples |
|---|---|---|
| Hardware endpoint | Own a pin, bus, timer, or interrupt claim | `DigitalInput`, `PwmOutput` |
| Circuit component | Give physical hardware semantic behavior | `Button`, `MonoLed`, `RgbLed` |
| Behavior/circuit | Coordinate components without owning their physics | `Blink`, mode selector, cue simulator |

A component *has* endpoints; it is not a specialized pin. Composition is the
default. Inheritance is reserved for a narrow, substitutable capability where
runtime polymorphism is worth its flash and RAM cost.

## Lifecycle

First-class components use this lifecycle:

```cpp
Status initialize  ();
void   shutdown    () noexcept;
bool   initialized () const;
```

Construction creates a valid inert object. `initialize()` claims and configures
all resources transactionally. Failure rolls back partial work. `shutdown()` is
idempotent and the destructor calls it. Generic outputs return to high
impedance; semantic devices explicitly choose a safe inactive state.

The library never throws. It remains safe when used by a program that does:
stack unwinding invokes `noexcept` destructors and releases active claims.

## Resource rules

- Outputs, PWM channels, chip selects, and interrupt lines are exclusive.
- Shared buses require one explicit bus owner; devices borrow a bus lease.
- A multi-resource component claims everything or nothing.
- Pin identity, electrical capability, and ownership are distinct concepts.
- PWM is not called analog voltage.
- Board capabilities come from a board profile, beginning with Mega 2560.
- No component silently changes another component's pin mode.

## First input component

`Button` owns a `DigitalInput`. It defaults to the internal pull-up
and a switch to ground, so the electrical signal is active-low while `active()`
is true when pressed. It exposes both:

- diagnostic raw state, which may bounce; and
- debounced state plus pressed/released events.

`update(TimePoint now)` advances it deterministically. Events are non-consuming
snapshots that remain stable for one update cycle. Each accepted press must be
released before another is accepted; simultaneous presses are invalid input.
