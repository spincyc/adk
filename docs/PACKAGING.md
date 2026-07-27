# Arduino packaging

ADK is both a normal C++ library and an Arduino library. The repository root is
the Arduino library root. A release must install from Library Manager without
generated files, submodules, symlinks, or repository-specific build tools.

The first-class layout is:

```text
adk/
├── library.properties
├── keywords.txt
├── src/
│   ├── Adk.h
│   ├── core/
│   ├── input/
│   └── output/
├── examples/
│   ├── Lesson004DigitalOutput/
│   │   └── Lesson004DigitalOutput.ino
│   └── Project001ReactionTimer/
│       └── Project001ReactionTimer.ino
├── docs/
├── lessons/
├── legacy/
└── tests/
```

Only `src/` contains library code. Arduino compiles it recursively and adds only
`src/` to the include path. Public examples include `<Adk.h>`; they do not use
relative paths or files from `tests/`, `lessons/`, or `legacy/`.

`legacy/` preserves the preview interface for reference. Nothing in `src/`,
`examples/`, or the first-class documentation includes it. It is not a
compatibility layer.

## Metadata contract

Keep `library.properties` at the repository root with UTF-8 `key=value` lines:

```properties
name=Adk
version=0.1.0
author=Kevin Shanahan
maintainer=Kevin Shanahan <kevin.p.shanahan@gmail.com>
sentence=Deterministic C++ components for learning Arduino circuits.
paragraph=Provides lifecycle-managed input and output components, host tests, and progressive projects for the Arduino Mega 2560.
category=Device Control
url=https://github.com/spincyc/adk
architectures=avr
includes=Adk.h
```

Rules:

- Use normalized `Adk`, not the fully capitalized `ADK`.
- Make `includes` match the primary header's spelling and case.
- Keep `sentence` at 80 characters or fewer and `paragraph` at 256 or fewer.
- Use a valid semantic version with no `v` in `library.properties`.
- Declare only architectures compiled in CI. Use `avr` until another family is
  continuously tested.
- Add `depends` only for required Library Manager dependencies. ADK should prefer
  no external runtime dependencies.
- Check that `Adk` remains unique, case-insensitively, in the Library Manager
  index before the first submission.
- Do not add `.development`, executable binaries, symlinks, or submodules.

The primary header is a small include surface. It may include supported public
component headers, but it contains no definitions beyond those justified as
constants, templates, or trivial compile-time operations.

`keywords.txt` is optional. If present, keep it limited to public types,
functions, constants, and the `Adk.h` include. It is editor highlighting, not an
API catalog.

## Examples and lessons

`examples/` is the canonical source for every supported Arduino sketch. Each
example follows the Arduino sketch rule exactly:

```text
examples/<SketchName>/<SketchName>.ino
```

Use descriptive CamelCase names. Number lesson and project examples so the IDE
menu preserves curriculum order. Every example:

- compiles for `arduino:avr:mega`;
- uses only supported public headers;
- is deterministic unless its lesson explicitly studies physical entropy;
- documents pins, voltage, and required components in a terse header comment;
- leaves hardware in the component's documented safe state;
- demonstrates one preferred usage path;
- has a corresponding host test when behavior can be simulated.

Do not maintain a second editable copy under `lessons/`. Lesson Markdown and TeX
list or excerpt the canonical file from `examples/`. The site build may copy
examples into its download tree, but copied files are generated artifacts and
must never be edited or committed as source.

If a lesson needs an intentionally incomplete exercise, store it as a distinct,
compilable example such as `Lesson005DigitalInputExercise`; do not generate it by
editing the canonical solution. Keep answers in a separate
`Lesson005DigitalInputSolution` example when publishing the answer is useful.

The source relationship is:

```text
examples/*.ino ── compile ──> Mega 2560 firmware
       │
       ├── include/excerpt ──> HTML lesson
       ├── include/excerpt ──> PDF lesson
       └── copy at site build ──> downloadable sketch
```

This makes drift detectable: one sketch feeds firmware validation and both
documentation formats. HTML should carry links, API details, corrections, and
searchable code. PDFs should emphasize bench procedure, worksheets, diagrams,
and printable observations; they need not duplicate the HTML prose.

## Local gates

Install the AVR core once:

```sh
arduino-cli core update-index
arduino-cli core install arduino:avr
```

Compile every canonical example from the repository root:

```sh
for sketch in examples/*/*.ino
do
    directory=${sketch%/*}
    arduino-cli compile \
        --fqbn arduino:avr:mega \
        --library . \
        "$directory"
done
```

Run Arduino Lint in three modes:

```sh
arduino-lint \
    --compliance strict \
    --project-type library \
    .

arduino-lint \
    --compliance strict \
    --library-manager submit \
    --project-type library \
    .

arduino-lint \
    --compliance strict \
    --library-manager update \
    --project-type library \
    .
```

Use `submit` before the initial registry request and `update` for every later
release. Both belong in CI; warnings are reviewed even when the command exits
successfully. Pin the Arduino CLI, AVR core, Arduino Lint, and Actions versions
used by CI.

Also test the installed archive, not only `--library .`:

1. Create a clean release archive from the intended tag.
2. Install it into a temporary Arduino CLI user directory.
3. Compile every installed example for `arduino:avr:mega`.
4. Confirm no build depends on ignored or untracked repository files.

Record flash and static RAM sizes. Fail CI when an established example exceeds
its explicit budget without an approved update.

## Release gate

Before tagging:

1. Run host tests, formatting, documentation, site, Arduino compile, and all
   Arduino Lint gates.
2. Confirm `library.properties` has the release version and stable library name.
3. Confirm the archive contains `library.properties`, `src/`, `examples/`,
   `LICENSE`, and `README.md`.
4. Confirm it contains no build output, `.development`, symlinks, submodules, or
   executable binaries.
5. Compile every example from a clean archive installation.
6. Commit the version change.
7. Create and push the matching release tag, conventionally `v0.1.0`.

The version value is `0.1.0`; the Git tag may be `v0.1.0`. Never move or replace
a published tag. Increment the version and publish a new tag instead.

For initial Library Manager inclusion, submit the repository URL to the
[Arduino library registry][registry]. After acceptance, its indexer discovers
new compliant tags automatically. It checks for releases periodically; consult
the registry log before assuming a delayed version failed.

## Authoritative references

- [Arduino library specification][spec]
- [Arduino sketch specification][sketch]
- [Arduino Lint command reference][lint-command]
- [Arduino Lint library rules][lint-rules]
- [Library Manager requirements and release process][registry]

[spec]: https://docs.arduino.cc/arduino-cli/library-specification/
[sketch]: https://docs.arduino.cc/arduino-cli/sketch-specification/
[lint-command]: https://arduino.github.io/arduino-lint/latest/commands/arduino-lint/
[lint-rules]: https://arduino.github.io/arduino-lint/latest/rules/library/
[registry]: https://github.com/arduino/library-registry/blob/main/FAQ.md
