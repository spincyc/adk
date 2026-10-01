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
transistor, and a diode stands across the buzzer's coil from the start.
This is the protection every switched coil in the course gets, the
pattern beside [Lesson 3's buzzer](../003-reaction-duel/index.md). With
the LED and a meter you can watch the transistor switch the coil on and
off. The diode's own work, at the instant you let go, is over in well
under a millisecond: too quick for your eyes or a meter. A
battery-powered oscilloscope can show it. The Mega supplies 5 V from
USB; no upload is needed.

## Predict

Current flows through a coil while you hold the button. When you let go,
the transistor stops feeding it, but the coil's current cannot stop at
once. Before powering the build, predict whether the LED will light while
you press, and what a meter on the transistor's collector will read with
the button held and released. Will the buzzer play a steady note while
the button is held, or might it only click as the current changes? If
you have a scope, predict where the coil's current goes at the moment you
let go, and what that does to the collector's voltage.

## Build it

!!! warning "Unplug first"
    Unplug USB before changing any wiring. Keep the **220 Ω resistor in
    series with the passive buzzer**: its coil is only about 16 Ω. Keep the
    LED's own **1 kΩ resistor in series** too. Fit the diode before powering
    the circuit and never remove it: it is the coil's safe path, and without
    it each release could put a large voltage spike on the transistor. Never
    run this buzzer directly from a Mega pin. If a part gets hot or smells,
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
collector. Check these three connections before plugging in USB. The
schematic shows the same paths; the [schematic key](../../electricity/schematics.md)
names each symbol.

<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 560 460" width="560"
     role="img" aria-labelledby="coil-diode-title coil-diode-desc">
  <title id="coil-diode-title">Coil, flyback diode and transistor switch schematic</title>
  <desc id="coil-diode-desc">Five volts feeds three branches. A 220 ohm resistor feeds the buzzer coil, whose other end is the collector of the S8050 transistor; the 1N4007 diode sits across the coil with its band at the 220 ohm side. A 1 kilohm resistor and red LED also feed the collector. The push button and a 1 kilohm resistor feed the base, and a 10 kilohm resistor joins the base to ground. The emitter goes to ground.</desc>
  <g fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round">
    <path d="M70 50 H370"/>
    <path d="M70 50 L70 330"/>
    <path d="M70 330 L84 330"/>
    <path d="M136 330 L150 330"/>
    <circle cx="88" cy="330" r="4"/>
    <circle cx="132" cy="330" r="4"/>
    <path d="M80 316 H140 M110.0 316 V302 M102.0 302 H118.0"/>
    <path d="M150 330 L175.0 330"/>
    <path d="M245.0 330 L270 330"/>
    <rect x="175.0" y="317" width="70" height="26"/>
    <path d="M270 330 L270 340.0"/>
    <path d="M270 410.0 L270 420"/>
    <rect x="257" y="340.0" width="26" height="70"/>
    <path d="M270 420 V428 M250 428 H290 M257 436 H283 M264 444 H276"/>
    <path d="M270 330 L288 330"/>
    <path d="M288 310 V350"/>
    <path d="M288 321 L370 300 L370 280"/>
    <path d="M288 339 L370 360 L370 420"/>
    <path d="M333.1 350.6 L323.5 353.4"/>
    <path d="M333.1 350.6 L326.1 343.4"/>
    <circle cx="331.0" cy="330" r="30"/>
    <path d="M370 420 V428 M350 428 H390 M357 436 H383 M364 444 H376"/>
    <path d="M370 50 L370 65.0"/>
    <path d="M370 135.0 L370 150"/>
    <rect x="357" y="65.0" width="26" height="70"/>
    <path d="M370 150 L370 159.0"/>
    <path d="M370 159.0 a9 9 0 0 1 0 18 a9 9 0 0 1 0 18 a9 9 0 0 1 0 18 a9 9 0 0 1 0 18"/>
    <path d="M370 231.0 L370 240"/>
    <path d="M370 240 L370 280"/>
    <path d="M370 150 L470 150"/>
    <path d="M470 240 L370 240"/>
    <path d="M470 240 L470.0 208.0"/>
    <path d="M470.0 182.0 L470 150"/>
    <path d="M485.0 208.0 L455.0 208.0 L470.0 182.0 Z"/>
    <path d="M485.0 182.0 L455.0 182.0"/>
    <path d="M220 50 L220 55.0"/>
    <path d="M220 125.0 L220 130"/>
    <rect x="207" y="55.0" width="26" height="70"/>
    <path d="M220 140 L220.0 162.0"/>
    <path d="M220.0 188.0 L220 210"/>
    <path d="M205.0 162.0 L235.0 162.0 L220.0 188.0 Z"/>
    <path d="M205.0 188.0 L235.0 188.0"/>
    <path d="M200.0 169.0 L184.0 177.0"/>
    <path d="M184.0 177.0 L187.8 171.1"/>
    <path d="M184.0 177.0 L191.0 177.5"/>
    <path d="M200.0 181.0 L184.0 189.0"/>
    <path d="M184.0 189.0 L187.8 183.1"/>
    <path d="M184.0 189.0 L191.0 189.5"/>
    <path d="M220 130 L220 140"/>
    <path d="M220 210 L220 280 L370 280"/>
  </g>
  <g fill="currentColor">
    <circle cx="270" cy="330" r="5"/>
    <circle cx="370" cy="150" r="5"/>
    <circle cx="370" cy="240" r="5"/>
    <circle cx="370" cy="280" r="5"/>
  </g>
  <g fill="currentColor" font-size="17" font-family="system-ui, sans-serif">
    <text x="20" y="56">5 V</text>
    <text x="82" y="285">button</text>
    <text x="210" y="310" text-anchor="middle">1 kΩ</text>
    <text x="215" y="385" text-anchor="end">10 kΩ</text>
    <text x="410" y="345">S8050</text>
    <text x="388" y="106">220 Ω</text>
    <text x="386" y="190" font-size="15">buzzer</text>
    <text x="386" y="208" font-size="15">coil</text>
    <text x="490" y="188">1N4007</text>
    <text x="490" y="208" font-size="15">band</text>
    <text x="490" y="224" font-size="15">at top</text>
    <text x="352" y="143" text-anchor="end" font-size="15">+</text>
    <text x="150" y="96">1 kΩ</text>
    <text x="238" y="180">red LED</text>
    <text x="384" y="274" font-size="14">collector, column 31</text>
  </g>
</svg>

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
3. Set the meter to **DC volts (V⎓)**, black lead in **COM**, red lead in
   **V**. Put the black probe in a free hole of the bottom − rail and the
   red probe in **e31**, a free hole in the collector's strip at column
   31. Hold the button and read the meter, then release it and read again.
   Keep the probe tips apart.

<!-- measure -->

| Button | Your collector reading |
|---|---:|
| Held | ____ V |
| Released | ____ V |

Held, the transistor is a closed switch: the collector sits near GND,
about **0.1 V**, and the 220 Ω resistor and coil share the rest of the
supply. About 4.9 V across 236 Ω means roughly **21 mA** through the coil.
Released, the switch is open, no current flows, and the collector reads
close to the **5 V** supply. The LED and the meter show only these two
steady states, the switching. They cannot show the moment of release,
when the diode does its work; only a scope can.

### If you have an oscilloscope

Read the [scope and generator primer](../../electricity/skills.md#scope-and-generator)
first. Use a battery-powered scope with its **ground clip only on the GND
rail**, and put the probe tip in e31, the collector. Start with DC
coupling, **1 V/div**, about **50 µs/div**, and the trigger on a rising
edge at about 2.5 V, in **Single** mode: the release happens once, and
Single holds it on the screen. Hold the button, arm the trigger, then
release the button: the trace jumps from near 0 V to
about **0.7 V above the 5 V supply**, stays there briefly, then settles
at 5 V. To enlarge that bump, set **0.2 V/div**, use the vertical offset
to bring the 5 V level to the middle of the screen, and move the trigger
level to about 5.3 V. How long it lasts depends on the coil: expect tens
to a few hundred microseconds. Never attach the ground clip to the
collector or buzzer legs.

## Why it happens

While the button is held, current flows from the 220 Ω resistor into
buzzer +, through the coil to buzzer −, and on through the transistor to
GND. The diode points the other way across the coil, so it carries
nothing.

When you let go, the transistor stops taking that current, but a coil's
current cannot stop at once. It keeps flowing the same way through the
coil, from buzzer + to buzzer −; only its path changes. Out of buzzer −,
it now goes through the diode, from its unbanded end to its banded end,
and back into buzzer +: round the short loop of coil and diode, until
the coil's stored energy is used up. Pushing current through the diode
lifts the collector about 0.7 V above buzzer +, which is the bump a scope
shows. Without the diode, the coil would drive the collector far higher
to keep its current moving, and that spike could damage the transistor.
That is why the diode goes in before the circuit is powered, and never
comes out.

## Check your result

Did the LED and the collector readings match your predictions for held
and released? Which part of your prediction could the meter check, and
which needed a scope? In one sentence, say where the coil's current goes
at the moment you let go.

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
