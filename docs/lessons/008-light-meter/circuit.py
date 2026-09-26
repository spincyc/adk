# Every part at its home, left to right: the five LEDs of the bar on pins 26
# to 30 in columns 6 to 30, each from its pin into row j, through its 220 Ω
# across the middle gap, the LED, and a black jumper to the − rail; then the
# light sensor's divider in column 40, from the top + rail down through the
# photoresistor across the gap to the point A1 reads, and on through 10 kΩ
# to the − rail. Of Lesson 7's dimmer, only the Mega's power wires stay.
bench = Bench ("A photoresistor divider on A1, and five LEDs on pins 26 to 30", columns=(1, 50))

# The five wires leave the double header as a ribbon and spread onto their
# lanes at once: 27 hops over 26 (the one crossing the header's paired rows
# make unavoidable) to reach j12 from above, while 28, 29 and 30 run below
# row j and rise into it. A1's wire runs below the board, clear of its edge.
lanes = {26: [(4.10, 1.00), (4.50, 1.00), (4.50, 1.05), (5.90, 1.05)],
         27: [(4.30, 1.05), (4.30, 0.90), (6.50, 0.90)],
         28: [(4.10, 1.10), (4.40, 1.10), (4.40, 1.15), (7.10, 1.15)],
         29: [(4.30, 1.15), (4.30, 1.25), (7.70, 1.25)],
         30: [(4.10, 1.30), (4.20, 1.30), (4.20, 1.35), (8.30, 1.35)]}
for pin, color, column in ((26, "red", 6), (27, "yellow", 12), (28, "green", 18),
                           (29, "blue", 24), (30, "white", 30)):
    bench.wire (str (pin), f"j{column}", via=lanes[pin])
    bench.resistor ("220 Ω", f"g{column}", f"e{column}")
    bench.led (color, anode=f"b{column}", cathode=f"b{column + 1}")
    bench.wire (f"a{column + 1}", f"B-{column + 1}")

bench.wire ("j40", "T+40")
bench.photoresistor ("f40", "e40")
bench.wire ("A1", "a40", via=[(2.30, 2.85), (9.25, 2.85)])
bench.resistor ("10 kΩ", "c40", "c43")
bench.wire ("a43", "B-43")

# Readings to take with a multimeter: the divider's middle in room light and
# with the sensor covered, and the photoresistor's own share of the 5 V.
bench.measure ("The divider's middle, in room light", red="d40", black="GND",
               expect="about 2.5 V", when="Room light")
bench.measure ("The divider's middle, covered", red="d40", black="GND",
               expect="about 0.5 V", when="Sensor covered")
bench.measure ("Across the photoresistor, covered", red="h40", black="d40",
               expect="about 4.5 V", when="Sensor covered")
