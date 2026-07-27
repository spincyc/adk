# Packaging

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
│   ├── status.h
│   ├── resource.h
│   └── component headers and out-of-line sources
├── examples/
│   ├── Lesson001DigitalOutput/
│   │   └── Lesson001DigitalOutput.ino
│   └── Lesson003ReactionTimer/
│       └── Lesson003ReactionTimer.ino
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
version=0.3.0
author=Kevin Shanahan
maintainer=Kevin Shanahan <kevin.p.shanahan@gmail.com>
sentence=Deterministic RAII circuit components for Arduino Mega 2560 lessons.
paragraph=Provides explicit resource ownership, safe lifecycle control, deterministic I/O, validated sensor observations, operator interfaces, and replayable project engines.
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
examples/*/*.ino ── compile ──> Mega 2560 firmware
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
arduino-cli core install arduino:avr@1.8.8
```

This is the reviewed core used by CI, `make bootstrap`, and the checked-in
firmware-size baselines.

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

ADK has no recorded official Library Manager inclusion, so CI and
`make arduino-lint-release` currently use `submit`. After inclusion is
independently confirmed, change the reviewed release mode to `update`; do not
run both contextual modes for every release. The explicit
`arduino-lint-submit` and `arduino-lint-update` targets remain available for
policy review. Local release evidence and CI use Arduino Lint 1.3.0.

Also test the installed archive, not only `--library .`:

1. Create a clean release archive from the intended tag.
2. Install it into a temporary Arduino CLI user directory.
3. Compile every installed example for `arduino:avr:mega`.
4. Confirm no build depends on ignored or untracked repository files.

The reproducible local and CI gate is:

```sh
make package-smoke
make package-smoke PACKAGE_REF=0.2.0
```

It exports `PACKAGE_REF` with Git, applies the release `export-ignore` rules,
installs the result into an isolated library directory, rejects symlinks and
executable files, and compiles every packaged example without `--library .`.

Record flash and static RAM sizes. Fail CI when an established example exceeds
its explicit budget without an approved update.

## Native C++ source archive

The separate native export is intentionally narrower than Arduino packaging:

```sh
make native-package
make native-package-smoke
```

`native-package` writes `build/package/adk-native.tar.gz` with this layout:

```text
adk-native/
├── include/adk/          public declaration headers
├── src/                  Arduino-header-free C++ sources
├── manifest/
│   ├── README.md         exact scope and limitations
│   └── sources.txt       authoritative compiled-source inventory
├── LICENSE
└── README.md
```

The archive contains source, not a prebuilt library. The smoke target extracts
it into a clean temporary directory, compiles every manifest entry as C++17,
creates a temporary `libadk.a`, then builds and runs an
`InertChannelAssessor` consumer with no repository-relative inputs.

All public declaration headers are exported so peer includes resolve, but the
manifest intentionally omits Arduino-bound endpoint implementations. Header
presence is not a claim that a hardware-facing type links or operates on a
native host. The demonstrated support claim is limited to the portable source
set and value-only consumer exercised by the smoke test. There is no system
installer, CMake package, pkg-config metadata, package-manager integration, or
cross-toolchain ABI promise.

This export does not change `library.properties`, the Arduino archive layout,
or `package-smoke`; both packaging gates remain independently required.

## Release gate

Before tagging, run the complete non-publishing local gate from a clean,
committed tree:

```sh
make release-check PACKAGE_REF=HEAD ARDUINO_LINT_RELEASE_MODE=submit
```

It confirms ref, metadata, changelog, and export policy before running the
software, firmware, documentation, site, archive-consumer, and selected lint
checks. It performs no tag, push, deployment, or Library Manager submission.
Then review:

1. Run host tests, formatting, documentation, site, Arduino compile, and all
   Arduino Lint gates.
2. Confirm `library.properties` has the release version and stable library name.
3. Confirm the archive contains `library.properties`, `src/`, `examples/`,
   `LICENSE`, and `README.md`.
4. Confirm it contains no build output, `.development`, symlinks, submodules, or
   executable binaries.
5. Compile every example from a clean archive installation.
6. Build the native source archive and run its clean C++17 consumer smoke test.
7. Confirm every native manifest path resolves inside the export.
8. Commit the version change.
9. Create and push a tag exactly matching the metadata version.

The current version is `0.3.0`; its matching tag is `0.3.0`, consistent with
the existing tag history. Never move or replace a published tag. Increment the
version and publish a new tag instead.

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
