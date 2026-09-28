# The screen stays at its far-right home. The clock module sits above
# the breadboard, its GND and VCC from the top rails by columns 13 and 15,
# SDA and SCL from 20 and 21. Both modules stay in Lesson 33.
bench = Bench ("A clock module on pins 20 and 21, and the LCD on pins 31 to 36", columns=(1, 35))

bench.screen (text=("Date  2026-09-24", "Time    20:30:05"))
bench.home_rtc ()

# A reading to take with a multimeter: the top rails, which carry the
# Mega's 5 V to the clock and the screen. The probes go at the Mega's end,
# where the meter lies clear of the LCD. The page adds the coin cell, which
# a probe touches on the module itself.
bench.measure ("The clock's supply, on the top rails", red="T+4", black="T-4",
               expect="about 5 V", when="Mega plugged in")
