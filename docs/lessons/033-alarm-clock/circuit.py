# Lesson 32's screen and clock module stay. The rotary encoder takes
# its home in row a, columns 15 to 19, on 18, 19 and 22; the snooze button
# and passive buzzer take their ordinary homes. Lesson 31's stepper driver
# returns below the Mega with its usual power wires into B+5 and B-6. The
# power module feeds only the bottom rails, at B+42 and B-42; the screen
# and clock keep the Mega's 5 V on the top rails.
bench = Bench ("Lesson 32's clock and LCD, with a knob on pins 18, 19 and 22, a snooze button on "
               "pin 23, a passive buzzer on pin 10, and a stepper flag on pins A8 to A11 powered "
               "from the power module", columns=(1, 63))

bench.screen (text=("Time    06:58:30", "Alarm   07:00   "), risers=(4.55, 0.1))
bench.home_rtc (sda=[(3.75, 0.35), (3.95, 0.35), (3.95, -1.6), (8.5, -1.6), (8.5, -0.8)],
                scl=[(3.85, 0.45), (4.05, 0.45), (4.05, -1.5), (8.4, -1.5), (8.4, -0.9)])

bench.home_encoder ()

bench.home_button ("23")

bench.home_buzzer ("passive")

bench.power_module ()
bench.home_stepper ()

# Readings to take with a multimeter, the power module switched on.
bench.measure ("The buzzer's pin while it rings", red="10", black="GND", expect="about 2 V",
               when="While a note plays")
bench.measure ("The flag's supply, on the bottom rails", red="B+40", black="B-40",
               expect="about 5 V", when="Power module on")
