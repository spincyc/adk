---
lesson: 46
promise: Put four sensors in the garden and read them indoors, with the time of the latest report and a warning when the news stops.
time: 90 minutes
level: 3
parts:
  - "Board A, the garden: Lesson 45's Board A, with its LoRa modem and divider"
  - "Board A: the DHT11 module, the 18B20 module (37 in 1 kit), the 10 kΩ thermistor and the photoresistor"
  - "Board A: 2 × 10 kΩ resistors (brown, black, black, red, brown), a green LED, a 220 Ω resistor, 10 jumper wires and 6 female-to-male jumper wires"
  - "Board B, indoors: Lesson 45's Board B, with its LoRa modem and divider (a second Mega and breadboard aren't in one kit, and the modems are an add-on)"
  - "Board B: the LCD, its contrast knob and 220 Ω resistor, as in Lesson 13, and the DS1307 clock module from Lesson 32"
  - "Board B: the RGB LED, 3 × 220 Ω resistors (red, red, black, black, brown), 20 jumper wires and 4 female-to-male jumper wires"
ideas:
  - Readings as whole numbers, in tenths
  - How often to send
  - A report number that marks each new report
  - Stale news, and saying so
---

## What you'll build

<!-- closeup B -->

A weather station in two halves. Board A goes in the garden with four
sensors: the DHT11 for the air's temperature and humidity, the 18B20 and
the thermistor for two more temperatures, and the photoresistor for the
daylight. Every five seconds it sends a report over the bridge. Board B,
indoors, shows the readings on the screen one after another, with the time
the latest report arrived, and its light glows blue, green or red for a
cold, mild or hot garden. Unplug the garden and, a few seconds later, the
screen says **No news** and the light goes out.

## The idea

**Whole numbers, in tenths.** The bridge from Lesson 43 keeps named
numbers the same on both boards, and they are whole numbers only: a
`long`, like `215` or `-3`, never `21.5`. A temperature has a fraction
worth keeping, so Board A sends it in **tenths of a degree**: it
multiplies by ten and rounds, and Board B divides by ten again.

<p class="formula">21.5 °C × 10 = 215 → across the bridge → 215 ÷ 10 = 21.5 °C</p>

Sending plain 21 would lose the half degree. The humidity is fine in whole
percent, and so is the light: `light.read (0, 100)` turns the light
sensor's 0 to 1023 into 0 to 100, as the knob did in Lesson 15.

**How often.** Every message keeps the air busy for about a twentieth of a
second, and the bridge sends a change as soon as it can, up to ten times a
second. Some readings wobble all the time: the light sensor's number
changes by one or two on almost every read, and the thermistor's tenths
flick up and down. Shared on every pass of `loop ()`, they would keep the
radio talking ten times a second about nothing. The weather changes over
minutes, so the garden sends a **report** every five seconds, and in
between the bridge only repeats itself every two seconds, so each board
knows the other is still there.

**A report number.** Board B needs to know when a report arrives, even
when every reading in it is the same as the last. So each report carries a
number that goes up by one every time, `report`: 1, 2, 3... Board B's
`bridge.changed ("report")` is true once for each new report, and that is
when it reads the clock.

**Stale news.** A number on a screen doesn't say how old it is. A garden
reading from an hour ago looks just like one from now, so Board B shows
the time each report arrived, from the clock module. And the bridge says
whether the other board has been heard in the last five seconds: when it
hasn't, the bottom row says **No news** and the light fades out, so the
last reading can't pass for today's weather.

!!! question "Predict"
    Once it's running, you'll unplug Board A's USB cable. How long until
    Board B notices? What will the light do, and what will the top row of
    the screen show then? Write down your guesses.

## Build it

!!! warning "Unplug first"
    Unplug both boards before you wire. Keep each board's LoRa modem at
    its bridge home, with its 1 kΩ and 2 kΩ divider and its VDD on the
    Mega's 3.3V pin, just as in Lesson 45, and take everything else off.
    Neither board has a motor or a servo this time, so neither needs the
    power module: each Mega's 5V feeds its top rails.

### Board A, the garden

The garden's sensors go where Lesson 14 put them. The DHT11 and the 18B20
sit above the board at their homes. The light sensor's divider takes its
home in column 40, as in Lesson 8, so the thermistor's divider, built the
same way, stands in column 33 instead. The green LED on pin 28, at its
home in column 18, shows the link. Check each module's **S**, **+** and
**−** before you plug it in.

<!-- bench A -->

<!-- steps A -->

When you are done, these are the connections Board A makes:

<!-- connections A -->

### Board B, indoors

The screen goes at its home, as in Lesson 13, and the clock module on its
side above it, as in Lesson 32. The RGB LED has the same shape as beside
the screen in Lesson 15, but the modem has those columns now, so it stands
just past the modem: its longest leg in the bottom − rail at B-49, red in
a48, green in a51 and blue in a53, each color's resistor across the gap
above it.

<!-- bench B -->

<!-- steps B -->

When you are done, these are the connections Board B makes:

<!-- connections B -->

## Code it

Each board has its own sketch. Open **File → Examples → Adk →
lessons/046-remote-weather → Garden** for Board A:

<!-- sketch A -->

What's new:

- `adk::Dht11`, `adk::Ds18b20`, `adk::Thermistor` and `adk::AnalogInput`
  are Lesson 14's thermometers and Lesson 8's light sensor, just as they
  were. `online` is the green LED, lit while `bridge.isConnected ()`.
- `report` ticks every five seconds. Then the sketch adds one to
  `reports` and shares it, and each reading after it.
- `tenths ()` turns a temperature in degrees into whole tenths:
  `lround ()` rounds to the nearest whole number, so 21.46 °C becomes
  214.6 and then 215.
- Six names on the bridge: `report`, `air`, `humid`, `probe`, `ntc` and
  `light`. A bridge holds up to eight, each up to seven letters.

Then **File → Examples → Adk → lessons → 046-remote-weather → Indoors** for
Board B:

<!-- sketch B -->

What's new:

- `bridge.changed ("report")` is true in the one pass of `loop ()` in
  which a new report arrives. Then `rtc.now ()` reads the clock once, and
  the light fades to the garden's color.
- `bridge.value ("air")` is the latest air temperature, in tenths.
  `celsius ()` divides a temperature by `10.0`, with the point, so that the
  answer keeps its fraction, and `adk::fixed` from Lesson 15 shows it,
  here with one decimal.
- `comfortOf ()` picks blue, green or red from the tenths, as `colorOf ()`
  did in Lesson 15: 180 tenths is 18 °C.
- `page` ticks every three seconds, and `showWeather ()` shows the next of
  three readings on the top row with a `switch`. The bottom row is the time
  of the latest report, after **Heard** while the garden is heard and
  **No news** once it isn't.
- `!bridge.isConnected ()` fades the light out whenever the garden has
  been silent for five seconds.

## Upload it

1. Plug in Board A and upload **Garden** to it. Its green LED stays off:
   indoors isn't talking yet.
2. Plug in Board B and upload **Indoors** to it. Until the first report
   the bottom row says `No news 00:00:00`: 00:00:00 means never.
3. Within five seconds, Board A's green LED lights, Board B's light glows
   and the screen shows something like:

    ```text
    Air 21.5°C  45%
    Heard   14:32:05
    ```

    Every three seconds the top row moves on: the 18B20 and the thermistor
    side by side (`DS 21.1 NTC 21.4`), then `Light 64%`.

4. Cover the light sensor with a finger. The next report, up to five
   seconds later, says the garden went dark. Pinch the thermistor's bead:
   its number climbs, a report at a time.

Now test your prediction: unplug Board A. For up to five seconds, nothing
changes: that's how long the bridge waits before it gives up on the other
board. Then the light fades out, and within three more seconds the bottom
row says `No news` with the time of the last report. The top row keeps
showing the last readings: they are the last news there is, and the time
says how old it is. Plug Board A back in, and the next report brings
everything back.

## If it doesn't work

| What you see | Try this |
|---|---|
| Board B always says `No news` and Board A's green LED stays off | The boards don't hear each other. Check each modem as in Lesson 43: TXD into f44, pin 15 into j44, pin 14 into j46 with the 1 kΩ and 2 kΩ, RXD into c46, GND into B-42 and VDD on the 3.3V pin. |
| Board A's green LED is on, but Board B says `No news` | Board B hears nothing, but Board A hears Board B: check Board B's TXD and pin 15, and Board A's RXD and its divider. |
| `Air 0.0°C  0%` | The DHT11 on Board A isn't answering: check S to pin 16, + to T+36, − to T-37, and wait two seconds. |
| `DS 0.0` | Check the 18B20's S goes to pin 17, and its + and − aren't swapped. |
| `NTC` shows about −77 or hundreds | As in Lesson 14: the thermistor's legs in f33 and e33, the red wire from j33 to the top + rail, the 10 kΩ from c33 to c36. |
| `Light` stays at 0 or 100 | Check the photoresistor in f40 and e40, the red wire from j40 to T+40, the 10 kΩ from c40 to c43, and A1's wire in a40. |
| The time is `00:00:00`, or wrong | Check the clock module's SDA on pin 20 and SCL on 21. Lesson 32 shows how to set it. |
| The light never comes on | Check its longest leg is in B-49, and each color's wire: pin 5 to j48, pin 6 to j51, pin 7 to j53. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    Each report goes out as one line of text, with every name that has
    changed:

    ```text
    @report=12 air=215 humid=45 probe=211 ntc=214 light=64
    ```

    That's 54 characters, and a line may have 56, so a whole report fits
    in one message. A longer one, with minus signs and hundreds, would go
    as two, a tenth of a second apart. Every two seconds each bridge sends
    all its names again, so a message lost to noise is soon made good, and
    Board B, which shares nothing, sends just the `@`, so that Board A
    knows it's there.

    Board B's clock is read only when a report arrives, which takes about
    a millisecond. The screen is written every three seconds, so the
    `No news` can come up to three seconds after the bridge gives up.

## Make it yours

1. **No flicker.** Bring back Lesson 15's gap: a garden that hovers
   around 25 °C makes the light swap between green and red at each
   report. Change `comfortOf ()` so it changes the mood only a whole
   degree past each edge, as `judgeComfort ()` did.
2. **Seconds ago.** Add an `adk::Stopwatch` to Board B, restart it with
   each report, and show how many seconds ago it came instead of the time,
   as Lesson 41 did.
3. **Highs and lows.** Keep the lowest and highest air temperatures since
   Board B started, in tenths, and show them as a fourth reading on the top
   row.
4. **A real garden.** Put Board A in a box in the shade, run it from a USB
   power bank, and see how far you can take it before `No news` comes up.
   A waterproof 18B20 probe on a long cable can measure the soil.
5. **Once a minute.** Make the report beat 60 seconds. What does Board B's
   bottom row look like now? Why doesn't it say `No news` between reports?

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Measure Board A while
both boards run, and watch Board B's screen for the `Light` reading.

!!! question "Predict"
    `light.read (0, 100)` turns 0 V into 0 and 5 V into 100. If A1 reads
    2.5 V, what will Board B's screen say?

<!-- measure A -->

What the numbers tell you:

- **The light divider's middle** is what A1 reads. 5 V is 100, so each
  volt is 20: 2.5 V shows as `Light 50%` indoors. Your number depends on
  your room, and the screen shows it within a report or two.
- **Covered**, the photoresistor's resistance climbs, it takes most of the
  5 V, and A1 falls towards 0: the garden has gone dark.
- The number crossed the bridge as a plain whole number, `light=50`: the
  voltage on one board became a word on the other.
