---
lesson: 21
promise: Build a little turret that looks around, finds the nearest person and turns a fan on them.
time: 1½ hours
level: 3
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Breadboard power module and its 9 V adapter
  - SG90 servo and its horns
  - HC-SR04 ultrasonic sensor
  - L293D motor driver chip
  - DC motor with its fan blade
  - 11 jumper wires
  - 6 female-to-male jumper wires
  - Sticky tape or putty, and a strip of card
ideas:
  - Scanning with a sensor on a servo
  - Finding the nearest thing in a sweep
  - A machine that works in steps
  - A function that answers with a struct
  - Two motors sharing one supply
---

## What you'll build

<!-- closeup -->

A turret that keeps watch. The servo swings the ultrasonic sensor slowly
from side to side, like a lighthouse, measuring as it goes. When the sweep
is done it swings back to point at whatever was nearest, the fan on the
turret spins up, and it blows at you: harder the closer you are. Step
aside and it goes quiet, sweeps again, and finds you in your new spot.

It brings together the servo from Lesson 17, the sensor from Lesson 19 and
the fan from Lesson 20.

## The idea

**A sensor on a servo sees in more than one direction.** On its own, the
HC-SR04 measures along one narrow beam, about 15 degrees wide. Mount it on
the servo's horn and step it from 30° to 150° in 10° steps, taking a reading
at each, and you get 13 readings across a wide arc: a rough picture of what
is around, like a bat or a ship's sonar.

**Finding the nearest.** The sketch keeps a note as it sweeps, called a
**sighting**: the smallest distance so far, and the angle where it saw it.
Each new reading is compared with the note; if it's smaller, the note is
replaced. At the end, the note holds the answer. If even the nearest thing
is beyond 80 cm, it doesn't count as "somebody": a far wall is ignored.

| Angle | 30° | 40° | 50° | 60° | 70° | … |
|---|---|---|---|---|---|---|
| Reading | none | 72 cm | 41 cm | 38 cm | 55 cm | … |
| Nearest so far | none | 72 cm at 40° | 41 cm at 50° | **38 cm at 60°** | 38 cm at 60° | … |

**Speed from distance.** Once aimed, the fan's speed comes from the
distance, from full speed (255) at 15 cm down to 110 at 80 cm:

<p class="formula">speed = map (distance, 15, 80, 255, 110)</p>

So someone 40 cm away gets a speed of about 200. Closer than 15 cm the fan
stops altogether: nobody wants a fan blade in their face.

**One supply, two motors.** The servo and the fan both draw their power
from the breadboard power module's bottom rails, while the Mega's own 5 V
feeds the sensor and the chip's logic, as in Lesson 20. The module can
give about 700 mA. A servo in motion can take several hundred milliamps,
and so can a starting motor. So the fan rests whenever the turret turns,
and the turret keeps still while the fan blows: the two never pull hard
together.

!!! question "Predict"
    Stand 40 cm in front of the turret, with a wall 60 cm from it off to
    one side. Which will the turret turn to, and why? What happens if you
    walk away while it's blowing?

## How the fan works

The sketch repeats three steps for as long as it runs:

| Step | What happens | Moves on when |
|---|---|---|
| **Sweep** | The fan rests. The turret steps from 30° to 150°, pausing at every 10° to take a fresh reading, and remembers the nearest thing. | The sweep ends. If nothing was within 80 cm, it waits a second and sweeps again. |
| **Aim** | The turret turns back to the nearest thing's angle, gently, 8 ms per degree. | It arrives. |
| **Blow** | It keeps measuring, and sets the fan's speed from the distance. | 4 seconds pass, or the target moves out of reach or closer than 15 cm. Then the fan stops, and it's back to Sweep. |

## Build it

!!! warning "Unplug first"
    Unplug the USB cable, unplug the power module's adapter and switch the
    module off before you change any wiring. Keep its jumpers as in
    Lesson 20: the bottom one on **5V**, the top one **OFF**. The servo and
    the fan take their power only from the module, never from the Mega.
    Keep fingers, hair and faces clear of the fan blade whenever the power
    is on.

This is Lesson 20's fan circuit without the knob and the button, plus the
servo and the sensor. Build it on the breadboard first, then mount the
sensor and the fan on the servo.

<!-- bench -->

<!-- steps -->

??? info "Making the turret"
    Fit the longest horn on the servo and turn it by hand, gently, to see
    where its travel ends; the middle of its travel is 90°. Tape a strip of
    card across the horn, then stick the sensor on it facing forwards, and
    the motor beside it with the fan blade pointing the same way and clear
    of everything when it spins. Stand the servo on the desk with putty or
    tape so it can't walk.

    The motor's own leads won't reach from the horn down to the
    breadboard, so lengthen each one with a female-to-male jumper wire:
    push the lead's end into the wire's female end, and the male end into
    the hole the lead went in, black's in j14 and red's in j17.

    Leave the jumper wires slack, so the turret can swing all the way
    without tugging on them, and check the blade can't touch a wire. The
    SG90 can carry the sensor and the motor easily if they sit close to the
    middle of the horn.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 021-follow-me-fan**:

<!-- sketch -->

Read it from the top:

- Three parts, `turret` (the servo), `sensor` and `fan`, and `blowing`, an
  `adk::Timer` as in Lesson 3. The constants below them are the numbers to
  play with: the sweep's ends and step, the safety distance, the reach and
  how long it blows.
- `struct Sighting` is the note from the table above: an angle and a
  distance, kept together.
- `loop ()` is the three steps: `sweep ()`, then, if the nearest thing is
  within reach, `turnTo ()` its angle and `blow ()`. Each step finishes
  before the next begins, so the loop reads like the table.
- `Sighting sweep ()` is a function that answers with a whole sighting,
  both numbers at once. The note starts at 400 cm, which is what
  `measure ()` gives when no echo comes back, as in Lesson 19, so anything
  the sensor really sees is nearer. Then a `for` loop steps over the
  angles, and `nearest = {angle, cm};` replaces both halves of the note
  together.
- `blow ()` starts the timer, then measures and sets the speed for as long
  as `blowing.isRunning ()`. `break` leaves the `while` loop early, just as
  it leaves a `switch`, when the target walks out of reach or comes too
  close. Either way, the fan stops at the end.
- `turnTo ()` glides with `turret.moveTo ()` from Lesson 17, taking 8 ms per
  degree it has to travel: slow enough that the sensor and motor on the
  horn don't swing about. `abs ()` gives a number without its sign, so a
  turn of −40° or 40° takes the same 320 ms. `adk::wait ()` then waits for
  the glide and 50 ms more, for the turret to settle and old echoes to die
  away.
- `measure ()` calls `adk::update ()` until `sensor.measured ()` says a brand
  new reading has arrived. `do { ... } while (...)` is a `while` loop that
  asks its question at the end, so it always updates at least once: the
  reading can never be one that `measure ()` already handed back.

## Upload it

1. Plug in the USB cable and upload the sketch.
2. Plug in the power module's adapter and switch it on.
3. The turret swings to 30° and starts its sweep, stepping steadily to 150°,
   which takes a little over two seconds.
4. Stand in front of it, 30 to 60 cm away. At the end of the sweep it turns
   to face you and the fan spins up. Lean closer and it blows harder; lean
   in closer than 15 cm and it stops.
5. Step to one side. Within a few seconds it stops, sweeps, and turns to
   your new spot.

With nobody in reach, it sweeps, waits a second, and sweeps again.

You predicted which it would turn to, you or the wall. It turns to you:
the sweep keeps only the nearest thing it saw, and at 40 cm you are nearer
than the wall at 60. Walk away while it's blowing, and once the sensor sees
nothing within 80 cm, the fan stops. The turret sweeps again and turns to
the nearest thing left: the wall, which is within reach, so now it blows
at the wall.

## If it doesn't work

| What you see | Try this |
|---|---|
| The servo doesn't move | Is the power module on? Check the servo's plug: brown to the − rail, red to the + rail, and orange straight to pin 44. |
| It sweeps but never stops to blow | It never saw anything within 80 cm. Stand closer, or check the sensor still faces forwards on the horn. |
| It turns to the wrong place | The sensor sees a wall, the desk or a wire that's nearer than you. Clear the space in front of it, or tilt the sensor up a little. |
| The fan never spins | Check the power module's bottom jumper is on 5V, and that the motor's leads, through their extra wires, reach j14 and j17. Lesson 20's table has more. |
| The Mega resets or the servo jerks when the fan starts | The supply is struggling: make sure the servo's red wire goes to the bottom + rail, which only the module feeds, and that the Mega's GND is joined to the module's. |
| The turret twitches at the ends of its sweep | Some servos can't reach 30° or 150°. Try `leftmost = 40` and `rightmost = 140`. |
| The **L** LED blinks long and short flashes | ADK found a problem with a pin. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    The servo's pulses come from the Mega's Timer 5, which makes them in
    hardware, so they stay steady even while the sensor holds everything
    up for up to 25 ms timing an echo. `turret.moveTo ()` glides by moving
    the pulse a little further on every `adk::update ()`, which is why
    `turnTo ()` waits with `adk::wait ()`, which keeps updating, and never
    with `delay ()`.

    `turret.angle ()` reports where the servo has been told to be, which is
    how `turnTo ()` knows how far it has to go.

## Make it yours

1. **Wider or finer.** Change `step` to 5 degrees for a finer sweep. How
   much longer does a sweep take? Is it worth it?
2. **Sweep both ways.** Sweep from left to right, then right to left, so
   the turret never has to fling itself back to the start. A second `for`
   loop can count down, with `angle -= step`.
3. **Follow closely.** Instead of a full sweep, after blowing check just
   three angles with `turnTo ()` and `measure ()`: 10° either side of the
   last target, and the target itself. The turret will track you as you
   walk slowly past.
4. **Show off.** Add the LED gauge from Lesson 19 so the lights show how
   close the target is while the fan blows. The chip sits in two of the
   LEDs' home columns, so lay the three out past it, in columns 24, 30
   and 36.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts, the black
lead in **COM** and the red one in **V**. Never use the **10A** jack here: it
joins the two probes, and across the power module's rails that is a short
circuit.

The fan's speed comes from a distance, and the meter can watch that happen
on the enable pin, 4. To give it time, change `blowTime` to `20000` and
upload, so the fan blows for 20 seconds at a time. Stand a book up on the
desk 70 cm in front of the turret, with nothing else nearer, and work from
beside the breadboard. The fan blows at the book, and the turret swings it
round at every sweep, so keep your fingers, the probes and their leads out
of its reach. Keep each probe tip in its own hole: a tip across two holes
joins them.

!!! question "Predict"
    The sketch sets the speed with `map (distance, 15, 80, 255, 110)`. What
    will the meter read with the book at 70 cm, and then at 40 cm?

<!-- measure -->

What the numbers tell you:

- **At 70 cm** the sketch works out a speed of 133, and PWM gives the pin
  133 ÷ 255 × 5 V ≈ 2.6 V: the fan is switched on a little over half the
  time.
- **At 40 cm** it works out 200, and the meter reads about 3.9 V: nearer,
  so the fan is on for more of the time, and blows harder. Try other
  distances between 15 and 80 cm, and work out what the meter should say
  before you look.
- **During a sweep** the reading drops to 0. The fan rests whenever the
  turret turns, so the servo and the motor never pull hard on the power
  module together.
