---
lesson: 4
promise: Mix any color you like from red, green and blue light, and glide between them.
time: 45 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - RGB LED (common cathode, four legs)
  - 3 × 220 Ω resistors (red, red, black, black, brown)
  - 1 push button
  - 6 jumper wires
ideas:
  - Dimming by switching fast, called PWM
  - Mixing colors from red, green and blue light
  - Colors as numbers, and fading between them
  - A list of colors, and constants that never change
  - The color wheel
---

## What you'll build

<!-- closeup -->

A lamp with moods. It warms up to a sunset orange, and every press of the
button glides it smoothly to the next mood: ocean blue, forest green, candy
pink. The fifth mood is a rainbow that drifts slowly all the way round the
colors, and one more press brings back the sunset. Put a ping-pong ball or a
scrap of white paper over the LED, turn the lights down, and it really does
set a mood.

## The idea

A pin is either fully on, at 5 V, or fully off. So how can a light be half
bright? The Mega cheats, very quickly. It switches the pin on and off about
490 times a second and changes how much of each flicker is *on*. That's
called **pulse-width modulation**, or **PWM**. Your eye can't follow 490
flickers a second, so it sees the average. Brightness is a number from 0
(always off) to 255 (always on), so 64 means on for a quarter of the time:

<p class="formula">time on = <span class="fraction"><span>64</span><span>255</span></span> ≈ 25 %</p>

Only some pins can do PWM: on the Mega, pins 2 to 13 and 44 to 46. The
RGB LED's home is pins 5, 6 and 7.

An **RGB LED** is three tiny LEDs, red, green and blue, in one lens. They
share one leg, the longest, called the **common** leg; in the kit's RGB LED
it is the **cathode** (−) of all three, so it goes to GND. Each of the other
three legs is one color's long leg (+), and each needs its own 220 Ω
resistor, just like any LED. The red one takes about 14 mA, as in Lesson 1;
green and blue keep more voltage for themselves, about 3.2 V, so they take
about 8 mA.

Mix the three and you can make almost any color. In ADK a color is an
`adk::Color`, three numbers, `{red, green, blue}`, each from 0 to 255:
`{0, 0, 255}` is pure blue, `{255, 255, 255}` is all three at full, and
`{0, 0, 0}` is off. ADK has names for common ones, such as
`adk::color::orange`, and it can **fade** from one color to another over any
time you choose.

!!! question "Predict"
    Mixing red and green *paint* makes a muddy brown. What do you think you
    see when the red and green *lights* are both fully on and blue is off:
    the color `{255, 255, 0}`? Write down your guess.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring. Each color leg needs
    its own 220 Ω resistor: one resistor shared by all three would let the
    red LED take most of the current and starve the others.

<!-- bench -->

<!-- steps -->

??? info "Finding the RGB LED's legs"
    The longest leg is the common one. Hold the LED with its legs pointing
    down and the longest leg second from the left: then the legs are, from
    left to right, red, common, green and blue. That's how the kit's RGB LED
    is made, but if your colors come out in the wrong places after you
    upload, you haven't damaged anything: see the table below.

    The legs come out of the LED a tenth of an inch apart, but their holes
    are further apart than that: red in a6, green three columns on in a9,
    blue in a11, and the common leg down in the − rail at B-7. Spread the
    legs gently with your fingers until each lines up with its hole, then
    push the LED in. The common leg is long enough to reach straight down
    into the − rail, so it needs no wire of its own.

    The RGB LED takes the red LED's place from Lesson 3, so that LED's
    resistor, in g6 and e6, can stay where it is: it now feeds the red leg.
    The button on pin 22 stays too, with its wires.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson04MoodLamp**:

<!-- sketch -->

What's new:

- `adk::RgbLed lamp {5, 6, 7};` is an RGB LED with its red leg on pin 5,
  green on pin 6 and blue on pin 7.
- `adk::color::orange` is one of ADK's named colors, `{255, 64, 0}`. Just
  as `adk::` in front of a name says it belongs to ADK, `adk::color::` says
  it is one of ADK's colors.
- `constexpr adk::Array moods {...};` is a list of four colors, an
  **`adk::Array`**. Its places are numbered from 0, so `moods[0]` is orange
  and `moods[3]` is pink, and it knows its own size: `moods.size ()` is 4.
- `constexpr` means the value is settled when the sketch is compiled, and
  never changes while it runs. `constexpr int rainbow = moods.size ();`
  makes the rainbow mood number 4, the one after the last color. Add a
  color to `moods` and the rainbow moves along by itself.
- `lamp.fadeTo (moods[mood], 1000);` starts a smooth fade to a new color,
  lasting 1000 ms, and returns at once. ADK moves the color a little further
  on every `adk::update ()`, so the button keeps working mid-fade.
- `mood = (mood + 1) % (rainbow + 1);` counts 0, 1, 2, 3, 4 and then back to
  0. `%` gives the **remainder** after dividing: when `mood + 1` reaches 5,
  5 % 5 is 0.
- In the rainbow mood, `lamp.isFading ()` says whether a fade is still
  going. Each time one ends, `driftAroundTheWheel ()` starts the next, to a
  color a little further round the wheel.
- `adk::wheel (hue)` turns a position round the color wheel into a color:
  0 is red, 85 green, 170 blue, and on round toward red again. `hue` is a
  **`uint8_t`**, a whole number from 0 to 255 that wraps back to 0 after
  255, just as a wheel comes back round. It takes one byte of memory, where
  an `int` takes two.

## Upload it

Upload the sketch. The LED fades up to orange over two seconds. Press the
button: in one second it glides to blue. Press again for green, and again
for pink. The next press starts the rainbow, which drifts round every color
in about ten seconds, and the next brings back orange.

You predicted what red and green light make together. Test it: change the
first mood to `adk::Color {255, 255, 0}` (in the list, three bare numbers
need `adk::Color` in front to say they are a color), upload, and look. Red
and green light together make **yellow**. Mixing light adds colors
together, while paint takes them away. ADK's own `adk::color::yellow` is
`{255, 160, 0}`, with a little less green, because the green LED looks
brighter to your eye than the red one.

## If it doesn't work

| What you see | Try this |
|---|---|
| Nothing lights at all | Check the longest leg is in the − rail (B-7), and the black wire from the Mega's GND reaches B-3. |
| One color never appears | Follow that color from its pin: pin 5 to j6, the resistor from g6 across the gap to e6, the red leg in a6. Green is pin 6 and column 9, blue pin 7 and column 11. |
| The colors are in the wrong places | Your LED's legs are in a different order. Swap the numbers in `adk::RgbLed lamp {5, 6, 7};` until orange looks orange, or swap the wires. |
| Still nothing, and the longest leg is in the − rail | You may have a common-anode LED, whose longest leg is +. Move that leg to the + rail, add a red wire from the Mega's 5V pin to the + rail, and write `adk::RgbLed lamp {5, 6, 7, adk::ActiveLow};`. |
| White looks a little pink or blue | That's normal: the three tiny LEDs are not exactly equally bright. |
| The button does nothing | Push it firmly into the board, and check the black wire from a4 to the − rail. |

??? note "How it works"
    Each PWM pin is driven by a **timer**, a counter inside the Mega that
    counts up and down on its own and flips the pin at the right moments,
    so your sketch never has to. Pin 5 uses Timer 3, and pins 6 and 7 Timer
    4. `adk::setup ()` checks that each pin you give an `RgbLed` can do PWM:
    try `{5, 6, 22}` and the Mega's **L** LED blinks pin 22's number, 2 long
    and 2 short. With `adk::setup (Serial)` it would say *adk: pin 22 cannot
    do PWM; use 2-13 or 44-46*.

    A fade works out its color from the time. Half way through a
    1000 ms fade from orange to blue, 500 ms in, each of the three numbers
    is half way between its start and its end.

## Make it yours

1. **Your own moods.** Change the four colors to your own mixes, such as
   `adk::Color {255, 0, 40}` for a hot pink. To add a fifth color, put it in
   `moods`: nothing else needs to change, because `rainbow` and the `%` both
   come from `moods.size ()`.
2. **Slow motion.** Make the rainbow take a whole minute to go round. How
   long should each of its 32 steps last?
3. **Candle.** Add a candle mood that flickers. Each time a fade ends, make
   a color `adk::Color flame = adk::color::orange;`, give it a little more or
   less green with `flame.green = random (40, 100);`, and fade to it in
   100 ms.
4. **Night light.** Make a long press, held for two seconds, fade the lamp
   slowly to `adk::color::off` over a minute, so it can send you to sleep.
   An `adk::Timer`, as in Lesson 3, can time the press: start it for
   2000 ms when the button goes down and `stop ()` it when
   `button.wasReleased ()`. If it `expired ()`, the button was held for two
   seconds.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts, the black
lead in **COM** and the red in **V**. Never move the red lead to the **A**
jack for these: set for current, the meter is just a wire, and would short
out whatever you put it across.

The sketch needs no changes: each mood stays put until you press the
button. The lamp starts in orange, `{255, 64, 0}`, and one press takes it to
blue, `{0, 0, 255}`. Keep each tip in its own hole.

!!! question "Predict"
    In orange, green is 64: pin 6 is on for a quarter of the time and off for
    the rest, 490 times a second. What will the meter show on pin 6: 5 V,
    0 V, or something in between?

<!-- measure -->

What the numbers tell you:

- **The red pin**, at 255, is on all the time: about 5 V, like an LED's pin
  in Lesson 1.
- **The green pin**, at 64, reads about 1.25 V. A meter can't follow 490
  flickers a second any better than your eye can, so it shows the average:
  on for 64 parts in 255, a quarter of the time, and a quarter of 5 V is
  1.25 V. Make a mood with a green of 128 and it reads about 2.5 V.
- **Across the red LED** is about 2 V, and **across the blue LED** about
  3.2 V: the blue one keeps more for itself. That leaves 3 V across the red
  leg's 220 Ω resistor but only 1.8 V across the blue's, which is why red
  takes about 14 mA and blue about 8 mA: 1.8 V ÷ 220 Ω ≈ 8 mA.
