---
lesson: 33
title: Alarm Clock
arc: Time
promise: Build a bedside clock that wakes you with a tune, with a knob to set it and a snooze button.
time: 90 minutes
level: 3
sketch: Lesson33AlarmClock
parts:
  - Your clock from Lesson 32, still built
  - Rotary encoder module
  - Push button
  - Passive buzzer
  - 220 Ω resistor (red, red, black, black, brown)
  - 5 female-to-male jumper wires
  - 4 jumper wires
ideas:
  - A device as a set of states
  - Times of day as one number
  - Setting a value with a knob
  - Playing a tune while the clock keeps going
---

## What you'll build

<!-- closeup -->

Your clock from Lesson 32 becomes an alarm clock. The top row shows the time;
the bottom row shows when the alarm will ring. Press the knob and turn it to
choose the hour, press and turn again for the minutes, and press once more.
When the moment comes, a wake-up tune plays and **Wake up!** flashes on the
screen, until you press snooze for five more minutes, or press the knob to
stop it until tomorrow.

## The idea

An alarm clock has to compare times: is it seven o'clock yet? That's easy when
a time is one number instead of two, so the sketch counts **minutes after
midnight**:

<p class="formula">07:30 → 7 × 60 + 30 = 450</p>

A day has 24 × 60 = 1440 minutes, so times run from 0, midnight, to 1439,
one minute to midnight. The alarm rings when the clock's minute becomes the
alarm's number.

Snooze adds five minutes, but what if that runs past midnight? The sketch uses
`%`, which gives the **remainder** after dividing. Snooze at 23:58, minute
1438:

<p class="formula">(1438 + 5) % 1440 = 1443 − 1440 = 3</p>

Minute 3 is 00:03, three minutes past midnight, just as it should be.

The other idea is the one from the Reaction Duel in Lesson 3: a device that
behaves differently depending on what it's doing, its **state**. The same
button press means "set the hour" when the clock is showing the time, and
"stop" when the alarm is ringing.

!!! question "Predict"
    You set the alarm for 06:45, and at 06:45 it rings. You press snooze twice,
    each time as soon as it rings again. When does it ring the third time?
    What will the bottom row show while you wait?

## How the alarm clock works

The sketch is always in one of four states:

| State | The screen shows | The knob | The knob's button | Snooze |
|---|---|---|---|---|
| Showing | The time, and `Alarm 07:00` or `Snooze 07:05` | Nothing | Go to Hour? | Nothing |
| Hour? | The time, and `Hour?` with the alarm | Changes the hour | Go to Minute? | Nothing |
| Minute? | The time, and `Minute?` with the alarm | Changes the minutes | Back to Showing, with a beep | Nothing |
| Ringing | The time, and **Wake up!** flashing | Nothing | Stop until tomorrow | Five more minutes |

Four times a second the sketch reads the clock and updates the screen. When a
new minute begins and it matches the alarm (or the end of a snooze), the
clock goes from Showing to Ringing.

## Build it

!!! warning "Unplug first"
    Always unplug the USB cable before you change any wiring. The passive
    buzzer always goes through its 220 Ω resistor: its coil is only about
    16 Ω, and on its own it would draw far more than a pin should give.

Keep your clock from Lesson 32 just as it is. The new parts are the snooze
button in the first three columns, the passive buzzer and its resistor after
the LCD's pins, and the knob, on five wires. Everything is listed below, the
old parts too, so you can check them.

<!-- bench -->

<!-- steps -->

??? info "The knob's wires"
    The knob is the rotary encoder from Lesson 29. Its CLK and DT pins go to
    18 and 19, its push switch SW to pin 22, and it takes 5 V and GND from
    the top rails. If yours has its pins in a different order, go by the
    printed names, not their places.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → Lesson33AlarmClock**:

<!-- sketch -->

Read it from the top:

- The parts: the LCD and clock from Lesson 32, the knob (`knob` turns,
  `knobButton` is its push switch), the passive buzzer as an `adk::Speaker`,
  and the snooze button.
- `WakeUp` is a tune, written as notes the way you did in Lesson 5. It ends
  with a rest, so there's a pause each time it repeats.
- `enum State` lists the four states, and `state` says which one the clock is
  in now. `alarm` and `ringAt` are times in minutes after midnight: `ringAt`
  is the alarm itself, or the end of a snooze.
- `loop ()` checks the clock four times a second, then does what the current
  state asks: answer the alarm, move on to the next setting, or change the
  alarm when the knob turns.
- `checkTheClock ()` only starts the alarm as a **new** minute begins
  (`minute != lastMinute`). Without that, stopping the alarm at 07:00 would
  set it off again a quarter of a second later, because it would still be
  07:00.
- `nextSetting ()` moves to the next state in the list with
  `State (state + 1)`: from Showing to Hour? to Minute?, and then back to
  Showing.
- `changeAlarm ()` adds 60 minutes for each click while you set the hour, and
  1 minute while you set the minutes, and wraps round with `%`.
- `answerAlarm ()` starts the tune again whenever it finishes, and
  `speaker.play ()` never waits for it, so the clock keeps ticking on the
  screen and the buttons still work while it plays.

## Upload it

Upload the sketch. The top row shows the time; the bottom row shows
`Alarm   07:00`. Now set the alarm for two minutes from now:

1. Press the knob. The bottom row says `Hour?`. Turn the knob until the hour
   is right.
2. Press again: `Minute?`. Turn until the minutes are right.
3. Press once more. The buzzer beeps and the bottom row says `Alarm` again.

When the minute comes, the tune plays and **Wake up!** flashes. Press snooze:
the tune stops and the bottom row shows `Snooze` with a time five minutes
later. When it rings again, press the knob, and it stops until tomorrow.

## If it doesn't work

| What you see | Try this |
|---|---|
| Turning the knob right makes the numbers go down | Swap the knob's CLK and DT wires, on pins 18 and 19. |
| One click of the knob moves two minutes, or two clicks move one | Your encoder steps differently: give it a third number, as in `adk::RotaryEncoder knob {18, 19, 2};`, and try 2 or 1. |
| Pressing the knob does nothing | Check SW goes to pin 22. The knob's + and GND must be wired too. |
| The alarm never rings | It only rings as a new minute begins, so set it at least a minute ahead. Check the bottom row says `Alarm` and not `Hour?` or `Minute?`. |
| The screen flashes **Wake up!** but there's no sound | Check the buzzer's + leg, the longer one, is in g32 next to the resistor, and the resistor goes from i28 to i32. |
| The snooze button does nothing | It must straddle the middle gap in columns 1 and 3, with pin 23's wire in j1 and the black wire from j3 to the − rail. |
| The time is wrong | See Lesson 32: the clock module keeps whatever time it was set to. |

??? note "How it works"
    `adk::Speaker` plays each note with the Mega's Timer 2, which makes the
    buzzer's square wave by itself; `adk::update ()` only has to start the
    next note when one ends. That's why a Speaker stops pins 9 and 10 from
    dimming LEDs, as you found in Lesson 5.

    Rewriting both rows of the LCD takes about three milliseconds, so the
    sketch only does it four times a second, or when something changes. The
    knob is read on every `adk::update ()`, so no click is missed while the
    screen is being written.

## Make it yours

1. **An off switch.** Make the snooze button switch the alarm on and off when
   it isn't ringing, and show `on` or `off` at the end of the bottom row.
2. **Your own tune.** Replace `WakeUp` with a tune of your own. Something
   gentle to start, perhaps, or something you really can't sleep through.
3. **Give up.** A clock left ringing all day is annoying. Make it stop by
   itself after two minutes. You'll need to remember when it started ringing.
4. **Big digits.** Show the time on the four-digit display from Lessons 11
   and 12 as well, with `adk::FourDigitDisplay` and
   `showTime (hour, minute)`. It won't fit alongside everything else on this
   breadboard, so you'll need to plan a new layout.
