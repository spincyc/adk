# Keep the generator and common ground from E13–E14. Swap the
# capacitor and resistor: the capacitor now runs from the output to GND.
bench = Bench ("A 1 kΩ and 1 µF low-pass filter driven by an isolated generator",
               columns=(1, 14))

bench.module ("sensor", name="wave", at=(1, 4), pins=["OUT", "GND"],
              label="isolated generator",
              detail="battery powered; set OUT to a 0–4 V sine wave")

bench.stage ("the resistor and capacitor filter")
bench.wire ("wave.OUT", "j6")
bench.resistor ("1 kΩ", "g6", "e6")
bench.wire ("b6", "j10")
bench.capacitor ("1 µF", "g10", "e10")
bench.wire ("a10", "B-10")
bench.wire ("wave.GND", "B-5")
