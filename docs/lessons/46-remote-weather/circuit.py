# Each board keeps its LoRa modem at the bridge home, as in Lesson 45:
# below the board under columns 42-47, aerial down, its TXD up into f44
# beside RX3 (pin 15) in j44, TX3 (pin 14) reaching its RXD in c46 through
# 1 kΩ across the gap and 2 kΩ down to the − rail, its GND in B-42, and its
# VDD on the Mega's own 3.3V pin.
#
# Board A, the garden: Lesson 14's thermometers at their homes, the DHT11
# and the 18B20 above the board and the thermistor's divider on A2, here in
# column 33 because the light sensor's divider takes its home in column 40;
# A1's wire runs outside A2's, so they never cross. The green LED on 28 at
# its home in column 18 shows the link.
garden = Bench ("Board A, the garden: a DHT11 on pin 16, an 18B20 on pin 17, a thermistor "
                "divider on A2, a light divider on A1, a green LED on pin 28 and a LoRa modem on "
                "pins 14 and 15", columns=(1, 50), sketch="Garden")


def bridge_home (bench):
    bench.module ("lora_modem", "modem", at=(9.415, 3.45), facing="up")
    bench.wire ("modem.GND", "B-42")
    bench.wire ("3.3V", "modem.VDD", via=[(1.59, 5.3), (10.3, 5.3), (10.3, 2.8)])
    bench.wire ("14", "j46", via=[(3.15, -1.95), (9.9, -1.95)])
    bench.resistor ("1 kΩ", "g46", "e46")
    bench.resistor ("2 kΩ", "a46", "B-46")
    bench.wire ("modem.RXD", "c46", color="grey")
    bench.wire ("modem.TXD", "f44", color="purple", via=[(9.7, 2.0)])
    bench.wire ("15", "j44", via=[(3.25, -1.85), (9.7, -1.85)])


bridge_home (garden)

garden.wire ("28", "j18")
garden.resistor ("220 Ω", "g18", "e18")
garden.led ("green", anode="b18", cathode="b19")
garden.wire ("a19", "B-19")

garden.module ("sensor", "probe", at=(7.68, -1.15), label="18B20")
garden.wire ("17", "probe.S")
garden.wire ("probe.+", "T+27")
garden.wire ("probe.−", "T-28")

garden.module ("dht11", "dht", at=(8.58, -1.27))
garden.wire ("16", "dht.S")
garden.wire ("dht.+", "T+36")
garden.wire ("dht.−", "T-37")

garden.wire ("j33", "T+33")
garden.thermistor ("f33", "e33")
garden.wire ("A2", "a33", via=[(2.39, 2.85), (8.55, 2.85)])
garden.resistor ("10 kΩ", "c33", "c36")
garden.wire ("a36", "B-36")

garden.wire ("j40", "T+40")
garden.photoresistor ("f40", "e40")
garden.wire ("A1", "a40", via=[(2.29, 2.95), (9.25, 2.95)])
garden.resistor ("10 kΩ", "c40", "c43")
garden.wire ("a43", "B-43")

# Readings to take with a multimeter on Board A: the light divider's middle,
# which Board B shows as a percentage, in room light and covered.
garden.measure ("The light divider's middle, on A1", red="A1", black="GND", expect="about 2.5 V",
                when="room light: Board B shows about Light 50%")
garden.measure ("The light divider's middle, covered", red="A1", black="GND",
                expect="about 0.5 V", when="a finger over the photoresistor")

# Board B, indoors: the course's screen at its home, and the clock module on
# its side above it, as in Lesson 32. The RGB LED has the same shape as at
# its home beside the screen, but the modem has columns 42-47, so it stands
# just past them: its legs in a48, a51 and a53, its common leg in B-49.
indoors = Bench ("Board B, indoors: the LCD on pins 31 to 36, a clock module on 20 and 21, an RGB "
                 "LED on 5, 6 and 7 and a LoRa modem on pins 14 and 15", columns=(1, 56),
                 sketch="Indoors")

bridge_home (indoors)

indoors.screen (text=("Air 21.5°C  45%", "Heard   14:32:05"))
indoors.module ("rtc", at=(6.5, -1.4), facing="right")
indoors.wire ("rtc.GND", "T-29")
indoors.wire ("rtc.VCC", "T+30")
indoors.wire ("20", "rtc.SDA", via=[(3.75, -1.6), (8.5, -1.6), (8.5, -0.8)])
indoors.wire ("21", "rtc.SCL", via=[(3.85, -1.5), (8.4, -1.5), (8.4, -0.9)])

indoors.wire ("5", "j48", via=[(2.45, -2.05), (10.1, -2.05)])
indoors.wire ("6", "j51", via=[(2.35, -2.15), (10.4, -2.15)])
indoors.wire ("7", "j53", via=[(2.25, -2.25), (10.6, -2.25)])
indoors.resistor ("220 Ω", "g48", "e48")
indoors.resistor ("220 Ω", "g51", "e51")
indoors.resistor ("220 Ω", "g53", "e53")
indoors.rgb_led (red="a48", common="B-49", green="a51", blue="a53")

boards = {"A": garden, "B": indoors}
