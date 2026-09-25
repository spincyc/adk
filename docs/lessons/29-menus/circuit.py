# The LCD stands in row a from column 1, its screen lying off the bottom edge.
# Its power comes from the left: GND, then short jumpers to RW and K; the
# contrast knob sits right above its first pins, jumpered across the middle
# gap. The data wires follow, then the backlight resistor and, last, the lamp
# on pin 3. The rotary encoder sits above the Mega, beside pins 18 and 19.
bench = Bench ("An LCD on pins 31 to 36, a rotary encoder on 18 and 19 with its switch "
               "on 22, and a lamp on pin 3")

bench.lcd (1, row="a", text=(">Level   60%", " Mode    Steady"))
bench.wire ("GND4", "d1")
bench.wire ("c1", "c5", color="black")
bench.wire ("b5", "b16", color="black")
bench.potentiometer ("j1", "j3", "j5")
bench.wire ("5V3", "f5")
bench.wire ("h5", "h2", color="red")
bench.wire ("i1", "e1", color="black")
bench.wire ("i2", "e2", color="red")
bench.wire ("i3", "e3", color="grey")
bench.wire ("31", "d4")
bench.wire ("32", "d6")
bench.wire ("33", "d11")
bench.wire ("34", "d12")
bench.wire ("35", "d13")
bench.wire ("36", "d14")
bench.wire ("i5", "i15", color="red")
bench.resistor ("220 Ω", "g15", "d15")

bench.wire ("3", "i22")
bench.resistor ("220 Ω", "h22", "h18")
bench.led ("red", anode="j18", cathode="j19")
bench.wire ("f19", "e19", color="black")
bench.wire ("c19", "c16", color="black")

bench.module ("encoder", at=(3.0, -1.6))
bench.wire ("18", "encoder.CLK", color="white")
bench.wire ("19", "encoder.DT", color="grey")
bench.wire ("22", "encoder.SW", color="brown")
bench.wire ("5V", "encoder.+")
bench.wire ("GND", "encoder.GND")
