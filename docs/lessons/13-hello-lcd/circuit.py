# The course's screen at its home, from column 5: the contrast knob, then
# the LCD in row a with its power, contrast and signal wires, and its
# backlight's resistor at the far end. It stays in these holes in every
# lesson that uses it.
bench = Bench ("An LCD1602 on pins 31 to 36, with a contrast knob and its backlight through 220 Ω",
               columns=(1, 30))

bench.screen (text=("Hello, LCD!", ""))
