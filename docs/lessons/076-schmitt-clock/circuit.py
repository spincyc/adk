# Keep E20's LED, its 1 kΩ resistor and return wire, the supply
# jumper and nearby bypass capacitor. Replace the NAND with a Schmitt IC.
bench = Bench ("A Schmitt inverter and RC feedback make an LED blink",
               columns=(1, 24))

bench.stage ("the SN74HC14N and its supply")
bench.chip ("SN74HC14N", pins=["1A", "1Y", "2A", "2Y", "3A", "3Y", "GND",
                                "4Y", "4A", "5Y", "5A", "6Y", "6A", "VCC"],
            first=16)
bench.wire ("j16", "T+16")              # pin 14, VCC
bench.wire ("a22", "B-22")              # pin 7, GND
bench.capacitor ("100 nF", "g15", "e15")
bench.wire ("j15", "i16")               # bypass capacitor to VCC
bench.wire ("a15", "B-15")              # bypass capacitor to GND
bench.wire ("B-13", "T-13")             # top − rail for unused inputs

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
bench.capacitor ("10 µF", "c16", "c15", polarized=True)

bench.stage ("the red LED and its 1 kΩ resistor")
bench.wire ("c17", "j6")                # separate output branch
bench.resistor ("1 kΩ", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "B-7")
