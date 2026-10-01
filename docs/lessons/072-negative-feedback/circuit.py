# Keep E16's MCP6002 supply, bypass capacitors, and B follower. Change A's
# feedback to a direct wire and feed it from the knob at home. The 1 kΩ
# load stands from b12 into the − rail, fed from pin 1's strip by a short
# jumper; the page first moves that jumper's b15 end to c40, on the knob's
# wiper, and back.
bench = Bench ("An MCP6002 output follows a knob while feeding a 1 kΩ load",
               columns=(1, 42))

bench.stage ("the MCP6002 and its supply")
bench.chip ("MCP6002", pins=["OUTA", "−A", "+A", "VSS",
                              "+B", "−B", "OUTB", "VDD"], first=15)
bench.wire ("a18", "B-18")                    # pin 4, VSS
bench.wire ("j15", "T+15")                    # pin 8, VDD
bench.wire ("B-21", "T-21")                   # the top − rail is GND, near pin 4
bench.capacitor ("100 nF", "T+16", "T-16")    # beside pin 8's supply wire
bench.capacitor ("10 µF", "T+13", "T-13", polarized=True)

bench.stage ("amplifier B's steady midpoint")
bench.resistor ("10 kΩ", "g24", "e24")
bench.wire ("j24", "T+24")
bench.resistor ("10 kΩ", "g25", "e25")
bench.wire ("b24", "j25")
bench.wire ("a25", "B-25")
bench.wire ("c24", "j18")
bench.wire ("j17", "j16")

bench.stage ("amplifier A's feedback and load")
bench.wire ("a15", "a16")
bench.wire ("b15", "b12")
bench.resistor ("1 kΩ", "a12", "B-12")

bench.home_knob ()
bench.stage ("the knob's reference for amplifier A")
bench.wire ("b40", "a17")

bench.measure ("Knob wiper at +A", red="a17", black="GND",
               expect="about 2 V", when="Knob set near 2 V")
bench.measure ("Loaded output at OUTA", red="a15", black="GND",
               expect="about 2 V, close to the wiper", when="Knob set near 2 V")
