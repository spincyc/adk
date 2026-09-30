# Keep E01's steady path and every wire in the same holes.
bench = Bench ("A red LED from 5 V through 220 Ω to GND", columns=(1, 20))

bench.stage ("the red LED and its resistor")
bench.wire ("T+6", "j6")
bench.resistor ("220 Ω", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "B-7")

bench.measure ("Supply, from 5 V to GND", red="5V", black="GND", expect="about 5 V")
bench.measure ("Across the 220 Ω resistor", red="h6", black="d6", expect="about 3 V")
bench.measure ("Across the red LED", red="c6", black="c7", expect="about 2 V")
