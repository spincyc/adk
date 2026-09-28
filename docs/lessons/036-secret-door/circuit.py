# The Secret Door: the screen at the far end, the RFID reader and tap
# sensor at their homes below the Mega, and the active buzzer in column
# 33. The servo keeps its fixed position below the board, with + at B+35
# and − at B-36. The power module feeds only the bottom rails, at B+42
# and B-42; the screen runs from the Mega's 5 V on the top rails.
bench = Bench ("The Secret Door: an LCD on pins 31 to 36, an RFID reader on the SPI pins and 45, "
               "a tap sensor on A12, a servo latch on 44 and an active buzzer on 12", columns=(1, 63))

bench.power_module ()
bench.screen (text=("Secret Door", "Card or knock..."))

bench.home_rfid ()

bench.home_tap ()

bench.home_servo ()

bench.home_buzzer ("active")

# Readings to take with a multimeter, the power module switched on.
bench.measure ("The latch's supply, on the bottom rails", red="B+40", black="B-40",
               expect="about 5 V", when="Power module on")
bench.measure ("The buzzer's pin during a beep", red="12", black="GND", expect="about 4.5 V",
               when="A long beep")
