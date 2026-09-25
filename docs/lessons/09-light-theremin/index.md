---
lesson: 9
title: Light Theremin
arc: The analog world
promise: Play music by waving your hand through the air.
time: 1 hour
level: 2
sketch: Lesson09LightTheremin
parts:
  - Lesson 8's light meter, built and working
  - 10 kΩ potentiometer (the knob)
  - Passive buzzer (green circuit board underneath)
  - 220 Ω resistor (red, red, black, black, brown)
  - 5 more jumper wires
ideas:
  - Turning a sensor reading into steps on a musical scale
  - The pentatonic scale, and octaves as doubling
  - Changing a note only when it needs to change
  - Growing a project from a circuit that already works
---

## What you'll build

<!-- closeup -->

An instrument you play without touching it. Hold your hand above the light
sensor and lower it slowly: the buzzer climbs a musical scale, note by note,
while the LEDs light up to show which note is sounding. Lift your hand away
and it falls silent. Turn the knob to jump the whole instrument up or down
an octave. Play a tune, or just wave and let it sing.

## The idea

One of the very first electronic instruments, the **theremin**, was
invented in 1920. Its player moves their hands near two antennas and never
touches it. Yours listens to light instead: your hand's shadow on the
photoresistor.

In Lesson 8 you cut the range from darkest to brightest into six slices to
make a bar. Here the shadow is cut into eleven slices. Slice 0, with no hand
near, is silence. Slices 1 to 10 are ten notes, lowest to highest: the
darker the shadow, the higher the note.

The ten notes are two rounds of the **pentatonic scale**: C, D, E, G and A.
Five notes that never clash, so whatever order your hand plays them in, it
sounds like music. (The black keys of a piano make a pentatonic scale too.)

Going up an **octave** means exactly doubling the frequency: the A in the
middle of a piano is 440 Hz, and the next A up is 880 Hz. The knob picks one
of three octaves by dividing its reading by 342, which gives 0, 1 or 2. The
sketch multiplies each note by 1, 2 or 4 to match. With the knob in the
middle, slice 3 is E, 330 Hz, doubled to 660 Hz.

!!! question "Predict"
    Hold your hand high above the sensor and lower it slowly. Will you hear
    a smooth slide, like a siren, or separate steps? And what will the LEDs
    do as the notes climb? Write down your guess.

## How the theremin works

The sketch goes through two stages.

| Stage | What happens |
|---|---|
| Learning | For the first five seconds all five LEDs blink. The sketch records the brightest reading, with no hand near (`open`), and the darkest, with the sensor covered (`covered`), just as the light meter did. Then it plays a little C, E, G, C to say it's ready. |
| Playing | A hundred times a second it reads and smooths the light, works out the slice, and either falls silent or plays that slice's note in the knob's octave. |

Each of the five note names has its own LED, so the lights show the tune:

| Slices | Note | LED |
|---|---|---|
| 1 and 6 | C | red |
| 2 and 7 | D | yellow |
| 3 and 8 | E | green |
| 4 and 9 | G | blue |
| 5 and 10 | A | white |

A note that is already sounding is left alone. Restarting a tone a hundred
times a second would make it click and buzz, so the sketch remembers the
pitch it's playing and only calls the buzzer when the pitch changes.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you add anything. The buzzer **always**
    goes through its 220 Ω resistor: its coil is only about 16 Ω, so on its
    own it would try to draw far more current than a pin should give. As in
    Lesson 7, the knob's middle leg goes only to A0.

Leave Lesson 8's light meter exactly as it is. The knob goes in after the
LEDs, and the buzzer after the knob. The whole build looks like this:

<!-- bench -->

The first steps are Lesson 8's; if your light meter works, start at the
potentiometer.

<!-- steps -->

??? info "Which buzzer is which?"
    The kit has two buzzers that look alike. The **passive** one, which you
    need here, shows a green circuit board when you turn it over. The
    **active** one from Lesson 3 is sealed with black plastic underneath and
    often has a white sticker on top. The active buzzer can only make its
    own single tone; the passive one plays whatever note the Mega sends it.

    The buzzer stands across the middle gap: its + leg, the longer one, in
    f37 in the top half, where the resistor brings pin 10's signal, and its
    other leg in e37 in the bottom half, where a short black wire takes it
    to GND. Its legs are 0.3 inch apart, exactly the width of the gap.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → Lesson09LightTheremin**:

<!-- sketch -->

What's new:

- `adk::Speaker speaker {10};` is the passive buzzer, as in Lesson 5.
- `scale` is an `adk::Array` of ten pitches, using the note names from
  Lesson 5: `adk::note::c4` is the C in the middle of a piano, 262 Hz.
  Written as `adk::Array scale {...}`, with no `<...>`, it takes its type
  and its size from what's inside the braces. `octaveUp` holds the three
  multipliers, 1, 2 and 4.
- `ready` is a melody, a list of `adk::Note` as in Lesson 5, and
  `speaker.play (ready)` plays it while the sketch carries on.
- `knob.read () / 342` divides the knob's reading, 0 to 1023, by 342. Whole
  numbers divide into whole numbers, and any remainder is thrown away, so
  the answer is 0, 1 or 2: the octave.
- `learnTheRoom ()` is the light meter's from Lesson 8, except that it
  keeps the brightest reading, with no hand near, in `open`, and the
  darkest, with the sensor covered, in `covered`. `for (auto& led : bar)`
  sets all five LEDs blinking while it learns.
- `shadowSlice ()` uses `map ()` and `constrain ()` as the light meter did,
  to keep the answer between 0 and 10, even if the light goes brighter or
  darker than anything the sketch learned.
- `playNote ()` works out the pitch, and only if it differs from `playing`
  does it call `speaker.tone (pitch)`. A tone with no length keeps sounding
  until the next `tone ()` or `stop ()`.
- `note % 5` gives the note's place in the five note names, 0 to 4,
  whichever octave it is in: `%` is Lesson 4's remainder after dividing.
- `fallSilent ()` stops the buzzer and darkens the LEDs, once.

## Upload it

Upload the sketch. All five LEDs blink: you have five seconds to show the
sensor your hand. Wave it over the sensor a few times, and cover the sensor
completely with a finger at least once. When the blinking stops, the buzzer
plays a quick rising C, E, G, C.

Now hold your hand about a hand's width above the sensor and lower it
slowly. Did you predict a smooth slide? You hear separate steps instead,
because the sketch cuts the shadow into slices, and every slice is one
note: C, D, E, G, A and up again. The LEDs don't fill up like the light
meter's bar: the red, yellow, green, blue and white LEDs take turns to
light, one at a time, and each lights for its note in either octave.

Cover the sensor completely for the highest note. Lift your hand away and
the buzzer stops. Turn the knob to one end, then the other: the same notes
jump down and up by an octave.

## If it doesn't work

| What you see or hear | Try this |
|---|---|
| The LEDs follow your hand, but no sound | Check the wire from pin 10 is in j34, the resistor goes from h34 to h37, the buzzer's + leg is in f37 and its other leg in e37, and the black wire joins a37 to the − rail. Make sure it's the passive buzzer. |
| The slightest shadow plays the top note | The sketch never saw the sensor covered. Press the Mega's reset button and cover the sensor fully while the LEDs blink. |
| It never goes quiet | The room is darker than when the sketch learned it: a light went off, or your own shadow falls on the sensor. Press reset and let it learn again in the light you'll play in. |
| Notes flutter between two neighbors | Your hand is at the edge of a slice, or the lamp above you flickers. Move a little, or try a steadier light. |
| The knob changes nothing | Check A0's wire is in a31 and the knob's outer legs reach both rails. The octave only changes at a third and two thirds of the way round. |
| No LEDs light at all | Upload Lesson 8's sketch again: if the light meter doesn't work either, fix it first using its table. |

??? note "How it works"
    `adk::Speaker` uses Arduino's `tone ()`, which sets up the Mega's Timer 2
    to flip pin 10 high and low at the note's frequency on its own, without
    the sketch having to. That's why a Speaker stops PWM on pins 9 and 10, as
    Lesson 5 explained: they share that timer.

    `speaker.play (ready)` doesn't wait for the melody to finish. It starts
    the first note, and each `adk::update ()` checks whether it's time for
    the next one, so the fanfare plays while `loop ()` is already running.
    Calling `tone ()` or `stop ()` ends the melody at once.

## Make it yours

1. **A different mood.** Swap the scale for the blues: C, E♭, F, G♭, G, B♭.
   The note names only have sharps, so E♭ is `adk::note::ds4` (the same
   key as D♯), G♭ is `fs4` and B♭ is `as4`. Two octaves are twelve notes,
   so `shadowSlice ()` needs thirteen slices, and six notes to an octave
   don't fit five LEDs: decide what the lights should show.
2. **The real theremin sound.** Instead of steps, slide smoothly: play
   `speaker.tone (map (level, open, covered, 200, 2000))` whenever the
   pitch has changed by more than a few hertz. It sounds like a 1950s space
   film.
3. **Stop droning.** If your hand holds still for three seconds, let the
   note stop by itself: add an `adk::Timer`, start it for 3000 ms in
   `playNote ()` whenever the note changes, and when it has `expired ()`,
   keep the speaker quiet until your hand moves to another note.
4. **Record and replay.** Keep the notes you play in an
   `adk::Vector<adk::Note, 16>`, the growing list from Lesson 6, and
   play them back with `speaker.play (tune.data (), tune.size ())` when you
   hold the sensor covered for two seconds.
