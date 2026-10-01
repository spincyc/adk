# The isolated generator drives amplifier A through a 10 kΩ input resistor;
# the USB-powered Mega supplies the MCP6002 but uses no signal pins.
# Amplifier A's three resistors lie just below the chip, and amplifier B
# holds its own midpoint.
bench = Bench ("An MCP6002 makes a small sine wave about twice as tall",
               columns=(1, 28))

bench.module ("generator", name="wave", at=(1, 4), label="isolated generator",
              detail="battery powered; set OUT to a 0.5–1.5 V sine wave at 100 Hz",
              shows="100.0 Hz")

bench.stage ("the MCP6002 and its supply")
bench.chip ("MCP6002", pins=["OUTA", "−A", "+A", "VSS",
                              "+B", "−B", "OUTB", "VDD"], first=15)
bench.wire ("a18", "B-18")                    # pin 4, VSS
bench.wire ("j15", "T+15")                    # pin 8, VDD
bench.wire ("B-21", "T-21")                   # the top − rail is GND, near pin 4
bench.capacitor ("100 nF", "T+16", "T-16")    # beside pin 8's supply wire
bench.capacitor ("10 µF", "T+13", "T-13", polarized=True)

bench.stage ("amplifier A and its feedback resistors")
bench.wire ("a15", "a13")                     # pin 1, OUTA
bench.resistor ("10 kΩ", "c13", "c16")        # OUTA to −A (pin 2)
bench.resistor ("10 kΩ", "a16", "B-16")       # −A to GND
bench.wire ("wave.OUT", "a20")
bench.wire ("wave.GND", "B-5")
bench.resistor ("10 kΩ", "c20", "c17")        # input resistor to +A (pin 3)

bench.stage ("amplifier B's steady midpoint")
bench.resistor ("10 kΩ", "g24", "e24")        # 5 V to midpoint
bench.wire ("j24", "T+24")
bench.resistor ("10 kΩ", "g25", "e25")        # midpoint to GND
bench.wire ("b24", "j25")
bench.wire ("a25", "B-25")
bench.wire ("c24", "j18")                     # midpoint to pin 5, +B
bench.wire ("j17", "j16")                     # pin 6 −B to pin 7 OUTB

bench.probe ("CH1 · + input A, pin 3", tip="d17", ground="GND", channel=1,
             expect="a sine wave from about 0.5 V to 1.5 V", when="generator on")
bench.probe ("CH2 · output A, pin 1", tip="b15", ground="GND", channel=2,
             expect="about 1 V to 3 V, twice the input", when="generator on")
