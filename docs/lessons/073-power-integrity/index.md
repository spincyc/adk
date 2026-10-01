---
lesson: 73
promise: See how nearby capacitors smooth the steps a switching load makes in a local supply.
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
  - 7 jumper wires
ideas:
  - A switching load steps its local supply voltage down; nearby capacitors smooth short steps
---

## What you'll build

<!-- closeup -->

The Mega's USB 5 V feeds a **local supply** through a 10 Ω resistor. An
isolated generator switches a red LED load on and off at 1 kHz. A scope
shows the local voltage with and without two capacitors beside that load.
The Mega supplies power; its signal pins and sketch do not make the wave.

## Predict

The LED takes current only while the transistor is on. Predict what
happens to the voltage **after the 10 Ω resistor** when the LED lights,
and for how long. Then predict how two capacitors from that local supply
to GND might change that step. Write down both predictions before
measuring.

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
resistor. Neither capacitor connects across the 10 Ω resistor. The
schematic shows the same nodes (the [schematic key](../../electricity/schematics.md)
names each symbol):

<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 620 440" width="620"
     role="img" aria-labelledby="local-supply-title local-supply-desc">
  <title id="local-supply-title">Local supply schematic</title>
  <desc id="local-supply-desc">USB 5 volts passes through a 10 ohm resistor to the local supply at column 4, which the scope probe touches at h6. A 100 microfarad capacitor, plus side up, and a 100 nanofarad capacitor join the local supply to ground. The local supply feeds a 220 ohm resistor and red LED to the S8050's collector; its emitter goes to ground. The generator drives the base through 1 kilohm, with 10 kilohms from base to ground.</desc>
  <g fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round">
    <path d="M50 60 L75.0 60"/>
    <path d="M145.0 60 L170 60"/>
    <rect x="75.0" y="47" width="70" height="26"/>
    <path d="M170 60 L500 60"/>
    <path d="M250 60 L250 119.0"/>
    <path d="M250 131.0 L250 190"/>
    <path d="M230 119.0 H270"/>
    <path d="M230 137.0 Q250 127.0 270 137.0"/>
    <path d="M250 132.0 L250 190"/>
    <path d="M250 190 V198 M230 198 H270 M237 206 H263 M244 214 H256"/>
    <path d="M370 60 L370 119.0"/>
    <path d="M370 131.0 L370 190"/>
    <path d="M350 119.0 H390"/>
    <path d="M350 131.0 H390"/>
    <path d="M370 190 V198 M350 198 H390 M357 206 H383 M364 214 H376"/>
    <path d="M500 60 L500 70.0"/>
    <path d="M500 140.0 L500 150"/>
    <rect x="487" y="70.0" width="26" height="70"/>
    <path d="M500 160 L500.0 177.0"/>
    <path d="M500.0 203.0 L500 220"/>
    <path d="M485.0 177.0 L515.0 177.0 L500.0 203.0 Z"/>
    <path d="M485.0 203.0 L515.0 203.0"/>
    <path d="M480.0 184.0 L464.0 192.0"/>
    <path d="M464.0 192.0 L467.8 186.1"/>
    <path d="M464.0 192.0 L471.0 192.5"/>
    <path d="M480.0 196.0 L464.0 204.0"/>
    <path d="M464.0 204.0 L467.8 198.1"/>
    <path d="M464.0 204.0 L471.0 204.5"/>
    <path d="M500 150 L500 160"/>
    <path d="M500 220 L500 260"/>
    <circle cx="80" cy="340" r="24"/>
    <path d="M67 340 q6.5 -12 13 0 q6.5 12 13 0"/>
    <path d="M80 316 L80 310 L110 310"/>
    <path d="M110 310 L185.0 310"/>
    <path d="M255.0 310 L330 310"/>
    <rect x="185.0" y="297" width="70" height="26"/>
    <path d="M330 310 L330 320.0"/>
    <path d="M330 390.0 L330 400"/>
    <rect x="317" y="320.0" width="26" height="70"/>
    <path d="M330 400 V408 M310 408 H350 M317 416 H343 M324 424 H336"/>
    <path d="M80 364 L80 376"/>
    <path d="M80 376 V384 M60 384 H100 M67 392 H93 M74 400 H86"/>
    <path d="M400 310 L418 310"/>
    <path d="M418 290 V330"/>
    <path d="M418 301 L500 280 L500 260"/>
    <path d="M418 319 L500 340 L500 400"/>
    <path d="M463.1 330.6 L453.5 333.4"/>
    <path d="M463.1 330.6 L456.1 323.4"/>
    <circle cx="461.0" cy="310" r="30"/>
    <path d="M330 310 L400 310"/>
    <path d="M500 400 V408 M480 408 H520 M487 416 H513 M494 424 H506"/>
  </g>
  <g fill="currentColor">
    <circle cx="170" cy="60" r="5"/>
    <circle cx="250" cy="60" r="5"/>
    <circle cx="370" cy="60" r="5"/>
    <circle cx="330" cy="310" r="5"/>
  </g>
  <g fill="currentColor" font-size="17" font-family="system-ui, sans-serif">
    <text x="14" y="66">5 V</text>
    <text x="110" y="40" text-anchor="middle">10 Ω</text>
    <text x="196" y="100" text-anchor="end" font-size="15">local supply</text>
    <text x="196" y="118" text-anchor="end" font-size="15">column 4 · probe h6</text>
    <text x="216" y="117.0" font-size="16">+</text>
    <text x="276" y="132">100 µF</text>
    <text x="396" y="132">100 nF</text>
    <text x="518" y="110">220 Ω</text>
    <text x="518" y="196">red LED</text>
    <text x="20" y="290" font-size="15">generator, 1 kHz</text>
    <text x="220" y="290" text-anchor="middle">1 kΩ</text>
    <text x="312" y="362" text-anchor="end">10 kΩ</text>
    <text x="540" y="330">S8050</text>
  </g>
</svg>

<!-- connections -->

## Try it

The [scope and generator primer](../../electricity/skills.md#scope-and-generator)
explains AC coupling and the vertical offset used here; do its output
check before connecting the generator.

The probe goes as drawn here, its ground clip in a free hole of the
bottom − rail.

<!-- probe -->

1. With USB unplugged and the generator off, clip the scope's ground lead to
   the bottom − rail by column 6. Put the probe tip in **h6**: the jumper
   from b4 brings the local supply from column 4's lower strip to the top of
   the LED's 220 Ω resistor, so this hole is on the local side of the 10 Ω
   resistor too. Set the scope near **50 mV per division** and about **1 ms
   across the screen**. Use AC coupling to enlarge the small change, or use
   DC coupling and shift the roughly 5 V trace into view with the vertical
   offset.
2. For the **without capacitors** trace, keep all power unplugged and lift
   **both capacitors** out of their holes. Leave the 10 Ω resistor, LED,
   transistor, base resistors, generator leads, and other jumpers in place.
   Plug in USB, check the generator is set to a **0–4 V square wave at
   1 kHz**, then enable its output. Record the local trace's high-to-low
   change while the LED switches. Expect a square step of roughly
   **0.13 V**, the local supply sitting low for the whole time the LED is
   lit; USB voltage and parts vary.
3. Switch the generator off and **unplug USB**. Put both capacitors back
   exactly as drawn: 100 µF **+ to the local supply**, striped **− to the
   bottom − rail**; 100 nF across those same two nets. Check the probe
   ground again. Plug in USB, enable the same generator wave, and record
   the trace with the same scope settings. Expect a much smaller ripple,
   about **30 mV** from top to bottom, its edges rounded into slopes.

At 1 kHz the LED may look steadily lit; the scope shows what changes
each cycle.

| Generator | Capacitors | Your ripple, top to bottom |
|---|---|---:|
| 1 kHz | Out | ____ mV |
| 1 kHz | In | ____ mV |
| 100 Hz | In | ____ mV |

## Why it happens

When the LED lights, about 13 mA flows through the 10 Ω resistor, which
uses a little of the USB voltage: 13 mA × 10 Ω is about **0.13 V**. With
nothing else on the local supply, that drop lasts exactly as long as the
LED is lit, so the local voltage steps down and back up with the
generator's square wave.

The capacitors hold charge on the local side of the resistor. When the
LED lights, they supply some of its current at once, and their voltage
sags only gradually as they give up charge; while the LED is off, the
10 Ω resistor recharges them. How fast that happens is set by
**10 Ω × 100 µF = 1 ms**. At 1 kHz the LED is lit for only half a
millisecond at a time, too short for the capacitors to run down far, so
the step shrinks to a small ripple. The 100 µF part does nearly all of
this work here: 10 Ω × 100 nF is only a millionth of a second. The
100 nF ceramic part is there for much faster changes, such as a logic
chip's switching edges, which this scope view is too slow to show.
Lifting out only the 100 nF should make little visible difference.

## Change one thing

Predict first: with both capacitors in place, will the ripple be smaller
or larger if the LED stays lit for ten times as long each time?

1. Leave the capacitors and probe in place. Change only the generator's
   frequency to **100 Hz**, keeping its 0–4 V square wave, and set the
   scope to about **10 ms across the screen**.
2. Record the ripple from top to bottom.

Expect about **130 mV**, nearly the whole step again, with slopes that
level off. At 100 Hz the LED is lit for 5 ms at a time, five times as long
as 10 Ω × 100 µF, so the capacitors have time to give up most of their
charge. A nearby capacitor smooths short changes in a supply, not long
ones. Turn the generator off and unplug USB when finished.

## Check your result

Compare your three ripples with your predictions. Did the capacitors
shrink the 1 kHz step? Did the ripple grow again at 100 Hz? In one
sentence, explain why the same capacitors smooth the faster load better.

## If it doesn't work

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
