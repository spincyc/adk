# The keypad from above the gap between the Mega and the breadboard, its
# eight wires rising straight from pins 22 to 29, rows then columns. The
# screen is Lesson 13's, wired exactly as before.
bench = Bench ("A 4×4 keypad on pins 22 to 29, with the LCD from Lesson 13", columns=(1, 35))

bench.potentiometer ("e1", "e2", "e3")
bench.lcd (5, row="a", text=("12 x 34", "= 408"))

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

bench.module ("keypad", "keypad", at=(3.4, -4.1))
for pin, name in zip (range (22, 30), ("R1", "R2", "R3", "R4", "C1", "C2", "C3", "C4")):
    bench.wire (str (pin), f"keypad.{name}")
