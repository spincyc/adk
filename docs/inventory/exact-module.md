# Exact module inventory

Copy this file to a dated specimen record before selecting an adapter or
applying power. One record describes one physical specimen, not a seller
family. Unknown fields remain `unknown`; they are not inferred from appearance.

## Record identity

| Field | Recorded value |
|---|---|
| Record date and recorder | |
| Kit vendor, SKU, and advertised revision | |
| Purchase source and date | |
| Specimen ID | |
| Front and back photograph paths | |
| PCB text, revision, and seller aliases | |
| Active-device markings | |
| Primary device datasheet | |
| Module schematic or traced-circuit record | |

Aliases are discovery metadata only. A `KY-` label, color, pin count, or seller
lesson number does not establish electrical identity and does not appear in a
C++ type name.

## Connector and electrical identity

| Field | Recorded value |
|---|---|
| Connector labels and physical order | |
| Supply range and polarity | |
| Quiescent and peak current | |
| Input absolute and operating limits | |
| Output type, level, polarity, and inactive state | |
| Onboard regulator and rail | |
| Pull-ups and their rail | |
| Indicator/loading behavior | |
| Protection, clamp, or flyback path | |
| Address straps, bus address, or configuration | |
| External-power and backfeed behavior | |

For a PCF8574 LCD backpack, also record the expander suffix, address straps,
expander-to-LCD bit map, I2C pull-up rail, backlight transistor and polarity,
LCD controller marking, and contrast circuit. For a breadboard power board,
record input connector polarity, regulator markings, rail-selection jumpers,
USB/barrel priority, reverse-current behavior, loaded rail voltage, current
limit, and temperature. Do not energize either from a familiar-looking pinout.

## Behavior and admission

| Field | Recorded value |
|---|---|
| Canonical behavior family | |
| Candidate ADK interface | |
| Planned behavior or existing host model | |
| Exact physical adapter status | |
| Energy class and hazard flags | |
| Circuit-native evidence channel | |
| Construction-inert and shutdown state | |
| Faults to inject | |
| Explicit non-claims and exclusions | |

Gas, smoke, and air-quality aliases do not establish a chemical species,
concentration range, calibration, heater duty, or alarm function. Record them
as unidentified analog/comparator behavior until a separately reviewed
gas-response experiment supplies the exact sensor, controlled harmless
stimulus, ventilation, disposal, and claim boundary. ADK does not use such a
module for life safety.

## Unpowered inspection and bench gate

- [ ] Photographs and markings agree with this record.
- [ ] Pin order and power path were traced with all power removed.
- [ ] Primary ratings cover the complete module circuit.
- [ ] Current-limited first-power limits and stop conditions are written.
- [ ] Resource acquisition and rollback evidence is separate from safe-state
      evidence.
- [ ] Startup, active, fault, reset, shutdown, and power-removal predictions
      are written.
- [ ] The lesson hardware-acceptance card names this specimen ID.

Until every applicable item passes with recorded measurements, describe the
adapter as planned or host verified with bench acceptance open, never
hardware verified or supported.
