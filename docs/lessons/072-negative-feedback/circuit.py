# Keep Lesson 71's MCP6002 supply, bypass capacitors, and B follower.
# Change A's feedback to a direct wire and feed it from the knob at home.
bench = Bench ("An MCP6002 output follows a knob while feeding a 1 kΩ load",
               columns=(1, 42))

bench.stage ("the MCP6002 and its supply")
bench.chip ("MCP6002", pins=["OUTA", "−A", "+A", "VSS",
                              "+B", "−B", "OUTB", "VDD"], first=15)
bench.wire ("a18", "B-18")
bench.wire ("j15", "T+15")
bench.capacitor ("100 nF", "h15", "b18")
bench.wire ("B-13", "T-13")
bench.capacitor ("10 µF", "T+16", "T-16", polarized=True)

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
bench.wire ("b15", "j28")
bench.resistor ("1 kΩ", "g28", "e28")
bench.wire ("a28", "B-28")

bench.home_knob ()
bench.stage ("the knob's reference for amplifier A")
bench.wire ("b40", "a17")

bench.measure ("Knob wiper at +A", red="a17", black="GND",
               expect="about 2 V", when="Knob set near 2 V")
bench.measure ("Loaded output at OUTA", red="a15", black="GND",
               expect="close to the wiper voltage", when="Knob set near 2 V")

bench.closeup (13, 42)
