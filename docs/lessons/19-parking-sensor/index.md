---
lesson: 19
promise: Measure distance with an echo, and turn it into lights and faster and faster beeps.
time: 1 hour
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - HC-SR04 ultrasonic sensor
  - Green, yellow and red LEDs
  - 3 × 220 Ω resistors (red, red, black, black, brown)
  - Active buzzer
  - 10 jumper wires
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
    under the **+** on its top, goes in the top half, in f34.

<!-- bench -->

<!-- steps -->

??? info "Joining the two − rails"
    The long black wire at the far end, from B-60 to T-60, is new. The
    Mega's GND comes into the bottom − rail at B-3, but the sensor's GND
    goes into the top one, close to the sensor. The two rails aren't joined
    inside the breadboard, so this wire joins them, and the sensor, the
    LEDs and the buzzer all share the Mega's GND. The sensor's VCC wire
    takes a 5V pin of its own, the inner one at the top of the long header,
    just along from Trig and Echo: it needs only about 15 mA.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open **File → Examples → Adk → Lesson19ParkingSensor**:

<!-- sketch -->

What's new:

- `adk::Ultrasonic sensor {14, 15};` is the sensor, with its trigger pin
  first and its echo pin second. From then on ADK measures by itself,
  about sixteen times a second.
- The four `constexpr` numbers are the table above: where the ticking
  starts, where the light turns yellow, then red, and where the tone goes
  steady. A name says what each number is for, and keeps the two places
  that use `slowDown` or `danger` in step when you change it.
- `sensor.distance ()` is the latest distance in centimeters, and
  `sensor.ok ()` says whether an echo came back at all. When nothing
  is within about 4 m, the `?:` from Lesson 12 counts it as 400 cm: plenty
  of room.
- `showGauge ()` switches each LED with `set ()`, which takes true or false,
  so each light is simply on in its own zone.
- `soundWarning ()` changes the beat of an `adk::Every` from Lesson 11 on
  the fly: `beeps.period (cm * 10)` sets the time between beats, 300 ms at
  30 cm. On each beat, `buzzer.beep (50)` sounds the buzzer for 50 ms and
  switches it off by itself. Closer than 10 cm, `buzzer.beep (100)` is
  asked for on every pass of `loop ()`: as each beep ends, the next pass
  starts another, so they run together into one tone, and it stops by
  itself within a tenth of a second of the hand moving back.

## Upload it

Upload the sketch as usual. Point the sensor at an open space: the green LED
lights and all is quiet. Now bring a book or your hand slowly towards it.
Inside a meter the ticking starts; at 50 cm the yellow LED takes over and the
ticks come faster; at 20 cm it's red; and at a hand's width the ticks join
into one steady tone. Pull back, and it all happens in reverse.

A flat, hard surface such as a book or a wall gives the steadiest readings.

You predicted the echo, the light and the beeps for a hand 30 cm away.
Hold it there and check: the echo takes 30 × 58 = 1,740 µs, under two
thousandths of a second; 30 cm is in the yellow zone; and the beeps come
every 30 × 10 = 300 ms, a little over three a second.

## If it doesn't work

| What you see | Try this |
|---|---|
| Always green and silent, even close up | The sensor never hears an echo. Check Trig goes to pin 14 and Echo to 15, not the other way round, and that VCC goes to the 5V pin at the top of the long header and GND to the top − rail. |
| Stuck on red with a steady tone | Something is very close to the sensor, or it sees the edge of the breadboard or a wire: point it clear of the desk. |
| The lights jump about | Soft things like a jumper or a curtain soak up sound, and slanted ones bounce it away. Try a book held square to the sensor. |
| Closer than about 2 cm it goes green | That is a real limit: the sensor can't hear an echo that comes back while it is still sending. |
| The lights work but there's no sound | Check the buzzer's + leg is in f34, under pin 12's wire in j34, and that the black wire from a34 reaches the bottom − rail. |
| An LED never lights | Its long leg goes in row b of its resistor's column (b6, b12 or b18), its short leg just to the right, where the black wire from row a runs to the − rail. |
| The **L** LED blinks long and short flashes | ADK found a problem with a pin. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    Every 60 ms, from inside `adk::update ()`, the sensor object sends a
    10 µs pulse on Trig and times how long Echo stays high. It divides that
    time by 58 to get centimeters, rounding to the nearest one. Waiting for
    the echo holds everything else up for as long as the round trip takes,
    so ADK gives up after 25 ms, the time an echo takes from about 4.3 m;
    beyond that, `ok ()` is false.

    The 60 ms between pings is the shortest the sensor's datasheet allows:
    it lets the echoes of one ping die away, so they can't be mistaken for
    the next one's.

## Make it yours

1. **Your own garage.** Change `slowDown`, `danger` and `touching` to suit
   a toy car, or your bike against a wall.
2. **Watch the numbers.** Start `Serial` as in Lesson 2, print the distance
   with `adk::println (Serial, distance);` in `loop ()`, and open the Serial
   Plotter (Lesson 7) to watch a graph of your hand coming and going.
3. **Flashing danger.** Add the blue LED on pin 29, with its own 220 Ω
   resistor, in column 24, laid out like the other three, and make it blink
   quickly with `blink (200)` whenever something is closer than 10 cm.
4. **Doorway counter.** Point the sensor across a doorway and count how many
   times the distance drops below 50 cm and comes back again. Beep once for
   each person who walks through.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts, the black
lead in **COM** and the red one in **V**.

A meter is far too slow to see an echo: at 30 cm the round trip is over in
under two thousandths of a second. The sensor's wires also run straight to
the Mega, with no hole for a probe. What the meter can see is what the
sketch decides from the echoes. Each reading needs something that keeps
still in front of the sensor, so stand a book up on the desk: it is much
steadier than a hand, and it leaves both hands free for the probes.

!!! question "Predict"
    With the book 30 cm in front of the sensor, which of the lights' pins,
    26, 27 or 28, reads 5 V? And as you slide the book slowly away, where
    will that reading drop to 0?

<!-- measure -->

What the numbers tell you:

- **The yellow light's pin** reads about 5 V with the book at 30 cm, and
  pins 26 and 28 read 0: 30 cm is in the yellow zone. Now slide the book
  slowly away along a ruler, with the probes still in place. The reading
  should drop to 0 as the book passes 50 cm, `slowDown` in the sketch,
  give or take a centimeter. If it does, the distance the Mega worked out
  from echo times, at 58 µs for every centimeter, agrees with the ruler.
- **Across the buzzer**, with the book 5 cm away, the buzzer sounds without
  a break and the meter settles at about 4.5 V. That is a little under
  5 V because the buzzer takes up to 30 mA, and a pin's voltage sags a
  little under that much load. Move the book back to 30 cm and the number
  jumps about and never settles: the pin is on for 50 ms in every 300, too
  quick for the meter to follow.
- **Across the green LED** is about 3.2 V, where the red LED in Lesson 1
  kept about 2 V. That leaves only 1.8 V for the green one's resistor, so
  it takes about 8 mA, against red's 14 mA. Measure the red LED here too,
  from b6 to b7 with the book closer than 20 cm, and see the difference
  for yourself.
