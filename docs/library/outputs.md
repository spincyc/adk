# Lights and sound

Asking a part for what it is already doing changes nothing, so `blink ()`,
`fadeTo ()`, `beep ()`, `tone ()` and `play ()` can be called from every
pass of `loop ()`; asking for something different starts afresh.

<!-- api led.h Led -->

<!-- api rgb_led.h RgbLed -->

<!-- api color.h Color -->

The colors `adk::color::off`, `white`, `red`, `orange`, `yellow`, `green`,
`cyan`, `blue`, `purple`, `magenta` and `pink` are ready to use, and
`adk::blend ()` and `adk::wheel ()` make new ones.

<!-- api buzzer.h Buzzer -->

<!-- api speaker.h Speaker -->

<!-- api speaker.h Note -->

Note names run from `adk::note::c2` to `adk::note::c8`, with `s` for sharp:
`adk::note::cs4` is C♯4, and `adk::note::a4` is 440 Hz. `adk::note::rest` is
a silence.

<!-- api relay.h Relay -->
