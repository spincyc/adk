---
lesson: 20
promise: Spin a fan at any speed, either way round, with a driver chip doing the heavy lifting.
time: 1 hour
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Breadboard power module and its 9 V adapter
  - L293D motor driver chip
  - DC motor with its fan blade
  - 10 kΩ potentiometer
  - Push button
  - 13 jumper wires
ideas:
  - Why a motor never runs from a pin
  - The H-bridge, which turns a motor either way
  - PWM on the enable pin sets the speed
  - A separate supply for the motor, with a shared GND
---

## What you'll build

<!-- closeup -->

A small fan on your desk that you drive like a model car. Turn the knob and
it wakes up, slow at first, then faster until it's roaring; press the button
and it slows, pauses for a moment, and spins the other way, blowing air
back at itself. The Mega does the thinking, but the power comes from a
separate supply, through a chip built to handle motors.

## The idea

**Why not just use a pin?** A Mega pin can give about 20 mA. The kit's
little motor wants around 200 mA when it's spinning, and several times
that for the instant it starts. Ask a pin for ten or twenty times its limit
and it will be damaged, or the Mega will reset. A motor is also a coil of
wire, and the current in a coil tries to keep flowing: when it's switched
off it sends a kick of voltage back down its wires. Motors never connect to
a pin directly. They go through a
**driver** and take their power from their own supply, here the breadboard
power module.

**The H-bridge.** To run a motor both ways, you need to be able to connect
either of its leads to +5 V and the other to GND. Four switches do it, drawn
round the motor like the letter H:

<p class="formula">close the top-left and bottom-right switches: current flows one way → forward<br>
close the top-right and bottom-left switches: current flows the other way → backward</p>

The **L293D** holds two of these H-bridges, made of transistors, plus the
diodes that soak up the motor's kick. Its two halves are identical; this
build uses the half along the chip's top row. Two of its inputs choose the
direction, and its **enable** input switches that half on or off:

| Forward (pin 8) | Backward (pin 9) | Enable (pin 4) | The motor |
|---|---|---|---|
| HIGH | LOW | on | spins forward |
| LOW | HIGH | on | spins backward |
| LOW | LOW | on | brakes: both leads are joined to GND, so it stops quickly |
| any | any | off | coasts to a stop |

**Speed.** The enable pin gets PWM, as the dimmer's LED did in Lesson 7. A
speed of 128 out of 255 switches the motor on about half the time, a
thousand times a second, and the motor's weight smooths that into half
power. Below about 100 the kit's motor only hums: it hasn't enough push to
get going.

**Two supplies, one GND.** The power module feeds only the bottom + rail,
and the chip's VCC2 pin takes the motor's power from there, so the big
currents never go near the Mega. The top + rail carries the Mega's own
5 V: to the chip's VCC1, which powers the part of the chip that listens
to the Mega, and to the knob, which reads from 0 to 1023 as it did in
Lesson 7. The Mega's GND is joined to the module's GND, or the Mega's
"HIGH" would mean nothing to the chip.
The chip keeps a volt or two for itself, so the motor sees about 3 V of
the module's 5 V: plenty for this 3–6 V motor.

!!! question "Predict"
    The fan is spinning fast and you press the button. Does it flip round
    instantly, or will you see something happen in between?

## Build it

!!! warning "Unplug first"
    Unplug the USB cable, unplug the power module's adapter and switch the
    module off before you change any wiring. Set the module's bottom yellow
    jumper to **5V**, never 3.3V, and its top one to **OFF**: the Mega's 5V
    feeds the top rails, and the two supplies must never be joined. The
    chip's notch (the little half moon at one end) faces left, towards the
    Mega. Keep fingers and hair clear of the fan blade whenever the power is
    on.

<!-- bench -->

<!-- steps -->

??? info "The L293D's pins, all sixteen"
    Seen from above with its notch on the left, pin 1 is at the bottom left
    and the numbers run anticlockwise, along the bottom row and back along
    the top:

    | Pin | Name | Here | Pin | Name | Here |
    |---|---|---|---|---|---|
    | 1 | 1,2EN | not used | 16 | VCC1 | 5 V for the chip itself |
    | 2 | 1A | not used | 15 | 4A | backward, from pin 9 |
    | 3 | 1Y | not used | 14 | 4Y | the motor's black lead |
    | 4, 5 | GND | to the − rail | 13, 12 | GND | joined inside |
    | 6 | 2Y | not used | 11 | 3Y | the motor's red lead |
    | 7 | 2A | not used | 10 | 3A | forward, from pin 8 |
    | 8 | VCC2 | 5 V for the motor | 9 | 3,4EN | enable, from pin 4 |

    The four GND pins in the middle are joined inside the chip, and carry
    its heat away, so one wire grounds them all. A high on 3A puts the
    supply on 3Y, so forward puts + on the motor's red lead. The library's
    reference names the chip's other half (pins 1 to 7); either half works
    the same.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson20Fan**:

<!-- sketch -->

What's new:

- `adk::Motor fan {4, 8, 9};` is the motor and its half of the driver:
  the enable pin, then the forward pin, then the backward pin.
- `fan.speed (s)` takes a number from −255 to 255. Positive is forward,
  negative backward, and 0 lets it coast. Multiplying the speed by
  `direction`, which is 1 or −1, is all it takes to reverse: pressing the
  button turns 1 into −1 and back.
- `knobSpeed ()` is the knob from Lesson 7, read as how far round it is
  turned, from 0 to 100. For the first tenth of the turn the `?:` gives 0,
  so the fan stays still; after that `map ()` turns 10–100 into speeds from
  `slowest`, 100, up to 255, skipping the range where the motor only hums.

## Upload it

1. Plug in the USB cable and upload the sketch.
2. Plug the adapter into the power module and press its switch: its small
   LED lights.
3. Turn the knob. For the first tenth of its turn the fan stays still, then
   it starts, slowly, and speeds up as you keep turning.
4. Press the button. The fan slows, stops for a moment, and spins the other
   way, blowing the air backwards. Press it again to swap back.

You predicted what a press does to a fast fan. It doesn't flip round at
once: ADK lets it coast for half a second, slowing right down, before it
drives it the other way. Watch the blade: it slows, nearly stops, then
speeds up backwards.

Switch the power module off when you finish, before you unplug the USB.

## If it doesn't work

| What you see | Try this |
|---|---|
| Nothing spins at all | Is the power module's LED on, with its bottom jumper on 5V? Check the red wire from a19 to the bottom + rail (the motor's supply) and the one from j12 to the top + rail (the chip's). |
| It hums but doesn't turn | The speed is too low: turn the knob further. A flick of the blade helps a sluggish motor start. |
| It only ever spins one way | The wire from pin 8 or pin 9 is in the wrong hole: pin 8's goes to j18, pin 9's to j13. |
| The Mega resets when the fan starts | The motor is taking power from the Mega. Its supply goes from a19 to the bottom + rail, which only the power module feeds; and the module's top jumper must be off, so its 5 V never meets the Mega's. |
| The chip gets hot | Unplug everything at once and check the motor's leads go to j14 and j17, not straight to a rail. |
| The button does nothing | The button straddles the middle gap; pin 22's wire goes in j2 and the black wire from a4 to the − rail. |
| The **L** LED blinks long and short flashes | ADK found a problem with a pin. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    `fan.speed ()` sets the two direction pins and then writes the speed to
    the enable pin with `analogWrite ()`. Pin 4 makes PWM at about 980
    pulses a second, fast enough that the motor feels an average.

    Reversing a spinning motor at once would briefly draw about twice its
    stall current. So when the direction changes, ADK switches the motor
    off, lets it coast for half a second while your sketch carries on,
    long enough for a small fan to slow right down, and only then drives
    it the other way. That is the pause you saw.

## Make it yours

1. **Emergency stop.** Add a second button on pin 23 that calls
   `fan.brake ()`, which stops the motor faster than coasting by shorting
   its leads together through the chip. `loop ()` sets the speed on every
   pass, so keep a `bool stopped`, and leave the speed alone while it's
   true, until the knob is turned right down.
2. **Gentle start.** Instead of jumping to the knob's speed, creep towards
   it: on each beat of an `adk::Every {20}`, move the fan a few steps from
   `fan.speed ()`, the speed it has now, towards the knob's.
3. **Breeze.** Make the fan blow forwards for ten seconds and backwards for
   ten, like an oscillating fan: turn `direction` round on each beat of an
   `adk::Every {10000}` instead of on a press.
4. **Automatic fan.** Add the ultrasonic sensor from Lesson 19 and switch
   the fan on only when someone is within 60 cm. The next lesson takes
   this much further.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts, the black
lead in **COM** and the red one in **V**. Never use the **10A** jack here: it
joins the two probes, and across the power module's rails that is a short
circuit.

First make the fan safe to work beside. Pull the blade off the motor's
shaft: the motor turns just the same without it, and there is nothing left
to catch your fingers or the meter's leads. If you keep the blade on, keep
your fingers, the probes and their leads well out of its circle. Then give
the fan one slow, fixed speed, so the readings hold still: in `loop ()`,
change `knobSpeed ()` to `128` and upload. Put it back when you have
finished. The probes go into holes close beside the chip, so keep each tip
in its own hole: a tip across two holes joins them.

!!! question "Predict"
    At a speed of 128 out of 255, what will the meter read on the enable
    pin, 4? And across the motor, what will change when you press the
    button?

<!-- measure -->

What the numbers tell you:

- **The enable pin** reads about 2.5 V. PWM switches it between 5 V and 0
  about a thousand times a second, on for 128 parts in every 255: half the
  time. The meter can't follow that, so it shows the average, half of 5 V.
  Try `200` in place of `128`: 200 ÷ 255 × 5 V ≈ 3.9 V. The meter shows the
  speed in the code as a voltage.
- **The forward pin** reads about 5 V while the fan runs forwards, and pin
  9 reads 0: the first row of the table in *The idea*. Press the button:
  both read 0 for half a second while the fan coasts, then pin 9 reads 5 V
  and pin 8 stays at 0.
- **Across the motor** needs the enable pin on all the time, so change
  `128` to `255` for this one. The red probe goes beside the motor's red
  lead and the black probe beside its black one. The meter reads about
  3 V, not 5: the chip keeps a volt or two for itself, and forward puts +
  on the red lead. Now press the button. After the pause it
  reads about −3 V. The minus sign means the chip has swapped which of the
  motor's leads gets the supply and which gets GND, so the current runs
  through the motor the other way: that is the H-bridge at work.
