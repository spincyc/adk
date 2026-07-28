# Unpowered specimen capture

This conditional procedure qualifies a selected physical revision when the
official listing and primary documentation do not settle its electrical
identity. It is not required to establish the authorized product-family set.
It does not admit a circuit, establish an electrical rating, or count as
hardware acceptance.

## Prepare

- Work on one specimen at a time on a dry, nonconductive surface.
- Disconnect USB, barrel leads, batteries, supplies, loads, antennas, and
  jumper wires.
- Assign a neutral specimen ID. Do not use a seller alias as identity.
- Quarantine damaged, leaking, swollen, corroded, heat-damaged, mains-exposed,
  laser, heater, ignition, or unknown stored-energy hardware.
- Keep faces, addresses, credentials, geolocation, receipts, and unrelated
  serial numbers out of photographs.

Do not connect a Mega, meter, programmer, or logic analyzer during the
photograph pass.

## Required photographs

1. Complete specimen beside its ID card and a metric ruler.
2. Straight-on, original-resolution front and back images.
3. Every connector edge, showing physical order, keying, and labels.
4. Every active-device marking, including suffixes, logos, and line breaks.
5. PCB model/revision, crystal, regulator, driver, optocoupler, and address
   straps.
6. Pin-one, diode/capacitor, battery, LED, jumper, and DIP polarity details.
7. Power connectors, fuses, regulators, protection, flyback parts, isolation
   slots, pull-ups, and load terminals.
8. Any ambiguity, rework, missing part, corrosion, or contradictory silkscreen.

Keep an unedited original. Suggested names:

```text
<specimen-id>_<yyyy-mm-dd>_<front|back|edge-n|marking-n|connector-n|damage-n>.<ext>
```

## Transcribe

Copy text verbatim and write uncertain characters as `[?]`. Record connector
order relative to a named photograph. Keep seller names, `KY-` labels, PCB
color, and bundle position as aliases only.

Until primary-source matching proves them, record device identity, pin
function, supply, polarity, current, logic levels, rails, output type,
protection, isolation, backfeed behavior, address, and safe load as `unknown`.
Photographs never prove a maximum rating.

First energization is a separate reviewed bench procedure performed only after
identity, full circuit ratings, current limit, protection, test points, stop
conditions, and physical disconnect are known.
