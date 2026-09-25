---
lesson: 24
title: Room Alarm
arc: Invisible signals
promise: Guard your room with an alarm you arm from the remote, which gives intruders ten seconds before the siren.
time: 2 hours
level: 3
sketch: Lesson24RoomAlarm
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - LCD1602 character display
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - HC-SR501 PIR motion sensor
  - IR receiver module and the kit's remote
  - Active buzzer
  - RGB LED
  - 4 × 220 Ω resistors (red, red, black, black, brown)
  - 21 jumper wires
  - 6 female-to-male jumper wires
ideas:
  - A device as a set of states
  - Exit and entry delays
  - Keying in a code, digit by digit
  - Combining a sensor, a remote, a screen and sound
---

## What you'll build

<!-- closeup -->

A real burglar alarm for your room. Press POWER on the remote and the
screen says *Leave now...* and counts down from ten, beeping each second,
while you slip out. Then it says *ARMED* and the light glows a dim red.
When someone walks in, the PIR sensor sees them: the screen demands the
code and counts down again. Key in the right four digits on the remote and
it relaxes to *Disarmed*; get it wrong, or run out of time, and the siren
wails while the light flashes red and blue.

## The idea

**An alarm is a handful of states.** Like the reaction game in Lesson 3, the
alarm is always in exactly one state, and each state knows what it's
waiting for. Some events only matter in one state: movement means nothing
while the alarm is disarmed, and POWER does nothing once it's armed.

**Two delays.** Real alarms are kind to their owners. The **exit delay**
gives you time to leave after arming it, and the **entry delay** gives you
time to reach the keypad and key in the code when you come back. Only then
does the siren sound. Here both are ten seconds, counted down on the
screen and beeped out loud.

**A code, digit by digit.** The remote's number buttons key in the code.
The sketch builds the number up the way you'd write it: each new digit
multiplies what's there by ten and adds itself.

<p class="formula">1 → 12 → 123 → 1234</p>

After the fourth digit it compares the number with the secret one, `Code`
in the sketch. Right, and the alarm disarms; wrong, and it gives a long beep
and waits for four digits again. Each digit shows as a star, so someone
looking over your shoulder learns nothing.

**The contrast, without a knob.** In Lesson 13 a knob set the LCD's
contrast by choosing the voltage on its V0 pin. Here a fixed 1 kΩ resistor
from V0 to GND does the job and saves space: on most LCD1602 modules it
gives crisp, dark text. If your screen shows only faint text, or a row of
solid blocks, use the knob from Lesson 13 instead.

!!! question "Predict"
    You arm the alarm and walk out, but you're still in front of the PIR
    sensor when the countdown reaches zero. What will the alarm do next?

## How the alarm works

| State | Light | Screen | What it waits for |
|---|---|---|---|
| **Disarmed** | green | Disarmed | POWER starts the exit delay. |
| **Leaving** | yellow | Leave now... and a countdown | A beep every second; at zero it is Armed. |
| **Armed** | dim red | ARMED | Movement starts the entry delay. |
| **Entering** | orange | Code, quick! and a countdown | A beep every second; at zero the siren sounds. |
| **Sounding** | flashing red and blue | ALARM! | The siren wails until the code is keyed in. |

In every state but Disarmed, the right code disarms the alarm at once, even
while you're leaving.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring. This is the biggest
    build so far: take it a step at a time, and check each step against the
    picture before moving on. Match the IR receiver's and the PIR's pins by
    their printed names (the PIR's are under its dome). The buzzer's longer
    leg, under the **+** on its top, goes in the top row, f40.

<!-- bench -->

<!-- steps -->

??? info "What the short wires beside the LCD do"
    The LCD stands in row a, so the rail below it is out of reach there.
    Its power arrives from the left instead. Column 6 is a small GND
    column, joined to the − rail; from it, one black wire takes GND to VSS
    (column 10), another black wire joins RW (column 14) to VSS so the
    display is only ever written, and the 1 kΩ resistor runs from it to V0
    (column 12). VDD (column 11) takes 5 V straight from the Mega. The
    backlight takes 5 V from the top rail through its 220 Ω resistor, and its
    K pin takes GND from the top − rail.

    Everything else joins the rails too, and the long black wire at the far
    end ties the top − rail to the bottom one, so every part shares GND
    with the Mega.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson24RoomAlarm**:

<!-- sketch -->

Read it from the top:

- Seven parts: the screen, the PIR sensor (active high, as in Lesson 23),
  the receiver, the siren, the status light, and two beats: `second` for the
  countdowns and `wail` for the siren.
- `Code` and `Delay` are yours to change. `digitButtons` lists the remote's
  0 to 9 buttons in order, so a button's place in the list is its digit.
- `enum State` names the five states, and `state` holds the one the alarm is
  in now.
- `loop ()` passes each fresh button press to `pressed ()`, then checks the
  one thing the current state is waiting for.
- `pressed ()` arms a disarmed alarm on POWER, and turns number buttons into
  digits for `keyIn ()`, which adds a star and checks the fourth digit.
- `countedDown ()` does one second's work on each tick of `second`: counts
  down, beeps and shows the number, and says when it has reached zero.
- `startCountdown ()` and `enter ()` change state: the light's color, the
  message and a fresh code line. `second.restart ()` makes the first tick
  come a whole second later.

## Upload it

1. Upload the sketch. The screen says *Disarmed* and the light glows green.
   If the screen is blank or shows blocks, see the table below.
2. Keep out of the PIR's view for a minute while it settles.
3. Press POWER on the remote. The light turns yellow and the screen counts
   down from 10, beeping every second. Leave its view.
4. At zero the screen says *ARMED* and the light glows dim red.
5. Walk back in. The light turns orange, *Code, quick!* appears and the
   countdown beeps again. Key in 1, 2, 3, 4 on the remote: four stars, and
   the alarm is *Disarmed*.
6. Now let the countdown run out: the siren wails and the light flashes red
   and blue until you key in the code.

## If it doesn't work

| What you see | Try this |
|---|---|
| The screen is lit but blank | The contrast is too faint: put the knob from Lesson 13 in place of the 1 kΩ resistor, or try a 330 Ω one. |
| The top row is solid blocks | The screen has power but isn't hearing the Mega: check pins 31 to 36 land in columns 13, 15 and 20 to 23. |
| It arms and straight away asks for the code | The PIR still saw movement when the countdown ended; it stays on for a few seconds after the last movement. Leave sooner, or make `Delay` longer. |
| It never notices you | Give the PIR a minute after power-up, check its OUT pin goes to A12, and turn its time knob fully anticlockwise. |
| The remote does nothing | Aim at the receiver's window. Upload Lesson 22's sketch to check your remote's codes, and change `digitButtons` if yours differ. |
| No beeps | The buzzer's + leg goes in f40, under pin 12's wire in j40, and its other leg's column needs the wire from a40 to the − rail. |
| The **L** LED blinks long and short flashes | ADK found a problem with a pin. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    Everything here happens without waiting. The receiver decodes codes in
    the background from its interrupt, the PIR switch and the two
    `adk::Every` beats are checked on every pass of `loop ()`, and each
    `siren.beep ()` switches itself off. So the alarm can count down, listen
    to the remote and watch for movement all at the same time.

    Writing to the LCD takes a little time: about 0.1 ms a character, and
    2 ms to clear it. That's why the sketch clears the screen only when the
    state changes, and otherwise just rewrites the few characters that
    change, such as the countdown in the top right corner.

## Make it yours

1. **Your own code.** Change `Code` to four digits of your own, and `Delay`
   to suit how far you have to walk.
2. **Three strikes.** Count wrong codes, and after three go straight to the
   siren, even during the entry delay.
3. **Silent alarm.** Add a mode, chosen with the EQ button, where the alarm
   only flashes the light and writes *INTRUDER* on the screen, without a
   sound.
4. **More tripwires.** Add the obstacle module or the beam-break sensor from
   Lesson 23 across your doorway, so that opening the door starts the entry
   delay too.
