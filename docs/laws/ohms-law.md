# Ohm's law

The law the course uses most: every LED's resistor, every current found
with a voltmeter and every divider comes from it. If you are working
through the electricity course, do E02 and E03 first: this page gives
away what they find.

## The law in three forms {#ohms-law}

<!-- law ohms-law -->

Rearranged, the same law answers whichever of the three you don't know:

| To find | Use | Example from the course |
|---|---|---|
| The current | I = V ÷ R | 3 V across 220 Ω drives about 14 mA (Lesson 1) |
| The voltage | V = I × R | 2.5 mA through 1 kΩ leaves 2.5 V across it (E04) |
| The resistance | R = V ÷ I | To drop 3 V at 14 mA takes 3 V ÷ 0.014 A ≈ 214 Ω, so 220 Ω |

With [the shortcuts](units.md#shortcuts), volts ÷ kilohms gives
milliamps, which is why 3 V across 1 kΩ is simply 3 mA.

## Where you measure it {#measure}

You see it before you name it. E03 swaps 220 Ω, 1 kΩ and 2 kΩ in front of
the same LED, and the LED dims as the resistance rises:

<!-- drawing 058-resist-the-flow closeup -->

E02 finds the current from the resistor's voltage, and Lesson 1's
*Measure it* does the same for a blinking LED.

## Finding a current with a voltmeter {#current-from-voltage}

The course keeps the meter on DC volts and never asks you to switch it to
current. To find a current, measure the voltage across a resistor you
know, and divide:

<p class="formula">current in mA = <span class="fraction"><span>1000 × voltage across the resistor in V</span><span>its resistance in Ω</span></span></p>

That works anywhere a known resistor sits in the path, and the meter
never becomes part of the circuit. A meter set to current has almost no
resistance of its own: put its probes across a supply and it is a short
circuit. E05 uses the trick on purpose, with a 10 Ω resistor in the feed
whose few tens of millivolts give the current to both branches.

## Choosing an LED's resistor {#led-resistor}

An LED isn't a resistor: it keeps about the same voltage across itself,
whatever current flows (see [forward voltage](diodes-transistors-op-amps.md#forward-voltage)).
So the resistor's job is to take the rest of the supply and set the
current:

1. **The supply:** 5 V from a pin or the + rail.
2. **The LED's share:** about 2 V for red, about 3 V for white or blue.
3. **The resistor's share** is what's left, by
   [Kirchhoff's voltage law](kirchhoff.md#voltage-law): 5 V − 2 V = 3 V.
4. **Pick a current** below a pin's 20 mA: 10 to 15 mA is bright.
5. **Ohm's law:** R = 3 V ÷ 0.014 A ≈ 214 Ω. Take the next value up that
   you have, 220 Ω, never the next one down.

A larger resistor is always safe, just dimmer: with 1 kΩ the same LED
takes about 3 mA, which is why Lesson 10 gives each of a digit's seven
segments 1 kΩ: together they share one chip's current.

## Where it doesn't apply {#limits}

- **LEDs and diodes** don't follow it: a little more voltage makes much
  more current. That is why they always need a resistor, and why E03's
  three currents aren't in exact proportion to the resistances.
- **Photoresistors and thermistors** are resistors whose resistance
  changes with light or warmth. At any moment Ohm's law still holds;
  Lessons 8 and 14 read the change through a [divider](dividers.md).
- **Capacitors and coils** follow it only once nothing is changing: a
  charged capacitor passes no steady current, and a coil acts like a wire
  of its winding's resistance. While a voltage changes, see
  [Capacitors and coils](capacitors-and-coils.md).
- **A wire** has almost no resistance, so a wire straight across a supply
  lets through as much current as the supply can give. That is a short
  circuit, and [Power and heat](power.md#short-circuit) shows why it gets
  hot.

## Common mistakes {#mistakes}

- **Using the whole supply voltage** for one part. 5 V ÷ 220 Ω = 23 mA,
  but the LED takes 2 V, so only 3 V is across the resistor.
- **Dropping a prefix.** 3 ÷ 220 = 0.0136, in amps: 13.6 mA, not
  0.0136 mA.
- **Mixing parts.** The voltage in the sum must be the voltage across the
  same part whose resistance you use.

## Check yourself

1. A meter reads 1.5 V across a 1 kΩ resistor. What current flows
   through it?
2. E05's 10 Ω feed reads 60 mV. How much current feeds the two branches?
3. A green LED keeps about 2.2 V on a 5 V pin. Which of the kit's 220 Ω,
   1 kΩ and 2 kΩ keeps it under 15 mA and brightest?
4. With 2 kΩ in E03 the current is a little more than half what it is
   with 1 kΩ. Why not exactly half?

??? note "Answers"
    1. 1.5 V ÷ 1 kΩ = 1.5 mA.
    2. 0.060 V ÷ 10 Ω = 0.006 A = 6 mA.
    3. The resistor takes 5 V − 2.2 V = 2.8 V. With 220 Ω that is about
       12.7 mA, under 15 mA and the brightest of the three.
    4. At the smaller current the LED keeps a little less voltage, so the
       resistor gets a little more than before, and the current is a
       little more than half.

## Lessons that rely on it

<!-- relied on -->
