# Laid out from the end of the breadboard nearest the Mega: the contrast knob
# first, then the LCD with its pin 1 (VSS) next to the knob, so that short
# jumpers join them. Power comes down from the top rails, the six signal
# wires follow in pin order, and the backlight's resistor sits at the far end.
bench = Bench ("An LCD1602 on pins 31 to 36, with a contrast knob and its backlight through 220 Ω",
               columns=(1, 35))

bench.potentiometer ("e1", "e2", "e3")
bench.lcd (5, row="a", text=("Hello, LCD!", ""))

bench.wire ("5V", "T+3")
bench.wire ("GND", "a1")
bench.wire ("b1", "b5", color="black")
bench.wire ("e5", "T-5")
bench.wire ("e6", "T+6")
bench.wire ("d3", "d6", color="red")
bench.wire ("c2", "c7", color="brown")
bench.wire ("e9", "T-9")

bench.wire ("31", "e8")
bench.wire ("32", "e10")
bench.wire ("33", "e15")
bench.wire ("34", "e16")
bench.wire ("35", "e17")
bench.wire ("36", "e18")

bench.resistor ("220 Ω", "e19", "f19")
bench.wire ("j19", "T+19")
bench.wire ("e20", "T-21")
