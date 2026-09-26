---
lesson: 37
promise: Build a real FM radio, tune it with the rotary knob, and see each station's name on the screen.
time: 1 hour
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - The LCD, knob and 220 Ω resistor from Lesson 13, wired as before
  - Si4703 FM radio board, CJMCU-470, with its pins soldered on (add-on, not in the kit)
  - Wired earbuds or headphones with a 3.5 mm plug (not in the kit)
  - Rotary encoder module
  - The kit's second 10 kΩ potentiometer, for the volume
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 5 female-to-male jumper wires
  - 8 jumper wires
ideas:
  - How an FM station sends sound, and its name
  - Tuning, and seeking the next station
  - A 3.3 V chip that the Mega only ever pulls down
  - Two resistors in a tug-of-war
---

## What you'll build

<!-- closeup -->

A radio. Plug in your earbuds and press the rotary knob: the radio sweeps
up the band and stops at the first station it can hear clearly. The screen
shows its frequency, **98.8 MHz**, whether it is in **Stereo**, and a bar
for how strong it is. A moment later the station's own name appears, sent
through the air along with the music. Turn the rotary knob and the radio
steps along the band a click at a time; turn the volume knob and the music
gets louder or softer.

## The idea

**Sound on a radio wave.** A radio station sends a wave that swings back and
forth millions of times a second: 98.8 MHz means 98.8 million times. To
carry music, the station makes that speed wobble a tiny bit, faster and
slower, in step with the sound. That is **frequency modulation**, FM. The
chip on the radio board, the **Si4703**, listens to one frequency at a time,
follows the wobble, and turns it back into sound for your earbuds.

The board has no aerial of its own: the **earbuds' cable** is the aerial.
That's why it only finds stations with earbuds plugged in.

**Stations sit on a ladder.** FM stations use the band from 87.5 to
108 MHz, on steps of 0.1 MHz, so tuning is choosing a step. One click of the
rotary knob moves one step; pressing it asks the chip to **seek**: to
step up by itself until it hears a station strong enough to be clear. The
strength is measured in dBµV, from 0 to 75: above about 25 a station is
clear, and below 15 it is mostly hiss. The sketch draws it as a bar, one
block for every 10.

In North and South America the steps are 0.2 MHz, so stations always end
in an odd number, such as 88.1 or 101.5, and the radio also has to cut the
treble a little differently. The first lines of the sketch choose where you
are:

```cpp
constexpr adk::FmBand band = adk::FmBand::World;
```

Leave it as `World`, or change it to `Americas` if you live there.

**A name in the air.** Many stations also send **RDS**, a slow trickle of
letters alongside the music: their name, eight letters long, and a line of
text, often the song that's playing. The name arrives two letters at a time,
so it takes a second or two to appear, and only if the signal is good.

**A 3.3 V chip.** The radio works at 3.3 V, and its pins must never see the
Mega's 5 V. So the Mega never *pushes* a 1 on its three wires. To send a 0
it connects the wire to GND; to send a 1 it lets go, and the radio board's
own resistors lift the wire up to 3.3 V. The Mega counts anything above
about 3 V as a 1, so it can read the radio's answers too.

The reset wire, **RST**, is the odd one out: the board holds it *low* with a
10 kΩ resistor, which keeps the chip asleep. So you add a **1 kΩ** resistor
from RST up to 3.3 V. The two resistors pull against each other, and the
1 kΩ, ten times stronger, wins: RST sits at about 3 V, high enough to count
as a 1, and the chip wakes. When the Mega wants to reset the radio, it pulls
RST down to 0 V, which beats both.

!!! question "Predict"
    Once the radio has found a station, pull the earbuds' plug out of its
    socket. You won't hear anything, of course, but the screen keeps
    showing the signal. What do you think will happen to the bar?

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring, and check your work
    before you plug it back in.

!!! danger "3.3 V only"
    The radio's **3.3V** pin goes to the Mega's **3.3V** pin, on the short
    header beside 5V, never to 5V. None of the radio's pins may touch 5 V,
    and the sketch never drives them high: see
    [Safety](../../safety.md#radios).

Keep the screen from Lesson 36 as it is, with the Mega's GND and 5V wires,
and take everything else off, the power module too: nothing here needs it.

Most of these radio boards come with their row of eight pins loose, and the
pins have to be soldered on before the board can stand in the breadboard.
If you haven't learned to solder yet, ask someone who has, or buy a board
with its pins already fitted.

The radio stands in row j, columns 45 to 52, its board lying back over the
top rails, like the accelerometer in Lesson 28. Its three signal wires, and
its 3.3 V, come round the bottom of the screen and up into row f. The volume
knob stands at its home beside the screen, and the rotary encoder sits at
its home above the Mega, as in Lesson 29.

<!-- bench -->

<!-- steps -->

??? info "The radio's eight pins"
    From the left, as they stand in row j:

    | Pin | Job | Goes to |
    |---|---|---|
    | GPIO2, GPIO1 | Spare outputs of the chip | Nothing |
    | RST | Reset: low keeps the chip asleep | Pin 42, and 1 kΩ to 3.3 V |
    | SEN | For another way of talking to the chip | Nothing |
    | SCLK | The clock: one pulse for each bit | Pin 41 |
    | SDIO | The data, both ways | Pin 40 |
    | GND | Ground | The bottom − rail |
    | 3.3V | Power | The Mega's 3.3V pin |

    The 1 kΩ lies along row h, from h47 in RST's column to h52 in the
    3.3V column. Plug the earbuds into the socket on the side of the board.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 037-fm-radio**:

<!-- sketch -->

What's new:

- `constexpr adk::FmBand band` says where you are listening, as in the idea
  above.
- `adk::FmRadio radio {40, 41, 42, band};` is the radio, with its SDIO,
  SCLK and RST pins, in that order. `adk::setup ()` wakes it, which takes
  about 0.6 s while its crystal settles, and tunes it to the bottom of the
  band.
- `dial` is the rotary encoder from Lesson 29 and `dialButton` its push
  switch; `volumeKnob` is the potentiometer on A0, read as in Lesson 7. The
  potentiometer beside the LCD only sets the contrast: on this page, *the
  rotary knob* is the encoder and *the volume knob* is the one on A0.
- `radio.step (dial.turned ())` moves one step for each click, up for
  clockwise and down for anticlockwise, and round from the top of the band
  to the bottom. `radio.seekUp ()` finds the next station up.
- Tuning takes about 60 ms, and a seek up to a few seconds, but neither
  stops the sketch: the radio gets on with it while `loop ()` carries on.
  `radio.isTuning ()` is true meanwhile, and the screen says `Tuning`.
- `radio.setVolume (volumeKnob.read (0, 15))` turns the knob's 0 to 1023
  into the radio's volumes, 0 for silent to 15, with `read (0, 15)` as in
  Lesson 7. Each change is a message on the wires, so ADK only sends one
  when the number changes: the sketch can ask five times a second.
- `radio.frequency ()` is in tenths of a megahertz: 988 means 98.8 MHz.
  Dividing by `10.0`, with its decimal point, keeps the tenths, and
  `adk::fixed (..., 1)` prints the number with one decimal place. A
  frequency under 100 MHz gets a space in front, so the digits line up.
- `radio.stationName ()` is the station's RDS name, or empty text until one
  arrives. `radio.signal ()` is the strength in dBµV, and dividing by 10
  gives the number of blocks, drawn from column 9 after `lcd.at (9, 1)`
  moves there. `block`, 0xFF, is a character the LCD always has, with every
  dot lit.
- `radio.ok ()` says whether the radio answered when the Mega started. If it
  didn't, the screen says so, as the clock did in Lesson 32.

## Upload it

Plug your earbuds into the radio, then upload the sketch. After a moment the
top row shows ` 87.5 MHz`, the bottom of the band. If the text is faint or
missing, turn the contrast knob beside the LCD.

1. Press the rotary knob. The screen says `Tuning` while the radio seeks,
   then stops on a station: its frequency on top, and its signal bar below.
2. Turn the volume knob until the music is comfortable. All the way down is
   silent.
3. Wait a few seconds. If the station sends RDS, its name appears in the
   bottom left corner.
4. Press again for the next station, or turn the rotary knob a click at a
   time to step along the band. Between stations the bar drops and you hear
   hiss.

Now try your prediction. Pull the earbuds' plug out: the bar drops, a
strong station to a block or two and a weak one to nothing, and the station
may go to `Mono`. The cable was the aerial, and without it the radio hears
almost nothing. Plug them back in and the bar comes back. Try holding the
cable straight, or near a window: a longer, straighter aerial catches more
of the wave.

## If it doesn't work

| What you see | Try this |
|---|---|
| The screen says **No radio found!** | Check pins 40, 41 and 42 go to f50, f49 and f47, and that the radio's 3.3V and GND are wired. The 1 kΩ must join h47 to h52: without it the chip never wakes. |
| Seek finds nothing, or only hiss | Plug the earbuds in all the way: they are the aerial. Try by a window, away from the computer. |
| Turning skips stations you know are there | Check `band`: only in the Americas should it be `Americas`, whose steps are 0.2 MHz. |
| The volume knob does nothing | Check A0's wire goes to a58, the red jumper from d59 to T+61, and the black one from a57 to the − rail. |
| Turning the rotary knob goes the wrong way | Swap its CLK and DT wires, on pins 18 and 19. |
| It takes two clicks to move one step | Your encoder steps differently: give it a third number, as in Lesson 29, `adk::RotaryEncoder dial {18, 19, 2};`, and try 2 or 1. |
| The name never appears | Not every station sends one. Try a strong, big station. |
| A row of solid blocks, or a blank lit screen | Turn the contrast knob, the one beside the LCD. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    SDIO and SCLK are a two-wire bus, like I2C in Lesson 28, and ADK drives
    it one bit at a time. It can't use the Mega's own I2C pins, 20 and 21,
    because the Mega board has resistors pulling those up to 5 V, which the
    radio mustn't see. A pin set as an input, with its pull-up off, has let
    go; set as an output at 0 V, it pulls down. ADK only ever switches
    between those two.

    The chip keeps its settings in sixteen numbers called registers. Tuning
    writes the wanted step into one of them; the chip takes up to 60 ms to
    settle on it, and sets a flag when it's done. ADK leaves the chip alone
    until then, because talking to it mid-tune can knock it off. Seeking is
    the same, except the chip steps along by itself until it finds a station
    at least 25 dBµV strong.

    While it's playing, ADK reads the chip every 40 ms for the signal,
    stereo and any RDS letters that have arrived. RDS comes in groups of
    four numbers, and each group says what it carries: two letters of the
    name, or letters of the text. A group with too many errors in it is
    thrown away, so on a weak station the name can take a while.

    FM stations boost their treble before sending, and a radio cuts it back
    by the same amount; the Americas boost it a little more than the rest of
    the world. `band` sets that as well as the steps.

## Make it yours

1. **Seek down.** Wire a push button on pin 23, at its home beside the
   screen in columns 38 to 40, and make it call `radio.seekDown ()`.
2. **Favorites.** Make an `adk::Array` of your three favorite stations, in
   tenths of a megahertz, such as `988`. Make pressing the rotary knob go to
   the next one with `radio.tune ()`, instead of seeking.
3. **What's playing?** Open the Serial Monitor with `Serial.begin (9600)` in
   `setup ()`, and print `radio.radioText ()` whenever
   `radio.textChanged ()`. Many stations send the song and the artist.
4. **In numbers.** Show the signal as a number, `radio.signal ()` then
   `dBuV`, instead of a bar. Walk around the room with the radio and find
   the best spot.
5. **Remember.** Save the frequency in EEPROM, as the safe in Lesson 18
   saved its code, and tune back to it in `setup ()`.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Keep each probe tip in
its own hole, so it can't touch the one beside it.

!!! question "Predict"
    The Mega runs on 5 V, and its pin 40 is wired straight to the radio's
    SDIO. Between messages nothing is pulling SDIO down. What will it read:
    5 V, 3.3 V, or 0 V?

<!-- measure -->

What the numbers tell you:

- **SDIO resting** reads about 3.3 V, not 5 V. The Mega has let go of it,
  and the only thing holding it up is the radio board's resistor to its own
  3.3 V. The Mega only ever pulls it down, 25 times a second for a moment
  while it reads the chip's news, too briefly for the meter to show.
- **RST** reads about 3.0 V. The 1 kΩ pulls it up towards 3.3 V and the
  board's 10 kΩ pulls it down towards 0 V, and the stronger one wins most of
  the tug-of-war: 3.3 V × 10 ÷ 11 = 3.0 V. That's still well above what the
  chip needs to count it as awake.
- **The supply** is the Mega's 3.3V pin, from a small regulator on the Mega
  that makes 3.3 V out of its 5 V. The radio takes only a little current,
  well within what that pin can give.
