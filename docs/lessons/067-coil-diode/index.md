---
lesson: 67
promise: Light an LED with a coil, and give the coil current a safe path when switched off.
time: 25 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Push button
  - Red LED
  - Passive buzzer (the one with a green board showing underneath)
  - S8050 transistor (check its E–B–C pin order)
  - 1N4007 diode
  - 220 Ω resistor (red, red, black, black, brown)
  - 2 × 1 kΩ resistors (brown, black, black, brown, brown)
  - 10 kΩ resistor (brown, black, black, red, brown)
  - 10 jumper wires
ideas:
  - A flyback diode gives coil current a path when its switch opens
---

## What you'll build

<!-- closeup -->

A button switches a red LED and the kit's passive buzzer through a
transistor. The LED lights clearly while you hold the button. A diode stays
across the buzzer's coil from the start; you may hear a faint click when
you press or release. A battery-powered oscilloscope can show the brief
voltage change at the transistor when you release it. The Mega supplies
5 V from USB; no upload is needed.

## Predict

Current flows through a coil while you hold the button. When you let go,
the transistor stops feeding it, but the coil's current cannot stop at
once. Before powering the build, predict whether the LED will light while
you press and where the coil's current will go on release. Will the buzzer
play a steady note while the button is held, or might it only click as
the current changes?

## Build it

!!! warning "Unplug first"
    Unplug USB before changing any wiring. Keep the **220 Ω resistor in
    series with the passive buzzer**: its coil is only about 16 Ω. Keep
    the LED's own **1 kΩ resistor in series** too. Fit the diode before
    powering the circuit and never remove it while powered. Never run
    this buzzer directly from a Mega pin. If a part gets hot or smells,
    unplug at once and check the wiring.

Start with the breadboard clear and USB unplugged. Remove any previous
parts and wires before following the complete steps below. They include
the Mega's GND wire into the bottom − rail hole nearest it and its 5 V
wire into the top + rail hole nearest it.

This experiment uses the transistor switching idea from
[E10](../065-control-with-a-transistor/index.md).
The coil experiment in [E11](../066-inductor-current/index.md) is an
optional scope comparison; the button, LED and protective diode here make
a complete experiment on their own.

Use the **passive** buzzer, with the green board visible underneath and a
**+** mark beside one leg. The S8050 drawing assumes **E–B–C**, left to
right with the marked flat face toward you and its legs pointing down.
Check the marking and pin order for your transistor before inserting it;
kit versions can differ.

<!-- bench -->

<!-- steps -->

Trace the coil path: **+ rail → 220 Ω → buzzer + → buzzer − → collector →
emitter → − rail**. A second branch runs **+ rail → 1 kΩ → red LED →
collector**. The button feeds the base through the other 1 kΩ. A 10 kΩ
resistor holds the base at GND when the button is released. The 1N4007
goes **directly across the buzzer**, after the 220 Ω resistor: its banded
end reaches buzzer + and its unbanded end reaches buzzer − and the
collector. Check these three connections before plugging in USB.

<!-- connections -->

## Try it

1. With USB unplugged, point to the diode's band at c36 and its unbanded
   end at c33. Check that c36 shares the buzzer's **+** node, and c33
   shares its **−** node and the collector.
2. Plug the Mega into USB. The red LED should be off. Press and hold the
   button: it should light. Release it: it should go out. Record any sound
   at each change. You may hear a faint click on pressing or releasing;
   this passive buzzer has no circuit to make a sustained note from
   steady DC.
3. Compare what you noticed with your prediction. When the transistor
   opens, the LED goes dark, but the coil briefly keeps current moving
   through the diode and back through the coil. The diode gives that
   current a loop while it dies away.

The diode normally blocks current from buzzer + to buzzer − while the
button is held. At release, the coil drives current the other way around
the short diode-and-coil loop. Keep the diode connected throughout the
experiment.

### If you have an oscilloscope

Use a battery-powered scope with its **ground clip only on the GND rail**.
Put the probe tip on the collector node at a free hole in column 31's
lower strip. Set DC coupling, trigger on the rising edge, and begin around
100 µs/div; adjust the time and voltage scales until you can see the brief
release event. The collector should rise from near GND when the button is
held, show a brief diode-clamped transient when you let go, then settle
near the 5 V supply. The exact trace depends on the coil, resistor and
scope. Never attach the ground clip to the collector or buzzer legs.

## If it doesn't work

Unplug USB before each wiring check:

| What you see | Check |
|---|---|
| LED never lights | Check its long leg in b6, short leg in b7, its 1 kΩ resistor across g6–e6, and the jumper from a7 to b33. |
| LED stays lit without a press | Check that the button straddles the middle gap and the 10 kΩ resistor joins the base in column 30 to the − rail. |
| LED works, but no click is audible | A quiet click may be missed; use the scope if available. Check the passive buzzer's + mark in f33, the 220 Ω resistor across g36–e36, and the jumper from a36 to j33. |
| Collector trace does not change | Check the S8050's marking and E–B–C order, its emitter's − rail wire, and the button's 1 kΩ path into the base. |
| A part gets hot or smells | Unplug immediately. Check that + reaches the buzzer only through 220 Ω, the LED only through 1 kΩ, and that the diode's band is on the buzzer + side. |

## About the sketch

The physical button and transistor do the switching. USB powers the
rails even if the Mega still has an older sketch. This matching ADK sketch
claims no signal pins:

<!-- sketch -->

This is an expected circuit behavior, not a record of a hardware trial.
