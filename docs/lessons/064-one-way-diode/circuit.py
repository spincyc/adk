# One path from USB-powered 5 V to GND. The 1N4007 stands where E01's red
# jumper was, its banded end down in j6 toward the resistor and the red LED
# at its column 6 home; turn only that diode around for the dark trial.
bench = Bench ("A 1N4007 diode and red LED in one 1 kΩ-limited path", columns=(1, 14))

bench.stage ("the diode, resistor and LED path")
bench.diode (anode="T+6", cathode="j6")
bench.resistor ("1 kΩ", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "B-7")
