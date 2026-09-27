# Each board keeps its LoRa modem at the bridge home, as in Lesson 46.
#
# Board A, by the door: Lesson 46's sensors go, and the green LED on 28
# stays to show the link. The PIR, the tilt switch and the beam-break
# sensor come back at their homes from Lesson 23: the PIR below the Mega on
# A12, powered from the power header; the tilt switch in c32 and c33 on
# A14; the beam-break sensor below the board under column 36 on A15. The
# obstacle sensor's home under column 45 is the modem's now, and the tap
# sensor's would share the power header with the PIR, so they take the
# places Lesson 46's DHT11 and 18B20 had above the board, on their pins, 16
# and 17, and their top-rail holes. The red LED on 26 shows whether the den
# has armed the alarm. The modem's 3.3 V wire runs round the PIR and below
# everything, and comes up past the modem.
door = Bench ("Board A, by the door: a PIR on A12, a tilt switch on A14, a beam-break sensor on "
              "A15, an obstacle sensor on pin 16, a tap sensor on pin 17, a red LED on pin 26, a green "
              "LED on pin 28 and a LoRa modem on pins 14 and 15", columns=(1, 63), sketch="Door")


def bridge_home (bench, power=((1.59, 5.3), (10.3, 5.3), (10.3, 2.8))):
    bench.home_modem (tx=[(3.15, -1.95), (9.9, -1.95)], rx=[(3.25, -1.85), (9.7, -1.85)],
                      txd=[(9.7, 2.0)], supply=list (power))


bridge_home (door, power=((1.59, 2.9), (1.1, 2.9), (1.1, 5.3), (10.3, 5.3), (10.3, 2.8)))

for pin, color in (("26", "red"), ("28", "green")):
    door.home_led (pin, color)

door.home_pir ()

door.module ("sensor", name="tap", at=(7.68, -1.15), pins=["S", "+", "−"], label="tap sensor")
door.wire ("17", "tap.S")
door.wire ("tap.+", "T+27")
door.wire ("tap.−", "T-28")

door.tilt_switch ("c32", "c33")
door.wire ("A14", "a32")
door.wire ("a33", "B-33")

door.home_beam ()

door.module ("sensor", name="obstacle", at=(8.63, -1.15), label="obstacle sensor",
             pins=("GND", "+", "OUT", "EN"))
door.wire ("16", "obstacle.OUT")
door.wire ("obstacle.+", "T+36")
door.wire ("obstacle.GND", "T-35")

# Readings to take with a multimeter on Board A: the red LED's pin, which
# the den sets over the bridge, and the tilt switch's pin both ways up.
door.measure ("Pin 26, the armed light, disarmed", red="26", black="GND", expect="0 V",
              when="the den says Disarmed")
door.measure ("Pin 26, the armed light, armed", red="26", black="GND", expect="about 5 V",
              when="after POWER in the den")
door.measure ("The tilt switch's pin, tipped", red="A14", black="GND", expect="about 5 V",
              when="on its side")

# Board B, in the den: Lesson 46's screen and clock module stay, and the RGB
# LED goes. The active buzzer takes its place beside the screen in column
# 51, pin 12's wire coming over the top. The IR receiver's place beside the
# screen is the clock's, so it sits above the board further along, over
# columns 38 to 40, its power from the top rails below it.
den = Bench ("Board B, in the den: the LCD on pins 31 to 36, a clock module on 20 and 21, an IR "
             "receiver on pin 2, an active buzzer on pin 12 and a LoRa modem on pins 14 and 15",
             columns=(1, 56), sketch="Den")

bridge_home (den)

den.screen (text=("Armed   Door ok", "Motion  21:07:43"))
den.home_rtc ()

den.module ("ir_receiver", name="receiver", at=(8.89, -1.7))
den.wire ("2", "receiver.S")
den.wire ("receiver.+", "T+39")
den.wire ("receiver.−", "T-40")

den.home_buzzer ("active", via=[(1.69, -2.3), (10.4, -2.3)])

boards = {"A": door, "B": den}
