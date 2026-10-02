# What to buy

Everything the course uses beyond [the kit](kit.md): each path's shopping
list, then what each instrument, tool and extra part must do. Read each
purchasing gate before ordering: some routes still have unresolved parts
or supply requirements.

Prices are rough US-dollar planning allowances, not complete quotes;
maker prices and specifications were rechecked on 1 October 2026 where
available. Confirm stock, regional versions and the delivered accessories;
tax and postage are extra. **ADK sells nothing and earns nothing
from any shop.** Links identify the maker and, where useful, a shop or
distributor search. Outside the US, search your country's shops for the
same model name or part number. Each
model's electrical claims use its maker's pages, manuals and datasheets;
a shop's listing establishes price or availability, not compatibility.
The unresolved requirements below are part of the shopping list. None has
yet been used in a [recorded build](builds.md), so dimensional and
electrical specifications do not establish physical fit or measured results.

## Shopping lists {#lists}

### Project path, Lessons 1 to 36

| Qty | Item | Rough cost |
|---|---|---|
| 1 | [Elegoo Mega 2560 Most Complete Starter Kit](https://us.elegoo.com/products/elegoo-mega-2560-the-most-complete-starter-kit): Mega, USB cable, breadboard, parts, power module, 9 V adapter and battery; see the kit checks below | US$55–70 |
| 1 | Elegoo 37 in 1 Sensor Modules Kit, for the modules marked *(37 in 1)* in [the parts](kit.md#the-parts); confirm stock and pin labels | About US$60 for V3; presently sold out at the maker |
| 1 | BSS138 I2C level shifter on the Adafruit 757 layout, for Lessons 28, 30 and 50: one with its headers fitted if you can find it, as Adafruit's own comes with them loose, to [solder](#soldering) | US$4–8 |
| 1 | [Small screwdriver](#screwdriver) | US$8–20 |
| 1 | [BusBoard PW-9VX 9 V battery snap](https://busboard.com/PW-9VX), with two breadboard pins, if the kit has only its barrel-ended snap; for Lessons 35 and 53 | About US$3 |
| 1 | [Digital multimeter](#multimeter), for *Measure it* (optional) | US$20–30 |
| | **About, with the optional meter** | **US$150–190**, before missing kit parts or soldering tools |

**Kit checks.** Elegoo's current
[Mega manual](https://m.media-amazon.com/images/I/D1oC-c3G5TS.pdf) lists a
DS3231 clock module and a battery snap ending in a DC barrel plug.
The course draws a DS1307-style module: ADK supports both clock chips,
but check the module's printed pins and coin-cell charging circuit before
following Lesson 32. A barrel-ended battery lead cannot plug into the
relay's breadboard circuit; the PW-9VX above has separate male pins.
It uses 15 cm, 26 AWG leads, as specified by BusBoard.

The [V3 sensor kit](https://us.elegoo.com/products/elegoo-37-in-1-sensor-modules-kit-v3)
and [older kit](https://us.elegoo.com/products/elegoo-37-in-1-sensor-kit)
were shown sold out by Elegoo at this check. Obtain the
exact modules and confirm their printed pin order before buying the full
project path; the price total is not evidence that a complete kit is in stock.

### Radio arcs, Lessons 37 to 42

| Qty | Item | Rough cost |
|---|---|---|
| 1 set | QIACHIP 433 MHz set, two RYLR896, two E32 with aerials, two Heltec V3; all subject to the gates below | US$90–95 |
| | Without Lesson 41's E32 modules | US$75–80 |
| | Without Lesson 42's Heltec boards | About US$55 |
| | Without either | About US$40 |
| 1 set | Exact CJMCU-470 FM board and wired 3.5 mm earbuds; variant still to verify | Quote needed |
| 2 each | USB-C data cables and 5 V supplies of at least 600 mA for Heltec, if not already owned | Quote needed |

Some add-on boards arrive with their header pins loose, to be
[soldered](#soldering).

### Radio purchasing gates {#radio-purchasing-gates}

The subtotals above use current maker or maker-fulfilled prices for the
named hardware. They exclude the FM board, cables, supplies and soldering
tools. They do not establish a permitted transmitting configuration:
[Safety](safety.md#radios) separates timing, emissions and authorization.

| Course part | Source-supported specification and accessories | Resolve before purchase or power |
|---|---|---|
| CJMCU-470 FM board | Lesson 37 requires the eight-pin 3.3 V board, fitted headers and wired 3.5 mm earbuds. [SparkFun's Si4703 guide](https://learn.sparkfun.com/tutorials/si4703-fm-radio-receiver-hookup-guide/hardware-overview) documents its own board, whose revisions can swap supply pins. | An identifiable maker schematic, current requirement and exact pinout for the CJMCU board are missing. SparkFun's board documentation does not establish the clone's wiring; no exact replacement is established here. |
| QIACHIP WL102-341 / RX470-4 set | [Maker](https://qiachip.com/collections/all-products-1/products/qiachip-433mhz-superheterodyne-rf-receiver-and-transmitter-module): about US$5.40; 433.92 MHz ASK/OOK, TX supply 2–3.6 V, RX 2.2–5 V, fitted four-pin headers and two aerials. Check RX VCC/DO/DO/GND and TX EN/DAT/+/−. | Maker-reported TX power exceeds 11 dBm. No applicable authorization or measured radiated limit for the lesson build is established, with or without an aerial. |
| REYAX RYLR896 pair | [Maker](https://reyax.com/product/LoRa/RYLR896) and [datasheet](https://reyax.com/upload/products_download/download_file/RYLR896_EN.pdf): 2–3.6 V supply, 3.3 V UART, six pins, integrated aerial. [Maker-fulfilled distributor listing](https://www.digikey.com/en/products/detail/reyax/RYLR896/22145027): about US$16 each. | Confirm authorization for the band, aerial and modulation settings. The complete-module maximum current at the two-board lessons' 10 dBm is missing: 49.7 mA at 14 dBm is typical, not a maximum, and does not establish the Mega's 50 mA supply budget. |
| Ebyte E32-433T20D pair | [Maker shop](https://ebyteiot.com/products/5pcs-sx1278-lora-uart-rf-module-433mhz-20dbm-long-range-3km-transceiver-transmitter-receiver-sma-k-antenna-e32-433t20d-v8-x) offers a current V8 pair with TX433-JKD-20P aerials for about US$16. The board has seven pins, 3.3 V I/O, a supply accepting the lesson's 5 V, and SMA-K socket. | ADK targets the legacy SX1278 six-byte C0 configuration interface documented in Ebyte manual v1.0 (2022-05-12). Current AT-capable revisions are not validated replacements; obtain the exact legacy revision or wait for verification. Confirm the aerial and permitted operating arrangement too. |
| Heltec WiFi LoRa 32 V3 pair | [Maker](https://heltec.org/project/wifi-lora-32-v3/): US$17.90–19.90 each; regional 863–870 or 902–928 MHz option, included aerial and loose headers, USB-C, IPEX/U.FL aerial socket. [Meshtastic supports V3](https://meshtastic.org/docs/hardware/devices/heltec-automation/lora32/). | Add two USB-C data cables and two 5 V supplies rated at least 600 mA. Fit headers to the board connected to the Mega. Confirm regional hardware and authorization for its aerial; attach the snap-on pigtail before power. |

Ebyte's [linked FCC grant](https://www.cdebyte.com/pdf-down.aspx?id=2783)
covers 903–927 MHz, so it does not establish authorization for the
433 MHz model. Its offered [TX433-JKD-20P aerial](https://www.cdebyte.com/products/TX433-JKD-20P)
is rated 4 dBi: setting 10 dBm conducted power does not by itself establish
the EU's 10 mW ERP limit. The
[current E32 manual](https://www.cdebyte.com/pdf-down.aspx?id=4218)
also describes changed AUX modes and inconsistent channel-frequency
figures. A shared model name is not enough to establish compatibility.

### Two boards, Lessons 43 to 55

Besides the pair of RYLR896 modems from Lesson 40:

**Supply gate:** resolve the RYLR896 current budget in the
[radio table](#radio-purchasing-gates) before powering a modem from the
Mega's 3.3 V pin. A second kit does not resolve that missing specification.

| Qty | Item | Rough cost |
|---|---|---|
| 1 | A second Elegoo Mega 2560 Most Complete Starter Kit, which covers all of Board B's parts | US$55–70 |
| | *or* a second Mega 2560, USB cable, breadboard and jumpers, a second power module **and its matching 9 V adapter** for Lesson 51, and a second MAX7219 matrix, joystick and passive buzzer for Lessons 54 and 55 | Separate quote needed; no complete bundle verified |
| 1–2 | [USB power banks](#power-banks) that stay on at a small load, USB wall chargers you already have, or a long USB cable, to put the boards in different rooms | US$0–90 |

### Electricity path, E01 to E24

| Qty | Item | Rough cost |
|---|---|---|
| 1 | Elegoo Mega 2560 Most Complete Starter Kit, the same as the project path's | US$55–70 |
| 1 | [Digital multimeter](#multimeter) with DC volts, from E02 on | US$20–30 |
| 1 set | The [extra parts](#extra-parts) in each module's [equipment gate](electricity/index.md#equipment-gates): resistors, capacitors, a 100 mH inductor, an MCP6002, a 74HC00 and a 74HC14 | US$10–15, plus postage |
| | **About** | **US$85–115**, plus postage |

### Scope extension, for E11, E13 to E16 and E18

!!! warning "Generator connection still unresolved"
    This is a provisional budget, not a complete shopping route. Before
    buying, resolve the [generator power lead](#generator): the socket's
    dimensions and a matching, rated lead are not established below.
    A 5 V / 2 A label alone is not enough to choose a plug.

| Qty | Item | Rough cost |
|---|---|---|
| 1 | A battery-powered two-channel [oscilloscope](#oscilloscope), with two probes | US$300–420 |
| 1 | The candidate [signal generator](#generator), pending its power connection | US$187 at UNI-T US |
| 1 | A [USB power bank](#power-banks) that stays on at a small load, to run the generator | US$45 |
| 1 | A manufacturer-confirmed USB power lead for that generator; confirm whether supplied | Unresolved; no substitute lead priced |
| | **Provisional subtotal** | **US$535–655**, excluding any extra power lead |

## Instruments and tools at a glance {#at-a-glance}

| Item | Needed for | Must have | Rough cost |
|---|---|---|---|
| [Digital multimeter](#multimeter) | *Measure it* in most project lessons, which you can skip without one; the electricity path from E02 on | DC volts from tens of millivolts to 9 V; about 10 MΩ input | US$20–30; a school meter about US$230 |
| [Extra parts](#extra-parts) | The electricity path's equipment gates | The values and ratings in the gates | US$10–15 |
| [Oscilloscope](#oscilloscope) | E11, E13–E16 and E18; optional in E12, E17 and E23 | Its own battery; two channels and two probes; 50 mV per division; AC coupling or a vertical offset; a Single trigger | US$300–420 |
| [Signal generator](#generator) | The same investigations | Power from a power bank; amplitude and offset set together to give 0–4 V; a High-Z setting; at least 5 mA | Candidate US$187; power lead unresolved |
| [Stopwatch](#stopwatch) | E08 and E21 | Seconds and tenths | A phone's clock |
| [Small screwdriver](#screwdriver) | The relay's terminals, in Lessons 35 and 53 | A flat blade about 2.5 mm wide | US$8–20 |
| [Soldering iron](#soldering), supply, cable, stand and lead-free solder | Header pins on add-on boards that arrive without them: the level shifter (Lesson 28), the FM radio board (Lesson 37), some 433 MHz modules (Lesson 38) and a Heltec board (Lesson 42) | Temperature control and a stand | About US$70–145, with a solder allowance |
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
| UNI-T UT61E+ | 220 mV, reading 0.01 mV | For more digits: 22 000 counts; about 1 GΩ on DC mV and 10 MΩ on other DC voltage ranges | US$80–110 | [Maker](https://meters.uni-trend.com/product/ut61plus-series/) · [Amazon](https://www.amazon.com/s?k=UNI-T+UT61E%2B) |
| Fluke 114 | 600 mV, reading 0.1 mV | A school meter: no current function at all, so nothing to set wrong; over 10 MΩ input on V⎓; a three-year warranty. Use its V⎓ or mV⎓ setting, not Auto-V/LoZ, which is about 3 kΩ and would load a circuit | US$230 | [Maker](https://www.fluke.com/en-us/product/electrical-testing/digital-multimeters/fluke-114) · [Amazon](https://www.amazon.com/s?k=Fluke+114) |

The manuals confirm these ranges and input loading:
[UT33A+, technical specifications](https://meters.uni-trend.com/download/ut33a-b-c-d-user-manual/),
[UT61E+, DC voltage table](https://meters.uni-trend.com/download/ut61b-d-e-user-manual/)
and [Fluke 114, specifications](https://media.fluke.com/5e1db354-3a6e-49cf-9edd-b10800c0e608_original%20file.pdf).
Each includes voltage test leads and batteries. The
[oscilloscope](#oscilloscope) below has a meter that also meets this list;
you do not need both. Obtain a current quote: these meter prices are
allowances, and UNI-T's global pages do not establish dealer stock.

## Extra parts for the electricity path {#extra-parts}

The kit covers E01 to E24 except for these, which each module's
[equipment gate](electricity/index.md#equipment-gates) lists. Any part
with the same value and at least the same rating will do: if your kit's
capacitors include a 10 µF, a 100 µF or a 100 nF (marked 104) rated at
least 10 V, use them. The part numbers below are ones whose makers'
datasheets establish the electrical ratings and dimensions, with long
leads or a pitch close to the drawn holes. Bend leads gently away from
the body; never force thick leads into a breadboard. Check distributor
stock and the complete order cost; the US$10–15 is an allowance.

| Part | For | Part number | Its datasheet gives | Qty | Find it |
|---|---|---|---|---|---|
| 10 Ω resistor | E05, E18 | Stackpole RNF14FTD10R0 | ¼ W metal film, 1 % | 2 | [Maker](https://www.seielect.com/catalog/sei-rnf_rnmf.pdf) · [DigiKey](https://www.digikey.com/en/products/result?keywords=RNF14FTD10R0) |
| 100 kΩ resistor | E21 | Stackpole RNF14FTD100K | ¼ W metal film, 1 % | 2 | [Maker](https://www.seielect.com/catalog/sei-rnf_rnmf.pdf) · [DigiKey](https://www.digikey.com/en/products/result?keywords=RNF14FTD100K) |
| 1000 µF electrolytic, at least 10 V | E07–E08 | Nichicon UVR1C102MPD | 16 V; 10 × 16 mm, legs 5 mm apart | 1 | [Maker](https://www.nichicon.co.jp/english/series_items/catalog_pdf/e-uvr.pdf) · [DigiKey](https://www.digikey.com/en/products/result?keywords=UVR1C102MPD) |
| 100 µF electrolytic, at least 10 V | E18, E23 | Nichicon UVR1H101MPD | 50 V; 8 × 11.5 mm, legs 3.5 mm apart | 1 | [Maker](https://www.nichicon.co.jp/english/series_items/catalog_pdf/e-uvr.pdf) · [DigiKey](https://www.digikey.com/en/products/result?keywords=UVR1H101MPD) |
| 10 µF electrolytic, at least 10 V | E16, E21 | Rubycon 50YXJ10M5X11 | 50 V, ±20 %; 5 × 11 mm, legs 2 mm apart | 2 | [Maker](https://www.rubycon.co.jp/wp-content/uploads/catalog-aluminum/YXJ.pdf) · [DigiKey](https://www.digikey.com/en/products/result?keywords=50YXJ10M5X11) |
| 1 µF film capacitor, nonpolar | E13–E15 | WIMA MKS4C041003C00KSSD | Polyester, 63 V, ±10 %; 10 mm lead pitch, close to the drawing's 10.16 mm gap; 4 × 9 × 13 mm body | 1 | [Maker](https://www.wima.de/wp-content/uploads/media/e_WIMA_MKS_4.pdf) · [Mouser](https://www.mouser.com/c/?q=MKS4C041003C00KSSD) |
| 100 nF ceramic capacitor | E16, E18, E19–E21 | Vishay K104K15X7RF53L2 | 50 V, X7R; legs 2.5 mm apart | 2 | [Maker](https://www.vishay.com/en/product/45171/) · [DigiKey](https://www.digikey.com/en/products/result?keywords=K104K15X7RF53L2) |
| 100 mH inductor, at least 10 mA, under about 500 Ω | E11 | Murata 19R107C | 100 mH ±10 %; up to 70 mA; at most 90 Ω | 1 | [Maker](https://pim.murata.com/en-us/pim/details/?partNum=19R107C) · [DigiKey](https://www.digikey.com/en/products/result?keywords=19R107C) |
| MCP6002 op-amp, 8-pin DIP | E16–E17 | Microchip MCP6002-I/P | Runs from 1.8 V to 6 V; rail-to-rail input and output, with output headroom depending on load | 1 | [Maker](https://ww1.microchip.com/downloads/aemDocuments/documents/MSLD/ProductDocuments/DataSheets/MCP6001-1R-1U-2-4-1-MHz-Low-Power-Op-Amp-DS20001733L.pdf) · [DigiKey](https://www.digikey.com/en/products/result?keywords=MCP6002-I%2FP) |
| 74HC00 NAND gates, 14-pin DIP | E19–E20 | Texas Instruments SN74HC00N | Runs from 2 V to 6 V | 1 | [Maker](https://www.ti.com/product/SN74HC00) · [DigiKey](https://www.digikey.com/en/products/result?keywords=SN74HC00N) |
| 74HC14 Schmitt inverters, 14-pin DIP | E21 | Texas Instruments SN74HC14N | Runs from 2 V to 6 V; at 4.5 V, switches up between 1.55 V and 3.13 V | 1 | [Maker](https://www.ti.com/product/SN74HC14) · [DigiKey](https://www.digikey.com/en/products/result?keywords=SN74HC14N) |

E11's prediction rests on the inductor's own resistance: with the
19R107C's 90 Ω at most and a generator's 50 Ω, the resistor's voltage
levels at about 3.5–3.8 V and rises with a timescale of about 90 µs,
at nominal inductance; its ±10% tolerance can put the time constant
outside the lesson's nominal range. Its 0.8 mm leads and 6 mm pitch need
checking against the breadboard and the drawing's 10.16 mm gap. The
[Murata catalog](https://www.murata.com/-/media/webrenewal/support/library/catalog/products/power-products/power-magnetics.ashx?la=en-gb)
does not establish that the leads can be formed to those holes with
enough insertion depth. Mechanical fit remains a purchasing gate;
do not force the part into place.

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
- **At least 1 MS/s** (a million samples a second) with both channels on.
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

UNI-T's [manual](https://unitrend.oss-cn-hongkong.aliyuncs.com/uploads/attach/20250624/1101440aab45413202954e0d05344.pdf)
lists two probes for the **DL** model, a battery, charging adapter and
meter leads; the **CL** has only one channel. Check that the quote
includes those accessories. Use the probes at ×10 and set each channel
to match; the display then accounts for their attenuation. Compensate
each probe as the manual describes. Unplug the charger and USB data
cable during measurements. Its grounds are common, as the
[primer](electricity/skills.md#scope-and-generator) explains.

### The signal generator {#generator}

**Must have:**

- **Power from a USB power bank or a battery**, with the charger and
  USB data cable unplugged, to avoid an earth path through the instrument's
  supply. Its GND still joins the circuit's common GND.
- **Sine and square waves** from 100 Hz to 1 kHz.
- **Amplitude and DC offset, each set on its own:** 4 V peak to peak
  around +2 V, a wave from 0 V to 4 V that never goes below 0 V, for
  E11, E13 to E15 and E18; 1 V peak to peak around +1 V for E16's
  0.5–1.5 V.
- **A High-Z load setting**, so the voltage it shows is the voltage it
  makes into these circuits, which draw very little.
- **At least 5 mA** of output: E11 and E13 draw about 4 mA through
  1 kΩ.

**Purchasing gate: the power connection is still incomplete.** The
candidate below has the required signal range. UNI-T's current
[US page](https://uni-trendus.com/products/utg932e) permits a 5 V / 2 A
USB source and lists a USB-to-DC power cable. Its separate accessory list
names only a USB data cable, however, and the
[manual, page 8](https://unitrend.oss-cn-hongkong.aliyuncs.com/uploads/attach/20250624/10092357fe28aabb6c2d6cc206583.pdf)
recommends the supplied adapter. Neither identifies the barrel plug's
outer diameter, inner diameter and insertion length. Obtain confirmation
of those dimensions, the centre-positive polarity shown in the manual's
socket illustration, and the exact supplied or replacement power lead
with a rating of at least 2 A. The proposed V25/lead/generator combination
has no recorded bench test. Do not guess a plug from its appearance.

| Model | What it has, by UNI-T's datasheet and manual | Rough cost | Find it |
|---|---|---|---|
| UNI-T UTG932E, candidate | Two outputs; sine to 30 MHz, square to 15 MHz; 50 Ω source impedance, High-Z load setting; up to 20 Vpp and ±10 V offset in High-Z, with amplitude and offset sharing the output range. A 0–4 V wave is within that range. Rated 10 Vpp into 50 Ω implies 100 mA peak for a centred sine, above the course's 5 mA need. Supply: 5 V / 2 A, under 10 W | US$187; power lead to confirm | [Maker](https://instruments.uni-trend.com/products/waveform-generators/UTG900E) · [US maker shop](https://uni-trendus.com/products/utg932e) |

Some handheld scopes include a generator. It must produce **0–4 V**,
including the +2 V offset at 4 Vpp; quoting its maximum Vpp alone does
not establish that. No checked combined instrument has been established
to meet the complete list. For example, the
[OWON HDS200 page](https://www.owon.com.hk/products_owon_hds200_series_digital_oscilloscope)
gives 5 Vpp but does not establish the required simultaneous offset.
[Joy-IT's JT-JDS2915](https://joy-it.net/en/products/JT-JDS2915) documents
power-bank operation, but its linked datasheet does not specify the
offset range. Neither is a complete substitute based on those figures.

Once the power connection is documented, check the lead's disconnected
plug with the meter on DC volts: black on the outer sleeve and red at
the centre must read about **+5 V**, with the probe tips kept apart.
This checks voltage and polarity, not current capacity or fit. Run the
generator from the charged bank with the bank's charger unplugged;
leave the generator's USB data port unplugged too. Use its supplied
**UT-L02 BNC-to-clip signal lead** to grip two separate male jumper
ends, OUT and GND, whose other ends go into the specified holes. Keep
OUT off every Mega pin and every + rail. Follow the
[output check](electricity/skills.md#check-the-output-first), resetting
amplitude and offset after selecting High-Z. The generator can produce
far more than 4 V.

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
| Pinecil V2 | 100–400 °C; USB-C PD 12–20 V, 3 A. The iron and tip need the charger, cable and stand listed below | US$25.99 community price | [Maker](https://pine64.com/product/pinecil-smart-mini-portable-soldering-iron/) |
| PinePower 65 W GaN 2C1A charger | 100–240 V AC input, US plug and AU/EU/UK adapters; one USB-C port supplies 20 V at 3.25 A. Leave its other ports unused while soldering | US$24.99 | [Maker](https://pine64.com/product/pinepower-65w-gan-2c1a-charger-with-international-plugs/) |
| Pine64 1.5 m silicone USB-C to USB-C cable | The maker's flexible power cable for its iron | US$3.99 | [Maker](https://pine64.com/product/usb-type-c-to-usb-type-c-silicone-power-charging-cable-1-5-meter-length/) |
| Pinecil portable mini stand | Holds the hot iron; cleaning sponge included | US$1.99 | [Maker](https://pine64.com/product/pinecil-portable-mini-stand/) |
| Hakko FX-888DX | A soldering station for a classroom: 50 to 480 °C under closed-loop control, for US 120 V mains; its iron holder comes with it | US$120 | [Maker](https://hakkousa.com/fx-888dx.html) · [Amazon](https://www.amazon.com/s?k=Hakko+FX-888DX) |
| MG Chemicals 4900-18G | Lead-free SAC305 solder (tin, silver and copper), 0.81 mm, no-clean flux; 21 g. Maker restricts its sale in the EU and UK: obtain a locally available electronics solder meeting the same needs there | US$10–20 allowance | [Maker](https://mgchemicals.com/products/soldering-supplies/solder-wire/lead-free-solder/) · [Amazon](https://www.amazon.com/s?k=MG+Chemicals+4900-18G) |

The four-item Pinecil route totals **US$56.96 before solder and postage**
at the listed community prices. The Hakko alternative includes its iron,
tip, holder, sponge and cleaning wire; this US model requires 120 V.
Borrowing a complete setup or having the headers fitted avoids buying
these tools. The EU/UK solder alternative is still to be selected and priced.

### USB power banks {#power-banks}

A power bank avoids a mains-earth path through the generator's supply, and
lets a two-board lesson's second board sit in another room. Many banks
switch themselves off when their load draws less than somewhere between
50 and 200 mA, to save their battery once a phone is charged, and a Mega
draws less than that. Some banks' low-current modes still turn off after
a couple of hours.

**Must have:** a USB-A port that stays on at a small load, and 5 V at
2 A from that port for the generator.

| Model | Notes, by its maker | Rough cost | Find it |
|---|---|---|---|
| Voltaic Systems V25 | Two USB-A ports, each 5 V at 2 A, 3 A together; Always On has no low-current cutoff. Do not use USB-C for input or output while relying on Always On | US$45 | [Maker](https://voltaicsystems.com/v25/) · [Amazon](https://www.amazon.com/s?k=Voltaic+Systems+V25) |

For the boards, a USB wall charger you already have, 5 V at 1 A or
more, is another option wherever a socket is near; check that it stays
on with the board's load. It is not the generator's supply for these
investigations: the generator needs the battery arrangement above.
