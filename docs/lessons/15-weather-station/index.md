---
lesson: 15
title: Weather Station
arc: Words and weather
promise: Build a desk weather station with a comfort light and a heat alarm you set with a knob.
time: 90 minutes
level: 2
sketch: Lesson15WeatherStation
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - The LCD, knob and 220 Ω resistor from Lesson 13, wired as before
  - DHT11 temperature and humidity module
  - RGB LED
  - 3 × 220 Ω resistors (red, red, black, black, brown)
  - The kit's second 10 kΩ potentiometer, for the alarm
  - Active buzzer
  - 3 female-to-male jumper wires and 11 more jumper wires
ideas:
  - Putting sensors, lights and a screen together
  - Thresholds with a gap between them, so nothing flickers
  - A setting chosen with a knob
---

## What you'll build

<!-- closeup -->

A little weather station for your desk. The screen shows the temperature and
the humidity, and says whether the room is **Cold**, **Comfy** or **Hot**. An
RGB light glows the same news in color: blue, green or red, fading gently from
one to the next. Turn the alarm knob to pick a temperature, and if the room
ever gets that hot, the buzzer beeps and the screen shouts **TOO HOT!**

## The idea

This project puts together four things you already know: the DHT11 from
[Lesson 14](../14-thermometers/index.md), the screen from
[Lesson 13](../13-hello-lcd/index.md), the RGB LED from Lesson 4 and a knob
from Lesson 7. The new idea is about **deciding** calmly.

Say the room is comfy up to 25 °C and hot above that. What happens when the
temperature sits right at the edge? The DHT11 reads 25, then 26, then 25, then
26, as sensors do, and a simple rule would flip the light green, red, green,
red, every two seconds. Annoying.

The cure is two thresholds with a gap between them, called **hysteresis**. It
takes 26 °C to switch from comfy to hot, but it takes 24 °C to switch back. At
25 °C nothing changes: the station stays in whichever mood it was in. A
wobble of one degree can no longer cross both lines. A home heating
thermostat works exactly like this, so the boiler doesn't click on and off all
day.

The alarm knob uses the same trick you met with the dimmer: `knob.read (10, 40)`
turns the knob's 0 to 1023 into a temperature from 10 to 40 °C. Halfway round,
511 becomes 10 + 30 × 511 / 1023, which is 24 °C (the Mega drops the fraction).

!!! question "Predict"
    Once it's running, you'll warm the DHT11 by cupping your hands round it and
    breathing gently on it, until the light turns red. Then you'll let it cool.
    At what temperature do you think the light turns red? At what temperature
    will it turn green again? Write both down.

## How the station works

The station keeps two things in mind: the room's **mood**, and whether the
**alarm** is ringing. Each changes only by these rules. The mood is judged
each time a new reading arrives, every two seconds; the alarm is checked four
times a second, so it answers the knob straight away.

| Mood now | Changes when | To |
|---|---|---|
| Comfy (green) | it is 26 °C or more | Hot (red) |
| Comfy (green) | it is 17 °C or less | Cold (blue) |
| Hot (red) | it is 24 °C or less | Comfy (green) |
| Cold (blue) | it is 19 °C or more | Comfy (green) |

| Alarm | Changes when | To |
|---|---|---|
| Quiet | it is as hot as the knob's setting | Ringing: a beep every second, **TOO HOT!** |
| Ringing | it is a degree below the setting, or you turn the knob up past the temperature | Quiet |

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you wire. Keep Lesson 13's screen as it is and
    add the rest to the right. Two things to check twice: the RGB LED's
    longest leg is the common one and goes in f26, and the buzzer's + mark
    goes in f42. The two long wires at the far end join the bottom rails to the
    top ones, so the alarm knob gets power too.

<!-- bench -->

<!-- steps -->

??? info "Two knobs"
    The LCD's contrast knob and the alarm knob are both 10 kΩ potentiometers;
    the Mega kit comes with two. If one has gone missing, use the one you have
    for the alarm, and give the screen a fixed contrast instead: take out the contrast knob and the brown wire from c2 to
    c7, and put a 1 kΩ resistor (brown, black, black, brown, brown) from c5 to
    c7, between the LCD's VSS and V0. Many screens read well like that; if
    yours is too faint or too dark, you'll need a second knob after all.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson15WeatherStation**:

<!-- sketch -->

What's new:

- The parts: the screen and DHT11 you know, `light` (the RGB LED, from
  Lesson 4), `knob` (Lesson 7), `buzzer` (Lesson 3), and two beats: `refresh`
  four times a second for the knob, the alarm and the screen, and `beat` once
  a second for the alarm's beeps.
- `enum class Comfort { Cold, Comfy, Hot };` names the three moods, as
  `State` named the duel's states in Lesson 3.
- `comfyLow`, `comfyHigh` and `margin` are `float`s, like the readings they
  are compared with, so a margin of half a degree would work too.
- `loop ()` does three jobs. When a new reading arrives, `judgeComfort ()`
  decides the mood. Four times a second it reads the knob, checks the alarm
  and redraws the screen. And while the alarm rings, it beeps once a second.
- `judgeComfort ()` is the first table above, written in C++. The mood
  changes only from where it is now. `auto was = comfort;` remembers the mood
  it had, so the light only fades when the mood really changes, and
  `light.fadeTo ()` takes a second to glide to the new color.
- `checkAlarm ()` has the same kind of gap: it starts ringing at the setting,
  and stops a degree below it. `beat.restart ()` starts the once-a-second
  beeps from that moment.
- `showWeather ()` writes each row with `lcd.at ()` and one `adk::print ()`.
  `adk::fixed (dht.temperature (), 0)` is the reading with no decimals, as
  `lcd.print (value, 0)` printed it in Lesson 14, but it can go in a row of
  pieces. `ringing ? "TOO HOT! " : "Alarm at "` picks how the bottom row
  starts.
- `colorOf ()` and `nameOf ()` turn a mood into a color and a word, each with
  a `switch`. A `case` that `return`s needs no `break`, because `return`
  leaves the function at once. `default:` catches every value without a
  `case` of its own, here Comfy. The words are padded with spaces, so a short
  word rubs out a longer one.

## Upload it

Upload the sketch. The light glows green, and for a second the screen says
`Measuring...`; then it shows something like this, with the knob's setting
on the bottom row:

```text
23°C 45% Comfy
Alarm at 30°C
```

Turn the knob and the setting follows it, from 10 °C to 40 °C. Now test your
prediction: set the alarm to 28 °C and warm the DHT11 with your hands and
breath. At 26 °C the light fades to red and the screen says **Hot**. If it
reaches 28 °C, the buzzer beeps every second and the bottom row reads
**TOO HOT!** Turn the knob up past the temperature and it stops. Let the sensor
cool, and the light stays red at 25 °C, turning green only at 24 °C. So the
light turns red at 26 °C and green again at 24 °C: not the same temperature
both ways, which is the gap doing its job.

## If it doesn't work

| What you see | Try this |
|---|---|
| `Measuring...` never goes away | The DHT11 isn't answering: check S to pin 16, + to the + rail and − to the − rail at the far end. |
| The light stays off | Check the RGB LED's longest leg is in f26 and the black wire from j26 goes to the − rail. |
| One color is missing | Follow that color's pin: pin 5 to j22 and the resistor h22–h25 (red), pin 6 to j31 and g27–g31 (green), pin 7 to j35 and h28–h35 (blue). |
| The light shows the wrong colors | The LED is in back to front, or the pin wires are swapped. |
| The alarm setting is stuck at 10 or 40 | The knob has no power: check the long red and black wires from the top rails to the bottom rails, and a37 to + and a39 to −. |
| The setting runs backwards | That's fine, or swap the red and black wires on the knob's outer legs. |
| No beep when it says **TOO HOT!** | Check the buzzer's + leg is in f42 with pin 12's wire in j42, and j45 goes to −. |

??? note "How it works"
    Nothing in `loop ()` ever waits. The DHT11 takes a reading every two
    seconds by itself, and `dht.measured ()` is true for just the one pass of
    `loop ()` in which a new reading arrives, so the mood is judged once per
    reading. `refresh` and `beat` are `adk::Every` beats from Lesson 11. The
    buzzer's `beep (150)` switches it on and lets `adk::update ()` switch it
    off 150 ms later, while everything else carries on. The RGB LED's fade is
    worked out in steps inside `adk::update ()` too, so the colors glide even
    while the screen is being redrawn.

## Make it yours

1. **Your comfort.** Change `comfyLow`, `comfyHigh` and `margin` to suit your
   room. What happens with a margin of 0, and why is that a bad idea?
2. **Too dry, too damp.** Add a humidity mood: below 30 % the air is dry,
   above 60 % it's damp. Show it on the bottom row when the alarm isn't
   ringing, with its own gap so it doesn't flicker.
3. **A frost alarm.** Make the alarm work the other way too: a button on pin 22
   switches it between "too hot" and "too cold".
4. **Highs and lows.** Remember the highest and lowest temperatures since the
   station started, and show them for three seconds whenever the knob is
   turned all the way down.
