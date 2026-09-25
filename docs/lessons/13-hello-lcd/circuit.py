# The course's screen at its home, from column 5: the contrast knob, then
# the LCD in row a with its power, contrast and signal wires, and its
# backlight's resistor at the far end. It stays in these holes in every
# lesson that uses it.
bench = Bench ("An LCD1602 on pins 31 to 36, with a contrast knob and its backlight through 220 Ω",
               columns=(1, 30))

bench.screen (text=("Hello, LCD!", ""))

# Readings to take with a multimeter: the contrast voltage the knob sets,
# and the 5 V the backlight shares with its resistor.
bench.measure ("The contrast voltage, on V0", red="d6", black="B-4", expect="about 0.6 V",
               when="letters sharp")
bench.measure ("Across the backlight's resistor", red="h23", black="c23", expect="about 2 V",
               when="backlight on")
bench.measure ("Across the backlight", red="c23", black="c24", expect="about 3 V",
               when="backlight on")
