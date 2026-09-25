---
lesson: 22
title: Remote Control
arc: Invisible signals
promise: Read the kit's remote control, and use it to run a colored lamp from across the room.
time: 45 minutes
level: 2
sketch: Lesson22RemoteControl
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - IR receiver module
  - The kit's remote control
  - RGB LED
  - 3 × 220 Ω resistors (red, red, black, black, brown)
  - 6 jumper wires
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

??? info "Why the resistors fan out"
    The RGB LED's four legs are only a tenth of an inch apart, too close for
    three resistors side by side. So each resistor stands across the middle
    gap and leans a little, from its wire's column down to its leg's
    column. Each color gets its own 220 Ω, as in Lesson 4: red takes about
    13 mA, green and blue about 9 mA.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson22RemoteControl**:

<!-- sketch -->

What's new:

- `adk::IrReceiver receiver {2};` is the receiver. It must go on a pin that
  can interrupt the Mega (2, 3, 18, 19, 20 or 21); more on that below.
- `receiver.wasReceived ()` is true for one pass of `loop ()` each time a
  code arrives, and `receiver.isRepeat ()` says whether it was a repeat
  from a held button. `receiver.command ()` is the button's number.
- `printCode ()` prints each new code in hexadecimal: the `HEX` in
  `Serial.println (button, HEX)` asks for it.
- `obey ()` decides what each button does. Power only toggles on a fresh
  press, so holding it doesn't make the lamp flicker; volume works on
  repeats too, so holding it keeps going.
- The two arrays, as in Lesson 6, pair each number button with a color:
  the loop finds the button in the first and takes the color from the
  second.
- `dimmed ()` uses `adk::blend ()` to find the color some eighths of the way
  up from off, and `lamp.fadeTo ()` from Lesson 4 glides to it in 200 ms.

## Upload it

1. Upload the sketch and open the Serial Monitor at 9600 baud.
2. Aim the remote at the receiver's dark window and press each button in
   turn. Each press prints a line such as `Button code 0x45`; check them
   against the table above. If the receiver has a small LED, it flickers as
   each code arrives.
3. Press POWER: the lamp fades up to white. Press 1 to 6 to change its
   color, hold VOL− to dim it and VOL+ to brighten it, and press POWER to
   fade it out.

## If it doesn't work

| What you see | Try this |
|---|---|
| Nothing prints and the lamp doesn't react | Check the receiver's signal pin goes to pin 2 and its other two pins to the top rails, the right way round. Try from closer, and away from bright sunlight. |
| Codes print, but not the ones in the table | Your remote is a different model. Change the names in the sketch to the numbers you see, for example `button == 0x45`. |
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
2. **Sleep timer.** Make ⏯ fade the lamp slowly to off over a minute:
   `lamp.fadeTo (adk::color::off, 60000)`.
3. **More colors.** Use buttons 7, 8 and 9 for orange, cyan and pink
   (`adk::color::orange`, `cyan` and `pink`).
4. **Another remote.** Try a TV remote from home. Some speak NEC and will
   print codes; many use other languages that ADK doesn't decode, and print
   nothing at all.
