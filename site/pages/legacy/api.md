# Legacy API

The legacy preview exposed global initialization and lightweight LED objects:

```cpp
adk::led::Mono led (LED_BUILTIN);

adk::initialize ();
led.on          ();
led.off         ();
```

It also supplied `adk::led::Rgb` and named `adk::color` values. These names are
documented to explain archived sketches, not as a compatibility promise.

## Why it was replaced

The preview could demonstrate output commands, but its global lifecycle did
not express which object owned a pin, how partial initialization failed, when
hardware returned to a safe state, or how time and I/O could be tested without
a board. Those omissions become serious as circuits compose.

The supported design therefore makes ownership, lifecycle, error status, time,
and hardware access explicit. Destruction remains safe in programs that use
exceptions even though ADK does not throw internally.

## Migration rule

Treat an old sketch as a circuit specification:

1. identify each pin and its required safe state;
2. construct the corresponding supported components;
3. initialize each component and check its status;
4. drive deterministic updates explicitly; and
5. verify shutdown and destruction with host tests before uploading.

There is intentionally no automatic source-compatibility shim. Consult the
supported component reference for current declarations and contracts.

