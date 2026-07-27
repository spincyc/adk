# Current API

This page documents the API implemented in `src/` today. It is the imported
compatibility interface, not the target resource-owning API.

Include the complete library with:

```cpp
#include <adk.h>
```

## Global lifecycle

```cpp
namespace adk {
    void initialize ();
    void update     ();
}
```

`initialize()` visits every live, registered `Object` and invokes its protected
initialization hook. `update()` similarly invokes each update hook. Programs
using periodic objects call `update()` once per loop iteration.

Both functions return `void`. The current implementation has no initialization
error result, resource-claim system, initialized-state query, or global
shutdown operation. Repeated initialization is not rejected.

## Registered objects

```cpp
struct adk::Object
{
    static void initializeAll ();
    static void updateAll     ();

    Object          ();
    virtual ~Object () noexcept;

    Object            (const Object&) = delete;
    Object& operator= (const Object&) = delete;
    Object            (Object&&)      = delete;
    Object& operator= (Object&&)      = delete;
};
```

Construction adds an object to the compatibility registry. Destruction removes
it. Objects cannot be copied or moved. The destructor prevents a destroyed
object from remaining registered, but it does **not** restore pin modes or
output levels. Derived implementations provide a protected `initialize()` hook
and may override `update()`.

## Pins

`adk::pin::Id` is `uint8_t`; `adk::pin::Value` is `int32_t`.

| Type | Public operation | Current behavior |
|---|---|---|
| `pin::Base` | `Id pin() const` | Returns the configured pin number |
| `pin::Input` | inherited construction | Configures `INPUT` during global initialization |
| `pin::Output` | inherited construction | Configures `OUTPUT` during global initialization |
| `analog::Input` | `pin::Value read() const` | Calls Arduino `analogRead` |
| `analog::Output` | `void write(pin::Value) const` | Calls Arduino `analogWrite` |
| `digital::Input` | `bool read() const` | True when Arduino `digitalRead` is `HIGH` |
| `digital::InputPullUp` | inherited construction | Configures `INPUT_PULLUP` |
| `digital::Output` | `void write(bool) const` | Writes `HIGH` for true and `LOW` for false |

On the Mega 2560, Arduino `analogWrite` normally means PWM output rather than a
true analog voltage. The target API will name this distinction explicitly.
The compatibility API does not prevent reads or writes before initialization
and does not detect conflicting use of one pin.

## LEDs and colors

```cpp
adk::led::Mono status (LED_BUILTIN);

status.on  ();
status.off ();
```

`led::Mono` derives from `digital::Output`. `on()` writes high and `off()` writes
low; active-low hardware therefore needs external adaptation.

```cpp
adk::led::Rgb indicator (6, 5, 3);

indicator.on  (adk::color::orange ());
indicator.off ();
```

`led::Rgb` contains three `analog::Output` objects and provides read-only access
to them through `red()`, `green()`, and `blue()`. Its constructor order is red,
green, blue. `on(const color::Rgb&)` writes all three channel values and `off()`
writes zero to each.

`color::Rgb` stores three `uint8_t` channels, provides channel accessors and
equality comparisons, and has factories for `off`, `red`, `green`, `blue`, and
`orange`.

## Complete compatibility example

```cpp
#include <adk.h>

adk::led::Mono status (LED_BUILTIN);

void setup ()
{
    adk::initialize ();
}

void loop ()
{
    adk::update ();

    status.on  ();
    delay      (100);
    status.off ();
    delay      (100);
}
```

This example reflects the current API. New code should consult the component
status table before adopting a planned interface.
