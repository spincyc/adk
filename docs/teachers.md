# Teaching with ADK

This page is for anyone running ADK with a class or a club: how long it
takes, a plan for each arc, what the room needs, how to print the lessons
as worksheets, and ways to see what learners have understood.

!!! warning "Build each lesson before you teach it"
    Nobody has yet built the lessons on real hardware and recorded the
    result, so what each one says you'll see is a careful prediction.
    Build a lesson yourself before a class does, and
    [report your build](builds.md), whether it worked or not.

## How long it takes

Each lesson's page gives its own estimate of how long one learner, or a
pair, takes to build it, upload it and try it. Added up:

| Part of the course | Lessons | Time |
|---|---|---|
| The kit's projects | 1–36 | <!-- hours 1-36 --> |
| Radios | 37–42 | <!-- hours 37-42 --> |
| Two boards | 43–55 | <!-- hours 43-55 --> |
| **The project path** | **1–55** | **<!-- hours 1-55 -->** |
| **The electricity path** | **E01–E24** | **<!-- hours 56-79 -->** |

A class takes longer than one learner: getting kits out, tidying up and
talking about what happened all take time. Plan an arc of three lessons
as a week or two of sessions, and use the times below to fit lessons
together; a lesson longer than your session splits well after *Build it*.
The electricity path can run alongside the projects; the
[guided route](guided.md) shows where each investigation fits.

Some parts are optional. *Make it yours* and *Measure it* sections are
extras for quick finishers, and *Measure it* needs a multimeter. Lesson 41
needs an amateur radio license in the USA and Canada, and Lesson 42 needs
two Heltec boards and a phone with the Meshtastic app; a class without them
reads those lessons and carries on, as [the course map](course.md) says.

## Session plans: the project path

Each arc is a session plan: its lessons in order, the time each takes, and
the ideas each teaches, which make its objectives. The third lesson of each
arc, marked ★, is a project that brings the arc's ideas together, and makes
a good assessment.

<!-- session plan projects -->

## Session plans: the electricity path

Each module of three investigations is a session plan, in the same way.
The [syllabus](electricity/index.md#equipment-gates) lists what each module
needs beyond the kit.

<!-- session plan electricity -->

## The room and the kits

- **One kit to a pair.** Two learners share a Mega, a breadboard and a kit:
  one reads the steps and checks, the other builds, and they swap each
  lesson. [What to buy](kit.md#what-to-buy) adds up each path's parts.
- **Keep each pair's build.** Each lesson carries on from the one before:
  parts stay in their [breadboard homes](kit.md#breadboard-homes), and a
  lesson's steps say what to keep, what to take out and what to add. Store
  each pair's breadboard and Mega together between sessions, labeled, and
  the next lesson starts where the last one stopped.
- **Check the kits first.** Lesson 3 needs an S8050 transistor and a
  1N4007 diode, and Lessons 28, 30 and 50 a level shifter beyond the kits;
  [the kit page](kit.md#i2c-level-shifter) says what to look for. Count the
  2 kΩ resistors before the radio lessons.
- **Set up the computers first.** Install the Arduino IDE 2, ADK Boards and
  the library on every computer before the first lesson, as
  [Getting started](start.md) shows. The compiler is a download of 100 to
  170 MB for each computer, and a school network may block it. Chromebooks
  can't run ADK.
- **Keep a class on one version.** The site always shows the newest ADK;
  [Updating ADK](start.md#updating-adk) shows how a class can stay on one
  release for a term. Every page's footer, and every PDF's, names the
  version.
- **Safety.** Go through [Safety](safety.md) with the class before the
  first build. An adult does or watches any soldering, and nothing ever
  connects to mains.

### Radios in a classroom

Radios in one room hear each other. Before the radio lessons:

- **433 MHz (Lesson 38).** Every receiver hears every transmitter in range,
  so pairs' messages mix. Take turns, or spread the pairs apart and keep
  the aerials off.
- **LoRa modems (Lessons 40 and 43–55).** The sketches put every modem on
  network 6, so groups would hear each other. Give each group its own
  network number, from 1 to 15, and have them add it to every modem's
  settings, both boards' in a two-board lesson: `.network = 12`, as
  Lesson 43's *Make it yours* shows.
- **The law.** [Radios](safety.md#radios) says what each radio may send and
  where. Lesson 41's modules need an amateur radio license in the USA and
  Canada; Lesson 42's boards keep to the region you set.

### Two-board lessons

Lessons 43 to 55 join two Megas over radio, usually in different rooms.
Each board is a whole setup: a Mega, a breadboard, its own parts and one of
the two LoRa modems from Lesson 40. Two pairs make a good team, one pair to
a board:

- One pair's build carries on into Lesson 43 as **Board A**; the other
  pair starts **Board B** from an empty breadboard. Each board then
  carries on from the same board in the lesson before, so a team keeps its
  boards for the rest of the course. Pairs can swap boards between arcs,
  leaving the builds where they are.
- A team needs one pair of modems between its two boards, so the second
  pair's modems are spare. Lesson 51 needs a breadboard power module for
  each board, and Lessons 54 and 55 a second matrix, joystick and passive
  buzzer: [Two boards](kit.md#two-boards) lists what Board B needs.
- To put the boards in different rooms, each needs its own USB power: a
  power bank or a long cable.

## Printing lessons

Every lesson has a printable PDF, linked at the top of its page and named
in its footer with ADK's version. For a worksheet, print every page but
the last: a project lesson's answers to *Check yourself* start the last
page of its PDF, so the pages before it have the questions without them.
Each page's footer gives its number out of the total, so in the print
dialog choose pages 1 to one less than the total, and keep the last page
for yourself.

The PDFs are US Letter size, and print on A4 at 97 % with the print
dialog's *Fit to page*.

Print the lessons a class needs at the start of term, and print again
after you update ADK: the site always shows the newest lessons.

## Seeing what learners understand

- **Predictions.** Every lesson asks for a prediction before the
  experiment. Have learners write theirs down, then compare it with what
  happened. A wrong prediction they can explain afterwards is learning,
  not failure.
- **Check yourself.** The questions at the end of each project lesson make
  a short exit ticket; the answers are on the PDF's last page.
- **Explain a line.** Point at a line of the sketch, or a wire on the
  breadboard, and ask what it does and what would happen without it.
- **Find a fault.** While a pair isn't looking, take out one wire, with
  USB unplugged, or change one number in the sketch, then let them find it
  with the lesson's *If it doesn't work* table.
- **Projects.** The ★ lessons and their *Make it yours* challenges show
  what a pair can do with an arc's ideas. Ask for a short note or a photo
  of what they changed and why.
- **Measurements.** In the electricity path, each investigation's record of
  prediction, reading and explanation is the work to look at; the
  [design challenges](electricity/challenges.md) make good assessments.

The [glossary](glossary.md) lists the words the course teaches, each with
the lesson that teaches it.
