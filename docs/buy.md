# What to buy

Everything the course uses beyond [the kit](kit.md): each path's shopping
list, then what to look for in each instrument and tool, with models that
meet it.

Prices are rough US dollars, checked in October 2026; they vary by shop
and country, and postage is extra. **ADK sells nothing and earns nothing
from any shop.** Each model links to its maker's page and to a search for
it on Amazon, rather than to one listing, which comes and goes; outside the
US, search your own country's Amazon for the same model name, or buy from
any shop you trust. A model listed here meets the course's needs by its
maker's published specifications. None has yet been used in a
[recorded build](builds.md), so check a listing against the *Must have*
list before you buy.

## Shopping lists {#lists}

### Project path, Lessons 1 to 36

| Qty | Item | Rough cost |
|---|---|---|
| 1 | Elegoo Mega 2560 Most Complete Starter Kit: the Mega, USB cable, breadboard, parts, power module, 9 V adapter and 9 V battery | US$55–70 |
| 1 | Elegoo 37 in 1 Sensor Modules Kit, for the modules marked *(37 in 1)* in [the parts](kit.md#the-parts) | US$30–40 |
| 1 | BSS138 I2C level shifter on the Adafruit 757 layout, for Lessons 28, 30 and 50: one with its headers fitted if you can find it, as Adafruit's own comes with them loose, to [solder](#soldering) | US$4–8 |
| 1 | [Small screwdriver](#screwdriver) | US$3–5 |
| 1 | [Digital multimeter](#multimeter), for *Measure it* (optional) | US$20–45 |
| | **About** | **US$95–170** |

### Radio arcs, Lessons 37 to 42

| Qty | Item | Rough cost |
|---|---|---|
| 1 set | Each of the [add-on radios](kit.md#add-on-radios), with aerials | US$105–160 |
| | Without Lesson 41's E32 modules, where they need a license | US$90–135 |
| | Without Lesson 42's Heltec boards | US$50–100 |
| | Without either | US$40–75 |

Some add-on boards arrive with their header pins loose, to be
[soldered](#soldering).

### Two boards, Lessons 43 to 55

Besides the pair of RYLR896 modems from Lesson 40:

| Qty | Item | Rough cost |
|---|---|---|
| 1 | A second Elegoo Mega 2560 Most Complete Starter Kit, which covers all of Board B's parts | US$55–70 |
| | *or* a second Mega 2560 (a compatible board costs less than an Arduino one), USB cable and breadboard, a second breadboard power module for Lesson 51, and a second MAX7219 matrix, joystick and passive buzzer for Lessons 54 and 55 | US$40–75 |
| 1–2 | [USB power banks](#power-banks) or USB wall chargers, or a long USB cable, to put the boards in different rooms | US$10–45 |

### Electricity path, E01 to E24

| Qty | Item | Rough cost |
|---|---|---|
| 1 | Elegoo Mega 2560 Most Complete Starter Kit, the same as the project path's | US$55–70 |
| 1 | [Digital multimeter](#multimeter) with DC volts, from E02 on | US$20–45 |
| 1 set | The extra parts in each module's [equipment gate](electricity/index.md#equipment-gates): capacitors, a 100 mH inductor, an MCP6002, a 74HC00 and a 74HC14, a 10 Ω and two 100 kΩ resistors | US$10–20 |
| | **About** | **US$85–135** |

### Scope extension, for E11, E13 to E16 and E18

| Qty | Item | Rough cost |
|---|---|---|
| 1 | A battery-powered two-channel [oscilloscope](#oscilloscope) | US$90–210 |
| 1 | A [signal generator](#generator) that sets both amplitude and offset, run from a power bank, with its cable | US$25–150 |
| | **About** | **US$115–360** |

## Instruments and tools at a glance {#at-a-glance}

| Item | Needed for | Must have | Rough cost |
|---|---|---|---|
| [Digital multimeter](#multimeter) | *Measure it* in most project lessons, which you can skip without one; the electricity path from E02 on | DC volts down to millivolts | US$20–45; a school meter about US$200 |
| [Oscilloscope](#oscilloscope) | E11, E13–E16 and E18; optional in E12, E17 and E23 | Its own battery, two channels, AC coupling or a vertical offset | US$90–210 |
| [Signal generator](#generator) | The same investigations | Power from a battery or power bank; amplitude and offset to give 0–4 V | US$25–150 |
| [Stopwatch](#stopwatch) | E08 and E21 | Seconds and tenths | A phone's clock |
| [Small screwdriver](#screwdriver) | The relay's terminals, in Lessons 35 and 53 | A flat blade about 2.5 mm wide | US$3–20 |
| [Soldering iron](#soldering), stand and lead-free solder | Header pins on add-on boards that arrive without them: the level shifter (Lesson 28), the FM radio board (Lesson 37), some 433 MHz modules (Lesson 38) and a Heltec board (Lesson 42) | Temperature control and a stand | US$30–130 |
| [USB power bank](#power-banks) | Two-board lessons with the boards in different rooms; the signal generator | Stays on at a small load | US$10–45 |

## Digital multimeter {#multimeter}

The course only ever uses a meter's **DC volts**: it finds currents by
measuring the voltage across a resistor ([Ohm's law](laws/ohms-law.md#current-from-voltage)),
so the current jacks are never needed.
[Measurement skills](electricity/skills.md) shows how to take a reading.

**Must have:**

- **DC volts that show millivolts.** E05 reads about 30 mV across a
  10 Ω resistor. A meter that shows 0.001 V on its lowest DC range will
  do; one with a 200 mV or 600 mV DC range shows tenths of a millivolt.
- **About 10 MΩ input resistance**, as nearly every digital meter has, so
  the meter barely loads what it measures.
- **Helpful:** auto-ranging, so there is no range to choose; a continuity
  beeper, for finding a broken wire; auto power-off; a hold button.

| Model | Lowest DC range | Notes | Rough cost | Find it |
|---|---|---|---|---|
| UNI-T UT33A+ | 200 mV, reading 0.1 mV | Auto-ranging (the B+, C+ and D+ are not); about 10 MΩ input; hold, beeper, auto-off; fused current jacks | US$20 | [Maker](https://meters.uni-trend.com/product/ut33plus-series/) · [Amazon](https://www.amazon.com/s?k=UNI-T+UT33A%2B) |
| Kaiweets HT118A | 600 mV, 6000 counts | Auto-ranging; hold, beeper, auto-off | US$45 | [Maker](https://kaiweets.com/products/ht118a-digital-multimeter) · [Amazon](https://www.amazon.com/s?k=KAIWEETS+HT118A) |
| UNI-T UT61E+ | 220 mV, reading 0.01 mV | For more digits: 22 000 counts, 0.1 % on DC | US$90–130 | [Maker](https://meters.uni-trend.com/product/ut61plus-series/) · [Amazon](https://www.amazon.com/s?k=UNI-T+UT61E%2B) |
| Fluke 114 | 600 mV, reading 0.1 mV | A school meter: no current function at all, so nothing to set wrong, and a three-year warranty. Use its V⎓ or mV⎓ setting, not Auto-V/LoZ, whose low input resistance would load a circuit | US$200 | [Maker](https://www.fluke.com/en-us/product/electrical-testing/digital-multimeters/fluke-114) · [Amazon](https://www.amazon.com/s?k=Fluke+114) |

Fluke's cheaper 101, 106 and 107 show DC only to 0.001 V. They will do,
but without the extra digit.

## Oscilloscope and signal generator {#scope-and-generator}

Only the optional scope extension needs them: E11, E13 to E16 and E18,
with a scope optional in E12, E17 and E23. Read the
[scope and generator primer](electricity/skills.md#scope-and-generator)
before buying: it explains why both must run from their own batteries,
never from the mains, and how to check the generator's output before it
touches a circuit.

### The oscilloscope {#oscilloscope}

**Must have:** its own battery; two channels; at least 1 MS/s (a million
samples a second), which every current model beats by far; AC coupling or
a vertical offset, to see a small ripple on a 5 V rail.

| Model | What it has | Rough cost | Find it |
|---|---|---|---|
| FNIRSI 2C23T | Two channels, 50 MS/s, 10 MHz, AC and DC coupling, a 3000 mAh battery; also a meter and a small generator, which can't reach 4 V | US$90 | [Maker](https://www.fnirsi.com/products/2c23t) · [Amazon](https://www.amazon.com/s?k=FNIRSI+2C23T) |
| OWON HDS242S | Two channels, 40 MHz, a battery; also a meter with a 200 mV DC range, which can serve as the course's meter, and a small generator limited to ±2.5 V | US$110–210 | [Maker](https://www.owon.com.hk/products_owon_hds200_series_digital_oscilloscope) · [Amazon](https://www.amazon.com/s?k=OWON+HDS242S) |

### The signal generator {#generator}

**Must have:**

- **Power from a battery or a USB power bank**, so its output floats:
  nothing joins it to the mains earth.
- **Sine and square waves** from 100 Hz to 1 kHz.
- **Amplitude and DC offset, each set on its own,** so it can make a wave
  that swings from 0 V to 4 V and never goes below 0 V: 4 V peak to peak,
  around a 2 V offset.
- **At least 5 mA** of output.

Many handheld scopes now include a small generator, and the syllabus
allows one that meets this list. Of the sixteen checked for this page in
October 2026, none does by its published specifications: FNIRSI's (2C23T, 2C53T, 2C53P, 2D15P) stop at about 3.3 V, and
Hantek's (2D42, 2D72) and OWON's (HDS200 and HDS300 series) stay within
±2.5 V. Pair the scope with a separate generator:

| Model | What it has | Rough cost | Find it |
|---|---|---|---|
| JDS6600 (sold as Joy-IT JT-JDS6600 in Europe, and by Koolertron and others) | Two outputs; amplitude 2 mV to 20 V peak to peak; offset −9.99 V to +9.99 V; 50 Ω output; up to 20 mA, by Joy-IT's datasheet. Takes 5 V DC | US$100–140 | [Joy-IT](https://joy-it.net/en/products/JT-JDS6600) · [Amazon](https://www.amazon.com/s?k=JDS6600+signal+generator) |
| Joy-IT JT-JDS2915 | Amplitude up to 20 V peak to peak, 50 Ω output; Joy-IT documents running it from a power bank. Its offset range is from a review, not the maker. Sold in Europe | about US$120–150 (€110–135) | [Joy-IT](https://joy-it.net/en/products/JT-JDS2915) · [Amazon](https://www.amazon.com/s?k=JDS2915+signal+generator) |
| FG-100 DDS (no maker's name) | The budget choice: knobs for amplitude and offset, up to ±10 V by its sellers and by Hackaday's review, but no readout of either, so the scope shows what you set | US$20–25 | [Amazon](https://www.amazon.com/s?k=FG-100+DDS+function+generator) |

**Powering the generator.** Run it from a [USB power bank](#power-banks)
through a USB-to-5.5 mm plug cable (check that the plug fits its 5 V
input), never from its mains adapter, and leave its USB data port
unconnected, so its output stays floating. Before each investigation, do
the primer's [output check](electricity/skills.md#check-the-output-first):
these generators can make far more than 4 V, and a 50 Ω setting can
double what they show.

## Other tools {#tools}

### Stopwatch {#stopwatch}

E08 times a capacitor charging, and E21 counts a clock's blinks. The
stopwatch in any phone's clock app is enough.

### Small screwdriver {#screwdriver}

The relay module's screw terminals, in Lessons 35 and 53, need a small
flat blade, about 2.5 mm wide. Any precision screwdriver does; a set such
as the [iFixit Moray](https://www.ifixit.com/products/moray-driver-kit)
(about US$20) covers other small screws too.

### Soldering iron, stand and solder {#soldering}

A few add-on boards arrive with their header pins loose. Buy them with
pins fitted if you can, or ask someone experienced, and read
[Soldering](safety.md#soldering) first.

**Must have:** temperature control, a stand that holds the hot iron, and
lead-free solder.

| Model | Notes | Rough cost | Find it |
|---|---|---|---|
| Pinecil V2 | A small iron powered by USB-C (12 V to 20 V, from a USB-C PD charger, sold separately), so it works in any country; 100 to 400 °C; sleeps when set down. Add a stand | US$26, plus a stand and charger | [Maker](https://pine64.com/product/pinecil-smart-mini-portable-soldering-iron/) · [Amazon](https://www.amazon.com/s?k=Pinecil+V2) |
| Adafruit 180 and stand 150 | An adjustable 30 W iron for US mains, with a separate stand and sponge | US$33 | [Iron](https://www.adafruit.com/product/180) · [Stand](https://www.adafruit.com/product/150) |
| Hakko FX-888DX | A soldering station with its holder, for a classroom | US$120 | [Maker](https://hakkousa.com/fx-888dx.html) · [Amazon](https://www.amazon.com/s?k=Hakko+FX-888DX) |
| Lead-free solder, such as SparkFun TOL-09163 | 15 g of tin, silver and copper solder: plenty for a few headers | US$5 | [SparkFun](https://www.sparkfun.com/solder-lead-free-15-gram-tube.html) |

### USB power banks {#power-banks}

A power bank lets a two-board lesson's second board sit in another room,
and keeps the signal generator floating. Many banks switch themselves off
when their load draws less than somewhere between 50 and 200 mA, to save
their battery once a phone is charged, and a Mega draws less than that.
Some banks' low-current modes still turn off after a couple of hours.

**Must have:** a port that stays on at a small load, or simply a USB wall
charger wherever a socket is near.

| Model | Notes | Rough cost | Find it |
|---|---|---|---|
| Voltaic Systems V25 | Its maker says its USB-A ports have no low-current cut-off and stay on however little a device draws | US$45 | [Maker](https://voltaicsystems.com/v25/) · [Amazon](https://www.amazon.com/s?k=Voltaic+Systems+V25) |
| A USB wall charger | Never switches off; 5 V at 1 A or more | US$10 | Any |
