# Electricity, one experiment at a time

This is a second route through ADK. The main course has Lessons 1–55; these
24 electricity investigations have their own E01–E24 numbers, with each
title linking to its drawing and build steps. You can start here without
finishing the main course. Links back to it show where an idea is used, not
extra prerequisites. Each investigation asks for a prediction, one change,
and a visible or measured result. The results below are expectations from
the circuits, not records of hardware trials.

[Start E01: Close the loop](../lessons/056-close-the-loop/index.md){ .md-button .md-button--primary }

## Choose a route

**Kit and meter route:** Work through E01–E10, then E12, E22, E23 and E24.
E01–E03 use one steady LED circuit powered by the Mega's USB 5 V; they
need no IDE or upload. E02 adds the meter. E05 needs a 10 Ω resistor,
and E07–E08 and E23 the inexpensive capacitors, from the table below.
E12 builds the kit's buzzer into the protected-coil pattern; its LED and
a meter show the switching, and only a scope shows the diode at work.
After E04, try the [loaded-divider design challenge](challenges.md#loaded-divider)
to see why a measured divider voltage changes when a load is attached.

**Scope and generator extension:** After E10, try E11; after E08, try
E13–E15; then E16–E18. These investigations need the isolated instruments
described below; read the [scope and generator
primer](skills.md#scope-and-generator) and do its output check before the
first one. E11 helps explain E12, and E15 helps explain E23, but neither is
required to build the later kit-and-meter experiment.

**Logic branch:** After E05, try E19–E20 with the DIP logic chip listed
below; after E08, try E21. E24 teaches its own serial logic levels, so
this branch is optional for the kit-and-meter route.

Use the [measurement skills](skills.md) when recording a result, the
[schematic key](schematics.md) to trace a path, and the
[diagnosis guide](diagnose.md) when a reading surprises you. The
[design challenges](challenges.md) give you a way to change one circuit
and check your own prediction.

## Equipment gates

The [ADK starter kit](../kit.md) has the Mega, breadboard, wires, LEDs,
buttons, 220 Ω/1 kΩ/2 kΩ/10 kΩ resistors, potentiometer, S8050 and
1N4007. Add a digital multimeter with DC volts for the measured route.
[What to buy](../kit.md#what-to-buy) turns this table into a shopping list
for each path. Check each module's extra parts before starting it:

| Module | Investigations | Equipment beyond the kit and DC voltmeter |
|---|---|---|
| 1. DC paths | E01–E03 | None; E01 does not need the meter or an upload. |
| 2. Sharing | E04–E06 | E05: 10 Ω resistor. E04 and E06 use kit parts. |
| 3. Charge and diodes | E07–E09 | 1000 µF electrolytic capacitor rated at least 10 V for E07–E08; a stopwatch for E08. E09 uses kit parts. |
| 4. Switches and coils | E10–E12 | E10 and E12 use kit parts. E11 needs a 100 mH inductor rated at least 10 mA, generator and two-channel scope. |
| 5. Alternating signals | E13–E15 | 1 µF nonpolar film capacitor, generator and two-channel scope. |
| 6. Gain and power | E16–E18 | E16: MCP6002 DIP, 100 nF ceramic and 10 µF electrolytic capacitors rated at least 10 V, generator and scope; E17 reuses that chip circuit, which its page shows how to build without a generator, and may use a DC voltmeter instead of the scope; E18: 10 Ω resistor, 100 µF and 100 nF capacitors, generator and scope. |
| 7. Logic and memory | E19–E21 | E19–E20: 74HC00 DIP and 100 nF capacitor; E21: 74HC14 DIP, 2 × 100 kΩ resistors, 10 µF electrolytic and 100 nF ceramic capacitors, a stopwatch. |
| 8. Mega signals | E22–E24 | E22 and E24 use kit parts; E23 adds a 100 µF electrolytic capacitor rated at least 10 V. Its scope comparison is optional. |

For E11, E13–E16 and E18, use a **battery-powered two-channel oscilloscope**
sampling at 1 MS/s or more, with AC coupling or vertical offset, and a
**battery-powered, floating waveform generator** with adjustable 0–4 V
output, sine and square waves from 100 Hz to 1 kHz, and at least 5 mA
output. A handheld two-channel scope with a built-in generator fits, if
its generator can set both amplitude and offset. Join the generator's
signal ground to circuit ground. Keep its output off the Mega's USB +
rail and all Mega inputs. A scope alone is optional in E12, E17 and E23.
The [scope and generator primer](skills.md#scope-and-generator) explains
the settings, the output check to do first, and why the instruments must
run from their own batteries.

## How to work through one investigation

1. **Predict:** write down what the LED, meter or trace will do.
2. **Build or use:** unplug power, make the stated circuit or open the linked
   ADK build, check it, then power it.
3. **Observe:** record what actually happens, including a number when there
   is one.
4. **Explain:** point to the path and use the one idea for that investigation.
5. **Change one thing:** make the stated comparison with power unplugged.
6. **Check:** compare the two results with the prediction and fix the
   explanation if needed.

Use [Getting started](../start.md) when a lesson calls for an upload, and
read [Safety](../safety.md) before wiring. Each investigation's example
keeps a stable three-digit folder name in the Arduino IDE and the
repository: E04's is `examples/lessons/059-resistors-in-series`, and E24's
is `079-serial-link`. Physical circuits use only the Mega's
USB 5 V or the isolated 0–4 V generator. Never use mains, the kit's
9 V adapter, or a motor or servo driven from a pin. Unplug before changing
wires; put a resistor in series with **every** LED. Keep the
Mega's GND at the bottom − rail hole nearest it and its 5 V at the top +
rail hole nearest it, as in the [kit homes](../kit.md#breadboard-homes).
Do not join a generator output to the Mega's 5 V rail. Keep the meter in its
DC voltage mode, with the red lead in the voltage jack. Probe **across** a
part; find its current by dividing the resistor's voltage by its resistance.

## 1. DC paths and measurements

### E01 — [Close the loop](../lessons/056-close-the-loop/index.md) {#e01-close-the-loop}

**Idea:** Current needs a complete path. **Before:** Read
[Safety](../safety.md). **Use:** A steady USB 5 V → 220 Ω resistor → LED
→ GND circuit; no upload. **Predict, do, see:** Predict what happens
when GND is missing. Unplug, lift only the Mega's GND wire, then power
again. The LED stays dark; restore the wire and it lights. **ADK
connection:** [Lesson 1](../lessons/001-blink/index.md) uses the same LED
and resistor home, but switches it from a pin.

### E02 — [Measure across and through][e02-guide] {#e02-across-and-through}

[e02-guide]: ../lessons/057-measure-across-and-through/index.md

**Idea:** Voltage is between two points; current passes through a part.
**Before:** E01. **Use:** The same steady circuit and a DC voltmeter; no
upload. **Predict, do, see:** Predict whether the LED and resistor each
get all 5 V. Measure the supply and across each part; the two part
readings add to the supply voltage. Divide the resistor voltage by
220 Ω to find the current through both series parts. **ADK connection:**
The measurement explains the LED current in
[Blink](../lessons/001-blink/index.md).

### E03 — [Resist the flow](../lessons/058-resist-the-flow/index.md) {#e03-resist-the-flow}

**Idea:** More resistance gives less current at the same supply voltage.
**Before:** E02. **Use:** The same steady circuit, meter, and 220 Ω,
1 kΩ and 2 kΩ resistors; no upload. **Predict, do, see:** Predict which
resistor makes the LED dimmest; use each resistor in turn, unplugging
before every swap. The LED dims as resistance rises. Measure the voltage
across each resistor and divide by its resistance to estimate the series
current. **ADK connection:** Changes the resistor used with the LED in
[Blink](../lessons/001-blink/index.md).

## 2. Sharing current and voltage

### E04 — [Resistors in series](../lessons/059-resistors-in-series/index.md) {#e04-series-resistors}

**Idea:** Series parts carry the same current and share the voltage.
**Before:** E03. **Use:** E04's Mega USB 5 V build: two 1 kΩ resistors
and a meter. **Predict, do, see:** Predict the voltage across each equal resistor;
wire them end to end across 5 V and measure each and the pair. Each reads
about half the supply; the two readings add to the whole. Swap one for
2 kΩ and see the larger share move to it. **ADK connection:** Explains
the [knob divider in Lesson 7](../lessons/007-dimmer/index.md).

### E05 — [Branches in parallel][e05-guide] {#e05-parallel-branches}

[e05-guide]: ../lessons/060-branches-in-parallel/index.md

**Idea:** Parallel branches have the same voltage and their currents add.
**Before:** E04. **Use:** E05's Mega USB 5 V build: two red LEDs, two
1 kΩ resistors, a 10 Ω resistor that feeds both, meter. **Predict, do,
see:** Predict what adding a second LED-and-resistor branch will do to
the first LED and to the current through the shared 10 Ω resistor. Add it
beside the first, not in line with it. Both light about as brightly, and
the voltage across the 10 Ω resistor about doubles, from roughly 30 mV to
60 mV: about 3 mA, then 6 mA. Each 1 kΩ resistor's voltage divided by
1 kΩ gives its branch's current, and the two add up to the current
through the 10 Ω feed.
**ADK connection:**
Extends the separate LEDs in [Lesson 2](../lessons/002-buttons/index.md).

### E06 — [Tap a divider](../lessons/061-tap-a-divider/index.md) {#e06-tap-a-divider}

**Idea:** A potentiometer is an adjustable voltage divider. **Before:**
E04. **Use:** E06's knob-only Mega build and a DC voltmeter.
**Predict, do, see:** Predict the wiper voltage halfway around;
measure A0 to GND while turning the knob, and watch the Serial Plotter.
The voltage sweeps from about 0 to 5 V while the plotted A0 reading goes
from 0 to 1023. **ADK connection:** Lesson 7 already builds the divider;
E06 adds a meter reading at its documented A0 connection.

## 3. Stored charge and one-way paths

### E07 — [Charge a capacitor](../lessons/062-charge-a-capacitor/index.md) {#e07-charge-a-capacitor}

**Idea:** A capacitor stores separated charge. **Before:** E02. **Use:**
E07's Mega USB 5 V build, 10 kΩ, 1000 µF capacitor and voltmeter.
**Predict, do, see:** Put the capacitor's marked − leg at GND, charge it
through 10 kΩ and watch its voltage rise toward 5 V. Unplug USB and lift
the charging jumper's rail end into a free hole: with no path, the
capacitor holds its voltage. Move that end into the top − rail, which a
link wire joins to GND; the capacitor discharges through 10 kΩ and the
meter falls gradually, so the stored charge was still there after the
source left. Never short its legs.
**ADK connection:** The slow change helps explain
[Lesson 12's timer](../lessons/012-stopwatch/index.md), although that timer
runs in code.

### E08 — [Time an RC pair](../lessons/063-time-an-rc-pair/index.md) {#e08-time-an-rc-pair}

**Idea:** Resistance times capacitance sets a charging timescale.
**Before:** E07. **Use:** E07's USB 5 V build, another 10 kΩ, stopwatch and
meter. **Predict, do, see:** Time from 0 V to about 3.2 V with 10 kΩ.
Unplug USB and discharge as in E07, through the charging resistors into
the top − rail; verify near 0 V before charging through two 10 kΩ in
series.
Predict and observe roughly 10 s then 20 s; the exact times depend on
the parts and your timing. **ADK connection:** Gives a physical counterpart to
the waits in [Lesson 12](../lessons/012-stopwatch/index.md).

### E09 — [One-way diode](../lessons/064-one-way-diode/index.md) {#e09-one-way-diode}

**Idea:** A diode conducts mainly in one direction. **Before:** E03.
**Use:** E09's Mega USB 5 V build: 1N4007, red LED and 1 kΩ resistor.
**Predict, do, see:** Wire 5 V → diode → resistor → LED → GND, with the
diode's banded end away from the + rail. Predict what reversing only the
diode will do. The LED lights in the first orientation and stays dark in the
second; its resistor stays in place for both trials. **ADK connection:** The
same diode appears across the buzzer in [Lesson
3](../lessons/003-reaction-duel/index.md).

## 4. Electronic switches and magnetism

### E10 — [Control with a transistor][e10-guide] {#e10-transistor-switch}

[e10-guide]: ../lessons/065-control-with-a-transistor/index.md

**Idea:** A small base current controls a larger collector current.
**Before:** E09. **Use:** E10's Mega USB 5 V build: S8050, button, red
LED, 220 Ω LED resistor, 1 kΩ base resistor and 10 kΩ base pull-down,
a second 10 kΩ resistor and a meter. Check the marked S8050's E–B–C pin
order. **Predict, do, see:** Put the LED and its resistor between + and
collector, emitter at GND, and the button through 1 kΩ from + to base.
Predict the LED state before pressing; it is off until the button
supplies base current. With the button held, the resistor voltages give
about 14 mA through the LED and 4.2 mA into the base. Swap the 1 kΩ base
resistor for 10 kΩ: the LED stays as bright while the base current falls
to about 0.36 mA, a fortieth of the collector current. **ADK connection:**
Isolates the switch used for the [Lesson 3 buzzer](../lessons/003-reaction-duel/index.md).

### E11 — [An inductor resists change][e11-guide] {#e11-inductor-current}

[e11-guide]: ../lessons/066-inductor-current/index.md

**Idea:** An inductor makes current change gradually. **Before:** E10 and
the [scope primer](skills.md#scope-and-generator). **Use:** E11's generator
build: 0–4 V square wave, 1 kΩ resistor, 100 mH inductor, two-channel scope.
**Predict, do, see:** At 100 Hz, compare the generator edge with the voltage
across the 1 kΩ resistor, which stands for current. Wire generator output →
inductor → 1 kΩ → GND. Put both scope ground clips at GND, channel 1 on
generator output and channel 2 at the inductor/resistor junction. With the
inductor, channel 2 rises most of the way in roughly 0.1 ms and levels at
about 2.6–3.8 V, below 4 V because the coil's winding and the generator
take a share; replace the inductor with a wire and the edge is much
sharper. **ADK connection:** The coil in [Lesson
3's buzzer](../lessons/003-reaction-duel/index.md) is why its switch has a
protective diode.

### E12 — [Give a coil a safe path](../lessons/067-coil-diode/index.md) {#e12-coil-diode}

**Idea:** A flyback diode gives coil current a path when its switch opens.
**Before:** E10. E11 gives an optional scope comparison; with a scope, read
the [scope primer](skills.md#scope-and-generator). **Use:** E12's Mega USB
5 V build: kit passive buzzer, S8050, button, 220 Ω buzzer resistor, 1 kΩ base
resistor, 10 kΩ base pull-down, 1N4007, and red LED with its own 1 kΩ
resistor; a scope is optional. **Predict, do, see:** Wire 5 V → 220 Ω →
buzzer + → buzzer − → collector; emitter goes to GND. Put the button through
1 kΩ from 5 V to base and 10 kΩ from base to GND. Fit the diode directly
across the buzzer from the start: its banded end at buzzer + and unbanded
end at collector. The separate resistor–LED branch also feeds the collector.
Predict a lit LED while the button is held and possibly a faint click as you
press and release. A meter on the collector reads about 0.1 V while the
button is held and about 5 V when it is released: without a scope, you
build the protection and check the switching, but cannot see the diode
work. With a scope, probe the collector against GND to see it rise about
0.7 V above 5 V for a moment on release, as the coil's current carries on
through the diode. The
220 Ω stays in series with the roughly 16 Ω coil; the 5 V rail, not a Mega
I/O pin, supplies its current. **ADK connection:** Combines [Lesson 5's
passive buzzer](../lessons/005-melody-maker/index.md) with the transistor
and flyback diode pattern in [Lesson
3](../lessons/003-reaction-duel/index.md).

## 5. Alternating signals

### E13 — [Current reverses](../lessons/068-alternating-current/index.md) {#e13-alternating-current}

**Idea:** Alternating current flows first one way, then the other.
**Before:** E08 and the [scope primer](skills.md#scope-and-generator).
**Use:** E13's generator build: 0–4 V, 1 kHz sine wave (2 V offset), 1 µF
nonpolar film capacitor, 1 kΩ resistor, scope. **Predict, do, see:** Connect
generator → capacitor → resistor → ground. With both scope ground clips at
circuit ground, DC-couple channel 1 at the generator output and channel 2 at
the resistor's top. Predict whether channel 2 can go below ground. Channel 1
stays between 0 and 4 V while channel 2 swings about 2 V above and below
0 V: the capacitor charges to the generator's 2 V average, so current flows
toward GND while the input is above 2 V and reverses while it is below.
Replace the capacitor with a wire and channel 2 matches channel 1, never
below 0 V. **ADK connection:** Deepens the waveform idea behind the [Lesson
5 buzzer](../lessons/005-melody-maker/index.md).

### E14 — [Count a waveform][e14-guide] {#e14-frequency-and-period}

[e14-guide]: ../lessons/069-frequency-and-period/index.md

**Idea:** Frequency counts cycles per second; period is time per cycle.
**Before:** E13. **Use:** E13's circuit, generator and scope; the [scope
primer](skills.md#scope-and-generator) explains the timebase. **Predict, do,
see:** Change only the generator from 100 Hz to 1 kHz. Predict how many
cycles fit in 10 ms; the scope shows about one, then ten. Measure one period
as about 10 ms, then 1 ms. **ADK connection:** Explains why changing the
note in [Lesson 5](../lessons/005-melody-maker/index.md) changes pitch.

### E15 — [Filter and phase](../lessons/070-filter-and-phase/index.md) {#e15-filter-and-phase}

**Idea:** An RC low-pass filter reduces fast changes and delays the output.
**Before:** E08, E14 and the [scope primer](skills.md#scope-and-generator).
**Use:** E15's generator build: 0–4 V sine wave, 1 kΩ series resistor, 1 µF
film capacitor from output to ground, both scope channels. **Predict, do,
see:** Compare input and output at 100 Hz, then 1 kHz. The output is smaller
at 1 kHz, and its peaks come later than the input peaks; the capacitor's
charge cannot follow as quickly. **ADK connection:** Gives a circuit
explanation for smoothing the [Lesson 7
dimmer](../lessons/007-dimmer/index.md).

## 6. Gain, feedback and clean power

### E16 — [Make a signal larger](../lessons/071-amplifier-gain/index.md) {#e16-amplifier-gain}

**Idea:** An amplifier changes a signal by a chosen gain. **Before:** E15
and the [scope primer](skills.md#scope-and-generator). **Use:** E16's USB
5 V MCP6002 build, five 10 kΩ resistors and a sixth for the comparison, 100 nF
and 10 µF supply capacitors, generator and scope. Place both capacitors
across the supply near the chip, with the 10 µF + leg at 5 V. Wire the
unused amplifier as a follower: its + input uses a 2.5 V divider and its
output joins its − input. **Predict, do, see:** Wire amplifier A as a
non-inverting amplifier with one 10 kΩ from output to − input and one from
− input to GND; feed + input a 0.5–1.5 V sine wave at 100 Hz through a 10 kΩ
input resistor. Predict the output range; it follows at about 1–3 V, twice
the input. Put a second 10 kΩ in series in the feedback path and the gain
becomes 3: about 1.5–4.5 V. **ADK connection:** A sensor with a small output
needs a stage like this; [Lesson 8's](../lessons/008-light-meter/index.md)
photoresistor divider already swings widely enough to read without one.

### E17 — [Feed back the output](../lessons/072-negative-feedback/index.md) {#e17-negative-feedback}

**Idea:** Negative feedback makes an output follow a reference. **Before:**
E16; with a scope, the [scope primer](skills.md#scope-and-generator).
**Use:** E17's USB-powered MCP6002 with both nearby supply capacitors and
its unused amplifier terminated as in E16; add the kit's potentiometer,
scope or meter, and a 1 kΩ load resistor. **Predict, do, see:** Set the
knob's wiper to 2.0 V, then hang the 1 kΩ load straight on it: the wiper
falls to about 0.6 V. Connect the op-amp output to its − input (a follower),
the knob's wiper to + input and the load from output to GND; the output
stays near 2.0 V. Turn the knob between about 1 V and 3 V; input and output
readings track even while the output feeds the load. **ADK connection:** The
knob comes from [Lesson 7](../lessons/007-dimmer/index.md); this is a new
buffer circuit, not a change to that lesson's wiring.

### E18 — [Keep the supply steady](../lessons/073-power-integrity/index.md) {#e18-power-integrity}

**Idea:** Supply resistance makes load changes disturb the local voltage;
a nearby capacitor reduces short ripples. **Before:** E07, E10, E14 and
the [scope primer](skills.md#scope-and-generator).
**Use:** E18's USB 5 V build: 10 Ω in the + feed, E10's transistor
and LED load pattern, generator at 1 kHz square wave, 1 kΩ base resistor,
10 kΩ base pull-down, 100 µF and 100 nF capacitors, scope. **Predict, do,
see:** Copy E10's LED and transistor path onto locally fed rails, omitting
the button. Drive the base from generator output through 1 kΩ; keep the
10 kΩ pull-down and join generator ground to Mega GND. Probe the local +
rail with the scope ground at GND, AC-coupled at 50 mV/div (or with a
vertical offset). The rail steps down by about 0.13 V for as long as the
LED is lit. Add 100 µF and 100 nF across the local rails, with 100 µF +
at local + and − at GND; the step shrinks to a ripple of about 30 mV.
Change only the generator to 100 Hz and the ripple grows to about
130 mV: the capacitors smooth short changes, not long ones.
**ADK connection:** Helps explain why
[Lesson 15's sensors and display](../lessons/015-weather-station/index.md)
share power and need a sound ground path.

## 7. Logic, memory and timing

### E19 — [Make a truth table](../lessons/074-nand-logic/index.md) {#e19-nand-logic}

**Idea:** A logic gate maps input levels to one output level. **Before:**
E05. **Use:** E19's USB 5 V build: 74HC00, two buttons with 10 kΩ
pull-downs, red LED with 1 kΩ resistor, 100 nF supply capacitor. Tie unused
gate inputs to GND. **Predict, do, see:** Try no buttons, each button alone,
then both. The NAND output LED is on except when both inputs are high;
write the four-row truth table. **ADK connection:** Turns the button
decisions in [Lesson 2](../lessons/002-buttons/index.md) into hardware.

### E20 — [Remember one bit](../lessons/075-set-reset-latch/index.md) {#e20-set-reset-latch}

**Idea:** Feedback can hold a digital state after an input ends. **Before:**
E19. **Use:** E20's USB 5 V build: E19's 74HC00, two buttons with 10 kΩ
pull-ups, LED with 1 kΩ resistor and 100 nF supply capacitor. Cross-couple
two NAND gates into an active-low
set/reset latch; each button pulls only its named input to GND. Press only
one button at a time. **Predict, do, see:** Power-up state is unknown, so
press Reset once first. Then press Set and release; the LED stays on. Press
Reset and release; it stays off. **ADK connection:** A hardware version of
the state in
[Lesson 6's Simon game](../lessons/006-simon/index.md).

### E21 — [Make a clock tick](../lessons/076-schmitt-clock/index.md) {#e21-schmitt-clock}

**Idea:** A charging capacitor and a threshold can make a repeating clock.
**Before:** E08, E20. **Use:** E21's USB 5 V build: 74HC14,
100 kΩ feedback resistor, 10 µF capacitor with − at GND, red LED with
1 kΩ resistor, 100 nF supply capacitor, stopwatch and meter. Tie unused
inputs to GND, and take out E20's wires before the new chip goes in: its
pins do different jobs. **Predict, do, see:** Feed one inverter's output
back to its input through 100 kΩ and put the capacitor from input to GND.
With R × C = 1 s, predict the pace; the LED on the output blinks about
once every 0.8–1.1 s. Change to two 100 kΩ resistors in series and it
blinks about half as fast, slowly enough for a meter on the capacitor to
follow it between about 1.7 V and 2.7 V, the chip's two thresholds.
**ADK connection:** A physical cousin of the timed events in
[Lesson 12](../lessons/012-stopwatch/index.md).

## 8. Measuring and sending with the Mega

### E22 — [Sample a voltage](../lessons/077-sampling/index.md) {#e22-sampling}

**Idea:** Sampling turns a changing voltage into separate numbered readings.
**Before:** E06. **Use:** E22's version of
[Lesson 7's Dimmer build](../lessons/007-dimmer/index.md),
meter, Serial Plotter and Serial Monitor. **Predict, do, see:** Turn the
knob slowly and compare the wiper voltage with the plotted 0–1023
reading; near 2.5 V it is near 512. In the Serial Monitor the printed
readings move in whole steps while the voltage changes smoothly; on the
Plotter's 0–1023 scale a step of one is too small to see. At a boundary,
adjacent numbers may flicker because of electrical noise. **ADK
connection:** Reuses Lesson 7's sketch and wiring exactly.

### E23 — [Average PWM](../lessons/078-pwm-average/index.md) {#e23-pwm-average}

**Idea:** PWM changes its on-time, and a filter can turn that into an
average voltage. **Before:** E08, E22. E15 gives an optional scope
comparison. **Use:** E23's Mega build based on
[Lesson 7](../lessons/007-dimmer/index.md): pin 3's LED keeps its 220 Ω
resistor; add a separate 10 kΩ from pin 3 to a meter point and 100 µF from
that point to GND (− leg to GND). **Predict, do, see:** Turn the knob from
low to high; the LED brightens while the filtered meter point rises from
about 0 to 5 V after settling for several seconds. On the scope (see the
[scope primer](skills.md#scope-and-generator)), pin 3 still switches
between 0 and 5 V. The filter branch is separate from the
LED branch.

### E24 — [Send a byte down a wire](../lessons/079-serial-link/index.md) {#e24-serial-link}

**Idea:** A serial port can receive the bytes it sends when its TX and RX
pins have a complete path.
**Before:** E01. Follow E23 for the guided route; E19's gate experiment
is optional. **Use:** E24's Mega build: one 1 kΩ resistor between
Serial1 TX1 (pin 18) and RX1 (pin 19), a 10 kΩ pull-up from RX1 to 5 V,
and an E24 sketch that sends a known byte and prints what Serial1 receives
to the USB Serial Monitor.
**Predict, do, see:** The same Serial1 UART sends and receives the byte;
predict a matching echo. Unplug, remove the link and power again, and no
byte returns. This loopback checks the path, not timing between two
devices; for that, set the Serial Monitor to 4800 or 19200 baud while the
Mega still sends at 9600, and the computer's receiver, timing the bits
wrongly, shows a jumble. The resistor limits current if a pin is
accidentally configured as an output. **ADK connection:** The wire and shared timing are the small-scale
version of the messages in [Lesson 38](../lessons/038-radio-messages/index.md).

## What could come next

These are **planned topics, not numbered lessons or tested builds**. The
current guided course ends at E24.

| Possible branch | What a later investigation could measure |
|---|---|
| Low-voltage AC | Impedance, RMS voltage and resonance with suitable isolated instruments. |
| Power conversion | Rectification and regulation at low voltage, with measured ripple and load changes. |
| Digital systems | Gate timing, stored state and communication between two devices. |
