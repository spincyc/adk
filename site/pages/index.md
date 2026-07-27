# ADK

Build observable Arduino circuits from small, deterministic C++ components.
ADK is both a resource-owning library and an evidence-centered course for the
Arduino Mega 2560.

<nav class="landing-actions" aria-label="Primary">
  <a class="landing-primary" href="start/">Start with the command line</a>
  <a href="lessons/">Browse lessons 001–018</a>
</nav>

> **Current status:** the first-class API is experimental. Host tests and Mega
> 2560 compilation pass for published material; physical acceptance remains
> open until each lesson has a recorded bench result.

## Learn in visible steps

Lesson 001 starts with a known diagnostic signal. Later lessons reuse that
observability while adding input, timing, PWM, sound, analog measurement, and
displays. Every third lesson composes the preceding ideas into a deterministic
project.

```cpp
adk::Runtime       runtime;
adk::DigitalOutput led (runtime.resources (), LED_BUILTIN);

led.initialize ();
led.write      (adk::Level::High);
```

[Follow the course map](course.md), inspect the
[supported API](api-supported.md), or go directly to the
[lesson index](lessons/index.md). Projects currently include a reaction timer,
Simon, an adaptive night light, and a tabletop traffic junction.

## Use it as a library

The public components use explicit ownership, deterministic time, transactional
initialization, and RAII cleanup without exceptions or heap allocation. Build,
test, compile every Mega example, generate the lesson PDFs, and preview the site
from the command line.

[Install and build ADK](start.md) or review the
[component catalog](components.md).

## Explore the research

Research tracks apply the same explicit-state methodology beyond lesson
circuits. The transparent USB and HDMI mesh explores routing a remote console
over a shared switched network; it is a design and host-model effort, not
working endpoint hardware.

[Read the mesh roadmap](projects/mesh-roadmap.md) or browse the
[USB phase-one research](projects/usb3-matrix.md).

## Verify before wiring

Start with the [safety rules](safety.md). Every circuit provides a non-Serial
observation path and separates software evidence from physical bench evidence.
The imported preview remains frozen under [Legacy](legacy/index.md).

[View the source on GitHub](https://github.com/spincyc/adk).
