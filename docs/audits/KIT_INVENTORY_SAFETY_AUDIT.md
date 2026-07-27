# Kit inventory source and safety audit

Date: 2026-07-27

This is an independent audit addendum. It does not establish that any part is
owned, supported, safe, or bench-tested.

## Inventory finding

"The full set of Arduino kits" is not a reproducible bill of materials.
Official and third-party Arduino-branded store listings change, and similarly
named kits contain different boards and modules. For example, the current
official Starter Kit R4 contains an Uno R4 WiFi, while this project targets the
Mega 2560. The older multilingual Starter Kit lists specific parts such as an
L293D, IRF520, 4N35, PKM22EPP-40 piezo, small motor, and servo. Neither listing
proves the identity of a part in the user's collection.

Create an owned-inventory record before selecting hardware:

- kit manufacturer, exact product name, SKU, revision, and purchase era;
- board/module markings and clear front/back photographs;
- component manufacturer and complete part number;
- connector pinout, logic voltage, supply range, current, and load ratings;
- link and retrieval date for the manufacturer datasheet;
- whether the specimen was inspected, electrically characterized, and accepted
  on the bench.

Do not infer a module pinout from wire color, a reseller photograph, or another
module with a similar name.

## Source-quality gate

Use sources in this order:

1. silicon/component manufacturer datasheet and errata;
2. board/module manufacturer schematic, datasheet, and revision history;
3. Arduino's board datasheet, schematic, or official product inventory;
4. standards owner, regulator, or statute for compliance and legal claims;
5. maintained vendor library documentation for software behavior.

Tutorials, distributor summaries, store copy, community posts, and search-result
snippets may aid discovery but cannot establish maximum ratings, pinouts,
protection, calibration, certification, or lawful radio use. Archive the exact
primary document revision used by a bench acceptance record when its license
permits redistribution; otherwise record its stable URL and document revision.

The word `supported` requires an identified specimen, primary sources, a
reviewed circuit, deterministic host tests, a Mega 2560 build, and recorded
hardware acceptance. Until then use `candidate`, `planned`, or `simulated`.

## Modules requiring special treatment

| Module or topic | Audit disposition | Minimum lesson boundary |
|---|---|---|
| Bare LED, button, potentiometer, passive piezo | Normally suitable at low voltage | Exact value/polarity, resistor calculation, current budget, test point, safe startup and shutdown |
| DHT11 or other climate sensor | Educational indication only | Exact model, voltage/pull-up, timing source, invalid/stale data, stated accuracy; no weather, health, storage, or safety claim |
| Photoresistor, phototransistor, flame detector | Relative optical experiment only | Ambient-light and geometry dependence; no fire detector, flame safeguard, security, or calibrated-lux claim |
| MQ-series gas or carbon-monoxide sensor | Do not use as a safety detector | Exact sensor, heater power, conditioning time, ventilation, calibration gas limitations, cross-sensitivity, and no life-safety conclusion; prefer synthetic fixtures |
| Sound or microphone module | Privacy-sensitive | Local controlled sounds, visible capture indicator, no speech recording or covert observation, no acoustic safety or calibrated-SPL claim |
| Heart-rate, skin, or body sensor | Not a medical device or diagnosis lesson | Consent, local data minimization, no clinical claim, no consequential decision; synthetic traces must be sufficient |
| PIR, camera, RFID/NFC, fingerprint module | Privacy/access-control sensitive | Consenting participants and owned tokens only, visible sensing indicator, local storage limits, deletion method; no real security credential or surveillance deployment |
| Soil-moisture, rain, steam, or water sensor | Corrosion and liquid hazard | Current-limited low-voltage bench use, electronics physically separated from liquid, dry hands before rewiring, no unattended irrigation or environmental calibration claim |
| Relay module | Inert low-voltage loads only | Exact revision, coil/input current, contact topology, isolation gaps, back-power behavior, flyback/protection, independent load-power removal; no mains |
| DC motor, vibration motor, fan, solenoid | Externally powered and restrained | Rated driver, stall-current measurement, current-limited supply, flyback strategy, thermal check, guarded mechanism, physical disconnect; never GPIO-powered |
| Servo or robot arm | Pinch/motion hazard | Separate rated supply, common-ground plan, bounded motion, restrained load, keep-clear zone, physical disconnect, no automatic restart |
| Ultrasonic or infrared range sensor | Indication only | Exact voltage/timing, minimum range and target dependence, timeout/stale data, oscilloscope/test-point evidence; never a sole collision-safety sensor |
| LCD, LED matrix, high-brightness LED, IR emitter, laser | Eye/thermal limits differ | Current and duty limits, viewing-distance rule, dim startup; lasers are excluded unless a separately reviewed low-power curriculum is approved |
| RTC with coin cell | Battery ingestion and polarity hazard | Installed holder, correct chemistry/polarity, no loose cells around children, no charging of primary cells |
| SD card or persistent storage | Data integrity and privacy | Power-loss/corruption tests, bounded writes, explicit erase/export behavior, no sensitive records by default |
| Bluetooth, Wi-Fi, LoRa, 315/433 MHz, NRF, cellular | Regulatory, authorization, and privacy sensitive | Exact certified module, region/band/power/antenna, lawful owned payload, credentials handling, visible transmit state; synthetic or receive-only work first |
| Infrared remote | Owned published protocol only | Harmless kit equipment, visible capture state, no access-control cloning; prerecorded synthetic vectors remain sufficient |
| USB host/device attachment | Host-security sensitive | Owned peripherals, explicit class allowlist and physical confirmation; no credential capture, unauthorized impersonation, or borrowed vendor ID |
| HDMI capture/switching | Licensing and content-protection sensitive | Self-generated unprotected patterns first; licensed HDMI/HDCP components for protected paths; no key extraction or protection bypass |
| 9 V battery clip, Li-ion/LiPo, USB-C PD, large supply | Power-source-specific review | Reverse-polarity, fuse/current limit, short-circuit and backfeed analysis; no loose lithium-cell curriculum without protected charger and cell documentation |
| Mains, ignition, fireworks launcher, unknown RF replay | Excluded | Simulation and inert indicators only |

The official Arduino store itself lists third-party sensor sets containing
relays, MQ7 carbon-monoxide sensors, flame sensors, body sensors, water/steam
sensors, motors, and other parts. Store inclusion is not evidence that an ADK
beginner lesson may use them without the controls above.

## Minimum module documentation

Every hardware-backed component and lesson must record:

1. **Identity:** markings, revision, photograph, primary datasheet, and date
   retrieved.
2. **Electrical contract:** logic and supply voltages, idle/peak/stall/heater
   current, source/sink direction, pull-ups, load limits, protection, and shared
   ground or isolation.
3. **Resource contract:** Mega pins, ADC reference, timer, interrupt, UART, I2C
   address, SPI chip select, memory, and timing conflicts.
4. **Safe lifecycle:** unpowered wiring inspection; acquire/configure/start;
   reset, disconnect, brownout, timeout, and shutdown behavior; independent
   power removal where motion or loads exist.
5. **Observation:** prediction, named test point or visible/audible indicator,
   expected range and timing, what the observation proves, and what it cannot
   prove.
6. **Validity:** operating range, uncertainty, warm-up/calibration, saturation,
   open/short detection, stale-data policy, and prohibited conclusions.
7. **Evidence status:** deterministic fixtures, Mega compilation, analyzer or
   meter evidence, exact bench specimen, result, operator, date, and commit.
8. **Privacy/security/regulatory limits:** data retained, consent and deletion,
   authorization, region/band, identifiers, trademarks/licenses, and excluded
   uses where applicable.

## Primary references checked

- [Arduino Mega 2560 Rev3 documentation](https://docs.arduino.cc/hardware/mega-2560)
- [Arduino Mega 2560 Rev3 datasheet](https://docs.arduino.cc/resources/datasheets/A000067-datasheet.pdf)
- [Arduino Starter Kit Multi-Language inventory](https://store.arduino.cc/collections/diy-challenge/products/arduino-starter-kit-multi-language)
- [Arduino Starter Kit R4 inventory](https://store.arduino.cc/products/starter-kit-r4)
- [Arduino Gravity 27-sensor set inventory](https://store.arduino.cc/products/gravity-27-pcs-sensor-set-for-arduino)
- [USB-IF compliance program](https://www.usb.org/compliance)
- [HDMI Adopter overview](https://www.hdmi.org/adopter/index)
- [Digital Content Protection licensing](https://www.digital-cp.com/licensing)
- [17 U.S.C. section 1201](https://uscode.house.gov/view.xhtml?edition=prelim&f=treesort&fq=true&hl=false&num=0&req=granuleid%3AUSC-2021-title17-section1201)

## Release blockers

- No canonical owned-kit inventory currently identifies exact specimens.
- A store page is presently being used as the strongest source for some kit
  inventories; individual module datasheets must replace it before support.
- Any lesson selecting a generic `relay`, `servo`, `motor driver`, `RF module`,
  `gas sensor`, or `water sensor` remains simulated until its exact revision and
  primary documentation are reviewed.
- Bench acceptance must not be inferred from Arduino compilation, host tests,
  diagrams, or possession of a nominally matching kit.
