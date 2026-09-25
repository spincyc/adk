# The RGB LED stands at its home, as in Lesson 4: pins 5, 6 and 7 into j6,
# j9 and j11, a 220 Ω resistor across the gap above each colored leg, its
# common leg in B-7. The RFID reader lies below the middle of the Mega (its
# place in Lesson 36 too, leaving room on the left for the tap sensor): 3.3V
# comes across from the power header, and GND, 45 and the SPI pins drop
# beside the long header, nested, then run under the Mega and step down
# onto the reader's pins.
bench = Bench ("An RC522 RFID reader on the SPI pins 50 to 53 and pin 45, powered from 3.3 V, "
               "and an RGB LED on pins 5, 6 and 7", columns=(1, 30))

bench.wire ("5", "j6")
bench.wire ("6", "j9")
bench.wire ("7", "j11")
bench.resistor ("220 Ω", "g6", "e6")
bench.resistor ("220 Ω", "g9", "e9")
bench.resistor ("220 Ω", "g11", "e11")
bench.rgb_led (red="a6", common="B-7", green="a9", blue="a11")

bench.module ("rfid", at=(1.65, 4.17), facing="up")
bench.wire ("3.3V", "rfid.3.3V", via=[(1.6, 2.8), (2.1, 2.8)])
bench.wire ("GND.long", "rfid.GND", via=[(4.25, 2.4), (4.25, 3.0), (2.3, 3.0)])
bench.wire ("53", "rfid.SDA", via=[(4.35, 3.1), (2.8, 3.1)])
bench.wire ("52", "rfid.SCK", via=[(4.45, 3.2), (2.7, 3.2)])
bench.wire ("51", "rfid.MOSI", via=[(4.55, 3.3), (2.6, 3.3)])
bench.wire ("50", "rfid.MISO", via=[(4.65, 3.4), (2.5, 3.4)])
bench.wire ("45", "rfid.RST", via=[(4.75, 3.5), (2.2, 3.5)])

bench.closeup (1, 28)
