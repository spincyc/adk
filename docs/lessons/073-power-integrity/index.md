---
lesson: 73
promise: See how nearby capacitors reduce a brief dip in a local supply.
time: 25 minutes
level: 3
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Battery-powered, isolated waveform generator with 0–4 V square output
  - Battery-powered oscilloscope and probe
  - Red LED
  - S8050 transistor (check its E–B–C pin order)
  - 10 Ω resistor (brown, black, black, gold, brown)
  - 220 Ω resistor (red, red, black, black, brown)
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 10 kΩ resistor (brown, black, black, red, brown)
  - 100 µF polarized capacitor rated at least 10 V
  - 100 nF ceramic capacitor
  - 7 jumper wires and 2 female-to-male wires
ideas:
  - A changing load briefly lowers its local supply voltage; nearby capacitors reduce the dip
---

## What you'll build

<!-- closeup -->

The Mega's USB 5 V feeds a **local supply** through a 10 Ω resistor. An
isolated generator switches a red LED load on and off at 1 kHz. A scope
shows the local voltage with and without two capacitors beside that load.
The Mega supplies power; its signal pins and sketch do not make the wave.

## Predict

The LED takes current only while the transistor is on. Predict what
happens to the voltage **after the 10 Ω resistor** when the LED lights.
Then predict how two capacitors from that local supply to GND might change
the dip. Write down both predictions before measuring.

## Build it

!!! warning "Unplug before changing the circuit"
    Unplug USB and switch off the generator before moving parts, wires, or
    probe clips. Use a battery-powered, isolated generator set to **0–4 V**.
    Its OUT goes only through the 1 kΩ base resistor, never to a Mega pin
    or either + rail. Keep the **220 Ω resistor in series with the LED**.
    The 100 µF capacitor's striped − leg belongs at GND; check its rating
    and stripe before powering. Put the scope ground clip only on the
    common bottom − rail, never on the local supply or a floating point.

Take out the parts and signal wires from the previous build. Keep the
Mega's GND wire in the bottom − rail hole nearest it and its 5 V wire in
the top + rail hole nearest it. Put the red LED in its familiar holes and
the S8050 in its [E10](../065-control-with-a-transistor/index.md)
E–B–C holes. Check the marking and pin order of your transistor before
inserting it; similar packages differ.

<!-- bench -->

<!-- steps -->

The **10 Ω resistor in column 4** crosses the breadboard's middle gap.
Its lower end makes column 4's lower strip the local supply. The 220 Ω
LED path and both capacitor + sides meet this local supply. Each
capacitor's other side meets the bottom − rail, **after** the 10 Ω
resistor. Neither capacitor connects across the 10 Ω resistor.

<!-- connections -->

## Compare the ripples

1. With USB unplugged and the generator off, put the scope ground clip
   in a free bottom − rail hole. Put the probe tip in a free hole of
   **column 4's lower strip**, beside the capacitor's + leg. Set the
   scope near **50 mV per division** and about **1 ms across the screen**.
   Use AC coupling to enlarge the small change, or use DC coupling and
   shift the roughly 5 V trace into view with the vertical offset.
2. For the **without capacitors** trace, keep all power unplugged and lift
   **both capacitors** out of their holes. Leave the 10 Ω resistor, LED,
   transistor, base resistors, generator wires, and other jumpers in place.
   Plug in USB, check the generator is set to a **0–4 V square wave at
   1 kHz**, then enable its output. Record the local trace's high-to-low
   change while the LED switches. The dip may be around a tenth of a volt;
   USB voltage and parts vary.
3. Switch the generator off and **unplug USB**. Put both capacitors back
   exactly as drawn: 100 µF **+ to the local supply**, striped **− to the
   bottom − rail**; 100 nF across those same two nets. Check the probe
   ground again. Plug in USB, enable the same generator wave, and record
   the trace with the same scope settings.
4. Compare the two recorded changes with your prediction. The ripple
   should be smaller with the capacitors. Turn the generator off and
   unplug USB when finished.

At 1 kHz the LED may look steadily lit; the scope shows what changes
each cycle.

When the LED lights, its current flows through the 10 Ω resistor, which
uses a little of the USB voltage. Around 10 mA through 10 Ω gives a drop
near **0.1 V**. The capacitors can briefly supply some of the changing
current from the local side of that resistor. The 100 µF part holds more
charge; the 100 nF ceramic part sits in parallel to help with fast edges.
The exact trace depends on the USB supply, leads, and scope settings.

## If the trace surprises you

- **No LED switching:** With power off, check the generator's OUT wire,
  its common GND wire, the 1 kΩ base resistor, the 10 kΩ pull-down, and
  the S8050's marked E–B–C order.
- **LED stays dark:** Check that the 10 Ω feed crosses the gap in column
  4 and that the 220 Ω resistor stays between the local supply and the
  LED's long leg. Check the short leg reaches the collector and the
  emitter reaches GND.
- **No visible ripple:** Check that the probe tip is on the *local* side
  of the 10 Ω resistor, that the scope ground clip is at GND, and that
  AC coupling or vertical offset makes a small change visible.
- **A part gets hot or smells:** Unplug immediately. Check for a wire
  directly between + and − and for a reversed 100 µF capacitor.

## About the sketch

No upload is needed: USB powers this circuit even if the Mega has an
older sketch. The matching ADK sketch claims no I/O pins:

<!-- sketch -->

This shows why parts sharing a supply, such as the sensors and display in
[Lesson 15](../015-weather-station/index.md), need a sound power path.
The expected ripple is a circuit prediction; this build has not been
checked on physical hardware.
