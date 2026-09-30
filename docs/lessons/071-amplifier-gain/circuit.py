# The isolated generator drives amplifier A; the USB-powered Mega supplies
# the MCP6002 but uses no signal pins. Amplifier B holds its own midpoint.
bench = Bench ("An MCP6002 makes a small sine wave about twice as tall",
               columns=(1, 28))

bench.module ("sensor", name="wave", at=(1, 4), pins=["OUT", "GND"],
              label="isolated generator")

bench.stage ("the MCP6002 and its supply")
bench.chip ("MCP6002", pins=["OUTA", "−A", "+A", "VSS",
                              "+B", "−B", "OUTB", "VDD"], first=15)
bench.wire ("a18", "B-18")                    # pin 4, VSS
bench.wire ("j15", "T+15")                    # pin 8, VDD
bench.capacitor ("100 nF", "h15", "b18")     # shortest supply strips at the IC
bench.wire ("B-13", "T-13")                  # common GND for nearby bulk capacitor
bench.capacitor ("10 µF", "T+16", "T-16", polarized=True)

bench.stage ("amplifier A and its feedback resistors")
bench.wire ("wave.OUT", "a17")               # pin 3, +A
bench.wire ("wave.GND", "B-5")
bench.resistor ("10 kΩ", "g20", "e20")       # OUTA to −A
bench.wire ("a15", "j20")                    # pin 1, OUTA
bench.wire ("a16", "b20")                    # pin 2, −A
bench.resistor ("10 kΩ", "g21", "e21")       # −A to GND
bench.wire ("c16", "j21")
bench.wire ("a21", "B-21")

bench.stage ("amplifier B's steady midpoint")
bench.resistor ("10 kΩ", "g24", "e24")       # 5 V to midpoint
bench.wire ("j24", "T+24")
bench.resistor ("10 kΩ", "g25", "e25")       # midpoint to GND
bench.wire ("b24", "j25")
bench.wire ("a25", "B-25")
bench.wire ("c24", "j18")                    # midpoint to pin 5, +B
bench.wire ("j17", "j16")                    # pin 6 −B to pin 7 OUTB

bench.closeup (13, 26)
