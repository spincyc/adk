# Follow the nodes

A schematic shows **which points are connected**. A breadboard shows
**where the parts fit**. Two holes in one breadboard metal strip are one
electrical node even if a schematic draws them far apart. The five holes
from f through j in a numbered column share a strip; the five from a
through e share another. The middle gap separates those strips. Power
rails are separate strips until a wire joins them.

These text marks are quick reminders. The labeled circuit drawing below
uses standard rectangular resistor symbols and shows its connections.

| Text sketch | Read it as | In these investigations |
|---|---|---|
| `—[1 kΩ]—` | A resistor between two nodes | Limits LED current or divides voltage. |
| `—|>|—` | A diode or LED pointing from anode toward cathode | The LED's short leg and a diode's band mark the cathode. |
| `—||—` | A capacitor between two nodes | A polarized capacitor's marked − leg goes where its lesson says. |
| `GND` | The reference node | The Mega GND wire reaches the grounded − rail. |
| `●` | A joined node | Every branch drawn from it shares a voltage. |

Follow an LED path from the supply through its **series resistor and LED**
back to GND. Never bypass the resistor. A line crossing another line is
joined only if the drawing or connection list says so.

## Trace one divider

Read this schematic first, then use
[E04's breadboard drawing and connection list](../lessons/059-resistors-in-series/index.md)
to find its three labeled nodes. The rectangle is a resistor; a line
joins its end to a node. The dot at the tap names the shared connection.

<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 620 190" width="620"
     role="img" aria-labelledby="divider-title divider-desc">
  <title id="divider-title">Two-resistor voltage divider schematic</title>
  <desc id="divider-desc">Five volts passes through resistor R1, one kiloohm, to a tap. Resistor R2, one kiloohm, connects the tap to ground.</desc>
  <g fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round">
    <path d="M50 90 H135 M245 90 H310 M310 90 H365 M475 90 H550 V135"/>
    <rect x="135" y="72" width="110" height="36"/>
    <rect x="365" y="72" width="110" height="36"/>
    <circle cx="310" cy="90" r="5" fill="currentColor"/>
    <path d="M310 90 V48 M529 135 H571 M536 144 H564 M544 153 H556"/>
  </g>
  <g fill="currentColor" font-size="17" font-family="system-ui, sans-serif">
    <text x="32" y="64">5 V</text>
    <text x="162" y="62">R1 · 1 kΩ</text>
    <text x="392" y="62">R2 · 1 kΩ</text>
    <text x="285" y="37">Tap</text>
    <text x="580" y="147">GND</text>
  </g>
</svg>

1. Point to the supply node at **j4**. Which resistor leg shares its
   strip? Trace back to the top + rail.
2. Point to the tap at **f7**. Find the end of the first resistor and
   the start of the second on the same upper strip of column 7. Predict
   its voltage relative to GND.
3. Point to the return at **f10**. Follow its wire to the grounded
   bottom − rail. With USB unplugged, check that no wire skips either
   resistor from 5 V to GND.

**Check:** The first resistor runs i4–i7, the second g7–g10, and the
shared column 7 strip is the tap. Equal 1 kΩ resistors should put it
near 2.5 V on a nominal 5 V supply. The exact voltage depends on the
USB supply and resistor values. You can now add a branch at that same
node in the [loaded-divider challenge](challenges.md#loaded-divider).
