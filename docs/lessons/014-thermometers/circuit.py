# Lesson 13's screen stays at the far end. The thermistor's divider
# stands in column 37, with its resistor running down to the bottom − rail.
# The 18B20 and DHT11 return to their homes above the board.
bench = Bench ("A DHT11 on pin 16, a thermistor divider on A2 and an 18B20 on pin 17, "
               "with the screen from Lesson 13", columns=(1, 45))

bench.screen (text=("DHT11 23°C  45%", "NTC 23.4 DS 23.1"))

bench.home_dht11 ()

bench.home_divider ("thermistor")

bench.home_ds18b20 ()

# Readings to take with a multimeter: the thermistor divider's middle point
# on A2, at room temperature and warmed, and the thermistor's own share.
bench.measure ("The divider's middle, on A2", red="A2", black="GND", expect="about 2.3 V",
               when="at room temperature")
bench.measure ("Across the thermistor", red="h37", black="b37", expect="about 2.7 V",
               when="at room temperature")
bench.measure ("A2 with the bead warmed", red="A2", black="GND", expect="about 2.8 V",
               when="just after pinching the bead for 20 s")
