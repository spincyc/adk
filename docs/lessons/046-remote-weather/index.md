---
lesson: 46
promise: Put four sensors in the garden and read them indoors, with missing or stale readings marked and the time the latest sensor record arrived.
time: 90 minutes
level: 3
parts:
  - "Board A, the garden: Lesson 45's Board A, with its LoRa modem and divider"
  - "Board A: the DHT11 module, the 18B20 module (37 in 1 kit), the 10 kΩ thermistor and the photoresistor"
  - "Board A: 2 × 10 kΩ resistors (brown, black, black, red, brown), a green LED, a 220 Ω resistor, 8 jumper wires and 6 female-to-male jumper wires"
  - "Board B, indoors: Lesson 45's Board B, with its LoRa modem and divider (a second Mega and breadboard aren't in one kit, and the modems are an add-on)"
  - "Board B: the LCD, its contrast knob and 220 Ω resistor, as in Lesson 13, and the DS1307 clock module from Lesson 32"
  - "Board B: the RGB LED, 3 × 220 Ω resistors (red, red, black, black, brown), 21 jumper wires and 4 female-to-male jumper wires"
ideas:
  - Readings as whole numbers, in tenths
  - How often to send
  - A report number carried with each reading
  - Stale news, and saying so
---

## What you'll build

<!-- closeup B -->

A weather station in two halves. Board A goes in the garden with four
sensors: the DHT11 for the air's temperature and humidity, the 18B20 and
the thermistor for two more temperatures, and the photoresistor for the
daylight. Every five seconds it sends a report over the bridge. Board B,
indoors, shows the readings on the screen one after another, with the time
the latest sensor record arrived, and its light glows blue, green or red for a
cold, mild or hot garden. Unplug the garden and, a few seconds later, the
screen says **No news** and the light goes out.

As before, send on 915 MHz only where it's allowed: see
[Radios](../../safety.md#radios).

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

**A report number with each reading.** Board B needs to know when a new
measurement arrives, even when its value is unchanged. Every five seconds
Board A adds one to `reports`. It sends each reading with that number:
`bridge.shareEvent ("air", reports, temperature)`. The number and reading
travel as one record. Board B gets the report number from `value ("air")`
and the temperature from `payload ("air")`. Different sensor records may
arrive in different packets; there is no promise of one complete report.

**Stale news.** Board B keeps a stopwatch for each of the five readings.
A new `air` record restarts only the air stopwatch. A radio heartbeat or
a new light reading cannot make old air data fresh. After ten seconds
without a new reading, that value becomes `----`; a reading that has never
arrived also shows dashes. The bottom row says **Stale** if any reading is
missing or old, or **No news** if the radio has not been heard in five
seconds. Its time is the latest arrival of any sensor record, not a
measurement time shared by all sensors.

A broken thermometer is different again. The garden checks each digital
thermometer's `ok ()`, and sends `noReading`, −10000, instead of its last
good reading when it fails. That number is outside its measuring range.
Indoors shows `----` for an invalid reading. The comfort light needs a
fresh, valid air temperature; otherwise it fades out.

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
home in column 37, as in Lesson 8. The thermistor needs a second divider
in column 33, with its 10 kΩ resistor along row c from c33 to c36. The green
LED on pin 28, at its home in column 18, shows the link. Check each module's **S**, **+** and
**−** before you plug it in.

<!-- bench A -->

<!-- steps A -->

When you are done, these are the connections Board A makes:

<!-- connections A -->

### Board B, indoors

Its modem goes back to Serial3: take out the wires from pins 16 and 17, and
put pins 14 and 15 into j28 and j26 again. The screen goes at its home, as
in Lesson 13, and the clock module on its side above it, as in Lesson 32.
The RGB LED keeps its home from Lesson 15: its longest leg in the bottom
− rail by column 7, red in a6, green in a9 and blue in a11, each color's
resistor across the gap above it.

<!-- bench B -->

<!-- steps B -->

When you are done, these are the connections Board B makes:

<!-- connections B -->

## Code it

Each board has its own sketch. Open **File → Examples → Adk → lessons →
046-remote-weather → Garden** for Board A:

<!-- sketch A -->

What's new:

- `adk::Dht11`, `adk::Ds18b20`, `adk::Thermistor` and `adk::AnalogInput`
  are Lesson 14's thermometers and Lesson 8's light sensor, just as they
  were. `online` is the green LED, lit while `bridge.isConnected ()`.
- `report` ticks every five seconds. Then the sketch adds one to
  `reports` and shares each reading with that number.
- `tenths ()` turns a temperature in degrees into whole tenths:
  `lround ()` rounds to the nearest whole number, so 21.46 °C becomes
  214.6 and then 215.
- `noReading` marks a failed DHT11 or 18B20 reading. Its value travels in
  the same slot as the temperature, so a validity flag cannot arrive
  separately from the reading it describes.
- Five names on the bridge: `air`, `humid`, `probe`, `ntc` and `light`. A bridge holds up to eight, each up to seven letters.

Then **File → Examples → Adk → lessons → 046-remote-weather → Indoors** for
Board B:

<!-- sketch B -->

What's new:

- `names` lists the five readings; `age` holds a stopwatch for each.
  The loop restarts a stopwatch only when its own record changes. `fresh ()`
  checks presence, age and radio contact before a value can be used.
- `bridge.payload ("air")` is the air temperature, in tenths. For a valid,
  fresh temperature, `printReading ()` divides by `10.0` to keep the
  fraction, and `adk::fixed` shows one decimal. Other values stay whole.
- `comfortOf ()` picks blue, green or red from the tenths, as `colorOf ()`
  did in Lesson 15: 180 tenths is 18 °C.
- `page` ticks every three seconds, and `showWeather ()` shows the next of
  three readings on the top row with a `switch`. The bottom row is the time
  of the latest sensor record. **Heard** means all five readings are
  fresh, **Stale** means some are not, and **No news** means radio silence.
- The light needs a fresh, valid air reading. Radio silence, old air
  data or a failed DHT11 fade it out.

## Upload it

1. Plug in Board A and upload **Garden** to it. Its green LED stays off:
   indoors isn't talking yet.
2. Plug in Board B and upload **Indoors** to it. Its screen stays blank
   for about three seconds, until the first page. Until the first report
   the screen says `Waiting for data` and `No report yet`.
3. Within a few seconds, Board A's green LED lights, Board B's light glows
   and the screen shows something like:

    ```text
    Air 21.5°C  45%
    Heard   14:32:05
    ```

    Every three seconds the top row moves on: the 18B20 and the thermistor
    side by side (`DS 21.1 NTC 21.4`), then `Light 64%`, and round again,
    so the first page you see may well be the `DS` one. Radio heartbeats alone do not count as a weather report.

4. Cover the light sensor with a finger. The next report, up to five
   seconds later, says the garden went dark. Pinch the thermistor's bead:
   its number climbs, a report at a time.

Now test your prediction: unplug Board A. For up to five seconds, nothing
changes: that's how long the bridge waits before it gives up on the other
board. Then the light fades out, and within three more seconds the bottom
row says `No news` with the time of the last sensor record. The top row
shows dashes once contact is lost. Plug Board A back in, and new sensor
records restore their readings.

!!! question "Predict: a failed thermometer"
    If the DHT11 stops answering but Board A keeps sending reports, should
    the screen show the old temperature with a new time?

Unplug Board A, remove only the DHT11's signal wire, and power it again.
The next air and humidity records show dashes and the light stays off; the other
sensors can still report. Unplug before restoring the wire.

## If it doesn't work

| What you see | Try this |
|---|---|
| Board B always says `No news` and Board A's green LED stays off | The boards don't hear each other. Check each modem as in Lesson 43: TXD into f26, pin 15 into j26, pin 14 into j28 with the 1 kΩ and 2 kΩ, RXD into c28, GND into the bottom − rail by column 24 and VDD on the 3.3V pin. |
| Board A's green LED is on, but Board B says `No news` | Board B hears nothing, but Board A hears Board B: check Board B's TXD and pin 15, and Board A's RXD and its divider. |
| Air or humidity shows `----` | The reading is missing, old or invalid. If the radio is connected, check the DHT11: check S to pin 16, + to the top + rail by column 36, − to the top − rail by column 37, and wait two seconds. |
| `DS ----` | The reading is missing, old or invalid. Check the 18B20's Y pin (the signal) goes to pin 17, its R to the top + rail and its G to the top − rail. |
| `NTC` shows about −77 or hundreds | As in Lesson 14: the thermistor's legs in f33 and e33, the red wire from j33 to the top + rail, the 10 kΩ from c33 to c36. |
| `Light` stays at 0 or 100 | Check the photoresistor in f37 and e37, the red wire from j37 to the top + rail by column 37, the 10 kΩ from a37 to the bottom − rail, and A1's wire in c37. |
| The time stays `00:00:00` after reports arrive, or is wrong | Check the clock module's SDA on pin 20 and SCL on 21. Lesson 32 shows how to set it. |
| The light never comes on | Check its longest leg is in the bottom − rail by column 7, and each color's wire: pin 5 to j6, pin 6 to j9, pin 7 to j11. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    Records look like this:

    ```text
    @air=12:215 humid=12:45 probe=12:211
    @ntc=12:214 light=12:64
    ```

    A line may have 56 characters, so the bridge can split a report
    between packets. Each colon keeps a reading with its own report
    number. Losing the first packet cannot make its three readings fresh
    merely because the second arrived. Every two seconds the bridge
    repeats its current records; repeated numbers do not restart ages.
    The latest record may replace an older unsent one, so this is not a
    complete measurement log.

    Board B sends `@` heartbeats because it shares no values. Its screen
    updates every three seconds, so `No news` can appear up to three
    seconds after the bridge gives up.

## Make it yours

1. **No flicker.** Bring back Lesson 15's gap: a garden that hovers
   around 25 °C makes the light swap between green and red at each
   report. Change `comfortOf ()` so it changes the mood only a whole
   degree past each edge, as `judgeComfort ()` did.
2. **Seconds ago.** Use each reading's existing stopwatch to show its
   age beside it. Why might the two temperatures have different ages?
3. **Highs and lows.** Keep the lowest and highest air temperatures since
   Board B started, in tenths, and show them as a fourth reading on the top
   row.
4. **A real garden.** Put Board A in a box in the shade, run it from a USB
   power bank, and see how far you can take it before `No news` comes up.
   A waterproof 18B20 probe on a long cable can measure the soil.
5. **Once a minute.** Make the report beat 60 seconds. Reports now become
   `Stale` after ten seconds even though radio heartbeats continue. Change
   `staleAfter` to 120000 on Board B. Why are these separate checks?

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
- The number crossed the bridge as a plain whole number, `light=12:50`: the
  voltage on one board became a word on the other.
