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

from bench import Bench, load  # noqa: E402
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

if failures:
    sys.exit ("tests/circuits.py:\n  " + "\n  ".join (failures))
