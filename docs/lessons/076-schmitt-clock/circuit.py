# Keep E20's LED, its 1 kΩ resistor and return wire, the supply wires,
# the rail link and the bypass capacitor on the top rails. Replace the NAND
# with a Schmitt IC.
bench = Bench ("A Schmitt inverter and RC feedback make an LED blink",
               columns=(1, 24))

bench.stage ("the SN74HC14N and its supply")
bench.chip ("SN74HC14N", pins=["1A", "1Y", "2A", "2Y", "3A", "3Y", "GND",
                                "4Y", "4A", "5Y", "5A", "6Y", "6A", "VCC"],
            first=16)
bench.wire ("T+16", "j16")              # pin 14, VCC
bench.wire ("a22", "B-22")              # pin 7, GND
bench.wire ("B-23", "T-23")             # the top − rail is GND too
bench.capacitor ("100 nF", "T+15", "T-15")  # beside pin 14's supply wire

bench.stage ("the unused inverter inputs")
bench.wire ("a18", "B-18")              # pin 3, 2A
bench.wire ("a20", "B-21")              # pin 5, 3A
bench.wire ("j21", "T-21")              # pin 9, 4A
bench.wire ("j19", "T-19")              # pin 11, 5A
bench.wire ("j17", "T-17")              # pin 13, 6A

bench.stage ("the 100 kΩ feedback and 10 µF timing capacitor")
bench.wire ("b17", "j11")               # pin 2 output to feedback resistor
bench.resistor ("100 kΩ", "g11", "e11")
bench.wire ("b16", "b11")               # feedback to pin 1 input
bench.capacitor ("10 µF", "a16", "B-16", polarized=True)

bench.stage ("the red LED and its 1 kΩ resistor")
bench.wire ("c17", "j6")                # separate output branch
bench.resistor ("1 kΩ", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "B-7")

bench.measure ("The timing capacitor, slowed", red="d16", black="GND",
               expect="rises to about 2.7 V, falls to about 1.7 V, and again",
               when="With two 100 kΩ resistors, as in Slow it down")
