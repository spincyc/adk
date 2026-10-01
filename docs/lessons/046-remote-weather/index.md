---
lesson: 46
promise: Put four sensors in the garden and read them indoors, with the time the latest report came, and dashes when the garden goes quiet.
time: 1½ hours
level: 3
parts:
  - "Board A, indoors: Lesson 45's Board A, with its LoRa modem, divider and screen"
  - "Board A: the DS1307 clock module from Lesson 32, the RGB LED, 3 × 220 Ω resistors (red, red, black, black, brown), 3 jumper wires and 4 female-to-male jumper wires"
  - "Board B, the garden: Lesson 45's Board B, with its LoRa modem and divider (a second Mega and breadboard aren't in one kit, and the modems are an add-on)"
  - "Board B: the DHT11 module, the 18B20 module (37 in 1 kit), the 10 kΩ thermistor and the photoresistor"
  - "Board B: 2 × 10 kΩ resistors (brown, black, black, red, brown), a green LED, a 220 Ω resistor, 9 jumper wires and 6 female-to-male jumper wires"
ideas:
  - Readings as whole numbers, in tenths
  - How often to send
  - A report number, to tell a new report from an old one
  - Saying when the news is old
laws:
  - {law: divider, section: measure-it, for: "Explains A1 falling when the photoresistor is covered"}
  - {law: sampling, section: measure-it, for: "Turns A1's 2.5 V into Light 50%"}
---

## What you'll build

<!-- closeup A -->

A weather station in two halves. Board B goes in the garden with four
sensors: the DHT11 for the air's temperature and humidity, the 18B20 and
the thermistor for two more temperatures, and the photoresistor for the
daylight. Every five seconds it sends a report over the bridge. Board A,
indoors, shows the readings on its screen one after another, with the time
the latest report came, and its light glows blue, green or red for a cold,
mild or hot garden. Unplug the garden and, a few seconds later, the
readings turn to dashes, the screen says **No news** and the light goes
out.

As before, send on 915 MHz only where it's allowed: see
[Radios](../../safety.md#radios).

## The idea

**Whole numbers, in tenths.** The bridge from Lesson 43 keeps named
numbers the same on both boards, and they are whole numbers only: a
`long`, like `215` or `-3`, never `21.5`. A temperature has a fraction
worth keeping, so the garden sends it in **tenths of a degree**: it
multiplies by ten and rounds, and Board A divides by ten again.

<p class="formula">21.5 °C × 10 = 215 → across the bridge → 215 ÷ 10 = 21.5 °C</p>

Sending plain 21 would lose the half degree. The humidity is fine in whole
percent, and so is the light: `light.read (0, 100)` turns the light
sensor's 0 to 1023 into 0 to 100, as the knob did in Lesson 15.

**How often.** Every message keeps the air busy for about a twentieth of a
second. Some readings wobble all the time: the light sensor's number
changes by one or two on almost every read. Shared on every pass of
`loop ()`, they would keep the radio talking ten times a second about
nothing. The weather changes over minutes, so the garden sends a
**report** every five seconds, and nothing new in between.

**A report number.** The weather often stays the same, so report after
report can bring the very same readings. Then the bridge sends nothing
new, and Board A can't tell a fresh report from an old one. So the garden
counts its reports, `reports`, and shares the count as one more name,
`report`. The count goes up by one every time, even when every reading
stays the same:

<p class="formula">report=12 → report=13: a new report has come</p>

Board A watches `bridge.changed ("report")`, and notes the time.

**Old news.** When the garden goes quiet, Board A still holds its last
readings. Showing them would say the garden is still 21.5 °C, when nobody
knows. So Board A shows the readings only while `bridge.isConnected ()`:
after five seconds of silence they turn to dashes, and the bottom row
says **No news**. A broken thermometer is different again: the garden is
still talking, but that thermometer has nothing to say. So the garden
sends −10000 in its place, a number no thermometer here can read, and
Board A shows dashes for it.

The radio can lose a message or two on the way, as
[Lesson 43](../043-the-bridge/index.md#what-the-bridge-can-lose)
explains; a lost report comes again within two seconds.

!!! question "Predict"
    Once it's running, you'll unplug Board B, the garden. How long until
    Board A notices? What will the light do, and what will the screen show
    then? Write down your guesses.

## Build it

!!! warning "Unplug first"
    Unplug both boards before you wire. Keep each board's LoRa modem at
    its bridge home, with its 1 kΩ and 2 kΩ divider and its VDD on the
    Mega's 3.3V pin, just as in Lesson 45. Neither board has a motor or a
    servo this time, so neither needs the power module: each Mega's 5V
    feeds its top rails.

### Board A, indoors

Board A keeps its screen from Lesson 45, and the joystick comes out. The
clock module lies on its side above the board, as in Lesson 32. The RGB
LED keeps its home from Lesson 15: its longest leg in the bottom − rail
by column 7, red in a6, green in a9 and blue in a11, each color's
resistor across the gap above it.

<!-- bench A -->

<!-- steps A -->

When you are done, these are the connections Board A makes:

<!-- connections A -->

### Board B, the garden

Lesson 45's turret comes out. The modem goes back to Serial3: its wires
from pins 16 and 17 come out, and pins 14 and 15 go into j28 and j26
again. The garden's sensors go where Lesson 14 put them. The DHT11 and
the 18B20 sit above the board at their homes. The light sensor's divider
takes its home in column 37, as in Lesson 8. The thermistor needs a
second divider in column 33, with its 10 kΩ resistor along row c from c33
to c36. The green LED on pin 28, at its home in column 18, shows the
link. Check each module's **S**, **+** and **−** before you plug it in.

<!-- bench B -->

<!-- steps B -->

When you are done, these are the connections Board B makes:

<!-- connections B -->

## Code it

Each board has its own sketch. Open **File → Examples → Adk → lessons →
046-remote-weather → Indoors** for Board A:

<!-- sketch A -->

What's new:

- `bridge.changed ("report")` is true in the one pass of `loop ()` in
  which a new report number arrives. Then `rtc.now ()` notes the time in
  `heardAt`, for the bottom row. `adk::DateTime heardAt {};` starts it
  with empty braces: every part of the date and time is 0 until a report
  comes.
- `hasNews ()` is true once a report has come, while the garden can be
  heard. `bridge.value ("report")` is 0 until the first report, as every
  value is until it arrives.
- `bridge.value ("air")` is the air temperature, in tenths. For a
  temperature, `showReading ()` divides by `10.0` to keep the fraction,
  and `adk::fixed` shows one decimal. With no news, or a thermometer that
  didn't answer, it shows `----` instead.
- `comfortOf ()` picks blue, green or red from the tenths, as `colorOf ()`
  did in Lesson 15: 180 tenths is 18 °C. Without a temperature, the light
  fades out.
- `page` ticks every three seconds, and `showWeather ()` shows the next of
  three readings on the top row with a `switch`, and the time of the
  latest report on the bottom row.

**Indoors** is longer than the sketch of a lesson like this one usually
is. It has five readings to fit on a two-row screen, so three pages of
them, and two kinds of missing news to show apart: a garden gone quiet,
and a thermometer that didn't answer. Each of those takes a few lines of
its own.

Then **File → Examples → Adk → lessons → 046-remote-weather → Garden** for
Board B:

<!-- sketch B -->

What's new:

- `adk::Dht11`, `adk::Ds18b20`, `adk::Thermistor` and `adk::AnalogInput`
  are Lesson 14's thermometers and Lesson 8's light sensor, just as they
  were. `online` is the green LED, lit while `bridge.isConnected ()`.
- `report` ticks every five seconds. Then the sketch adds one to
  `reports`, shares it, and shares every reading after it.
- `tenths ()` turns a temperature in degrees into whole tenths:
  `lround ()` rounds to the nearest whole number, so 21.46 °C becomes
  214.6 and then 215.
- `dht.ok () ? ... : noReading` is the `? :` from Lesson 12: the reading
  while the thermometer answers, and `noReading`, −10000, when it doesn't.
- Six names on the bridge: `report`, `air`, `humid`, `probe`, `ntc` and
  `light`. A bridge holds up to eight, each up to seven letters.

## Upload it

1. Plug in Board A and upload **Indoors** to it. After about three
   seconds the screen says `No report yet`.
2. Plug in Board B and upload **Garden** to it. Its green LED lights within
   a second or two, once the boards hear each other.
3. Within five seconds the first report comes. Board A's light glows, and
   its screen shows something like:

    ```text
    Air 21.5°C  45%
    Heard   14:32:05
    ```

    Every three seconds the top row moves on: the 18B20 and the thermistor
    side by side (`DS 21.1 NTC 21.4`), then `Light 64%`, and round again,
    so the first page you see may well be the `DS` one. The time changes
    with every report, every five seconds.

4. Cover the light sensor with a finger. The next report, up to five
   seconds later, says the garden went dark. Pinch the thermistor's bead:
   its number climbs, a report at a time.

You predicted what happens when the garden goes. Unplug Board B. For up to
five seconds, nothing changes: that's how long the bridge waits before it
gives up on the other board. Then the light fades out, and within three
more seconds the readings turn to dashes and the bottom row says
`No news`, with the time of the last report. Plug Board B back in: once it
has started, the boards find each other again, and its first report brings
the readings back.

!!! question "Predict: a broken thermometer"
    If the DHT11 stops answering but the garden keeps sending reports,
    what should the screen show for the air?

Unplug Board B, take out only the DHT11's signal wire, and power it again.
The next report brings dashes for the air and the humidity, and the light
stays off, while the other sensors still report. The old temperature with
a new time would have been a lie. Unplug before you put the wire back.

## If it doesn't work

| What you see | Try this |
|---|---|
| Board A always says `No news` and Board B's green LED stays off | The boards don't hear each other. Check each modem as in Lesson 43: TXD into f26, pin 15 into j26, pin 14 into j28 with the 1 kΩ and 2 kΩ, RXD into c28, GND into the bottom − rail by column 24 and VDD on the 3.3V pin. |
| Board B's green LED is on, but Board A says `No news` | Board A hears nothing, but Board B hears Board A: check Board A's TXD and pin 15, and Board B's RXD and its divider. |
| Board A says `No report yet` for more than five seconds | Check that Board B runs **Garden**, and that its modem is back on pins 14 and 15, not 16 and 17. |
| Air or humidity shows `----` | The DHT11 didn't answer. Check S to pin 16, + to the top + rail by column 36, − to the top − rail by column 37, and wait two seconds. |
| `DS ----` | Check the 18B20's Y pin (the signal) goes to pin 17, its R to the top + rail and its G to the top − rail. |
| `NTC` shows about −77 or hundreds | As in Lesson 14: the thermistor's legs in f33 and e33, the red wire from j33 to the top + rail, the 10 kΩ from c33 to c36. |
| `Light` stays at 0 or 100 | Check the photoresistor in f37 and e37, the red wire from j37 to the top + rail by column 37, the 10 kΩ from a37 to the bottom − rail, and A1's wire in c37. |
| The time stays `00:00:00` after reports arrive, or is wrong | Check the clock module's SDA on pin 20 and SCL on 21. Lesson 32 shows how to set it. |
| The light never comes on | Check its longest leg is in the bottom − rail by column 7, and each color's wire: pin 5 to j6, pin 6 to j9, pin 7 to j11. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    A report goes out as two messages, because a line holds at most 56
    letters:

    ```text
    @1/1 report=12 air=215 humid=45 probe=211 ntc=214
    @1/1 light=64
    ```

    `1/1` are the two boards' start numbers, from Lesson 43. If one of the
    two messages is lost, the bridge sends everything again two seconds
    later, and a repeat isn't a change, so the time on the screen doesn't
    move.

    Board A shares nothing, so it sends just `@1/1` every two seconds,
    which is how the garden knows it is there. Its screen changes every
    three seconds, so `No news` can come up to three seconds after the
    bridge gives up.

## Make it yours

1. **No flicker.** Bring back Lesson 15's gap: a garden that hovers
   around 25 °C makes the light swap between green and red at each
   report. Change `comfortOf ()` so it changes the mood only a whole
   degree past each edge, as `judgeComfort ()` did.
2. **Seconds ago.** Keep an `adk::Stopwatch`, restart it with each new
   report, and show how many seconds ago the report came instead of the
   time.
3. **Highs and lows.** Keep the lowest and highest air temperatures since
   Board A started, in tenths, and show them as a fourth reading on the
   top row.
4. **A real garden.** Put Board B in a box in the shade, run it from a USB
   power bank, and see how far you can take it before `No news` comes up.
   A waterproof 18B20 probe on a long cable can measure the soil.
5. **Once a minute.** Make the garden report every 60 seconds. Does Board
   A still know the garden is there between reports? Why?

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Measure Board B while
both boards run, and watch Board A's screen for the `Light` reading.

!!! question "Predict"
    `light.read (0, 100)` turns 0 V into 0 and 5 V into 100. If A1 reads
    2.5 V, what will Board A's screen say?

<!-- measure B -->

What the numbers tell you:

- **The light divider's middle** is what A1 reads. 5 V is 100, so each
  volt is 20: 2.5 V shows as `Light 50%` indoors. Your number depends on
  your room, and the screen shows it within a report or two.
- **Covered**, the photoresistor's resistance climbs, it takes most of the
  5 V, and A1 falls towards 0: the garden has gone dark.
- The number crossed the bridge as a plain whole number, `light=50`: the
  voltage on one board became a word on the other.

## Check yourself

1. Why does the garden send 215 for 21.5 °C, and what does Board A do
   with it?
2. The garden's readings haven't changed all afternoon. How does Board A
   still know when a new report has come?
3. Why does Board A show dashes, instead of the last readings, once the
   garden goes quiet?

??? note "Answers"
    1. The bridge carries whole numbers only, so the garden sends the
       temperature in tenths to keep the half degree. Board A divides by
       `10.0` and shows 21.5 again.
    2. The report number goes up by one with every report, even when every
       reading stays the same, so `bridge.changed ("report")` is true.
    3. The bridge keeps the last readings it heard, but nobody knows whether
       they are still true. Showing them would say the garden is still that
       warm; dashes say the news is old.
