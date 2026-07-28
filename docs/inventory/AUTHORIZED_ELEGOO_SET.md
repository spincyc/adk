# Authorized Elegoo product-family set

Status: **family scope established; electrical qualification and bench
acceptance remain separate**  
Observed: 2026-07-27

The complete authorized set for this source snapshot is the deduplicated union
of:

1. Elegoo's official Mega 2560 Most Complete Starter Kit tutorial archive,
   `V1.0.2023.05.05`, using the packing list in its English manual; and
2. Elegoo's official **Upgraded 37 in 1 Sensor Modules Kit** product image and
   `V2.0.0.2020.03.10` tutorial page.

This resolves which product families are in curriculum scope. It does not
claim that a particular carton was inspected, and it does not establish an
electrical identity, pin order, polarity, rating, supported adapter, or bench
result. Those claims remain gated by the selected physical revision and the
[unpowered capture procedure](UNPOWERED_CAPTURE.md) where official evidence is
insufficient.

Elegoo also sells a materially different **37 in 1 Sensor Modules Kit V3.0**.
It is not this snapshot's baseline. A box marked `V3.0`, or one containing
pressure, color, smoke, CO, air, or soil-moisture modules, requires a separate
source snapshot rather than silently changing this list.

## Sources and evidence strength

| ID | Official source | Revision basis | Evidence used |
|---|---|---|---|
| `mega-2023` | [Mega tutorial page][mega-tutorial] and [official archive][mega-zip] | Archive `V1.0.2023.05.05`; English manual filename `V1.0.2021.05.13` | Manual packing-list pages 4–9 |
| `mega-web` | [Mega product page][mega-product] | Live page observed 2026-07-27 | Partial “What's In Box” table only |
| `sensor37-v2` | [Upgraded 37 product page][sensor37-product], [official inventory image][sensor37-image], and [tutorial page][sensor37-tutorial] | Image revision exposed as 2021 asset; tutorial archive named `V2.0.0.2020.03.10` | Complete labelled 37-item product image |

The Mega archive SHA-256 is
`8bbe19fb701a4b550b3227bf6d6fb98d8125ce8455d01d850f715d912f3ee8a5`.
The official 37 tutorial page still identifies the V2 archive, but its direct
Elegoo download currently returns 404. The page and first-party product image
therefore establish family membership; they do not establish hidden circuit
details.

## Mega source manifest

Quantities below are from the revisioned manual packing list, not inferred from
lesson use.

| Product-family entry | Quantity |
|---|---:|
| Mega 2560 controller; LCD1602 with pin header; RC522 RFID module | 1 each |
| Prototype expansion module; breadboard power module | 1 each |
| GY-521 inertial module; SG90 servo | 1 each |
| Stepper motor; ULN2003 stepper-driver module | 1 each |
| IR remote; MAX7219 module | 1 each |
| One-digit and four-digit 7-segment displays | 1 each |
| L293D; 74HC595; active buzzer; passive buzzer | 1 each |
| 10 kΩ potentiometer | 2 |
| HC-SR501 PIR; sound module; water-level module; HC-SR04 ultrasonic | 1 each |
| DS1307 RTC; rotary encoder; DHT11 module; IR receiver; joystick | 1 each |
| 5 V relay; fan blade and 3–6 V DC motor set; 4×4 membrane keypad | 1 each |
| 830-point breadboard; 9 V battery with snap; 9 V/1 A adapter; USB cable | 1 each |
| Breadboard jumper-wire set | 65 wires |
| Female-to-male DuPont wires | 20 |
| Resistor assortment | 120 total |
| Thermistor | 1 |
| Rectifier diodes | 5 |
| 100 µF and 10 µF electrolytic capacitors | 2 each |
| PN2222 and S8050 transistors | 5 each |
| Tilt-ball switch | 1 |
| Buttons | 5 |
| Red, yellow, blue, green, and white LEDs | 5 each color |
| RGB LEDs | 2 |
| 104 and 22 pF ceramic capacitors | 5 each |
| Photoresistors | 2 |

The live partial product table conflicts with this manual on some counts,
including female-to-male wires, RGB LEDs, and photoresistors. Counts stay
source-specific. The archive also documents random shipment of alternate
MAX7219, RTC, and inertial-module revisions, including DS3231 and QMI8658
alternatives; those are variant families, not proof of the selected specimen.

## Upgraded 37-in-1 source manifest

The official inventory image labels 37 modules, one of each:

| # | Elegoo label | # | Elegoo label |
|---:|---|---:|---|
| 1 | Joystick | 20 | 18B20 Temp |
| 2 | Two-Color | 21 | Rotary Encoder |
| 3 | IR Emission | 22 | Relay |
| 4 | Membrane Switch (4×4 keypad) | 23 | HC-SR501 |
| 5 | RGB LED | 24 | GY-521 |
| 6 | 7 Color Flash | 25 | Power Supply |
| 7 | Laser Emit | 26 | Temp and Humidity |
| 8 | SMD RGB | 27 | Photo-Interrupter |
| 9 | Tilt-Switch | 28 | Tap Module |
| 10 | Photo-Resistor | 29 | Tracking |
| 11 | Ultrasonic Sensor (HC-SR04) | 30 | Magnetic Spring |
| 12 | Button | 31 | Avoidance |
| 13 | Active Buzzer | 32 | Digital Temperature |
| 14 | Shock | 33 | Flame |
| 15 | Water Level Sensor | 34 | Linear Hall |
| 16 | IR Receiver | 35 | Big Sound |
| 17 | Passive Buzzer | 36 | Metal Touch |
| 18 | DS1307 RTC Module | 37 | Small Sound |
| 19 | LCD 1602 Module |  |  |

The pictured tutorial/code disc and 100-resistor gift are extras, not members
of the numbered 37. Preserve active/passive buzzers, RGB variants, and
big/small sound boards as distinct product families.

## Deduplicated union rule

Entries appearing in both manifests count once in family-level coverage while
retaining both provenances and each source's quantity. The clear overlaps are:

- joystick, rotary encoder, 4×4 keypad, button, tilt switch, photoresistor;
- active and passive buzzer, and sound-module family;
- HC-SR04, HC-SR501, water-level module, IR receiver;
- DS1307 RTC, LCD1602, GY-521, DHT11, relay, and breadboard power module.

Variants are not flattened merely because they share a broad behavior. In
particular, active/passive buzzer, big/small sound, RGB/SMD RGB/two-color,
DS1307/DS3231, GY-521/QMI8658, and bare versus module-mounted parts retain
separate identities.

## Admission states

| State | Meaning |
|---|---|
| Listing authorized | The family appears in one of the cited source manifests and may be planned in the curriculum |
| Electrical identity open | Exact IC, PCB revision, pinout, topology, limits, or polarity is unresolved |
| Host verified | Deterministic software behavior is verified without claiming a physical adapter |
| Bench accepted | A named specimen passed the required physical acceptance record |

Listing authorization is never authorization to energize hardware. Lasers
remain unpowered/excluded; relay work is never mains; gas/heater exposure,
physiological claims, unknown IR replay, ignition, and pyrotechnic control
remain excluded. Motors, servos, steppers, relays, and supply modules retain
their stronger energy and safe-state gates.

## Curriculum reconciliation

The union directly authorizes the joystick and rotary-encoder families for
lessons 031–033, so host-contract and family-level planning may proceed.
Revision-specific wiring and physical acceptance remain open.

The engagement-first curriculum assigns the authorized sensor families as
follows:

- lessons 040–042: optical sensing;
- lessons 043–045: one identified GY-521/MPU-6050 or QMI-8658 inertial
  revision, orientation, and the balance-table instrument;
- lessons 046–048: the listed Metal Touch family, a contact or existing
  joystick fallback, bounded stepper motion, and the kinetic sculpture;
- lessons 049–051: RC522 identity observations and the parts carousel;
- lessons 052–054: identified IR receive and listed IR-emission families,
  restricted to known local codes;
- lessons 055–057: the cooperative escape-room console;
- lessons 058–060: four-digit and MAX7219 display work;
- lessons 061–063: a museum monitor using the listed Water Level, thermistor,
  Digital Temperature, Flame, and Magnetic Spring families;
- lessons 064–066: single-wire transport and the listed 18B20 Temp family;
- lessons 067–069: normalized records from the identified inertial revision;
- lessons 070–072: characterization of identified low-voltage
  analog/comparator families; and
- lessons 079–081: inert low-energy component qualification.

Metal Touch is listing-authorized, but its exact topology, pinout, thresholds,
and safe stimulus remain gated by specimen evidence. `Digital Temperature` is
a distinct label from `18B20 Temp`; it remains electrically unidentified and
must not be treated as an 18B20 or assigned a powered adapter until specimen
evidence proves its identity.

The prior long-range plan also contained several candidates imported from
other 37-sensor taxonomies. They are not authorized by this source union:

- DS1302 in lesson 073; the cited Elegoo baseline supplies DS1307, with a
  documented DS3231 shipping alternative;
- BMP180 and PCF8591 in lesson 074;
- the color-sensor arc in lessons 076–078; and
- rain, soil-moisture, generic capacitive-touch, and heartbeat/pulse modules;
  these appeared in historical planning prose but not in either cited
  manifest; and
- any programmable IR transmitter beyond the listed IR-emission family until
  its exact revision and safe use are established.

Lessons 073–078 remain reserved for authorized-family re-scoping; no
replacement subjects have been selected. Historical references to the
unauthorized candidates document earlier planning only. They are not evidence
that such specimens exist and confer no curriculum or electrical scope.

[mega-product]: https://www.elegoo.com/en-gb/collections/arduino-kits/products/elegoo-mega-2560-the-most-complete-starter-kit
[mega-tutorial]: https://www.elegoo.com/en-gb/blogs/arduino-projects/elegoo-mega-2560-the-most-complete-starter-kit-tutorial
[mega-zip]: https://download.elegoo.com/01%20STEM%20Kits/02%20Mega%202560/Complete/ELEGOO%20The%20Most%20Complete%20Starter%20Kit%20for%20MEGA%20V1.0.2023.05.05.zip
[sensor37-product]: https://www.elegoo.com/products/elegoo-37-in-1-sensor-kit
[sensor37-image]: https://www.elegoo.com/cdn/shop/products/elegoo-upgraded-37-in-1-sensor-modules-kit-compatible-with-arduino-ide-arduino-stem-kits-elegoo-shop-592935.jpg?v=1622707708
[sensor37-tutorial]: https://www.elegoo.com/en-gb/blogs/arduino-projects/elegoo-upgraded-37-in-1-sensor-modules-kit-tutorial
