# Lesson 1's red LED stays at its home in column 6. The buttons on 22 and 23
# join it at their homes, across the middle gap in columns 2-4 and 8-10, and
# the yellow LED on 27 stands at its home in column 12. Each signal comes in
# at row j, crosses the gap through its part, and returns by the bottom
# − rail, which the Mega's GND feeds at B-3. The buttons' wires reach row j
# from above and the LEDs' from lanes just below it, nested so none cross.
bench = Bench ("Two buttons on pins 22 and 23, and two LEDs on pins 26 and 27", columns=(1, 20))

bench.home_button ("22")

bench.home_button ("23")

bench.home_led ("26", "red", via=[(4.45, 1.0), (4.45, 1.15)])

bench.home_led ("27", "yellow", via=[(4.35, 1.05), (4.35, 1.25)])

# Readings to take with a multimeter: the left button's pin up and pressed,
# and the yellow LED's pin while the right button is held.
bench.measure ("Pin 22, button up", red="22", black="GND", expect="about 5 V",
               when="Left button up")
bench.measure ("Pin 22, button pressed", red="22", black="GND", expect="0 V",
               when="Left button held down")
bench.measure ("The yellow LED's pin", red="27", black="GND", expect="about 5 V",
               when="Right button held down")
