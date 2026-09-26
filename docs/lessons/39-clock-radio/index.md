---
lesson: 39
promise: Build a bedside clock radio that wakes you with your favorite station, fading in gently.
time: 90 minutes
level: 3
parts:
  - Your circuit from Lesson 38, with its screen and button
  - The FM radio (add-on, not in the kit), earbuds, 1 kΩ resistor and volume knob from Lesson 37
  - DS1307 clock module with its coin cell, from Lesson 32
  - Rotary encoder module
  - 9 female-to-male jumper wires
  - 6 jumper wires
ideas:
  - Putting a radio, a clock and two knobs together
  - A device as a set of states, again
  - A volume that fades in over half a minute
---

## What you'll build

<!-- closeup -->

A clock radio for your bedside. The top row shows the time, and after a
little bell, the time of the alarm. The bottom row shows the station and its
frequency. Turn the rotary knob to tune, and press the button to switch the
radio on and off. Press the rotary knob to set the alarm: turn for the hour,
press, turn for the minutes, press again. In the morning the radio comes on
by itself, so quietly at first you can hardly hear it, and grows louder over
half a minute until it's playing as loud as the volume knob says. One press
of the button, and it's quiet again.

## The idea

There is no new part here. The FM radio and the two knobs are Lesson 37's,
the clock is Lesson 32's, and the way the alarm is set comes from the alarm
clock in Lesson 33, with times of day counted in minutes after midnight:

<p class="formula">07:30 → 7 × 60 + 30 = 450</p>

What's new is putting them together, and a gentle start. Instead of
jumping straight to full volume, the radio **fades in**. The sketch starts
an `adk::Timer` for 30 seconds when the alarm goes off, and five times a
second it sets the volume to the share of the half minute that has gone by:

<p class="formula">volume = knob × <span class="fraction"><span>time since the alarm</span><span>30 s</span></span></p>

With the volume knob at 12, the radio starts at 0, reaches 6 after 15
seconds, and 12 after 30, where it stays. The radio has sixteen volumes, 0
to 15, so it steps up about every two seconds.

## How the clock radio works

The sketch is always in one of three states, as the alarm clock was:

| State | The bottom row shows | Turning the rotary knob | Pressing the rotary knob |
|---|---|---|---|
| **Showing** | The station and its frequency | Tunes the radio | Go to SettingHour |
| **SettingHour** | `Alarm hour?` | Changes the alarm's hour | Go to SettingMinute |
| **SettingMinute** | `Alarm minute?` | Changes the alarm's minutes | Back to Showing |

The top row always shows the time and the alarm. Separately, `playing`
says whether the radio is on. The button switches it on or off at any time.
When a new minute begins that matches the alarm, while the clock is
showing the time and the radio is off, the radio switches on and the fade
begins.

!!! question "Predict"
    The volume knob is at 12, and the alarm has just gone off. Ten seconds
    later, how loud is the radio? And if you turn the volume knob down to 6
    at that moment, what happens to the rest of the fade?

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring, and check your work
    before you plug it back in. The FM radio's 3.3V pin goes to the Mega's
    **3.3V** pin, never to 5V.

!!! danger "Check your module before fitting a cell"
    As in Lesson 32: if your clock module is a Tiny RTC, or a DS3231 board
    sold as ZS-042, it charges its cell, so fit a rechargeable LIR2032, never
    a CR2032.

Keep the screen and the button from Lesson 38 just where they are, with the
two short black jumpers from f51 and a57 to the − rail: the FM radio and the
volume knob use them again. Take out the 433 MHz modules, their 1 kΩ and
2 kΩ resistors, and the rest of their wires. Then everything comes back to
its home:

- the **clock module** lies on its side above the board, as in Lessons 32
  and 33, its GND and VCC dropping into T-29 and T+30;
- the **rotary encoder** sits above the Mega on five wires, as in
  Lesson 33;
- the **FM radio** stands in row j, columns 45 to 52, as in Lesson 37, with
  its 1 kΩ along row h and its four wires coming up from below;
- the **volume knob** stands in e57 to e59, its A0 wire also coming round
  the bottom of the screen.

<!-- bench -->

<!-- steps -->

Plug the earbuds into the radio. Their cable is its aerial, so let it hang
loose rather than coiled up.

??? info "The knobs, the button and the modules"
    The rotary knob is the encoder: CLK and DT on 18 and 19, its push switch
    on 22, + from the inner 5V pin at the top of the long header, GND from
    the GND pin beside pin 13. The volume knob is the potentiometer on A0;
    the one beside the LCD only sets the contrast. The button on 23 is the
    radio's on and off switch.

    The clock module takes SDA and SCL from pins 20 and 21, and its power
    from the top rails. The FM radio takes SDIO, SCLK and RST from pins 40,
    41 and 42, its power from the Mega's 3.3V pin, and has the 1 kΩ from RST
    to 3.3 V, just as in Lesson 37.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson39ClockRadio**:

<!-- sketch -->

Read it from the top:

- `band` is Lesson 37's choice of band: change it to `Americas` if you live
  there.
- The parts: the LCD and clock from Lesson 32, the radio, `dial`,
  `dialButton` and `volumeKnob` from Lesson 37, and `radioButton`, the
  button on 23. `fade` is the `adk::Timer` that times the half minute, and
  `tick` reads the clock five times a second.
- `bell` is a little bell, drawn in eight rows of dots as your own
  characters were in Lesson 13. `setup ()` stores it in slot 1.
- `enum class State` names the three states from the table, and `alarm` is
  in minutes after midnight, as in Lesson 33. `playing` is true while the
  radio is on.
- `loop ()` hands each press and turn of the rotary knob to a function of
  its own. The button flips `playing` and stops any fade, so pressing it
  during a fade switches the radio straight off.
- `dialPressed ()` moves from state to state, as the knob did in Lesson 33,
  but with only three states: there's nothing to beep or snooze.
- `dialTurned ()` tunes the radio when the clock is showing the time, and
  otherwise changes the alarm by an hour or a minute a click. Tuning
  `return`s at once, because the alarm doesn't need wrapping round the day.
- `readTheClock ()` starts the radio and the fade only as a new minute
  begins (`time != clockTime`), exactly as Lesson 33 started its tune, and
  only if the radio isn't on already. Then it writes the top row: the time
  with `printTime ()` and the seconds, the bell with `lcd.write (1)`, and
  the alarm.
- `showBottomRow ()` prints what the rotary knob is setting, or the
  station's name and, from column 11, its frequency, as in Lesson 37.
- `setVolume ()` does the fade. `fade.remaining ()` is how much of the half
  minute is left, and 0 once it's over or when there's no fade at all, so
  `faded` is how much has gone by: all of it, except during a fade.
  `full * faded / fadeLength` is the formula from the idea. While the radio
  is off, `full` is 0, and so is the volume. As in Lesson 37, the radio
  only hears about a volume that has changed.

## Upload it

Plug in the Mega and upload the sketch. If the screen is blank or shows a
row of blocks, turn the contrast knob beside the LCD. The top row shows the
time and `07:00` after the bell. The bottom row shows ` 87.5`, the bottom of
the band.

1. Press the button: the radio comes on. Turn the rotary knob to a station
   you like; after a second or two its name appears. Set the volume knob to
   how loud you'd like to be woken.
2. Press the button to switch the radio off.
3. Set the alarm for two minutes from now: press the rotary knob, turn to
   the hour, press, turn to the minutes, and press once more. The bottom row
   shows the station again.
4. Wait. As the minute begins, the radio comes on, very quietly, and gets
   louder over half a minute.
5. Press the button, and it's quiet.

You predicted how loud the radio would be after ten seconds. It's at 4: the
knob's 12, times ten seconds out of thirty. Turn the volume knob to 6 then,
and the fade carries on from 2, a third of 6, up to 6 by the end: the
sketch works out the volume afresh every time from the knob's setting now
and the time gone by, so the whole fade shrinks to fit.

## If it doesn't work

| What you see | Try this |
|---|---|
| The screen says **No clock found!** | Check the clock's SDA goes to pin 20 and SCL to pin 21, as in Lesson 32. |
| The time is wrong | The clock keeps whatever time it was set to: see Lesson 32. |
| The bottom row stays at ` 87.5` with no name, and the radio is silent | Try Lesson 37's sketch: it says whether the radio answers. Check pins 40, 41 and 42, the 1 kΩ from h47 to h52, and the radio's 3.3V. |
| The radio never comes on by itself | The alarm only starts as a new minute begins, so set it at least a minute ahead, and press the rotary knob until the bottom row shows the station again. It won't start if the radio is already on. |
| It comes on, but stays silent | Turn the volume knob up: the fade climbs to the knob's volume, and 0 is silent. |
| The button does nothing | It must straddle the gap in columns 38 to 40, with pin 23's wire in j38 and the black jumper from a40 to the − rail. |
| Turning the rotary knob goes the wrong way | Swap its CLK and DT wires, on pins 18 and 19. |
| A row of solid blocks, or a blank lit screen | Turn the contrast knob beside the LCD, not the volume knob. |
| The **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    The clock and the radio each talk over two wires, but not the same two.
    The clock uses the Mega's own I2C pins, 20 and 21, which have pull-ups
    to 5 V on the Mega board; the DS1307 is happy with that. The radio can't
    be, so ADK talks to it on 40 and 41 instead, only ever pulling them low.

    Everything happens in small steps so nothing waits for anything else.
    The encoder is read on every `adk::update ()`, a thousand times a second;
    the clock and the screen five times a second; the radio, every 40 ms,
    for its signal and name. A tune takes the radio about 60 ms, and it gets
    on with it on its own while the clock carries on ticking.

    The volume changes in steps, because the radio has only sixteen of them.
    Each step is about 2 dB, a small change to your ears, so a fade that
    takes a step every two seconds sounds like one gentle rise.

## Make it yours

1. **Snooze.** Make pressing the rotary knob while the radio fades in
   switch it off and set it to come back five minutes later, as the
   snooze did in Lesson 33. You'll need `ringAt`, the alarm or the end of a
   snooze, in place of `alarm` in `readTheClock ()`.
2. **Sleep timer.** At night, make the radio switch itself off after 20
   minutes: start a second `adk::Timer` when the button switches it on, and
   set `playing` to false when it expires.
3. **Fade out.** Make the sleep timer fade the radio out over the last
   minute, rather than stopping it all at once.
4. **Alarm off.** Hold the button for two seconds to switch the alarm off
   for the weekend, and show the bell only while it's on.
5. **Remember the station.** Save the frequency in EEPROM whenever the
   radio switches off, and tune back to it in `setup ()` with
   `radio.tune ()`.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. The music itself changes
far too fast for the meter, so these readings are on the volume knob, where
the sketch gets the volume from.

!!! question "Predict"
    With the volume knob halfway round, A0 reads about 2.5 V. The sketch
    turns 0 to 1023 into 0 to 15. What volume does the radio get: 7, 7.5 or
    8?

<!-- measure -->

What the numbers tell you:

- **Halfway**, A0 reads about 2.5 V, which the Mega reads as about 512 out
  of 1023. `read (0, 15)` turns that into 512 × 15 ÷ 1023 = 7.5, and a whole
  number drops the half: volume 7. The radio has no half volumes.
- **A quarter of the way**, about 1.25 V, is about 256, and volume 3. Every
  third of a volt on A0 is one more step of volume, so turning the knob
  from end to end walks through all sixteen.
