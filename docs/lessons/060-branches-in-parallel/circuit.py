# USB 5 V reaches both branches through one 10 Ω resistor, standing from
# the top + rail into column 4, so the voltage across it shows the current
# the branches share. Each branch runs from that feed through its own 1 kΩ
# resistor and red LED to the bottom − rail; no I/O pin drives either LED.
bench = Bench ("Two red LED and 1 kΩ resistor branches in parallel, fed through 10 Ω",
               columns=(1, 20))

bench.stage ("the shared 10 Ω feed")
bench.resistor ("10 Ω", "T+4", "j4")

bench.stage ("the first LED branch")
bench.wire ("i4", "i6")
bench.resistor ("1 kΩ", "g6", "e6")
bench.led ("red", anode="b6", cathode="b7")
bench.wire ("a7", "B-7")

bench.stage ("the second LED branch")
bench.wire ("h4", "h12")
bench.resistor ("1 kΩ", "g12", "e12")
bench.led ("red", anode="b12", cathode="b13")
bench.wire ("a13", "B-13")

bench.measure ("Across the 10 Ω feed, first branch only", red="T+7", black="g4",
               expect="about 0.03 V (30 mV)", when="Jumper from h4 to h12 out")
bench.measure ("Across the 10 Ω feed, both branches", red="T+7", black="g4",
               expect="about 0.06 V (60 mV)", when="Both LEDs on")
bench.measure ("Across the first branch", red="j6", black="B-7",
               expect="about 5 V", when="Both LEDs on")
bench.measure ("Across the second branch", red="j12", black="B-13",
               expect="about 5 V", when="Both LEDs on")
bench.measure ("Across the first 1 kΩ resistor", red="h6", black="d6",
               expect="about 3 V", when="One branch, then both")
bench.measure ("Across the second 1 kΩ resistor", red="i12", black="d12",
               expect="about 3 V", when="Both LEDs on")
