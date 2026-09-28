# Lesson 32's clock, unchanged: the screen and the clock module at their
# homes, SDA and SCL now stepping right round the knob. The knob, the
# rotary encoder, sits at its home above the Mega, on 18, 19 and 22. The
# snooze button and the passive buzzer take their homes beside the screen,
# their pin wires coming over the top. The flag is Lesson 31's stepper, its
# driver at its home as it was there; the power module beside the board
# feeds only the bottom rails, for the driver, from B+61 and B-61, so the
# screen and clock keep the Mega's 5V on the top rails. The screen's knob
# jumper takes column 5 and the LCD covers the bottom rails from column 6,
# so the driver's + and − come in nearer the Mega than Lesson 31's B+5 and
# B-6, at B+4 and B-4.
bench = Bench ("Lesson 32's clock and LCD, with a knob on pins 18, 19 and 22, a snooze button on "
               "pin 23, a passive buzzer on pin 10, and a stepper flag on pins A8 to A11 powered "
               "from the power module", columns=(1, 63))

bench.screen (text=("Time    06:58:30", "Alarm   07:00   "), risers=(4.55, 0.1))
bench.home_rtc (sda=[(3.75, 0.35), (3.95, 0.35), (3.95, -1.6), (8.5, -1.6), (8.5, -0.8)],
                scl=[(3.85, 0.45), (4.05, 0.45), (4.05, -1.5), (8.4, -1.5), (8.4, -0.9)])

bench.home_encoder (above=True, lift=0.3)

bench.home_button ("23", via=[(4.4, 0.80), (4.4, -1.7), (9.1, -1.7)])

bench.home_buzzer ("passive", via=[(1.80, -2.4), (10.4, -2.4)])

bench.power_module ()
# The driver without its usual + and − wires, which come in below; the +
# wire rises between B-3 and B-4 to reach B+4.
bench.home_stepper (powered=False)
bench.wire ("stepper.+", "B+4", via=[(3.56, 3.65), (5.65, 3.65), (5.65, 2.5)])
bench.wire ("stepper.−", "B-4", via=[(3.66, 3.75), (5.7, 3.75)])

# Readings to take with a multimeter, the power module switched on.
bench.measure ("The buzzer's pin while it rings", red="10", black="GND", expect="about 2.4 V",
               when="While a note plays")
bench.measure ("The flag's supply, on the bottom rails", red="B+57", black="B-57",
               expect="about 5 V", when="Power module on")
