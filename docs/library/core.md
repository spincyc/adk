# Core

The calls every sketch uses, the plain pins everything else is built on,
and a few helpers.

## Setting up and running

<!-- api object.h -->

<!-- api object.h Object -->

## Pins

<!-- api digital.h DigitalOutput -->

<!-- api digital.h DigitalInput -->

<!-- api analog.h AnalogInput -->

<!-- api analog.h PwmOutput -->

## Helpers

<!-- api every.h Every -->

<!-- api timer.h Timer -->

<!-- api timer.h Stopwatch -->

<!-- api analog.h Smoother -->

<!-- api debouncer.h Debouncer -->

## Containers and printing

The AVR has no C++ standard library, so ADK brings the few containers a sketch
wants, with the standard names for the same operations. None of them uses the
heap: each reserves all its room when it is declared, so the compiler's RAM
report counts it.

<!-- api containers.h Array -->

<!-- api containers.h Vector -->

<!-- api containers.h Deque -->

<!-- api containers.h Span -->

Any two of them compare item by item with `adk::equal (list, other)`, true
when they hold equal items in the same order, like `std::ranges::equal`:
`adk::equal (typed, code)` says whether the keys typed are the code.

### Printing

`Serial.print ()` prints one thing at a time. These print a whole line in one
call, to `Serial` or an `Lcd`:

```cpp
adk::println (Serial, "Red wins in ", time, " ms!");
lcd.at (0, 1);
adk::print (lcd, adk::fixed (celsius, 1), " C");
adk::println (Serial, "Card 0x", adk::hex (card, 8));
```

<!-- api print.h -->

<!-- api print.h Text -->

## Claiming pins

A part claims the pins and timers it uses from its `setup ()`. You only need
these when you write a part of your own.

<!-- api board.h -->

## Buses

The I2C and SPI buses that the clock, accelerometer and RFID reader share.
They are small and have no hidden buffers, so a sketch that declares none of
those parts pays nothing for them.

### I2C

<!-- api i2c.h -->

### SPI

<!-- api spi.h -->
