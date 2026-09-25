# The Secret Door. The power module at the right end feeds both pairs of
# rails with 5 V: the screen, at its home, takes its power from the top
# rails, and the servo latch, at its home below the board, from B+53 and
# B-54; the Mega's GND joins them at B-3. The RFID reader keeps its place
# from Lesson 34 and the tap sensor its place from Lesson 35, below the
# Mega, wired the same way. The active buzzer stands beside the screen in
# column 51, pin 12's wire coming over the top.
bench = Bench ("The Secret Door: an LCD on pins 31 to 36, an RFID reader on the SPI pins and 45, "
               "a tap sensor on A12, a servo latch on 44 and an active buzzer on 12", columns=(1, 63))

bench.power_module ("right", top="5V", bottom="5V")
bench.screen (text=("Secret Door", "Card or knock..."))

bench.module ("rfid", at=(1.65, 4.17), facing="up")
bench.wire ("3.3V", "rfid.3.3V", via=[(1.6, 2.8), (2.1, 2.8)])
bench.wire ("GND.long", "rfid.GND", via=[(4.25, 2.4), (4.25, 3.0), (2.3, 3.0)])
bench.wire ("53", "rfid.SDA", via=[(4.35, 3.1), (2.8, 3.1)])
bench.wire ("52", "rfid.SCK", via=[(4.45, 3.2), (2.7, 3.2)])
bench.wire ("51", "rfid.MOSI", via=[(4.55, 3.3), (2.6, 3.3)])
bench.wire ("50", "rfid.MISO", via=[(4.65, 3.4), (2.5, 3.4)])
bench.wire ("45", "rfid.RST", via=[(4.75, 3.5), (2.2, 3.5)])

bench.module ("sensor", name="tap", at=(0.43, 3.62), pins=["S", "+", "−"], label="tap sensor",
              facing="up")
bench.wire ("A12", "tap.S", via=[(3.5, 2.95), (0.85, 2.95)])
bench.wire ("5V.power", "tap.+", via=[(1.7, 2.9), (0.75, 2.9)])
bench.wire ("GND.power", "tap.−", via=[(1.8, 2.85), (0.65, 2.85)])

bench.module ("servo", at=(9.85, 3.45), facing="up")
bench.wire ("44", "servo.signal", via=[(4.85, 1.9), (4.85, 3.9), (9.75, 3.9), (9.75, 3.3), (10.5, 3.3)])
bench.wire ("servo.+", "B+53")
bench.wire ("servo.−", "B-54")

bench.wire ("12", "j51", via=[(1.7, -1.85), (10.4, -1.85)])
bench.buzzer ("f51", "e51", kind="active")
bench.wire ("a51", "B-51")

# Readings to take with a multimeter, the power module switched on.
bench.measure ("The latch's supply, on the bottom rails", red="B+49", black="B-49",
               expect="about 5 V", when="Power module on")
bench.measure ("The buzzer's pin during a beep", red="12", black="GND", expect="about 4.5 V",
               when="A long beep")
