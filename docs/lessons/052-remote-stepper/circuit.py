# Two boards, each with its LoRa modem at the bridge's home: below the
# board under columns 42-47, aerial down, its TXD up into f44 beside RX3
# (pin 15) in j44, TX3 (pin 14) into j46 and down through 1 kΩ and 2 kΩ
# to the − rail, the modem's RXD into c46 between them, its GND into B-42
# and its VDD from the Mega's 3.3V pin.
#
# Board A: the screen at its home, as in Lesson 13, and the knob on A0 at
# its home beside the screen, e57 to e59, A0's wire coming round below
# the modem. The four-digit display would cover the modem's divider, so
# the screen shows the angles.
#
# Board B: the stepper's driver below the Mega, as in Lesson 31, on the
# power module's bottom rails at 5 V; the top jumper is off, as nothing
# uses the top rails.
def modem (bench):
    bench.module ("lora_modem", "modem", at=(9.415, 3.45), facing="up")
    bench.wire ("modem.GND", "B-42")
    bench.wire ("3.3V", "modem.VDD")
    bench.wire ("14", "j46")
    bench.resistor ("1 kΩ", "g46", "e46")
    bench.resistor ("2 kΩ", "a46", "B-46")
    bench.wire ("modem.RXD", "c46", color="grey")
    bench.wire ("modem.TXD", "f44", color="purple")
    bench.wire ("15", "j44")


knob = Bench ("Board A: a knob on A0, the LCD on pins 31 to 36, and a LoRa modem on pins 14 "
              "and 15, its VDD from the Mega's 3.3V pin", columns=(1, 62), sketch="Knob")

knob.screen (text=("Knob says 90°", "Arrived: 90°"))
modem (knob)
knob.potentiometer ("e57", "e58", "e59")
knob.wire ("a57", "B-57")
knob.wire ("A0", "a58", via=[(2.19, 5.35), (11.1, 5.35)])
knob.wire ("d59", "T+61")
knob.closeup (30, 62)

# Readings to take with a multimeter: the knob's middle leg, whose voltage
# is the angle it asks for.
knob.measure ("The knob at 90°", red="A0", black="GND", expect="about 1.25 V",
              when="the screen says Knob says 90°")
knob.measure ("The knob at 180°", red="A0", black="GND", expect="about 2.5 V",
              when="the screen says Knob says 180°")

turntable = Bench ("Board B: a stepper motor's driver on pins A8 to A11, powered from the "
                   "breadboard power module, and a LoRa modem on pins 14 and 15, its VDD from "
                   "the Mega's 3.3V pin", columns=(1, 63), sketch="Turntable")

turntable.power_module ("right", top="off", bottom="5V")
modem (turntable)
turntable.module ("stepper", at=(3.1, 4.5), facing="up")
turntable.wire ("A8", "stepper.IN1", via=[(3.09, 3.5), (4.45, 3.5)])
turntable.wire ("A9", "stepper.IN2", via=[(3.19, 3.3), (4.35, 3.3)])
turntable.wire ("A10", "stepper.IN3", via=[(3.29, 3.1), (4.25, 3.1)])
turntable.wire ("A11", "stepper.IN4", via=[(3.39, 2.9), (4.15, 2.9)])
turntable.wire ("stepper.+", "B+5", via=[(3.56, 3.65), (5.8, 3.65)])
turntable.wire ("stepper.−", "B-6", via=[(3.66, 3.75), (5.9, 3.75)])
turntable.closeup (1, 63)

boards = {"A": knob, "B": turntable}
