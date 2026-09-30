# Each board keeps its LoRa modem at the bridge home, as in Lesson 47.
#
# Board A, in the nursery: Lesson 47's tripwires and red LED go, and the
# green LED on 28 stays to show the link. The light sensor's divider comes
# back to its home on A1, as in Lesson 46. The sound sensor lies below the
# Mega where the PIR was, powered from the power header, its AO on A5; its
# DO stays unconnected. The water sensor lies beside it, its S on A6 and
# its + on A7, the pin beside it, which powers it only while it is read;
# its − goes to the power header's other GND.
nursery = Bench ("Board A, in the nursery: a sound sensor on A5, a water sensor on A6 powered "
                 "from A7, a light divider on A1, a green LED on pin 28 and a LoRa modem on "
                 "pins 14 and 15", columns=(1, 50), sketch="Nursery")

nursery.home_modem ()

nursery.home_led ("28", "green")

nursery.module ("sensor", name="sound", at=(1.42, 3.8), pins=("AO", "G", "+", "DO"),
                label="sound sensor", facing="up")
nursery.wire ("5V.power", "sound.+")
nursery.wire ("GND.power", "sound.G")
nursery.wire ("A5", "sound.AO")

nursery.module ("sensor", name="water", at=(2.35, 3.8), pins=("−", "+", "S"),
                label="water sensor", facing="up")
nursery.wire ("A6", "water.S")
nursery.wire ("A7", "water.+")
nursery.wire ("GND.power2", "water.−")

nursery.home_divider ("photoresistor")

# Board B, with the parent: Lesson 47's screen and clock module stay, and the
# IR receiver stays to hush the alarm. The passive buzzer on pin 10 takes
# the active buzzer's place in column 33. The LED matrix returns to its
# home, with its usual GND wire into B-5.
parent = Bench ("Board B, with the parent: the LCD on pins 31 to 36, a clock module on 20 and 21, "
                "an IR receiver on pin 2, a passive buzzer on pin 10, an LED matrix on pins 47 "
                "to 49 and a LoRa modem on pins 14 and 15", columns=(1, 56), sketch="Parent")

parent.home_modem ()

parent.screen (text=("Quiet  Lit  Dry", "Loud at 02:14:07"))
parent.home_rtc ()

parent.module ("ir_receiver", name="receiver", at=(8.89, -1.7))
parent.wire ("2", "receiver.S")
parent.wire ("receiver.+", "T+39")
parent.wire ("receiver.−", "T-37")

parent.home_buzzer ("passive")

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
parent.home_matrix (pixels=[row[::-1] for row in reversed (BARS)])

# Readings to take with a multimeter on Board B: the buzzer's pin while the
# leak alarm sounds, and once POWER has hushed it.
parent.measure ("Pin 10, the leak alarm sounding", red="10", black="B-39", expect="about 2.2 V",
                when="the water sensor in water")
parent.measure ("Pin 10, hushed", red="10", black="B-39", expect="0 V",
                when="after POWER, still in water")

boards = {"A": nursery, "B": parent}
