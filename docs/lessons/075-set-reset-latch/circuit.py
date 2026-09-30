# Keep Lesson 74's SN74HC00N, buttons, LED and supply capacitor in their
# holes. Each button now pulls a normally high input low; two gates feed back.
bench = Bench ("Two NAND gates remember the last button press", columns=(1, 26))

bench.stage ("the SN74HC00N and its supply capacitor")
bench.chip ("SN74HC00N", first=16,
            pins=["1A", "1B", "1Y", "2A", "2B", "2Y", "GND",
                  "3Y", "3A", "3B", "4Y", "4A", "4B", "VCC"])
bench.wire ("T+16", "j16")               # pin 14, VCC
bench.wire ("a22", "B-22")               # pin 7, GND
bench.capacitor ("100 nF", "g15", "e15")
bench.wire ("T+15", "j15")               # bypass capacitor to VCC
bench.wire ("a15", "B-15")               # bypass capacitor to GND

bench.stage ("the Set button and its pull-up")
bench.button (2)
bench.resistor ("10 kΩ", "T+4", "j2")
bench.wire ("a4", "B-4")
bench.wire ("a2", "c16")                 # set_N, gate 1 pin 1

bench.stage ("the Reset button and its pull-up")
bench.button (8)
bench.resistor ("10 kΩ", "T+9", "j8")
bench.wire ("a10", "B-10")
bench.wire ("a8", "c19")                 # reset_N, gate 2 pin 4

bench.stage ("the two feedback paths")
bench.wire ("a21", "c17")               # pin 6 (Qbar) to pin 2
bench.wire ("a18", "c20")               # pin 3 (Q) to pin 5

bench.stage ("the unused gate inputs")
bench.wire ("B-23", "T-23")             # nearby top ground rail
bench.wire ("j17", "T-17")              # pin 13
bench.wire ("j18", "T-18")              # pin 12
bench.wire ("j20", "T-19")              # pin 10
bench.wire ("j21", "T-21")              # pin 9

bench.stage ("the red output LED and its 1 kΩ resistor")
bench.wire ("b18", "j6")                # pin 3, Q
bench.resistor ("1 kΩ", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "B-7")

bench.closeup (1, 24)
