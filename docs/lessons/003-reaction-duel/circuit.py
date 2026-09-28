# Lesson 2's build stays as it is: the buttons on 22 and 23 and the red and
# yellow LEDs on 26 and 27. The green LED on 28 and the active buzzer on 12
# join them at their homes. The LEDs' wires reach row j from nested lanes
# just below it; pin 12's comes over the top of the board.
bench = Bench ("A two-player reaction game: buttons on 22 and 23, LEDs on 26 to 28, "
               "and an active buzzer on 12", columns=(1, 40))

bench.home_button ("22")

bench.home_button ("23")

bench.home_led ("26", "red", via=[(4.45, 0.95), (4.45, 1.15)])

bench.home_led ("27", "yellow", via=[(4.35, 1.00), (4.35, 1.25)])

bench.home_led ("28", "green", via=[(4.25, 1.15), (4.25, 1.35), (7.05, 1.35)])

bench.home_buzzer ("active", via=[(1.60, 0.45), (8.65, 0.45)])

# Readings to take with a multimeter at Go, while the yellow light is on and
# the buzzer sounds: the LED shares the pin's 5 V with its resistor, the
# buzzer takes it all.
bench.measure ("Across the yellow LED", red="b12", black="b13", expect="about 2 V",
               when="Yellow light on, at Go")
bench.measure ("Across the buzzer", red="i34", black="b34", expect="about 4.5 V",
               when="Buzzer sounding, at Go")
