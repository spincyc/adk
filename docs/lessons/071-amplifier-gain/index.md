---
lesson: 71
promise: Make a small sine wave about twice as tall with two feedback resistors.
time: 30 minutes
level: 3
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - MCP6002 in an 8-pin PDIP package
  - 5 × 10 kΩ resistors (brown, black, black, red, brown), and a sixth
    for the comparison
  - 100 nF ceramic capacitor
  - 10 µF electrolytic capacitor rated at least 10 V
  - Isolated, battery-powered 0–4 V waveform generator
  - Battery-powered two-channel oscilloscope with two probes
  - 11 jumper wires
ideas:
  - Feedback resistors set the gain of a non-inverting amplifier
---

## What you'll build

<!-- closeup -->

An MCP6002 amplifier takes a small sine wave from an isolated generator,
through a 10 kΩ input resistor. Two equal **10 kΩ feedback resistors** set
its gain to about two. On the
scope, its output wave should be about twice as tall as the input wave.
The Mega supplies USB 5 V but uses no signal pins.

## Predict

Set the generator to a **100 Hz sine wave from 0.5 V to 1.5 V**. What
lowest and highest voltages would you expect if the amplifier doubled
both numbers? Write down your prediction before you switch the circuit on.

## Build it

!!! warning "Power off before wiring"
    Unplug the Mega's USB cable and switch off and unplug the generator
    before moving parts or wires. Use only the Mega's USB 5 V supply and
    an isolated, battery-powered generator whose signal stays between 0 V
    and 5 V. Never send a negative voltage to the chip or a generator
    signal to a Mega input. Keep the generator output off whenever USB
    power is disconnected. Connect both scope ground clips only to the
    common bottom − rail.

Take out E15's filter parts and their jumpers, and lift the generator's blue
OUT lead. Keep the generator, its black GND lead, and the Mega's GND wire in
the bottom − rail hole nearest the Mega. The generated steps add the Mega's
5 V wire to the top + rail hole nearest it. With the MCP6002's notch
pointing left, pin 1 is its output A, pin 2 its − input A, pin 3 its + input
A, pin 4 GND, pin 5 the second amplifier's + input, pin 6 its − input, pin 7
its output, and pin 8 its 5 V supply. Check the chip marking before
powering.

Follow the generated steps. Pins 8 and 4 sit at opposite corners of the
chip, so the **100 nF and 10 µF supply capacitors** stand across the top +
and − rails: the 100 nF right beside pin 8's supply wire, and the 10 µF
two columns to its left, with its + leg on 5 V and striped − leg on GND.
A black wire at column 21, close to pin 4, joins the top − rail to GND. Amplifier A's three resistors lie just below the chip. The
generator reaches pin 3 only through the **10 kΩ input resistor**: the
op-amp's input takes almost no current, so the resistor costs nothing
here, but if the generator were ever on while the chip had no power it
would keep the current into pin 3 below about 0.4 mA. The second
amplifier is held steady: two other 10 kΩ resistors make a midpoint near
2.5 V for its + input, while its output joins its − input.

<!-- bench -->

<!-- steps -->

In schematic form, with the chip's pin numbers (the
[schematic key](../../electricity/schematics.md) names each symbol):

<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 620 420" width="620"
     role="img" aria-labelledby="gain-two-title gain-two-desc">
  <title id="gain-two-title">Non-inverting amplifier schematic</title>
  <desc id="gain-two-desc">The generator feeds the MCP6002's + input A, pin 3, through a 10 kilohm input resistor. Output A, pin 1, feeds back through a 10 kilohm resistor to the − input A, pin 2, and another 10 kilohm resistor joins pin 2 to ground. Channel 1 watches pin 3 and channel 2 watches pin 1.</desc>
  <g fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round">
    <path d="M300 90.0 L410 150 L300 210.0 Z"/>
    <path d="M310 180 H324"/>
    <path d="M310 120 H324 M317 113 V127"/>
    <circle cx="130" cy="200" r="24"/>
    <path d="M117 200 q6.5 -12 13 0 q6.5 12 13 0"/>
    <path d="M130 176 L130 120 L150 120"/>
    <path d="M150 120 L165.0 120"/>
    <path d="M235.0 120 L250 120"/>
    <rect x="165.0" y="107" width="70" height="26"/>
    <path d="M250 120 L300 120"/>
    <path d="M410 150 L480 150"/>
    <path d="M480 150 L480 250"/>
    <path d="M260 250 L335.0 250"/>
    <path d="M405.0 250 L480 250"/>
    <rect x="335.0" y="237" width="70" height="26"/>
    <path d="M300 180 L260 180 L260 250"/>
    <path d="M260 250 L260 260.0"/>
    <path d="M260 330.0 L260 340"/>
    <rect x="247" y="260.0" width="26" height="70"/>
    <path d="M260 340 V348 M240 348 H280 M247 356 H273 M254 364 H266"/>
    <path d="M130 224 L130 340"/>
    <path d="M130 340 V348 M110 348 H150 M117 356 H143 M124 364 H136"/>
  </g>
  <g fill="currentColor">
    <circle cx="250" cy="120" r="5"/>
    <circle cx="480" cy="150" r="5"/>
    <circle cx="260" cy="250" r="5"/>
  </g>
  <g fill="currentColor" font-size="17" font-family="system-ui, sans-serif">
    <text x="318" y="76">MCP6002 A</text>
    <text x="16" y="196">generator</text>
    <text x="16" y="216">0.5–1.5 V</text>
    <text x="200" y="158" text-anchor="middle">10 kΩ</text>
    <text x="200" y="178" text-anchor="middle" font-size="15">input</text>
    <text x="242" y="96" text-anchor="middle">pin 3 · ch 1</text>
    <text x="480" y="126" text-anchor="middle">pin 1 · ch 2</text>
    <text x="370" y="288" text-anchor="middle">10 kΩ</text>
    <text x="250" y="236" text-anchor="end">pin 2</text>
    <text x="240" y="302" text-anchor="end">10 kΩ</text>
    <text x="330" y="360" font-size="15">Pin 8 to 5 V, pin 4 to GND, with 100 nF</text>
    <text x="330" y="380" font-size="15">and 10 µF across them. Amplifier B,</text>
    <text x="330" y="400" font-size="15">pins 5–7, holds 2.5 V; not drawn.</text>
  </g>
</svg>

The generator's OUT reaches **+ input A (pin 3)** through the input
resistor. One feedback resistor joins **output A (pin 1)** to **− input A
(pin 2)**, through the short jumper from pin 1's strip; the other joins
pin 2 to GND. Check these three nodes against the generated connection
list:

<!-- connections -->

## Compare input and output

The two probes go as drawn here: each tip in a free hole, each ground clip
in a free hole of the bottom − rail.

<!-- probe -->

1. With both power sources still off, clip **both scope ground leads** to
   the bottom − rail, by columns 17 and 15. Put channel 1's tip in **d17**,
   in pin 3's strip, and channel 2's tip in **b15**, in pin 1's strip.
   Keep the metal tips apart. Set both channels to
   **DC coupling** and start near **2 ms/div** and **0.5 V/div**.
2. Check the generator is set to a **0.5–1.5 V sine wave at 100 Hz** with
   its output off. Plug in USB, then turn on the generator. Record the
   lowest and highest voltage on each channel. Channel 1 should span
   about **0.5–1.5 V**; channel 2 should span about **1–3 V**.
3. Compare your readings with the prediction. Are the two waves in step?
   Is channel 2 roughly twice channel 1 at a peak and at a trough? Turn
   off the generator output before unplugging USB.

## Why it happens

The amplifier raises its output until the voltage fed back to its − input
is close to the voltage at its + input. The two equal resistors split the
output voltage in half before it reaches that − input. So the output needs
to be about **twice the input**: gain = 1 + 10 kΩ ÷ 10 kΩ = **2**. This is
how a sensor with a small output voltage can be made easier to measure.
[Lesson 8's light meter](../008-light-meter/index.md) needs no amplifier:
its photoresistor divider already swings across a large part of the
0–5 V range.

The actual peaks may be a little different. Resistors have tolerances,
the generator may not deliver its exact setting, and the chip has a small
input error. Compare the **measured** output range with twice the
**measured** input range, rather than expecting perfect numbers.

## Check your result

The output should follow the input at about twice its voltage throughout
the wave. If its trough is near 1 V and its peak near 3 V while the input
runs near 0.5–1.5 V, the feedback resistors are setting gain near two.
Record your own four endpoint readings; this page gives predictions, not
a recorded hardware result.

## Change one thing

Predict first: if the resistor from output A to pin 2 becomes **20 kΩ**,
twice as large, what range will the output cover for the same 0.5–1.5 V
input? Write it down.

1. Turn off the generator output, then unplug USB.
2. Move the **a13** end of the short jumper from pin 1's strip to
   **a10**. Lay the sixth 10 kΩ resistor along row b from **b10 to b13**.
   Output A now reaches pin 2 through two 10 kΩ resistors in series.
3. Plug in USB, turn on the generator with the same settings, and record
   both channels' lowest and highest voltages again.

Gain = 1 + 20 kΩ ÷ 10 kΩ = **3**, so the output should span about
**1.5–4.5 V**: three times the input, and still inside the 0–5 V supply.
A larger input would push the output into the 5 V rail, where it flattens
the tops of the wave. Turn off the generator, unplug USB, then take out the
extra resistor and put the jumper's end back in a13.

## If the traces surprise you

| What you see | Check with USB unplugged and generator off |
|---|---|
| Output stays near 0 V or 5 V | Check the MCP6002 notch, pin 8 to 5 V, pin 4 to GND, and both 10 kΩ feedback paths at pin 2. |
| Input appears but output is about the same size | Check that the output-to-pin-2 resistor and pin-2-to-GND resistor are separate paths, not a wire around a resistor. |
| Both traces look flat | Check generator OUT and GND, its 100 Hz sine setting, the scope's DC coupling, and the timebase. |
| Trace changes when a scope clip moves | Put both ground clips on the common bottom − rail and check the generator's black GND lead reaches it. |

## About the sketch

The analog circuit does all the work here, without code. The matching ADK
sketch claims no Mega signal pins and needs no upload:

<!-- sketch -->

The stated voltages follow from the circuit design and datasheet; no
hardware trial has been recorded for this lesson. The [MCP6002 datasheet](https://ww1.microchip.com/downloads/en/devicedoc/mcp6001-1r-1u-2-4-1-mhz-low-power-op-amp-ds20001733l.pdf)
gives its PDIP pinout and recommends a local 0.01–0.1 µF bypass capacitor
plus at least 1 µF of bulk capacitance.
