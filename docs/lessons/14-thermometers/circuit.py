# Lesson 13's screen stays where it was, in columns 1 to 21. The two sensor
# modules sit above the board, each over the rails that power it, the DHT11
# first; the thermistor's divider comes after the screen's overhang, where
# the wire from A2 can reach it from below, and the optional 18B20 comes last.
bench = Bench ("A DHT11 on pin 16, a thermistor divider on A2 and an 18B20 on pin 17, "
               "with the LCD from Lesson 13", columns=(1, 45))

bench.potentiometer ("e1", "e2", "e3")
bench.lcd (5, row="a", text=("DHT11 23°C  45%", "NTC 23.4 DS 23.1"))

bench.wire ("5V", "T+3")
bench.wire ("GND", "a1")
bench.wire ("b1", "b5", color="black")
bench.wire ("e5", "T-5")
bench.wire ("e6", "T+6")
bench.wire ("d3", "d6", color="red")
bench.wire ("c2", "c7", color="brown")
bench.wire ("e9", "T-9")

bench.wire ("31", "e8")
bench.wire ("32", "e10")
bench.wire ("33", "e15")
bench.wire ("34", "e16")
bench.wire ("35", "e17")
bench.wire ("36", "e18")

bench.resistor ("220 Ω", "e19", "f19")
bench.wire ("j19", "T+19")
bench.wire ("e20", "T-21")

bench.module ("dht11", "dht", at=(7.38, -1.25))
bench.wire ("16", "dht.S")
bench.wire ("dht.+", "T+24")
bench.wire ("dht.−", "T-25")

bench.wire ("e35", "T+35")
bench.thermistor ("b35", "b37")
bench.wire ("A2", "a37")
bench.resistor ("10 kΩ", "c37", "c40")
bench.wire ("e40", "T-40")

bench.module ("sensor", "probe", at=(9.18, -1.25), label="18B20")
bench.wire ("17", "probe.S")
bench.wire ("probe.+", "T+42")
bench.wire ("probe.−", "T-43")

bench.closeup (17, 44)
