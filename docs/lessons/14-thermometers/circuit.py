# Lesson 13's screen stays exactly where it was. The thermistor's divider
# takes its home in column 40, just past the screen, and A2's wire reaches
# it round the bottom of the screen. The two sensor modules sit above the
# board, powered from the top rails: the 18B20 nearer the Mega and the
# DHT11 after it, so that pin 16's wire can pass over the 18B20 and no
# wire crosses another.
bench = Bench ("A DHT11 on pin 16, a thermistor divider on A2 and an 18B20 on pin 17, "
               "with the screen from Lesson 13", columns=(1, 45))

bench.screen (text=("DHT11 23°C  45%", "NTC 23.4 DS 23.1"))

bench.module ("dht11", "dht", at=(8.58, -1.27))
bench.wire ("16", "dht.S")
bench.wire ("dht.+", "T+36")
bench.wire ("dht.−", "T-37")

bench.wire ("j40", "T+40")
bench.thermistor ("f40", "e40")
bench.wire ("A2", "a40")
bench.resistor ("10 kΩ", "c40", "c43")
bench.wire ("a43", "B-43")

bench.module ("sensor", "probe", at=(7.68, -1.15), label="18B20")
bench.wire ("17", "probe.S")
bench.wire ("probe.+", "T+27")
bench.wire ("probe.−", "T-28")

# Readings to take with a multimeter: the thermistor divider's middle point
# on A2, at room temperature and warmed, and the thermistor's own share.
bench.measure ("The divider's middle, on A2", red="A2", black="GND", expect="about 2.3 V",
               when="at room temperature")
bench.measure ("Across the thermistor", red="h40", black="b40", expect="about 2.7 V",
               when="at room temperature")
bench.measure ("A2 with the bead warmed", red="A2", black="GND", expect="about 2.8 V",
               when="just after pinching the bead for 20 s")
