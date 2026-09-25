---
lesson: 34
title: RFID
arc: Keys you can't see
promise: Read the secret number inside a plastic card, and teach the Mega which cards are yours.
time: 45 minutes
level: 2
sketch: Lesson34Rfid
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - RC522 RFID reader, with its card and key fob
  - RGB LED
  - 3 × 220 Ω resistors (red, red, black, black, brown)
  - 7 female-to-male jumper wires
  - 4 jumper wires
ideas:
  - Radio tags that need no battery
  - A card's unique number, in hexadecimal
  - The SPI bus
  - A module that runs on 3.3 V
---

## What you'll build

<!-- closeup -->

An RGB LED glows a quiet blue, waiting. Hold the white card near the reader
and the LED flashes red: the Mega has never seen this card before. Its secret
number appears on your computer screen. Copy the number into the sketch,
upload it again, and now the same card makes the LED glow green, while the
key fob, still a stranger, gets red.

## The idea

**RFID** stands for radio-frequency identification. The coil of copper around
the edge of the reader board makes a radio field that swings back and forth
13.56 million times a second. The card has a coil too, and a tiny chip, but no
battery. Held within a few centimetres of the reader, its coil catches enough
of the field to power the chip, which answers with its **UID**, its unique
ID: a number no other card should have.

The kit's card and fob have 4-byte UIDs: 32 bits, enough for more than four
billion different numbers. Numbers like that are easier to read in
**hexadecimal**, which counts in sixteens: the digits go 0 to 9, then A, B, C,
D, E and F for ten to fifteen. Each hex digit stands for exactly four bits,
so a 32-bit UID is eight hex digits, written with `0x` in front to say it's
hex:

<p class="formula">0x1A2B3C4D = 439,041,101</p>

The reader talks to the Mega over **SPI**, a faster cousin of the I2C bus you
used for the accelerometer. SPI has separate wires for each direction: MOSI
carries bits from the Mega to the reader, MISO from the reader back, SCK is
the clock that times each bit, and a fourth wire, the **select** line,
tells this reader that it's being spoken to. The Mega's SPI pins are 50 to 53.
Confusingly, the reader calls its select pin SDA.

The reader's chip runs on **3.3 V**, not 5 V, so it takes its power from the
Mega's 3.3V pin.

!!! question "Predict"
    How far from the reader will the card still be read: 1 cm, 5 cm or 50 cm?
    Write down your guess, then test it once everything works.

## Build it

!!! warning "Unplug first"
    Always unplug the USB cable before you change any wiring, and check your
    work before you plug it back in.

!!! danger "3.3 V only"
    The reader's 3.3V pin goes to the Mega's **3.3V** pin, on the short
    header beside 5V. Never connect it to 5V: 5 V would damage the reader's
    chip.

    Its signal wires still get 5 V from the Mega, which is more than the
    chip is rated for. This course wires it that way, as most people do, and
    the reader usually copes. For a build that has to last, protect those
    wires as the box below and the [safety page](../../safety.md) explain.

<!-- bench -->

<!-- steps -->

??? info "The reader's 5 V signals"
    The reader takes 3.3 V power, but the Mega drives its SDA, SCK, MOSI and
    RST wires at 5 V, which is more than its chip is rated for. Almost
    everyone wires these readers this way and they work, but for a build that
    has to last years, put a level shifter in those four wires, or a 1 kΩ
    resistor from the Mega's pin and a 2 kΩ resistor from the reader's pin to
    GND. The reader's MISO needs nothing: 3.3 V is already a high for the
    Mega. The [safety page](../../safety.md) lists this with the kit's other
    parts that need care.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → Lesson34Rfid**:

<!-- sketch -->

What's new:

- `adk::Rfid reader {53, 45};` is the reader, with its select pin on 53 and
  its reset on 45. It uses the SPI pins 50, 51 and 52 without being told.
- `knownCards` is the list of cards the sketch knows, in an `adk::Array` as
  in Lesson 4. Here the Array's type is written out: `<uint32_t, 2>` says
  it holds two `uint32_t`s, whole numbers of 32 bits, big enough for a card's
  number. Add a card, and the 2 becomes 3.
- `quietBlue` is the dim blue the LED glows while it waits.
- `reader.ok ()` says whether the reader answered when the Mega started. If
  it didn't, the LED shows yellow, so you can tell without a computer.
- `reader.wasRead ()` is an event, like `wasPressed ()` in Lesson 2: true once
  each time a card arrives, not all the while it stays there.
- `auto card = reader.uid ();` is the card's number, and
  `Serial.println (card, HEX)` prints it in hexadecimal.
- `isKnown ()` goes through `knownCards` one card at a time with a
  range-`for`, and says `true` as soon as it finds this card, or `false` if
  none match.
- `light.show ()` and `light.fadeTo ()` are the RGB LED from Lesson 4: a
  flash of green or red that fades back to blue over two seconds.

## Upload it

1. Upload the sketch and open the Serial Monitor at 9600 baud. The LED glows
   dim blue.
2. Hold the card flat against the reader. The LED flashes red and fades back,
   and the Serial Monitor shows a line such as `Card 0x93A1F20B`.
3. Copy that number into `knownCards`, in place of `0x12345678`, and upload
   again.
4. Now the card makes the LED flash green. The fob still gets red: put its
   number in place of `0x9ABCDEF0` if it's yours too.

Now try your prediction: lift the card slowly away from the reader, hold it
still, and find the furthest it still makes a flash. You predicted 1, 5 or
50 cm. Most readers manage a few centimetres, far nearer 5 than 50: further
away, the card's coil can't catch enough of the field to power its chip.

## If it doesn't work

| What you see | Try this |
|---|---|
| The LED is yellow from the start | The reader didn't answer. Check all seven wires, especially 3.3V and GND, and that SDA goes to 53 and RST to 45. |
| The LED stays blue when you hold a card | Hold the card flat and still, right against the middle of the reader. Check MISO goes to 50 and MOSI to 51: they're easy to swap. |
| A card from home is never read | The reader only reads cards with a 4-byte UID, like the kit's card and fob. Stickers, travel cards and phones often have 7-byte UIDs. |
| The colors are wrong | Check the LED's legs: red in a6, the longest leg in B-7, green in a9 and blue in a11. |
| The Serial Monitor shows nonsense | Set it to 9600 baud. |
| A known card still flashes red | Check you copied every digit after `0x` exactly. A number shorter than eight digits is fine: the Serial Monitor leaves out leading zeros. |
| The little **L** LED blinks long and short flashes | ADK found a pin problem in the sketch. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    Ten times a second, the reader sends out a short radio call that means
    "is any card there?". A card in the field answers, and the reader then
    asks for its UID. ADK never waits for the answers: each `adk::update ()`
    starts a step or collects its reply over SPI, in well under a millisecond,
    so the LED keeps fading smoothly while the reader works.

    A UID is like a name badge: easy to read, and not hard to copy. It's
    fine for a game or a toy, but real door locks check a secret stored on
    the card as well.

## Make it yours

1. **Hello, you.** Give each known card a name, and print it on the Serial
   Monitor when it's read.
2. **Keep glowing.** Keep the LED green for as long as a known card stays on
   the reader, and fade only when it leaves: `reader.isPresent ()` is true
   while a card is there.
3. **Count them.** Count how many times each known card has been shown, and
   flash the LED that many times.
4. **Learn a card.** Add a button on pin 22. When it's pressed, the next card
   read is added to the list of known cards. You'll need a list that can
   grow: make `knownCards` an `adk::Vector<uint32_t, 10>`, as in Lesson 6,
   and `push_back ()` each new card.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM**, the red one in **V**. The reader's seven wires
run straight from the Mega's pins to the reader, with no breadboard hole on
the way, so there's nowhere on this build to touch a probe to its 3.3 V
supply, or to the 5 V on its signal wires. The RGB LED's pins, though, show
what the sketch does with each card.

!!! question "Predict"
    `quietBlue` is `{0, 0, 40}`: blue at 40 out of 255. What will the meter
    read on the blue leg's pin while the LED waits?

<!-- measure -->

What the numbers tell you:

- **The blue leg's pin** reads about 0.8 V. The pin is switched on for 40
  parts in 255 of the time, too fast to see, and the meter shows the
  average, just as your eye does: 40 ÷ 255 × 5 V ≈ 0.8 V.
- **The red leg's pin** reads 0 while the LED waits. Hold a card the sketch
  doesn't know to the reader, and it jumps to about 5 V as the LED flashes
  red, then slides back down to 0 over two seconds as `fadeTo` runs, while
  the blue pin climbs back to 0.8 V. Take the card away and bring it back
  to see it again: each arrival is read once.
