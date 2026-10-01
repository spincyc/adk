# Gate 1 of the SN74HC00N uses pins 1 and 2 as inputs and pin 3 as its output.
# Its other six inputs have defined low levels; their outputs need no wires.
bench = Bench ("Two buttons and an SN74HC00N make a NAND truth table",
               columns=(1, 26))

bench.stage ("the SN74HC00N and its supply capacitor")
bench.chip ("SN74HC00N", first=16,
            pins=["1A", "1B", "1Y", "2A", "2B", "2Y", "GND",
                  "3Y", "3A", "3B", "4Y", "4A", "4B", "VCC"])
bench.wire ("T+16", "j16")
bench.wire ("a22", "B-22")
bench.capacitor ("100 nF", "g15", "e15")
bench.wire ("T+15", "j15")
bench.wire ("a15", "B-15")

bench.stage ("button A and its pull-down resistor")
bench.button (2)
bench.wire ("T+4", "j2")
bench.wire ("a4", "c16")
bench.resistor ("10 kΩ", "b16", "B-16")

bench.stage ("button B and its pull-down resistor")
bench.button (8)
bench.wire ("T+9", "j8")
bench.wire ("a10", "c17")
bench.resistor ("10 kΩ", "b17", "B-17")

bench.stage ("the unused gate inputs")
bench.wire ("a19", "B-19")
bench.wire ("a20", "B-21")
bench.wire ("B-23", "T-23")
bench.wire ("j17", "T-17")
bench.wire ("j18", "T-18")
bench.wire ("j20", "T-19")
bench.wire ("j21", "T-21")

bench.stage ("the red output LED and its 1 kΩ resistor")
bench.wire ("b18", "j6")
bench.resistor ("1 kΩ", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "B-7")
