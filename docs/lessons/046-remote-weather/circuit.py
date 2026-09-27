# Each board keeps its LoRa modem at the bridge home, as in Lesson 45, on
# Serial3 (pins 14 and 15), its VDD on the Mega's own 3.3V pin.
#
# Board A, the garden: Lesson 14's thermometers, the DHT11 and the 18B20
# at their homes and the thermistor's divider on A2, here in column 33
# because the light sensor's divider on A1 takes the home they share; A1's
# wire runs outside A2's, so they never cross. The green LED on 28 at its
# home shows the link.
garden = Bench ("Board A, the garden: a DHT11 on pin 16, an 18B20 on pin 17, a thermistor "
                "divider on A2, a light divider on A1, a green LED on pin 28 and a LoRa modem on "
                "pins 14 and 15", columns=(1, 50), sketch="Garden")


def bridge_home (bench):
    bench.home_modem (tx=[(3.15, -1.95), (9.9, -1.95)], rx=[(3.25, -1.85), (9.7, -1.85)],
                      txd=[(9.7, 2.0)], supply=[(1.59, 5.3), (10.3, 5.3), (10.3, 2.8)])


bridge_home (garden)

garden.home_led ("28", "green")

garden.home_ds18b20 ()

garden.home_dht11 ()

garden.wire ("j33", "T+33")
garden.thermistor ("f33", "e33")
garden.wire ("A2", "a33", via=[(2.39, 2.85), (8.55, 2.85)])
garden.resistor ("10 kΩ", "c33", "c36")
garden.wire ("a36", "B-36")

garden.home_divider ("photoresistor", via=[(2.29, 2.95), (9.25, 2.95)])

# Readings to take with a multimeter on Board A: the light divider's middle,
# which Board B shows as a percentage, in room light and covered.
garden.measure ("The light divider's middle, on A1", red="A1", black="GND", expect="about 2.5 V",
                when="room light: Board B shows about Light 50%")
garden.measure ("The light divider's middle, covered", red="A1", black="GND",
                expect="about 0.5 V", when="a finger over the photoresistor")

# Board B, indoors: the course's screen and the clock module at their
# homes, as in Lesson 32. The RGB LED has the same shape as at its home
# beside the screen, but the modem's home takes those columns, so it stands
# just past them: its legs in a48, a51 and a53, its common leg in B-49.
indoors = Bench ("Board B, indoors: the LCD on pins 31 to 36, a clock module on 20 and 21, an RGB "
                 "LED on 5, 6 and 7 and a LoRa modem on pins 14 and 15", columns=(1, 56),
                 sketch="Indoors")

bridge_home (indoors)

indoors.screen (text=("Air 21.5°C  45%", "Heard   14:32:05"))
indoors.home_rtc ()

indoors.wire ("5", "j48", via=[(2.45, -2.05), (10.1, -2.05)])
indoors.wire ("6", "j51", via=[(2.35, -2.15), (10.4, -2.15)])
indoors.wire ("7", "j53", via=[(2.25, -2.25), (10.6, -2.25)])
indoors.resistor ("220 Ω", "g48", "e48")
indoors.resistor ("220 Ω", "g51", "e51")
indoors.resistor ("220 Ω", "g53", "e53")
indoors.rgb_led (red="a48", common="B-49", green="a51", blue="a53")

boards = {"A": garden, "B": indoors}
