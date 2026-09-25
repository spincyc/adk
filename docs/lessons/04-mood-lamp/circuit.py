# The button on 22 stays at its home across the middle gap in columns 2-4,
# with its wires, as in Lessons 2 and 3. The RGB LED stands at its home: its
# red, green and blue legs in a6, a9 and a11, each fed from pins 5, 6 and 7
# through a 220 Ω resistor across the gap (the one in g6-e6 is the red LED's
# from Lesson 3, left where it was), its common leg straight into the bottom
# − rail at B-7.
bench = Bench ("An RGB LED on pins 5, 6 and 7, and a button on pin 22", columns=(1, 20))

bench.wire ("22", "j2")
bench.button (2)
bench.wire ("a4", "B-4")

bench.wire ("5", "j6")
bench.wire ("6", "j9")
bench.wire ("7", "j11")
bench.resistor ("220 Ω", "g6", "e6")
bench.resistor ("220 Ω", "g9", "e9")
bench.resistor ("220 Ω", "g11", "e11")
bench.rgb_led (red="a6", common="B-7", green="a9", blue="a11")

# Readings to take with a multimeter: in the orange mood, {255, 64, 0}, the
# red pin is on all the time and the green pin a quarter of it, which the
# meter shows as a quarter of 5 V; across the red LED in orange and the blue
# LED in blue, the two colors' different voltages.
bench.measure ("The red pin, at 255", red="5", black="GND", expect="about 5 V",
               when="Orange")
bench.measure ("The green pin, at 64", red="6", black="GND", expect="about 1.25 V",
               when="Orange")
bench.measure ("Across the red LED", red="c6", black="GND", expect="about 2 V", when="Orange")
bench.measure ("Across the blue LED", red="c11", black="GND", expect="about 3.2 V",
               when="Blue")
