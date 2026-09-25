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

<!-- api analog.h Smoother -->

<!-- api debouncer.h Debouncer -->

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
