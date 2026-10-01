# Laws and formulas

The lessons bring in each idea by building and measuring it: you see an
LED dim before anyone names Ohm's law. These pages state the laws
directly, work each one through with the course's own circuits, and list
every lesson that relies on it, linked to the section that does. Each of
those sections ends with a link back here.

Nothing here is a prerequisite: every lesson still explains what it
needs. Come here when a lesson's number puzzles you, or after a lesson to
see where else its idea turns up. If you are working through the
electricity course, do the investigations first: these pages give away
what they find.

## Three laws do most of the work {#three-laws}

For any circuit of resistors on a steady supply, three laws are enough to
find every voltage and current:

| Law | Says |
|---|---|
| [Ohm's law](ohms-law.md) | The voltage across a resistor is the current through it times its resistance: V = I × R. |
| [Kirchhoff's voltage law](kirchhoff.md#voltage-law) | Around any loop, the parts' voltages add up to the supply's. |
| [Kirchhoff's current law](kirchhoff.md#current-law) | At any junction, the currents in add up to the currents out. |

Everything else here is one of three things: a shortcut built from those
three, such as [series and parallel resistors](kirchhoff.md#series) and
[dividers](dividers.md); the law for a part that isn't a resistor, such
as a [capacitor](capacitors-and-coils.md), a [diode](diodes-transistors-op-amps.md)
or a [logic input](digital.md); or [power](power.md), which says how much
heat a part must handle.

## How to work out a circuit {#how-to}

1. **Trace it.** Name each node: a strip of holes together with every
   strip a wire joins to it, as
   [Read a schematic](../electricity/schematics.md#follow-the-nodes) shows.
2. **Write down what you know:** the supply voltage, each resistance, and
   any part that keeps a fixed voltage, such as a red LED's 2 V.
3. **Share out the voltage** around each loop with the voltage law.
4. **Turn each resistor's voltage into a current** with Ohm's law.
5. **Add currents** where paths meet with the current law.
6. **Check:** the power in each part, every current against its limit,
   and whether the answer is sensible.

**Worked: Lesson 1's LED.** 5 V from the pin, less the LED's 2 V, leaves
3 V for the 220 Ω resistor. 3 V ÷ 220 Ω ≈ 14 mA, under a pin's 20 mA,
and the resistor turns 3 V × 14 mA ≈ 0.04 W into heat, far under its
rating.

## Every law {#every-law}

Each page's laws, what each says, and the first investigation and project
lesson that rely on it.

<!-- laws -->

The [glossary](../glossary.md) has the words in brief, and
[Measurement skills](../electricity/skills.md) how to take the readings
these laws explain. Lesson-specific sums, such as the speed of sound in
Lesson 19 or a stepper's steps, stay in their own lessons.
