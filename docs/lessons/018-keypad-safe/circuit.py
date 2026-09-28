# Lesson 17's power module, screen and servo stay as they are; its knob
# goes. The keypad comes back from Lesson 16, in the same place, and the
# active buzzer stands at its home beside the screen. As in Lesson 17, the
# module lies beside the board and feeds only the bottom rails, for the
# servo, and the Mega's 5V feeds the top rails, for the screen; the Mega's
# GND joins them all at B-3.
bench = Bench ("A keypad safe: the keypad on pins 22 to 29, the screen on 31 to 36, a servo latch "
               "on 44 powered by the power module, and the buzzer on 12", columns=(1, 63))

bench.power_module ()

bench.screen (text=("Locked. Code?", "****"), risers=(4.8, 0.05))

bench.home_keypad ()

bench.home_buzzer ("active", via=[(1.60, -0.25), (10.4, -0.25)])

bench.home_servo (via=[(4.55, 1.90), (4.55, 3.01), (10.5, 3.01)])

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
