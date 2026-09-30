# Measure, calculate, explain

Use this short routine for each [electricity investigation](index.md). The
linked lesson gives the exact probe holes and the expected range. Write your
prediction before looking at its result.

## Set up a voltage reading

1. Unplug USB or turn the generator off before placing or moving wires.
   Put the black meter lead in **COM** and the red lead in **V**. Select
   **DC volts (V⎓)**; use a range above 5 V if the meter has ranges.
2. Put one probe on each side of the thing you want to measure. For a
   point's voltage relative to GND, put black on the grounded − rail and
   red on that point. Keep the metal probe tips apart.
3. Power the circuit, wait for a steady reading, and write down the value
   **with its units and probe positions**. A minus sign means red is at
   the lower voltage; check the probe positions before changing wiring.

The meter reads a voltage **between** its probes. To find current in a
series resistor, keep the meter on DC volts, measure across that resistor,
then calculate **current = resistor voltage ÷ resistance**. For example,
3.0 V across 220 Ω gives about 0.014 A, or 14 mA. Do not put meter leads
set to current across a supply. [E02](../lessons/057-measure-across-and-through/index.md)
walks through the voltage readings and current calculation.

## Checkpoint: one LED path

In E02, predict the voltage across the steady LED and resistor. Measure
each and the whole path. Record your own three readings:

| Across resistor | Across LED | Whole path | Sum of first two |
|---:|---:|---:|---:|
| ____ V | ____ V | ____ V | ____ V |

The two part readings should add close to the whole-path reading. Use the
resistor reading to calculate the current. If the sum is far off, check
that all readings were taken at the lesson's marked holes, with the
leads still in the voltage sockets. Then
use the [diagnosis guide](diagnose.md) to trace the path.

## Keep a useful record

For each trial, write: **I changed ___; I held ___ fixed; I predicted
___; I measured or saw ___; I now think ___ because ___.** Record an
approximate number when a number matters. A dark LED or missing trace is
an observation too; check the build before treating it as a result.

Next, try the [schematic-to-breadboard checkpoint](schematics.md#trace-one-divider)
and the [loaded-divider challenge](challenges.md#loaded-divider).
