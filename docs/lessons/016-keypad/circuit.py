# Lesson 13's screen, wired exactly as before, with its six signal wires
# rising a little further right, so that the keypad's eight wires from
# pins 22 to 29 rise beside the Mega between the header and them, up to
# the keypad above the gap.
bench = Bench ("A 4×4 keypad on pins 22 to 29, with the screen from Lesson 13", columns=(1, 30))

bench.screen (text=("12 x 34", "= 408"), risers=(4.8, 0.05))

bench.home_keypad ()

# Readings to take with a multimeter: the screen's four data wires, D7 to
# D4, which keep the second half of the last character sent. The black
# probe goes in the column of the backlight's K, which is GND, since the
# bottom − rail lies under the screen.
bench.measure ("D7, from pin 36", red="36", black="c24", expect="0 V", when="just after typing 5")
bench.measure ("D6, from pin 35", red="35", black="c24", expect="about 5 V",
               when="just after typing 5")
bench.measure ("D5, from pin 34", red="34", black="c24", expect="0 V", when="just after typing 5")
bench.measure ("D4, from pin 33", red="33", black="c24", expect="about 5 V",
               when="just after typing 5")
