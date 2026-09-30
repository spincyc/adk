# The isolated generator drives a coil and a 1 kΩ current-sense resistor.
# The Mega contributes only its usual GND wire to the bottom − rail.
bench = Bench ("A coil slows the rise of current through a 1 kΩ resistor",
               columns=(1, 14))

bench.module ("sensor", name="wave", at=(1, 4), pins=["OUT", "GND"],
              label="isolated generator")

bench.stage ("the coil and current-sense resistor")
bench.wire ("wave.OUT", "j6")
bench.inductor ("100 mH", "g6", "e6")
bench.wire ("a6", "j10")
bench.resistor ("1 kΩ", "g10", "e10")
bench.wire ("a10", "B-10")
bench.wire ("wave.GND", "B-5")

bench.closeup (1, 12)
