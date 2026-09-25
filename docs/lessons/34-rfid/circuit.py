# The RGB LED sits at the Mega's end of the board: each color's pin reaches
# the top half, a 220 Ω resistor bridges the middle gap, and the LED's legs
# stand in row a, its common leg to the − rail. The RFID reader lies below
# the Mega on seven wires, powered from the 3.3V pin.
bench = Bench ("An RC522 RFID reader on the SPI pins 50 to 53 and pin 45, powered from 3.3 V, "
               "and an RGB LED on pins 5, 6 and 7", columns=(1, 24))

bench.wire ("GND", "B-3")
bench.wire ("5", "j8")
bench.wire ("6", "j5")
bench.wire ("7", "j2")
bench.resistor ("220 Ω", "h8", "e8")
bench.resistor ("220 Ω", "h5", "e5")
bench.resistor ("220 Ω", "h2", "e2")
bench.rgb_led (red="a8", common="a6", green="a5", blue="a2")
bench.wire ("b6", "B-6")
bench.module ("rfid", at=(2.2, 3.6), facing="up")
bench.wire ("3.3V", "rfid.3.3V")
bench.wire ("45", "rfid.RST", color="yellow")
bench.wire ("GND", "rfid.GND")
bench.wire ("50", "rfid.MISO", color="purple")
bench.wire ("51", "rfid.MOSI", color="white")
bench.wire ("52", "rfid.SCK", color="brown")
bench.wire ("53", "rfid.SDA", color="grey")
bench.closeup (1, 22)
