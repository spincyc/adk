#!/usr/bin/env python3

import argparse
import importlib.util
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


ALL_BOUNDARIES = (
    {
        "lesson": "058",
        "sketch": "examples/Lesson058MultiplexedDigits",
        "object_symbol": "multiplexedDigitPolicyObjectBytes",
        "flash_target": 12 * 1024,
        "flash_hard": 16 * 1024,
        "sram_target": 768,
        "sram_hard": 1024,
        "stack_target": 320,
        "stack_hard": 448,
        "object_target": 192,
        "object_hard": 256,
    },
    {
        "lesson": "059",
        "sketch": "examples/Lesson059Max7219Presentation",
        "object_symbol": "max7219PresentationPolicyObjectBytes",
        "flash_target": 16 * 1024,
        "flash_hard": 20 * 1024,
        "sram_target": 1024,
        "sram_hard": 1536,
        "stack_target": 384,
        "stack_hard": 512,
        "object_target": 192,
        "object_hard": 256,
    },
    {
        "lesson": "060",
        "sketch": "examples/Lesson060DualDisplayTimingDesk",
        "object_symbol": "dualDisplayTimingDeskObjectBytes",
        "flash_target": 24 * 1024,
        "flash_hard": 32 * 1024,
        "sram_target": 2048,
        "sram_hard": 3072,
        "stack_target": 640,
        "stack_hard": 896,
        "object_target": 640,
        "object_hard": 896,
    },
)


def object_sizes(compiler, nm, root, temporary, unused):
    object_path = temporary / "display_timing_object_sizes.o"
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
    ]
    for lesson, header in (
        ("059", root / "src/max7219_presentation_policy.h"),
        ("060", root / "src/dual_display_timing_desk.h"),
    ):
        if header.is_file():
            command.append(f"-DADK_HAS_LESSON_{lesson}=1")
    command.extend(
        (
            str(root / "probes/display_timing_object_sizes.cpp"),
            "-o",
            str(object_path),
        )
    )
    probe.run(command, cwd=root)
    sizes = {}
    for line in probe.output(
        (str(nm), "--print-size", "--size-sort", str(object_path))
    ).splitlines():
        match = re.match(
            r"^[0-9a-fA-F]+\s+([0-9a-fA-F]+)\s+\w\s+(\w+Bytes)$",
            line,
        )
        if match:
            sizes[match.group(2)] = int(match.group(1), 16)
    return sizes, command


def load_reviews(root, review_path):
    path = root / review_path
    if not path.is_file():
        return {}
    document = json.loads(path.read_text(encoding="utf-8"))
    if document.get("schema") != 1 or not isinstance(document.get("reviews"), list):
        raise probe.ProbeError(f"invalid target-miss review document: {path}")
    reviews = {}
    known_lessons = {boundary["lesson"] for boundary in probe.BOUNDARIES}
    authorities = {
        "docs/design/LESSONS_058_060_DISPLAY_TIMING_DESK_PLAN.md"
        "#deterministic-proof",
        "docs/design/LESSON_058_MULTIPLEXED_DIGITS_STRESS_PASS.md"
        "#architecture-stress-table",
    }
    required = {
        "lesson",
        "metric",
        "observed_bytes",
        "target_bytes",
        "hard_bytes",
        "authority",
        "disposition",
        "rationale",
    }
    for review in document["reviews"]:
        if set(review) != required:
            raise probe.ProbeError(f"invalid target-miss review fields: {review}")
        if review["lesson"] not in known_lessons:
            raise probe.ProbeError(f"target-miss review names unknown lesson: {review}")
        if review["disposition"] != "accepted-target-miss":
            raise probe.ProbeError(f"invalid target-miss disposition: {review}")
        if review["authority"] not in authorities:
            raise probe.ProbeError(
                f"target-miss review lacks controlling authority: {review}"
            )
        authority_path = root / review["authority"].split("#", 1)[0]
        marker = (
            f"Resource-review: lesson={review['lesson']} "
            f"metric={review['metric']} observed={review['observed_bytes']} "
            f"target={review['target_bytes']} hard={review['hard_bytes']} "
            f"disposition={review['disposition']}"
        )
        if marker not in authority_path.read_text(encoding="utf-8"):
            raise probe.ProbeError(
                f"target-miss review marker is absent from {review['authority']}: "
                f"{marker}"
            )
        key = (review["lesson"], review["metric"])
        if key in reviews:
            raise probe.ProbeError(f"duplicate target-miss review: {key}")
        reviews[key] = review
    return reviews


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--arduino-cli", default="arduino-cli")
    parser.add_argument("--fqbn", default="arduino:avr:mega")
    parser.add_argument(
        "--require-through",
        choices=("058", "059", "060"),
        default="058",
    )
    parser.add_argument(
        "--evidence-json",
        default="build/evidence/display-timing-resource-probe.json",
    )
    parser.add_argument(
        "--review-file",
        default="probes/display_timing_resource_reviews.json",
    )
    arguments = parser.parse_args()
    probe.BOUNDARIES = tuple(
        boundary
        for boundary in ALL_BOUNDARIES
        if boundary["lesson"] <= arguments.require_through
    )
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
    return probe.main()


if __name__ == "__main__":
    raise SystemExit(main())
