# Repository tool registry

ADK indexes repo-local tools with [tmt](https://github.com/spincyc/tmt) in the
committed `tmt.json` registry. Tools live in `tools/` and run as `tools/<id>`
whether or not tmt itself is installed.

## Before writing a script

Read `tmt.json` and run `tools/<id> --help` first. Prefer editing the nearest
existing tool over adding a near-duplicate. After deriving anything repeatable,
record it with `tmt note <slug>`; at the reported threshold scaffold it with
`tmt new <slug>` and paste in the derived logic. Recording candidates is cheap
and must not be pre-filtered by guessed value.

`aiq` remains the authority for work state. Tool-candidate events are recorded
only through `tmt note` and read with `tmt candidates`.

## What a tool must satisfy

| Requirement | Rule |
|---|---|
| Scope | One tool answers one question |
| Help | `tools/<id> --help` exits 0 |
| Machine output | `--json` prints exactly one compact key-sorted object including `"v":1` |
| Errors | one compact JSON object on stderr, nothing on stdout, exit 70 |
| Findings | check-style tools exit 0 when clean and 1 when they have findings |
| Determinism | byte-stable output for unchanged inputs: sorted iteration, no timestamps, no locale dependence |
| Honesty | `mutates` and `idempotent` in the registry describe real behavior |

`tools/<id>.test` is the tool's gate and must contain real assertions; the
stable stage refuses to promote an unmodified scaffold. `tmt stage <id> stable`
is the only way to promote — never hand-edit the `stage` field. Every other
registry field is maintained by hand, and each edit must leave `tmt check`
green. `make tool-registry-check` runs that battery as part of
`make quality-lint`, and skips with a notice when tmt is absent.

A stable tool may not depend on a draft tool. Draft-on-draft is permitted.

## Why the probe scripts stay in `scripts/`

`scripts/` holds the build, publication, and resource-probe programs the
Makefile drives. The exact resource probes are deliberately **not** migrated to
`tools/`: each published lesson's fingerprint hashes its probe script *by
path*, so relocating one would invalidate the recorded, independently reviewed
resource dispositions for that lesson. Those scripts are also single-purpose
Make steps rather than reusable answers to a question.

Tools in `tools/` are the reusable derivations that would otherwise be
rewritten from scratch in each session.

## Current tools

| Tool | Stage | Purpose |
|---|---|---|
| `align-parens` | stable | Align function-call parentheses in C++ sources to the `make style-check` rule |

`align-parens` imports `scripts/check_style.py` so the definition of a function
parenthesis has exactly one authority; that coupling is why it is repo-local
rather than portable. Use `tools/align-parens src/new_file.cpp` when adding
C++ sources, and `--check` in a read-only context.
