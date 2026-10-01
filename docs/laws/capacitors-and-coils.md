# Capacitors and coils

Resistors respond at once. Capacitors and coils take time: a capacitor's
voltage can't jump, and a coil's current can't. That delay makes timers,
smoothing filters and steady supplies, and it is why a coil needs a
diode. If you are working through the electricity course, do E07 and E08
first: this page gives away what they find.

## A capacitor stores charge {#capacitor}

<!-- law capacitor -->

A capacitor is two metal plates kept apart by a thin insulator. Current
can't cross the gap, but charge can gather on one plate and leave the
other, and the more charge it holds, the higher its voltage. Its **capacitance** C, in farads, is the charge it holds for each volt.
E07's 1000 µF at 5 V holds 0.001 F × 5 V = 0.005 coulombs. To change its
voltage, charge must flow in or out, so a current only flows **while the
voltage changes**:

<p class="formula">I = C × <span class="fraction"><span>change in V</span><span>time it takes</span></span></p>

- Once charged, it passes no steady current: it **blocks DC**.
- Its voltage can't jump: a sudden change would need an endless current.
  That is what makes it useful for smoothing.
- It keeps its charge after the supply goes, which is why E07 can
  disconnect it and still read its voltage, and why every page says
  never to short a capacitor's legs.

Electrolytic capacitors, the large ones in cans, have a + and a − leg
and a highest safe voltage printed on them; fit them the right way round.
Small ceramic ones have no polarity.

<!-- drawing 062-charge-a-capacitor closeup -->

## Charging through a resistor: τ = R × C {#rc-time}

<!-- law rc-time -->

Charge a capacitor through a resistor and it fills fast at first, then
more slowly as its voltage nears the supply's, because there is less
voltage left across the resistor to push current. How long it takes is
set by the **time constant**, τ (the Greek letter tau):

| After | The capacitor has gone | E08's 10 kΩ and 1000 µF, from 0 to 5 V |
|---|---|---|
| 1 τ | 63 % of the way | 10 s, 3.2 V |
| 2 τ | 86 % | 20 s, 4.3 V |
| 3 τ | 95 % | 30 s, 4.75 V |
| 5 τ | over 99 % | 50 s, all but a few hundredths of a volt |

Discharging is the same curve upside down: after one τ, 37 % is left.
With [the shortcut](units.md#shortcuts) kilohms × microfarads gives
milliseconds, so 10 kΩ × 1000 µF = 10 000 ms. Double the resistor, as E08
does, and every time doubles.

??? note "The curve as a formula"

    Charging from 0 V toward a supply V<sub>s</sub>, after a time t:

    <p class="formula">V = V<sub>s</sub> × (1 − e<sup>−t ÷ τ</sup>)</p>

    e is about 2.718. At t = τ, 1 − e<sup>−1</sup> ≈ 0.63.

The same curve sets other timings in the course:

- **E21's clock** charges and discharges 10 µF through 100 kΩ, τ = 1 s,
  between a Schmitt trigger's two thresholds (see
  [Logic levels](digital.md#schmitt)).
- **E23's filter** turns PWM into a steady voltage through 10 kΩ and
  100 µF, τ = 1 s, so it settles in a few seconds.
- **E15's filter**, 1 kΩ and 1 µF, has τ = 1 ms, which is why it follows
  a slow wave and smooths a fast one (see [Waves](signals.md#filter)).

## Supply capacitors {#decoupling}

A chip draws its current in short gulps each time its outputs switch.
The wires from the supply have a little resistance, and by Ohm's law each
gulp makes the chip's supply dip. A **supply capacitor**, often 100 nF,
placed right across the chip's power pins, holds charge close by and
gives the gulps itself, so the dip never reaches the chip. A larger
electrolytic, such as 10 µF or 100 µF, covers longer dips.

E18 has you measure the dip and the cure. It predicts that a 10 Ω feed
drops the local rail by about 0.13 V whenever an LED switches on, and
that 100 µF with 100 nF shrink a 1 kHz ripple to about 30 mV, though not
a slower 100 Hz one. That is why E16, E19, E20 and E21
put a capacitor beside each chip.

## A coil resists a change in current {#inductor}

<!-- law inductor -->

A coil of wire, an **inductor**, makes a magnetic field when current
flows through it. Changing the current means changing the field, and the
coil pushes back with a voltage:

<p class="formula">V = L × <span class="fraction"><span>change in I</span><span>time it takes</span></span></p>

Its **inductance** L is in henrys. Where a capacitor's voltage can't jump,
a coil's **current** can't: switched on, it rises gradually, with its own
time constant, L ÷ R. E11's 100 mH coil with 1 kΩ: 0.1 H ÷ 1000 Ω = 0.1 ms, which the scope
shows as a rounded edge where a plain wire gives a sharp one.

## Flyback: why a coil needs a diode {#flyback}

<!-- law flyback -->

Switch a coil off suddenly and its current still wants to keep flowing.
With nowhere to go, the change is very fast, so V = L × change ÷ time is
very large: the coil makes a spike of tens of volts or more, enough to
damage the transistor or pin that switched it.

A **flyback diode** across the coil, pointing back toward the supply,
gives that current a loop to die away in, and the spike stays within
about 0.7 V of the supply. Every coil the course switches off with a
transistor or driver has one: the buzzer in Lesson 3 and E12, the relay
module's own in Lesson 35, and the driver chips' own in Lessons 20 and 31.
Lesson 5's passive buzzer, run straight from a pin, needs none: when the
pin goes LOW it joins the coil to GND, so its current, kept small by the
220 Ω, always has a path.

## Check yourself

1. How long is τ for 100 kΩ and 10 µF? For 2 kΩ and 100 nF?
2. E08 charges toward 5 V. About what does the meter read after 30 s
   with one 10 kΩ resistor?
3. Why would a capacitor with no resistor in front of it not show E08's
   slow rise?
4. Why must a flyback diode be fitted the right way round?

??? note "Answers"
    1. 100 kΩ × 10 µF = 1000 ms = 1 s. 2 kΩ × 100 nF = 2 kΩ × 0.1 µF =
       0.2 ms.
    2. 30 s is three time constants, so about 95 % of 5 V, about 4.75 V.
    3. With almost no resistance, τ = R × C is almost zero: it charges
       almost at once, limited only by the supply.
    4. Fitted backwards, it would conduct whenever the coil is switched
       on, carrying current around the coil instead of through it; with a
       coil wired straight to the supply, that is a short through the
       diode and transistor. And it would do nothing for the spike.
       Pointing toward the supply, it conducts only the coil's own
       current, once the switch opens.

## Lessons that rely on them

<!-- relied on -->
