# The RGB LED at its home, as in Lesson 4: its legs in row a of columns 6, 9
# and 11, its longest leg in the − rail, and a 220 Ω resistor across the
# middle gap above each colored leg, fed from pins 5, 6 and 7. The IR
# receiver sits above the gap between the Mega and the breadboard, under
# the LED's wires: its signal to pin 2, its power from the Mega's 5V and GND
# at the ends of the long header.
bench = Bench ("An IR receiver on pin 2, and an RGB LED on pins 5, 6 and 7, each color through "
               "a 220 Ω resistor", columns=(1, 30))

bench.wire ("5", "j6", via=[(2.45, -1.15), (5.9, -1.15)])
bench.wire ("6", "j9", via=[(2.35, -1.25), (6.2, -1.25)])
bench.wire ("7", "j11", via=[(2.25, -1.35), (6.4, -1.35)])
bench.resistor ("220 Ω", "g6", "e6")
bench.resistor ("220 Ω", "g9", "e9")
bench.resistor ("220 Ω", "g11", "e11")
bench.rgb_led (red="a6", common="B-7", green="a9", blue="a11")

bench.module ("ir_receiver", name="receiver", at=(4.3, -0.9))
bench.wire ("2", "receiver.S")
bench.wire ("receiver.+", "5V.long")
bench.wire ("receiver.−", "GND.long")

bench.closeup (1, 30)
