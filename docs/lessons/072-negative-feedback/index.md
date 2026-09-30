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
  - 7 more jumper wires
ideas:
  - Negative feedback makes an output follow an input while feeding a load
---

## What you'll build

<!-- closeup -->

Keep [E16's MCP6002 amplifier](../071-amplifier-gain/index.md),
its supply capacitors, and its second amplifier's steady connection.
Change amplifier A into a **voltage follower**: its output connects
straight back to its − input. A knob sets the + input; a 1 kΩ resistor
loads the output. Compare the two voltages as you turn the knob.

## Predict

Set the knob's wiper to about **2 V**. With a 1 kΩ resistor from the
output to GND, will the output stay near 2 V or fall much lower? Predict
what will happen at about **1 V** and **3 V** too, before measuring.

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
capacitors, and the two 10 kΩ resistors that hold amplifier B's + input
at a midpoint. Keep B's output (pin 7) joined to its − input (pin 6).
Keep the Mega's GND and 5 V wires in their usual rail holes nearest it.

Remove E16's generator and its two wires. Remove **only** the two
10 kΩ gain resistors near amplifier A and their wires. Connect A's
output (pin 1) directly to its − input (pin 2). Connect the kit knob's
wiper to A's + input (pin 3), with the knob's outer legs at GND and 5 V.
The 1 kΩ load goes from A's output to GND. The knob stays at its usual
home and also connects to A0 so the sketch can show its setting.

<!-- bench -->

<!-- steps -->

Use the generated connections to check the three separate A nodes:
pin 3 with the wiper, pins 1 and 2 together, and the load's other end
at GND.

<!-- connections -->

## Compare the voltages

1. With USB still unplugged, connect both scope ground clips to the
   bottom − rail. Put channel 1's tip on a free hole in **pin 3's lower
   strip** and channel 2's tip on a free hole in **pin 1's lower strip**.
   Set both channels to DC coupling. Keep the metal tips apart.
2. Plug in USB. Turn the knob until channel 1 reads about **1 V**,
   then **2 V**, then **3 V**. At each setting, record both channels.
   The output should be close to the input at each setting. The 1 kΩ
   load remains connected throughout.
3. Compare the readings with your prediction. At a middle setting,
   the two flat traces should sit at nearly the same voltage. If you
   have only a voltmeter, put its black lead in COM and its red lead in
   the voltage socket. Set it to DC volts. Leave the black probe on GND
   and measure the wiper and output in turn at each knob setting.

| Knob setting | Your input voltage | Your loaded output voltage |
|---|---:|---:|
| Near 1 V | ____ V | ____ V |
| Near 2 V | ____ V | ____ V |
| Near 3 V | ____ V | ____ V |

The Mega sketch also prints the knob's A0 reading from **0 to 1023**.
This number helps you return to a setting; the scope or meter compares
the actual voltages. The chip's output is not connected to a Mega signal
pin.

## Why it happens

The op-amp changes its output until the voltage at its − input is close
to the voltage at its + input. Here the output itself feeds the − input,
so the output settles near the knob's voltage. The 1 kΩ resistor draws
current from the op-amp output to GND while this feedback holds the
voltage near the reference. Middle settings around 1–3 V leave room
between the output and both supply rails; do not expect exact tracking
at 0 V or 5 V.

## Check your result

<!-- measure -->

The input and loaded output should rise together and be close at each
middle setting. Their difference need not be zero. This expected result
comes from the circuit and [MCP6002 datasheet](https://ww1.microchip.com/downloads/en/devicedoc/mcp6001-1r-1u-2-4-1-mhz-low-power-op-amp-ds20001733l.pdf);
no physical hardware trial has been recorded for this lesson.

## If the readings surprise you

| What you see | Check with USB unplugged |
|---|---|
| Output stays near 0 V or 5 V | Check chip notch, pin 8 to 5 V, pin 4 to GND, and the direct pin 1 to pin 2 feedback wire. |
| Input does not change | Check both outer knob legs go to opposite rails and its wiper reaches pin 3. |
| Input changes but output differs greatly | Check the 1 kΩ load reaches output and GND, and that neither output nor the wiper is shorted to a supply rail. |
| A scope trace moves when a ground clip moves | Put both ground clips on the common bottom − rail and use DC coupling. |

## About the sketch

The analog feedback happens in the MCP6002. The sketch reads the knob
on A0 and prints its position; it does not drive the output:

<!-- sketch -->
