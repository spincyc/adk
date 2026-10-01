# Measure, calculate, explain

Use this short routine for each [electricity investigation](index.md). The
linked lesson gives the exact probe holes and the expected range. Write your
prediction before looking at its result.

## Set up a voltage reading

1. Unplug USB or turn the generator off before placing or moving wires.
   Put the black meter lead in **COM** and the red lead in **V**. Select
   **DC volts (V⎓)**; use a range above 5 V if the meter has ranges.
2. Put one probe on each side of the thing you want to measure. For a
   point's voltage relative to GND, put black on the grounded − rail and
   red on that point. Keep the metal probe tips apart.
3. Power the circuit, wait for a steady reading, and write down the value
   **with its units and probe positions**. A minus sign means red is at
   the lower voltage; check the probe positions before changing wiring.

The meter reads a voltage **between** its probes. To find current in a
series resistor, keep the meter on DC volts, measure across that resistor,
then calculate **current = resistor voltage ÷ resistance**. For example,
3.0 V across 220 Ω gives about 0.014 A, or 14 mA. Do not put meter leads
set to current across a supply. [E02](../lessons/057-measure-across-and-through/index.md)
walks through the voltage readings and current calculation.

## Checkpoint: one LED path

In E02, predict the voltage across the steady LED and resistor. Measure
each and the whole path. Record your own three readings:

| Across resistor | Across LED | Whole path | Sum of first two |
|---:|---:|---:|---:|
| ____ V | ____ V | ____ V | ____ V |

The two part readings should add close to the whole-path reading. Use the
resistor reading to calculate the current. If the sum is far off, check
that all readings were taken at the lesson's marked holes, with the
leads still in the voltage sockets. Then
use the [diagnosis guide](diagnose.md) to trace the path.

## Use the scope and generator {#scope-and-generator}

E11, E13–E16 and E18 use a two-channel oscilloscope and a waveform
generator; E12, E17 and E23 can use the scope too, without the generator. Read this before the first one,
and do the output check below before the generator touches a circuit.

### Why they must run from batteries

A mains-powered bench scope joins its ground clips to the building's
earth through its power cord, and a desktop computer usually earths its
USB ground, so the Mega's GND is earthed too. If such a scope's ground
clip touched the 5 V rail, it would short USB 5 V to earth through the
clip lead. A scope and generator running from their own batteries are
**floating**: their ground joins the circuit only where you clip it, so a
slip cannot make that short. That is why the course asks for
battery-powered instruments. A handheld two-channel oscilloscope with a
built-in waveform generator is one kind that fits. Check that its
generator can set both amplitude and DC offset, and use it on its battery
with the charger unplugged.

**Ground clips go only on GND.** A scope's ground clips are joined to each
other inside it, and in a combined scope and generator, to the
generator's ground too. Clip them only to the circuit's GND rail: a ground
clip on any other point joins that point to GND.

### Read the screen

The trace is the voltage at the probe tip, measured from its ground clip,
drawn from left to right as time passes.

| Setting | What it means |
|---|---|
| **V/div** (volts per division) | The vertical scale: at 1 V/div, each grid square is 1 V tall. |
| **Timebase** (ms/div or µs/div) | The horizontal scale: at 1 ms/div, each grid square is 1 ms wide. A screen is often ten squares wide, so 1 ms/div shows 10 ms. |
| **Trigger** | The scope waits until one channel crosses a set level in a set direction, such as channel 1 rising through 2 V, then draws from that moment. Each sweep starts at the same point of the wave, so a repeating wave stands still. Keep the level inside the wave's range. In **Auto** mode the scope also draws when nothing triggers, which suits a steady wave; for something that happens once, such as a button's release, choose **Normal** or **Single**: Single waits, draws the first trigger it sees, then holds it on the screen. |
| **DC coupling** | Shows the whole voltage, including any steady part. Use it unless a page says otherwise. |
| **AC coupling** | Takes away the steady average and shows only the changes, so a small ripple on 5 V can be enlarged. It always centres the trace on 0 V, so it cannot show whether a signal really goes below 0 V. |
| **Offset** (vertical position) | Slides the trace up or down without changing it. With DC coupling, an offset of about −5 V brings a trace near 5 V to the middle of the screen, so a small V/div can show its changes. |
| **Peak-to-peak** (Vpp) | The height from the lowest point to the highest. A sine wave from 0 V to 4 V is 4 Vpp, and its middle, 2 V, is its average or DC offset. |
| **Probe ×1/×10** | If a probe has this switch, set the scope channel to match it, or every reading is ten times wrong. |

### Set the generator

Generators describe a 0–4 V wave in one of two ways: **amplitude 4 Vpp
with a +2 V offset**, or **high level 4 V, low level 0 V**. Both mean the
same wave. Many generators also have a load or output setting, often
**50 Ω** or **High-Z**. Choose **High-Z**: these circuits draw very little
current, and a generator set for a 50 Ω load puts out about twice the
voltage it displays, 0–8 V instead of 0–4 V.

### Check the output first

Do this whenever you set up the generator, before it is wired to any
circuit:

1. Join the generator's **OUT** straight to channel 1's probe tip and its
   **GND** to channel 1's ground clip. Leave the breadboard out of it.
2. Set channel 1 to DC coupling and **1 V/div**, the timebase to about
   **1 ms/div** for 100 Hz or **0.2 ms/div** for 1 kHz, and the trigger
   to channel 1, rising, at about 2 V.
3. Turn on the generator's output. The trace should run from **0 V to
   about 4 V**, never below 0 V and never above 4 V. If it reaches about
   8 V, switch the generator to High-Z; if it dips below 0 V, set the
   offset or low level again.
4. Turn the output off. Then wire the generator into the unpowered
   circuit as its page says.

## Keep a useful record

For each trial, write: **I changed \_\_\_; I held \_\_\_ fixed; I predicted
\_\_\_; I measured or saw \_\_\_; I now think \_\_\_ because \_\_\_.** Record an
approximate number when a number matters. A dark LED or missing trace is
an observation too; check the build before treating it as a result.

Next, try the [schematic-to-breadboard checkpoint](schematics.md#trace-one-divider)
and the [loaded-divider challenge](challenges.md#loaded-divider).
