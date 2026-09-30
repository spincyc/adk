# The ultrasonic sensor on 14 and 15 sits at its home above the Mega, the
# place it keeps in Lesson 21; the top − rail it takes GND from is joined
# to the bottom one by the link at the far end. The gauge's LEDs stand at
# their homes, red on 26, yellow on 27 and green on 28, and the active
# buzzer on 12 at its home, its wire passing over the sensor.
bench = Bench ("An ultrasonic sensor on pins 14 and 15, red, yellow and green LEDs on 26, 27 "
               "and 28, and an active buzzer on 12", columns=(1, 63))

bench.home_ultrasonic ()

for pin, color in (("26", "red"), ("27", "yellow"), ("28", "green")):
    bench.home_led (pin, color)

bench.home_buzzer ("active")

bench.closeup (1, 37)

# Readings to take with a multimeter, with a book standing still in front of
# the sensor, or nothing there at all.
bench.measure ("The yellow light's pin", red="27", black="GND", expect="about 5 V",
               when="a book 30 cm away")
bench.measure ("Across the buzzer", red="i33", black="b33", expect="about 4.8 V",
               when="a book 5 cm away")
bench.measure ("Across the green LED", red="b18", black="b19", expect="about 3.2 V",
               when="nothing within 50 cm")
