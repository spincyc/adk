# Lesson 17's power module and servo stay as they are; its knob goes. The
# screen and the keypad come back from Lesson 16, in the same places, and
# the active buzzer stands beside the screen at its home in column 51. The
# power module feeds everything on the board, the screen included, so the
# Mega's 5V stays off the rails; its GND still joins them at B-3.
bench = Bench ("A keypad safe: the keypad on pins 22 to 29, the screen on 31 to 36, a servo latch "
               "on 44 and the buzzer on 12, all powered by the power module", columns=(1, 63))

bench.power_module ("right", top="5V", bottom="5V")

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
bench.wire ("44", "servo.signal")
