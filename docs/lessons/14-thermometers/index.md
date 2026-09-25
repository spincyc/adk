---
lesson: 14
title: Thermometers
arc: Words and weather
promise: Measure the temperature three different ways, and see which thermometer you trust.
time: 60 minutes
level: 2
sketch: Lesson14Thermometers
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - The LCD, knob and 220 Ω resistor from Lesson 13, wired as before
  - DHT11 temperature and humidity module
  - 10 kΩ thermistor
  - 10 kΩ resistor (brown, black, black, red, brown)
  - 18B20 temperature module (from the 37-in-1 kit; optional)
  - 6 female-to-male jumper wires and 3 more jumper wires
ideas:
  - A sensor that talks in timed pulses
  - Resistance that changes with heat
  - A whole thermometer on one chip
  - Comparing sensors
  - Numbers with decimals, float
---

## What you'll build

<!-- closeup -->

Three thermometers, each working in its own way, report side by side on the
screen from [Lesson 13](../13-hello-lcd/index.md): the DHT11, which also
measures how damp the air is; a thermistor, which is just a resistor that
cares about heat; and the 18B20, a tiny digital thermometer. Pinch one, breathe
on another, and watch them argue.

## The idea

A thermometer turns heat into something the Mega can read. These three do it
in three different ways.

**The DHT11 talks in timed pulses.** It is a digital sensor on one wire. The
Mega pulls the wire low for 20 ms to say "measure now", and the sensor answers
with 40 bits. Each bit is a high pulse, and its *length* is the message: about
27 µs means 0, about 70 µs means 1. Forty bits make five bytes: the humidity,
its tenths, the temperature, its tenths, and a **checksum**, the first four
added up. If the air is at 23 °C and 45 % humidity, the bytes are 45, 0, 23, 0
and 68, and because 45 + 0 + 23 + 0 = 68, the Mega knows nothing was garbled
on the way. The DHT11 is slow (one reading every two seconds) and rough: ±2 °C
and ±5 % humidity.

**The thermistor changes its resistance.** It is 10 kΩ at 25 °C and less when
it is warmer. In a divider with a 10 kΩ resistor, like the potentiometer in
Lesson 7 or the photoresistor in Lesson 8, that turns into a voltage on A2:

<p class="formula">voltage on A2 = 5 V × <span class="fraction"><span>10 kΩ</span><span>thermistor + 10 kΩ</span></span></p>

At 25 °C that's 5 V × 10 / 20 = 2.5 V, a reading of about 511. At 30 °C the
thermistor drops to about 8.0 kΩ, the pin rises to 2.77 V and reads about 567.
The relationship isn't a straight line, so ADK works out the resistance from
the reading and then uses the thermistor's **beta equation**, a formula from
its datasheet with one number, beta, that says how steeply its resistance
falls. ADK assumes 3950, typical for this kind of thermistor; the kit doesn't
say, so the last challenge shows you how to check yours.

**The 18B20 measures itself.** It is a whole thermometer on a chip: it measures,
turns the answer into a number and sends it down one wire, using a scheme
called **1-Wire**. The number is in sixteenths of a degree, so 23.125 °C
arrives as 370, and a check code comes with it. It takes 750 ms over each
reading and is good to ±0.5 °C, the best of the three.

!!! question "Predict"
    When it's running, you'll pinch the thermistor's black bead between finger
    and thumb for twenty seconds, then let go. Which of the three readings
    will change? How far, and how fast will it come back? Then you'll breathe
    gently on the DHT11. Which of its two numbers will jump more? Write your
    guesses down.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you wire. Keep Lesson 13's circuit as it is:
    everything new goes to the right of it. Each module's pins are marked
    **S**, **+** and **−**. Check the marks on yours before you wire it, because
    a module wired backwards can get hot; if one does, unplug at once.

<!-- bench -->

<!-- steps -->

!!! tip "No 18B20?"
    It comes in the 37-in-1 sensor kit, not the Mega kit. Leave it and its
    three wires out: the sketch shows `--` in its place, and everything else
    works.

??? info "Why the modules need only three wires"
    The DHT11 and the 18B20 never push their data wire high. They only pull it
    low, or let go. Something has to bring it back up when they let go, and
    that is a **pull-up resistor** from the data wire to 5 V. Each module has
    one soldered on its little board, so S, + and − are all it needs. A bare
    DHT11 or 18B20, without the board, needs a 10 kΩ or 4.7 kΩ resistor added
    from its data pin to 5 V.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson14Thermometers**:

<!-- sketch -->

What's new:

- `adk::Dht11 dht {16};`, `adk::Thermistor thermistor {A2};` and
  `adk::Ds18b20 probe {17};` are the three thermometers. They each take their
  readings on their own, inside `adk::update ()`, so the sketch just asks for
  the latest: `dht.temperature ()`, `dht.humidity ()`,
  `thermistor.celsius ()` and `probe.celsius ()`.
- Each of those hands back a **`float`**, a number with decimals, such as
  23.4. An `int` holds only whole numbers: it would keep the 23 and lose the
  .4. `showReading ()` takes its reading as `float value`.
- `lcd.print (value, 1)` prints a `float` with one digit after the point,
  rounding the rest, so 23.46 shows as 23.5. With 0 it shows whole numbers.
- `dht.ok ()` and `probe.ok ()` say whether the latest reading arrived whole.
  Until the first one, or if the sensor is missing, they are false and
  `showReading ()` shows `--` instead of a number.
- `char (223)` is the character with code 223, which on this screen is a
  little degree sign, and `constexpr char degree` gives it a name. A `char`
  is one character; `const char*`, from Lesson 3, is a whole piece of text.
- `adk::print (lcd, degree, "C  ");` prints its pieces in a row, like
  `adk::println ()` from Lesson 2, but on the screen and without ending the
  line.
- The spaces printed at the end of each row rub out anything left over when a
  number gets shorter, say from 10.0 to 9.9.

## Upload it

Upload the sketch. Within a couple of seconds the screen fills in, something
like this (the DHT11 shows `--` until its first reading):

```text
DHT11 23°C  45%
NTC 23.4 DS 23.1
```

The three temperatures should agree within a degree or two. Now test your
prediction: pinch the thermistor's bead. Only its reading changes: it climbs
within seconds, a degree or more every few seconds, while the other two stay
put, and when you let go it drifts back, more slowly than it rose. Breathe
on the DHT11 and it's the humidity that jumps, by ten or twenty percent, then
settles over the next minute; its temperature moves much less.

## If it doesn't work

| What you see | Try this |
|---|---|
| `DHT11 --` never changes to a number | Check the DHT11's S goes to pin 16, + to the + rail and − to the − rail. It needs a second after power-up, so wait two. |
| `NTC` shows about −77 | A2 reads 0: the thermistor isn't connected. Check its legs are in b35 and b37, and the red wire from e35 to the + rail. |
| `NTC` shows hundreds of degrees | A2 reads 1023: the thermistor is shorted, or the 10 kΩ resistor isn't connected to the − rail (e40). |
| `NTC` is ten degrees or more away from the others | Check the resistor is 10 kΩ (brown, black, black, red, brown), not 1 kΩ. |
| `DS --` with the 18B20 fitted | Check its S goes to pin 17, and its + and − aren't swapped. |
| The screen is blank or shows blocks | Go back to Lesson 13's table: the LCD wiring or the contrast knob. |

??? note "How it works"
    Each thermometer is read at its own pace, inside `adk::update ()`:

    - **DHT11:** every two seconds ADK holds pin 16 low for 20 ms, then listens
      for the 40 bits, timing each high pulse. Listening takes about 4 ms with
      interrupts switched off, because the pulses are too short to risk
      missing. A reading whose checksum doesn't add up is thrown away, and the
      last good one is kept.
    - **Thermistor:** A2 is read every 100 ms and smoothed with a Smoother, as
      in Lesson 8, so the number doesn't flicker.
    - **18B20:** ADK asks it to measure, comes back 750 ms later, reads nine
      bytes from it and checks their code, then asks for the next reading.
      Collecting a reading takes about 10 ms.

## Make it yours

1. **Fahrenheit.** Show the thermistor in °F with `thermistor.fahrenheit ()`.
   For the others, °F = °C × 9 / 5 + 32. Keep it all in `float`s: in whole
   numbers, 9 / 5 is just 1.
2. **A race on the Serial Plotter.** Every second, print the three
   temperatures on one line with one `adk::println (Serial, ...)`, separated
   by spaces, and open **Tools → Serial Plotter**. Pinch, breathe and blow,
   and watch which line reacts first.
3. **Highs and lows.** Keep the lowest and highest thermistor temperatures
   since the Mega started, in two `float` variables, and show them on the
   bottom row instead.
4. **Tune your thermistor.** If it disagrees with the 18B20 by more in your
   warm hand than at room temperature, its beta may not be 3950. Try
   `adk::Thermistor thermistor {A2, 3435};` and other values from 3000 to
   4300, and keep the one that agrees best.
