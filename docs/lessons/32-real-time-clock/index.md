---
lesson: 32
promise: Build a clock that keeps the right time, even while it's unplugged.
time: 45 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - LCD1602 display
  - 10 kΩ potentiometer
  - 220 Ω resistor (red, red, black, black, brown)
  - DS1307 clock module with its coin cell
  - 4 female-to-male jumper wires
  - 17 jumper wires
ideas:
  - A clock chip that runs on a battery
  - Counting seconds with a quartz crystal
  - The I2C bus again, for a second chip
  - Setting the clock from the moment you compiled
---

## What you'll build

<!-- closeup -->

The LCD shows today's date and the time, down to the second. Unplug the Mega,
go and have lunch, and plug it back in: the clock is still right. A little
coin cell on the clock module kept it counting while everything else was
switched off.

## The idea

The Mega can count milliseconds with `millis ()`, but it forgets everything the
moment it loses power, and it starts again from zero. A **real-time clock**
is a chip that does one job: it counts seconds, minutes, hours, days, months
and years, with its own **coin cell** to keep it going when the Mega is off.
A cell like the CR2032 keeps a DS1307 running for years.

The clock counts vibrations of a tiny **quartz crystal**, the silver can on the
module. It vibrates exactly 32,768 times a second, and 32,768 is 2 multiplied
by itself 15 times. So the chip halves the count 15 times and gets exactly one
tick a second. A crystal like this can be about 20 millionths fast or slow,
and there are 86,400 seconds in a day:

<p class="formula">86,400 × <span class="fraction"><span>20</span><span>1,000,000</span></span> ≈ 1.7 seconds a day</p>

That's less than a minute a month.

The clock module talks over **I2C**, the same two wires, SDA on pin 20 and
SCL on pin 21, that the accelerometer used in Lesson 28. Every chip on the bus
has an address, and the clock answers to 0x68. So does the accelerometer,
so the two can only share the bus if one of them moves to another address.

A brand-new clock chip sits **stopped**, with no idea what time it is, until
something sets it. The sketch sets it once, to the moment you compiled the
sketch: the Arduino IDE writes that date and time into your program when it
builds it.

!!! question "Predict"
    Once the clock is running, unplug the USB cable for a minute, then plug it
    back in. Will the screen show the time you unplugged it, the time you
    compiled the sketch, or the right time? Write down your guess.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring, and check your work
    before you plug it back in.

!!! danger "Check your module before fitting a cell"
    Many clock modules just draw on their coin cell. A few also *charge* it
    from the 5 V supply: the Tiny RTC, and DS3231 boards sold as ZS-042. If
    yours has one of those names, fit a rechargeable LIR2032, never a CR2032,
    which isn't made to be charged and can swell or leak. If you can't tell,
    ask a grown-up to check the module's description before you fit a cell.

<!-- bench -->

<!-- steps -->

The clock module lies on its side above the board, clear of the LCD's signal
wires, so its GND and VCC wires drop straight into the top rails. Its SDA and
SCL wires come over the top from pins 20 and 21.

??? info "How the LCD is wired"
    The LCD is the one from Lesson 13, lying off the bottom edge of the board
    so its screen reads the right way up. That puts it over the bottom rails,
    so its power comes from the top rails, carried down by the red and black
    wires in columns 9, 10 and 13. The knob's outer legs connect to the LCD's
    GND and 5 V pins, and its middle leg sets the contrast on pin 3, V0.
    The backlight, pins 15 and 16, gets 5 V through the 220 Ω resistor that
    bridges the middle gap.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson32RealTimeClock**:

<!-- sketch -->

What's new:

- `adk::Rtc rtc;` is the clock module. It needs no pin numbers: I2C
  always uses pins 20 and 21.
- `rtc.isRunning ()` is false for a clock that has never been set, or whose
  cell went flat. Only then does the sketch set it, with
  `rtc.set (adk::compiledAt ())`. A clock that is already running is left
  alone, so resetting the Mega never throws away the right time.
- `adk::Every tick {200};` from Lesson 11 reads the clock five times a
  second, so the seconds on the screen change within a fifth of a second of
  the real ones.
- `auto now = rtc.now ();` asks the chip for the date and time, and keeps
  them in `now`, an `adk::DateTime`: `now.year`, `now.month`, `now.day`,
  `now.hour`, `now.minute` and `now.second`, on the 24-hour clock.
- `rtc.ok ()` says whether the chip answered just then. If it didn't, the
  screen says so, and which pins to check.
- `showDateAndTime ()` prints each row with one `adk::print` at
  `lcd.at (0, 0)` or `lcd.at (0, 1)`, as in Lesson 13. `now.minute / 10` is
  the tens digit and `now.minute % 10` the ones, the trick from Lesson 10,
  so nine minutes past eight shows as `20:09`, not `20:9`.

## Upload it

Upload the sketch and look at the LCD. If you see a row of blocks or
nothing, turn the contrast knob until the text is sharp. The top row shows
the date, the bottom row the time, and the seconds tick.

The clock will be a few seconds slow: the time it took to upload after
compiling. Now test your prediction: unplug the USB cable for a minute, and
plug it back in.

You predicted what the screen would show. It shows the right time. While
the Mega was off, the coin cell kept the clock chip counting, and when the
sketch started again, `rtc.isRunning ()` was true, so it left the time
alone.

## If it doesn't work

| What you see | Try this |
|---|---|
| The screen says **No clock found!** | Check SDA goes to pin 20 and SCL to pin 21, not the other way round, and that the module's VCC and GND are wired. |
| The time is wrong but ticking | Your module was already running with some other time, so the sketch left it alone. Delete the `if` line and its braces around `rtc.set`, upload, then put them back and upload again. |
| The time is right until you unplug, then starts again from the upload time | The coin cell is missing or flat. |
| The date and time read `2000-01-01` and don't move, or nonsense | The chip isn't running: check its cell is fitted, with + facing up. |
| A row of solid blocks, or a blank lit screen | Turn the contrast knob. If nothing changes, check the LCD's wires against the connections above. |
| The little **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    Inside the DS1307 are seven numbers, one each for seconds, minutes, hours,
    the day of the week, the date, the month and the year, stored the way we
    write them: each decimal digit in four bits of its own. `rtc.now ()` reads
    all seven in one go over I2C, which takes about a millisecond, and turns
    them back into ordinary numbers.

    `adk::compiledAt ()` reads two pieces of text the compiler writes into
    every program, `__DATE__` and `__TIME__`, such as `"Sep 24 2026"` and
    `"20:30:05"`, and turns them into an `adk::DateTime`.

## Make it yours

1. **Month names.** Show the date as `24 Sep 2026`. Make an `adk::Array` of
   the twelve names, each a `const char*`, and pick one with
   `now.month - 1`.
2. **Twelve hours.** Show the time on the 12-hour clock with `am` or `pm`.
   What should midnight and noon show?
3. **Good morning.** Use the bottom row for a greeting that changes with
   `now.hour`: good morning, good afternoon, good evening, and time for bed.
4. **How good is it?** Check the clock against a phone every day for a week.
   How many seconds does it gain or lose a day? Use the working above, the
   other way round, to find how many millionths fast or slow your crystal is.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM**, the red one in **V**. The clock module has two
supplies: the top rails, which it shares with the screen, and its own coin
cell. The rails' + and − holes are only 2.5 mm apart: push each probe tip
into its own hole, so neither slips across to the other rail.

!!! question "Predict"
    The top rails give the clock chip 5 V. Will its coin cell read more than
    that, less, or the same?

<!-- measure -->

Then the coin cell, which has no hole of its own: keep the black probe in
the top − rail, where the clock's GND is, and touch the red probe to the
cell's flat top, the side marked **+**, and nothing else.

What the numbers tell you:

- **The clock's supply** is the Mega's 5 V, carried from the red wire in
  T+3 all along the top + rail to the clock's VCC wire in T+30, and to the
  screen.
- **The coin cell** gives less: about 3 V, or a little more from a
  rechargeable LIR2032. While the rails give more than the cell, the
  DS1307 runs from them and leaves the cell alone. When the
  5 V goes, as it did in your unplug test, the chip switches over to the
  cell by itself and just keeps counting, which takes so little current
  that a cell lasts for years.
