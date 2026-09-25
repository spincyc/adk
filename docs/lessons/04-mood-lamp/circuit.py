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
