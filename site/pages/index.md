# ADK

Build observable Arduino circuits from small, deterministic C++ objects. ADK is
a resource-owning library and an evidence-centered course for the Arduino Mega
2560—built, tested, and published entirely from the command line.

<nav class="landing-actions" aria-label="Getting started">
  <a class="landing-primary" href="start/">Build from the command line</a>
  <a href="lessons/">Follow lessons 001–030</a>
  <a href="components/">Browse components</a>
</nav>

> **Current boundary:** lessons 001–030 are published and host verified, and
> their canonical examples compile for the Mega 2560. Physical acceptance
> remains open until each lesson has a recorded bench result.
> [Lesson 030](lessons/030.md) is the current published capstone. Lesson 031 is
> the next queued component; lessons 031–081 remain the retained kit expansion.

## One method, three ways in

<div class="landing-paths">
  <section>
    <h3>Build</h3>
    <p>Install stock Arch tools, run deterministic host tests, compile every
    Mega example, and build the course from Make targets.</p>
    <p><a href="start/">Start with the CLI →</a></p>
  </section>
  <section>
    <h3>Learn</h3>
    <p>Add one ownership or circuit idea at a time. Every third lesson combines
    earlier components into a replayable project.</p>
    <p><a href="course/">Open the course map →</a></p>
  </section>
  <section>
    <h3>Compose</h3>
    <p>Use clean RAII endpoints, components, and behavior engines in another
    Arduino project without adopting an IDE workflow.</p>
    <p><a href="components/">Inspect components →</a></p>
  </section>
</div>

## Make behavior observable

Lesson 001 starts with a known diagnostic signal. Later lessons reuse that
observability while adding input, timing, PWM, sound, analog measurement, and
displays. Serial output may explain evidence, but it is never the circuit’s only
proof.

```cpp
adk::Runtime       runtime;
adk::DigitalOutput led (runtime.resources (), LED_BUILTIN);
adk::Status        status = led.initialize ();

if (status.ok ())
{
    status = led.write (adk::Level::High);
}

if (!status.ok ())
{
    led.shutdown ();
}
```

Public objects use explicit ownership, deterministic time, transactional
initialization, and RAII cleanup without exceptions or heap allocation. The
published projects are the reaction timer, Simon, adaptive night light,
traffic junction, environmental station, inert access trainer, bench rover,
greenhouse controller, telemetry console, and a physically inert show-cue
simulator with continuity-gated cues and replayable audit evidence.

[Read the supported API](api-supported.md), begin with
[Lesson 001](lessons/001.md), or download the printable companion from each
lesson page.

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
The imported preview remains frozen and unsupported under
[Legacy](legacy/index.md).

[View the source on GitHub](https://github.com/spincyc/adk).
