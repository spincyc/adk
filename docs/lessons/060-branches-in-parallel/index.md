---
lesson: 60
promise: See two LED branches share one voltage while their currents add.
time: 25 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 2 red LEDs
  - 10 Ω resistor (brown, black, black, gold, brown)
  - 2 × 1 kΩ resistors (brown, black, black, brown, brown)
  - 6 jumper wires
  - Digital multimeter with DC volts
ideas:
  - Parallel branches share a voltage, and their currents add
---

## What you'll build

<!-- closeup -->

Two red LEDs light from the Mega's USB-powered 5 V supply. Each has its own
1 kΩ resistor, and both take their current through one shared **10 Ω**
resistor. You will light one branch, then both, and use a meter to watch
the current through the shared resistor double while the first LED stays
as it was.

## Predict

A **branch** is one complete path from the supply to the − rail. Here the
first path goes through one 1 kΩ resistor and one LED, and the second
through another resistor and LED. They meet only at their ends, so they
are **in parallel**: each has the same voltage between its ends, about
5 V. The current for both comes in through the 10 Ω resistor, splits
between the branches, and comes back together at the − rail.

Start with only the first branch connected. When you add the second
branch, will the first LED get brighter, dimmer, or stay about as bright?
What will happen to the current through the shared 10 Ω resistor? Write
down your guesses before the trial.

## Build it

!!! warning "Unplug first"
    Take the USB cable out before changing any wire. Check that each LED
    has a 1 kΩ resistor in its own path before powering the build. The
    10 Ω resistor carries only a few milliamps here. If it ever gets warm,
    unplug at once: a wire is joining column 4 to the − rail.

Keep the Mega's GND wire in the bottom − rail hole nearest it and its 5 V
wire in the top + rail hole nearest it. Take out the other parts and wires
from E04. The 10 Ω resistor stands from the top + rail by column 4 into
**j4**. Column 4's holes f–j then feed both branches: a short jumper from
i4 reaches the first branch's 1 kΩ resistor in column 6, and a longer one
from h4 the second branch's in column 12. The long leg of each LED faces
its resistor; the short leg faces the − rail.

<!-- bench -->

<!-- steps -->

The breadboard joins holes a–e in each column and holes f–j in that column,
but not across the middle gap. Each 1 kΩ resistor crosses that gap. The two
branches join each other only at column 4's upper strip and at the bottom
− rail.

<!-- connections -->

## Try it

Set the meter to DC volts (**V⎓**): black lead in **COM**, red lead in
**V**. On its 20 V range the 10 Ω readings are a few hundredths of a volt;
a 2 V or 200 mV range, if your meter has one, shows them in more detail.
Leave the meter in voltage mode throughout, and keep the metal probe tips
from touching each other.

1. Check both paths. Lift out the jumper from h4 to h12, so that only the
   first branch is connected, then plug USB in. Only the first LED should
   light; notice its brightness. Measure across the 10 Ω resistor, red on
   the top + rail by column 7 and black on **g4**, and across the first
   1 kΩ resistor, red on **h6** and black on **d6**. Record both.
2. Unplug USB. Put the jumper back from h4 to h12, then plug USB in.
   Compare the first LED with your prediction and look for the second LED.
   Measure the 10 Ω resistor and the first 1 kΩ resistor again, then the
   second 1 kΩ resistor, **i12** to **d12**, and each whole branch.

<!-- measure -->

| Branches connected | Across 10 Ω | Across first 1 kΩ | Across second 1 kΩ |
|---|---:|---:|---:|
| First only | ____ V | ____ V | — |
| Both | ____ V | ____ V | ____ V |

The first LED should look about as bright in both trials, and its 1 kΩ
reading should hardly change: roughly 3 V each time. The 10 Ω reading
should about double, from roughly 0.03 V to 0.06 V. Each whole branch
reads about 5 V. The two LEDs need not match each other exactly, even
when both are red.

## Why it happens

Each branch begins at the same feed and ends at the same − rail, so both
get the same voltage. The second branch gives current another path; it
does not put a second resistor in the first LED's path. That is why adding
it leaves the first LED about as bright.

All the current for both branches passes through the 10 Ω resistor, so its
voltage shows the total. For 10 Ω, **10 mV across it means 1 mA through
it**: about 30 mV is 3 mA for one branch, and about 60 mV is 6 mA for two.
For a 1 kΩ resistor, **1 V across it means 1 mA**, so about 3 V across
each is about 3 mA in each branch. The two branch currents add up to the
current through the feed: **3 + 3 = 6 mA**. That is the current feeding
the LED branches, not the whole USB current: the Mega itself also uses
power. The 10 Ω resistor takes only a few hundredths of a volt, too little
to dim either LED.

## Check your result

Did the first LED's brightness change when you reconnected the second
branch? Did the 10 Ω reading about double? Add your two branch currents
and compare the sum with the current from your 10 Ω reading. Write one
sentence explaining the split and the sum.

## If it doesn't work

| What you see | Try this, with USB unplugged |
|---|---|
| Neither LED lights | Check the Mega's 5 V and GND rail wires, and the 10 Ω resistor from the top + rail by column 4 into j4. |
| One LED is dark | Check its jumper from column 4, and that its long leg faces its resistor and its short leg faces −. |
| The 10 Ω reading shows 0.00 | Use a smaller range if your meter has one, and check the probes are on the top + rail and g4, on either side of the 10 Ω resistor. |
| The meter reads a negative voltage | Swap its probes; keep the red lead in the V jack. |
| The 10 Ω resistor gets warm | Unplug at once and look for a wire joining column 4 to the − rail. |

## About the sketch

The circuit lights from the Mega's **5 V power pin**, not a programmable
signal pin. No upload is needed: plugging in USB powers the build even if
the Mega has an older sketch. The matching ADK sketch claims no I/O pins.
This expected result has not been tested on hardware:

<!-- sketch -->
