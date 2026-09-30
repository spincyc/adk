# Two resistors in one path, from the Mega's USB-powered 5 V rail to GND.
# The first ends and the second begins in the upper strip at column 7.
bench = Bench ("Two resistors in series between 5 V and GND", columns=(1, 14))

bench.stage ("the two resistors in series")
bench.wire ("T+4", "j4")
bench.resistor ("1 kΩ", "i4", "i7")
bench.resistor ("1 kΩ", "g7", "g10")
bench.wire ("f10", "B-10")

bench.measure ("Across the first 1 kΩ resistor", red="h4", black="f7",
               expect="about 2.5 V", when="two 1 kΩ resistors fitted")
bench.measure ("Across the second 1 kΩ resistor", red="f7", black="f10",
               expect="about 2.5 V", when="two 1 kΩ resistors fitted")
bench.measure ("Across the pair", red="h4", black="f10",
               expect="about 5 V", when="two 1 kΩ resistors fitted")
