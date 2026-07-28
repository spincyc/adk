# Lessons 037–039 sensor evidence report

Status: research evidence, 2026-07-28. This report narrows the contact and
acoustic specimen search. It does not identify the user's physical specimens,
authorize a circuit, close an E1 gate, or record bench acceptance.

## Evidence rules

- A kit name, retail photograph, tutorial family name, PCB color, or apparent
  layout is candidate evidence, not specimen identity.
- A component datasheet applies only after its marking and package have been
  matched to the specimen. A family datasheet does not establish which member
  or manufacturing revision is present.
- Conflicting revisions remain conflicts. Newer limits must not be projected
  backward onto a bundled older part.
- No source found here establishes calibrated sound-pressure level, loudness,
  frequency response, force, acceleration, impact energy, or damage.
- All remaining inspection below is unpowered. This report requests no
  energization and makes no physical-verification claim.

The repository baseline is
[`AUTHORIZED_ELEGOO_SET.md`](../inventory/AUTHORIZED_ELEGOO_SET.md). It
authorizes the documented product families but deliberately does not turn a
family name into an exact electrical identity.

## Source and revision matrix

| Source | Revision or identity | What it supports | What it does not support |
|---|---|---|---|
| [ELEGOO Mega tutorial landing page](https://www.elegoo.com/blogs/arduino-projects/elegoo-mega-2560-the-most-complete-starter-kit-tutorial) | Web landing page | ELEGOO publishes a tutorial/code archive for the Mega Most Complete Starter Kit | The identity or revision of a loose specimen |
| [ELEGOO Mega V1.0 archive](https://download.elegoo.com/01%20STEM%20Kits/02%20Mega%202560/Complete/ELEGOO%20The%20Most%20Complete%20Starter%20Kit%20for%20MEGA%20V1.0.2023.05.05.zip) | Filename `V1.0.2023.05.05`; downloaded archive SHA-256 `8bbe19fb701a4b550b3227bf6d6fb98d8125ce8455d01d850f715d912f3ee8a5` | A reproducible official tutorial snapshot containing Lesson 8 Ball Switch and Lesson 20 sound-module material | Proof that the user's specimen shipped with this archive or matches every pictured part |
| [ELEGOO download catalog](https://www.elegoo.com/pages/download) and [catalog path data](https://download.elegoo.com/ELEGOO_Website_Tutorial_Path/Path_01_STEM%20Kits.js) | Live catalog | Provenance of the official archive path | Immutable archive contents or specimen identity |
| Bundled `Lesson 8 Ball Switch.pdf` in the archive | The embedded photograph shows a black cylindrical bare switch marked `HDX HDX`, while the lesson's reproduced data are for the Light Country AT407/409/411/411-2/411-R2/514/514-3 family | A source-to-specimen contradiction inside the official lesson; its reproduced AT-family limits are less than 6 mA at 24 VDC, more than 50 k operations, and 1 ohm contact resistance. Bundled code enables the input pull-up and interprets `LOW` as closure | That the pictured or shipped `HDX HDX` part is made by Light Country, belongs to the AT family, or has the reproduced ratings |
| [Current Light Country AT-family sheet](https://www.lightcountry.com/download/tilt/Ball-Rolling%20Switch%20AT.pdf) | Current live family document | A current manufacturer family source | A safe replacement for the older bundled limits. Its 1–50 mA, 1.5–24 V, and more than 100 k-operation statements differ materially from the bundled tutorial |
| [ELEGOO 37-in-1 Sensor Kit V2 product](https://www.elegoo.com/products/elegoo-37-in-1-sensor-kit), [official tutorial page](https://www.elegoo.com/en-gb/blogs/arduino-projects/elegoo-upgraded-37-in-1-sensor-modules-kit-tutorial), and [official product image](https://www.elegoo.com/cdn/shop/products/elegoo-upgraded-37-in-1-sensor-modules-kit-compatible-with-arduino-ide-arduino-stem-kits-elegoo-shop-592935.jpg?v=1622707708) | V2 product family | Candidate context for Tilt-Switch, Shock, Tap, Big Sound, and Small Sound boards | Proof that a user's board is V2, an exact schematic, or a component-level identity |
| V2 Tilt-Switch and Shock tutorial material | Tutorial-level module descriptions | The documents describe a 10 kΩ connection between center and `S` and a contact on the outer pins | Exact contact part/revision, every PCB connection, protection, or connector order on an uninspected board |
| V2 Tap tutorial material | Tutorial-level module description | The document describes the contact on the outer pins only | Exact contact part/revision or a universal mapping for similarly named modules |
| Bundled `Lesson 20` sound-module PDF | Red, approximately 42.5 × 15 mm “Large Microphone Module”; labels `AO`, `G`, `+5V`, `DO` | The tutorial describes an electret microphone board with analog output, adjustable threshold digital output, potentiometer, and indicator LEDs | Board schematic, PCB revision, U1 identity, microphone identity, comparator pull-up rail, output protection, or a resolved supply specification. Its prose mentions 3.3/5 V while its exercise uses 5 V |
| [Joy-IT KY-037](https://sensorkit.joy-it.net/en/sensors/ky-037) and [Joy-IT KY-038](https://sensorkit.joy-it.net/en/sensors/ky-038) | Third-party candidate families | Comparison candidates for large/small microphone-module layouts and functions | Identity of an ELEGOO or user specimen; permission to transfer their pinout or circuit claims |
| [Texas Instruments LM393 datasheet](https://www.ti.com/lit/ds/symlink/lm393.pdf) | TI LM393 family | Primary semiconductor evidence if the specimen's exact U1 marking and package match | Evidence that an unidentified U1 is an LM393, that the board uses a particular pull-up rail, or that every LM393-like module shares one schematic |

## Contact candidates and limitations

The official Mega lesson does not establish one coherent part identity. Its
photograph shows a black cylindrical bare switch marked `HDX HDX`, while its
reproduced data refer to the visually different Light Country AT family. The
AT documents are therefore candidate/reference material only and must not be
attributed to the pictured or shipped part. Even if a physical specimen
independently proves to be an AT member, the listed members are not
interchangeable identities, and the discrepancy between bundled and current
limits remains a revision boundary. Until markings, geometry, terminals, and
continuity behavior select an exact manufacturer part and revision, neither
limit set may authorize a canonical circuit.

The official 37-in-1 V2 Tilt-Switch, Shock, and Tap documentation supplies
additional module candidates. It is useful for planning what to inspect:
center versus outer pins, an apparent 10 kΩ path on Tilt/Shock, and an
outer-pin contact on Tap. It does not establish that a similarly named loose
board has that layout. Mercury, cracked glass, an unknown capsule, and a bare
piezo element remain excluded.

## Acoustic candidates and limitations

The reproducible Mega archive narrows one candidate to the red “Large
Microphone Module” described above. It supports only tutorial-level roles:
`AO` as an analog signal and `DO` as an adjustable threshold indication.
The unresolved 3.3/5 V prose, absent schematic, unidentified U1 and microphone,
and unknown output pull-up prevent an electrical connection claim.

KY-037 and KY-038 are comparison candidates, not aliases. Their documentation
can guide visual inspection but cannot establish the identity or connector
order of the physical board. The TI LM393 sheet becomes relevant only if an
exact readable marking selects that device. Nothing in these sources turns
the analog signal into calibrated SPL, speech content, loudness, or frequency
evidence; Lessons 038–039 remain limited to a relative envelope from a
qualified specimen.

## Requirement coverage

| Order and requirement | Research contribution | Status after research |
|---|---|---|
| 1 — identify contact specimen (`a69678fe-7f48-40d7-8b2d-a2045fa7f476`) | Exposes the Mega lesson's `HDX HDX` photograph versus Light Country AT-data contradiction and separately catalogs 37-in-1 Tilt/Shock/Tap candidates | Pending: online family evidence cannot identify the physical specimen |
| 2 — contact primary sources (`8ce718d2-a910-40e0-b445-09a5ee3dd0dc`) | Supplies official ELEGOO archive provenance and current Light Country family literature; records the revision conflict | Pending conditionally: select the exact part/revision first; a module still needs its exact schematic and active/protection-part sources |
| 3 — map contact specimen unpowered (`04c3343a-57cc-452c-8514-b397730561e1`) | Identifies candidate pin/contact patterns to test without adopting them | Pending: requires measurements on the physical specimen |
| 4 — identify acoustic specimen (`7befbf4a-c6bf-4b3e-8c2f-eba075536249`) | Narrows candidates to the archived Large Microphone Module and comparison-only KY-037/KY-038 families | Pending: photographs and markings must select the actual board and revision |
| 5 — acoustic primary sources (`ef9a1a86-e4f7-4b98-8849-d22aa8815755`) | Supplies official tutorial provenance, comparison documentation, and a conditional LM393 primary source | Pending conditionally: exact board schematic and marked-device datasheets remain required after identity |
| 6 — map acoustic specimen unpowered (`33c54ef0-612a-4ebc-a55e-21942296c1dd`) | Defines likely `AO`/`DO` questions while preserving supply, pull-up, and protection unknowns | Pending: requires continuity/resistance work on the physical specimen |

Research therefore advances all six requirements, but closes none. Four
requirements inherently require the physical specimen (1, 3, 4, and 6); the
two source requirements (2 and 5) can close only after that identity selects
the applicable sources.

## Minimum remaining physical actions

Keep the Mega, USB, and every external supply disconnected throughout:

1. Photograph the contact specimen at original resolution with an ID card or
   ruler: whole part, front and back, every terminal in order, every marking,
   and any damage or rework. Record whether it is a bare sealed-ball/dry
   contact or a conditioned board.
2. In continuity or resistance mode, record contact terminal pairs at rest
   and while gently actuated. For a board, trace ground, supply, signal,
   series/clamp parts, pull-up destination, and indicator loading. Record
   method, range, and values; do not infer a pin from position.
3. Photograph the acoustic board similarly, including every connector label,
   microphone capsule, potentiometer orientation, U1 and other device
   markings, PCB/revision text, polarity marks, and rework.
4. With the acoustic board unpowered, record continuity/resistance evidence
   for ground, supply, `AO`, `DO`, microphone bias/amplifier path, comparator
   output and pull-up destination, regulator/protection, LEDs, and all
   potentiometer terminals and direction. Preserve every unresolved node as
   unknown.
5. Match the observed markings and topology to the candidate documents.
   Obtain the exact board schematic and primary datasheets for each marked
   active/protection part. If the manufacturer publishes none, record that
   absence rather than substituting a look-alike schematic.

Only after this unpowered identity and mapping evidence is reviewed should a
separate, bounded energization plan be written. No step above is powered bench
acceptance, and no resulting acoustic observation may be described as SPL.
