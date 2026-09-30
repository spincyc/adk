# The red LED keeps its Lesson 1 home. Its only return to the Mega is
# through the bottom − rail; the + rails are unused.
bench = Bench ("A red LED on pin 26, through a 220 Ω resistor to GND", columns=(1, 20))

bench.home_led ("26", "red")
