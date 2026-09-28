# Lesson 14's screen and DHT11 stay in place; the thermistor and 18B20
# go. The RGB LED, active buzzer and alarm knob take their ordinary homes
# to the left of the screen, so none needs a special screen-side position.
bench = Bench ("A weather station: the DHT11 on pin 16, the RGB LED on pins 5 to 7, the knob on A0 "
               "and the buzzer on pin 12, with the screen from Lesson 13", columns=(1, 62))

bench.screen (text=("23°C 45% Comfy", "Alarm at 30°C"))

bench.home_dht11 ()

bench.home_rgb_led ()

bench.home_buzzer ("active")

bench.home_knob ()

bench.closeup (1, 63)

# Readings to take with a multimeter: the alarm knob's wiper, and the red
# and green pins of the comfort light, which remember the mood.
bench.measure ("The alarm knob's wiper, on A0", red="A0", black="GND", expect="about 3.4 V",
               when="the screen says Alarm at 30°C")
bench.measure ("Pin 6, the light's green", red="6", black="GND", expect="about 5 V",
               when="Comfy, the light green")
bench.measure ("Pin 5, the light's red", red="5", black="GND", expect="about 5 V",
               when="Hot, even once it has cooled to 25 °C")
