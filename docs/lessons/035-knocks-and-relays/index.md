---
lesson: 35
promise: Knock a secret rhythm on the table and make a relay click a lamp on.
time: 60 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Tap sensor module (37 in 1)
  - Relay module (37 in 1)
  - 9 V battery and its snap lead
  - Red LED
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 6 female-to-male jumper wires
  - 1 jumper wire
  - A small screwdriver for the relay's terminals
ideas:
  - Timing a pattern of knocks
  - A rhythm written as letters, and checked letter by letter
  - Relays, and circuits that are separate
  - Never mains
---

## What you'll build

<!-- closeup -->

Knock on the table twice, pause, and knock twice more. With each knock, the
Mega's little **L** LED blinks: it heard you. Get the rhythm right and the
relay clicks, and a red lamp on the breadboard lights up, powered by a 9 V
battery the Mega never touches. Knock the secret again and the lamp goes
out. Get it wrong and the L LED flickers: try again.

## The idea

The **tap sensor** is a tiny spring inside a metal tube. A knock nearby makes
the spring shake and touch the tube for a few milliseconds. On most tap
modules, the S pin drops to 0 V while it touches: it's active low, like the
sensors in Lesson 23. (If yours is the other way round, the table below
says what to change.) Each touch is far too short for a button's 20 ms
debounce, so this sketch reads the sensor with no debouncing, counts the
first touch as the knock, and ignores the next 80 ms while the spring
rattles.

A secret knock is a **rhythm**: what matters is the gaps between knocks. The
sketch times each gap with a stopwatch and writes it as a letter: **S** for
a short gap, under 400 ms, and **L** for a long one. Knocks at 0, 250, 900 and
1150 ms leave gaps of 250, 650 and 250 ms:

<p class="formula">250 → S, 650 → L, 250 → S: "SLS"</p>

When 1.5 seconds pass with no knock, the rhythm is over, and the sketch
compares its letters with the secret, `"SLS"`: the same number of letters,
and the same letter in each place.

The second idea is the **relay**: a switch worked by an electromagnet. When
pin 11 goes high, a coil inside the blue box pulls a metal contact across with
a click. The contact is a real switch, with no electrical connection to the
coil, so it can switch a completely **separate** circuit. Here that's a 9 V
battery, a 1 kΩ resistor and a red LED. The LED keeps about 2 V, so:

<p class="formula">current = <span class="fraction"><span>9 V − 2 V</span><span>1000 Ω</span></span> = 7 mA</p>

!!! question "Predict"
    If you knock the secret rhythm twice as slowly, will the lamp still
    switch? Work out the gaps and the letters before you try it.

## Build it

!!! warning "Unplug first"
    Always unplug the USB cable, and unclip the 9 V battery, before you change
    any wiring. Never let the battery's two terminals touch each other or
    anything metal.

Nothing from Lesson 34 stays on the breadboard, but keep its RFID reader,
with its card and fob, somewhere safe: it comes back in Lesson 36, wired
just as it was, to join this lesson's tap sensor.

!!! danger "Never mains electricity"
    The relay's label says it can switch 250 V. That is exactly why it is
    only ever used here with a battery and an LED. Never connect it to
    anything that plugs into a wall socket: mains wiring can kill.

<!-- bench -->

<!-- steps -->

To fit a wire into one of the relay's screw terminals, loosen the screw a few
turns, push the wire's end into the square hole beneath it, and tighten the
screw until the wire can't be pulled out.

??? info "Two circuits on one breadboard"
    The lamp's circuit goes from the battery's red lead into the relay's COM
    terminal, out of NO, through the resistor and the LED, and back to the
    battery's black lead. It's laid out like the LEDs you know, with the
    resistor across the middle gap and the LED below it, but the LED's short
    leg goes back to the battery, not to the − rail. Nothing in it touches
    the Mega, its pins or the rails. The relay's contacts join COM to **NO**
    (normally open) only while the relay is on; when it's off, COM joins
    **NC** (normally closed) instead. That's why the lamp's wire goes to NO:
    the lamp is off until the relay is told to switch.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 035-knocks-and-relays**:

<!-- sketch -->

What's new:

- `adk::Switch tap {A12, adk::ActiveLow, 0};` is the tap sensor, as the
  on/off sensors were in Lesson 23. The last number is the debounce time in
  milliseconds: none at all.
- `adk::Led knockLight {LED_BUILTIN};` is the Mega's own **L** LED on pin 13,
  which needs no wiring.
- `adk::Relay relay {11};` is the relay module. `relay.toggle ()` switches it
  to whichever it isn't: on or off.
- `char secret [] = "SLS"` is the secret, as text: a row of letters. The
  times beside it are `adk::Millis`, ADK's type for a time in milliseconds.
- `rhythm` is an `adk::Text` of up to 16 letters, like Snake's message in
  Lesson 27. It starts empty, `rhythm.print ()` adds the letter for each
  gap, and `rhythm.clear ()` empties it for the next try.
- `sinceKnock` is a stopwatch, as in Lesson 3, that starts again at every
  knock. Its `elapsed ()` is the gap so far: a new touch sooner than `rattle`
  is only the spring still shaking, and for the first 100 ms it keeps the
  **L** LED lit. `setup ()` starts it, so the very first knock isn't taken
  for a rattle.
- `quiet` is a timer that `hearKnock ()` sets going for 1.5 seconds at
  every knock. While it runs, a rhythm is under way, so each knock adds a
  letter; the first knock of a rhythm has no gap before it, and adds none.
  When it runs out, `quiet.expired ()`, the knocking has stopped, and
  `judgeRhythm ()` decides.
- `judgeRhythm ()` prints what it heard, then switches the relay if
  `rhythm == secret`: the same letters, in the same order, and no more.
  After switching, it waits half a second, so the relay's own click, which
  shakes the table, isn't heard as a knock.

## Upload it

Upload the sketch, then clip the battery back on. If you like, open the Serial
Monitor at 9600 baud to see what the sketch hears.

Knock on the table right beside the tap sensor, or tap the sensor itself with
a finger. The **L** LED blinks at each knock. Now knock the secret: two quick
knocks, a pause of about a second, two quick knocks. After a moment the relay
clicks, its own little LED lights, and the red lamp comes on. Knock the
secret again to switch it off. A wrong rhythm makes the L LED flicker for a
second, and the Serial Monitor shows what it heard, such as `Heard SSS`.

You predicted what happens if you knock the secret twice as slowly. Knocks at
0, 500, 1800 and 2300 ms leave gaps of 500, 1300 and 500 ms. All three are
400 ms or more, so the sketch hears `LLL`, not `SLS`, and the lamp stays as
it was. The sketch only knows short from long by the clock, not by how the
gaps compare with each other. The second challenge below fixes that.

## If it doesn't work

| What you see | Try this |
|---|---|
| The L LED never blinks | Check S goes to A12, + to 5V and − to GND. Tap the sensor itself, not just the table. |
| The L LED blinks on its own, or stays lit | Your tap sensor may be active high: change `adk::ActiveLow` to `adk::ActiveHigh`. |
| You hear the right rhythm but the sketch doesn't | Watch the Serial Monitor. Extra `S`s mean one knock counted twice: raise `rattle` to 150. An `L` where you meant `S`: knock faster, or raise `longGap`. |
| The relay clicks but the lamp stays dark | Check the battery's red lead is tight in COM, the wire from NO goes to j13, the LED's long leg is in b13, and the battery's black lead is in a14. Is the battery flat? |
| The lamp is on while the relay is off | The wire is in NC. Move it to NO. |
| The relay never clicks | Check S goes to pin 11, + to the inner 5V pin at the top of the long header, and − to the inner GND pin at its other end. |
| The relay switches on at start and off when told on | Your relay module is active low: write `adk::Relay relay {11, adk::ActiveLow};`. |

??? note "How it works"
    `adk::Relay` only switches pin 11 high or low. The relay module has a
    transistor that switches the coil's current, about 70 mA, from its +
    pin, so the Mega's pin only has to give a few milliamps. A diode across the
    coil soaks up the kick of voltage the coil makes when it's switched off,
    which would otherwise damage the transistor.

    `adk::stop ()` turns the relay off along with everything else, so any
    safe state switches the lamp off too.

## Make it yours

1. **Your own knock.** Change `secret`. "Shave and a haircut, two bits" is
   seven knocks, and knocked briskly its gaps are `"SSSSLS"`. Knock it more
   slowly and some short gaps pass 400 ms and turn into `L`s, so pick your
   speed and check on the Serial Monitor what the sketch hears.
2. **Any speed.** Make the rhythm work fast or slow: instead of 400 ms, call a
   gap long when it's more than one and a half times the first gap.
3. **Three strikes.** After three wrong rhythms, ignore all knocks for 30
   seconds, like the lockout in the Keypad Safe of Lesson 18.
4. **A timer switch.** Make the lamp switch itself off one minute after the
   secret knock turned it on, with an `adk::Timer` like `quiet`.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM**, the red one in **V**, never the **10A** socket.
Knock the secret to switch the lamp on: it stays on until you knock it
again, so there's nothing to slow down. All three readings are in the
lamp's own circuit, which only ever carries the battery's 9 V.

!!! question "Predict"
    With the lamp on, the battery's 9 V is shared between the resistor and
    the LED. How much will each get? And what will the first reading show
    once the relay switches the lamp off?

<!-- measure -->

What the numbers tell you:

- **The battery, through the relay**: about 9 V; a new battery reads a
  little more, a tired one less. Knock the secret again and the relay
  clicks off: the reading drops to 0. The relay's contact is the only thing
  joining the battery to the lamp, and nothing joins either of them to the
  Mega.
- **Across the resistor** and **across the LED** add up to the first
  reading. The red LED keeps about 2 V for itself, and the resistor takes
  the rest, about 7 V: 7 V ÷ 1000 Ω is the 7 mA worked out in the idea
  above.
