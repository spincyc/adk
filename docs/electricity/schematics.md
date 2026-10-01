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

## Symbols in the drawings

E12, E15, E16, E17, E18, E19, E20 and E21 each draw their circuit with
these symbols, labeled with the values, chip pins and holes their pages
use.

<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 620 410" width="620"
     role="img" aria-labelledby="symbol-key-title symbol-key-desc">
  <title id="symbol-key-title">Schematic symbol key</title>
  <desc id="symbol-key-desc">Fifteen symbols with their names: resistor, capacitor, electrolytic capacitor with its + side marked, diode with its band as a bar, LED with two arrows, coil, NPN transistor, push button, knob, op-amp, NAND gate, Schmitt inverter, signal generator, ground, and a dot for a joined node.</desc>
  <g fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round">
    <path d="M12 62 L27.0 62"/>
    <path d="M97.0 62 L112 62"/>
    <rect x="27.0" y="49" width="70" height="26"/>
    <path d="M186 22 L186 56.0"/>
    <path d="M186 68.0 L186 102"/>
    <path d="M166 56.0 H206"/>
    <path d="M166 68.0 H206"/>
    <path d="M316 22 L316 56.0"/>
    <path d="M316 68.0 L316 102"/>
    <path d="M296 56.0 H336"/>
    <path d="M296 74.0 Q316 64.0 336 74.0"/>
    <path d="M316 69.0 L316 102"/>
    <path d="M389 62 L421.0 62.0"/>
    <path d="M447.0 62.0 L479 62"/>
    <path d="M421.0 77.0 L421.0 47.0 L447.0 62.0 Z"/>
    <path d="M447.0 77.0 L447.0 47.0"/>
    <path d="M513 68 L545.0 68.0"/>
    <path d="M571.0 68.0 L603 68"/>
    <path d="M545.0 83.0 L545.0 53.0 L571.0 68.0 Z"/>
    <path d="M571.0 83.0 L571.0 53.0"/>
    <path d="M552.0 88.0 L560.0 104.0"/>
    <path d="M560.0 104.0 L554.1 100.2"/>
    <path d="M560.0 104.0 L560.5 97.0"/>
    <path d="M564.0 88.0 L572.0 104.0"/>
    <path d="M572.0 104.0 L566.1 100.2"/>
    <path d="M572.0 104.0 L572.5 97.0"/>
    <path d="M62 147 L62 153.5"/>
    <path d="M62 153.5 a9 9 0 0 1 0 18 a9 9 0 0 1 0 18 a9 9 0 0 1 0 18 a9 9 0 0 1 0 18"/>
    <path d="M62 225.5 L62 232"/>
    <path d="M141 192 L159 192"/>
    <path d="M159 172 V212"/>
    <path d="M159 183 L211 162 L211 147"/>
    <path d="M159 201 L211 222 L211 237"/>
    <path d="M187.6 212.6 L177.7 214.1"/>
    <path d="M187.6 212.6 L181.6 204.6"/>
    <circle cx="187.0" cy="192" r="30"/>
    <path d="M265 202 L279 202"/>
    <path d="M341 202 L355 202"/>
    <circle cx="283" cy="202" r="4"/>
    <circle cx="337" cy="202" r="4"/>
    <path d="M275 188 H345 M310.0 188 V174 M302.0 174 H318.0"/>
    <path d="M426 152 L426 157.0"/>
    <path d="M426 227.0 L426 232"/>
    <rect x="413" y="157.0" width="26" height="70"/>
    <path d="M474 192.0 L442 192.0"/>
    <path d="M442 192.0 L450.8 196.8"/>
    <path d="M442 192.0 L450.8 187.2"/>
    <path d="M523 138.0 L593 188 L523 238.0 Z"/>
    <path d="M533 158 H547"/>
    <path d="M533 218 H547 M540 211 V225"/>
    <path d="M508 158 L523 158"/>
    <path d="M508 218 L523 218"/>
    <path d="M593 188 L608 188"/>
    <path d="M20 294.0 H55 A28.0 28.0 0 0 1 55 350.0 H20 Z"/>
    <circle cx="89.0" cy="322" r="6"/>
    <path d="M8 308 L20 308"/>
    <path d="M8 336 L20 336"/>
    <path d="M95.0 322 L105.0 322"/>
    <path d="M148 294.0 L208 322 L148 350.0 Z"/>
    <circle cx="214" cy="322" r="6"/>
    <path d="M154 330 H170 V314 H178 M162 330 V314 H178"/>
    <path d="M136 322 L148 322"/>
    <path d="M220 322 L230 322"/>
    <circle cx="310" cy="322" r="24"/>
    <path d="M297 322 q6.5 -12 13 0 q6.5 12 13 0"/>
    <path d="M434 292 L434 314"/>
    <path d="M434 314 V322 M414 322 H454 M421 330 H447 M428 338 H440"/>
    <path d="M518 322 L598 322"/>
    <path d="M558 322 L558 352"/>
  </g>
  <g fill="currentColor">
    <circle cx="558" cy="322" r="5"/>
  </g>
  <g fill="currentColor" font-size="17" font-family="system-ui, sans-serif">
    <text x="62" y="126" text-anchor="middle" font-size="15">resistor</text>
    <text x="186" y="126" text-anchor="middle" font-size="15">capacitor</text>
    <text x="310" y="126" text-anchor="middle" font-size="15">electrolytic</text>
    <text x="434" y="126" text-anchor="middle" font-size="15">diode</text>
    <text x="558" y="126" text-anchor="middle" font-size="15">LED</text>
    <text x="62" y="256" text-anchor="middle" font-size="15">coil</text>
    <text x="186" y="256" text-anchor="middle" font-size="15">NPN transistor</text>
    <text x="310" y="256" text-anchor="middle" font-size="15">push button</text>
    <text x="434" y="256" text-anchor="middle" font-size="15">knob</text>
    <text x="558" y="256" text-anchor="middle" font-size="15">op-amp</text>
    <text x="62" y="386" text-anchor="middle" font-size="15">NAND gate</text>
    <text x="186" y="386" text-anchor="middle" font-size="15">Schmitt inverter</text>
    <text x="310" y="386" text-anchor="middle" font-size="15">generator</text>
    <text x="434" y="386" text-anchor="middle" font-size="15">ground</text>
    <text x="558" y="386" text-anchor="middle" font-size="15">joined node</text>
    <text x="282" y="54.0" font-size="16">+</text>
  </g>
</svg>

A diode's bar is its band, the cathode; an LED is a diode with two arrows
for its light. On an electrolytic capacitor the curved plate is the
striped − leg. The transistor's arrow marks its emitter, and its base is
the flat bar. An op-amp's − and + mark its two inputs, and its point is
the output. A small circle on a gate's output means *not*: a NAND gate is
an AND gate with its answer turned over. The Schmitt inverter's mark
inside shows its two switching thresholds.

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
