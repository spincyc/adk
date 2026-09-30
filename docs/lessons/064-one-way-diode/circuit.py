# One path from USB-powered 5 V to GND. The 1N4007's banded end faces
# the LED; turn only that diode around for the dark trial.
bench = Bench ("A 1N4007 diode and red LED in one 1 kΩ-limited path", columns=(1, 14))

bench.stage ("the resistor, diode and LED path")
bench.wire ("T+4", "j4")
bench.resistor ("1 kΩ", "g4", "e4")
bench.diode (anode="d4", cathode="d9")
bench.led ("red", anode="b9", cathode="b10")
bench.wire ("a10", "B-10")
