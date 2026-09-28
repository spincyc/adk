"""The circuit model's own checks, on small made-up circuits.

    python3 tests/circuits.py

A circuit.py that could never work must stop the site, and tests/pins.py
must see what each pin does. These build circuits that break the rules one
at a time, and some that keep them, and say which failed.
"""

import os
import sys
import tempfile

ROOT = os.path.dirname (os.path.dirname (os.path.abspath (__file__)))
sys.path.insert (0, os.path.join (ROOT, "docs", "_theme"))

from bench import Bench, hole_words, load, pin_words  # noqa: E402
from drawing import Drawing  # noqa: E402

failures = []


# The red LED on pin 26, as Lesson 1 builds it.
def blink (bench, pin="26", hole="j6"):
    bench.wire (pin, hole)
    bench.resistor ("220 Ω", "g6", "e6")
    bench.led ("red", anode="b6", cathode="b7")
    bench.wire ("a7", "B-7")
    return bench


def button (bench, pin="22"):
    bench.wire (pin, "j2")
    bench.button (2)
    bench.wire ("a4", "B-4")
    return bench


def check (name, build, error=None, draw=False):
    try:
        result = build (Bench ("test", columns=(1, 20)))
        result.finish ()
        if draw:
            Drawing (result).svg ()
    except ValueError as raised:
        if error is None or error not in str (raised):
            failures.append (f"{name}: raised {raised}")
        return None
    if error:
        failures.append (f"{name}: should have raised {error!r}")
    return result


def expect (name, got, wanted):
    if got != wanted:
        failures.append (f"{name}: {got!r}, not {wanted!r}")


# What finish () refuses.
check ("a good circuit", blink)
check ("a wire in the wrong hole", lambda b: blink (b, hole="j8"), "pin 26 reaches nothing")
check ("a pin on the + rail", lambda b: blink (b).wire ("27", "T+12"), "shorted to 5V")
check ("a pin on the − rail", lambda b: blink (b).wire ("27", "B-12"), "shorted to GND")
check ("two legs in one strip",
       lambda b: b.wire ("26", "j6").resistor ("220 Ω", "g6", "i6").led ("red", anode="b6",
                                                                          cathode="b7"),
       "share column 6 f-j")
check ("a button's joined legs", button)
check ("a knob's legs in one row",
       lambda b: b.potentiometer ("e5", "e6", "e7"), "across the gap")
check ("a knob across the gap",
       lambda b: b.potentiometer ("f5", "d6", "f7").wire ("j5", "T-5").wire ("j7", "T+7")
       .wire ("A0", "a6"))

# What the drawing refuses.
check ("waypoints on one grid point",
       lambda b: blink (b, "26").wire ("27", "j12", via=[(5.6, 1.0), (5.62, 1.0)])
       .resistor ("220 Ω", "g12", "e12").led ("yellow", anode="b12", cathode="b13")
       .wire ("a13", "B-13"), "fall on one point", draw=True)
check ("a waypoint that turns back",
       lambda b: blink (b).wire ("27", "j12", via=[(5.6, 1.0), (5.9, 1.0), (5.7, 1.0)])
       .resistor ("220 Ω", "g12", "e12").led ("yellow", anode="b12", cathode="b13")
       .wire ("a13", "B-13"), "turns back", draw=True)

# What the pins check sees.
both = check ("an LED and a button", lambda b: button (blink (b)))
expect ("pin modes", {pin: mode for pin, (mode, _) in both.pin_modes ().items ()},
        {"26": "output", "22": "input"})
swapped = check ("an LED and a button swapped", lambda b: button (blink (b, pin="22"), pin="26"))
expect ("swapped pin modes", {pin: mode for pin, (mode, _) in swapped.pin_modes ().items ()},
        {"22": "output", "26": "input"})
expect ("SDA is pin 20", check ("SDA", lambda b: blink (b, pin="SDA")).signal_pins (), {"20"})

# Module pins by their other names, but never at the wrong voltage.
modem = Bench ("test", columns=(1, 20)).module ("lora_modem", "modem", at=(6.0, 3.45), facing="up")
expect ("a 3.3 V modem's VCC", modem.module_pin ("modem.VCC")[1].name, "VDD")
try:
    modem.module_pin ("modem.5V")
    failures.append ("a 3.3 V modem's 5V: found a pin")
except ValueError as error:
    expect ("a 3.3 V modem's 5V", str (error), "the LoRa modem's VDD takes 3.3V, not 5V")

# A two-board lesson's circuit.
TWO = '''
a = Bench ("Board A", columns=(1, 20), sketch="Blinker")
a.wire ("26", "j6").resistor ("220 Ω", "g6", "e6").led ("red", anode="b6", cathode="b7")
a.wire ("a7", "B-7")
b = Bench ("Board B", columns=(1, 20), sketch="Presser")
b.wire ("22", "j2").button (2).wire ("a4", "B-4")
boards = {BOARDS}
'''
for boards, error in (('{"A": a, "B": b}', None), ('{"B": b, "A": a}', "in order"),
                      ('{"A": a}', "in order")):
    with tempfile.NamedTemporaryFile ("w", suffix=".py", delete=False) as file:
        file.write (TWO.replace ("{BOARDS}", boards))
    try:
        loaded = load (file.name)
        if error:
            failures.append (f"boards {boards}: should have raised {error!r}")
        else:
            expect ("board letters", [bench.board for bench in loaded.values ()], ["A", "B"])
    except ValueError as raised:
        if not error or error not in str (raised):
            failures.append (f"boards {boards}: raised {raised}")
    finally:
        os.unlink (file.name)

# Parts at their homes: a home_* call builds just what the lesson wrote out
# by hand. Adding the screen must not move a recurring part or its wires.
def finished (bench):
    bench.finish ()
    return bench


by_hand = finished (button (blink (Bench ("test", columns=(1, 20)))))
at_home = finished (Bench ("test", columns=(1, 20)).home_led ("26", "red").home_button ("22"))
expect ("parts at their homes", at_home.used, by_hand.used)
expect ("their connections", at_home.connections (), by_hand.connections ())

beside = Bench ("test", columns=(1, 62)).screen ()
beside.home_button ("23").home_buzzer ("active").home_rgb_led (green_resistor=13).home_knob ()
beside = finished (beside)
for hole, what in (("j8", "the button on 23"), ("f33", "the buzzer"), ("a6", "the RGB LED"),
                   ("f39", "the knob"), ("a47", "the LCD's first pin"),
                   ("a62", "the LCD's last pin")):
    expect (f"{what} beside the screen, in {hole}", hole in beside.used, True)

# Compare the complete home, including its power jumpers, on both boards.
for home, args in (("home_button", ("23",)), ("home_rgb_led", ()),
                    ("home_buzzer", ("passive",)), ("home_knob", ()),
                    ("home_divider", ("photoresistor",)), ("home_encoder", ()),
                    ("home_modem", ()), ("home_servo", ())):
    homes = []
    for screen in (False, True):
        bench = Bench ("test", columns=(1, 63))
        if screen:
            bench.screen ()
        before, wires = set (bench.used), len (bench.wires)
        getattr (bench, home) (*args)
        homes.append ((set (bench.used) - before, bench.wires[wires:]))
    expect (f"{home} keeps its holes and wires with the screen", homes[1], homes[0])

plain = Bench ("test", columns=(1, 63)).power_module ()
bridged = Bench ("test", columns=(1, 63)).power_module ()
plain.home_servo ()
bridged.home_modem ().home_servo ()
expect ("the servo stays put beside the modem",
        bridged.modules["servo"].reach_box (), plain.modules["servo"].reach_box ())
finished (bridged)

# These combinations used to force parts to move or cover their power holes.
analog = Bench ("test", columns=(1, 63))
for pin in ("26", "27", "28", "29", "30"):
    analog.home_led (pin, "red")
finished (analog.home_buzzer ("passive").home_divider ("photoresistor").home_knob ())
tilting = finished (Bench ("test", columns=(1, 63)).home_encoder ().home_gy521 ())
expect ("the encoder's power holes stay clear of the accelerometer",
        {"T-18", "T+19"} & tilting._covered (), set ())
check ("a button covers the RGB LED's normal green resistor",
       lambda b: b.home_rgb_led ().home_button ("23"), "would cover e9")
rgb_button = finished (Bench ("test", columns=(1, 63)).home_rgb_led (green_resistor=13)
                       .home_button ("23"))
expect ("moving the green resistor preserves all three LED signals",
        {pin: mode for pin, (mode, _) in rgb_button.pin_modes ().items ()},
        {"5": "output", "6": "output", "7": "output", "23": "input"})
# The green jumper and button ground crowd all three adjacent G-label
# positions. The close-up must still label that leg, using a leader.
Drawing (rgb_button).svg ("closeup")

# Automatic links are left of the LCD's body and remain standard wires
# for the generated carry-over steps and the close-up's extent.
links = finished (Bench ("test", columns=(1, 63)).home_knob ())
ground_link = next (wire for wire in links.wires
                    if {end[1] for end in wire[:2]} == {"B-41", "T-41"})
expect ("the new rail link is standard", links.is_standard (ground_link), True)
bottom = finished (Bench ("test", columns=(1, 63)).home_beam ())
positive_link = next (wire for wire in bottom.wires
                      if {end[1] for end in wire[:2]} == {"T+42", "B+42"})
expect ("the new positive rail link is standard", bottom.is_standard (positive_link), True)
lcd = finished (Bench ("test", columns=(1, 63)).screen ())
expect ("the rail links and power feed stay clear of the LCD",
        {"B-41", "T-41", "B-42", "B+42", "T+42"} & lcd._covered (), set ())
separate = finished (Bench ("test", columns=(1, 63)).power_module ("3.3V").screen ()
                     .home_modem (power="B+29"))
expect ("the screen keeps the Mega's 5 V", separate._powered ("T+", "5V"), True)
expect ("the modem keeps the power module's 3.3 V", separate._powered ("B+", "3.3V"), True)
expect ("the two positive rails are separate", separate._powered ("B+", "5V"), False)

sensing = finished (Bench ("test", columns=(1, 50)).home_ultrasonic ().home_modem ())
expect ("the modem on Serial2 beside the ultrasonic sensor",
        {"14", "15", "16", "17"} <= sensing.signal_pins (), True)

# The build's stages: a home_* call is one, named for its part and the
# pins it wires; steps in no stage are named for their first part other
# than a resistor, and a module placed among them starts another; stage ()
# names the steps after it.
def titles (bench):
    stages = []
    for _, _, step in bench.items:
        if step.stage not in stages:
            stages.append (step.stage)
    return [bench.stage_title (stage) for stage in stages]


staged = Bench ("test", columns=(1, 20)).home_led ("26", "red")
staged.wire ("27", "j12").resistor ("220 Ω", "g12", "e12")
staged.led ("yellow", anode="b12", cathode="b13").wire ("a13", "B-13")
staged.module ("lora_modem", "modem", at=(6.0, 3.45), facing="up").wire ("modem.GND", "B-15")
staged.stage ("the link").wire ("T-18", "B-18")
expect ("stage titles", titles (finished (staged)),
        ["power to the rails", "the red LED on pin 26", "the yellow LED on pin 27",
         "the LoRa modem", "the link"])
expect ("pins in words", pin_words ({"36", "A0", "9", "31", "33", "10", "32", "34", "35"}),
        "pins 9, 10, 31–36 and A0")
expect ("a pin in words", pin_words ({"A3"}), "pin A3")

# A rail's hole in words, never as B-7, whose B reads as row b: in running
# text, and in the steps table as the rail with its column beneath.
expect ("a rail's hole in words", hole_words ("B-7"), "the bottom − rail by column 7")
expect ("a hole in words", hole_words ("j6"), "j6")
railed = finished (blink (Bench ("test", columns=(1, 20))))
expect ("a rail in the steps", railed.items[-1][2].places,
        (("a7", ""), ("bottom − rail", "by column 7")))

if failures:
    sys.exit ("tests/circuits.py:\n  " + "\n  ".join (failures))
