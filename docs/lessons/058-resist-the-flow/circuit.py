# Keep Lesson 57's red LED and 220 Ω resistor at their Lesson 1 home. The
# 1 kΩ and 2 kΩ resistors take the 220 Ω resistor's place only while testing.
bench = Bench ("A red LED on pin 26, through a 220 Ω resistor to GND", columns=(1, 20))

bench.home_led ("26", "red")

# Read these voltages during the long on part of each blink. The resistor
# probes stay in the same strips when the resistor is changed.
bench.measure ("Pin 26 to GND", red="26", black="GND", expect="about 5 V",
               when="LED on")
bench.measure ("Across the resistor", red="h6", black="d6", expect="about 3 V",
               when="LED on")
