# Lesson 13's screen stays in columns 1 to 21, wired as before except that
# its red 5V wire from the Mega is gone: the power module at the far end now
# feeds both pairs of rails, and the screen's GND wire joins the Mega's GND
# to them. The buzzer follows the screen's pins on the top half; the servo
# latch lies below the board past the screen, its plug reaching the bottom
# rails; and the keypad hangs above the gap, as in Lesson 16.
bench = Bench ("A keypad safe: the keypad on pins 22 to 29, the LCD on 31 to 36, a servo latch on "
               "44 and the buzzer on 12, all powered by the power module", columns=(1, 63))

bench.potentiometer ("e1", "e2", "e3")
bench.lcd (5, row="a", text=("Locked. Code?", "****"))

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

bench.buzzer ("f23", "f26")
bench.wire ("12", "j23")
bench.wire ("j26", "T-27")

bench.module ("servo", "servo", at=(9.05, 3.45))
bench.wire ("servo.+", "B+45")
bench.wire ("servo.−", "B-46")
bench.wire ("44", "servo.signal")

bench.power_module ("right")

bench.module ("keypad", "keypad", at=(3.4, -4.6))
for pin, name in zip (range (22, 30), ("R1", "R2", "R3", "R4", "C1", "C2", "C3", "C4")):
    bench.wire (str (pin), f"keypad.{name}")

bench.closeup (1, 30)
