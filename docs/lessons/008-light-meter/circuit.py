# Every part at its home, left to right: the five LEDs of the bar on pins 26
# to 30, then the light sensor's divider on A1. Of Lesson 7's dimmer, only
# the Mega's power wires stay.
bench = Bench ("A photoresistor divider on A1, and five LEDs on pins 26 to 30", columns=(1, 50))

# The five wires leave the double header as a ribbon and spread onto their
# lanes at once: 27 hops over 26 (the one crossing the header's paired rows
# make unavoidable) to reach j12 from above, while 28, 29 and 30 run below
# row j and rise into it. A1's wire runs below the board, clear of its edge.
lanes = {26: [(4.10, 0.95), (4.50, 0.95), (4.50, 1.05), (5.90, 1.05)],
         27: [(4.30, 1.00), (4.30, 0.90), (6.50, 0.90)],
         28: [(4.10, 1.05), (4.40, 1.05), (4.40, 1.15), (7.10, 1.15)],
         29: [(4.30, 1.10), (4.30, 1.25), (7.70, 1.25)],
         30: [(4.10, 1.25), (4.20, 1.25), (4.20, 1.35), (8.30, 1.35)]}
for pin, color in ((26, "red"), (27, "yellow"), (28, "green"), (29, "blue"), (30, "white")):
    bench.home_led (str (pin), color, via=lanes[pin])

bench.home_divider ("photoresistor")

# Readings to take with a multimeter: the divider's middle in room light and
# with the sensor covered, and the photoresistor's own share of the 5 V.
bench.measure ("The divider's middle, in room light", red="d37", black="GND",
               expect="about 2.5 V", when="Room light")
bench.measure ("The divider's middle, covered", red="d37", black="GND",
               expect="about 0.5 V", when="Sensor covered")
bench.measure ("Across the photoresistor, covered", red="h37", black="d37",
               expect="about 4.5 V", when="Sensor covered")
