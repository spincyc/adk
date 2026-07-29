# Resource probes

Resource probes preserve aggregate firmware evidence that no single lesson
example can represent. They are compile-only fixtures, not hardware acceptance
evidence.

Run the Lessons 052--054 infrared probe with:

```sh
make ir-resource-check
```

The command starts from the canonical Lesson 054 sketch and injects
`ir_translator_maximum_composition.inc`, which adds the promoted Lesson 025
receiver footprint. A fresh Mega 2560 build must measure 21,864 B flash and
3,531 B static SRAM.

The same command compiles `ir_translator_object_size.cpp` with the AVR compiler
and reads its symbol size with `avr-nm`; `InertIrTranslator` must occupy 407 B.
It also compiles the relevant sources with `-fno-lto -fstack-usage`. The
conservative live-stack bound is 888 B:

```text
522 B loop
+ 210 B InertIrTranslator::prepareTranslation
+  89 B KnownIrEmissionPolicy::prepare
+  64 B interrupt reserve
+   3 B return-address allowance
```

The script uses tools discovered beside the compiler recorded by
`arduino-cli`, checks every result exactly, and removes its temporary build
directory.
