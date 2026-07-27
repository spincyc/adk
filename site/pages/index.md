# ADK

Deterministic, resource-owning C++ circuit components and an evidence-centered
Arduino Mega 2560 course.

The first-class API is **host verified and experimental**. It compiles for the
Mega 2560; physical acceptance is still recorded lesson by lesson. The original
preview is frozen under [Legacy](legacy/index.md).

## Start visibly

Lesson 001 begins with `DigitalOutput`, because a known diagnostic signal makes
later input, debounce, and reconfiguration work observable.

```cpp
adk::Runtime       runtime;
adk::DigitalOutput led (runtime.resources (), LED_BUILTIN);

led.initialize ();
led.write      (adk::Level::High);
```

- [Install and build](start.md)
- [Lessons 001–003](lessons/index.md)
- [Supported API](api-supported.md)
- [Course map](course.md)
- [Safety](safety.md)
- [GitHub repository](https://github.com/spincyc/adk)

Every third lesson builds a multi-component project. The first is a deterministic
reaction timer; lesson 006 is Simon.
