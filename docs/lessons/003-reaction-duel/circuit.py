# Lesson 2's red LED and button stay in place; a second pair and the
# active buzzer join them at their homes. The LEDs' signal wires stay
# below row j, leaving the button wires clear above them.
bench = Bench ("A two-player reaction game: buttons on 22 and 23, LEDs on 26 to 28, "
               "and an active buzzer on 12", columns=(1, 40))

bench.home_button ("22")

bench.home_button ("23")

bench.home_led ("26", "red", via=[(4.45, 0.95), (4.45, 1.15)])

bench.home_led ("27", "yellow", via=[(4.35, 1.00), (4.35, 1.25)])

bench.home_led ("28", "green", via=[(4.25, 1.15), (4.25, 1.35), (7.05, 1.35)])

bench.home_buzzer ("active")

# Readings to take with a multimeter at Go, while the yellow light is on and
# the buzzer sounds: the LED shares the pin's 5 V with its resistor, the
# buzzer takes it all.
bench.measure ("Across the yellow LED", red="b12", black="b13", expect="about 2 V",
               when="Yellow light on, at Go")
bench.measure ("Across the buzzer", red="i33", black="b33", expect="about 4.8 V",
               when="Buzzer sounding, at Go")
