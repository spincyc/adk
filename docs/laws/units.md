# Quantities and units

Every number in a circuit is an amount of something: so many volts, so
many milliamps. Keep the unit beside every number, from the reading to
the answer, and most mistakes show themselves before they cost a part.

## The quantities {#quantities}

| Quantity | In formulas | Unit | Its symbol | What it measures | In ADK |
|---|---|---|---|---|---|
| Charge | Q | coulomb | C | An amount of electricity | E07's capacitor holds about 5 mC at 5 V |
| Current | I | ampere, or amp | A | How much charge passes a point each second | 14 mA through Lesson 1's LED |
| Voltage | V | volt | V | The push between two points: the energy each coulomb gains or gives up between them | 5 V from USB; about 2 V across a red LED |
| Resistance | R | ohm | Ω | How many volts it takes to push each amp through a part | 220 Ω, 1 kΩ, 10 kΩ |
| Power | P | watt | W | How fast a part turns electricity into light, heat or motion | About 0.04 W in Lesson 1's resistor |
| Capacitance | C | farad | F | How much charge a capacitor stores for each volt across it | 100 nF beside a chip; 1000 µF in E07 |
| Inductance | L | henry | H | How hard a coil pushes back when its current changes | E11's 100 mH coil |
| Time | t | second | s | How long | E08's 10 s charge; a 104 µs serial bit |
| Frequency | f | hertz | Hz | How many cycles happen each second | 490 Hz PWM; a 1 kHz test wave |

The letter V does two jobs: it stands for *a voltage* in a formula and
for *volts* after a number, as in "V = 5 V". So does C: *a capacitance*
in a formula, *coulombs* after a number, as in "5 mC". The other
quantities use different letters for the two, such as I for a current
and A for amps.

## Prefixes {#prefixes}

<!-- law prefixes -->

Circuits use amounts from billionths to millions, so a prefix in front of
the unit says how big it is:

| Prefix | Symbol | Times | Example in ADK |
|---|---|---|---|
| mega | M | 1 000 000 | A meter's 10 MΩ input; the Mega's 16 MHz clock |
| kilo | k | 1 000 | 1 kΩ = 1000 Ω; a 1 kHz tone |
| — | — | 1 | 5 V; 220 Ω |
| milli | m | ÷ 1 000 | 14 mA = 0.014 A; a 20 ms debounce |
| micro | µ | ÷ 1 000 000 | 1000 µF = 0.001 F; 104 µs for one serial bit |
| nano | n | ÷ 1 000 000 000 | 100 nF = 0.0000001 F |

When a sum mixes prefixes, write each number in its plain unit first,
work it out, then put a prefix back on the answer:

<p class="formula">3.0 V ÷ 220 Ω = 0.0136 A = 13.6 mA</p>

### Shortcuts that keep the prefixes straight {#shortcuts}

Some pairs of prefixes cancel, so these come out right without any
zeros:

| Multiply or divide | Gives | Example |
|---|---|---|
| volts ÷ kilohms | milliamps | 3 V ÷ 1 kΩ = 3 mA |
| milliamps × kilohms | volts | 2.5 mA × 1 kΩ = 2.5 V |
| volts × milliamps | milliwatts | 3 V × 14 mA = 42 mW |
| kilohms × microfarads | milliseconds | 1 kΩ × 1 µF = 1 ms; 10 kΩ × 1000 µF = 10 000 ms = 10 s |
| 1 ÷ kilohertz | milliseconds | 1 ÷ 1 kHz = 1 ms |

They work because kilo (× 1000) and milli (÷ 1000) undo each other, and
kilo × micro is milli.

## Charge and current {#charge}

<!-- law charge -->

Everything is made of atoms, and atoms hold electric charge: positive in
their middles, negative in the electrons around them. In a metal some
electrons are free to wander from atom to atom. Push them all one way and
charge flows: that flow is a **current**. One amp is one coulomb passing
each second.

A current isn't used up as it goes round: all the charge that flows into
an LED flows out of its other leg, and on round the loop, which is
[Kirchhoff's current law](kirchhoff.md#current-law). What the LED takes
is **energy**, which is what a voltage measures (see
[Power and heat](power.md)). A capacitor is the one part that gathers
charge, on its plates: see [Capacitors and coils](capacitors-and-coils.md#capacitor).

Circuits are drawn and explained with current flowing from + to −, out of
the supply's + side and back into its − side. In a wire the electrons
actually drift the other way. Both descriptions give the same answers, so
the course, like nearly every book, uses + to − throughout.

## Decibels and dBm {#decibels}

<!-- law decibels -->

Radio power covers a huge range, from a transmitter's milliwatts to less
than a millionth of a billionth of a watt at the edge of range, so radios
count it in
**decibels**, dB, which add where powers multiply. **dBm** is decibels
compared with one milliwatt:

| dBm | Power | |
|---|---|---|
| +15 | about 32 mW | A LoRa modem at full power |
| +10 | 10 mW | |
| +3 | about 2 mW | Every 3 dB up doubles the power |
| 0 | 1 mW | |
| −10 | 0.1 mW | Every 10 dB down is ten times weaker |
| −100 | 0.000 000 000 1 mW | A weak but usable signal arriving |

So a step from 13 dBm to 10 dBm halves the power, and a signal that fades
from −80 dBm to −110 dBm has become a thousand times weaker. A receiver
can hear down to its **sensitivity**, about −130 dBm for Lesson 40's
modems; how far a signal is above that is how many decibels it can still
lose to a wall or a longer path. The **margin** a LoRa modem reports is
different: how far the signal stands above the radio's own noise, which
can fall to about −15 dB before messages stop. Lessons 40 and 55 read
their signals this way.

## Reading a value off a part {#reading-parts}

- **Resistors** carry colored bands; [Reading resistors](../kit.md#reading-resistors)
  shows how to decode them. A meter in its Ω setting, with the resistor
  out of the circuit, checks the answer.
- **Small ceramic capacitors** carry three digits in picofarads (pF, a
  millionth of a microfarad): two digits, then how many zeros. **104** is
  10 followed by four zeros, 100 000 pF, which is 100 nF.
- **Electrolytic capacitors** print their value and their highest safe
  voltage, such as *1000 µF 16 V*, and mark the − leg with a stripe.
- **Chips and modules** give their limits in a datasheet: the maker's
  document of what the part needs and can do.

## Why your reading won't match exactly {#tolerance}

No part is exactly its label, and the lessons' numbers are predictions,
so they say *about*:

- **Resistors** may be 1 % or 5 % off, and **electrolytic capacitors**
  20 %, so an RC time can be a fifth longer or shorter.
- **USB** gives anything from about 4.75 V to 5.25 V, and every voltage
  in the circuit scales with it.
- **LEDs** of one color differ by a tenth of a volt or more, which moves
  the voltage left for their resistor.
- **The meter** draws a little current itself, through about 10 MΩ. That
  is nothing beside a 1 kΩ divider, but a few percent beside E21's
  200 kΩ.

A reading within a tenth or so of the prediction agrees with it; one
that is twice or half as big points to a mistake in the build or the sum.

## Check yourself

1. Write 4.7 kΩ in ohms, and 0.36 mA in amps.
2. E08 charges a 1000 µF capacitor through 10 kΩ. What is R × C, in
   seconds?
3. A wave repeats 1000 times a second. How long is one cycle, in
   microseconds?
4. How much current does 3.3 V push through 2 kΩ?
5. A signal arrives at −100 dBm and the receiver can hear −130 dBm. How
   many times weaker could the signal get before it is lost?

??? note "Answers"
    1. 4.7 kΩ = 4700 Ω, and 0.36 mA = 0.00036 A.
    2. 10 kΩ × 1000 µF = 10 000 ms, which is 10 s.
    3. 1 ÷ 1000 Hz = 0.001 s = 1 ms = 1000 µs.
    4. 3.3 V ÷ 2 kΩ = 1.65 mA.
    5. It has 30 dB to spare, which is 10 × 10 × 10: a thousand times.

## Lessons that rely on it

<!-- relied on -->
