---
lesson: 66
promise: Watch a coil make current rise gradually instead of all at once.
time: 25 minutes
level: 3
parts:
  - Arduino Mega 2560
  - Breadboard
  - Isolated, battery-powered 0–4 V waveform generator with at least 5 mA output
  - Battery-powered two-channel oscilloscope sampling at 1 MS/s or more, with two probes
  - 100 mH inductor rated for at least 10 mA
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 3 jumper wires (and one spare for the comparison)
ideas:
  - An inductor slows changes in current
---

## What you'll build

<!-- closeup -->

An isolated generator sends a 0–4 V square wave through a **100 mH coil**
and a **1 kΩ resistor**. A two-channel scope compares the generator voltage
with the voltage across the resistor. The Mega provides only the familiar
GND rail wire; its USB cable may stay unplugged.

## Predict

At each rising edge, the generator voltage jumps up. The resistor voltage
shows the current through the path: more current means more voltage across
the resistor. Will that voltage jump up at the same instant, or climb over
a short time? Predict what will change if you replace the coil with a wire.

## Build it

!!! warning "Power off before wiring"
    Switch off and unplug the generator before placing or moving parts.
    Use only an **isolated, battery-powered generator** set to 0–4 V.
    Keep its output off the Mega's pins and the + rails. Both scope ground
    clips must go to the bottom − rail, never to a floating signal point.

Take out E10's button, LED, transistor, and resistors, and remove
the Mega's 5 V rail wire. Keep its GND wire in the bottom − rail hole
nearest the Mega. You may leave the Mega's USB unplugged. Put the coil
across the breadboard's center gap at **g6–e6**, then the resistor across
the gap at **g10–e10**. Follow the generated drawing and steps for the
jumper wires and the generator's own clip leads: blue OUT into j6, black
GND into the bottom − rail by column 5.

<!-- bench -->

<!-- steps -->

Current has one route: **generator OUT → coil → 1 kΩ resistor → common
GND → generator GND**. The coil has no direction. The 1 kΩ resistor limits
steady current to at most about 4 mA with a 4 V input, below the coil's
10 mA rating.

<!-- connections -->

## Compare the edges

If this is your first time with the scope and generator, read the
[scope and generator primer](../../electricity/skills.md#scope-and-generator)
and do its output check before you connect the generator to this circuit.

The two probes go as drawn here: each tip in a free hole, each ground clip
in a free hole of the bottom − rail.

<!-- probe -->

1. With the generator still off, clip **both scope ground leads** to the
   bottom − rail, by columns 6 and 9. Put channel 1's tip in **i6**, in the
   strip of the generator's OUT lead, and channel 2's tip in **i10**, at
   the **top of the resistor**. Keep the probe tips apart. Use DC coupling
   for both channels.
2. Set the generator to a **0–4 V square wave at 100 Hz**. Set the scope
   to trigger on channel 1's rising edge and start near **0.05 ms/div**
   (50 µs/div). Turn on the generator. Sketch both rising edges and record
   how long channel 2 takes to rise most of the way.
3. **Switch off and unplug the generator.** Remove only the coil. Bridge
   its two breadboard strips with the spare jumper from **g6 to e6**, the
   holes the coil's leads used.
   Leave the resistor and both probes in place. Check the route, reconnect
   the generator, and compare the same rising edges at the same scope
   settings.

With the coil, channel 2 should rise more gradually than channel 1:
about two thirds of the way up within roughly **0.1 ms**, and level by
about **0.3 ms**. It levels off below 4 V, at roughly **2.6–3.8 V**,
because the coil's own winding resistance and the generator's output
resistance, often 50 Ω, take part of the 4 V. With the jumper, channel 2
should follow channel 1 much more sharply, levelling near 3.8 V. The rise
timescale is **L ÷ R**: 0.1 H ÷ (1000 Ω + 50 Ω + the coil's winding
resistance, a few hundred ohms at most) is about **65–95 µs**. Record
what you observe before deciding whether it matches your prediction.

## Why it happens

Changing current in a coil changes its magnetic field. The coil pushes
back against that change, so current through the series resistor builds
over a short time. Since the resistor's voltage is current × 1 kΩ, its
trace lets you watch that build-up. A plain wire has almost no inductance,
so the resistor voltage rises much faster. This coil behavior is why
[Lesson 3's buzzer](../003-reaction-duel/index.md) has a protective diode
beside its switch.

## Check your result

Which channel rose first with the coil in place? Did channel 2 become
sharper after you bridged the coil? Those two observations show whether
the coil slowed the current change in your build.

## If the trace surprises you

| What you see | Check with the generator off and unplugged |
|---|---|
| Channel 2 stays flat | Check OUT to j6, the coil at g6–e6, the a6-to-j10 jumper, the resistor at g10–e10, and a10 to the bottom − rail. |
| Both traces look flat | Check that the generator is set to a 100 Hz square wave and that the scope triggers on channel 1. Check the timebase and probe coupling. |
| Channel 2 looks like channel 1 even with the coil | Look for a wire bypassing g6–e6 or channel 2's tip on i6 instead of i10. |
| Traces move when a clip moves | Put both ground clips only on the bottom − rail and check the generator's GND wire reaches that rail. |

## About the sketch

No upload or Mega signal pin is needed. The matching ADK sketch claims no
I/O pins:

<!-- sketch -->

The stated traces are expected from the circuit design. This lesson has
not been recorded as tried on hardware.
