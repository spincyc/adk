---
lesson: 23
promise: Set four invisible tripwires that notice movement, an obstacle, a tilt and a broken beam.
time: 1 hour
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - HC-SR501 PIR motion sensor
  - Obstacle-avoidance sensor module (37 in 1 kit)
  - Tilt ball switch
  - Photo-interrupter (beam-break) module (37 in 1 kit)
  - Red, yellow, green and blue LEDs
  - 4 × 220 Ω resistors (red, red, black, black, brown)
  - 13 jumper wires
  - 9 female-to-male jumper wires
ideas:
  - On/off sensor modules
  - Active-high and active-low outputs
  - Why a bare switch needs the pull-up and a module doesn't
  - adk::Switch and its polarity
---

## What you'll build

<!-- closeup -->

Four guards, each with its own light. Walk past the PIR sensor and the red
LED comes on. Put your hand in front of the obstacle sensor and the yellow
one lights. Tip the tilt switch over and green shows. Slide a piece of card
into the slot of the beam-break sensor and blue lights while it's there.
They make the tripwires for next lesson's room alarm, and each one teaches
you something about how a sensor says "yes".

## The idea

**A module does the sensing for you.** Each of these sensors has its own
little circuit board that does the hard part: noticing warmth moving, or
reflected infrared, or a beam of light being cut. It hands the Mega just
one signal: a plain HIGH or LOW, like a button. That's an **on/off
sensor**.

**But "yes" isn't always HIGH.** Each module decides for itself which level
means it has noticed something:

| Sensor | Notices | Its output when tripped | Called |
|---|---|---|---|
| PIR (HC-SR501) | warm things moving, like people | HIGH (3.3 V, which counts as HIGH) | active high |
| Obstacle module | infrared bouncing back off something near | LOW | active low |
| Beam-break module | its light beam being blocked | HIGH, on most boards | active high |
| Tilt ball switch | a tiny metal ball rolling off its contacts | (no output of its own) | a plain switch |

Active low looks backwards, but it's common: many modules pull their output
down to GND to say "yes", and let a resistor hold it up the rest of the
time.

**A bare switch needs the pull-up.** The tilt switch has no circuit board:
just two legs and a metal ball that joins them while the switch stands
upright. Joined, it connects its pin to GND, reading LOW; tipped over, it
connects the pin to nothing at all. A pin connected to nothing floats and
reads whatever it likes, so, as with the buttons in Lesson 2, the Mega's
own **pull-up** resistor (somewhere between 20 and 50 kΩ inside the chip)
holds it at 5 V until the switch pulls it down.

**`adk::Switch` handles all of it.** You tell it the pin and which level
means active:

- `adk::Switch obstacle {A13};` is **active low**, the default. ADK turns
  the pull-up on, which suits a switch or a module that pulls down.
- `adk::Switch motion {A12, adk::ActiveHigh};` is **active high**, with no
  pull-up, for a module that drives its output both ways itself.

Then `isActive ()` is simply true while the sensor says "yes", whichever
level that is, and `activated ()` is true for the one moment it changes.

!!! question "Predict"
    The tilt switch is `adk::Switch upright {A14};`, active while it stands
    upright. The sketch lights the green LED with
    `green.set (!upright.isActive ())`. With the switch standing up, is the
    green LED on or off? What about lying on its side?

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring. Match each module's
    pins by the names printed on it, not by where they sit: modules from
    different makers put them in different orders. The PIR's names are
    printed under its white dome: pull the dome gently off to read them,
    then press it back. The obstacle module may have a fourth pin marked
    **EN**: leave it unconnected. Each LED stands as in Lesson 19, its long
    leg in row b of its resistor's column.

<!-- bench -->

<!-- steps -->

??? info "Where the modules get their power"
    The PIR sits below the Mega and takes its 5 V and GND straight from the
    Mega's power header. The beam-break and obstacle sensors take theirs
    from the bottom rails beside them. The bottom + rail gets its 5 V from
    the top one, through the red wire from T+61 to B+61 at the far end.

??? info "Setting up the PIR and the obstacle module"
    The PIR has two orange knobs. Turn the **time** knob (often marked Tx)
    fully anticlockwise, so its output stays on for only about 3 seconds
    after the last movement; leave the **sensitivity** knob (Sx) in the
    middle. If it has a jumper with **L** and **H** beside it, put it on
    **H**, so that continued movement keeps the output on.

    The obstacle module has a small blue knob that sets how far it can see,
    and an LED of its own that lights when it sees something. Hold your
    hand about 10 cm away and turn the knob until that LED just comes on.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson23Tripwires**:

<!-- sketch -->

What's new:

- Four `adk::Switch` lines, one for each tripwire. The comments say which
  level each one gives when it's tripped, and that decides whether it's
  `adk::ActiveHigh` or left as the active-low default.
- `loop ()` sets each LED from its sensor's `isActive ()`. The tilt switch
  is the odd one out: it is active while *upright*, so its LED lights when
  it is **not** active, which is what the `!` means.
- `announce ()` prints a line to the Serial Monitor at the moment each
  tripwire goes off, using `activated ()`, or `deactivated ()` for the tilt
  switch, the moment it stops being upright.

## Upload it

Upload the sketch and open the Serial Monitor at 9600 baud.

!!! tip "Give the PIR a minute"
    After power-up, the PIR sensor takes up to a minute to settle, and may
    trip by itself a few times while it does. Keep still, or keep out of its
    view, until the red LED has stayed off for a while.

Then try each tripwire in turn:

- **Movement:** wave your arm in front of the PIR. The red LED lights and
  "Movement!" prints; keep still and it goes out a few seconds later.
- **Obstacle:** hold your hand close in front of the obstacle module. The
  yellow LED lights with the module's own LED, and "Something in front!"
  prints.
- **Tilt:** tip the tilt switch on its side. The green LED lights and
  "Tilted!" prints; stand it up again and it goes out.
- **Beam:** push a strip of card into the photo-interrupter's slot. The blue
  LED lights while the beam is blocked, and "Beam broken!" prints.

You predicted the green LED with the tilt switch standing up and lying
down. Standing up, the ball joins the switch's legs, so `upright` is
active, and the `!` turns that into false: green is **off**. On its side,
`upright` is not active, `!` makes it true, and green is **on**. So the
LED shows a tilt, even though the switch says "upright".

## If it doesn't work

| What you see | Try this |
|---|---|
| An LED is on while nothing is happening, and goes **off** when you trip it | That module is the other way round from most. Swap `adk::ActiveHigh` in or out of its line in the sketch. |
| The red LED keeps coming on by itself | Give the PIR a minute to settle, keep warm air and sunny windows out of its view, and turn its sensitivity knob down a little. |
| The red LED stays on for ages | Turn the PIR's time knob fully anticlockwise. |
| Yellow never lights, though the obstacle module's own LED does | Check its OUT pin goes to A13, not EN. |
| The obstacle module's own LED never lights | Check its + and GND reach the bottom rails, then turn its knob to see further. |
| Green never changes | Check A14's wire and the tilt switch's other leg's wire to the − rail. Tip it right over: some switches need more than a tilt. |
| Blue is always on or never on | Check the beam module's pins against their labels; + and S swapped is a common slip. |
| The **L** LED blinks long and short flashes | ADK found a problem with a pin. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    An `adk::Switch` reads its pin on every `adk::update ()`, and only
    believes a change once the pin has held its new level for 20 ms, the
    same debouncing as a button's. A tilt switch's ball rattles just as a
    button's contacts bounce.

    Active low asks for the pin's pull-up when the pin is claimed; active
    high leaves it off. A module that drives its own output isn't bothered
    either way, but a bare switch without a pull-up would float, flickering
    between HIGH and LOW.

## Make it yours

1. **A magnet tripwire.** Swap the beam-break module for the reed switch or
   Hall sensor module from the 37 in 1 kit, and trip it with a magnet. Is
   yours active high or active low? Find out by watching its LED.
2. **Keep score.** Count how many times each tripwire goes off, and print
   the four totals with one `adk::println ()` every time one changes.
3. **Chime.** Add the active buzzer on pin 12, as in Lesson 3, and give a
   short beep whenever any tripwire goes off.
4. **Latch.** Make each LED stay on once its tripwire has gone off, until
   you press a button on pin 22 to reset them all: just like a real alarm
   panel shows where the trouble was.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts, the black
lead in **COM** and the red one in **V**.

The modules' signal wires run straight to the Mega, with no hole for a
probe, but the tilt switch stands in the breadboard, where the meter can
show the pull-up at work. Tip the switch over on its legs, or stand it
up, before you put the probes in place: it stays where you leave it.

!!! question "Predict"
    Standing up, the tilt switch's ball joins its legs. Which way up will
    its pin, A14, read 5 V, and which way 0?

<!-- measure -->

What the numbers tell you:

- **Upright**, the pin reads 0 V. The ball joins it straight to the − rail,
  and the pull-up inside the chip, tens of thousands of ohms, can only push
  a trickle through it, 0.25 mA at the very most, as in Lesson 2: far too
  little to lift the pin.
- **Tipped over**, it reads about 5 V. Now nothing pulls the pin down, and
  the pull-up holds it up. So for this switch 0 V means active (upright)
  and 5 V means not active: that is what active low means.
- **The bottom rails** read about 5 V, though the Mega's 5 V goes only into
  the top + rail, at T+3. The red wire from T+61 to B+61 at the far end
  carries it round to the bottom rails, where the beam-break and obstacle
  sensors take their power.
