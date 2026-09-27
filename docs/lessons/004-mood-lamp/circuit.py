# The button on 22 stays at its home, with its wires, as in Lessons 2 and 3.
# The RGB LED stands at its home, its red, green and blue fed from pins 5, 6
# and 7 through a 220 Ω resistor each; the red's is the red LED's resistor
# from Lesson 3, left where it was.
bench = Bench ("An RGB LED on pins 5, 6 and 7, and a button on pin 22", columns=(1, 20))

bench.home_button ("22")

bench.home_rgb_led ()

# Readings to take with a multimeter: in the orange mood, {255, 64, 0}, the
# red pin is on all the time and the green pin a quarter of it, which the
# meter shows as a quarter of 5 V; across the red LED in orange and the blue
# LED in blue, the two colors' different voltages.
bench.measure ("The red pin, at 255", red="5", black="GND", expect="about 5 V",
               when="Orange")
bench.measure ("The green pin, at 64", red="6", black="GND", expect="about 1.25 V",
               when="Orange")
bench.measure ("Across the red LED", red="c6", black="GND", expect="about 2 V", when="Orange")
bench.measure ("Across the blue LED", red="c11", black="GND", expect="about 3.2 V",
               when="Blue")
