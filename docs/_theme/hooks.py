"""Build lesson pages from their parts.

A lesson page is docs/lessons/NN-name/index.md, with a circuit.py beside it
describing the build and a sketch in examples/. Markers in the page are
replaced when the site builds:

    <!-- bench -->         the pencil drawing of the whole bench
    <!-- closeup -->       a close-up of the breadboard
    <!-- steps -->         the build, step by step
    <!-- connections -->   which Mega pin reaches which part
    <!-- sketch -->        the example sketch, exactly as it compiles

Any page may also use <!-- arcs --> for the course as cards, and
<!-- course --> for the course as a table. The course comes from course.yml,
which also builds the Course navigation from the lessons that exist.

The build fails if the sketch and the circuit disagree about any pin, so a
drawing can never show a wire the code doesn't use, or miss one it does.
"""

import os
import re
import sys

import yaml
from mkdocs.exceptions import PluginError

sys.path.insert (0, os.path.dirname (__file__))

from bench import Bench  # noqa: E402

ROOT = os.path.dirname (os.path.dirname (os.path.dirname (os.path.abspath (__file__))))

# How many leading constructor arguments of each part are pins, and the bus
# pins a part uses without naming them.
PIN_ARGUMENTS = {
    "Led": 1, "Button": 1, "Switch": 1, "Buzzer": 1, "Speaker": 1, "RgbLed": 3,
    "DigitalOutput": 1, "DigitalInput": 1, "AnalogInput": 1, "PwmOutput": 1,
    "ShiftRegister": 3, "SevenSegment": 3, "FourDigitDisplay": 7, "Lcd": 6,
    "LedMatrix": 3, "Servo": 1, "Stepper": 4, "Motor": 3, "Relay": 1, "Keypad": 8,
    "RotaryEncoder": 2, "Joystick": 2, "Thermistor": 1, "Ultrasonic": 2, "Dht11": 1,
    "IrReceiver": 1, "Ds18b20": 1, "Rfid": 2, "Rtc": 0, "Mpu6050": 0,
}
BUS_PINS = {"Rtc": {"20", "21"}, "Mpu6050": {"20", "21"}, "Rfid": {"50", "51", "52"}}

COURSE = yaml.safe_load (open (os.path.join (os.path.dirname (__file__), "course.yml"),
                               encoding="utf-8"))
LESSONS = [dict (lesson, arc=arc["arc"], number=index)
           for index, lesson in enumerate ((lesson for arc in COURSE for lesson in arc["lessons"]), 1)
           for arc in [next (a for a in COURSE if lesson in a["lessons"])]]


def written (lesson):
    return os.path.exists (os.path.join (ROOT, "docs", "lessons", lesson["slug"], "index.md"))


# The Course tab lists every arc that has a written lesson.
def on_config (config):
    for item in config["nav"]:
        if isinstance (item, dict) and "Course" in item:
            sections = []
            for arc in COURSE:
                pages = [f"lessons/{lesson['slug']}/index.md" for lesson in arc["lessons"]
                         if written (lesson)]
                if pages:
                    sections.append ({arc["arc"]: pages})
            item["Course"] = ["course.md"] + sections
    return config


def on_page_markdown (markdown, page, config, files):
    meta = page.meta
    markdown = markdown.replace ("<!-- arcs -->", arcs ())
    markdown = markdown.replace ("<!-- course -->", course_table ())
    markdown = re.sub (r"<!-- drawing (\S+) (bench|closeup) -->", drawing, markdown)
    if "lesson" not in meta:
        return markdown

    lesson = LESSONS[meta["lesson"] - 1]
    if page.file.src_uri != f"lessons/{lesson['slug']}/index.md":
        raise PluginError (f"{page.file.src_uri}: course.yml calls lesson {meta['lesson']} "
                           f"{lesson['slug']}")
    meta["arc"] = lesson["arc"]
    meta["project"] = lesson.get ("project", False)
    # Lessons put their sections in the header, leaving the page wide
    # enough for the drawings.
    meta["hide"] = ["toc"]

    folder = os.path.dirname (page.file.abs_src_path)
    bench = load_bench (os.path.join (folder, "circuit.py"))
    sketch_name = meta["sketch"]
    sketch_path = os.path.join (ROOT, "examples", sketch_name, sketch_name + ".ino")
    sketch = open (sketch_path, encoding="utf-8").read ()

    check_pins (page.file.src_uri, sketch, bench)

    replacements = {
        "bench": figure (bench.svg ("bench", "bench"), bench.title, "bench"),
        "closeup": figure (bench.svg ("closeup", "closeup"),
                           "Close-up of the breadboard. Letters name the rows, numbers the columns.",
                           "closeup"),
        "steps": steps (bench),
        "connections": connections (bench),
        "sketch": f'```cpp title="{sketch_name}.ino" linenums="1"\n{sketch.rstrip ()}\n```',
    }
    for marker, content in replacements.items ():
        markdown = markdown.replace (f"<!-- {marker} -->", content)
    return markdown


# A drawing from any lesson, for use on another page.
def drawing (match):
    slug, view = match.groups ()
    bench = load_bench (os.path.join (ROOT, "docs", "lessons", slug, "circuit.py"))
    return figure (bench.svg (view, f"{slug}-{view}"), bench.title, view)


def load_bench (path):
    scope = {"Bench": Bench}
    try:
        exec (compile (open (path, encoding="utf-8").read (), path, "exec"), scope)
    except Exception as error:
        raise PluginError (f"{path}: {error}") from error
    return scope["bench"]


def sketch_pins (sketch):
    pins = set ()
    for kind, arguments in re.findall (r"adk::(\w+)\s+\w+\s*(?:\[\s*\])?\s*\{(.*?)\};", sketch, re.S):
        if kind not in PIN_ARGUMENTS:
            continue
        values = re.findall (r"\bA\d{1,2}\b|\bLED_BUILTIN\b|\b\d+\b", arguments)
        for value in values[:PIN_ARGUMENTS[kind]]:
            pins.add ("13" if value == "LED_BUILTIN" else value)
        pins |= BUS_PINS.get (kind, set ())
    return pins


def check_pins (where, sketch, bench):
    declared = sketch_pins (sketch)
    wired = bench.signal_pins ()
    # The built-in LED is on the board itself and needs no wire.
    wired_or_built_in = wired | ({"13"} & declared)
    missing = declared - wired_or_built_in
    extra = wired - declared
    if missing or extra:
        raise PluginError (
            f"{where}: the sketch and the circuit disagree. "
            f"Declared but not wired: {sorted (missing) or 'none'}. "
            f"Wired but not declared: {sorted (extra) or 'none'}.")


def figure (svg, caption, kind):
    return (f'<figure class="bench-figure bench-{kind}" markdown="0">\n{svg}\n'
            f'<figcaption>{caption}</figcaption>\n</figure>')


def steps (bench):
    lines = [f"{index}. {step}" for index, step in enumerate (bench.steps, 1)]
    return '<div class="build-steps" markdown>\n\n' + "\n".join (lines) + "\n\n</div>"


def connections (bench):
    lines = []
    for pins, legs in bench.connections ():
        things = [f"**{pin}**" for pin in pins] + legs
        lines.append ("- " + " — ".join (things))
    return '<div class="connections" markdown>\n\n' + "\n".join (lines) + "\n\n</div>"


def link (lesson, text=None):
    text = text or lesson["title"]
    return f"[{text}](lessons/{lesson['slug']}/index.md)" if written (lesson) else text


def arcs ():
    cards = ['<div class="arcs" markdown>']
    number = 0
    for index, arc in enumerate (COURSE, 1):
        cards.append (f'<section class="arc" markdown>\n\n<p class="arc-number">{index}</p>\n\n'
                      f'### {arc["arc"]}\n')
        for lesson in arc["lessons"]:
            number += 1
            item = f"{number}. {link (lesson)}"
            if lesson.get ("project"):
                item = f'{number}. <span class="project">★ {link (lesson)}</span>'
            cards.append (item)
        cards.append ("\n</section>")
    cards.append ("</div>")
    return "\n".join (cards)


def course_table ():
    rows = []
    for arc in COURSE:
        rows += [f"\n### {arc['arc']}\n", "| | Lesson | You build |", "|--:|---|---|"]
        for lesson in arc["lessons"]:
            number = LESSONS.index (next (l for l in LESSONS if l["slug"] == lesson["slug"])) + 1
            title = link (lesson)
            if lesson.get ("project"):
                title = f"★ {title}"
            rows.append (f"| {number:02d} | {title} | {lesson['builds']} |")
    return "\n".join (rows)
