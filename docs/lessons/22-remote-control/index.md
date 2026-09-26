---
lesson: 22
promise: Read the kit's remote control, and use it to run a colored lamp from across the room.
time: 45 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - IR receiver module
  - The kit's remote control
  - RGB LED
  - 3 × 220 Ω resistors (red, red, black, black, brown)
  - 4 jumper wires
  - 3 female-to-male jumper wires
ideas:
  - Infrared light, which you can't see
  - How a remote sends a code, and how to read yours
  - Choosing what to do from the code
---

## What you'll build

<!-- closeup -->

A lamp you control from the sofa. Press the remote's power button and an
RGB LED fades up to white; press 1 to 6 for red, green, blue, yellow,
purple or white; hold volume down and it dims step by step, volume up and it
brightens. And on the way, the Serial Monitor shows you the secret number
each button sends.

## The idea

**Light you can't see.** The little bulb on the end of the remote is an LED,
like the ones you've been using, but its light is **infrared**: a color just
beyond red, which your eyes can't see. Many phone cameras can, though: point
the remote at a phone's camera, press a button and look at the screen.

A room is full of infrared already, from sunlight and lamps. So the remote
doesn't simply shine: it flickers its LED 38,000 times a second, and the
receiver's dark window hides a detector that only answers light flickering
at that rate. Everything else is ignored.

**A code, as long and short gaps.** The kit's remote speaks a language
called **NEC**. Each press sends a burst of 9 ms, a gap of 4.5 ms, then 32
bits. Every bit is a short burst of 0.56 ms followed by a short gap (a 0) or
a long gap (a 1). The 32 bits carry an address, which says which remote
this is, and a command, which says which button, each sent twice, the
second time upside down, so the receiver can check nothing was lost:

<p class="formula">9 ms + 4.5 ms + 32 bits of about 1.7 ms ≈ 67 ms per press</p>

Hold a button down and the remote sends a short "still held" code, a
**repeat**, every 108 ms, about nine times a second.

**Reading your own remote.** The kit's remote sends these commands, and ADK
knows them by name, as `adk::remote::power` and so on:

| Button | Command | Button | Command | Button | Command |
|---|---|---|---|---|---|
| POWER | 0x45 | VOL+ | 0x46 | FUNC/STOP | 0x47 |
| ⏮ | 0x44 | ⏯ | 0x40 | ⏭ | 0x43 |
| ▼ | 0x07 | VOL− | 0x15 | ▲ | 0x09 |
| 0 | 0x16 | EQ | 0x19 | ST/REPT | 0x0D |
| 1 | 0x0C | 2 | 0x18 | 3 | 0x5E |
| 4 | 0x08 | 5 | 0x1C | 6 | 0x5A |
| 7 | 0x42 | 8 | 0x52 | 9 | 0x4A |

The `0x` means the number is written in hexadecimal, counting in sixteens:
0x45 is 4 × 16 + 5 = 69. Remotes differ, so the sketch prints every code it
receives: the first thing you'll do is check yours against this table.

!!! question "Predict"
    Point the remote at the ceiling instead of at the receiver and press
    POWER. Will the lamp still come on?

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring. The receiver's three
    pins may be marked **S**, **+** and **−**, or **Y**, **R** and **G** (Y is
    the signal, R is 5 V and G is GND): match them by their names, not by
    where they sit. The RGB LED's longest leg is its common one: it goes
    in the − rail. If the remote is new, pull out the clear plastic tab
    that keeps its battery from running down.

<!-- bench -->

<!-- steps -->

??? info "The lamp and the receiver"
    The RGB LED stands just as it did in Lesson 4: its color legs spread
    out into a6, a9 and a11, each with its own 220 Ω resistor standing
    across the middle gap above it, and its longest leg straight down in
    the − rail. Red takes about 14 mA, green and blue about 8 mA.

    The receiver sits between the Mega and the breadboard, under the LED's
    three wires. It takes its 5 V and GND straight from the Mega: the inner
    pins at the top and bottom ends of the long header.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson22RemoteControl**:

<!-- sketch -->

What's new:

- `adk::IrReceiver receiver {2};` is the receiver. It must go on a pin that
  can interrupt the Mega (2, 3, 18, 19, 20 or 21); more on that below.
- `struct Choice` pairs a number button with the color it chooses, and
  `choices` is an `adk::Array` of them, as in Lesson 5: one row for each
  button, so each button sits beside its color.
- `receiver.wasReceived ()` is true for one pass of `loop ()` each time a
  code arrives, and `receiver.isRepeat ()` says whether it was a repeat
  from a held button. `receiver.command ()` is the button's number.
- `obey ()` decides what each button does. Power only toggles on a fresh
  press, so holding it doesn't make the lamp flicker; volume works on
  repeats too, so holding it keeps going. The `for` loop looks through the
  choices for the button, and takes its color.
- `showLamp ()` uses `adk::blend ()` to find the color some eighths of the
  way up from off, and `lamp.fadeTo ()` from Lesson 4 glides to it, or to
  off, in 200 ms.
- `adk::hex (button, 2)` prints each new code in hexadecimal, with at
  least two digits, so 0x0C keeps the 0 in front.

## Upload it

1. Upload the sketch and open the Serial Monitor at 9600 baud.
2. Aim the remote at the receiver's dark window and press each button in
   turn. Each press prints a line such as `Button code 0x45`; check them
   against the table above. If the receiver has a small LED, it flickers as
   each code arrives.
3. Press POWER: the lamp fades up to white. Press 1 to 6 to change its
   color, hold VOL− to dim it and VOL+ to brighten it, and press POWER to
   fade it out.

You predicted whether the lamp would come on with the remote aimed at the
ceiling. Try it: in most rooms it does. Infrared bounces off ceilings and
walls just as light does, and the receiver only needs a little of it. In a
big room, or from far away, the bounce may be too faint.

## If it doesn't work

| What you see | Try this |
|---|---|
| Nothing prints and the lamp doesn't react | Check the receiver's signal pin goes to pin 2, its + to the 5V pin at the top of the long header and its − to the GND pin at the bottom end, the right way round. Try from closer, and away from bright sunlight. |
| Codes print, but not the ones in the table | Your remote is a different model. Put the numbers you see in the sketch in place of the names, for example `Choice {0x0C, adk::color::red}` or `button == 0x45`. |
| One press prints the same code several times | Some remotes send the whole code again, instead of a repeat, for as long as a button is held. Tap the button quickly. |
| A color is missing or wrong | Check that color's wire and resistor: pin 5 is red, 6 green and 7 blue, and each resistor lands in its leg's column. |
| The lamp is always off | The longest leg must be in the − rail, and that rail joined to GND. |
| The **L** LED blinks long and short flashes | ADK found a problem with a pin. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    The receiver's output goes low whenever infrared arrives. Pin 2 can
    **interrupt** the Mega: every time the level changes, the Mega drops
    what it's doing for a few microseconds, notes the time, and goes back.
    From the lengths of the bursts and gaps ADK decodes the bits as they
    come, so your sketch never has to wait for a code.

    When all 32 bits are in, ADK checks that the command and its upside
    down copy match, and throws the code away if they don't. The next
    `adk::update ()` hands it to your sketch.

## Make it yours

1. **Rainbow.** Make the EQ button start a slow walk round the color wheel
   with `adk::wheel ()`, as in Lesson 4, and any number button stop it.
2. **Sleep timer.** Make ⏯ (`adk::remote::play`) fade the lamp slowly to
   off over a minute, with `lamp.fadeTo (adk::color::off, 60000)` in place
   of `showLamp ()`.
3. **More colors.** Add three more rows to `choices`, so buttons 7, 8 and 9
   give orange, cyan and pink (`adk::color::orange`, `cyan` and `pink`).
4. **Another remote.** Try a TV remote from home. Some speak NEC and will
   print codes; many use other languages that ADK doesn't decode, and print
   nothing at all.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts, the black
lead in **COM** and the red one in **V**.

A meter is far too slow to see a code: a whole press is over in 67 ms. The
receiver's wires also run straight to the Mega, with no hole for a probe.
What the meter can see is what each code does to the lamp's pins, and the
lamp keeps its color until the next press, so there is plenty of time to
read it. Press the buttons first, then put the probes in place.

!!! question "Predict"
    Press 1 for red, then VOL− four times. What will the red pin, 5, read?

<!-- measure -->

What the numbers tell you:

- **At full brightness** the red pin reads about 5 V: its number is 255 out
  of 255, on all the time.
- **Four presses dimmer**, `brightness` is 4 eighths, so `adk::blend ()`
  makes the red number 255 × 4 ÷ 8 = 127, and PWM gives the pin
  127 ÷ 255 × 5 V ≈ 2.5 V. Each press of VOL− takes about 0.6 V off, an
  eighth of 5 V, until it stops at one eighth, as `brightness > 1` in the
  sketch says.
- **Across the blue LED**, after pressing 3 and VOL+ until it is back at
  full, the meter reads about 3.2 V. Red keeps about 2 V; measure it from
  b6 to the − rail after pressing 1. That is why blue takes about 8 mA to
  red's 14: (5 V − 3.2 V) ÷ 220 Ω ≈ 8 mA.
