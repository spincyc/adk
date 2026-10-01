---
lesson: 72
promise: Turn a knob and watch an op-amp output follow it while feeding a load.
time: 25 minutes
level: 3
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - MCP6002 in an 8-pin PDIP package
  - 2 × 10 kΩ resistors (brown, black, black, red, brown)
  - 1 kΩ load resistor (brown, black, black, brown, brown)
  - 10 kΩ potentiometer from the kit
  - 100 nF ceramic capacitor
  - 10 µF electrolytic capacitor rated at least 10 V
  - Battery-powered two-channel oscilloscope or DC voltmeter
  - 6 more jumper wires, or 16 in all if you start here
ideas:
  - Negative feedback makes an output follow an input while feeding a load
laws:
  - {law: ohms-law, section: why-it-happens, for: "Gives the 2 mA the follower supplies to 1 kΩ"}
  - {law: parallel, section: why-it-happens, for: "Combines the load and lower track into 800 Ω"}
  - {law: divider, section: why-it-happens, for: "Places 2.0 V at 40% along the knob's track"}
  - {law: loading, section: why-it-happens, for: "Predicts the wiper's fall to 0.59 V under 1 kΩ"}
  - {law: op-amp, section: why-it-happens, for: "Explains the follower holding 2.0 V under load"}
  - {law: sampling, section: try-it, for: "Expects A0 near 410 for a 2.0 V wiper"}
---

## What you'll build

<!-- closeup -->

Keep [E16's MCP6002 amplifier](../071-amplifier-gain/index.md),
its supply capacitors, and its second amplifier's steady connection.
Change amplifier A into a **voltage follower**: its output connects
straight back to its − input. A knob sets the + input; a 1 kΩ resistor
loads the output. First you hang that load straight on the knob, then on
the follower, and compare.

## Predict

Set the knob's wiper to about **2 V** with nothing drawing current from
it. If you connect a 1 kΩ resistor from the wiper straight to GND, will
the wiper stay near 2 V or fall much lower? If the same resistor hangs on
the follower's output instead, will that output stay near 2 V? Write down
both guesses.

## Build it

!!! warning "Unplug before changing the circuit"
    Unplug the Mega's USB cable, turn off the generator, and disconnect
    its two wires before moving any parts. The MCP6002 uses **5 V and
    GND**, never a negative supply. Check its notch, the 10 µF
    capacitor's + leg at 5 V and striped − leg at GND, and the nearby
    100 nF capacitor before restoring power. Keep scope ground clips on
    the common bottom − rail.

Keep the MCP6002 across the middle gap with its notch to the left,
pin 1 at column 15. Keep pin 8 at 5 V and pin 4 at GND, both supply
capacitors and the black wire joining the − rails at column 21, and the
two 10 kΩ resistors that hold amplifier B's + input at a midpoint. Keep
B's output (pin 7) joined to its − input (pin 6). Keep the Mega's GND
and 5 V wires in their usual rail holes nearest it.

Remove E16's generator, its two wires and the 10 kΩ input resistor.
Remove amplifier A's two feedback resistors and the short jumper from pin
1's strip. Connect A's output (pin 1) directly to its − input (pin 2).
Connect the kit knob's wiper to A's + input (pin 3), with the knob's
outer legs at GND and 5 V. The 1 kΩ load stands from **a12** into the
bottom − rail, and a short jumper from **b15**, in pin 1's strip, to
**b12** connects it to A's output. The knob stays at its usual home and
also connects to A0 so the sketch can show its setting.

**Starting here with a meter and no generator?** First follow
[E16's steps](../071-amplifier-gain/index.md) for only two of its stages,
**the MCP6002 and its supply** and **amplifier B's steady midpoint**,
skipping its generator and amplifier A. That is everything the steps
below keep from E16; then add what they add.

<!-- bench -->

<!-- steps -->

In schematic form, with the chip's pin numbers (the
[schematic key](../../electricity/schematics.md) names each symbol):

<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 600 380" width="600"
     role="img" aria-labelledby="follower-title follower-desc">
  <title id="follower-title">Voltage follower and load schematic</title>
  <desc id="follower-desc">The knob runs from 5 volts to ground. Its wiper feeds the MCP6002's + input A, pin 3, and the Mega's A0. Output A, pin 1, is wired straight to the − input A, pin 2, and a 1 kilohm load joins pin 1 to ground.</desc>
  <g fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round">
    <path d="M110 54 L110 80"/>
    <path d="M110 80 L110 135.0"/>
    <path d="M110 205.0 L110 260"/>
    <rect x="97" y="135.0" width="26" height="70"/>
    <path d="M200 170.0 L126 170.0"/>
    <path d="M126 170.0 L134.8 174.8"/>
    <path d="M126 170.0 L134.8 165.2"/>
    <path d="M110 260 V268 M90 268 H130 M97 276 H123 M104 284 H116"/>
    <path d="M200 170 L200 110"/>
    <circle cx="200" cy="104" r="5"/>
    <path d="M200 170 L280 170"/>
    <path d="M280 140.0 L390 200 L280 260.0 Z"/>
    <path d="M290 230 H304"/>
    <path d="M290 170 H304 M297 163 V177"/>
    <path d="M390 200 L450 200"/>
    <path d="M450 200 L450 300 L250 300 L250 230 L280 230"/>
    <path d="M450 200 L530 200"/>
    <path d="M530 200 L530 225.0"/>
    <path d="M530 295.0 L530 320"/>
    <rect x="517" y="225.0" width="26" height="70"/>
    <path d="M530 320 V328 M510 328 H550 M517 336 H543 M524 344 H536"/>
  </g>
  <g fill="currentColor">
    <circle cx="200" cy="170" r="5"/>
    <circle cx="450" cy="200" r="5"/>
  </g>
  <g fill="currentColor" font-size="17" font-family="system-ui, sans-serif">
    <text x="92" y="44">5 V</text>
    <text x="40" y="176" text-anchor="start">knob</text>
    <text x="40" y="196" text-anchor="start">10 kΩ</text>
    <text x="214" y="110">A0</text>
    <text x="170" y="158" text-anchor="end" font-size="15">wiper</text>
    <text x="264" y="160" text-anchor="end" font-size="15">pin 3</text>
    <text x="300" y="126">MCP6002 A</text>
    <text x="450" y="176" text-anchor="middle">pin 1</text>
    <text x="242" y="236" text-anchor="end" font-size="15">pin 2</text>
    <text x="548" y="256">1 kΩ</text>
    <text x="548" y="276" font-size="15">load</text>
    <text x="40" y="350" font-size="15">Supply and amplifier B as in E16.</text>
  </g>
</svg>

Use the generated connections to check the three separate A nodes:
pin 3 with the wiper, pins 1 and 2 together, and the load's other end
at GND.

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 072-negative-feedback** in the
Arduino IDE. The analog feedback happens in the MCP6002. The sketch only
reads the knob on A0 and prints its position, to help you set it; it
does not drive the output:

<!-- sketch -->

## Try it

Use the scope or a meter. On the scope, put both ground clips on the
bottom − rail, channel 1's tip on a free hole in **pin 3's lower strip**
(the wiper's voltage) and channel 2's tip on a free hole in **pin 1's
lower strip** (the output), with DC coupling; flat traces at a steady
height are what to expect. With a meter, set DC volts, black lead in
**COM** and red lead in **V**, leave the black probe on GND, and touch the
red probe to those two strips in turn. Keep the metal tips apart.

1. **Set the knob.** Plug in USB, upload the sketch, and open
   **Tools → Serial Monitor** at **9600** baud: it prints the knob's A0
   reading ten times a second. As built, nothing draws current from the
   wiper: the op-amp's + input takes almost none. Turn the knob until the
   wiper reads about **2.0 V**, then leave it there. The output should
   read about 2.0 V too. Note the A0 number the sketch prints; it should
   be near **410**.
2. **Load the knob directly.** Unplug USB. Move only the **b15** end of
   the load wire (the one from b15 to b12) to **c40**, a free hole in the
   wiper's strip. The 1 kΩ load now hangs straight on the knob. Plug in
   USB without touching the knob. The wiper should drop to about
   **0.6 V**, and A0 to about **120**: the knob's own resistance cannot hold its
   voltage while 1 kΩ draws current from it.
3. **Load the follower.** Unplug USB and put that end back in **b15**.
   Plug in USB. The wiper returns to about 2.0 V, and the output, now
   feeding the same 1 kΩ load, should read about **2.0 V** as well.
4. Turn the knob to about **1 V** and **3 V** and record both readings
   at each setting, with the load on the output.

| Knob setting | Your wiper voltage | Your output voltage |
|---|---:|---:|
| 2.0 V, load on the wiper | ____ V | ____ V |
| 2.0 V, load on the output | ____ V | ____ V |
| Near 1 V, load on the output | ____ V | ____ V |
| Near 3 V, load on the output | ____ V | ____ V |

<!-- measure -->

The A0 number helps you return to a setting; the scope or meter compares
the actual voltages. The chip's output is not connected to a Mega signal
pin.

## Why it happens

At 2.0 V the wiper sits 40% of the way along the knob's 10 kΩ track:
6 kΩ above it and 4 kΩ below. Hung on the wiper, the 1 kΩ load joins the
lower 4 kΩ in parallel, and together they act like 800 Ω, so the wiper
falls to about 5 V × 800 ÷ 6800 ≈ **0.59 V**. This is the
[loaded-divider challenge](../../electricity/challenges.md#loaded-divider)
again.

The op-amp changes its output until the voltage at its − input is close to
the voltage at its + input. Here the output itself feeds the − input, so the
output settles near the knob's voltage. Its + input draws almost no current,
so the knob stays unloaded at 2.0 V, while the op-amp supplies the load's
2 mA from its own 5 V supply. The feedback holds the output near the reference
even while the load draws that current. Middle settings around 1–3 V leave
room between the output and both supply rails; do not expect exact tracking
at 0 V or 5 V.

## Check your result

Compare your four rows with your predictions. The input and loaded
output should rise together and be close at each middle setting; their
difference need not be zero. In one sentence, explain why the knob's
voltage fell when it fed the load itself, but not when the follower did.

## If it doesn't work

| What you see | Check with USB unplugged |
|---|---|
| Output stays near 0 V or 5 V | Check chip notch, pin 8 to 5 V, pin 4 to GND, and the direct pin 1 to pin 2 feedback wire. |
| Input does not change | Check both outer knob legs go to opposite rails and its wiper reaches pin 3. |
| Input changes but output differs greatly | Check the 1 kΩ load reaches output and GND, and that neither output nor the wiper is shorted to a supply rail. |
| A scope trace moves when a ground clip moves | Put both ground clips on the common bottom − rail and use DC coupling. |

This expected result comes from the circuit and
[MCP6002 datasheet](https://ww1.microchip.com/downloads/en/devicedoc/mcp6001-1r-1u-2-4-1-mhz-low-power-op-amp-ds20001733l.pdf);
no physical hardware trial has been recorded for this lesson.
