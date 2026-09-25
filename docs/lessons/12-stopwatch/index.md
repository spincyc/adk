---
lesson: 12
title: Stopwatch
arc: Digits
promise: Build a stopwatch with laps, and a kitchen timer that beeps at zero.
time: 1½ hours
level: 3
sketch: Lesson12Stopwatch
parts:
  - Lesson 11's display circuit, built and working
  - 3 push buttons
  - Active buzzer (sealed, with a sticker on top)
  - 8 more jumper wires
ideas:
  - How a stopwatch keeps time, and ADK's Stopwatch and Timer
  - A device with states and modes
  - Start, stop, lap and reset on three buttons
  - Counting down, and a beep at zero
  - Splitting a time into seconds and tenths
---

## What you'll build

<!-- closeup -->

A stopwatch you can race with. Press start and the tenths of a second fly
past; press lap and the display freezes on your split time while the clock
keeps running underneath; press stop, then reset. Press mode and it turns
into a kitchen timer: set it counting down from 10 seconds, a minute or
three, and when it reaches zero it says **donE** and beeps.

## The idea

Since the moment it was switched on, the Mega has been counting
milliseconds: that count is `millis ()`, which Lesson 3 met. A stopwatch
doesn't need its own clock. It only has to remember **when it started**,
and subtract: the time now minus the time it started is how long it has
been running.

Stopping is a little harder, because a stopped stopwatch must remember its
time and carry on from there. So a stopwatch keeps two numbers: the time
**banked** before its latest start, and the moment of that start. While it
runs, its time is the banked time plus the time since the start. Start at
5000, stop at 12300, and 7300 milliseconds are banked; start again at
20000, and at 21500 the stopwatch shows 7300 + 1500 = 8800 milliseconds,
which is 8.8 seconds.

That is exactly what `adk::Stopwatch`, from Lesson 3, does for you:
`start ()`, `stop ()` and `reset ()` are its buttons, and `elapsed ()`
reads it.

A kitchen timer is the same stopwatch read backwards: the time left is the
total minus the time counted. To catch the moment it reaches zero, the
sketch also sets an `adk::Timer`, Lesson 3's countdown, for the time left
whenever the kitchen timer starts. Its `expired ()` says when.

!!! question "Predict"
    You start the stopwatch, stop it after 3 seconds, wait 10 seconds, and
    start it again. What will it show 2 seconds later? Write down your
    answer, then try it.

## How the stopwatch works

The stopwatch is always in one of three **states**:

| State | The display shows | Start/stop (22) | Lap/reset (23) | Mode (24) |
|---|---|---|---|---|
| Stopped | the time so far, standing still | starts it | resets it | next mode |
| Running | the time, changing | stops it | freezes a lap for 3 seconds | ignored |
| Done | **donE**, after three beeps | resets it | resets it | next mode |

The mode button steps through four **modes**: the stopwatch, which starts
at 0.0 and counts up, and timers that start at 10.0, 60.0 and 180.0 seconds
and count down. Only a timer can reach Done. Every button press gives a
short click from the buzzer, so you know it registered.

A lap freezes the display on the time when you pressed it, while the clock
keeps counting behind it; after three seconds the display catches up.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you add anything. Leave Lesson 11's display
    circuit exactly as it is: the buttons and the buzzer go in beyond the
    display, at the far end of the board. The **active** buzzer goes straight
    to its pin, with its **+** leg, the longer one, in the top half. It draws
    up to about 30 mA, which a pin can give, as long as it has the pin to
    itself.

<!-- bench -->

The first steps are Lesson 11's; if your display works, start at the first
button.

<!-- steps -->

??? info "The buttons and the buzzer"
    Each button straddles the middle gap, as in Lesson 2: its left pair of
    legs is joined to its pin's wire, and pressing it joins them to the
    right pair, whose short black wire goes to GND. From left to right they
    are start/stop (22), lap/reset (23) and mode (24).

    Use the **active** buzzer, the sealed one with a sticker: it makes its
    own tone whenever pin 12 is high. The passive buzzer, with its green
    board, would only click. The buzzer stands across the middle gap, one
    leg on each side, so its + leg in f59 meets pin 12's wire and its other
    leg in e59 meets the black wire to GND.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → Lesson12Stopwatch**:

<!-- sketch -->

What's new:

- `modes` is an `adk::Array` of the four times the mode button steps
  through, in milliseconds. Their type is `adk::Millis`, ADK's type for
  times: an `unsigned long`, a `long` from Lesson 8 that can't be negative,
  so it counts to about 49 days. An `int` would run out after 32 seconds.
- `enum class State { Stopped, Running, Done };` names the three states,
  and `startOrStop ()` decides with a `switch`, both as in Lesson 3: the
  same button starts, pauses or resets, depending on the state.
- `adk::Stopwatch stopwatch;` counts the time in every mode. The sketch's
  `start ()` calls `stopwatch.start ()`, its `pause ()` calls
  `stopwatch.stop ()`, and its `reset ()` calls `stopwatch.reset ()`, which
  sets it back to zero.
- `adk::Timer alarm;` is set by `start ()` for `clockTime ()`, which for a
  kitchen timer is the time left. `pause ()` stops it too, so it can never
  ring while the clock stands still. `alarm.expired ()` is true for the one
  update in which it runs out, which is the moment for `finish ()`.
- `adk::Timer lapShown;` holds a lap on the display: `lapOrReset ()` keeps
  the time in `lap` and starts `lapShown` for 3000 ms, and `showClock ()`
  shows `lap` while `lapShown.isRunning ()`.
- `loop ()` checks the alarm, reads the three buttons, and redraws the
  display, every time round. Each press clicks the buzzer with
  `buzzer.beep (20)`. `(current + 1) % modes.size ()` steps to the next
  mode, and back to the first after the last.
- `clockTime ()` turns the stopwatch into what the display shows: its time
  for the stopwatch, or `total - time` for a kitchen timer.
  `time < total ? total - time : 0` uses Lesson 7's `?:`, so a timer can
  never show less than zero.
- `showTime ()` splits the time, as in Lesson 10. `ms / 100` is the time in
  whole tenths of a second, and `% 10000` makes the stopwatch start again
  from 0.0 after 999.9 seconds. Then `tenths / 10` is the seconds, and
  `tenths % 10` the tenths left over: 123 tenths are 12 seconds and 3
  tenths.
- `char text [6];` is a row of six characters to write into: five for the
  time, and one more to mark where the text ends. `snprintf ()` writes the
  time there as text, such as `" 12.3"`. In the
  pattern `"%3d.%d"`, `%3d` stands for a whole number at least three
  characters wide, padded with spaces, which the display leaves blank; `%d`
  stands for the tenths; and the `.` lights the dot after the seconds.
- `finish ()` uses `adk::wait ()` between beeps, so the display stays lit
  while the buzzer sounds.

## Upload it

Upload the sketch. The display shows `0.0`. Press start/stop (22): it clicks
and the tenths start ticking up. Press lap/reset (23): the display freezes
for three seconds, then jumps to the true time. Press start/stop again to
stop it, and lap/reset to set it back to `0.0`.

Now try your prediction: start, stop after 3 seconds, wait 10 seconds, and
start again. Two seconds later it shows `5.0`. Did you predict 15? The 10
seconds it stood stopped don't count: the stopwatch banked 3.0 seconds,
and carried on from there.

Now press mode (24): the display shows `10.0`. Press start and it counts
down; at zero it shows **donE** and the buzzer beeps three times. Press any
button to set it back to `10.0`, or mode for `60.0`, `180.0`, and back to
the stopwatch.

## If it doesn't work

| What you see | Try this |
|---|---|
| The display is dark or scrambled | Go back to Lesson 11: upload its sketch and fix the display first. |
| A button does nothing | Its wire from the Mega goes in row j of its left legs (j45, j49, j53) and its black wire from row a of its right legs to the − rail. Check it straddles the gap. |
| The buttons do the wrong jobs | The wires from pins 22, 23 and 24 are crossed: 22 goes to the first button. |
| No beep, but the timer shows donE | The buzzer's + leg, the longer one, goes in f59, with pin 12's wire in j59, and its black wire runs from a59 to the − rail. Make sure it's the active buzzer. |
| The buzzer never stops | Its + leg is getting 5 V: check that the wire in j59 comes from pin 12, not from a 5V pin. |
| The mode button does nothing | It only works while the clock is stopped. Stop it first. |

??? note "How it works"
    `millis ()` counts in a variable of 32 bits, so it runs for about 49
    days before it goes back to 0. The stopwatch and the timers always
    subtract, the time now minus the time they started, and subtraction of
    `unsigned long` numbers gives the right answer even across that wrap.

    `start ()` sets the stopwatch and the alarm going together, and both
    begin counting on the next `adk::update ()`, from the same moment. So
    the alarm rings in the very update in which a timer's time left
    reaches zero.

    Each button is an `adk::Button`, which debounces it and makes
    `wasPressed ()` true for exactly one turn of `loop ()`, so one press
    starts or stops the clock exactly once. The buzzer's clicks use
    `beep ()`, which switches it off again by itself inside `adk::update ()`.

## Make it yours

1. **Hundredths.** Show seconds and hundredths, from `0.00` to `99.99`. What
   do you need to change in `showTime ()`?
2. **Minutes.** Past 60 seconds, switch to minutes and seconds with
   `display.showTime (minutes, seconds)`. `/ 60` and `% 60` split a number
   of seconds into minutes and seconds, as `/ 10` and `% 10` split tenths.
3. **Set your own timer.** Let lap/reset add 10 seconds to the countdown
   while a timer is stopped, instead of choosing from fixed modes.
4. **Final countdown.** Make the buzzer click once a second during a timer's
   last five seconds, like the end of a quiz show.
