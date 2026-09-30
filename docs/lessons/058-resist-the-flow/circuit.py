# Keep E02's home holes. During each trial, replace only the resistor.
bench = Bench ("A red LED from 5 V through 220 Ω to GND", columns=(1, 20))

bench.stage ("the red LED and its resistor")
bench.wire ("T+6", "j6")
bench.resistor ("220 Ω", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "B-7")

bench.measure ("Across the resistor", red="h6", black="d6", expect="about 3 V")
