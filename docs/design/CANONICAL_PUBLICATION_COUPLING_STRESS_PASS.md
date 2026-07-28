# Canonical publication coupling stress pass

This is the first application of the
[component design stress pass](../templates/component-design-stress-pass.md).
It assesses the repository mechanism exposed when a newly promoted lesson
becomes the live publication verifier's target. It does not change lesson or
site publication files.

## Boundary and evidence

Reviewer/date: repository guidance review with independent read-only review,
2026-07-28.

The publication workflow correctly verifies the newest promoted lesson's HTML,
PDF signature, and byte-identical canonical sketch. The current mechanism
encodes that boundary independently in several places:

- `scripts/check_site.py` already derives the newest configured lesson and its
  example from `mk/config.mk`, then validates landing/index links, HTML
  structural markers, the PDF link, and the example download in the generated
  site;
- `scripts/check_deployed_site.py` hard-codes the lesson number, title marker,
  download paths, example directory, and example-specific source marker;
- `tests/test_deployed_site.py` proves those constants follow the numerically
  latest lesson page, but builds fixtures from the same constants;
- lesson promotion separately updates build inventories, PDFs, site pages,
  navigation, status tables, and the landing-page published range.

The offline resolver is therefore already the closest thing to an authority,
and the deployed verifier is correct for Lesson 036. However, each promotion
still requires coordinated edits to a code-shaped copy of metadata. The
regression test detects a stale lesson number only after the new lesson page is
visible; it does not make the deployment verifier consume the offline
resolver, and its lesson-specific title and source markers must still be
invented for every boundary.

## Fit review

| Pressure | Finding |
|---|---|
| API and layering | `check_deployment()` is cleanly separated from fetching, but its generic verifier is coupled to one lesson's identity and component symbol. |
| Ownership and lifecycle | No runtime ownership issue. Metadata ownership is split across build, site, and deployment-check inputs. |
| Time and errors | Retries, timeout, response bounds, redirect checks, and deterministic failures are well bounded. Preserve them. |
| Resource budget | Negligible firmware impact; maintenance cost grows with every publication boundary. |
| Deterministic proof | Unit tests exercise fetch and validation behavior. Their “latest lesson” assertion guards one path but does not derive the complete artifact set from a canonical promotion record. |
| Packaging and documentation | Byte comparison to the audited canonical sketch is strong. The path selecting that sketch duplicates the resolver already present in the offline site checker. |
| Downstream effects | Every later lesson/project promotion touches deployment verification. A missed update can fail Pages after otherwise valid publication work. |

## Prior-decision impact

- One canonical sketch feeding firmware, HTML, PDF, download, and deployment
  comparison is **preserved**.
- Bounded post-deploy retries and origin/path checks are **preserved**.
- The requirement to verify the newest published lesson is **preserved**.
- The deployment verifier's independent encoding of newest-publication
  identity in Python constants is **challenged**: it repeats the build
  inventory/offline resolver instead of consuming that decision.

Disposition: **architectural remediation required**, bounded to publication
metadata and tooling. It is not a reason to redesign component APIs or change
existing lesson publications.

## Alternatives and bounded remediation

1. **Extract and share the existing promotion resolver (recommended).** Move
   the pure `mk/config.mk` parsing and derived artifact-path logic from
   `scripts/check_site.py` into a small importable module. Both offline and
   deployed checkers consume it. Keep lesson-independent structural markers
   (`Lesson NNN`, status, safety boundary, PDF signature, Arduino text and byte
   identity); do not require a duplicated lesson title or component-symbol
   marker.
2. **Make an explicit publication manifest authoritative.** Move promotion
   identity into one reviewed data file, make `mk/config.mk`, site staging, and
   both checkers consume or validate against it, and reject disagreement. This
   is more explicit but adds a format, parser, and migration surface.
3. **Retain synchronized constants.** Keep the present arrangement and update
   constants/tests per promotion. This has the lowest immediate edit cost but
   preserves the post-deploy failure mode and is not recommended.

Alternative 1 has a bounded compatibility cost: internal Python imports and
fixtures change, while generated URLs, Arduino packages, public component
APIs, and existing publications do not. Alternative 2 additionally migrates
Make/site ownership and requires schema validation. Neither changes firmware,
pin/timer/flash/SRAM budgets, electrical behavior, safety claims, or physical
acceptance. The main risk is incorrectly parsing Make syntax; fixtures must
cover malformed/empty/duplicate lesson inventories, a missing newest example,
and promotion from one boundary to the next.

The recommended next experiment is a pure resolver returning lesson number,
canonical example source, and derived deployed paths from representative
configuration fixtures. Prove both checkers select identical artifacts before
changing workflow wiring. Preserve all current network, retry, timeout,
response-size, redirect, PDF-signature, UTF-8, and byte-identity checks.

This authority choice affects Make, site staging, Pages verification, tests,
and future release tooling. Discuss the alternatives and record a durable
decision before implementation; do not fold it opportunistically into the
active Lessons 037--039 publication boundary.

## Gate result

- Disposition: architectural remediation required in publication tooling.
- Open risk: no single consumed resolver currently binds offline and deployed
  selection of the newest promoted artifacts.
- Required discussion or decision IDs: pending a user choice between shared
  build-inventory derivation and an explicit authoritative manifest.
- Remediation owner and next action: a separately leased publication-tooling
  task should prototype the pure shared resolver and return measured fixture
  evidence before workflow integration.
- Verification performed: source inspection of `scripts/check_site.py`,
  `scripts/check_deployed_site.py`, `tests/test_deployed_site.py`, and
  `mk/config.mk`; documentation link and repository gates are recorded with
  the enclosing guidance task.
- Promotion permitted: yes for this guidance and the already verified Lesson
  036 publication; no for selecting or implementing the wide remediation
  without the required discussion and durable decision.
