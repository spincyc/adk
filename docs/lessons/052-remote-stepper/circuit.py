# Two boards, each with its LoRa modem at the bridge's home, on Serial3
# (pins 14 and 15), its VDD from the Mega's 3.3V pin.
#
# Board A: the screen at its home, as in Lesson 13, and the knob on A0 at
# its home beside the screen, A0's wire coming round below the modem. The
# four-digit display would cover the modem's divider, so the screen shows
# the angles.
#
# Board B: the stepper's driver at its home, as in Lesson 31, on the power
# module's bottom rails at 5 V; the top jumper is off, as nothing uses the
# top rails.
knob = Bench ("Board A: a knob on A0, the LCD on pins 31 to 36, and a LoRa modem on pins 14 "
              "and 15, its VDD from the Mega's 3.3V pin", columns=(1, 62), sketch="Knob")

knob.screen (text=("Knob says 90°", "Arrived: 90°"))
knob.home_modem ()
knob.home_knob (via=[(2.19, 5.35), (11.1, 5.35)])
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

turntable.power_module ()
turntable.home_modem ()
turntable.home_stepper ()
turntable.closeup (1, 63)

boards = {"A": knob, "B": turntable}
