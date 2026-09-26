---
lesson: 31
title: Stepper
arc: Time
promise: Turn a motor to an exact angle by counting its steps, a quarter turn at a time.
time: 45 minutes
level: 2
sketch: Lesson31Stepper
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Power module and 9 V adapter
  - 28BYJ-48 stepper and ULN2003 driver
  - Push button
  - 6 female-to-male jumper wires
  - 3 jumper wires
  - A paper arrow and sticky tape
ideas:
  - How a stepper motor moves in steps
  - Half-steps and gearing
  - Why the motor needs a driver and its own power
  - Counting steps to reach an exact angle
---

## What you'll build

<!-- closeup -->

A paper arrow taped to a small motor points straight up. Press the button and
it swings round a quarter of a turn, then stops dead. Press again: right,
down, left, and up again, back where it began. While it moves, the
four red lights on the driver board flicker through a pattern, showing you
which of the motor's coils are switched on.

## The idea

The fan in Lesson 20 spun freely: you chose its speed, but never where it
stopped. A **stepper motor** is different: it moves in small, exact jumps called **steps**, and it stops after
each one. Inside the 28BYJ-48 are four coils of wire around a magnet. Switch
one coil on and the magnet turns to face it; switch on the next and it turns
on to that one. Your sketch switches the coils in order, and every switch is
one step.

ADK uses **half-steps**: one coil, then that coil and the next together, then
the next alone. That makes eight patterns for each round of the four coils,
and each pattern moves the motor half as far as a full step. The motor itself
takes 64 half-steps to turn once, and behind it is a **gearbox** that slows it
down 64 times, which makes it much stronger as well. So one turn of the
shaft takes:

<p class="formula">64 × 64 = 4096 half-steps</p>

A quarter turn is 1024 of them. Because the sketch counts every one, it always
knows where the shaft is, without any sensor to check: as exactly as the
gearbox allows, as you'll find out.

The coils need up to about 200 mA, ten times what a Mega pin can give. So
the pins don't power the coils: they tell the **ULN2003 driver** which coils
to switch, and the driver switches the current from the breadboard power
module, just as the servo in Lesson 17 took its power from there.

!!! question "Predict"
    The sketch moves the motor 500 half-steps every second. How long will one
    press take to turn the arrow a quarter of a turn? After four presses, will
    the arrow point exactly where it started? Write down your guesses.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable and switch the power module off before you change
    any wiring. Set the power module's bottom yellow jumper to **5V**, never
    3.3V. Never power the driver from the Mega's 5V pin: the motor could pull
    the Mega's power down and reset it, or damage it.

<!-- bench -->

<!-- steps -->

Set the power module's bottom yellow jumper to **5V**: it feeds the driver
through the bottom rails. Nothing uses the top rails, so set the top jumper to
**OFF**.

Last, push the motor's white plug into the socket on the driver board. It only
fits one way round. Plug the 9 V adapter into the power module's round socket,
tape the paper arrow to the motor's shaft so it points straight up, and stand
the motor on the table with the shaft facing you.

??? info "What the driver board does"
    The black chip on the driver board, the ULN2003, holds seven electronic
    switches. A milliamp or so from a Mega pin into IN1 closes the switch for
    the first coil, and then up to 200 mA can flow through that coil from the
    power module. The four LEDs, A to D, light
    with IN1 to IN4, so they show you each coil switching.

    The black wire from the Mega's GND to the − rail matters: the driver
    measures the Mega's signals against its own GND, so the two grounds have
    to be joined.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → Lesson31Stepper**:

<!-- sketch -->

What's new:

- `adk::Stepper motor {A8, A9, A10, A11};` is the driver board on four pins,
  given in the order IN1 to IN4. The analog pins work as ordinary on/off
  pins here.
- `adk::Stepper::StepsPerRevolution` is 4096, the half-steps in one turn,
  so the constant `quarterTurn` is 1024. It's a `long`, from Lesson 8,
  because the motor counts its steps in `long`s: an `int` stops at 32,767,
  only eight turns.
- `motor.speed (500);` asks for 500 half-steps a second, the fastest ADK
  turns it: much faster, and the motor can stall and just hum.
- `motor.step (quarterTurn);` starts a move of 1024 half-steps on from
  wherever the last move ends. It doesn't wait: `adk::update ()` takes each
  step when its moment comes, so the button is still watched while the motor
  turns. Press twice quickly and it goes on for half a turn.

## Upload it

Switch the power module on, then plug in the Mega and upload the sketch.
Press the button. The arrow swings a quarter of a turn and stops, and while
it moves the driver's four LEDs flicker. Four presses bring it all the way
round.

You predicted how long a press takes: 1024 half-steps at 500 a second is
1024 ÷ 500, just over two seconds. And after four presses the arrow is
*almost* where it started, but about 2 degrees past it. The gearbox isn't
exactly 64 to 1, so 4096 half-steps are a little more than one real turn.
You'll measure that in the third challenge below.

Which way does it turn? ADK means positive steps to turn the shaft clockwise,
seen from the shaft end, but that hasn't been checked on a real motor yet.
Watch yours: if it goes anticlockwise, that's fine, and you now know which
way positive means for your motor.

## If it doesn't work

| What you see | Try this |
|---|---|
| Nothing moves, and the driver's LEDs stay dark | Check the power module is switched on, its LED is lit, its bottom jumper is on 5V, and the red and black wires go from the bottom rails to the driver's + and − pins. |
| The LEDs flicker but the shaft doesn't turn | Push the motor's white plug fully into its socket. |
| The motor hums or shakes but hardly turns | Two of the IN wires are swapped: IN1 to A8, IN2 to A9, IN3 to A10, IN4 to A11. |
| Nothing happens when you press | The button must straddle the middle gap, with pin 22's wire in column 2 and the black wire from a4 to the − rail. |
| The Mega resets when the motor starts | The driver is taking power from the Mega. Its + pin must go to the power module's rail, never the Mega's 5V. |
| The little **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    ADK keeps two numbers for the motor: where it is and where it's going,
    both counted in half-steps from where it was when the Mega started. Each
    `adk::update ()` works out how many milliseconds have passed, and at 500
    half-steps a second it moves one half-step every two milliseconds,
    switching the four pins to the next of the eight patterns.

    When a move ends, ADK switches all four coils off, so the motor doesn't
    get warm standing still. The gearbox is stiff enough to hold the shaft
    where it is. `motor.hold (true)` keeps the coils on instead.

## Make it yours

1. **Slow motion.** Change `motor.speed (500)` to `motor.speed (2)`. Now
   watch the driver's LEDs: one, two, one, two... can you see the eight
   half-step patterns go round?
2. **Home.** Add a second button on pin 23 that sends the arrow home with
   `motor.moveTo (0);`. After four presses, does it turn back the short way or
   the long way? Work out why from what `moveTo` counts.
3. **The true turn.** The gearbox isn't exactly 64 to 1: it's about 63.68, so
   a real turn is about 4076 half-steps, and 4096 overshoots by nearly 2
   degrees. Press the button 40 times and see how far the arrow has crept.
   Then set `quarterTurn` to 1019 and try again.
4. **A second hand.** Make the arrow tick round once a minute, one step every
   second, like a clock's second hand. Use an `adk::Every tick {1000};` and
   count seconds, then move to `seconds * 4096L / 60` each tick, so the
   rounding never builds up.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): the dial on DC volts
(**V⎓**), the black lead in **COM** and the red one in **V**. Never use the
**10A** socket here: across a supply, it is a short circuit. These readings
hold still by themselves, so just switch the power module on. The + and −
holes of a rail pair are only 2.5 mm apart: push each probe tip into its own
hole, so neither can slip across and touch the other rail.

!!! question "Predict"
    The top jumper is off. What will the meter read across the top rails:
    5 V, 3.3 V, or nothing at all?

<!-- measure -->

What the numbers tell you:

- **The driver's supply** is the power module's 5 V; none of it comes from
  the Mega. Press the button and watch the reading while the motor turns: it
  hardly moves, because the module has plenty to spare for the coils'
  200 mA.
- **The top rails** read 0: with their jumper off, nothing feeds them. Each
  pair of rails gets its voltage from its own jumper. The − rails are joined
  inside the module, whatever the jumpers say.
- Now switch the power module off, leave the Mega plugged in, and press the
  button. The driver's supply reads 0, its LEDs stay dark and the arrow
  stays put, but the sketch counts its 1024 half-steps just the same:
  nothing tells it the motor didn't move. Switch the module back on: the
  sketch's count is now a quarter turn ahead of the real arrow, and nothing
  will put it right. That is the price of counting steps without a sensor.
