# Electricity, one experiment at a time

This is the syllabus for a parallel, self-guided electricity course. The ADK
lessons show what a circuit can *do*; these 24 short investigations ask why it
does it. Start at E01 and work in order. Each investigation has one main idea,
one change to make, and a result you can see on an LED, meter, plot or scope.
Each linked lesson has a wiring diagram and a matching sketch, even when
the circuit works without an upload. This syllabus describes expected
results; it does not record a hardware trial.

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

Use [Getting started](../start.md) for the Mega and IDE and read
[Safety](../safety.md) before wiring. Physical circuits use only the Mega's
USB 5 V or the isolated 0–4 V generator below. Never use mains, the kit's
9 V adapter, or a motor or servo driven from a pin. Unplug before changing
wires; put a resistor in series with **every** LED. Keep the
Mega's GND at the bottom − rail hole nearest it and its 5 V at the top +
rail hole nearest it, as in the [kit homes](../kit.md#breadboard-homes).
Do not join a generator output to the Mega's 5 V rail. Keep the meter in its
DC voltage mode, with the red lead in the voltage jack. Probe **across** a
part; find its current by dividing the resistor's voltage by its resistance.

**Equipment:** The [ADK starter kit](../kit.md) supplies the Mega,
breadboard, jumper wires, LEDs, buttons, 220 Ω/1 kΩ/2 kΩ/10 kΩ resistors,
potentiometer, S8050 and 1N4007. Add a digital multimeter with DC volts;
10 Ω and 100 kΩ resistors;
10 µF, 100 µF and 1000 µF electrolytic capacitors rated at least 10 V,
100 nF ceramic capacitors, a 1 µF nonpolar film capacitor, and a 100 mH
inductor rated for at least 10 mA. E16–E17 need an MCP6002 in a DIP
package; E19–E20 need a 74HC00 and E21 needs a 74HC14, both in DIP
packages. E11, E13–E18 and E23 use a battery-powered two-channel oscilloscope
sampling at 1 MS/s or more, with AC coupling or vertical offset. E12 can
use the same scope to show its brief switching event. E11 and
E13–E18 also need a battery-powered, floating waveform generator with
adjustable 0–4 V output, sine and square waves from 100 Hz to 1 kHz,
and at least 5 mA output. Join its signal ground to circuit ground when
driving or measuring a circuit. Keep its output off the USB + rail
and all Mega inputs.

## 1. DC paths and measurements

### E01 — [Close the loop](../lessons/056-close-the-loop/index.md) {#e01-close-the-loop}

**Idea:** Current needs a complete path. **Before:** [Getting started](../start.md).
**Use:** Lesson 56's Blink build with its 220 Ω LED resistor. **Predict,
do, see:** Predict what happens when GND is
missing; unplug, lift only the black GND wire, then power again. The LED
stays dark; restore the wire and it blinks. **ADK connection:** Lesson 1
already makes and explains this exact comparison.

### E02 — [Measure across and through][e02-guide] {#e02-across-and-through}

[e02-guide]: ../lessons/057-measure-across-and-through/index.md

**Idea:** Voltage is between two points; current passes through a part.
**Before:** E01. **Use:** Lesson 57's slow Blink build and a DC voltmeter.
**Predict, do, see:** Predict whether the LED and resistor each get all 5 V.
While the LED stays on, measure pin 26 to GND and across each part; the
two part readings add to the pin voltage. Divide the resistor voltage by
220 Ω to find the current through both series parts. **ADK connection:**
Adds measurements to [Blink](../lessons/001-blink/index.md).

### E03 — [Resist the flow](../lessons/058-resist-the-flow/index.md) {#e03-resist-the-flow}

**Idea:** More resistance gives less current at the same supply voltage.
**Before:** E02. **Use:** Lesson 58's slow Blink build, meter, and 220 Ω,
1 kΩ and 2 kΩ resistors. **Predict, do, see:** Predict which resistor makes
the LED dimmest; use each resistor in turn, unplugging before every swap.
The LED dims as resistance rises. Measure the voltage across each resistor
while lit and divide by its resistance to estimate the series current.
Voltage × current gives the power the resistor turns into heat. **ADK
connection:** Varies [Blink](../lessons/001-blink/index.md).

## 2. Sharing current and voltage

### E04 — [Resistors in series](../lessons/059-resistors-in-series/index.md) {#e04-series-resistors}

**Idea:** Series parts carry the same current and share the voltage.
**Before:** E03. **Use:** Lesson 59's Mega USB 5 V build: two 1 kΩ resistors
and a meter. **Predict, do, see:** Predict the voltage across each equal resistor;
wire them end to end across 5 V and measure each and the pair. Each reads
about half the supply; the two readings add to the whole. Swap one for
2 kΩ and see the larger share move to it. **ADK connection:** Explains
the [knob divider in Lesson 7](../lessons/007-dimmer/index.md).

### E05 — [Branches in parallel][e05-guide] {#e05-parallel-branches}

[e05-guide]: ../lessons/060-branches-in-parallel/index.md

**Idea:** Parallel branches have the same voltage and their currents add.
**Before:** E04. **Use:** Lesson 60's Mega USB 5 V build: two red LEDs, two
1 kΩ resistors, meter. **Predict, do, see:** Predict what adding a second
LED-and-resistor branch will do to the first LED. Add it across the same
rails, not in line with the first. Both light about as brightly. Measure
the voltage across each 1 kΩ resistor, divide by 1 kΩ for each branch's
current, and add those currents for the current feeding the two LED branches.
**ADK connection:**
Extends the separate LEDs in [Lesson 2](../lessons/002-buttons/index.md).

### E06 — [Tap a divider](../lessons/061-tap-a-divider/index.md) {#e06-tap-a-divider}

**Idea:** A potentiometer is an adjustable voltage divider. **Before:**
E04. **Use:** Lesson 61's knob-only Mega build and a DC voltmeter.
**Predict, do, see:** Predict the wiper voltage halfway around;
measure A0 to GND while turning the knob, and watch the Serial Plotter.
The voltage sweeps from about 0 to 5 V while the plotted A0 reading goes
from 0 to 1023. **ADK connection:** Lesson 7 already builds the divider;
E06 adds a meter reading at its documented A0 connection.

## 3. Stored charge and one-way paths

### E07 — [Charge a capacitor](../lessons/062-charge-a-capacitor/index.md) {#e07-charge-a-capacitor}

**Idea:** A capacitor stores separated charge. **Before:** E02. **Use:**
Lesson 62's Mega USB 5 V build, 10 kΩ, 1000 µF capacitor and voltmeter.
**Predict, do, see:** Put the capacitor's marked − leg at GND, charge it
through 10 kΩ and watch its voltage rise toward 5 V. Unplug USB, then
move only the rail end of the charging jumper from top + to bottom −; the
capacitor discharges through 10 kΩ and the meter falls gradually, so stored
energy was available
after the source left. Never short its legs.
**ADK connection:** The slow change helps explain
[Lesson 12's timer](../lessons/012-stopwatch/index.md), although that timer
runs in code.

### E08 — [Time an RC pair](../lessons/063-time-an-rc-pair/index.md) {#e08-time-an-rc-pair}

**Idea:** Resistance times capacitance sets a charging timescale.
**Before:** E07. **Use:** E07's USB 5 V build, another 10 kΩ, stopwatch and
meter. **Predict, do, see:** Time from 0 V to about 3.2 V with 10 kΩ.
Unplug USB and discharge as in E07 through the charging resistor to GND;
verify near 0 V before charging through two 10 kΩ in series.
Predict and observe roughly 10 s then 20 s; the exact times depend on
the parts and your timing. **ADK connection:** Gives a physical counterpart to
the waits in [Lesson 12](../lessons/012-stopwatch/index.md).

### E09 — [One-way diode](../lessons/064-one-way-diode/index.md) {#e09-one-way-diode}

**Idea:** A diode conducts mainly in one direction. **Before:** E03.
**Use:** Lesson 64's Mega USB 5 V build: 1N4007, red LED and 1 kΩ resistor.
**Predict, do, see:** Wire 5 V → resistor → diode → LED → GND; the
banded diode end faces the LED. Predict what reversing only the diode will
do. The LED lights in the first orientation and stays dark in the second;
its resistor stays in place for both trials. **ADK connection:** The same
diode appears across the buzzer in [Lesson 3](../lessons/003-reaction-duel/index.md).

## 4. Electronic switches and magnetism

### E10 — [Control with a transistor][e10-guide] {#e10-transistor-switch}

[e10-guide]: ../lessons/065-control-with-a-transistor/index.md

**Idea:** A small base current controls a larger collector current.
**Before:** E09. **Use:** Lesson 65's Mega USB 5 V build: S8050, button, red
LED, 220 Ω LED resistor, 1 kΩ base resistor and 10 kΩ base pull-down.
Check the marked S8050's E–B–C pin order. **Predict, do, see:** Put the
LED and its resistor between + and collector, emitter at GND, and the
button through 1 kΩ from + to base. Predict the LED state before pressing;
it is off until the button supplies base current. **ADK connection:**
Isolates the switch used for the [Lesson 3 buzzer](../lessons/003-reaction-duel/index.md).

### E11 — [An inductor resists a change][e11-guide] {#e11-inductor-current}

[e11-guide]: ../lessons/066-inductor-current/index.md

**Idea:** An inductor makes current change gradually. **Before:** E10.
**Use:** Lesson 66's generator build: 0–4 V square wave, 1 kΩ resistor, 100 mH
inductor, two-channel scope. **Predict, do, see:** At 100 Hz, compare the
generator edge with the voltage across the 1 kΩ resistor, which stands for
current. Wire generator output → inductor → 1 kΩ → GND. Put both scope
ground clips at GND, channel 1 on generator output and channel 2 at the
inductor/resistor junction. With the inductor, channel 2 rises over roughly
0.1 ms; replace the inductor with a wire and the edge is much sharper.
**ADK connection:** The coil in [Lesson 3's buzzer](../lessons/003-reaction-duel/index.md)
is why its switch has a protective diode.

### E12 — [Give a coil a safe path](../lessons/067-coil-diode/index.md) {#e12-coil-diode}

**Idea:** A flyback diode gives coil current a path when its switch opens.
**Before:** E10–E11. **Use:** Lesson 67's Mega USB 5 V build: kit passive buzzer,
S8050, button, 220 Ω buzzer resistor, 1 kΩ base resistor, 10 kΩ base
pull-down, 1N4007, and red LED with its own 1 kΩ resistor; a scope is
optional. **Predict, do, see:** Wire
5 V → 220 Ω → buzzer + → buzzer − → collector; emitter goes to GND. Put the button
through 1 kΩ from 5 V to base and 10 kΩ from base to GND. Fit the diode
directly across the buzzer from the start: its banded end at buzzer + and
unbanded end at collector. The separate resistor–LED branch also feeds
the collector. Predict a lit LED while the button is held and a faint
click as you press and release.
With a scope, probe the collector against GND to see a brief clamped
transient on release. The 220 Ω stays in series with the roughly 16 Ω coil;
the 5 V rail, not a Mega I/O pin, supplies its current. **ADK connection:**
Combines
[Lesson 5's passive buzzer](../lessons/005-melody-maker/index.md) with the
transistor and flyback diode pattern in
[Lesson 3](../lessons/003-reaction-duel/index.md).

## 5. Alternating signals

### E13 — [Current reverses](../lessons/068-alternating-current/index.md) {#e13-alternating-current}

**Idea:** Alternating current flows first one way, then the other.
**Before:** E07, E11. **Use:** Lesson 68's generator build: 0–4 V, 1 kHz sine wave
(2 V offset), 1 µF nonpolar film capacitor, 1 kΩ resistor, scope.
**Predict, do, see:** Connect generator → capacitor → resistor → ground.
With both scope ground
clips at circuit ground, DC-couple channel 1 at the generator output and
channel 2 at the resistor's top. Predict whether channel 2 can go below
ground. Channel 1 stays between 0 and 4 V while channel 2 swings above
and below 0 V: the capacitor removes the generator's 2 V DC offset and
current through the resistor reverses. **ADK connection:** Deepens the
waveform idea behind the [Lesson 5 buzzer](../lessons/005-melody-maker/index.md).

### E14 — [Count a waveform][e14-guide] {#e14-frequency-and-period}

[e14-guide]: ../lessons/069-frequency-and-period/index.md

**Idea:** Frequency counts cycles per second; period is time per cycle.
**Before:** E13. **Use:** E13's circuit, generator and scope. **Predict,
do, see:** Change only the generator from 100 Hz to 1 kHz. Predict how many
cycles fit in 10 ms; the scope shows about one, then ten. Measure one
period as about 10 ms, then 1 ms. **ADK connection:** Explains why changing
the note in [Lesson 5](../lessons/005-melody-maker/index.md) changes pitch.

### E15 — [Filter and phase](../lessons/070-filter-and-phase/index.md) {#e15-filter-and-phase}

**Idea:** An RC low-pass filter reduces fast changes and delays the output.
**Before:** E08, E14. **Use:** Lesson 70's generator build: 0–4 V sine wave,
1 kΩ series resistor, 1 µF film capacitor from output to ground, both
scope channels. **Predict, do, see:** Compare input and output at 100 Hz,
then 1 kHz. The output is smaller at 1 kHz, and its peaks come later than
the input peaks; the capacitor's charge cannot follow as quickly.
**ADK connection:** Gives a circuit explanation for smoothing the
[Lesson 7 dimmer](../lessons/007-dimmer/index.md).

## 6. Gain, feedback and clean power

### E16 — [Make a small signal larger](../lessons/071-amplifier-gain/index.md) {#e16-amplifier-gain}

**Idea:** An amplifier changes a signal by a chosen gain. **Before:** E15.
**Use:** Lesson 71's USB 5 V MCP6002 build, four 10 kΩ resistors,
100 nF and 10 µF supply capacitors, generator and scope. Place both
capacitors across the supply near the chip, with the 10 µF + leg at 5 V.
Wire the unused amplifier as a follower: its + input uses a 2.5 V divider
and its output joins its − input. **Predict, do, see:** Wire amplifier A as a non-inverting
amplifier with one 10 kΩ from output to − input and one from − input to
GND; feed + input a 0.5–1.5 V sine wave at 100 Hz. Predict the
output range; it follows at about 1–3 V, twice the input. **ADK connection:**
Shows what can happen inside the analog sensors used in
[Lesson 8](../lessons/008-light-meter/index.md).

### E17 — [Feed back the output](../lessons/072-negative-feedback/index.md) {#e17-negative-feedback}

**Idea:** Negative feedback makes an output follow a reference. **Before:**
E16. **Use:** Lesson 72's USB-powered MCP6002 with both nearby supply capacitors
and its unused amplifier terminated as in E16; add the kit's potentiometer,
scope or meter, and a 1 kΩ load resistor. **Predict, do, see:** Connect the op-amp
output to its − input (a follower), the knob's wiper to + input and 1 kΩ
from output to
GND. Turn the knob between about 1 V and 3 V; input and output traces
track even while the output feeds the load. **ADK connection:** The knob
comes from [Lesson 7](../lessons/007-dimmer/index.md); this is a new
buffer circuit, not a change to that lesson's wiring.

### E18 — [Keep the supply steady](../lessons/073-power-integrity/index.md) {#e18-power-integrity}

**Idea:** Supply resistance makes load changes disturb the local voltage;
a nearby capacitor reduces short ripples. **Before:** E07, E10, E14.
**Use:** Lesson 73's USB 5 V build: 10 Ω in the + feed, E10's transistor
and LED load pattern, generator at 1 kHz square wave, 1 kΩ base resistor,
10 kΩ base pull-down, 100 µF and 100 nF capacitors, scope. **Predict, do,
see:** Copy E10's LED and transistor path onto locally fed rails, omitting
the button. Drive the base from generator output through 1 kΩ; keep the
10 kΩ pull-down and join generator ground to Mega GND. Probe the local + rail with the scope
ground at GND, AC-coupled at 50 mV/div (or with a vertical offset). The rail
dips by roughly 0.1 V when the LED lights. Add 100 µF and 100 nF across
the local rails, with 100 µF + at local + and − at GND; the ripple shrinks.
**ADK connection:** Helps explain why
[Lesson 15's sensors and display](../lessons/015-weather-station/index.md)
share power and need a sound ground path.

## 7. Logic, memory and timing

### E19 — [Make a truth table](../lessons/074-nand-logic/index.md) {#e19-nand-logic}

**Idea:** A logic gate maps input levels to one output level. **Before:**
E05. **Use:** Lesson 74's USB 5 V build: 74HC00, two buttons with 10 kΩ
pull-downs, red LED with 1 kΩ resistor, 100 nF supply capacitor. Tie unused
gate inputs to GND. **Predict, do, see:** Try no buttons, each button alone,
then both. The NAND output LED is on except when both inputs are high;
write the four-row truth table. **ADK connection:** Turns the button
decisions in [Lesson 2](../lessons/002-buttons/index.md) into hardware.

### E20 — [Remember one bit](../lessons/075-set-reset-latch/index.md) {#e20-set-reset-latch}

**Idea:** Feedback can hold a digital state after an input ends. **Before:**
E19. **Use:** Lesson 75's USB 5 V build: E19's 74HC00, two buttons with 10 kΩ
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
**Before:** E08, E19. **Use:** Lesson 76's USB 5 V build: 74HC14,
100 kΩ feedback resistor, 10 µF capacitor with − at GND, red LED with
1 kΩ resistor, 100 nF supply capacitor. Tie unused inputs to GND.
**Predict, do, see:** Feed one inverter's output back to its input through
100 kΩ and put the capacitor from input to GND. The LED on the output
blinks; change to two 100 kΩ resistors in series and it blinks more slowly.
**ADK connection:** A physical cousin of the timed events in
[Lesson 12](../lessons/012-stopwatch/index.md).

## 8. Measuring and sending with the Mega

### E22 — [Sample a voltage](../lessons/077-sampling/index.md) {#e22-sampling}

**Idea:** Sampling turns a changing voltage into separate numbered readings.
**Before:** E06. **Use:** Lesson 77's version of
[Lesson 7's Dimmer build](../lessons/007-dimmer/index.md),
meter and Serial Plotter. **Predict, do, see:** Turn the knob slowly and
compare the wiper voltage with the plotted 0–1023 reading; near 2.5 V it
is near 512. The printed readings move in whole steps while
the voltage changes smoothly. At a boundary, adjacent numbers may flicker
because of electrical noise. **ADK connection:** Reuses
Lesson 7's sketch and wiring exactly.

### E23 — [Average PWM](../lessons/078-pwm-average/index.md) {#e23-pwm-average}

**Idea:** PWM changes its on-time, and a filter can turn that into an
average voltage. **Before:** E15, E22. **Use:** Lesson 78's Mega build based on
[Lesson 7](../lessons/007-dimmer/index.md): pin 3's LED keeps its 220 Ω
resistor; add a separate 10 kΩ from pin 3 to a meter point and 100 µF from
that point to GND (− leg to GND). **Predict, do, see:** Turn the knob from
low to high; the LED brightens while the filtered meter point rises from
about 0 to 5 V after settling for several seconds. On the scope, pin 3
still switches between 0 and 5 V. The filter branch is separate from the
LED branch.

### E24 — [Send a byte down a wire](../lessons/079-serial-link/index.md) {#e24-serial-link}

**Idea:** A serial port can receive the bytes it sends when its TX and RX
pins have a complete path.
**Before:** E19, E22. **Use:** Lesson 79's Mega build: one 1 kΩ resistor between
Serial1 TX1 (pin 18) and RX1 (pin 19), a 10 kΩ pull-up from RX1 to 5 V,
and an E24 sketch that sends a known byte and prints what Serial1 receives
to the USB Serial Monitor.
**Predict, do, see:** The same Serial1 UART sends and receives the byte;
predict a matching echo. Unplug, remove the link and power again, and no
byte returns. This loopback checks the path, not timing between two devices.
The resistor limits current if a pin is accidentally configured as an
output. **ADK connection:** The wire and shared timing are the small-scale
version of the messages in [Lesson 38](../lessons/038-radio-messages/index.md).
