# When the result surprises you

Begin with the exact symptom, then trace the circuit one node at a time.
The [linked lesson](index.md) gives its own hole-by-hole checks.

1. **Unplug USB or switch off the generator.** Keep an LED's resistor
   and any protective diode in place. Check part orientation, loose legs,
   the rail feeds and the intended path back to GND.
2. **Name the node you expected to change.** Use the
   [schematic key](schematics.md) and the lesson's connection list to
   locate its breadboard strip. Check that the probe touches that strip,
   not the one across the middle gap.
3. **Check the meter setup.** For these voltage checks: black lead in COM,
   red in V, DC volts selected. Put black at the circuit's GND and red
   at the named node. Keep probe tips apart.
4. **Power and read one point at a time.** Compare supply, input and
   output with your prediction. Unplug before changing any wire or part.

| Symptom | First safe check | What that check separates |
|---|---|---|
| LED stays dark | Unplug; trace supply → resistor → LED → GND and check the LED's short leg. | An open path or reversed LED from a wrong current prediction. |
| A voltage is near 0 V when you expected a middle value | Unplug; check the probe strip and the wires to 5 V and GND. | A misplaced probe or missing supply from a divider error. |
| A voltage is near 5 V when you expected a middle value | Unplug; check the path from that node through its lower resistor to GND. | A missing return from a changed resistor ratio. |
| A meter number alternates | Wait for the circuit to settle; in a blinking lesson, read only during the stated on interval. | Timing or contact movement from a steady reading. |
| A part gets warm or smells | Unplug at once; check for a direct + to − path, bypassed resistor or reversed electrolytic capacitor. | A wiring fault that needs correction before any more measurements. |

## Checkpoint: diagnose a divider

In [E04](../lessons/059-resistors-in-series/index.md), predict the
middle voltage and measure it with red on **f7** and black on the
grounded − rail. Suppose it reads nearly 5 V. With USB **unplugged**,
trace from f7 through the second 1 kΩ resistor at g7–g10 and the wire
from f10 to the bottom − rail. Which missing connection would leave the
tap high? Restore the intended path, power again, and remeasure. A
reading near 2.5 V supports the repaired path; it is still your measured
number that belongs in your record.

Use the same sequence for the [loaded-divider challenge](challenges.md#loaded-divider):
measure before and after the change, then explain the direction of the
new reading.
