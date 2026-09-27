# The ultrasonic sensor sits above the Mega just right of pins 14 and 15,
# the place it keeps in Lesson 21: 5 V from the inner 5V pin at the top of
# the long header, GND from the top − rail, which the link at the far end
# joins to the bottom one. The gauge's LEDs stand at their homes, red on 26
# in column 6, yellow on 27 in 12 and green on 28 in 18, and the active
# buzzer on 12 at its home in column 34, its wire passing over the sensor.
bench = Bench ("An ultrasonic sensor on pins 14 and 15, red, yellow and green LEDs on 26, 27 "
               "and 28, and an active buzzer on 12", columns=(1, 63))

bench.home_ultrasonic ()

for pin, color in (("26", "red"), ("27", "yellow"), ("28", "green")):
    bench.home_led (pin, color)

bench.home_buzzer ("active", via=[(1.69, -1.47), (8.7, -1.47)])

bench.closeup (1, 37)

# Readings to take with a multimeter, with a book standing still in front of
# the sensor, or nothing there at all.
bench.measure ("The yellow light's pin", red="27", black="GND", expect="about 5 V",
               when="a book 30 cm away")
bench.measure ("Across the buzzer", red="i34", black="b34", expect="about 4.5 V",
               when="a book 5 cm away")
bench.measure ("Across the green LED", red="b18", black="b19", expect="about 3.2 V",
               when="nothing within 50 cm")
