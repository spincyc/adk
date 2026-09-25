---
lesson: 19
title: Parking Sensor
arc: Distance and motors
promise: Measure distance with an echo, and turn it into lights and faster and faster beeps.
time: 60 minutes
level: 2
sketch: Lesson19ParkingSensor
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - HC-SR04 ultrasonic sensor
  - Green, yellow and red LEDs
  - 3 × 220 Ω resistors (red, red, black, black, brown)
  - Active buzzer
  - 11 jumper wires
  - 4 female-to-male jumper wires
ideas:
  - Measuring distance with an echo
  - The speed of sound, as microseconds per centimeter
  - Turning a distance into a beep rate
---

## What you'll build

<!-- closeup -->

Wave your hand in front of the sensor and bring it slowly closer. Far away,
the green light glows and everything is quiet. Inside a meter the buzzer
starts to tick, once a second; closer still the light turns yellow and the
ticks hurry, then red, and a hand's width away the ticks run together into
one long tone. It is exactly how a car's parking sensor tells a driver to
stop, and it works by listening for the echo of a sound too high for you to
hear.

## The idea

The HC-SR04 has two round "eyes". One is a tiny loudspeaker, marked **T**
for transmitter; the other is a microphone, **R** for receiver. When the
Mega pulses the **Trig** pin, the speaker sends eight clicks at 40,000
vibrations a second. That is **ultrasound**: far above the 20,000 a second
that the sharpest ears can hear. The sound bounces off whatever is in front
and comes back to the microphone. The sensor holds its **Echo** pin high for
exactly as long as the round trip took.

Sound is fast, but not that fast: at room temperature it travels 343 meters
a second, which is 0.0343 cm every microsecond (a millionth of a second).
To reach something 1 cm away and come back, it travels 2 cm, which takes

<p class="formula">time = <span class="fraction"><span>2 cm</span><span>0.0343 cm per µs</span></span> ≈ 58 µs</p>

So every 58 microseconds of echo means one more centimeter. An echo that
lasts 1,160 µs has come from 1,160 ÷ 58 = **20 cm** away. That is the whole
trick, and the Mega can time an echo to the microsecond.

The rest is choosing what to show. The sketch splits the distance into three
zones, and makes the gap between beeps 10 ms for every centimeter:

| Distance | Light | Sound |
|---|---|---|
| 1 m or more | green | quiet |
| 50 cm to 1 m | green | a beep every 0.5 to 1 second |
| 20 to 50 cm | yellow | a beep every 0.2 to 0.5 second |
| 10 to 20 cm | red | a beep every 0.1 to 0.2 second |
| closer than 10 cm | red | one steady tone |

!!! question "Predict"
    Hold your hand 30 cm from the sensor. How long will each echo take?
    Which light will be on, and roughly how many beeps will you hear each
    second? Write your answers down, then check them when it's built.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring, and check your
    wiring before you plug it back in. The sensor's four pins are printed
    VCC, Trig, Echo and GND on its front: match them by name. VCC wired to
    GND the wrong way round can ruin the sensor. The buzzer's longer leg,
    under the **+** on its top, goes nearest the Mega.

<!-- bench -->

<!-- steps -->

??? info "Two things that look new"
    **A wire joining the two − rails.** The rails along the top and bottom of
    the breadboard are not joined inside it. The sensor and the buzzer take
    their GND from the top − rail, and the LEDs from the bottom one, so the
    long black wire at the far end ties the two together.

    **The resistor after the LED.** In Lesson 1 the resistor came first.
    Here each LED's long leg meets the wire from the Mega, and the resistor
    sits between its short leg and GND. It works just the same: the same
    current flows all the way round the loop, so a resistor anywhere in it
    sets that current. With 220 Ω each LED takes about 13 mA.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson19ParkingSensor**:

<!-- sketch -->

What's new:

- `adk::Ultrasonic sensor {14, 15};` is the sensor, with its trigger pin
  first and its echo pin second. From then on ADK measures by itself,
  about sixteen times a second.
- `sensor.distance ()` is the latest distance in centimeters, and
  `sensor.hasEcho ()` says whether anything answered at all. When nothing
  is within about 4 m, the sketch counts it as 400 cm: plenty of room.
- `condition ? this : that` is a question in one line: if the condition is
  true it gives the first value, otherwise the second.
- `showGauge ()` switches each LED with `set ()`, which takes true or false,
  so each light is simply on in its own zone.
- `soundWarning ()` changes the beat of an `adk::Every` from Lesson 11 on
  the fly with `beeps.period ()`. On each beat, `buzzer.beep (50)` sounds
  the buzzer for 50 ms and switches it off by itself; `buzzer.on ()` holds
  it on for the steady tone.

## Upload it

Upload the sketch as usual. Point the sensor at an open space: the green LED
lights and all is quiet. Now bring a book or your hand slowly towards it.
Inside a meter the ticking starts; at 50 cm the yellow LED takes over and the
ticks come faster; at 20 cm it's red; and at a hand's width the ticks join
into one steady tone. Pull back, and it all happens in reverse.

A flat, hard surface such as a book or a wall gives the steadiest readings.

## If it doesn't work

| What you see | Try this |
|---|---|
| Always green and silent, even close up | The sensor never hears an echo. Check Trig goes to pin 14 and Echo to 15, not the other way round, and that VCC and GND reach the top rails. |
| Stuck on red with a steady tone | Something is very close to the sensor, or it sees the edge of the breadboard or a wire: point it clear of the desk. |
| The lights jump about | Soft things like a jumper or a curtain soak up sound, and slanted ones bounce it away. Try a book held square to the sensor. |
| Closer than about 2 cm it goes green | That is a real limit: the sensor can't hear an echo that comes back while it is still sending. |
| The lights work but there's no sound | Check the buzzer's + leg is in h27, with pin 12's wire in the same column, and that j30's wire reaches the top − rail. |
| An LED never lights | Its long leg goes in row a under the wire from the Mega (a6, a13 or a20), and its resistor's other end needs the black wire to the − rail. |
| The **L** LED blinks long and short flashes | ADK found a problem with a pin. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    Every 60 ms, from inside `adk::update ()`, the sensor object sends a
    10 µs pulse on Trig and times how long Echo stays high. It divides that
    time by 58 to get centimeters, rounding to the nearest one. Waiting for
    the echo holds everything else up for as long as the round trip takes,
    so ADK gives up after 25 ms, the time an echo takes from about 4.3 m;
    beyond that, `hasEcho ()` is false.

    The 60 ms between pings is the shortest the sensor's datasheet allows:
    it lets the echoes of one ping die away, so they can't be mistaken for
    the next one's.

## Make it yours

1. **Your own garage.** Change the 50, 20 and 10 cm limits in the sketch
   to suit a toy car, or your bike against a wall.
2. **Watch the numbers.** Start `Serial` as in Lesson 2, print the distance
   in `loop ()`, and open the Serial Plotter (Lesson 7) to watch a graph of
   your hand coming and going.
3. **Flashing danger.** Add the blue LED on pin 29 (with its own 220 Ω
   resistor) and make it blink quickly with `blink (200)` whenever
   something is closer than 10 cm.
4. **Doorway counter.** Point the sensor across a doorway and count how many
   times the distance drops below 50 cm and comes back again. Beep once for
   each person who walks through.
