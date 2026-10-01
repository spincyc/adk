# The isolated generator drives a coil and a 1 kΩ current-sense resistor.
# The Mega contributes only its usual GND wire to the bottom − rail.
bench = Bench ("A coil slows the rise of current through a 1 kΩ resistor",
               columns=(1, 14))

bench.module ("generator", name="wave", at=(1, 4), label="isolated generator",
              detail="battery powered; set OUT to a 0–4 V square wave at 100 Hz",
              shows="100.0 Hz", wave="square")

bench.stage ("the coil and current-sense resistor")
bench.wire ("wave.OUT", "j6")
bench.inductor ("100 mH", "g6", "e6")
bench.wire ("a6", "j10", color="yellow")
bench.resistor ("1 kΩ", "g10", "e10")
bench.wire ("a10", "B-10")
bench.wire ("wave.GND", "B-5")

bench.probe ("CH1 · generator OUT", tip="i6", ground="GND", channel=1,
             expect="a 0–4 V square wave with sharp edges", when="generator on")
bench.probe ("CH2 · top of the 1 kΩ resistor", tip="i10", ground="B-9", channel=2,
             expect="rises most of the way in about 0.1 ms, levelling at 2.6–3.8 V",
             when="generator on")
