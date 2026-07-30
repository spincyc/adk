#!/usr/bin/env python3

import argparse
import contextlib
import hashlib
import importlib.util
import io
import json
import pathlib
import re
import sys


ROOT = pathlib.Path(__file__).resolve().parent.parent
BASE_PATH = ROOT / "scripts/check_escape_console_resource_probe.py"
SPEC = importlib.util.spec_from_file_location("adk_exact_resource_probe", BASE_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("could not load the shared exact AVR probe implementation")
probe = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(probe)

BOARD_SRAM = 8192
ISR_RESERVE = 128
DESCRIPTOR_TARGET = 96
DESCRIPTOR_HARD = 128
RESIDUAL_SRAM_TARGET = 4096
RESIDUAL_SRAM_HARD = 3072
BOUNDARY_PATH = ROOT / "probes/component_qualification_boundary_079.json"
BOUNDARY = json.loads(BOUNDARY_PATH.read_text(encoding="utf-8"))
FINGERPRINT_CONTRACT = BOUNDARY.pop("fingerprint_contract")
RESOURCE_LAYOUT = {}
REVIEWS = {}


def residual_sram_gate(residual):
    if residual >= RESIDUAL_SRAM_TARGET:
        return "pass"
    if residual >= RESIDUAL_SRAM_HARD:
        return "target-miss"
    return "hard-fail"


def fingerprint_source_paths():
    return (
        "scripts/check_escape_console_resource_probe.py",
        "scripts/check_component_qualification_resource_probe.py",
        "probes/component_qualification_boundary_079.json",
        "probes/component_qualification_object_sizes.cpp",
        "src/bounded_low_side_driver_policy.h",
        "src/bounded_low_side_driver_policy.cpp",
        "examples/Lesson079BoundedLowSideDriver/"
        "Lesson079BoundedLowSideDriver.ino",
    )


def fingerprint_source_hashes():
    return {
        path: probe.sha256(ROOT / path)
        for path in fingerprint_source_paths()
    }


def read_size_symbols(nm, object_path):
    symbols = {}
    for line in probe.output(
        (str(nm), "--print-size", "--size-sort", str(object_path))
    ).splitlines():
        match = re.match(
            r"^[0-9a-fA-F]+\s+([0-9a-fA-F]+)\s+\w\s+(\w+Bytes)$",
            line,
        )
        if match:
            symbols[match.group(2)] = int(match.group(1), 16)
    return symbols


def object_sizes(compiler, nm, root, temporary, unused):
    object_path = temporary / "component_qualification_object_sizes.o"
    command = [
        str(compiler),
        "-c",
        "-mmcu=atmega2560",
        "-std=gnu++11",
        "-Os",
        "-fno-lto",
        "-fno-exceptions",
        "-fno-rtti",
        "-Isrc",
        str(root / "probes/component_qualification_object_sizes.cpp"),
        "-o",
        str(object_path),
    ]
    probe.run(command, cwd=root)
    RESOURCE_LAYOUT.update(read_size_symbols(nm, object_path))
    return RESOURCE_LAYOUT, command


def authority_section(root, authority):
    document, fragment = authority.split("#", 1)
    text = (root / document).read_text(encoding="utf-8")
    headings = list(
        re.finditer(r"^(#{1,6})[ \t]+(.+?)[ \t]*#*[ \t]*$", text, re.MULTILINE)
    )
    for index, heading in enumerate(headings):
        slug = re.sub(r"[^a-z0-9 -]", "", heading.group(2).lower())
        slug = re.sub(r"-+", "-", slug.replace(" ", "-")).strip("-")
        if slug != fragment:
            continue
        end = len(text)
        level = len(heading.group(1))
        for following in headings[index + 1:]:
            if len(following.group(1)) <= level:
                end = following.start()
                break
        return text[heading.start():end].replace(",", "")
    raise probe.ProbeError(f"target-miss review authority is missing: {authority}")


def load_reviews(root, review_path):
    global REVIEWS
    REVIEWS = {}
    path = root / review_path
    document = json.loads(path.read_text(encoding="utf-8"))
    if document.get("schema") != 1 or not isinstance(
        document.get("reviews"), list
    ):
        raise probe.ProbeError(f"invalid target-miss review document: {path}")
    required = {
        "lesson",
        "metric",
        "observed_bytes",
        "target_bytes",
        "hard_bytes",
        "authority",
        "disposition",
        "rationale",
        "fingerprint_sha256",
    }
    authority = (
        "docs/design/LESSON_079_BOUNDED_LOW_SIDE_DRIVER_STRESS_PASS.md"
        "#terminal-gate-result"
    )
    for review in document["reviews"]:
        if set(review) != required:
            raise probe.ProbeError(f"invalid target-miss review fields: {review}")
        if (
            review["lesson"] != "079"
            or review["metric"]
            not in ("flash", "static_sram", "synchronous_stack", "object")
            or review["authority"] != authority
            or review["disposition"] != "accepted-target-miss"
            or not isinstance(review["rationale"], str)
            or not review["rationale"].strip()
        ):
            raise probe.ProbeError(f"unsupported target-miss review: {review}")
        section = authority_section(root, authority)
        required_values = (
            review["observed_bytes"],
            review["target_bytes"],
            review["hard_bytes"],
            review["fingerprint_sha256"],
        )
        if any(str(value) not in section for value in required_values):
            raise probe.ProbeError(
                f"target-miss tuple is absent from {authority}: {review}"
            )
        key = review["metric"]
        if key in REVIEWS:
            raise probe.ProbeError(f"duplicate target-miss review: {key}")
        REVIEWS[key] = review
    return {}


def enrich_evidence(evidence_path):
    if not evidence_path.is_file():
        return 1
    report = json.loads(evidence_path.read_text(encoding="utf-8"))
    report["constants"].update(
        {
            "component_qualification_isr_reserve_bytes": ISR_RESERVE,
            "low_side_driver_descriptor_target_bytes": DESCRIPTOR_TARGET,
            "low_side_driver_descriptor_hard_bytes": DESCRIPTOR_HARD,
        }
    )
    state = report["boundaries"][0]
    if "measurements" not in state:
        return 1
    descriptor = RESOURCE_LAYOUT["lowSideDriverDescriptorBytes"]
    measurements = state["measurements"]
    residual = (
        BOARD_SRAM
        - measurements["static_sram_bytes"]
        - measurements["synchronous_stack_bytes"]
        - ISR_RESERVE
    )
    measurements.update(
        {
            "descriptor_bytes": descriptor,
            "isr_reserve_bytes": ISR_RESERVE,
            "residual_sram_bytes": residual,
        }
    )
    state["gates"]["descriptor"] = probe.gate(
        descriptor, DESCRIPTOR_TARGET, DESCRIPTOR_HARD
    )
    state["gates"]["residual_sram"] = residual_sram_gate(residual)
    payload = {
        "boundary": BOUNDARY,
        "contract": FINGERPRINT_CONTRACT,
        "measurements": measurements,
        "source_hashes": fingerprint_source_hashes(),
        "tools": {
            key: value
            for key, value in report.get("tools", {}).items()
            if key != "arduino_cli"
        },
    }
    state["fingerprint_contract"] = FINGERPRINT_CONTRACT
    state["fingerprint_source_hashes"] = payload["source_hashes"]
    state["fingerprint_sha256"] = hashlib.sha256(
        json.dumps(payload, sort_keys=True, separators=(",", ":")).encode(
            "utf-8"
        )
    ).hexdigest()
    measurement_keys = {
        "flash": "flash_bytes",
        "static_sram": "static_sram_bytes",
        "synchronous_stack": "synchronous_stack_bytes",
        "object": "object_bytes",
    }
    limit_keys = {
        "flash": ("flash_target", "flash_hard"),
        "static_sram": ("sram_target", "sram_hard"),
        "synchronous_stack": ("stack_target", "stack_hard"),
        "object": ("object_target", "object_hard"),
    }
    accepted = []
    for metric, review in REVIEWS.items():
        target_key, hard_key = limit_keys[metric]
        current = (
            measurements[measurement_keys[metric]],
            BOUNDARY[target_key],
            BOUNDARY[hard_key],
            state["fingerprint_sha256"],
        )
        reviewed = (
            review["observed_bytes"],
            review["target_bytes"],
            review["hard_bytes"],
            review["fingerprint_sha256"],
        )
        if state["gates"].get(metric) != "target-miss" or reviewed != current:
            raise probe.ProbeError(
                f"stale Lesson 079 {metric} target-miss review"
            )
        state["gates"][metric] = "reviewed-target-miss"
        accepted.append(review)
    state["accepted_reviews"] = accepted
    state["status"] = (
        "hard-fail"
        if "hard-fail" in state["gates"].values()
        else (
            "review-required"
            if "target-miss" in state["gates"].values()
            else (
                "reviewed-target-miss"
                if "reviewed-target-miss" in state["gates"].values()
                else "pass"
            )
        )
    )
    report["status"] = state["status"]
    evidence_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return 1 if state["status"] in ("hard-fail", "review-required") else 0


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--arduino-cli", default="arduino-cli")
    parser.add_argument("--fqbn", default="arduino:avr:mega")
    parser.add_argument(
        "--evidence-json",
        default="build/evidence/component-qualification-resource-probe.json",
    )
    parser.add_argument(
        "--review-file",
        default="probes/component_qualification_resource_reviews.json",
    )
    arguments = parser.parse_args()
    probe.BOUNDARIES = (BOUNDARY,)
    probe.object_sizes = object_sizes
    probe.load_reviews = load_reviews
    sys.argv = [
        sys.argv[0],
        "--arduino-cli",
        arguments.arduino_cli,
        "--fqbn",
        arguments.fqbn,
        "--require-complete",
        "--evidence-json",
        arguments.evidence_json,
        "--review-file",
        arguments.review_file,
    ]
    output = io.StringIO()
    with contextlib.redirect_stdout(output):
        probe_exit = probe.main()
    evidence_path = ROOT / arguments.evidence_json
    if probe_exit != 0 and not evidence_path.is_file():
        print(output.getvalue(), end="")
        return probe_exit
    exit_code = enrich_evidence(evidence_path)
    report = json.loads(evidence_path.read_text(encoding="utf-8"))
    state = report["boundaries"][0]
    measurements = state.get("measurements", {})
    if not measurements:
        print(output.getvalue(), end="")
        return exit_code
    print(
        f"Lesson {state['lesson']}: flash {measurements['flash_bytes']} B; "
        f"static SRAM {measurements['static_sram_bytes']} B; synchronous "
        f"stack {measurements['synchronous_stack_bytes']} B; object "
        f"{measurements['object_bytes']} B; residual SRAM "
        f"{measurements['residual_sram_bytes']} B ({state['status']})"
    )
    print(f"Evidence: {evidence_path.relative_to(ROOT)}")
    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
