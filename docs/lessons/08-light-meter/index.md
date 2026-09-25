---
lesson: 8
title: Light Meter
arc: The analog world
promise: Measure the light around you and show it as a glowing bar.
time: 45 minutes
level: 2
sketch: Lesson08LightMeter
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
  - Calibrating to the darkest and brightest
  - Smoothing a jumpy reading
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
    Unplug the USB cable before you change any wiring. This build uses both
    bottom rails: the red wire makes the + rail 5 V, the black wire makes the
    − rail GND. Every short black wire from row a goes to the − rail; and no
    wire should ever join the + rail straight to the − rail.

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

Each LED is built the way you built Lesson 7's: its pin's wire comes into
the long leg's column, and its short leg's resistor crosses the middle gap
to a short black wire down to the − rail. The red LED takes about 14 mA and
the white one about 9 mA; all five together are well within what the Mega
can supply.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → Lesson08LightMeter**:

<!-- sketch -->

What's new:

- `adk::Led bar [] {{26}, {27}, {28}, {29}, {30}};` declares the five
  LEDs as an array, one pair of braces per LED, so `bar[0]` is the red one
  on pin 26 and `bar[4]` the white one on pin 30.
- `adk::Smoother light {3};` is not a part of the circuit but a helper.
  `light.add (reading)` takes a new reading and gives back the smoothed
  value. The 3 means each reading moves it 1/2³, an eighth, of the way.
- `learnTheRoom ()` runs once, from `setup ()`. For five seconds, measured
  with `millis ()` as in Lesson 3, it keeps the lowest reading in `darkest`
  and the highest in `brightest`. It calls `adk::update ()` all the while,
  so the white LED keeps blinking.
- `map (level, darkest, brightest, 0, 6)` is Arduino's scaling function: it
  does for any range what `knob.read (0, 255)` did in Lesson 7. The bar can
  show six things, none to five LEDs lit, so the range is cut into six.
- The `for` loop visits each LED in turn, and `bar[led].set (led < lit)`
  lights it if it is below the top of the bar and turns it off otherwise.
  With `lit` at 2, `bar[0]` and `bar[1]`, red and yellow, are lit.

## Upload it

Upload the sketch, and watch the white LED: it blinks for five seconds while
the meter learns. During those five seconds, cover the sensor with your hand,
then shine a flashlight or a desk lamp right onto it.

When the blinking stops, the bar shows the light. Cover the sensor and the
LEDs go out one by one, red last; uncover it and they come back. Open
**Tools → Serial Plotter** at 9600 baud to watch the smoothed level as a
line that dips when your hand passes over.

## If it doesn't work

| What you see | Try this |
|---|---|
| The bar is always empty, or always full | The light didn't change during the first five seconds. Press the Mega's reset button and cover, then light, the sensor while the white LED blinks. |
| The bar is full in the dark and empty in the light | The photoresistor and the 10 kΩ have swapped places. The photoresistor goes from c4 to c6, the 10 kΩ from b6 to b10. |
| One LED never lights | Turn it round: its long leg goes in the column of its pin's wire. Check its resistor crosses the gap and its black wire reaches the − rail. |
| No LED ever lights | The black wires from row a must go to the − rail, the one with the blue line. |
| The level stays near 0 or near 1023 | Check A1's wire is in a6, and the red and black wires from the Mega reach the rails. |
| The top LED flickers on and off | The light is right at the edge of a slice. Try `adk::Smoother light {5};` for a calmer bar. |

??? note "How it works"
    `adk::Smoother` keeps its value eight times larger than it shows (for a
    shift of 3), which keeps the fractions a small computer would otherwise
    lose. Each `add ()` takes away an eighth of that store and adds the new
    reading. The first reading is taken as it is, so the bar starts in the
    right place instead of rising from zero. A smoother like this is called
    an *exponential moving average*: old readings fade away, never quite
    vanishing.

    `adk::Led`'s `blink ()` keeps time in `adk::update ()`, which is why
    `learnTheRoom ()` calls it on every turn of its loop. The first time
    `showBar ()` calls `set ()`, the blinking stops and the LED does as it's
    told.

## Make it yours

1. **Test your prediction.** Swap the photoresistor and the 10 kΩ resistor,
   press reset and let the meter learn again. What does the bar do now? Can
   you explain it with the divider?
2. **Night light.** Make the bar grow as the room gets darker: swap the
   `0` and the `6` in the `map ()`.
3. **One dot.** Light just the LED at the top of the bar, like a needle:
   `bar[led].set (led == lit - 1);`.
4. **Automatic lamp.** Add Lesson 7's white LED on pin 3 as an
   `adk::PwmOutput`, and make it brighter the darker the room gets, with
   `map (level, darkest, brightest, 255, 0)`. Keep the result between 0 and
   255 with `constrain ()`.
