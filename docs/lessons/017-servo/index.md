---
lesson: 17
promise: Make a motor follow the angle you ask for, powered the safe way.
time: 45 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - The LCD, knob and 220 Ω resistor from Lesson 13, wired as before
  - SG90 servo
  - Breadboard power module and its 9 V adapter
  - The kit's second 10 kΩ potentiometer
  - 6 more jumper wires
  - 2 female-to-male jumper wires, for the power module
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
sweeps half a circle. The screen from Lesson 13 shows the angle the knob
asks for, and the angle the Mega is sending as the needle glides after it.
Tape a paper scale behind the horn and you have a gauge that can point at anything:
the temperature, a score, or how hungry the cat is.

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
module**, which turns a 9 V adapter into a steady 5 V, and the Mega sends
only the signal. Two wires from the module feed only the bottom rails, for
the servo; the screen, as in Lesson 16, and the knob run on the Mega's own
5 V, on the top rails. The Mega's GND and the module's GND share the bottom
− rail; the top and bottom + rails stay separate. A pulse is a voltage
measured from GND, so the servo and Mega need that shared GND.

!!! question "Predict"
    Once it's all running, what do you think happens if you switch the power
    module off and turn the knob? And what will the servo do the moment you
    switch it back on? Write down your guess.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable and the power module's adapter before you wire. Keep
    Lesson 16's screen, its contrast knob and resistor with their wires, and
    the Mega's GND and 5V wires, just as they are, and take out everything
    else from Lesson 16: the keypad and its eight wires. Never connect the
    servo's red wire to the Mega's 5V pin or the top + rail. Keep fingers and
    hair away from the horn when it moves, and don't force it round by hand.

!!! danger "Never plug the power module into the breadboard"
    The pins underneath it are made for a breadboard's rails, but on this
    kit's breadboard they only fit the right way round at the end where the
    Mega is, and the Mega's own wires need that end. Turned round to fit the
    far end, its + pins would land in the − rails, with the Mega's GND: a
    short circuit. So it lies flat to the right of the breadboard, with
    nothing in those pins, and both its yellow jumpers **off**, each parked
    on one pin, where it joins nothing. Its power comes from the header in
    its middle instead: a red wire from its **5V** pin to the bottom + rail
    by column 42, and a black wire from its **GND** pin to the bottom − rail
    by column 42.

<!-- bench -->

<!-- steps -->

??? info "The power module"
    It takes 6.5 to 9 V from its barrel socket (the kit's 9 V adapter, or a
    9 V battery on a snap) and turns it into 5 V and 3.3 V, enough for a
    servo or two. The header in its middle has two rows of four pins: one
    row gives 3.3V, 3.3V, 5V, 5V, and every pin in the other is GND.
    That header gives both voltages whatever the jumpers say, so the
    jumpers can stay off. The two wires are female-to-male: their sockets
    go on the header's pins, and their pins into the rails. Its button
    switches it on, and its little LED lights when it is. It has a USB
    socket too; in this course, always power it through the barrel socket
    with the kit's adapter.

    The knob stands at its home, as in Lesson 7, its red wire from j41 up to
    the top + rail. That rail carries the Mega's own 5 V, from the red wire
    into the top + rail by column 3, and so does the screen's VDD: the Mega
    measures A0 against that 5 V, so a knob fed from it reads from 0 to 1023
    exactly, and the knob and the screen keep working with the power module
    switched off. The two 5 Vs never meet: the module's reaches only the
    bottom + rail, and the Mega's only the top one. Only their GNDs join, at
    the − rails.

When you are done, these are the connections your circuit makes:

<!-- connections -->

Now make the dial: cut a half circle from card, mark 0° at one end, 90° at
the top and 180° at the other end, and tape it behind the horn, its center on
the servo's shaft.

## Code it

Open **File → Examples → Adk → lessons → 017-servo**:

<!-- sketch -->

What's new:

- `adk::Servo needle {44};` names the servo on pin 44. Only pins 44, 45 and
  46 can drive a servo, because ADK makes the pulses with the Mega's Timer 5,
  in hardware, so they never wobble.
- `knob.read (0, 180)` turns the knob into an angle, as in Lesson 7.
- `refresh`, an `adk::Every` from Lesson 11, reads the knob and redraws the
  screen ten times a second. The servo keeps gliding between those readings.
- `target` remembers the last angle requested. It starts at -1, which means
  no angle has been chosen yet. After that, a new reading must differ by at
  least 2° to change the target; either end of the dial, 0° or 180°, always
  counts. Ignoring a one-degree wobble lets a glide finish instead of
  starting it again every time a noisy reading changes.
- `needle.moveTo (angle, 300);` asks the servo to glide to that angle over
  300 ms, while the sketch carries on. Asking again for the angle it is
  already gliding to changes nothing; a different target starts a new glide.
- Its sister `needle.write (angle);` jumps straight there, as fast as the
  servo can go.
- `needle.angle ()` is the angle the Mega is sending the servo now. During
  a glide it is how far the command has got. The servo has its own position
  sensor inside, but sends no position reading back to the Mega: the screen
  cannot tell whether the horn has followed the command.
- The screen is Lesson 13's `adk::Lcd`, with the degree sign and trailing
  spaces of Lesson 14.

## Upload it

Plug in the USB cable, then the power module's adapter, and press the module's
button so its LED lights. Upload the sketch. The servo swings to wherever the
knob points and stops, and the screen shows `Knob` and `Needle`, the angle
the Mega is currently sending. Turn the knob slowly: the needle follows.
Turn it quickly from one end to the other and stop: the command glides to
the new angle over 300 ms, and its number on the screen counts after the
knob's. A one-degree difference is allowed to keep a noisy reading from
restarting the glide; turning fully to either end always asks for 0° or 180°.

Now test your prediction. With the module switched off, the knob and the
screen still work, because they run on the Mega's own 5 V, and the screen
still shows the angle the Mega is sending. But the servo is limp: it has
the signal but no power to act on it. Keep this test short, because the
Mega's pulses still flow into the servo while it has no power. Switch the
module back on and the servo snaps to wherever the knob now points.

## If it doesn't work

| What you see | Try this |
|---|---|
| The servo never moves, though the screen shows the angles | Is the power module's LED on? Check the adapter and the button, then the module's two wires: red from its 5V pin to the bottom + rail by column 42, black from its GND pin to the bottom − rail by column 42. Then check the black wire from the Mega's GND to the bottom − rail by column 3: without it the servo can't read the signal. |
| The power module gets hot | Unplug its adapter and the USB cable at once. The module must lie beside the breadboard, never plugged into it, and its red wire must go to the bottom + rail by column 42 and its black one to the bottom − rail by column 42, never the other way round. |
| The screen is dark | It runs on the Mega's 5 V, not the module's: check the red wire from the Mega's 5V into the top + rail by column 3. |
| It moves, but not with the knob | Check the servo's orange wire goes to pin 44, the knob's middle leg to A0, and the red wire from j41 up to the top + rail by column 41. |
| The Mega resets or the USB disconnects when the servo moves | The servo is getting power from the Mega. Its red wire must go to the bottom + rail by column 35, fed by the power module. |
| The needle turns the opposite way to the knob | Nothing is wrong. To swap it, change `knob.read (0, 180)` to `knob.read (180, 0)`. |
| The servo hums or twitches when it should be still | Watch the knob's number. If it wobbles by 2° or more, check its wires; the second challenge tries a wider quiet zone. |
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
2. **A wider quiet zone.** Change `target - 1` and `target + 1` to
   `target - 3` and `target + 3`, so it takes a change of at least 4° to
   start a new glide. Predict how tiny turns of the knob will feel, then
   try it. Does the steadier needle make small adjustments harder? Put
   the two 1s back when you're done.
3. **Windshield wiper.** Forget the knob: make the horn sweep from 0° to 180°
   and back forever, using `needle.isMoving ()` to know when each sweep is
   done. Then let the knob set the speed.
4. **A real gauge.** Add the thermistor from
   [Lesson 14](../014-thermometers/index.md) on A2, and make the needle point
   to the temperature on a dial marked from 15 °C to 35 °C.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it to DC volts as in [Lesson 1](../001-blink/index.md#measure-it), black lead
in **COM** and red in **V**. The servo holds its angle and the knob stays
where you leave it, so the sketch needs no change. Keep your fingers clear
of the horn, and take care on the rails: a probe tip touching the + and −
rails together would short the power module.

!!! question "Predict"
    The servo's 5 V and the knob's 5 V come from two different places. When
    you switch the power module off, which of them will drop? Write down
    your guess, then take the first two readings with the module on, and
    again, briefly, with it off.

<!-- measure -->

What the numbers tell you:

- **The servo's 5 V** comes from the power module's own regulator. Switch
  the module off and it falls far below 5 V, and the servo goes limp. It
  may not reach 0 V, because the Mega's pulses can leak a little through
  the servo onto the rail. Switch the module back on once you have the
  reading.
- **The knob's 5 V** comes from the Mega, which gets it from your computer's
  USB port, down the top + rail, so it stays with the module off. That's
  why the knob and the screen still work. Compare it with the first
  reading: the two are seldom exactly equal. Joined, the higher would push
  current back into the other, which is why the module's red wire goes
  only to the bottom + rail: the Mega's 5 V has the top rails and the
  module's the bottom ones. Only their GNDs are joined, so that the servo
  can read the pulses.
- **The knob's wiper** is the angle, as a voltage. `knob.read (0, 180)`
  turns 0 V into 0° and 5 V into 180°, so 90° is 2.5 V, and each degree is
  5 V ÷ 180 ≈ 0.03 V. Turn the knob and watch the needle and the meter move
  together.
