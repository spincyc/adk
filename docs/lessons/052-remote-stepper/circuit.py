# Two boards, each with its LoRa modem at the bridge's home, on Serial3
# (pins 14 and 15), its VDD from the Mega's 3.3V pin.
#
# Board A: the screen at its home, as in Lesson 13, and the knob on A0 at
# its home in columns 39 to 41. The
# four-digit display would cover the modem's divider, so the screen shows
# the angles.
#
# Board B: the stepper's driver at its home, as in Lesson 31, on the
# bottom rails, which the power module beside the board feeds at 5 V;
# nothing uses the top rails.
knob = Bench ("Board A: a knob on A0, the LCD on pins 31 to 36, and a LoRa modem on pins 14 "
              "and 15, its VDD from the Mega's 3.3V pin", columns=(1, 62), sketch="Knob")

knob.screen (text=("Knob says 90°", "Arrived: 90°"))
knob.home_modem ()
knob.home_knob ()

# Readings to take with a multimeter: the knob's middle leg, whose voltage
# is the angle it asks for. The black probe goes in B-40, beside
# the knob and clear of the LCD.
knob.measure ("The knob at 90°", red="A0", black="B-40", expect="about 1.25 V",
              when="the screen says Knob says 90°")
knob.measure ("The knob at 180°", red="A0", black="B-40", expect="about 2.5 V",
              when="the screen says Knob says 180°")

turntable = Bench ("Board B: a stepper motor's driver on pins A8 to A11, powered from the "
                   "breadboard power module, and a LoRa modem on pins 14 and 15, its VDD from "
                   "the Mega's 3.3V pin", columns=(1, 63), sketch="Turntable")

turntable.power_module ()
turntable.home_modem ()
turntable.home_stepper ()

boards = {"A": knob, "B": turntable}
