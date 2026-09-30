# Each branch runs from the top + rail through its own 1 kΩ resistor and
# red LED to the bottom − rail. The Mega's USB-powered 5V and GND feed the
# rails; no I/O pin drives either LED.
bench = Bench ("Two red LED and 1 kΩ resistor branches in parallel across 5 V",
               columns=(1, 20))

bench.stage ("the first LED branch")
bench.wire ("T+6", "j6")
bench.resistor ("1 kΩ", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "B-7")

bench.stage ("the second LED branch")
bench.wire ("T+12", "j12")
bench.resistor ("1 kΩ", "g12", "e12")
bench.led ("red", anode="b12", cathode="b13")
bench.wire ("a13", "B-13")

bench.closeup (3, 16)

bench.measure ("The first branch, from + to −", red="T+6", black="B-7",
               expect="about 5 V", when="Both LEDs on")
bench.measure ("The second branch, from + to −", red="T+12", black="B-13",
               expect="about 5 V", when="Both LEDs on")
bench.measure ("Across the first 1 kΩ resistor", red="g6", black="a6",
               expect="about 3 V", when="Both LEDs on")
bench.measure ("Across the second 1 kΩ resistor", red="g12", black="a12",
               expect="about 3 V", when="Both LEDs on")
