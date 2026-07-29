#!/usr/bin/env python3

import argparse
import importlib.util
import json
import pathlib
import re
import shutil
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
RESIDUAL_SRAM_TARGET = 3072
RESIDUAL_SRAM_HARD = 2048
FIXED_BUFFER_TARGET = 256
FIXED_BUFFER_HARD = 512

LESSON_059_PUBLIC_VALUES = (
    "Max7219PresentationConfig",
    "Max7219Frame",
    "Max7219Command",
    "Max7219Receipt",
    "Max7219Failure",
    "Max7219PresentationSnapshot",
    "Max7219PresentationPreview",
)

LESSON_060_PUBLIC_VALUES = (
    "TimingDeskStopwatchState",
    "TimingDeskQualification",
    "TimingDeskPresentationDisposition",
    "TimingDeskFaultOwner",
    "TimingDeskControlIdentity",
    "TimingDeskControlEvidence",
    "DigitFrameReceipt",
    "MatrixFrameReceipt",
    "DualDisplayTimingDeskConfig",
    "DualDisplayEnvelope",
    "DualDisplayTimingDeskResult",
    "DualDisplayTimingDeskSnapshot",
)

RESOURCE_LAYOUTS = {}


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
    if (root / "src/max7219_presentation_policy.h").is_file():
        layout_path = temporary / "display_timing_public_layouts.cpp"
        layout_source = """
#include <max7219_presentation_policy.h>
#if defined (ADK_HAS_LESSON_060)
#include <dual_display_timing_desk.h>
#endif
#define ADK_LAYOUT(type) \\
    unsigned char type##Bytes[sizeof (adk::type)]; \\
    unsigned char type##Alignment[alignof (adk::type)]; \\
    static_assert (__is_standard_layout (adk::type), \\
                   #type " must remain standard-layout"); \\
    static_assert (__is_trivially_copyable (adk::type), \\
                   #type " must remain trivially copyable")

ADK_LAYOUT (Max7219PresentationConfig);
ADK_LAYOUT (Max7219Frame);
ADK_LAYOUT (Max7219Command);
ADK_LAYOUT (Max7219Receipt);
ADK_LAYOUT (Max7219Failure);
ADK_LAYOUT (Max7219PresentationSnapshot);
ADK_LAYOUT (Max7219PresentationPreview);

unsigned char max7219LogicalRowsFixedBufferBytes[8];
"""
        if (root / "src/dual_display_timing_desk.h").is_file():
            layout_source += """
ADK_LAYOUT (TimingDeskControlIdentity);
ADK_LAYOUT (TimingDeskStopwatchState);
ADK_LAYOUT (TimingDeskQualification);
ADK_LAYOUT (TimingDeskPresentationDisposition);
ADK_LAYOUT (TimingDeskFaultOwner);
ADK_LAYOUT (TimingDeskControlEvidence);
ADK_LAYOUT (DigitFrameReceipt);
ADK_LAYOUT (MatrixFrameReceipt);
ADK_LAYOUT (DualDisplayTimingDeskConfig);
ADK_LAYOUT (DualDisplayEnvelope);
ADK_LAYOUT (DualDisplayTimingDeskResult);
ADK_LAYOUT (DualDisplayTimingDeskSnapshot);

unsigned char timingDeskControlEvidenceCallerBufferBytes
    [3 * sizeof (adk::TimingDeskControlEvidence)];
unsigned char timingDeskReceiptPointerCallerBufferBytes
    [sizeof (adk::DigitFrameReceipt*)
     + sizeof (adk::MatrixFrameReceipt*)
     + sizeof (adk::Max7219Receipt*)];
"""
        layout_path.write_text(
            layout_source.lstrip(),
            encoding="utf-8",
        )
        layout_object = temporary / "display_timing_public_layouts.o"
        layout_command = [
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
        if (root / "src/dual_display_timing_desk.h").is_file():
            layout_command.append("-DADK_HAS_LESSON_060=1")
        layout_command.extend(
            [
            str(layout_path),
            "-o",
            str(layout_object),
            ]
        )
        probe.run(layout_command, cwd=root)
        host_compiler = shutil.which("c++")
        if host_compiler is None:
            raise probe.ProbeError("host C++ compiler is unavailable for trait checks")
        trait_path = temporary / "display_timing_public_traits.cpp"
        trait_path.write_text(
            """
#include <max7219_presentation_policy.h>
#if defined (ADK_HAS_LESSON_060)
#include <dual_display_timing_desk.h>
#endif
#include <type_traits>

static_assert (
    !std::is_copy_constructible<adk::Max7219PresentationPolicy>::value,
    "Max7219PresentationPolicy must remain non-copyable");
static_assert (
    !std::is_move_constructible<adk::Max7219PresentationPolicy>::value,
    "Max7219PresentationPolicy must remain non-movable");
#if defined (ADK_HAS_LESSON_060)
static_assert (
    !std::is_copy_constructible<adk::DualDisplayTimingDesk>::value,
    "DualDisplayTimingDesk must remain non-copyable");
static_assert (
    !std::is_move_constructible<adk::DualDisplayTimingDesk>::value,
    "DualDisplayTimingDesk must remain non-movable");
#endif
""".lstrip(),
            encoding="utf-8",
        )
        trait_command = [
            host_compiler,
            "-fsyntax-only",
            "-std=c++11",
            "-Isrc",
        ]
        if (root / "src/dual_display_timing_desk.h").is_file():
            trait_command.append("-DADK_HAS_LESSON_060=1")
        trait_command.append(str(trait_path))
        probe.run(trait_command, cwd=root)
        layouts = {}
        for line in probe.output(
            (str(nm), "--print-size", "--size-sort", str(layout_object))
        ).splitlines():
            match = re.match(
                r"^[0-9a-fA-F]+\s+([0-9a-fA-F]+)\s+\w\s+"
                r"(\w+(?:Bytes|Alignment))$",
                line,
            )
            if match:
                layouts[match.group(2)] = int(match.group(1), 16)
        RESOURCE_LAYOUTS["059"] = {
            "commands": (layout_command, trait_command),
            "symbols": layouts,
        }
        if (root / "src/dual_display_timing_desk.h").is_file():
            RESOURCE_LAYOUTS["060"] = {
                "commands": (),
                "symbols": {**layouts, **sizes},
            }
    return sizes, command


def enrich_evidence(evidence_path):
    if "059" not in RESOURCE_LAYOUTS or not evidence_path.is_file():
        return 0
    report = json.loads(evidence_path.read_text(encoding="utf-8"))
    report["commands"].extend(RESOURCE_LAYOUTS["059"]["commands"])
    report["constants"].update(
        {
            "display_timing_isr_reserve_bytes": ISR_RESERVE,
            "display_timing_residual_sram_target_bytes": RESIDUAL_SRAM_TARGET,
            "display_timing_residual_sram_hard_bytes": RESIDUAL_SRAM_HARD,
            "display_timing_residual_target_miss_reviewable": False,
            "fixed_buffer_target_bytes": FIXED_BUFFER_TARGET,
            "fixed_buffer_hard_bytes": FIXED_BUFFER_HARD,
        }
    )
    for state in report["boundaries"]:
        lesson = state["lesson"]
        if "measurements" not in state:
            continue
        residual = (
            BOARD_SRAM
            - state["measurements"]["static_sram_bytes"]
            - state["measurements"]["synchronous_stack_bytes"]
            - ISR_RESERVE
        )
        state["measurements"]["isr_reserve_bytes"] = ISR_RESERVE
        state["measurements"]["residual_sram_bytes"] = residual
        state["gates"]["residual_sram"] = (
            "pass" if residual >= RESIDUAL_SRAM_TARGET else "hard-fail"
        )
        state["gates"]["residual_sram_hard_floor"] = (
            "pass" if residual >= RESIDUAL_SRAM_HARD else "hard-fail"
        )
        if lesson not in RESOURCE_LAYOUTS:
            if state["gates"]["residual_sram"] == "hard-fail":
                state["status"] = "hard-fail"
                report["status"] = probe.merge_status(
                    report["status"], state["status"]
                )
            continue
        symbols = RESOURCE_LAYOUTS[lesson]["symbols"]
        names = (
            LESSON_059_PUBLIC_VALUES
            if lesson == "059"
            else LESSON_060_PUBLIC_VALUES
        )
        public_values = {}
        for name in names:
            size_symbol = f"{name}Bytes"
            alignment_symbol = f"{name}Alignment"
            if size_symbol not in symbols or alignment_symbol not in symbols:
                raise probe.ProbeError(
                    f"Lesson {lesson} public layout symbol is missing: {name}"
                )
            public_values[name] = {
                "size_bytes": symbols[size_symbol],
                "alignment_bytes": symbols[alignment_symbol],
                "standard_layout": True,
                "trivially_copyable": True,
            }
        state["measurements"]["public_values"] = public_values
        state["measurements"]["policy_traits"] = {
            "copy_constructible": False,
            "move_constructible": False,
        }
        if lesson == "059":
            fixed_buffer = symbols.get("max7219LogicalRowsFixedBufferBytes")
            if fixed_buffer is None:
                raise probe.ProbeError("Lesson 059 fixed-buffer symbol is missing")
            state["measurements"]["fixed_buffers"] = {
                "logical_rows_bytes": fixed_buffer,
            }
            state["gates"]["fixed_buffers"] = probe.gate(
                fixed_buffer,
                FIXED_BUFFER_TARGET,
                FIXED_BUFFER_HARD,
            )
            if state["gates"]["fixed_buffers"] == "target-miss":
                state["gates"]["fixed_buffers"] = "review-required"
        else:
            digit_policy = symbols.get("multiplexedDigitPolicyObjectBytes")
            matrix_policy = symbols.get("max7219PresentationPolicyObjectBytes")
            control_buffer = symbols.get(
                "timingDeskControlEvidenceCallerBufferBytes"
            )
            receipt_buffer = symbols.get(
                "timingDeskReceiptPointerCallerBufferBytes"
            )
            if None in (
                digit_policy,
                matrix_policy,
                control_buffer,
                receipt_buffer,
            ):
                raise probe.ProbeError(
                    "Lesson 060 child-object or caller-buffer symbol is missing"
                )
            child_lower_bound = digit_policy + matrix_policy
            object_bytes = state["measurements"]["object_bytes"]
            if object_bytes < child_lower_bound:
                raise probe.ProbeError(
                    "Lesson 060 object is smaller than its value-owned children"
                )
            state["measurements"]["owned_child_objects"] = {
                "multiplexed_digit_policy_bytes": digit_policy,
                "max7219_presentation_policy_bytes": matrix_policy,
                "mechanical_lower_bound_bytes": child_lower_bound,
                "parent_state_overhead_bytes": object_bytes - child_lower_bound,
            }
            state["measurements"]["caller_buffers"] = {
                "control_evidence_triplet_bytes": control_buffer,
                "receipt_pointer_triplet_bytes": receipt_buffer,
                "aggregate_bytes": control_buffer + receipt_buffer,
            }
            for name, value in (
                ("control_evidence_caller_buffer", control_buffer),
                ("receipt_pointer_caller_buffer", receipt_buffer),
                ("aggregate_caller_buffers", control_buffer + receipt_buffer),
            ):
                state["gates"][name] = probe.gate(
                    value,
                    FIXED_BUFFER_TARGET,
                    FIXED_BUFFER_HARD,
                )
                if state["gates"][name] == "target-miss":
                    state["gates"][name] = "review-required"
        state["status"] = (
            "hard-fail"
            if "hard-fail" in state["gates"].values()
            else (
                "review-required"
                if "review-required" in state["gates"].values()
                else state["status"]
            )
        )
        report["status"] = probe.merge_status(report["status"], state["status"])
    evidence_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return 1 if report["status"] in ("hard-fail", "review-required", "error") else 0


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
    result = probe.main()
    evidence_result = enrich_evidence(ROOT / arguments.evidence_json)
    return result or evidence_result


if __name__ == "__main__":
    raise SystemExit(main())
