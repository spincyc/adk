# What to buy

Everything the course uses beyond [the kit](kit.md): each path's shopping
list, then what each instrument, tool and extra part must do, with models
that do it.

Prices are rough US dollars, checked in October 2026; they vary by shop
and country, and postage is extra. **ADK sells nothing and earns nothing
from any shop.** Each model links to its maker's page and to a search for
it on Amazon, and each part to a search for it at a parts distributor,
rather than to one listing, which comes and goes; outside the US, search
your own country's shops for the same model name or part number. Each
model was checked against the *Must have* list using only what its maker
publishes, in its own pages, manuals and datasheets, never a seller's
listing or a review. Each meets the course's needs by its maker's
published specifications, except where this page says what its maker
leaves unsaid. None has yet been used in a [recorded build](builds.md),
so check a listing against the *Must have* list before you buy.

## Shopping lists {#lists}

### Project path, Lessons 1 to 36

| Qty | Item | Rough cost |
|---|---|---|
| 1 | Elegoo Mega 2560 Most Complete Starter Kit: the Mega, USB cable, breadboard, parts, power module, 9 V adapter and 9 V battery | US$55–70 |
| 1 | Elegoo 37 in 1 Sensor Modules Kit, for the modules marked *(37 in 1)* in [the parts](kit.md#the-parts) | US$30–40 |
| 1 | BSS138 I2C level shifter on the Adafruit 757 layout, for Lessons 28, 30 and 50: one with its headers fitted if you can find it, as Adafruit's own comes with them loose, to [solder](#soldering) | US$4–8 |
| 1 | [Small screwdriver](#screwdriver) | US$8–20 |
| 1 | [Digital multimeter](#multimeter), for *Measure it* (optional) | US$20–30 |
| | **About** | **US$115–170** |

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
| 1–2 | [USB power banks](#power-banks) that stay on at a small load, USB wall chargers you already have, or a long USB cable, to put the boards in different rooms | US$0–90 |

### Electricity path, E01 to E24

| Qty | Item | Rough cost |
|---|---|---|
| 1 | Elegoo Mega 2560 Most Complete Starter Kit, the same as the project path's | US$55–70 |
| 1 | [Digital multimeter](#multimeter) with DC volts, from E02 on | US$20–30 |
| 1 set | The [extra parts](#extra-parts) in each module's [equipment gate](electricity/index.md#equipment-gates): resistors, capacitors, a 100 mH inductor, an MCP6002, a 74HC00 and a 74HC14 | US$10–15, plus postage |
| | **About** | **US$85–115**, plus postage |

### Scope extension, for E11, E13 to E16 and E18

| Qty | Item | Rough cost |
|---|---|---|
| 1 | A battery-powered two-channel [oscilloscope](#oscilloscope), with two probes | US$300–420 |
| 1 | A [signal generator](#generator) that sets both amplitude and offset | US$110–150 |
| 1 | A [USB power bank](#power-banks) that stays on at a small load, to run the generator | US$45 |
| | **About** | **US$455–615** |

## Instruments and tools at a glance {#at-a-glance}

| Item | Needed for | Must have | Rough cost |
|---|---|---|---|
| [Digital multimeter](#multimeter) | *Measure it* in most project lessons, which you can skip without one; the electricity path from E02 on | DC volts from tens of millivolts to 9 V; about 10 MΩ input | US$20–30; a school meter about US$230 |
| [Extra parts](#extra-parts) | The electricity path's equipment gates | The values and ratings in the gates | US$10–15 |
| [Oscilloscope](#oscilloscope) | E11, E13–E16 and E18; optional in E12, E17 and E23 | Its own battery; two channels and two probes; 50 mV per division; AC coupling or a vertical offset; a Single trigger | US$300–420 |
| [Signal generator](#generator) | The same investigations | Power from a power bank; amplitude and offset set separately, to give 0–4 V; a High-Z setting; at least 5 mA | US$110–150 |
| [Stopwatch](#stopwatch) | E08 and E21 | Seconds and tenths | A phone's clock |
| [Small screwdriver](#screwdriver) | The relay's terminals, in Lessons 35 and 53 | A flat blade about 2.5 mm wide | US$8–20 |
| [Soldering iron](#soldering), stand and lead-free solder | Header pins on add-on boards that arrive without them: the level shifter (Lesson 28), the FM radio board (Lesson 37), some 433 MHz modules (Lesson 38) and a Heltec board (Lesson 42) | Temperature control and a stand | US$60–140, with solder |
| [USB power bank](#power-banks) | The signal generator; two-board lessons with the boards in different rooms | Stays on at a small load; 5 V at 2 A for the generator | US$45 |

## Digital multimeter {#multimeter}

The course only ever uses a meter's **DC volts**: it finds currents by
measuring the voltage across a resistor ([Ohm's law](laws/ohms-law.md#current-from-voltage)),
so the current jacks are never needed.
[Measurement skills](electricity/skills.md) shows how to take a reading.
The smallest reading is E05's, about 30 mV and then 60 mV across a 10 Ω
resistor; the largest is the 9 V battery in Lessons 35 and 53.

**Must have:**

- **DC volts from tens of millivolts to 9 V.** A meter whose lowest DC
  range is 200 mV to 600 mV shows E05's readings to a tenth of a
  millivolt; one that shows only 0.001 V on its lowest range will do,
  with less to read.
- **About 10 MΩ input resistance** on DC volts, so the meter barely loads
  what it measures.
- **Helpful:** auto-ranging, so there is no range to choose; an Ω setting,
  to [check a resistor](laws/units.md#reading-parts) out of the circuit;
  a continuity beeper, for finding a broken wire; auto power-off.

| Model | Lowest DC range | Notes, by its maker's manual | Rough cost | Find it |
|---|---|---|---|---|
| UNI-T UT33A+ | 200 mV, reading 0.1 mV | Auto-ranging (the B+, C+ and D+ are not); about 10 MΩ input; Ω, a continuity beeper and auto power-off; current jacks it never needs | US$20–30 | [Maker](https://meters.uni-trend.com/product/ut33plus-series/) · [Amazon](https://www.amazon.com/s?k=UNI-T+UT33A%2B) |
| UNI-T UT61E+ | 220 mV, reading 0.01 mV | For more digits: 22 000 counts; about 10 MΩ input on volts | US$80–110 | [Maker](https://meters.uni-trend.com/product/ut61plus-series/) · [Amazon](https://www.amazon.com/s?k=UNI-T+UT61E%2B) |
| Fluke 114 | 600 mV, reading 0.1 mV | A school meter: no current function at all, so nothing to set wrong; over 10 MΩ input on V⎓; a three-year warranty. Use its V⎓ or mV⎓ setting, not Auto-V/LoZ, which is about 3 kΩ and would load a circuit | US$230 | [Maker](https://www.fluke.com/en-us/product/electrical-testing/digital-multimeters/fluke-114) · [Amazon](https://www.amazon.com/s?k=Fluke+114) |

The [oscilloscope](#oscilloscope) below has a meter of its own that also
meets this list.

## Extra parts for the electricity path {#extra-parts}

The kit covers E01 to E24 except for these, which each module's
[equipment gate](electricity/index.md#equipment-gates) lists. Any part
with the same value and at least the same rating will do: if your kit's
capacitors include a 10 µF, a 100 µF or a 100 nF (marked 104) rated at
least 10 V, use them. The part numbers below are ones whose makers'
datasheets show they fit; one order from a parts distributor gets them
all.

| Part | For | Part number | Its datasheet gives | Qty | Find it |
|---|---|---|---|---|---|
| 10 Ω resistor | E05, E18 | Stackpole RNF14FTD10R0 | ¼ W metal film, 1 % | 2 | [Maker](https://www.seielect.com/catalog/sei-rnf_rnmf.pdf) · [DigiKey](https://www.digikey.com/en/products/result?keywords=RNF14FTD10R0) |
| 100 kΩ resistor | E21 | Stackpole RNF14FTD100K | ¼ W metal film, 1 % | 2 | [Maker](https://www.seielect.com/catalog/sei-rnf_rnmf.pdf) · [DigiKey](https://www.digikey.com/en/products/result?keywords=RNF14FTD100K) |
| 1000 µF electrolytic, at least 10 V | E07–E08 | Nichicon UVR1C102MPD | 16 V; 10 × 16 mm, legs 5 mm apart | 1 | [Maker](https://www.nichicon.co.jp/english/series_items/catalog_pdf/e-uvr.pdf) · [DigiKey](https://www.digikey.com/en/products/result?keywords=UVR1C102MPD) |
| 100 µF electrolytic, at least 10 V | E18, E23 | Nichicon UVR1H101MPD | 50 V; 8 × 11.5 mm, legs 3.5 mm apart | 1 | [Maker](https://www.nichicon.co.jp/english/series_items/catalog_pdf/e-uvr.pdf) · [DigiKey](https://www.digikey.com/en/products/result?keywords=UVR1H101MPD) |
| 10 µF electrolytic, at least 10 V | E16, E21 | Rubycon 50YXJ10M5X11 | 50 V, ±20 %; 5 × 11 mm, legs 2 mm apart | 2 | [Maker](https://www.rubycon.co.jp/wp-content/uploads/catalog-aluminum/YXJ.pdf) · [DigiKey](https://www.digikey.com/en/products/result?keywords=50YXJ10M5X11) |
| 1 µF film capacitor, nonpolar | E13–E15 | WIMA MKS2C041001F00KSSD | Polyester, 63 V, ±10 %; legs 5 mm apart | 1 | [Maker](https://www.wima.de/en/our-product-range/film-capacitors/mks-2/) · [DigiKey](https://www.digikey.com/en/products/result?keywords=MKS2C041001F00KSSD) |
| 100 nF ceramic capacitor | E16, E18, E19–E21 | Vishay K104K15X7RF53L2 | 50 V, X7R; legs 2.5 mm apart | 2 | [Maker](https://www.vishay.com/en/product/45171/) · [DigiKey](https://www.digikey.com/en/products/result?keywords=K104K15X7RF53L2) |
| 100 mH inductor, at least 10 mA, under about 500 Ω | E11 | Murata 19R107C | 100 mH ±10 %; up to 70 mA; at most 90 Ω | 1 | [Maker](https://pim.murata.com/en-us/pim/details/?partNum=19R107C) · [DigiKey](https://www.digikey.com/en/products/result?keywords=19R107C) |
| MCP6002 op-amp, 8-pin DIP | E16–E17 | Microchip MCP6002-I/P | Runs from 1.8 V to 6 V; inputs and output reach both supply rails | 1 | [Maker](https://ww1.microchip.com/downloads/aemDocuments/documents/MSLD/ProductDocuments/DataSheets/MCP6001-1R-1U-2-4-1-MHz-Low-Power-Op-Amp-DS20001733L.pdf) · [DigiKey](https://www.digikey.com/en/products/result?keywords=MCP6002-I%2FP) |
| 74HC00 NAND gates, 14-pin DIP | E19–E20 | Texas Instruments SN74HC00N | Runs from 2 V to 6 V | 1 | [Maker](https://www.ti.com/product/SN74HC00) · [DigiKey](https://www.digikey.com/en/products/result?keywords=SN74HC00N) |
| 74HC14 Schmitt inverters, 14-pin DIP | E21 | Texas Instruments SN74HC14N | Runs from 2 V to 6 V; at 4.5 V, switches up between 1.55 V and 3.13 V | 1 | [Maker](https://www.ti.com/product/SN74HC14) · [DigiKey](https://www.digikey.com/en/products/result?keywords=SN74HC14N) |

E11's prediction rests on the inductor's own resistance: with the
19R107C's 90 Ω at most and a generator's 50 Ω, the resistor's voltage
levels at about 3.5–3.8 V and rises with a timescale of about 90 µs,
inside the 2.6–3.8 V and 65–95 µs the page expects. Its legs are 0.8 mm
thick, about as thick as a breadboard's holes take: push them straight
in.

## Oscilloscope and signal generator {#scope-and-generator}

Only the optional scope extension needs them: E11, E13 to E16 and E18,
with a scope optional in E12, E17 and E23. Read the
[scope and generator primer](electricity/skills.md#scope-and-generator)
before buying: it explains why both must run from their own batteries,
never from the mains, and how to check the generator's output before it
touches a circuit.

### The oscilloscope {#oscilloscope}

**Must have:**

- **Its own battery**, used with its charger unplugged.
- **Two channels and two probes:** E11 and E13 to E16 compare two points
  at once.
- **At least 1 MS/s** (a million samples a second) with both channels
  on, which every current model beats by far.
- **50 mV per division at the probe tip**, for E18's ripple of about
  30 mV. Through a ×10 probe, that is 5 mV per division at the scope.
- **AC coupling, or a vertical offset** that brings a trace near 5 V to
  the middle of the screen at 50 mV per division, for E18; E12's optional
  close-up uses the offset at 0.2 V per division.
- **A Single trigger**, as well as Auto and Normal: E12's release happens
  once. Timebases from 50 µs per division (E11, E12) to 2 ms per division
  (E14 to E16) come with any scope.

| Model | What it has, by UNI-T's manual and page | Rough cost | Find it |
|---|---|---|---|
| UNI-T UTD1025DL | Two channels, 25 MHz, 250 MS/s; 5 mV to 20 V per division; DC and AC coupling, and an offset of ±1.2 V on its 5–100 mV scales, enough through a ×10 probe for a 5 V trace at 50 mV per division; 10 ns to 50 s per division; Auto, Normal and Single; two probes; a battery for about 6 hours. Its meter reads DC on a 400 mV range to 0.1 mV, with 10 MΩ input, so it can be the course's meter too | US$300–420 | [Maker](https://instruments.uni-trend.com/products/digital-oscilloscopes/UTD1000L) · [Amazon](https://www.amazon.com/s?k=UNI-T+UTD1025DL) |

Two-channel handhelds from OWON (HDS242) and Hantek (2C42) meet the rest
of this list for less, but each comes with one probe, so the second
channel needs a probe bought to match. A bench scope that runs from the
mains is earthed through its cord, so it does not fit. Rigol's DHO800
and DHO900 can run from a USB-C power bank, but Rigol's manuals ask for
the scope to be earthed through its ground lead, so they do not fit
either.

### The signal generator {#generator}

**Must have:**

- **Power from a USB power bank or a battery**, so its output floats:
  nothing joins it to the mains earth. Never its mains adapter, and its
  USB data port stays unplugged.
- **Sine and square waves** from 100 Hz to 1 kHz.
- **Amplitude and DC offset, each set on its own:** 4 V peak to peak
  around +2 V, a wave from 0 V to 4 V that never goes below 0 V, for
  E11, E13 to E15 and E18; 1 V peak to peak around +1 V for E16's
  0.5–1.5 V.
- **A High-Z load setting**, so the voltage it shows is the voltage it
  makes into these circuits, which draw very little.
- **At least 5 mA** of output: E11 and E13 draw about 4 mA through
  1 kΩ.

No generator checked for this page in October 2026 meets every item by
its maker's own documents. The one below meets all the others, and takes
5 V at 2 A, which a power bank gives; but UNI-T does not say it may run
from a power bank, and its manual recommends UNI-T's own adapter.

| Model | What it has, by UNI-T's datasheet and manual | Rough cost | Find it |
|---|---|---|---|
| UNI-T UTG932E | Two outputs; sine waves to 30 MHz and square waves to 15 MHz; amplitude up to 20 V peak to peak into High-Z, and DC offset up to ±10 V, each set on its own; a Load setting, High-Z unless you change it; a 50 Ω output that drives 10 V peak to peak into 50 Ω, up to 100 mA, far more than 5 mA; power 5 V DC at 2 A, under 10 W. The UTG962E is the same with sine waves to 60 MHz | US$110–150 | [Maker](https://instruments.uni-trend.com/products/waveform-generators/UTG900E) · [Amazon](https://www.amazon.com/s?k=UNI-T+UTG932E) |

Many handheld scopes now include a small generator, and the syllabus
allows one that meets this list. None checked does by its maker's
published specifications. OWON's HDS200 and HDS300 series stop at 2.5 V:
their offset into High-Z is at most 2.5 V less half the peak to peak.
Hantek's 2D42 and 2D72 reach 5 V peak to peak, but Hantek publishes no
offset range for them; its TO1000 tablets give 6 V peak to peak and a
±3 V offset without saying how far the two combine. Joy-IT's JT-JDS2915
is the only generator found whose maker documents running it from a
power bank, but Joy-IT publishes no offset range for it, and its
JT-JDS6600 has the offset range but nothing on power banks.

**Powering the generator.** Run it from a [USB power bank](#power-banks)
that gives 5 V at 2 A, never from its mains adapter, and leave its USB
data port unconnected, so its output floats. UNI-T does not publish the
size of its power socket: if its own power lead does not end in a USB-A
plug, use a USB-A lead with a barrel plug that fits the socket. Keep its
output off the Mega's 5 V rail and every Mega pin. Before each
investigation, do the primer's
[output check](electricity/skills.md#check-the-output-first): this
generator can make far more than 4 V, and set for a 50 Ω load it puts
out about twice what it shows.

## Other tools {#tools}

### Stopwatch {#stopwatch}

E08 times a capacitor charging, and E21 counts a clock's blinks. The
stopwatch in any phone's clock app is enough.

### Small screwdriver {#screwdriver}

The relay module's screw terminals, in Lessons 35 and 53, need a small
flat blade, about 2.5 mm wide.

| Model | Notes, by its maker | Rough cost | Find it |
|---|---|---|---|
| Wera 2035, 0.40 × 2.5 × 80 mm (05118008001) | One slotted electronics screwdriver with a 2.5 mm blade | US$8–12 | [Maker](https://hybris-media.wera.de/download/pdfgenerator-datasheets/en/05118008001.pdf) · [Amazon](https://www.amazon.com/s?k=Wera+2035+0.4+x+2.5) |
| iFixit Moray Driver Kit | A handle and 32 bits, among them 1.0, 2.5 and 4.0 mm flat blades, for other small screws too | US$20 | [Maker](https://www.ifixit.com/products/moray-driver-kit) |

### Soldering iron, stand and solder {#soldering}

A few add-on boards arrive with their header pins loose. Buy them with
pins fitted if you can, or ask someone experienced, and read
[Soldering](safety.md#soldering) first.

**Must have:** temperature control, a stand that holds the hot iron, and
lead-free solder.

| Model | Notes, by its maker | Rough cost | Find it |
|---|---|---|---|
| Pinecil V2 | A small iron set from 100 to 400 °C, which stands by when not in use; powered by USB-C PD at 12–20 V, 3 A, or a 12–24 V barrel plug, so it works in any country. Add Pine64's stand, and a USB-C PD charger that gives 20 V at 3 A | US$26, plus US$2 for the stand and a charger | [Maker](https://pine64.com/product/pinecil-smart-mini-portable-soldering-iron/) · [Stand](https://pine64.com/product/pinecil-portable-mini-stand/) · [Amazon](https://www.amazon.com/s?k=Pinecil+V2) |
| Hakko FX-888DX | A soldering station for a classroom: 50 to 480 °C under closed-loop control, for US 120 V mains; its iron holder comes with it | US$120 | [Maker](https://hakkousa.com/fx-888dx.html) · [Amazon](https://www.amazon.com/s?k=Hakko+FX-888DX) |
| MG Chemicals 4900-18G | Lead-free SAC305 solder (tin, silver and copper), 0.81 mm, with no-clean flux; 21 g, plenty for a few headers | US$10–20 | [Maker](https://www.mgchemicals.com/products/solder/non-leaded/sn96-4900/) · [Amazon](https://www.amazon.com/s?k=MG+Chemicals+4900-18G) |

### USB power banks {#power-banks}

A power bank runs the signal generator, so that its output floats, and
lets a two-board lesson's second board sit in another room. Many banks
switch themselves off when their load draws less than somewhere between
50 and 200 mA, to save their battery once a phone is charged, and a Mega
draws less than that. Some banks' low-current modes still turn off after
a couple of hours.

**Must have:** a USB-A port that stays on at a small load, and 5 V at
2 A from that port for the generator.

| Model | Notes, by its maker | Rough cost | Find it |
|---|---|---|---|
| Voltaic Systems V25 | 6700 mAh (24 Wh); two USB-A ports, each 5 V at 2 A, 3 A together. Voltaic: "The USB-A output will not shut off even if your device is drawing low or no power. There is no low current cutoff." | US$45 | [Maker](https://voltaicsystems.com/v25/) · [Amazon](https://www.amazon.com/s?k=Voltaic+Systems+V25) |

For the boards, a USB wall charger you already have, 5 V at 1 A or
more, does as well wherever a socket is near, since it never switches
off; never for the generator, which must float.
