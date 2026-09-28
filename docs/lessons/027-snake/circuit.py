# Lesson 26's matrix and joystick stay where they were, and the passive
# buzzer joins them at its home in column 33.
bench = Bench ("The joystick and LED matrix of Lesson 26, and a passive buzzer on pin 10 "
               "through 220 Ω", columns=(1, 40))

# The matrix is drawn turned half round, so its picture is given upside down.
SNAKE = ["........",
         "........",
         ".####...",
         "....#...",
         "....###.",
         "........",
         "..#.....",
         "........"]

bench.home_matrix (pixels=[row[::-1] for row in reversed (SNAKE)])

bench.home_joystick ()

bench.home_buzzer ("passive")
bench.closeup (1, 40)

# Readings to take with a multimeter while a long note plays: the pin's
# average, and how the buzzer and the resistor share it.
bench.measure ("Pin 10 during a note", red="10", black="GND", expect="about 2.5 V",
               when="a long note")
bench.measure ("Across the buzzer", red="i33", black="b33", expect="about 0.2 V",
               when="a long note")
bench.measure ("Across the resistor", red="b33", black="GND", expect="about 2.3 V",
               when="a long note")
