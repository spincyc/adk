# The RGB LED stands at its home, as in Lesson 4, on pins 5, 6 and 7. The
# RFID reader lies at its home below the middle of the Mega (its place in
# Lesson 36 too, leaving room on the left for the tap sensor), on the
# Mega's 3.3V, pin 45 and the SPI pins.
bench = Bench ("An RC522 RFID reader on the SPI pins 50 to 53 and pin 45, powered from 3.3 V, "
               "and an RGB LED on pins 5, 6 and 7", columns=(1, 30))

bench.home_rgb_led ()

bench.home_rfid ()

bench.closeup (1, 28)

# Readings to take with a multimeter on the RGB LED's pins.
bench.measure ("The blue leg's pin, waiting", red="7", black="GND", expect="about 0.8 V",
               when="LED glowing dim blue")
bench.measure ("The red leg's pin, for a stranger's card", red="5", black="GND",
               expect="about 5 V, falling to 0", when="As the red flash fades")
