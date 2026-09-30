# The red LED keeps its column 6 home. USB powers the Mega's 5 V rail;
# the rail, resistor, LED and GND form one steady path without a signal pin.
bench = Bench ("A red LED from 5 V through 220 Ω to GND", columns=(1, 20))

bench.stage ("the red LED and its resistor")
bench.wire ("T+6", "j6")
bench.resistor ("220 Ω", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "B-7")
