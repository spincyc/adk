# The ultrasonic sensor sits above the Mega just right of pins 14 and 15,
# the place it keeps in Lesson 21: 5 V from the inner 5V pin at the top of
# the long header, GND from the top − rail, which the link at the far end
# joins to the bottom one. The gauge's LEDs stand at their homes, red on 26
# in column 6, yellow on 27 in 12 and green on 28 in 18, and the active
# buzzer on 12 at its home in column 34, its wire passing over the sensor.
bench = Bench ("An ultrasonic sensor on pins 14 and 15, red, yellow and green LEDs on 26, 27 "
               "and 28, and an active buzzer on 12", columns=(1, 63))

bench.module ("ultrasonic", name="sensor", at=(2.715, -1.2))
bench.wire ("14", "sensor.Trig")
bench.wire ("15", "sensor.Echo")
bench.wire ("sensor.VCC", "5V.long")
bench.wire ("sensor.GND", "T-5")

for pin, color, column in (("26", "red", 6), ("27", "yellow", 12), ("28", "green", 18)):
    bench.wire (pin, f"j{column}")
    bench.resistor ("220 Ω", f"g{column}", f"e{column}")
    bench.led (color, anode=f"b{column}", cathode=f"b{column + 1}")
    bench.wire (f"a{column + 1}", f"B-{column + 1}")

bench.wire ("12", "j34", via=[(1.69, -1.47), (8.7, -1.47)])
bench.buzzer ("f34", "e34", kind="active")
bench.wire ("a34", "B-34")

bench.closeup (1, 37)

# Readings to take with a multimeter, with a book standing still in front of
# the sensor, or nothing there at all.
bench.measure ("The yellow light's pin", red="27", black="GND", expect="about 5 V",
               when="a book 30 cm away")
bench.measure ("Across the buzzer", red="i34", black="b34", expect="about 4.5 V",
               when="a book 5 cm away")
bench.measure ("Across the green LED", red="b18", black="b19", expect="about 3.2 V",
               when="nothing within 50 cm")
