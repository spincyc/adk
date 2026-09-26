---
lesson: 8
promise: Measure the light around you and show it as a glowing bar.
time: 45 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Photoresistor
  - 10 kΩ resistor (brown, black, black, red, brown)
  - Red, yellow, green, blue and white LEDs
  - 5 × 220 Ω resistors (red, red, black, black, brown)
  - 15 jumper wires
ideas:
  - The photoresistor, a resistor that light controls
  - A divider that turns resistance into a voltage
  - Calibrating to the darkest and brightest, with a while loop
  - Smoothing a jumpy reading
  - Keeping a number in range with min, max and constrain
  - Big whole numbers, with long
---

## What you'll build

<!-- closeup -->

Five LEDs in a row rise and fall with the light, like the level meter on a
music player. Cup your hand over the sensor and the bar drains away to
nothing; hold a flashlight over it and all five blaze. Walk it from a dark
corner to a sunny window and watch the bar climb.

## The idea

A **photoresistor** is a resistor that light controls. Its wavy track is
made of a material that conducts better the more light falls on it. In the
dark it is a million ohms or more; in ordinary room light, somewhere around
10 kΩ; under a bright lamp, 1 kΩ or less.

The Mega can't measure resistance, only voltage, so the photoresistor gets a
partner: a fixed 10 kΩ resistor. The two make a **voltage divider**, like the
knob in Lesson 7, except that the two halves are separate parts. Current
flows from 5 V through the photoresistor, past the point that A1 measures,
and through the 10 kΩ to GND. The two share the 5 V between them in
proportion to their resistance, and A1 sees the 10 kΩ's share:

<p class="formula">voltage at A1 = 5 V × <span class="fraction"><span>10 kΩ</span><span>10 kΩ + photoresistor</span></span></p>

In room light, with the photoresistor near 10 kΩ, that is half of 5 V, and
A1 reads about 512. Under a lamp, at 1 kΩ, it is 5 × 10 ÷ 11 ≈ 4.5 V, a
reading of about 930. So: **more light, higher reading**.

Every photoresistor is a little different, and so is every room. So the
sketch **calibrates**: for its first five seconds it records the darkest and
brightest readings it sees, then shares the bar out between those two.

Readings also wobble. Many lamps flicker 100 or 120 times a second, too fast
for you to see but not for the Mega, and hands shake. A **smoother** calms
them: each new reading only moves the value an eighth of the way toward
itself, so a single odd reading barely nudges the bar.

!!! question "Predict"
    What if you swapped the photoresistor and the 10 kΩ resistor, so the
    10 kΩ went to 5 V and the photoresistor to GND? Would the bar still
    follow the light? Use the divider to work out what would change, and
    write down your answer. You can try it at the end.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring. This build uses two
    rails: the Mega's red wire makes the top + rail 5 V, and its black wire
    makes the bottom − rail GND. Every short black wire from row a goes to
    the bottom − rail, and the short red wire from j40 to the top + rail; no
    wire should ever join a + rail straight to a − rail.

<!-- bench -->

<!-- steps -->

??? info "Two kinds of resistor"
    This circuit uses two values that are easy to mix up, so check the bands
    before each one goes in, or check the kit's resistor card.

    | Resistor | Bands | Job |
    |---|---|---|
    | 10 kΩ | brown, black, black, red, brown | the photoresistor's partner in the divider |
    | 220 Ω | red, red, black, black, brown | one per LED, to set its current |

    Swap them and nothing breaks, but a 10 kΩ on an LED makes it very dim,
    and a 220 Ω in the divider squashes all the readings up near 1023.

The steps begin by taking out everything from Lesson 7 except the Mega's
GND and 5V wires. Each LED of the bar is built the way you built Lesson 7's,
in its own home, columns 6, 12, 18, 24 and 30: its pin's wire comes into
row j, its resistor crosses the middle gap to the long leg, and the short
leg's column has a short black wire down to the − rail. The red LED takes
about 14 mA and the white one about 8 mA; all five together are well within
what the Mega can supply.

The light sensor's divider stands in column 40, in the order the current
flows through it: a red wire brings 5 V down from the top + rail into j40,
the photoresistor stands across the middle gap in f40 and e40, A1's wire
comes into a40, the point between the two halves, and the 10 kΩ lies along
row c from c40 to c43, where a black wire takes it down to the − rail.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → Lesson08LightMeter**:

<!-- sketch -->

What's new:

- `adk::Array<adk::Led, 5> bar {26, 27, 28, 29, 30};` makes the five LEDs
  as an `adk::Array`, as in Lesson 4: `<adk::Led, 5>` says it holds five
  LEDs, and each pin number makes one of them. `bar[0]` is the red one on
  pin 26 and `bar[4]` the white one on pin 30.
- `adk::Smoother light {3};` is not a part of the circuit but a helper.
  `light.add (reading)` takes a new reading and gives back the smoothed
  value. The 3 means each reading moves it 1/2³, an eighth, of the way.
- `learnTheRoom ()` runs once, from `setup ()`. It starts `learning`, an
  `adk::Timer` as in Lesson 3, for five seconds.
- `while (learning.isRunning ())` is a **`while` loop**. It asks its
  question, and while the answer is true it runs the lines in its braces,
  then comes back to ask again. So it goes round and round, as fast as it
  can, until the timer runs out. Each time round it calls `adk::update ()`,
  which moves the timer on and keeps the white LED blinking; without that,
  the timer would never run out and the loop would never end.
- `min (darkest, reading)` gives the smaller of two numbers, and `max ()`
  the larger. So `darkest` can only go down, and `brightest` only up.
- `map (level, darkest, brightest, 0, 6)` is Arduino's scaling function: it
  does for a number you already have what `knob.read (low, high)` does for
  a knob in Lesson 7, and for any range. The bar can
  show six things, from none to five LEDs lit, so the range is cut into six
  slices, 0 to 5. Only a level at the very top, or brighter than anything
  the meter learned, would come out as 6 or more, and a darker one below 0.
  `constrain (number, 0, 5)` keeps the answer between 0 and 5: below 0
  becomes 0, and above 5 becomes 5.
- `long lit` is a **`long`**, a whole number with far more room than an
  `int`: an `int` on the Mega stops at 32 767, and a `long` goes past two
  billion. `lit` is a `long` because that is what `map ()` hands back.
  `map ()` works in `long`s because it multiplies before it divides:
  scaling a reading of 1023 to 0 to 255 goes through 1023 × 255 = 260 865
  on the way. Here the answer is only 0 to 5, which an `int` would hold
  too.
- The counting `for` loop, as in Lesson 6, visits each LED in turn, and
  `bar[led].set (led < lit)` lights it if it is below the top of the bar
  and turns it off otherwise. With `lit` at 2, `bar[0]` and `bar[1]`, red
  and yellow, are lit.
- `adk::println (Serial, "level:", level);` sends the smoothed level to the
  Serial Plotter, as a line named *level*.

## Upload it

Upload the sketch, and watch the white LED: it blinks for five seconds while
the meter learns. During those five seconds, cover the sensor with your hand,
then shine a flashlight or a desk lamp right onto it.

When the blinking stops, the bar shows the light. Cover the sensor and the
LEDs go out one by one, red last; uncover it and they come back. Open
**Tools → Serial Plotter** at 9600 baud to watch the smoothed level as a
line that dips when your hand passes over.

What did you predict for swapping the two resistors? The bar would run
backwards. A1 would then see the photoresistor's share of the 5 V instead
of the 10 kΩ's, and the photoresistor's share shrinks as the light grows:
more light, lower reading. The meter would learn that just as well, but a
brighter room would empty the bar instead of filling it. The first
challenge below lets you check.

## If it doesn't work

| What you see | Try this |
|---|---|
| The bar is always empty, or always full | The light didn't change during the first five seconds. Press the Mega's reset button and cover, then light, the sensor while the white LED blinks. |
| The bar is full in the dark and empty in the light | The photoresistor and the 10 kΩ have swapped places. The photoresistor goes across the gap in f40 and e40, the 10 kΩ from c40 to c43. |
| One LED never lights | Turn it round: its long leg goes in the column of its pin's wire. Check its resistor crosses the gap and its black wire reaches the − rail. |
| No LED ever lights | The black wires from row a must go to the − rail, the one with the blue line. |
| The level stays near 0 or near 1023 | Check A1's wire is in a40, the red wire joins j40 to the top + rail, the black one joins a43 to the − rail, and the Mega's red and black wires reach T+3 and B-3. |
| The top LED flickers on and off | The light is right at the edge of a slice. Try `adk::Smoother light {5};` for a calmer bar. |

??? note "How it works"
    `adk::Smoother` keeps its value eight times larger than it shows (for a
    shift of 3), which keeps the fractions a small computer would otherwise
    lose. Each `add ()` takes away an eighth of that store and adds the new
    reading. The first reading is taken as it is, so the bar starts in the
    right place instead of rising from zero. A smoother like this is called
    an *exponential moving average*: old readings fade away, never quite
    vanishing.

    The timer and the blinking LED both keep time in `adk::update ()`,
    which is why `learnTheRoom ()` calls it on every turn of its `while`
    loop. The first time `showBar ()` calls `set ()`, the blinking stops and
    the LED does as it's told.

## Make it yours

1. **Test your prediction.** Swap the photoresistor and the 10 kΩ resistor,
   press reset and let the meter learn again. What does the bar do now? Can
   you explain it with the divider?
2. **Night light.** Make the bar grow as the room gets darker: swap the
   `0` and the `6` in the `map ()`.
3. **One dot.** Light just the LED at the top of the bar, like a needle:
   `bar[led].set (led == lit - 1);`. What does the needle do in the
   darkest slice?
4. **Automatic lamp.** Add Lesson 7's white LED on pin 3, back in its home
   in column 38, as an `adk::PwmOutput`, and make it brighter the darker the
   room gets, with `map (level, darkest, brightest, 255, 0)`. Keep the
   result between 0 and 255 with `constrain ()`, as `showBar ()` does.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. With it you can watch
the divider share out the 5 V as the light changes.

The divider doesn't need the sketch at all, so it stays still as long as the
light does. For the covered readings your hands are busy with the probes, so
cover the sensor with a bottle cap, or a small cup turned upside down.

!!! question "Predict"
    In room light, with the photoresistor near 10 kΩ, what should the
    divider's middle read? When you cover the sensor, will it go up or
    down? Use the formula above.

<!-- measure -->

What the numbers tell you:

- **In room light** the middle sits somewhere near half of 5 V. Anything
  from about 1.5 V to 4 V is normal: the brighter the room, the lower the
  photoresistor's resistance, and the higher the reading. The plotter's
  level × 5 ÷ 1023 comes out close to your meter's reading.
- **Covered**, the middle drops, to under 1 V: in the dark the photoresistor
  has far more resistance than the 10 kΩ, so it takes nearly all of the
  5 V, and little is left for A1.
- **Across the photoresistor** is its share. Add readings 2 and 3 and you
  get 5 V back. The two parts share the 5 V in proportion to their
  resistance, so your meter can work out the photoresistor's: 4.5 V is
  nine times 0.5 V, so it is nine times the 10 kΩ, about 90 kΩ. Try the
  same sum in room light, and under a lamp.
