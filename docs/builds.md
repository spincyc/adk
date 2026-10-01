# Builds on real hardware

Every lesson's sketch compiles, the library's tests pass, and a check holds
each sketch to the circuit in its drawings. None of that proves a circuit
works on a real bench: a part from a different batch, a loose breadboard
hole or a mistake in a drawing only shows up when someone builds it. So far
nobody has built the lessons and recorded the result, and until they do,
what a lesson says you'll see is a careful prediction.

This page is where those builds are recorded.

## Recorded builds

No builds have been recorded yet. Yours could be the first.

## Report your build

[Open an issue on GitHub](https://github.com/spincyc/adk/issues/new) and
tell us how it went. A build that didn't work is just as useful as one
that did. Say:

- **Which lesson**: its number and title, such as *Lesson 3, Reaction
  Duel* or *E07, Charge a Capacitor*.
- **What you built it with**: a genuine Arduino Mega or a compatible board,
  your kit and its version, and any part you swapped for another.
- **What happened**: whether it did what *What you'll build* and *Upload
  it* describe, and any *Measure it* readings beside the ones the lesson
  expects.
- **Anything that was wrong or unclear**: a step, a drawing, a hole, a
  word.
- **A photo of the build**, if you like.

Keep personal details out of it: no full name, address or school, and no
faces in photos. GitHub accounts are for people 13 and older, so younger
learners can ask an adult to send the report.

## Adding a build to this page

When a report shows a lesson built, record it under *Recorded builds* in
a table like this one, a row for each build, linking its report:

```markdown
| Lesson | Built | Board and kit | Result | Report |
|---|---|---|---|---|
| Lesson 1, Blink | October 2026 | Arduino Mega 2560 Rev3, Elegoo Most Complete kit | Worked as the page says | [#N](https://github.com/spincyc/adk/issues/N) |
```

Record the builds that didn't work too, with what went wrong, and fix the
lesson. One build that worked shows that a lesson can work, not that it
works with every kit.
