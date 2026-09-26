# Each board keeps its LoRa modem at the bridge home, as in Lesson 47.
#
# Board A, in the nursery: Lesson 47's tripwires and red LED go, and the
# green LED on 28 stays to show the link. The light sensor's divider comes
# back to its home in column 40 on A1, as in Lesson 46. The sound sensor
# lies below the Mega where the PIR was, powered from the power header, its
# AO on A5; its DO stays unconnected. The water sensor lies beside it, its S
# on A6 and its + on A7, the pin beside it, which powers it only while it
# is read; its − goes to the power header's other GND.
nursery = Bench ("Board A, in the nursery: a sound sensor on A5, a water sensor on A6 powered "
                 "from A7, a light divider on A1, a green LED on pin 28 and a LoRa modem on "
                 "pins 14 and 15", columns=(1, 50), sketch="Nursery")


def bridge_home (bench, power=((1.59, 5.3), (10.3, 5.3), (10.3, 2.8))):
    bench.module ("lora_modem", "modem", at=(9.415, 3.45), facing="up")
    bench.wire ("modem.GND", "B-42")
    bench.wire ("3.3V", "modem.VDD", via=list (power))
    bench.wire ("14", "j46", via=[(3.15, -1.95), (9.9, -1.95)])
    bench.resistor ("1 kΩ", "g46", "e46")
    bench.resistor ("2 kΩ", "a46", "B-46")
    bench.wire ("modem.RXD", "c46", color="grey")
    bench.wire ("modem.TXD", "f44", color="purple", via=[(9.7, 2.0)])
    bench.wire ("15", "j44", via=[(3.25, -1.85), (9.7, -1.85)])


bridge_home (nursery, power=((1.59, 2.9), (1.1, 2.9), (1.1, 5.3), (10.3, 5.3), (10.3, 2.8)))

nursery.wire ("28", "j18")
nursery.resistor ("220 Ω", "g18", "e18")
nursery.led ("green", anode="b18", cathode="b19")
nursery.wire ("a19", "B-19")

nursery.module ("sensor", name="sound", at=(1.42, 3.8), pins=("AO", "G", "+", "DO"),
                label="sound sensor", facing="up")
nursery.wire ("5V.power", "sound.+")
nursery.wire ("GND.power", "sound.G")
nursery.wire ("A5", "sound.AO")

nursery.module ("sensor", name="water", at=(2.35, 3.8), pins=("S", "+", "−"),
                label="water sensor", facing="up")
nursery.wire ("A6", "water.S")
nursery.wire ("A7", "water.+")
nursery.wire ("GND.power2", "water.−")

nursery.wire ("j40", "T+40")
nursery.photoresistor ("f40", "e40")
nursery.wire ("A1", "a40", via=[(2.29, 2.95), (9.25, 2.95)])
nursery.resistor ("10 kΩ", "c40", "c43")
nursery.wire ("a43", "B-43")

# Board B, with the parent: Lesson 47's screen and clock module stay, and the
# IR receiver stays to hush the alarm. The passive buzzer takes the active
# buzzer's place in column 51, on pin 10, with its 220 Ω resistor down to
# the − rail. The LED matrix lies at its home below the gap between the
# Mega and the board, but the screen's contrast knob has B-5, so its GND
# comes one hole nearer the Mega, into B-4.
parent = Bench ("Board B, with the parent: the LCD on pins 31 to 36, a clock module on 20 and 21, "
                "an IR receiver on pin 2, a passive buzzer on pin 10, an LED matrix on pins 47 "
                "to 49 and a LoRa modem on pins 14 and 15", columns=(1, 56), sketch="Parent")

bridge_home (parent, power=((1.59, 6.35), (10.3, 6.35), (10.3, 2.8)))

parent.screen (text=("Quiet  Lit  Dry", "Cried   02:14:07"))
parent.module ("rtc", at=(6.5, -1.4), facing="right")
parent.wire ("rtc.GND", "T-29")
parent.wire ("rtc.VCC", "T+30")
parent.wire ("20", "rtc.SDA", via=[(3.75, -1.6), (8.5, -1.6), (8.5, -0.8)])
parent.wire ("21", "rtc.SCL", via=[(3.85, -1.5), (8.4, -1.5), (8.4, -0.9)])

parent.module ("ir_receiver", name="receiver", at=(8.89, -1.7))
parent.wire ("2", "receiver.S")
parent.wire ("receiver.+", "T+39")
parent.wire ("receiver.−", "T-40")

parent.wire ("10", "j51", via=[(1.9, -2.3), (10.4, -2.3)])
parent.buzzer ("f51", "e51", kind="passive")
parent.resistor ("220 Ω", "a51", "B-51")

# The matrix is drawn turned half round, so its picture is given upside
# down: a bar graph of the last eight half seconds' sound.
BARS = ["........",
        "........",
        "......#.",
        "......#.",
        "...#..##",
        "..##..##",
        ".####.##",
        "########"]
parent.module ("matrix", at=(4.22, 3.9), facing="up",
               pixels=[row[::-1] for row in reversed (BARS)])
parent.wire ("48", "matrix.CLK")
parent.wire ("49", "matrix.CS")
parent.wire ("47", "matrix.DIN")
parent.wire ("B-4", "matrix.GND")
parent.wire ("5V.long", "matrix.VCC")

# Readings to take with a multimeter on Board B: the buzzer's pin while the
# leak alarm sounds, and once POWER has hushed it.
parent.measure ("Pin 10, the leak alarm sounding", red="10", black="B-39", expect="about 2.2 V",
                when="the water sensor in water")
parent.measure ("Pin 10, hushed", red="10", black="B-39", expect="0 V",
                when="after POWER, still in water")

boards = {"A": nursery, "B": parent}
