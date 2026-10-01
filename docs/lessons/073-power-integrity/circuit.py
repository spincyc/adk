# The 10 Ω feed ends at column 4's lower strip. Both capacitors bridge
# that local supply to GND, never the 10 Ω feed itself.
bench = Bench ("A switched LED load and capacitors on a local 5 V supply",
               columns=(1, 35))

bench.module ("generator", name="wave", at=(1, 4), label="isolated generator",
              detail="battery powered; set OUT to a 0–4 V square wave at 1 kHz",
              shows="1.000 kHz", wave="square")

bench.stage ("the 10 Ω feed and local supply")
bench.wire ("T+4", "j4")
bench.resistor ("10 Ω", "g4", "e4")

bench.stage ("the red LED and its 220 Ω resistor")
bench.wire ("b4", "j6")
bench.resistor ("220 Ω", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "b31")

bench.stage ("the S8050 transistor")
bench.transistor ("a29", "a30", "a31")
bench.wire ("b29", "B-29")

bench.stage ("the generator and base resistors")
bench.wire ("wave.OUT", "a32")
bench.resistor ("1 kΩ", "c32", "c30")
bench.resistor ("10 kΩ", "b30", "B-30")
bench.wire ("wave.GND", "B-5")

bench.stage ("the two local supply capacitors")
bench.capacitor ("100 µF", "c4", "B-4", polarized=True)
bench.wire ("d4", "b9")
bench.capacitor ("100 nF", "c9", "B-9")

bench.probe ("CH1 · local supply, after the 10 Ω", tip="h6", ground="GND", channel=1,
             expect="near 5 V, stepping down about 0.13 V while the LED is lit",
             when="generator on; AC coupling at 50 mV/div")
