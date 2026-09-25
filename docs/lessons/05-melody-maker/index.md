---
lesson: 5
title: Melody Maker
arc: Color and sound
promise: Turn four buttons into a keyboard, and teach the Mega a tune.
time: 45 minutes
level: 2
sketch: Lesson05MelodyMaker
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 4 push buttons
  - Passive buzzer (the one with a green board showing underneath)
  - 220 Ω resistor (red, red, black, black, brown)
  - 10 jumper wires
ideas:
  - Sound is a vibration, and pitch is how fast it vibrates
  - The passive buzzer, and why it needs a resistor
  - Notes and melodies as lists of numbers
  - A list of keys, and one loop for them all
  - Playing a tune while the sketch carries on
  - Why a sounding buzzer stops pins 9 and 10 dimming
---

## What you'll build

<!-- closeup -->

A tiny keyboard with four keys: C, D, E and G. Plug it in and it plays you
the first line of *Mary Had a Little Lamb*. Then it's your turn: each key
sounds its note for as long as you hold it down. Those four notes are all
that tune needs, so you can play it back, and plenty of others too.

## The idea

Every sound is something **vibrating**, pushing the air back and forth. The
air carries those pushes to your ear. How many times a second it vibrates is
the **frequency**, measured in **hertz** (Hz), and your ear hears frequency as
**pitch**: faster is higher. Middle C is 262 Hz. The A above it, which
orchestras tune to, is 440 Hz. Double a frequency and you get the same note
one **octave** higher: the next C up is 523 Hz.

The **passive buzzer** is a little electromagnet. Under its thin metal disc is
a coil of wire; switch a current through the coil and it pulls on the disc.
Switch it on and off 262 times a second and the disc flexes 262 times a
second: middle C. The Mega does that switching for you with `tone ()`, and
it can make any pitch you ask for. (The active buzzer in
[Lesson 3](../03-reaction-duel/index.md) had its own switching circuit inside,
so it could only ever play its one note.)

That coil is only about 16 Ω. Straight from a pin, it would try to take
5 V ÷ 16 Ω, about 310 mA: fifteen times more than a pin should ever give.
So the buzzer gets a 220 Ω resistor in series, and the two add up:

<p class="formula">current = <span class="fraction"><span>5 V</span><span>220 Ω + 16 Ω</span></span> ≈ 21 mA</p>

That's at most, and only while the pin is HIGH, which is half of every
vibration. On average the pin gives about 10 mA. The buzzer is a little
quieter with the resistor, but still plenty loud.

A **melody** is a list of notes, each a pitch and a length. In ADK one note
is an `adk::Note`, such as `{adk::note::e4, 400}`: E above middle C, for
400 ms. A whole tune is a list of them, and `speaker.play (tune)` plays it
while your sketch carries on with other things.

!!! question "Predict"
    The G key plays 392 Hz. Hold it down for exactly one second. How many
    times does the buzzer's disc move back and forth? And which key makes
    it move the slowest?

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring. **Never leave out the
    buzzer's 220 Ω resistor**, here or in any lesson: without it, the buzzer
    would draw far more current than pin 10 can safely give. Use the passive
    buzzer, with the green board showing underneath. It has a **+** marked on
    top beside one leg: follow the mark, and put that leg in f34, on pin
    10's side of the gap.

<!-- bench -->

<!-- steps -->

??? info "The signal's path through the buzzer"
    Follow pin 10's wire into j34. The buzzer stands across the middle gap,
    its **+** leg in f34 and its other leg in e34: they are 0.3 inch apart,
    just as far as the gap is wide. Below it, the 220 Ω resistor runs from
    a34 down into the − rail.

    For an LED the resistor came first, between the pin and the LED. Here it
    comes after the buzzer, between it and GND, and that works just as well.
    In a single loop the same current flows all the way round, so a resistor
    anywhere in the loop limits it.

    The four buttons are wired just as in [Lesson 2](../02-buttons/index.md):
    signal in at the top left, GND out at the bottom right. The first, on
    pin 22, is Lesson 4's button, left where it was; the RGB LED and its
    resistors come out to make room for the other three.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson05MelodyMaker**:

<!-- sketch -->

What's new:

- `struct Key` bundles a button with the pitch it plays, as Lesson 3's
  `Player` bundled a button with a light.
- `uint16_t pitch` is a whole number from 0 to 65 535: room for any pitch
  you can hear. A `uint8_t`, which stops at 255, would be too small.
- `adk::Array keys {Key {22, adk::note::c4}, ...};` is an `adk::Array` of
  keys, four structs in a row. `Key {22, adk::note::c4}` fills one in: its
  button's pin, then its pitch.
- `adk::note::c4` is a pitch by name: C in octave 4, middle C, 262 Hz. An
  `s` means sharp, so `adk::note::cs4` is C♯.
- `adk::Speaker speaker {10};` is a passive buzzer on pin 10. (ADK calls it a
  speaker because a small speaker, wired the same way, works too.)
- `constexpr adk::Note tune [] = {...};` is the melody: a list of notes,
  each `{pitch, milliseconds}`. The `[]` makes it C++'s own kind of list,
  which counts the notes for you and is what `speaker.play ()` takes. The
  comments beside it are the words, one syllable per note, and the long
  notes, 800 ms, fall on *lamb*.
- `speaker.play (tune);` starts the tune and returns straight away. The
  speaker moves on to each next note inside `adk::update ()`, so the loop
  keeps checking the keys all the time the tune plays.
- `for (auto& key : keys)` is a **range-for**: it runs the lines inside
  once for each key in the list, in order, with `key` standing for that
  key. One set of lines looks after all four. `auto` lets the compiler work
  out that each one is a `Key`, and the `&` means `key` is the real key, not
  a copy, as in Lesson 3.
- `speaker.tone (key.pitch);` sounds a note until `speaker.stop ();`. The
  variable `sounding` remembers the pitch of the key that is playing, or
  `adk::note::rest` (0) for none, so letting go of a key only stops its own
  note, not one you've pressed since.

## Upload it

Upload the sketch. The buzzer plays *Mary Had a Little Lamb*: E D C D, E E E,
D D D, E G G. Notice that the three Es are three separate notes, not one long
one. Then press the keys, left to right: C, D, E and G, each a step higher.
Each note lasts exactly as long as you hold its key.

You predicted how often the disc moves. Holding G for a second moves it back
and forth 392 times, and C, at 262 Hz, is the slowest.

Now play the tune yourself. The keys are C, D, E, G from left to right, so the
first line is: third, second, first, second, third, third, third.

Press a key while the tune is still playing: the tune stops and your note
takes over. That's the sketch carrying on while the tune plays.

## If it doesn't work

| What you see | Try this |
|---|---|
| Silence, from the tune and the keys | Follow pin 10's path: j34, the buzzer's **+** leg in f34 and its other leg in e34, and the resistor from a34 down into the − rail. |
| Every key makes the same harsh buzz, or no sound at all | You may have the active buzzer. Unplug, and swap in the passive one, with the green board underneath. |
| The tune plays, but a key does nothing | Push that button firmly into the board, all four legs in, and check its black wire from row a to the − rail. |
| Two keys play the same note, or the wrong ones | The signal wires may be in the wrong holes: pins 22, 23, 24 and 25 go to j2, j8, j14 and j20. |
| The sound is very quiet | Check it's the 220 Ω resistor (red, red, black, black, brown), not a 1 kΩ one. |

??? note "How it works"
    `tone ()` hands the switching to **Timer 2**, a counter inside the
    Mega that flips pin 10 at exactly the right rate while your sketch gets
    on with other things. Timer 2 is also the timer that makes PWM on pins
    9 and 10, so while a Speaker is declared, those two pins can't dim
    anything. ADK checks for you. Add a PWM output on pin 9 below the
    speaker, and use `adk::setup (Serial)`:

    ```cpp
    adk::PwmOutput glow {9};
    ```

    Upload and open the Serial Monitor at 9600 baud (you'll need
    `Serial.begin (9600);` first in `setup ()`), and ADK refuses to start:

    ```text
    adk: pin 9 needs a timer that is already in use
    adk: a Speaker stops PWM on pins 9 and 10
    ```

    That's why the buzzer's home is pin 10: it could never have dimmed an
    LED anyway. Take the line out again.

    In a melody, each note sounds for the first seven-eighths of its length
    and is silent for the last eighth, so the three Es of *lit-tle lamb* stay
    three notes.

## Make it yours

1. **Encore.** Make the tune play again whenever nobody has touched a key
   for ten seconds. Add an `adk::Timer`, as in Lesson 3. Start it for
   10 000 ms in `setup ()` and each time a key goes down, and when it
   `expired ()`, play the tune and start it again.
2. **A new song.** Write another tune, a list like `tune`. *Twinkle, Twinkle,
   Little Star* starts C C G G A A G. You'll need `adk::note::a4` (440 Hz)
   for the A, and `adk::note::rest` makes a silence.
3. **Higher and lower.** Move all four keys up an octave, to `c5`, `d5`,
   `e5` and `g5`, by changing their pitches in `keys`. Then try other notes
   altogether: which sets make tunes you recognise?
4. **Echo.** Keep each note you play in a list, `adk::Note recording [50];`,
   with how long you held it: an `adk::Stopwatch`, as in Lesson 3, can
   `restart ()` when a key goes down and give its `elapsed ()` time when it
   comes up. Five seconds after your last note, play it all back with
   `speaker.play (recording, count);`, where `count` is how many notes you
   recorded.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts, the black
lead in **COM** and the red in **V**. Never move the red lead to the **A**
jack for these: set for current, the meter is just a wire, and would short
out whatever you put it across.

The sketch needs no changes: a key's note sounds for as long as you hold
it. Ask a helper to hold a key while you hold the probes, or hold both
probes in one hand like chopsticks. Keep each tip in its own hole.

!!! question "Predict"
    While a note sounds, pin 10 switches from 5 V to 0 V and back hundreds
    of times a second. What will a meter, which is far too slow to follow
    that, show? And will G, at 392 Hz, read higher than C, at 262 Hz?

<!-- measure -->

What the numbers tell you:

- **Pin 10** reads about half of 5 V: the meter shows the average, and the
  pin is high for half of every vibration. Hold G, then C: the number stays
  the same. A higher note switches faster, but it is still high half the
  time. It reads a little under 2.5 V, because a pin gives slightly less
  than 5 V while it drives the buzzer.
- **Across the buzzer** is only about 0.15 V, and **across the resistor**
  about 2.1 V. In one loop the voltage is shared in proportion to
  resistance, and the coil is only 16 Ω of the loop's 236 Ω. Add the two
  and you get pin 10's reading back.
- The resistor's reading gives the current, by Ohm's law:
  2.1 V ÷ 220 Ω ≈ 10 mA on average, just what *The idea* worked out.
  Lesson 3's active buzzer read a steady, nearly full 5 V: that buzzer
  makes its own vibration, and this one's is made by the Mega.
