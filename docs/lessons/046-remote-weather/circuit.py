# Each board keeps its LoRa modem at the bridge home, as in Lesson 45, on
# Serial3 (pins 14 and 15), its VDD on the Mega's own 3.3V pin.
#
# Board A, indoors: Lesson 45's screen stays at its home, and the clock
# module joins it, as in Lesson 32. The RGB LED uses its ordinary home,
# legs in a6, a9 and a11, with the common leg in B-7.
indoors = Bench ("Board A, indoors: the LCD on pins 31 to 36, a clock module on 20 and 21, an RGB "
                 "LED on 5, 6 and 7 and a LoRa modem on pins 14 and 15", columns=(1, 56),
                 sketch="Indoors")

indoors.home_modem ()

indoors.screen (text=("Air 21.5°C  45%", "Heard   14:32:05"))
indoors.home_rtc ()

indoors.home_rgb_led ()

# Board B, the garden: Lesson 14's thermometers, the DHT11 and the 18B20
# at their homes and the thermistor's divider on A2, here in column 33
# because the light sensor's divider on A1 takes the home they share; A1's
# wire runs outside A2's, so they never cross. The green LED on 28 at its
# home shows the link.
garden = Bench ("Board B, the garden: a DHT11 on pin 16, an 18B20 on pin 17, a thermistor "
                "divider on A2, a light divider on A1, a green LED on pin 28 and a LoRa modem on "
                "pins 14 and 15", columns=(1, 50), sketch="Garden")

garden.home_modem ()

garden.home_led ("28", "green")

garden.home_ds18b20 ()

garden.home_dht11 ()

garden.wire ("j33", "T+33")
garden.thermistor ("f33", "e33")
garden.wire ("A2", "a33", via=[(2.55, 2.85), (8.55, 2.85)])
garden.resistor ("10 kΩ", "c33", "c36")
garden.wire ("a36", "B-36")

garden.home_divider ("photoresistor")

# Readings to take with a multimeter on Board B: the light divider's middle,
# which Board A shows as a percentage, in room light and covered.
garden.measure ("The light divider's middle, on A1", red="A1", black="GND", expect="about 2.5 V",
                when="room light: Board A shows about Light 50%")
garden.measure ("The light divider's middle, covered", red="A1", black="GND",
                expect="about 0.5 V", when="a finger over the photoresistor")

boards = {"A": indoors, "B": garden}
