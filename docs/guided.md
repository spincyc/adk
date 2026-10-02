# A guided route through both courses

Build the projects in their numbered order. At the pauses below, try an
electricity investigation that explains a circuit you have met. The two
courses keep their own numbers and build instructions; this route is a
choice, not a prerequisite for either course. Return to this page after
each step. The [project map](course.md) and
[electricity syllabus](electricity/index.md) show each path in full.

Start by reading [Safety](safety.md). E01 needs only the Mega, USB power,
LED and resistor; [Getting started](start.md) shows the exact setup. Set
up the Arduino IDE before project Lesson 1. E02 and later measured work
need a DC voltmeter. The electricity syllabus lists extra parts before
each module.

At every switch between courses, unplug USB and any other power source.
On the destination page, open **Starting from an empty board or another
lesson** for the complete steps; **Build from empty** walks through them
one at a time. Clear the previous build completely, including its power
wires, then follow every step. They restore the standard rail feeds and
each part's home connections. If the page already starts from an empty
board, its ordinary steps are complete. Use the ordinary keep/take-out
steps only when coming directly from the previous lesson in the same course.

1. **[E01–E03: one DC loop](lessons/056-close-the-loop/index.md).** Open
   the return, measure across each part, and change the resistor. If you
   have no meter yet, do E01 and come back for E02–E03 later.
2. **[Lessons 1–3: first light](lessons/001-blink/index.md).** Your first
   program now switches a path you have seen work steadily.
3. **[E09–E10: diode and transistor](lessons/064-one-way-diode/index.md).**
   Test the diode's direction and switch a separate path with the S8050:
   the two parts Lesson 3 put beside its buzzer. Only E10's measured
   comparison needs a meter; without one, still swap its base resistor
   and watch the LED. Use E09's complete build after clearing Lesson 3.
4. **[Lessons 4–6: color and sound](lessons/004-mood-lamp/index.md).**
   Use Lesson 4's complete build: E10's button was connected to 5 V and
   the transistor; Lesson 4 needs it connected to pin 22 and GND.
5. **[E04–E05: series and parallel](lessons/059-resistors-in-series/index.md).**
   Predict what two parts share and what happens when a second branch
   joins; E05 adds a 10 Ω resistor. Use E04's complete build after Lesson 6.
6. **[Lessons 7–9: the analog world](lessons/007-dimmer/index.md).**
7. **[E06: Tap a divider](lessons/061-tap-a-divider/index.md).** Measure
   the knob's middle voltage, then try
   [Load a divider](electricity/challenges.md#loaded-divider) to see it
   change when another part draws from it. Use E06's complete build after
   Lesson 9.
8. **[Lessons 10–12: digits and time](lessons/010-dice/index.md).**
9. **[E07–E08: stored charge](lessons/062-charge-a-capacitor/index.md).**
   Time a capacitor's voltage change, then compare that physical delay
   with the timer in Lesson 12. Use E07's complete build after Lesson 12.
10. **[Lessons 13–15: words and weather](lessons/013-hello-lcd/index.md).**
11. **[E12: Give a coil a safe path](lessons/067-coil-diode/index.md).** It
    has complete steps from an empty board; you may skip scope-based E11.
12. **[Lessons 16–36: keys, motion, sensors, games and projects](lessons/016-keypad/index.md).**
    Along the way, you can try
    [E19–E21: logic and memory](lessons/074-nand-logic/index.md), which
    use two inexpensive logic chips. E19 begins from an empty board.
    Use the complete build on whichever project lesson you return to.
13. **[Lessons 37–42: radio links](lessons/037-fm-radio/index.md).**
14. **[E22–E24: Mega signals](lessons/077-sampling/index.md).** Sample a
    knob voltage, smooth PWM, and send a byte through a wire. E22 and E24
    have complete starts; E23 needs a capacitor and can be skipped.
15. **[Lessons 43–54: two-board projects](lessons/043-the-bridge/index.md),
    then [Lesson 55: Reliability Meter](lessons/055-reliability-meter/index.md).**

## Optional instrument extension

After E10, [E11](lessons/066-inductor-current/index.md) shows an inductor's
changing current on a scope. After E08,
[E13–E15](lessons/068-alternating-current/index.md) measure isolated
low-voltage AC. E16–E18 then cover
[gain, feedback and supply ripple](lessons/071-amplifier-gain/index.md);
E17 needs only a meter if you build its follower without the generator.
The others need the isolated
generator, battery-powered scope and extra parts specified in the
[electricity equipment table](electricity/index.md#equipment-gates); read
the [scope and generator primer](electricity/skills.md#scope-and-generator)
before the first one.
They are optional to the kit-and-meter route; E13 starts from an empty
board so you can enter this extension when the instruments are available.
E17 also offers the complete build when you enter there with only a meter.
After any extension, use the complete build on the lesson you return to.

When a reading surprises you, use the short
[measurement routine](electricity/skills.md) and
[diagnosis guide](electricity/diagnose.md).
The [schematic exercise](electricity/schematics.md) lets you point to
the same three nodes in a drawing and on a breadboard before powering it.
