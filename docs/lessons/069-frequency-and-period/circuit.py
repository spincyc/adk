# Keep E13's generator, capacitor, resistor and wiring.
# Only the generator's frequency changes in this investigation.
bench = Bench ("Count cycles through a capacitor and 1 kΩ resistor",
               columns=(1, 14))

bench.module ("sensor", name="wave", at=(1, 4), pins=["OUT", "GND"],
              label="isolated generator",
              detail="battery powered; set OUT to a 0–4 V sine wave, 100 Hz then 1 kHz")

bench.stage ("the capacitor and resistor path")
bench.wire ("wave.OUT", "j6")
bench.capacitor ("1 µF", "g6", "e6")
bench.wire ("b6", "j10")
bench.resistor ("1 kΩ", "g10", "e10")
bench.wire ("a10", "B-10")
bench.wire ("wave.GND", "B-5")

bench.closeup (1, 12)
