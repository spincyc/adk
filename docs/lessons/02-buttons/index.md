---
lesson: 2
title: Buttons
arc: First light
promise: Make the Mega listen to your finger, and count every press.
time: 45 minutes
level: 1
sketch: Lesson02Buttons
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 2 push buttons
  - Red LED and yellow LED
  - 2 × 220 Ω resistors (red, red, black, black, brown)
  - 9 jumper wires
ideas:
  - Pins that listen as well as switch
  - Pull-up resistors, and why a pressed button reads LOW
  - Bounce, and how ADK waits it out
  - Events and states, wasPressed () and isPressed ()
  - Counting in a variable, and the Serial Monitor
---

## What you'll build

<!-- closeup -->

Two buttons and two lights. Tap the left button and the red LED comes on;
tap it again and it goes off, like the light switch by your door. Hold the
right button and the yellow LED glows for exactly as long as your finger stays
down, like a doorbell. Meanwhile your computer keeps score: every tap of the
left button appears on the screen, *Presses: 1*, *Presses: 2*, *Presses: 3*.

## The idea

In [Lesson 1](../01-blink/index.md) a pin *switched*: your program turned it
on and off. A pin can also *listen*. Set up as an **input**, it reports
whether it sees about 5 V, called **HIGH**, or about 0 V, called **LOW**.

A push button is two metal contacts and a spring. Here each button is wired
from its pin to GND, so pressing it joins the pin straight to 0 V. But when
it is *not* pressed, the pin is joined to nothing at all, and a pin joined to
nothing **floats**: it picks up stray electricity from the wires and your hand,
and reads HIGH or LOW at random.

So the Mega switches on a **pull-up**: a resistor inside the chip, between 20
and 50 kΩ, from the pin up to 5 V. It gently holds the pin HIGH until the
button, a much stronger path, pulls it LOW. That makes a pressed button read
LOW, which sounds backwards but never wavers. And the current that flows
while you press is tiny:

<p class="formula">current = <span class="fraction"><span>5 V</span><span>20 kΩ</span></span> = 0.25 mA at the very most</p>

That's why the buttons need no resistors of their own.

Metal contacts also **bounce**. When they snap together they spring apart and
touch again, several times, in the first few thousandths of a second
(milliseconds, ms). The Mega checks its pins many thousands of times a second,
fast enough to see every bounce, so one press could look like five. ADK
waits until a button has stayed the same for 20 ms before it believes it
has changed. You'll never notice 20 ms, and the Mega will never be fooled.

Now you can ask a button two different questions:

- **Is it pressed right now?** `isPressed ()` answers with a **state**: true
  for as long as you hold the button down.
- **Did it just go down?** `wasPressed ()` answers with an **event**: true
  once, at the moment of the press, and not again until you let go and press
  again.

A light switch wants the event: one tap, one change. A doorbell wants the
state. This sketch uses one of each.

!!! question "Predict"
    Press the left button and hold it down for three seconds. Does the red
    LED flicker on and off the whole time, stay on, or change just once? And
    how many presses will the screen count? Write down your guesses.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring. The buttons need no
    resistors, because the Mega's pull-ups look after them, but each LED still
    needs its own 220 Ω resistor, just as in Lesson 1.

<!-- bench -->

<!-- steps -->

??? info "How a button sits across the gap"
    A push button has four legs, and it straddles the middle gap with two
    legs on each side. Inside, each leg is joined to the leg straight across
    the gap from it, so the button's left legs are always joined to each
    other, and so are its right legs. Pressing joins left to right.

    So the signal comes in at the top left, in column 1, and GND leaves at
    the bottom right, in column 3: the pin only reaches GND while you press.
    Using opposite corners like this works even if your button happens to
    join its legs the other way inside. Push each button down firmly until all
    four legs are in; they are stiff, and a leg that misses its hole makes a
    button that never works.

    The LEDs are wired as in Lesson 1, but the resistor now stands across the
    middle gap: the signal arrives at the top of the board, crosses the gap
    through the resistor, and reaches the LED's long leg below.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson02Buttons**:

<!-- sketch -->

What's new since Lesson 1:

- `adk::Button leftButton {22};` says there is a button between pin 22 and
  GND. ADK switches on the pull-up and does the debouncing.
- `Serial.begin (9600);` opens the USB link to your computer, at 9600 bits a
  second. `adk::setup (Serial);` does everything `adk::setup ()` does, and if
  it finds a mistake in your sketch, it also explains it in words (see *How
  it works* below).
- `adk::update ();` comes first in `loop ()`. This is the moment ADK reads
  every button, so everything `wasPressed ()` and `isPressed ()` tell you
  comes from here. In Lesson 1, `adk::wait ()` did it for you while it
  waited; a loop that never waits calls `adk::update ()` itself.
- `if (leftButton.wasPressed ())` asks for the event. Then `red.toggle ()`
  flips the LED: off if it was on, on if it was off.
- `yellow.set (rightButton.isPressed ());` copies the right button's state to
  the yellow LED, on every pass: pressed lights it, not pressed puts it out.
- `int presses = 0;` makes a **variable**, a named box that holds a whole
  number, starting at 0. It sits outside the functions, so it keeps its
  number from one pass of `loop ()` to the next. `countPress ()` adds one and
  prints the total with `Serial.print ()` and `Serial.println ()`; the `ln`
  means "and start a new line".

## Upload it

Upload the sketch as in Lesson 1. Then:

1. Hold down the right button. The yellow LED lights, and goes out the moment
   you let go.
2. Tap the left button. The red LED comes on and stays on. Tap it again and
   it goes off.
3. Open the **Serial Monitor**: **Tools → Serial Monitor**, or the magnifying
   glass at the top right of the IDE. Set its speed menu to **9600 baud**.
   Each tap of the left button adds a line: *Presses: 1*, *Presses: 2*, and
   so on. Opening the Serial Monitor usually restarts the Mega, so the count
   may start again from 1.

Was your prediction right? Holding the button changes the red LED once, and
counts once: a press is one event, however long it lasts.

## If it doesn't work

| What you see | Try this |
|---|---|
| Pressing a button does nothing | Push the button firmly down until all four legs are in, across the gap in rows e and f. Check its black wire runs from row a to the − rail. |
| The yellow LED is on all the time | The right button's black wire may be on the wrong side. It belongs in a7; in column 5 it would join pin 23 to GND all the time. |
| Nothing works at all, buttons or LEDs | Check the black wire from the Mega's GND pin to the − rail (B-5). Every part here returns through it. |
| An LED never lights | Turn it round: its long leg goes in b10 (red) or b15 (yellow). Check its resistor reaches from row g, across the gap, to row e. |
| The Serial Monitor shows strange characters | Set the Serial Monitor's speed menu to 9600 baud. |
| The Serial Monitor stays empty | Tap the left button: the sketch only prints when you do. Check the port in **Tools → Port**. |
| The Mega's **L** LED blinks long and short flashes | ADK found a pin mistake in the sketch. Open the Serial Monitor to read what it is. |

??? note "How it works"
    Every time `adk::update ()` runs, each button reads its pin once. ADK
    keeps a note of the last time the reading changed; only when a reading
    has held for 20 ms does it accept it as the button's new state. The
    update in which that happens is the one where `wasPressed ()` (or
    `wasReleased ()`) is true. On the next update it is false again, so each
    press is seen exactly once.

    `adk::setup (Serial)` checks the pins like `adk::setup ()` does, and
    tells you what it found. Try a mistake on purpose. Change the yellow
    LED's line to

    ```cpp
    adk::Led    yellow      {23};
    ```

    and upload. Nothing lights, and the Mega's **L** LED blinks 2 long, 3
    short: pin 23. Open the Serial Monitor and it says why:

    ```text
    adk: pin 23 is used twice
    ```

    The right button already has pin 23, so ADK refuses to start and puts
    every pin back to a harmless input. Without that check, the sketch would
    drive the button's pin as an output, and pressing the button would
    short it to GND. Put `27` back before you go on.

## Make it yours

1. **Count both.** Give the right button a counter of its own, and print
   both totals each time either one changes.
2. **Blink while held.** Make the yellow LED blink while you hold the right
   button: `yellow.blink (200);` when it is pressed, `yellow.off ();` when it
   is not. Asking for the same blink again on every pass is fine: ADK only
   starts it once.
3. **Every fifth press.** Light the yellow LED for a moment on every fifth
   press of the left button. `presses % 5` is the remainder when `presses` is
   divided by 5, so it is `0` on presses 5, 10, 15, and so on.
4. **See the bounce.** Change the first button to `adk::Button leftButton
   {22, 0};`. The second number is how long ADK waits for a button to settle,
   in milliseconds, and 0 turns debouncing off. Tap the button many times and
   watch the count. Does it ever jump by two or three? Some buttons bounce
   more than others. Put it back to `{22}` when you have seen it.
