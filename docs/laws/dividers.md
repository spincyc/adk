# Voltage dividers

The formula the course meets most often: a knob, a light sensor, a
thermometer and most 3.3 V modules' inputs are all dividers. If you
are working through the electricity course, do E04 and E06 first: this
page gives away what they find.

## The voltage divider {#divider}

<!-- law divider -->

Two resistors in series carry one current, so by
[Ohm's law](ohms-law.md) each takes a share of the voltage in proportion
to its resistance. The point between them sits at R₂'s share. Equal
resistors give half; R₂ twice R₁ gives two thirds. E04 measures it with
two 1 kΩ resistors, then a 1 kΩ and a 2 kΩ:

<p class="formula">5 V × <span class="fraction"><span>2 kΩ</span><span>1 kΩ + 2 kΩ</span></span> ≈ 3.3 V</p>

The divider is everywhere in the course:

- **A knob** is a divider you can turn: its wiper slides along one long
  resistor, dividing it into R₁ and R₂. Lesson 7 reads it on A0, and E06
  measures the wiper sweeping from 0 to 5 V.
- **A light or temperature reading** is a divider with one resistor that
  changes: the photoresistor in Lesson 8, the thermistor in Lesson 14.
  The Mega reads the middle voltage and works back to the light or the
  warmth.
- **Talking to a 3.3 V module** from a 5 V pin: 1 kΩ from the pin and
  2 kΩ to GND give that same 3.3 V, so the radio modules of Lessons 38
  and 40 hear a HIGH without being harmed by 5 V. The larger resistor
  must be the one to GND.

<!-- drawing 061-tap-a-divider closeup -->

## Loading a divider {#loading}

<!-- law loading -->

A divider's formula assumes nothing draws current from its middle. Hang
a load there and it joins the lower resistor **in parallel**, making the
lower part smaller, so the output falls.

**Worked: the loaded-divider challenge.** Two 1 kΩ resistors give 2.5 V.
Add a 1 kΩ load from the middle to GND. The lower half becomes 1 kΩ in
[parallel](kirchhoff.md#parallel) with 1 kΩ, 500 Ω, and the output falls
to:

<p class="formula">5 V × <span class="fraction"><span>500 Ω</span><span>1000 Ω + 500 Ω</span></span> ≈ 1.67 V</p>

There is a quicker way to see any load's effect. Seen from its output, a
divider behaves like a single source of its unloaded voltage with a
resistance in series, R₁ and R₂ in parallel, written R₁ ∥ R₂. This is
**Thévenin's
theorem**: any network of resistors and supplies, seen from two points,
acts like one voltage behind one resistance.

**Worked: E17's knob.** Set to 2.0 V, the wiper has 6 kΩ of the 10 kΩ
track above it and 4 kΩ below. Seen from the wiper, that is 2.0 V behind
6 kΩ ∥ 4 kΩ = 2.4 kΩ. A 1 kΩ load and those 2.4 kΩ divide the 2.0 V
again:

<p class="formula">2.0 V × <span class="fraction"><span>1 kΩ</span><span>2.4 kΩ + 1 kΩ</span></span> ≈ 0.59 V</p>

which is the reading E17 predicts for its meter. The same idea explains a supply that sags
under load: E18's 10 Ω feed is a resistance in series with the 5 V, and
E11's generator has its own 50 Ω.

There are two ways out:

- **Keep the load much larger** than the divider's resistance, ten times
  or more, and the output barely moves. The Mega's analog inputs and a
  meter both draw almost nothing, so a knob reads true.
- **Buffer it** with an amplifier that copies the voltage and supplies the
  current itself: E17's [op-amp follower](diodes-transistors-op-amps.md#op-amp).

## Check yourself

1. A divider has 10 kΩ from 5 V to its middle and 10 kΩ from there to
   GND. What does the middle read with nothing attached? What resistance
   does it look like to a load?
2. A module's input needs no more than 3.3 V. Would 2 kΩ from the pin and
   1 kΩ to GND be safe instead of the course's 1 kΩ and 2 kΩ?
3. Why does a knob's middle read true on the Mega's A0 but fall when E17
   hangs 1 kΩ on it?
4. In the loaded-divider challenge, would a 10 kΩ load pull the middle
   down more or less than 1 kΩ?

??? note "Answers"
    1. 2.5 V. To a load it looks like 2.5 V behind 10 kΩ ∥ 10 kΩ = 5 kΩ.
    2. Safe, but too low: the input would get 5 V × 1 ÷ 3 ≈ 1.7 V, which
       a 3.3 V module may not read as HIGH.
    3. A0 draws almost no current, so it barely loads the divider. 1 kΩ is
       smaller than the knob's 2.4 kΩ source resistance, nowhere near ten
       times larger, so it pulls the middle down.
    4. Less. 1 kΩ in parallel with 10 kΩ is about 910 Ω, so the middle
       only falls to about 5 V × 910 ÷ 1910 ≈ 2.4 V.

## Lessons that rely on them

<!-- relied on -->
