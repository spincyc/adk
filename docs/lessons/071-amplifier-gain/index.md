---
lesson: 71
promise: Make a small sine wave about twice as tall with two feedback resistors.
time: 30 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - MCP6002 in an 8-pin PDIP package
  - 4 × 10 kΩ resistors (brown, black, black, red, brown)
  - 100 nF ceramic capacitor
  - 10 µF electrolytic capacitor rated at least 10 V
  - Isolated, battery-powered 0–4 V waveform generator
  - Battery-powered two-channel oscilloscope with two probes
  - 14 jumper wires
  - 2 female-to-male wires
ideas:
  - Feedback resistors set the gain of a non-inverting amplifier
---

## What you'll build

<!-- closeup -->

An MCP6002 amplifier takes a small sine wave from an isolated generator.
Two equal **10 kΩ feedback resistors** set its gain to about two. On the
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

Take out Lesson 70's filter parts, their jumpers, and the generator's OUT
wire. Keep the generator, its GND wire, and the Mega's GND wire in the
bottom − rail hole nearest the Mega. The generated steps add the Mega's
5 V wire to the top + rail hole nearest it. With the MCP6002's notch
pointing left,
pin 1 is its output A, pin 2 its − input A, pin 3 its + input A, pin 4
GND, pin 5 the second amplifier's + input, pin 6 its − input, pin 7 its
output, and pin 8 its 5 V supply. Check the chip marking before powering.

Follow the generated steps. Put the **100 nF capacitor in the nearest free
strips of pins 8 and 4**, and the **10 µF capacitor across the nearby +
and − rails**, with its + leg on 5 V and striped − leg on GND. The second
amplifier is held steady: two other 10 kΩ resistors make a midpoint near
2.5 V for its + input, while its output joins its − input.

<!-- bench -->

<!-- steps -->

The generator's OUT reaches **+ input A (pin 3)**. One feedback resistor
joins **output A (pin 1)** to **− input A (pin 2)**; the other joins pin 2
to GND. Check these three nodes against the generated connection list:

<!-- connections -->

## Compare input and output

1. With both power sources still off, clip **both scope ground leads** to
   free holes in the bottom − rail. Put channel 1's tip in a free hole in
   pin 3's strip, alongside generator OUT. Put channel 2's tip in a free
   hole in pin 1's strip. Keep the metal tips apart. Set both channels to
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
one way an analog sensor can make a small voltage easier to measure, as
in [Lesson 8's light meter](../008-light-meter/index.md).

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

## If the traces surprise you

| What you see | Check with USB unplugged and generator off |
|---|---|
| Output stays near 0 V or 5 V | Check the MCP6002 notch, pin 8 to 5 V, pin 4 to GND, and both 10 kΩ feedback paths at pin 2. |
| Input appears but output is about the same size | Check that the output-to-pin-2 resistor and pin-2-to-GND resistor are separate paths, not a wire around a resistor. |
| Both traces look flat | Check generator OUT and GND, its 100 Hz sine setting, the scope's DC coupling, and the timebase. |
| Trace changes when a scope clip moves | Put both ground clips on the common bottom − rail and check the generator GND wire reaches it. |

## About the sketch

This is a passive analog experiment. The matching ADK sketch claims no
Mega signal pins and needs no upload:

<!-- sketch -->

The stated voltages follow from the circuit design and datasheet; no
hardware trial has been recorded for this lesson. The [MCP6002 datasheet](https://ww1.microchip.com/downloads/en/devicedoc/mcp6001-1r-1u-2-4-1-mhz-low-power-op-amp-ds20001733l.pdf)
gives its PDIP pinout and recommends a local 0.01–0.1 µF bypass capacitor
plus at least 1 µF of bulk capacitance.
