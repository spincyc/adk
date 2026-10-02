# Safety

The breadboard circuits use 5 V or 3.3 V, and the relay's lamp in Lessons
35 and 53 runs from a 9 V battery on the breadboard: all low voltages that
are safe to touch. The Mega gets power from USB; later project lessons use
the kit's power module with a 9 V adapter. The electricity investigations
also use an isolated 0–4 V signal generator. These rules keep the parts
and wiring safe.

!!! danger "Never mains electricity"
    Nothing in this course connects to a wall socket, and nothing ever
    should. A relay module can *switch* mains, which is exactly why it is
    only ever used here with a battery and an LED. Mains wiring can kill.

## Every lesson

- **Unplug before you wire.** Take the USB cable out before changing any
  wiring, and check your work before you plug it back in.
- **Every LED needs a resistor.** Without one, an LED takes too much current
  and can damage itself and the Mega's pin. The lessons say which resistor
  to use.
- **Watch for heat and smell.** If a part gets hot or smells, unplug at once
  and look for a wire in the wrong place, usually a short from 5 V straight
  to GND.
- **Keep the bench dry and tidy.** Loose wire snippets on a desk can bridge
  pins underneath a board.

## Electricity investigations

- **Keep sources separate.** Do not join a generator output to the Mega's
  5 V or 3.3 V pin, or to any Mega pin. Follow each circuit's ground
  connection, and keep its output within the stated range.
- **Mind capacitor polarity.** Put the striped − leg where the drawing says,
  use the stated voltage rating, and discharge through the stated resistor
  before moving it. Do not short its legs.
- **Clip a scope to ground.** In these experiments, both scope ground clips
  go to the circuit's common GND rail. Keep them off signal points. Use only
  the isolated instruments named in the [electricity syllabus](electricity/index.md).

## Motors and servos

- **Never power a motor from a pin.** A pin can give about 20 mA; a small
  motor wants hundreds. Motors run through a driver chip, the L293D or the
  ULN2003, powered from the breadboard power module.
- **Servos get their own supply too.** Join the power module's GND to the
  Mega's GND so their signals agree.
- **Keep fingers and hair clear** of fan blades and anything that turns.

## Parts that need care

The [Arduino Mega pinout](https://docs.arduino.cc/resources/pinouts/A000067-full-pinout.pdf)
lists 20 mA per I/O pin. Use this operating limit when choosing resistors
and drivers; the chip’s 40 mA absolute maximum is not a design target.

| Part | Take care |
|---|---|
| RFID reader (RC522) | It runs on **3.3 V**. Power it from the Mega's 3.3V pin, never 5V. Every build includes a 1 kΩ / 2 kΩ divider on SDA, SCK, MOSI and RST: never bypass one. The [MFRC522 datasheet](https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf) limits these inputs to the supply plus 0.5 V. |
| Active buzzer | It can need 30 mA, above a Mega pin’s recommended 20 mA. Every build uses an S8050 transistor, 1 kΩ base resistor, 10 kΩ pull-down and 1N4007 diode. Verify the transistor’s marking and E–B–C pin order before inserting it; see Lesson 3. Short beeps do not make excess pin current acceptable. |
| GY-521 accelerometer | Its regulator accepts 5 V power, but its MPU-6050 signals use 3.3 V. Every build uses a BSS138 bidirectional I2C level shifter with LV at 3.3 V, HV at 5 V and common GND. A direct 5 V pull-up exceeds the [MPU-6050 input limit](https://product.tdk.com/system/files/dam/doc/product/sensor/mortion-inertial/imu/data_sheet/mpu-6000-datasheet1.pdf). |
| Passive buzzer | Always through its 220 Ω resistor: its coil is only about 16 Ω. |
| IR LED module | Always through its 220 Ω resistor: on its own the LED would take more current than it or the pin should. |
| Water sensor | Dip only its copper traces, never its parts or pins, and keep the water away from the boards. Current through wet traces corrodes them, so the lessons power it from a pin only while they read it. |
| 9 V battery | Never let its two terminals touch each other or anything metal. |
| Laser module | Not used in this course. A laser can damage eyes. |
| Clock module | If yours charges its coin cell (see its lesson), use a rechargeable LIR2032, never a CR2032. |

## Soldering

A few boards can arrive with their header pins loose, among them the level
shifter (Lesson 28), the FM radio board (Lesson 37), some 433 MHz modules
(Lesson 38) and a Heltec board (Lesson 42). Buy them with the pins fitted
if you can. If not:

- **An adult solders, or watches closely.** The iron's tip is hotter than
  300 °C, and stays hot for minutes after it is unplugged.
- **Keep the iron in its stand** whenever it isn't in your hand. Never
  try to catch one that falls.
- **Let fresh air in.** The smoke is from the flux in the solder; open a
  window, or let a small fan draw it away from your face.
- **Use lead-free solder**, and wash your hands when you finish, before
  you eat.
- **Wear safety glasses.** Hot flux can spit, and clipped pins fly.
- **Hold the board still** with a clamp or a lump of putty, not your
  fingers, and unplug the iron when you're done.

## Radios

The add-on radios in Lessons 37 to 55 need two kinds of care: their pins
work at 3.3 V, and a radio that sends is ruled by law.

- **Never put 5 V on a 3.3 V pin.** The FM radio, the 433 MHz
  transmitter, the LoRa radios and the Meshtastic board all work at 3.3 V.
  Where the Mega drives one of their pins, the signal goes through a
  divider: 1 kΩ from the Mega's pin to the radio's, and 2 kΩ from the
  radio's pin to GND, which turns 5 V into 3.3 V. Where the Mega only pulls
  a pin low or lets it go (the FM radio's SDIO and SCLK, the LoRa module's
  M0 and M1), the radio's own resistors lift it to 3.3 V instead. The LoRa
  module's are weak, and Ebyte asks for M0 and M1 never to float, so
  Lesson 41 shows a 10 kΩ to 3.3 V that holds them firmly. The FM radio's
  board holds its RST low, so Lesson 37 lifts it with a 1 kΩ to 3.3 V.
  Their outputs are safe for the Mega to read directly.
- **Check the complete supply budget.** The FM and 433 MHz drawings use
  the Mega's 3.3V pin; identify the exact boards and their current needs
  using the [purchasing gates](buy.md#radio-purchasing-gates). The RYLR896
  draws **49.7 mA typically at +14 dBm**, according to the
  [RYLR896 datasheet](https://reyax.com/upload/products_download/download_file/RYLR896_EN.pdf).
  That is not a maximum. Lesson 40's pair uses the power module; most
  two-board drawings use the Mega's 3.3V pin for one modem at 10 dBm.
  A complete-module maximum at that setting has not been established,
  so those drawings' 50 mA supply budget remains unverified. The radio
  chip's current alone cannot prove it. Resolve this supply requirement
  before powering that arrangement; do not join two 3.3 V supplies to
  compensate. Check that a power module's orange wire comes from its
  3.3V pin: 5V would exceed the modem's rating. The Meshtastic board runs
  from its own USB cable.
- **Fit the aerial before powering a LoRa radio.** Sending into no aerial
  can damage it, and the Meshtastic board starts sending as soon as it is
  set up.
- **Establish permission before transmitting.** The band, complete device,
  aerial, power, modulation and timing all matter. ADK has not recorded
  emissions tests or established applicable authorization for each lesson's
  actual configuration. The following are limits to check, not permission
  to operate a particular board. Until the configuration is established,
  read the transmitting lesson without powering its transmitter.

| Radio | Band | What remains to check |
|---|---|---|
| FM radio | 87.5–108 MHz | Receives only; check local restrictions on reception and use. |
| WL102-341 transmitter | 433.92 MHz | The USA and Canada have timing, radiated-emission, bandwidth and authorization requirements. Removing the aerial or keeping modules on one desk does not establish compliance. No applicable authorization or emissions measurement for this build is recorded. |
| E32-433T20D | Around 433 MHz | The course requires a licensed amateur operating arrangement in the USA and Canada. The EU's 433.05–434.79 MHz entry allows 10 mW ERP and at most 10% duty cycle; a 10 dBm conducted setting with an unspecified aerial does not establish that radiated limit. The exact revision and aerial need verification. |
| RYLR896 modem, Heltec board | Regional 868 or 915 MHz bands | Match the hardware, aerial and settings to applicable authorization and local rules. Region or frequency selection alone does not establish compliance. See the configuration notes below. |

- **Timing is only one requirement.**
  [US 47 CFR 15.231(e)](https://www.ecfr.gov/current/title-47/section-15.231)
  and [Canada RSS-210 A.1.5](https://ised-isde.canada.ca/site/spectrum-management-telecommunications/en/devices-and-equipment/radio-equipment-standards/radio-standards-specifications-rss/rss-210-licence-exempt-radio-apparatus-category-i-equipment)
  specify at most one second per transmission, then silence at least
  30 times as long and at least 10 seconds. ADK limits message duration
  and inserts a rest intended to meet that timing, including a 12.78-second
  rest on startup. Software pacing cannot establish the separate field
  strength, unwanted-emission or bandwidth limits. Equipment authorization
  also applies: see [US 15.201](https://www.ecfr.gov/current/title-47/section-15.201)
  and RSS-210 section 3. Do not presume a home-built exemption covers a kit.
- **Regional settings need regional evidence.**
  [EU Decision 2025/105](https://eur-lex.europa.eu/eli/dec_impl/2025/105/oj/eng)
  specifies radiated power and access conditions for short-range bands;
  check its national implementation and equipment conformity. For an
  established RYLR896 arrangement, 868.1 MHz / 14 dBm is only a starting
  setting for the 868.0–868.6 MHz, 25 mW ERP / 1% entry. The bridge's
  two-second refresh can exceed 1%; 869.525 MHz lies in the
  869.4–869.65 MHz / 10% entry, but repeated knob changes still need a
  duty-cycle budget. Neither setting verifies radiated power.
  Australia's [2025 class licence](https://www.legislation.gov.au/F2025L01047/asmade/text)
  includes 915–928 MHz subject to its equipment-category conditions;
  `.band = 921500000` avoids centring at the lower band edge, but does
  not establish the other conditions. Meshtastic's
  [region setting](https://meshtastic.org/docs/configuration/radio/lora/)
  selects regional bands and duty limiting where implemented; it is not
  evidence of a board's authorization.
- **An amateur licence has its own conditions.** A qualified operator
  must establish the permitted band, emissions, power and identification
  method. [US 47 CFR 97.119](https://www.ecfr.gov/current/title-47/section-97.119)
  requires identification at communication end and at least every ten
  minutes; other countries differ. Adding a call sign to a message alone
  does not establish an authorized operating arrangement.
- **Mesh messages are public.** Anyone nearby with Meshtastic can read
  the default channel. Lesson 42 sets up a private one; even so, never send
  anything personal.

## Grown-ups

The lessons are written for learners from about twelve upwards, working
alone or with a family member or teacher. Younger learners should build with
an adult alongside, and anyone under 18 should solder only with an adult
there.
