---
lesson: 17
title: Servo
arc: Keys and motion
promise: Make a motor turn to exactly the angle you ask for, powered the safe way.
time: 45 minutes
level: 2
sketch: Lesson17Servo
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - SG90 servo
  - Breadboard power module and its 9 V adapter
  - 10 kΩ potentiometer
  - 7 jumper wires
  - A piece of card and some tape, for the dial
ideas:
  - Setting an angle with the width of a pulse
  - Why motors need their own power supply
  - Jumping or gliding to a position
---

## What you'll build

<!-- closeup -->

A dial with a needle that obeys a knob. Turn the knob a little and the servo's
white arm, the **horn**, swings to match; turn it all the way and the horn
sweeps half a circle. Tape a paper scale behind the horn and you have a
gauge that can point at anything: the temperature, a score, or how hungry the
cat is.

## The idea

A **servo** is a small motor with a gearbox, a sensor that knows where its
output shaft is pointing, and a little circuit that keeps turning the motor
until the shaft points where it's told. You don't tell it to turn; you tell it
an **angle**, and it goes there and holds it.

The angle travels down the orange wire as a pulse, fifty times a second. The
*width* of the pulse is the message. ADK's default, the same as Arduino's own
Servo library, is a pulse of 544 µs (millionths of a second) for 0° and
2400 µs for 180°, and anything between means an angle in proportion. Servos
differ a little, so you can change both ends if yours needs it. Halfway, 90°,
is

<p class="formula">544 µs + <span class="fraction"><span>90</span><span>180</span></span> × (2400 µs − 544 µs) = 1472 µs</p>

and then the wire rests low until the next pulse, 20 ms after the last one
began. It's the same idea as PWM in Lesson 7, but here only the width of the
pulse matters, not how bright it would make an LED.

**A servo needs its own power.** While it moves, its motor gulps several
hundred milliamps in sudden bursts. A Mega pin can give about 20 mA, and even
the Mega's 5V pin, fed from your computer's USB port, can dip so far that the
Mega resets. So the servo takes its power from the **breadboard power
module**, which plugs into the rails and turns a 9 V adapter into a steady
5 V, and the Mega sends only the signal. One more wire joins the Mega's GND to
the rails: a pulse is a voltage measured from GND, and the servo can only read
it if they share the same GND.

!!! question "Predict"
    Once it's all running, what do you think happens if you switch the power
    module off and turn the knob? And what will the servo do the moment you
    switch it back on? Write down your guess.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable and the power module's adapter before you wire.
    Take out Lesson 16's screen and keypad with their wires, and the red
    wire from the Mega's 5V to the top + rail: the power module feeds the
    rails now. Keep the black GND wire into B-3. Plug the power module into
    the far end of the breadboard so that its **+** and **−** pins match the
    red **+** and blue **−** stripes on *both* sides of the breadboard, and
    set both of its yellow jumpers to **5V**, never 3.3V. The knob is the
    only thing on the Mega's 5V: its own red wire runs from the 5V pin on
    the power header, left of A0, into a47 by the knob's right leg. Never
    connect the servo's red wire to the Mega's 5V pin. Keep fingers and hair
    away from the horn when it moves, and don't force it round by hand.

<!-- bench -->

<!-- steps -->

??? info "The power module"
    It takes 6.5 to 12 V from its barrel socket (the kit's 9 V adapter, or a
    9 V battery on a snap) and turns it into 5 V on each pair of rails,
    enough for a servo or two. Its button switches it on, and its little LED
    lights when it is. It has a USB socket too; in this course, always power
    it through the barrel socket with the kit's adapter.

    The knob doesn't use the rails' 5 V. It takes the Mega's own, from the
    5V pin on the power header: the Mega measures A0 against that 5 V, so a
    knob fed from it reads from 0 to 1023 exactly, and it keeps working with
    the power module switched off. The two 5 Vs never meet; only their GNDs
    join, at the − rail.

When you are done, these are the connections your circuit makes:

<!-- connections -->

Now make the dial: cut a half circle from card, mark 0° at one end, 90° at
the top and 180° at the other end, and tape it behind the horn, its center on
the servo's shaft.

## Code it

Open **File → Examples → Adk → Lesson17Servo**:

<!-- sketch -->

What's new:

- `adk::Servo needle {44};` names the servo on pin 44. Only pins 44, 45 and
  46 can drive a servo, because ADK makes the pulses with the Mega's Timer 5,
  in hardware, so they never wobble.
- `knob.read (0, 180)` turns the knob into an angle, as in Lesson 7.
- `needle.moveTo (angle, 300);` asks the servo to glide to that angle over
  300 ms, while the sketch carries on. Asking again for the angle it is
  already gliding to changes nothing, which is why it can sit in `loop ()`.
- Its sister `needle.write (angle);` jumps straight there, as fast as the
  servo can go.

## Upload it

Plug in the USB cable, then the power module's adapter, and press the module's
button so its LED lights. Upload the sketch. The servo swings to wherever the
knob points and stops. Turn the knob slowly: the needle follows, a third of a
second behind. Turn it quickly from one end to the other and the needle
glides smoothly across instead of jerking.

Now test your prediction. With the module switched off, the knob still works,
because it runs on the Mega's own 5 V, but the servo is limp: it has the
signal but no power to act on it. Switch it back on and the servo snaps to
wherever the knob now points.

## If it doesn't work

| What you see | Try this |
|---|---|
| The servo never moves | Is the power module's LED on? Check the adapter, the button and that both jumpers are on 5V. Then check the black wire from the Mega's GND to the bottom − rail (B-3): without it the servo can't read the signal. |
| It moves, but not with the knob | Check the servo's orange wire goes to pin 44, the knob's middle leg to A0, and the red wire from the 5V pin on the Mega's power header to a47. |
| The Mega resets or the USB disconnects when the servo moves | The servo is getting power from the Mega. Its red wire must go to the bottom + rail (B+53), fed by the power module. |
| The needle turns the opposite way to the knob | Nothing is wrong. To swap it, change `knob.read (0, 180)` to `knob.read (180, 0)`. |
| The servo hums or twitches when it should be still | The knob's reading wobbles by one step, and the servo chases it. See the second challenge below. |
| It buzzes at one end of its travel | It's pushing against its end stop. Use `adk::Servo needle {44, 600, 2300};` to narrow the pulses a little. |
| The **L** LED blinks long and short flashes | A pin problem: see [Faults](../../library/index.md#faults). A servo on a pin other than 44, 45 or 46 is refused. |

??? note "How it works"
    `adk::setup ()` sets up Timer 5 to count in half-microseconds and start a
    pulse every 20 ms, but sends nothing until the first `write ()` or
    `moveTo ()`, so the servo doesn't twitch at power-up. After that the
    timer makes every pulse in hardware: the sketch only changes a number that
    says how wide the next pulse should be, and the change waits for the
    current pulse to finish, so a pulse is never cut short.

    A glide is a series of those changes. In every `adk::update ()`, ADK works
    out how far through the 300 ms it is and sets the pulse to that fraction
    of the way from the old width to the new one.

    `adk::stop ()` ends the pulses, and the servo goes limp.

## Make it yours

1. **Jump or glide.** Change `moveTo (angle, 300)` to `write (angle)`, then
   try `moveTo (angle, 2000)`. Which feels most like a real gauge?
2. **Steady needle.** Only move when the knob has changed by at least 2°:
   compare `angle` with `needle.angle ()`, the angle it is sending now.
3. **Windscreen wiper.** Forget the knob: make the horn sweep from 0° to 180°
   and back forever, using `needle.isMoving ()` to know when each sweep is
   done. Then let the knob set the speed.
4. **A real gauge.** Add the thermistor from
   [Lesson 14](../14-thermometers/index.md) on A2, and make the needle point
   to the temperature on a dial marked from 15 °C to 35 °C.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it to DC volts as in [Lesson 1](../01-blink/index.md#measure-it), black lead
in **COM** and red in **V**. The servo holds its angle and the knob stays
where you leave it, so the sketch needs no change. Keep your fingers clear
of the horn, and take care on the rails: a probe tip touching the + and −
rails together would short the power module.

!!! question "Predict"
    The servo's 5 V and the knob's 5 V come from two different places. When
    you switch the power module off, which of them will fall to 0 V? Write
    down your guess, then take the first two readings with the module on,
    and again with it off.

<!-- measure -->

What the numbers tell you:

- **The servo's 5 V** comes from the power module's own regulator. Switch
  the module off and it falls close to 0 V, and the servo goes limp.
- **The knob's 5 V** comes from the Mega, which gets it from your computer's
  USB port, so it stays with the module off. That's why the knob still
  works. Compare it with the first reading: the two are seldom exactly
  equal. Joined, the higher would push current back into the other, which
  is why the Mega's 5V never goes to the rails while the power module is
  there. Only their GNDs are joined, so that the servo can read the
  pulses.
- **The knob's wiper** is the angle, as a voltage. `knob.read (0, 180)`
  turns 0 V into 0° and 5 V into 180°, so 90° is 2.5 V, and each degree is
  5 V ÷ 180 ≈ 0.03 V. Turn the knob and watch the needle and the meter move
  together.
