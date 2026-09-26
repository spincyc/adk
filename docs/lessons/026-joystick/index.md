---
lesson: 26
promise: Steer a dot around the matrix with a thumb stick and draw with it.
time: 45 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - The LED matrix and button from Lesson 25
  - Joystick module
  - 10 female-to-male jumper wires
  - 3 jumper wires
ideas:
  - A joystick is two knobs at right angles
  - Reading an axis from −100 to 100
  - The dead zone in the middle
  - Directions for games
---

## What you'll build

<!-- closeup -->

An etch-a-sketch you can hold in your hand. Push the stick and a dot glides
across the matrix, leaving a glowing trail behind it: nudge it and it
creeps, push it all the way and it races. Click the stick down to lift the
pen and move without drawing, and press the button to wipe the picture
clean.

## The idea

Take the potentiometer from Lesson 7 and turn it into a stick: tilting the
stick turns the knob. Now put a second knob underneath at right angles.
That's a joystick. One knob follows left and right, the other up and down,
and each makes a voltage from 0 to 5 V that an analog pin reads as 0 to 1023.
Let go and springs pull the stick back to the middle, near 512 on both.

Under the stick is a push switch too: press the stick straight down and it
clicks. It works like any button, so ADK reads it as an `adk::Button`.

ADK turns each axis into a percentage. `joystick.x ()` goes from −100, all
the way left, through 0 in the middle, to 100, all the way right.
`joystick.y ()` does the same from down to up. ADK records where the stick
rests when the sketch starts and measures from there, so keep your fingers
off it for that moment.

A released stick never comes back to quite the same place: 509 one time,
517 the next. If every little wobble counted, the dot would drift on its
own. So ADK has a **dead zone**: anything within 10 of the middle reads 0.
The stick has to move a tenth of its travel before anything happens.

For games you often want *which way*, not *how far*. `joystick.direction ()`
answers Up, Down, Left, Right or Center. It picks the axis pushed furthest,
once it passes 50, and holds on to that direction until the stick falls back
below 30. The gap between 50 and 30 stops a stick resting near halfway from
flickering between two answers, like the two thresholds of the comfort light
in Lesson 15.

In this sketch the dot's speed comes from how far you push. Every 20
milliseconds the dot moves `joystick.x () / 4` hundredths of a dot, so at
full push, 100 / 4 = 25 hundredths each step, it crosses one dot every four
steps: about 12 dots a second.

!!! question "Predict"
    If you push the stick halfway to the right, so `joystick.x ()` reads
    about 50, how many dots a second will the pen move? Work it out, then
    time it against the eight dots across the matrix.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you wire. If you still have Lesson 25 built,
    keep the matrix as it is. The joystick's own switch takes pin 22, so the
    button moves to its place for pin 23, in columns 8 to 10, with its wire
    from pin 23 and its ground from the bottom − rail, as the steps show.

<!-- bench -->

<!-- steps -->

??? info "Which way round do you hold it?"
    Hold the joystick with its row of pins pointing to your **left**. Then
    right is positive x and up, away from you, is positive y, and the dot
    goes where you push. The drawing shows the pins pointing up only so the
    wires are easy to follow; your jumpers are long enough to hold it
    comfortably.

    The module's pins are labeled GND, +5V, VRx, VRy and SW. VRx and VRy are
    the two knobs' middle legs, the wipers.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 026-joystick**:

<!-- sketch -->

What's new:

- `adk::Joystick joystick {A3, A4};` names the stick and the analog pins for
  its x and y knobs.
- `adk::Button stick {22};` is the switch under the stick. It is a plain
  button, wired from pin 22 to GND inside the module.
- `penX` and `penY` hold the pen's place in **hundredths of a dot**, from 0
  to 799. `penX / 100` throws the hundredths away and leaves the dot, 0 to 7.
  Keeping the hundredths lets a gentle push add up slowly, instead of being
  lost.
- `movePen ()` runs on every tick of `step`, 50 times a second. It adds
  `joystick.x () / 4`, and takes away `joystick.y () / 4` because the
  matrix counts y downwards while the stick counts it upwards.
  `constrain (value, 0, 799)` keeps the pen on the matrix, as it kept
  readings in range in Lesson 8.
- `drawn` remembers whether the picture has a dot under the pen. When the
  pen moves on, `matrix.set (oldX, oldY, drawn)` puts back exactly what was
  there, and `matrix.get ()` reads the dot the pen arrives on.
- With the pen down, the dot under it stays lit and becomes part of the
  picture: `drawn = drawn || penDown` makes sure of that when you put the
  pen down. With the pen up, `blink` switches `penShown` on and off every
  200 milliseconds, so you can see where you are without drawing.

## Upload it

Upload the sketch without touching the stick. A single dot lights near the
middle of the matrix. Push the stick: the dot moves that way and leaves a
trail. A small push creeps, a big push races. Click the stick straight
down: the dot starts to blink, and now it moves without drawing. Click again
to draw. Press the button and the picture vanishes, leaving just the pen.

You predicted the speed at half a push. `joystick.x ()` reads 50, and
`50 / 4` is 12 (whole numbers drop the half), so the pen moves 12 hundredths
of a dot on each of the 50 steps a second: 600 hundredths, or 6 dots a
second. That is half the full speed, and it crosses the eight dots in a
little over a second.

## If it doesn't work

| What you see | Try this |
|---|---|
| The dot creeps away on its own | The stick was touched while the sketch started, so ADK learned the wrong middle. Press the Mega's reset button with your hands off the stick. |
| The dot goes the wrong way | Hold the joystick with its pins pointing left. If up and down still swap with left and right, VRx and VRy are swapped: A3 goes to VRx. |
| The dot doesn't move at all | Check +5V goes to 5V and GND to GND; without power both axes read 0. |
| Clicking the stick does nothing | Press the stick straight down until it clicks, and check SW goes to pin 22. |
| The button doesn't clear | Its wire belongs in pin 23 now, going to j8, and its right-hand column needs the black jumper from a10 to the bottom − rail. |
| The matrix stays dark | Its wiring from Lesson 25 may have come loose: DIN 47, CLK 48, CS 49. |

??? note "How it works"
    On every `adk::update ()` the joystick reads both analog pins, which
    takes about a tenth of a millisecond each. It compares each reading with
    the middle it recorded at setup. Each side of the middle is scaled on its
    own, so both ends reach 100 even when the middle isn't exactly 512: with
    a middle of 500, a reading of 750 is 250 / 523 of the way to the right
    end, 47. Anything between −10 and 10 becomes 0.

    `direction ()` keeps its answer until the stick falls back below 30 on
    that axis, and `moved ()` is true for just one update when the direction
    changes: one event per push, like `wasPressed ()` on a button.

## Make it yours

1. **Faster, slower.** Change `/ 4` to `/ 2` or `/ 8`. What happens to
   the gentlest pushes?
2. **One dot per push.** Replace the smooth movement with steps: when
   `joystick.moved ()` is `adk::Joystick::Right`, add 100 to `penX`, and so on
   for the other directions. Which is easier for neat drawings?
3. **Mirror drawing.** Whenever the pen draws a dot, also light
   `(7 - x, y)`, so everything you draw appears twice, like a butterfly.
4. **A drawing that fades.** Every two seconds, clear the dot the pen drew
   longest ago, so the trail is always the same length. You will need to
   remember the order the dots were drawn in: come back to this after the
   next lesson, whose snake does exactly that with an `adk::Deque`.
