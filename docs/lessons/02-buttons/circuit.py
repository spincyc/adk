# Lesson 1's red LED stays at its home in column 6. The buttons on 22 and 23
# join it at their homes, across the middle gap in columns 2-4 and 8-10, and
# the yellow LED on 27 stands at its home in column 12. Each signal comes in
# at row j, crosses the gap through its part, and returns by the bottom
# − rail, which the Mega's GND feeds at B-3. The buttons' wires reach row j
# from above and the LEDs' from lanes just below it, nested so none cross.
bench = Bench ("Two buttons on pins 22 and 23, and two LEDs on pins 26 and 27", columns=(1, 20))

bench.wire ("22", "j2")
bench.button (2)
bench.wire ("a4", "B-4")

bench.wire ("23", "j8")
bench.button (8)
bench.wire ("a10", "B-10")

bench.wire ("26", "j6", via=[(4.45, 1.0), (4.45, 1.15)])
bench.resistor ("220 Ω", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "B-7")

bench.wire ("27", "j12", via=[(4.35, 1.05), (4.35, 1.25)])
bench.resistor ("220 Ω", "g12", "e12")
bench.led ("yellow", anode="b12", cathode="b13")
bench.wire ("a13", "B-13")

# Readings to take with a multimeter: the left button's pin up and pressed,
# and the yellow LED's pin while the right button is held.
bench.measure ("Pin 22, button up", red="22", black="GND", expect="about 5 V",
               when="Left button up")
bench.measure ("Pin 22, button pressed", red="22", black="GND", expect="0 V",
               when="Left button held down")
bench.measure ("The yellow LED's pin", red="27", black="GND", expect="about 5 V",
               when="Right button held down")
