# The Secret Door. The power module beside the board feeds only the bottom
# rails, from B+61 and B-61: the servo latch, at its home below the board,
# takes its 5 V from the bottom rails, while the screen, at its home, runs
# from the Mega's 5V on the top rails; the Mega's GND joins them all at B-3.
# The RFID reader comes back to its place from Lesson 34 and the tap sensor
# keeps its place from Lesson 35, below the Mega, wired the same way. The
# active buzzer stands at its home beside the screen, pin 12's wire coming
# over the top.
bench = Bench ("The Secret Door: an LCD on pins 31 to 36, an RFID reader on the SPI pins and 45, "
               "a tap sensor on A12, a servo latch on 44 and an active buzzer on 12", columns=(1, 63))

bench.power_module ()
bench.screen (text=("Secret Door", "Card or knock..."))

bench.home_rfid ()

bench.home_tap ()

bench.home_servo (via=[(4.85, 1.85), (4.85, 3.9), (9.75, 3.9), (9.75, 3.3), (10.5, 3.3)])

bench.home_buzzer ("active", via=[(1.60, -1.85), (10.4, -1.85)])

# Readings to take with a multimeter, the power module switched on.
bench.measure ("The latch's supply, on the bottom rails", red="B+49", black="B-49",
               expect="about 5 V", when="Power module on")
bench.measure ("The buzzer's pin during a beep", red="12", black="GND", expect="about 4.5 V",
               when="A long beep")
