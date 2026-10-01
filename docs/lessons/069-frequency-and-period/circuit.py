# Keep E13's generator, capacitor, resistor and wiring.
# Only the generator's frequency changes in this investigation.
bench = Bench ("Count cycles through a capacitor and 1 kΩ resistor",
               columns=(1, 14))

bench.module ("generator", name="wave", at=(1, 4), label="isolated generator",
              detail="battery powered; set OUT to a 0–4 V sine wave, 100 Hz then 1 kHz",
              shows="100.0 Hz")

bench.stage ("the capacitor and resistor path")
bench.wire ("wave.OUT", "j6")
bench.capacitor ("1 µF", "g6", "e6")
bench.wire ("b6", "j10")
bench.resistor ("1 kΩ", "g10", "e10")
bench.wire ("a10", "B-10")
bench.wire ("wave.GND", "B-5")

bench.probe ("CH1 · generator OUT", tip="i6", ground="GND", channel=1,
             expect="1 cycle in 10 ms at 100 Hz, 10 at 1 kHz", when="generator on")
bench.probe ("CH2 · top of the 1 kΩ resistor", tip="i10", ground="B-9", channel=2,
             expect="the same cycles, centred on 0 V", when="generator on")
