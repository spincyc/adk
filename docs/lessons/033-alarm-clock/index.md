---
lesson: 33
promise: Build a bedside clock that wakes you with a tune and a flag, with a knob to set it and a snooze button.
time: 1½ hours
level: 3
parts:
  - Your clock from Lesson 32, still built
  - Rotary encoder module
  - Push button
  - Passive buzzer
  - 220 Ω resistor (red, red, black, black, brown)
  - 28BYJ-48 stepper and ULN2003 driver, from Lesson 31
  - Breadboard power module and its 9 V adapter
  - 8 more female-to-male jumper wires
  - 8 more jumper wires
  - A paper flag and sticky tape
ideas:
  - A device as a set of states
  - Times of day as one number
  - Setting a value with a knob
  - Playing a tune and moving a flag while the clock keeps going
---

## What you'll build

<!-- closeup -->

Your clock from Lesson 32 becomes an alarm clock. The top row shows the time;
the bottom row shows when the alarm will ring. Press the knob and turn it to
choose the hour, press and turn again for the minutes, and press once more.
When the moment comes, a paper flag on Lesson 31's stepper motor swings up,
a wake-up tune plays and **Wake up!** flashes on the screen, until you press
snooze for five more minutes, or press the knob to stop it until tomorrow.
Either way, the flag lies down again.

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

The flag shows the state for anyone across the room: it stands up while the
clock is ringing, and lies flat the rest of the time. Because the stepper
counts its steps, as you saw in Lesson 31, the sketch never has to look at
the flag. It only says where the flag should be, and the motor goes there.

!!! question "Predict"
    You set the alarm for 06:45, and at 06:45 it rings. You press snooze twice,
    each time as soon as it rings again. When does it ring the third time?
    What will the bottom row show while you wait?

    The flag takes about two seconds to stand up. If you press snooze one
    second after the alarm starts, what will the flag do?

## How the alarm clock works

The sketch is always in one of four states, and these are their names in
the code:

| State | The bottom row shows | The knob | The knob's button | Snooze |
|---|---|---|---|---|
| **Showing** | `Alarm 07:00`, or `Snooze 07:05` | Nothing | Go to SettingHour | Nothing |
| **SettingHour** | `Hour?` and the alarm | Changes the hour | Go to SettingMinute | Nothing |
| **SettingMinute** | `Minute?` and the alarm | Changes the minutes | Back to Showing | Nothing |
| **Ringing** | **Wake up!**, flashing | Nothing | Stop until tomorrow: Showing | Five more minutes: Showing |

The flag is up in `Ringing` and flat in the other three. The top row always
shows the time. Ten times a second the sketch checks the clock and
redraws the screen. When a new minute begins and it matches the alarm (or
the end of a snooze), the clock goes from `Showing` to `Ringing`.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable and switch the power module off before you change
    any wiring. The passive buzzer always goes through its 220 Ω resistor:
    its coil is only about 16 Ω, and on its own it would draw far more than
    a pin should give. The power module lies beside the board as in Lesson
    31, never plugged into it, both its jumpers **off**, its red wire from
    **5V** to the bottom + rail and its black wire from **GND** to the
    bottom − rail, both by column 42: the Mega's 5V already feeds the top
    rails, and two supplies must never feed the same rail. Never power the
    stepper's driver from the Mega's 5V pin.

Keep your clock from Lesson 32 just as it is. The new parts are the snooze
button in columns 8 to 10, the passive buzzer in column 33 with its
resistor down to the − rail, the rotary encoder in row a, columns 15 to
19, and the flag: the stepper's driver below the Mega, as in Lesson 31,
with the power module lying to the right of the board.

The power module feeds only the bottom rails, for the stepper's driver, as
in Lesson 31. The screen and the clock keep the Mega's 5 V on the top rails,
as in Lesson 32. The steps below say what to keep and what to add.

<!-- bench -->

<!-- steps -->

The driver's red wire goes into the bottom + rail by column 5, and its
black wire into the bottom − rail by column 6, just as in Lesson 31.

Push the motor's white plug into the driver's socket. Tape a paper flag to
the motor's shaft so that, looking at the end of the shaft, the flag lies
flat and points to the left. Do this before you switch on: the sketch counts
the flag's steps from wherever it is when the Mega starts.

??? info "The knob's wires"
    The knob is the rotary encoder from Lesson 29. Its CLK and DT pins go to
    18 and 19, its push switch SW to pin 22, and its power comes from the
    top rails: GND from e15 to the − rail by column 18, and + from e16 to
    the + rail by column 19. It stands in the same holes as in Lesson 29.
    If yours has its pins in a different order, go by the printed names,
    not their places.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 033-alarm-clock**:

<!-- sketch -->

Read it from the top:

- The parts: the LCD and clock from Lesson 32, the knob from Lesson 29
  (`knob` turns, `knobButton` is its push switch), the passive buzzer as an
  `adk::Speaker`, the snooze button, and the flag, Lesson 31's stepper on
  A8 to A11. The potentiometer beside the LCD is a knob too, but it only
  sets the contrast: on this page, *the knob* always means the rotary
  encoder.
- `wakeUp` is a tune, written as notes the way you did in Lesson 5. It ends
  with a rest, so there's a pause each time it repeats.
- `flagUp` is a quarter turn, 1024 half-steps, as in Lesson 31: from lying
  flat to standing up.
- `enum class State` names the four states from the table, as the Reaction
  Duel did in Lesson 3. `alarm` and `ringAt` are times in minutes after
  midnight: `ringAt` is the alarm itself, or the end of a snooze.
  `clockTime` is the clock's time when the sketch last read it.
- `loop ()` hands each press and each turn of the knob to a function of its
  own, lets snooze stop the alarm while it rings, and checks the clock ten
  times a second, when `tick` ticks.
- Its last line says where the flag should be, with the conditional from
  Lesson 12: `flagUp` while the state is `Ringing`, and 0, flat, in every
  other state. `moveTo ()` only sets where the motor is heading and never
  waits, so the tune, the knob and the screen carry on while the flag
  moves. Asked again for the place it is already heading to, it just
  carries on.
- `knobPressed ()` is the *knob's button* column of the table, as a
  `switch`. Finishing the minutes and stopping the alarm both call
  `sleepUntil (alarm)`.
- `knobTurned ()` moves the alarm 60 minutes for each click while you set
  the hour, and 1 while you set the minutes, with the `+=` from Lesson 21.
  In the other two states the knob does nothing: a bare `return;` leaves
  the function at once, as Lesson 6's `return` did, without handing
  anything back. The last line wraps the alarm round the day with `%`,
  adding a whole day first, as Lesson 30 added the count of mazes, so that
  turning back from 00:00 gives 23:00 rather than a time below zero.
- `sleepUntil ()` stops the tune, sets when to ring next, and goes back to
  `Showing`. Snooze calls it with five minutes from now; the knob's button
  calls it with the alarm, which comes round again tomorrow. So setting a
  new alarm also cancels a snooze.
- `readTheClock ()` takes the time from `rtc.now ()`, as in Lesson 32, and
  turns it into minutes after midnight. It starts ringing only as a
  **new** minute begins (`time != clockTime`). Without that, stopping the
  alarm at 07:00 would set it off again a tenth of a second later, because
  it would still be 07:00. `clockTime` starts at -1, which is no time at
  all, so the very first reading counts as a new minute. While it rings,
  it asks for the tune ten times a second: a tune that is already playing
  just carries on, and one that has ended starts again. `speaker.play ()`
  never waits, so the clock keeps ticking on the screen and the buttons
  still work while it plays.
- `showAlarm ()` fills the bottom row. `label` is a piece of text, a
  `const char*` as in Lesson 14. It starts as `Alarm` or `Snooze`, chosen
  with a `?:` from Lesson 12, and the `switch` changes it while you set the
  alarm. While the alarm rings, **Wake up!** shows when the seconds are
  even and a blank row when they're odd (`second % 2`), so it flashes in
  time with the clock; then `return` leaves before the label is printed.
- `printTime ()` turns minutes after midnight back into hours and minutes,
  425 into `07:05`, and prints each as two digits with the `/` and `%` of
  Lesson 10.

## Upload it

Switch the power module on, then plug in the Mega and upload the sketch.
The top row shows the time; the bottom row shows `Alarm 07:00`. Now set the
alarm for two minutes from now:

1. Press the knob. The bottom row says `Hour?`. Turn the knob until the hour
   is right.
2. Press again: `Minute?`. Turn until the minutes are right.
3. Press once more. The bottom row says `Alarm` again, with your time.

When the minute comes, the flag swings up, the tune plays and **Wake up!**
flashes. Press snooze: the tune stops, the flag lies down again, and the
bottom row shows `Snooze` with a time five minutes later. When it rings
again, press the knob, and it stops until tomorrow.

You predicted what happens with an alarm at 06:45 and two quick snoozes.
Each snooze is five minutes from the minute you press it, so it rings again
at 06:50 and a third time at 06:55. While you wait, the bottom row says
`Snooze 06:50`, and then `Snooze 06:55`.

You also predicted what the flag does if you snooze while it is still
rising. It turns straight back from wherever it has got to, about halfway,
and lies flat again about a second later. The moment the state changes,
`loop ()` asks for 0 instead of `flagUp`, and since the motor has counted
every step on the way up, it knows exactly how far back 0 is.

## If it doesn't work

| What you see | Try this |
|---|---|
| Turning the knob right makes the numbers go down | Swap the knob's CLK and DT wires, on pins 18 and 19. |
| One click of the knob moves two minutes, or two clicks move one | Your encoder steps differently: give it a third number, as in `adk::RotaryEncoder knob {18, 19, 2};`, and try 2 or 1. |
| Pressing the knob does nothing | Check SW goes to pin 22. The knob's + and GND must be wired too. |
| The alarm never rings | It only rings as a new minute begins, so set it at least a minute ahead. Check the bottom row says `Alarm` and not `Hour?` or `Minute?`. |
| The screen flashes **Wake up!** but there's no sound | Check the buzzer's + leg, the one by its + mark, is in f33, in pin 10's column, that pin 10's wire is in j33, and that the resistor goes from a33 to the − rail. |
| The snooze button does nothing | It must straddle the middle gap in columns 8 and 10, with pin 23's wire in j8 and the black wire from a10 to the − rail. |
| The time is wrong | See Lesson 32: the clock module keeps whatever time it was set to. |
| The time stands still, perhaps at 00:00:00 | Upload Lesson 32's sketch. It says **No clock found!** if the Mega can't hear the clock: check SDA goes to pin 20 and SCL to pin 21. If the clock had stopped, it starts it again. Then upload this sketch again. |
| The flag never moves, and the driver's LEDs stay dark | Switch the power module on and check its LED is lit, its red wire goes from 5V to the bottom + rail and its black one from GND to the bottom − rail, both by column 42, and the driver's red wire goes to the bottom + rail by column 5 and its black wire to the bottom − rail by column 6. |
| The flag hums or shakes but hardly turns | Check IN1 goes to A8, IN2 to A9, IN3 to A10 and IN4 to A11. |
| The flag swings down instead of up | Your motor turns the other way: tape the flag on pointing right instead. |
| The flag stands up while it's quiet, and lies flat when it rings | The Mega started while the flag was up, and counts from there. Unplug it, turn the flag flat by hand, and plug it in again. |
| A row of solid blocks, or a blank lit screen | Turn the contrast knob, the potentiometer beside the LCD, not the new knob. |

??? note "How it works"
    `adk::Speaker` plays each note with the Mega's Timer 2, which makes the
    buzzer's square wave by itself; `adk::update ()` only has to start the
    next note when one ends. That's why a Speaker stops pins 9 and 10 from
    dimming LEDs, as you found in Lesson 5.

    Rewriting both rows of the LCD takes about three milliseconds, so the
    sketch doesn't do it on every pass of `loop ()`, only when `tick` ticks,
    ten times a second. That's still quick enough that the numbers seem to
    follow the knob at once. The knob itself is read on every
    `adk::update ()`, so no click is missed while the screen is being
    written.

    The flag takes a half-step every two milliseconds, so while the screen
    is being written it misses a step's moment or two, and rises a little
    slower than 500 half-steps a second. It never loses count: ADK counts
    the steps the motor takes, not the time that passes.

## Make it yours

1. **A beep.** Make the buzzer beep as you finish setting the alarm. In
   `knobPressed ()`, give the `SettingMinute` case two lines under its
   label, `sleepUntil (alarm);` and then
   `speaker.tone (adk::note::c6, 150);`, followed by `break;`.
2. **No clock?** Copy Lesson 32's check into `readTheClock ()`: if
   `rtc.ok ()` is false, show **No clock found!** and which pins to check,
   and `return` before anything else. The label row then needs three
   spaces after the time, to rub out the end of the message.
3. **An off switch.** Make the snooze button switch the alarm on and off when
   it isn't ringing: a `bool` that `readTheClock ()` checks before it rings.
   Show `on` or `off` at the end of the bottom row.
4. **Your own tune.** Replace `wakeUp` with a tune of your own. Something
   gentle to start, perhaps, or something you really can't sleep through.
5. **Give up.** A clock left ringing all day is annoying. Make it stop by
   itself after two minutes: start an `adk::Timer` when it starts ringing,
   and call `sleepUntil (alarm)` when the timer runs out.
6. **Half up.** While a snooze is running, hold the flag halfway up,
   `flagUp / 2`, so you can see at a glance that the alarm will ring again
   soon. Which variable already tells the sketch that a snooze is running?
7. **Big digits.** Show the time on the four-digit display from Lessons 11
   and 12 as well, with `adk::FourDigitDisplay` and
   `showTime (hour, minute)`. It won't fit alongside everything else on this
   breadboard, so you'll need to plan a new layout.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM**, the red one in **V**, never the **10A** socket.
For the first reading, set the alarm a minute or two ahead and let it ring:
the tune goes on until you press snooze. Push each probe tip into its own
hole: the rails' + and − holes are only 2.5 mm apart, and a tip that slips
across would join them.

!!! question "Predict"
    While a note plays, pin 10 switches between 5 V and 0 V hundreds of
    times a second, spending half its time at each. A meter on DC volts
    shows the average. What will it read?

<!-- measure -->

What the numbers tell you:

- **The buzzer's pin** is high half the time and low half the time, so the
  meter shows about half of 5 V. It's a little under 2.5 V, because the pin
  sags a little below 5 V while it pushes current through the buzzer. The
  number jumps about as the notes come and go, and drops to 0 in the pause
  at the end of the tune. Press snooze and it stays at 0: the pin shows the
  state as plainly as the flag does.
- **The flag's supply** is the power module's 5 V on the bottom rails,
  while the screen and the clock run from the Mega's 5 V on the top rails.
  Watch the reading while the flag moves: it hardly changes, because the
  module has plenty to spare for the motor.

## Check yourself

1. Why does the sketch keep each time of day as one number, minutes after
   midnight, instead of an hour and a minute?
2. What would happen if `readTheClock ()` started ringing whenever `time`
   matched `ringAt`, without the `time != clockTime` part?
3. The sketch never looks at the flag. How does the flag still know to lie
   down when you press snooze?

??? note "Answers"
    1. One number is easy to compare and to add to: "is it time yet?" is
       just `time == ringAt`, and five more minutes is a plain addition,
       with `%` wrapping it round past midnight.
    2. Pressing the knob to stop it until tomorrow wouldn't work. A tenth
       of a second later it would still be the alarm's minute, so it would
       start ringing again, over and over until that minute was over.
    3. The last line of `loop ()` asks for `flagUp` only while the state is
       `Ringing`, and 0 in every other state. Snooze changes the state, and
       the stepper, which has counted every step, turns back to 0.
