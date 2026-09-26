# Lesson 17's power module, screen and servo stay as they are; its knob
# goes. The keypad comes back from Lesson 16, in the same place, and the
# active buzzer stands beside the screen at its home in column 51. As in
# Lesson 17, the module's top jumper is off, so the Mega's 5V feeds the top
# rails for the screen, and the module feeds only the bottom rails, for the
# servo; the Mega's GND joins them all at B-3.
bench = Bench ("A keypad safe: the keypad on pins 22 to 29, the screen on 31 to 36, a servo latch "
               "on 44 powered by the power module, and the buzzer on 12", columns=(1, 63))

bench.power_module ("right", top="off", bottom="5V")

bench.screen (text=("Locked. Code?", "****"), risers=(4.8, 0.05))

bench.module ("keypad", "keypad", at=(3.04, -4.95))
turns = (0.5, 0.45, 0.4, 0.35, None, 0.4, 0.45, 0.5)
for index, name in enumerate (("R1", "R2", "R3", "R4", "C1", "C2", "C3", "C4")):
    riser, height = 4.25 + 0.05 * index, 0.8 + 0.05 * index
    socket = 4.05 + 0.1 * index
    via = [(riser, height)]
    if turns[index]:
        via += [(riser, turns[index]), (socket, turns[index])]
    bench.wire (str (22 + index), f"keypad.{name}", via=via)

bench.wire ("12", "j51", via=[(1.7, -0.25), (10.4, -0.25)])
bench.buzzer ("f51", "e51", kind="active")
bench.wire ("a51", "B-51")

bench.module ("servo", "servo", at=(9.85, 3.45), facing="up")
bench.wire ("servo.+", "B+53")
bench.wire ("servo.−", "B-54")
bench.wire ("44", "servo.signal", via=[(4.55, 1.95), (4.55, 3.01), (10.5, 3.01)])

# Readings to take with a multimeter: the screen's VDD, which takes the
# Mega's 5 V from the top + rail, and its D4 wire, which shows a star's
# code while the safe is locked and the digit's own while choosing. The
# bottom − rail lies under the screen, so the black probe goes in a column
# the screen joins to GND: RW's, then the backlight's K.
bench.measure ("The screen's 5 V, from the Mega", red="b10", black="c13",
               expect="about 5 V", when="power module on or off")
bench.measure ("D4 after a star", red="33", black="c24", expect="0 V",
               when="locked, just after typing any digit")
bench.measure ("D4 after a 5", red="33", black="c24", expect="about 5 V",
               when="choosing a code, just after typing 5")
