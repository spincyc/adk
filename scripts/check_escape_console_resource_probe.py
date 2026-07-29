#!/usr/bin/env python3

import argparse
import hashlib
import json
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile


BOARD_SRAM = 8192
ISR_RESERVE = 128
MINIMUM_RESIDUAL_SRAM = 2048
CALL_RETURN_BYTES = 3
STATUS_SEVERITY = {
    "pass": 0,
    "reviewed-target-miss": 1,
    "pending": 2,
    "review-required": 3,
    "hard-fail": 4,
    "error": 5,
}

BOUNDARIES = (
    {
        "lesson": "055",
        "sketch": "examples/Lesson055ClueConstraintModel",
        "object_symbol": "clueConstraintModelObjectBytes",
        "flash_target": 16 * 1024,
        "flash_hard": 20 * 1024,
        "sram_target": 1536,
        "sram_hard": 2048,
        "stack_target": 384,
        "stack_hard": 512,
        "object_target": 512,
        "object_hard": 640,
    },
    {
        "lesson": "056",
        "sketch": "examples/Lesson056FaultAwareOperatorPanel",
        "object_symbol": "faultAwareOperatorPanelObjectBytes",
        "flash_target": 24 * 1024,
        "flash_hard": 28 * 1024,
        "sram_target": 2560,
        "sram_hard": 3072,
        "stack_target": 640,
        "stack_hard": 768,
        "object_target": 384,
        "object_hard": 512,
    },
    {
        "lesson": "057",
        "sketch": "extras/probes/EscapeConsoleMaximumComposition",
        "object_symbol": "inertEscapeConsoleObjectBytes",
        "flash_target": 32 * 1024,
        "flash_hard": 40 * 1024,
        "sram_target": 4096,
        "sram_hard": 4608,
        "stack_target": 1024,
        "stack_hard": 1280,
        "object_target": 1024,
        "object_hard": 1280,
    },
)


class ProbeError(RuntimeError):
    pass


def run(command, **kwargs):
    return subprocess.run(command, check=True, text=True, **kwargs)


def output(command, **kwargs):
    return subprocess.check_output(command, text=True, **kwargs)


def tool_beside(build_directory, name):
    commands = json.loads(
        (build_directory / "compile_commands.json").read_text(encoding="utf-8")
    )
    compiler = pathlib.Path(commands[0]["arguments"][0])
    tool = compiler.parent / name
    if not tool.is_file():
        raise ProbeError(f"{name} was not found beside {compiler}")
    return tool


def tool_version(tool):
    for arguments in ((str(tool), "--version"), (str(tool), "-v")):
        try:
            text = output(arguments, stderr=subprocess.STDOUT).splitlines()
            if text:
                return text[0].strip()
        except subprocess.CalledProcessError:
            pass
    raise ProbeError(f"could not read the version of {tool}")


def section_sizes(size_tool, elf_path):
    sections = {}
    for line in output((str(size_tool), "-A", str(elf_path))).splitlines():
        columns = line.split()
        if len(columns) >= 2 and columns[0].startswith("."):
            sections[columns[0]] = int(columns[1])
    data = sections.get(".data", 0)
    static_sram = data + sections.get(".bss", 0) + sections.get(".noinit", 0)
    return sections.get(".text", 0) + data, static_sram


def merge_status(current, candidate):
    return (
        candidate
        if STATUS_SEVERITY[candidate] > STATUS_SEVERITY[current]
        else current
    )


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def compile_sketch(arguments, root, temporary, boundary):
    build_directory = temporary / f"lesson-{boundary['lesson']}"
    map_path = build_directory / "firmware.map"
    sketch_path = root / boundary["sketch"]
    if boundary["lesson"] == "057":
        descriptor = (
            sketch_path / "EscapeConsoleMaximumComposition.ino"
        ).read_text(encoding="utf-8")
        match = re.fullmatch(
            r"// canonical-source: ([A-Za-z0-9_./-]+)\n", descriptor
        )
        if match is None:
            raise ProbeError(
                "Lesson 057 maximum-composition descriptor is not canonical"
            )
        canonical = root / match.group(1)
        try:
            canonical.relative_to(root / "examples")
        except ValueError as error:
            raise ProbeError(
                "Lesson 057 maximum-composition source leaves examples/"
            ) from error
        if not canonical.is_file():
            raise ProbeError(
                f"Lesson 057 canonical fixture does not exist: {canonical}"
            )
        sketch_path = temporary / "EscapeConsoleMaximumComposition"
        sketch_path.mkdir()
        shutil.copyfile(
            canonical, sketch_path / "EscapeConsoleMaximumComposition.ino"
        )
    command = [
        arguments.arduino_cli,
        "compile",
        "--fqbn",
        arguments.fqbn,
        "--library",
        str(root),
        "--build-path",
        str(build_directory),
        "--build-property",
        "compiler.c.extra_flags=-fno-lto -fno-jump-tables -fstack-usage "
        "-maccumulate-args",
        "--build-property",
        "compiler.cpp.extra_flags=-fno-lto -fno-jump-tables -fstack-usage "
        "-maccumulate-args",
        "--build-property",
        "compiler.S.extra_flags=-fno-lto",
        "--build-property",
        f"compiler.c.elf.extra_flags=-fno-lto -Wl,-Map,{map_path}",
    ]
    command.append(str(sketch_path))
    run(command, cwd=root)
    elf_paths = list(build_directory.glob("*.elf"))
    if len(elf_paths) != 1:
        raise ProbeError(
            f"Lesson {boundary['lesson']} produced {len(elf_paths)} ELF files"
        )
    if not map_path.is_file():
        raise ProbeError(f"Lesson {boundary['lesson']} did not produce a link map")
    return build_directory, elf_paths[0], command


def object_sizes(compiler, nm, root, temporary, include_lesson_057):
    object_path = temporary / "escape_console_object_sizes.o"
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
    if include_lesson_057:
        command.append("-DADK_HAS_INERT_ESCAPE_CONSOLE=1")
    command.extend(
        (
            str(root / "probes/escape_console_object_sizes.cpp"),
            "-o",
            str(object_path),
        )
    )
    run(command, cwd=root)
    sizes = {}
    for line in output(
        (str(nm), "--print-size", "--size-sort", str(object_path))
    ).splitlines():
        match = re.match(
            r"^[0-9a-fA-F]+\s+([0-9a-fA-F]+)\s+\w\s+(\w+Bytes)$",
            line,
        )
        if match:
            sizes[match.group(2)] = int(match.group(1), 16)
    return sizes, command


def linked_functions(nm, cxxfilt, elf_path):
    functions = {}
    mangled_names = []
    records = []
    for line in output(
        (str(nm), "-S", "-n", "--defined-only", str(elf_path))
    ).splitlines():
        match = re.match(
            r"^([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+([TtWw])\s+(.+)$", line
        )
        if not match:
            continue
        record = {
            "address": int(match.group(1), 16),
            "size": int(match.group(2), 16),
            "mangled": match.group(4),
        }
        records.append(record)
        mangled_names.append(record["mangled"])
    demangled_names = output(
        (str(cxxfilt),), input="\n".join(mangled_names) + "\n"
    ).splitlines()
    if len(demangled_names) != len(records):
        raise ProbeError("avr-c++filt returned an incomplete symbol list")
    for record, demangled in zip(records, demangled_names):
        record["demangled"] = demangled
        functions[record["address"]] = record
    return functions


def stack_records(build_directory):
    records = []
    for usage_path in build_directory.rglob("*.su"):
        for line in usage_path.read_text(encoding="utf-8").splitlines():
            match = re.match(r"^(.+)\t(\d+)\t(\S+)$", line)
            if not match:
                continue
            signature_match = re.match(r"^[^:]+:\d+:\d+:(.+)$", match.group(1))
            if signature_match is None:
                continue
            records.append(
                {
                    "signature": signature_match.group(1),
                    "bytes": int(match.group(2)),
                    "kind": match.group(3),
                    "file": str(usage_path.relative_to(build_directory)),
                }
            )
    return records


def assign_stack(functions, records):
    def canonical(signature):
        value = signature.replace("{anonymous}", "(anonymous namespace)")
        value = re.sub(r"\s+\[clone [^\]]+\]$", "", value)
        template = re.search(r"\s+\[with (.*)\]$", value)
        if template:
            for assignment in template.group(1).split(";"):
                if "=" not in assignment:
                    continue
                name, replacement = assignment.split("=", 1)
                value = re.sub(
                    rf"\b{re.escape(name.strip())}\b",
                    replacement.strip(),
                    value,
                )
        value = re.sub(r"\s+\[with .*\]$", "", value)
        value = re.sub(
            r"\badk::(?:MicrosecondTimePoint|MicrosecondDuration)::Raw\b",
            "unsigned long",
            value,
        )
        aliases = {
            "uint8_t": "unsigned char",
            "int8_t": "signed char",
            "uint16_t": "unsigned int",
            "int16_t": "int",
            "uint32_t": "unsigned long",
            "int32_t": "long",
            "uintptr_t": "unsigned int",
        }
        for alias, underlying in aliases.items():
            value = re.sub(rf"\b{alias}\b", underlying, value)
        value = re.sub(r"([~A-Za-z_][A-Za-z0-9_:]*)<[^()]+>(?=\()", r"\1", value)
        value = re.sub(r"\s+", "", value)
        value = re.sub(
            r"const([A-Za-z_][A-Za-z0-9_:<>]*)(&|\*)",
            r"\1const\2",
            value,
        )
        return value

    for function in functions.values():
        demangled = function["demangled"]
        canonical_demangled = canonical(demangled)
        matches = [
            record
            for record in records
            if record["signature"] == demangled
            or record["signature"].endswith(" " + demangled)
            or canonical(record["signature"]).endswith(canonical_demangled)
            or (
                "(" not in demangled
                and re.search(
                    rf"(?:^|\s){re.escape(demangled)}\([^)]*\)$",
                    record["signature"],
                )
            )
        ]
        values = {(record["bytes"], record["kind"]) for record in matches}
        if len(values) == 1:
            function["stack"], function["stack_kind"] = values.pop()


def instruction_graph(objdump, elf_path, functions):
    instructions = {address: [] for address in functions}
    current = None
    for line in output((str(objdump), "-d", str(elf_path))).splitlines():
        header = re.match(r"^([0-9a-fA-F]+)\s+<[^>]+>:$", line)
        if header:
            header_address = int(header.group(1), 16)
            if header_address in instructions:
                current = header_address
            else:
                containing = [
                    address
                    for address, function in functions.items()
                    if address < header_address < address + function["size"]
                ]
                current = containing[0] if len(containing) == 1 else None
            continue
        if current is None:
            continue
        instruction = re.match(
            r"^\s*([0-9a-fA-F]+):\s+(?:[0-9a-fA-F]{2}\s+)+"
            r"\s*([a-z][a-z0-9.]*)\s*(.*)$",
            line,
        )
        if instruction:
            instructions[current].append(
                (
                    int(instruction.group(1), 16),
                    instruction.group(2),
                    instruction.group(3),
                )
            )

    def resolve_target(operand):
        match = re.search(r"\b0x([0-9a-fA-F]+)\b", operand)
        if match:
            target = int(match.group(1), 16)
        else:
            match = re.search(r"\b([0-9a-fA-F]+)\s+<", operand)
            if not match:
                return None
            target = int(match.group(1), 16)
        if target in functions:
            return target
        containing = [
            address
            for address, function in functions.items()
            if address <= target < address + function["size"]
        ]
        return containing[0] if len(containing) == 1 else None

    graph = {address: [] for address in functions}
    dynamic = {address: [] for address in functions}
    unresolved = {address: [] for address in functions}
    for address, body in instructions.items():
        for instruction_address, mnemonic, operand in body:
            if mnemonic in ("icall", "eicall", "ijmp", "eijmp"):
                dynamic[address].append(
                    f"0x{instruction_address:x}: {mnemonic} {operand}".rstrip()
                )
            elif mnemonic in ("call", "rcall", "jmp", "rjmp"):
                target = resolve_target(operand)
                if target is None:
                    unresolved[address].append(
                        f"0x{instruction_address:x}: {mnemonic} {operand}".rstrip()
                    )
                elif target != address:
                    edge_bytes = (
                        CALL_RETURN_BYTES if mnemonic in ("call", "rcall") else 0
                    )
                    graph[address].append((target, edge_bytes, mnemonic))
            elif mnemonic.startswith("br"):
                target = resolve_target(operand)
                if target is not None and target != address:
                    graph[address].append((target, 0, mnemonic))
        if body:
            instruction_address, mnemonic, _ = body[-1]
            following = [
                candidate
                for candidate in functions
                if instruction_address < candidate <= instruction_address + 4
            ]
            if (
                len(following) == 1
                and mnemonic not in ("ret", "reti", "jmp", "rjmp")
            ):
                graph[address].append((following[0], 0, "fallthrough"))
    return graph, dynamic, unresolved, instructions


def assign_assembly_stack(functions, graph, instructions):
    def target_address(operand):
        match = re.search(r"\b0x([0-9a-fA-F]+)\b", operand)
        return int(match.group(1), 16) if match else None

    conditional = re.compile(r"^(?:br(?!eak)|cpse|sbr[cs]|sbi[cs])")
    for function_address, function in functions.items():
        if "stack" in function:
            continue
        body = instructions[function_address]
        if not body:
            continue
        addresses = [record[0] for record in body]
        indexes = {address: index for index, address in enumerate(addresses)}
        states = [(addresses[0], 0, ())]
        observed = {}
        maximum = 0
        terminated = False
        unsupported = False
        while states:
            address, depth, returns = states.pop()
            state_key = (address, returns)
            prior = observed.get(state_key)
            if prior is not None:
                if prior != depth:
                    raise ProbeError(
                        "assembly stack depth is path-dependent or recursive in "
                        f"{function['demangled']} at 0x{address:x}"
                    )
                continue
            observed[state_key] = depth
            index = indexes[address]
            _, mnemonic, operand = body[index]
            if mnemonic == "push":
                depth += 1
            elif mnemonic == "rcall":
                target = target_address(operand)
                next_address = (
                    addresses[index + 1] if index + 1 < len(addresses) else None
                )
                if target == next_address:
                    depth += CALL_RETURN_BYTES
                elif (
                    target is not None
                    and target not in functions
                    and function_address <= target
                    < function_address + function["size"]
                ):
                    if target not in indexes or next_address is None:
                        raise ProbeError(
                            "unresolved intra-function assembly rcall in "
                            f"{function['demangled']}"
                        )
                    depth += CALL_RETURN_BYTES
                    maximum = max(maximum, depth)
                    states.append(
                        (target, depth, returns + (next_address,))
                    )
                    continue
            elif mnemonic == "pop":
                depth -= 1
                if depth < 0:
                    raise ProbeError(
                        f"assembly stack underflow in {function['demangled']}"
                    )
            elif mnemonic in ("ret", "reti"):
                if returns:
                    if depth < CALL_RETURN_BYTES:
                        raise ProbeError(
                            f"assembly call-stack underflow in "
                            f"{function['demangled']}"
                        )
                    states.append(
                        (returns[-1], depth - CALL_RETURN_BYTES, returns[:-1])
                    )
                    continue
                if depth != 0:
                    raise ProbeError(
                        "assembly return has an unbalanced stack in "
                        f"{function['demangled']}"
                    )
                terminated = True
                continue
            elif mnemonic in ("out", "sts"):
                token = operand.split(",", 1)[0].strip()
                try:
                    register_address = int(token, 0)
                except ValueError:
                    register_address = -1
                if register_address in (0x3D, 0x3E, 0x5D, 0x5E):
                    unsupported = True
                    break
            maximum = max(maximum, depth)
            next_addresses = []
            if mnemonic in ("jmp", "rjmp"):
                target = target_address(operand)
                if target in indexes:
                    next_addresses.append(target)
                elif depth != 0:
                    raise ProbeError(
                        f"tail transfer leaks stack in {function['demangled']}"
                    )
                else:
                    terminated = True
            elif conditional.match(mnemonic):
                if index + 1 < len(body):
                    next_addresses.append(addresses[index + 1])
                if mnemonic.startswith("br"):
                    target = target_address(operand)
                    if target in indexes:
                        next_addresses.append(target)
                elif index + 2 < len(body):
                    next_addresses.append(addresses[index + 2])
            elif index + 1 < len(body):
                next_addresses.append(addresses[index + 1])
            states.extend(
                (next_address, depth, returns)
                for next_address in next_addresses
            )
        if unsupported:
            continue
        if terminated or graph[function_address]:
            function["stack"] = maximum
            function["stack_kind"] = "static-objdump"


def exact_stack(functions, graph, dynamic, unresolved):
    roots = [
        address
        for address, function in functions.items()
        if function["demangled"] in ("setup", "loop", "setup()", "loop()")
    ]
    if len(roots) != 2:
        raise ProbeError("the linked fixture must expose exactly setup() and loop()")

    visiting = []
    memo = {}

    def visit(address):
        if address in visiting:
            cycle = visiting[visiting.index(address) :] + [address]
            names = " -> ".join(functions[item]["demangled"] for item in cycle)
            raise ProbeError(f"reachable recursion is forbidden: {names}")
        if address in memo:
            return memo[address]
        function = functions[address]
        if dynamic[address]:
            raise ProbeError(
                f"reachable dynamic/indirect transfer in {function['demangled']}: "
                + ", ".join(dynamic[address])
            )
        if unresolved[address]:
            raise ProbeError(
                f"reachable unresolved transfer in {function['demangled']}: "
                + ", ".join(unresolved[address])
            )
        if "stack" not in function:
            raise ProbeError(
                "reachable function has no compiler stack record: "
                f"{function['mangled']} ({function['demangled']}) at "
                f"0x{address:x}"
            )
        if function["stack_kind"] not in ("static", "static-objdump"):
            raise ProbeError(
                f"reachable function has {function['stack_kind']} stack usage: "
                f"{function['demangled']}"
            )
        visiting.append(address)
        best_total = function["stack"]
        best_path = []
        for target, edge_bytes, _ in graph[address]:
            child_bytes, child_path = visit(target)
            candidate = (
                function["stack"] + edge_bytes + child_bytes
                if edge_bytes
                else max(function["stack"], child_bytes)
            )
            if candidate > best_total:
                best_total = candidate
                best_path = child_path
        visiting.pop()
        total = best_total
        path = [
            {
                "address": f"0x{address:x}",
                "mangled": function["mangled"],
                "demangled": function["demangled"],
                "local_bytes": function["stack"],
            }
        ] + best_path
        memo[address] = total, path
        return memo[address]

    root_results = [visit(root) for root in roots]
    maximum = max(root_results, key=lambda item: item[0])
    reachable = sorted(memo)
    graph_evidence = {
        "nodes": [
            {
                "address": f"0x{address:x}",
                "mangled": functions[address]["mangled"],
                "demangled": functions[address]["demangled"],
                "local_bytes": functions[address]["stack"],
                "stack_kind": functions[address]["stack_kind"],
            }
            for address in reachable
        ],
        "edges": [
            {
                "caller_address": f"0x{caller:x}",
                "callee_address": f"0x{callee:x}",
                "instruction": mnemonic,
                "return_address_bytes": edge_bytes,
            }
            for caller in reachable
            for callee, edge_bytes, mnemonic in graph[caller]
            if callee in memo
        ],
    }
    return maximum[0], maximum[1], graph_evidence


def gate(value, target, hard):
    if value > hard:
        return "hard-fail"
    if value > target:
        return "target-miss"
    return "pass"


def load_reviews(root, review_path):
    path = root / review_path
    if not path.is_file():
        return {}
    document = json.loads(path.read_text(encoding="utf-8"))
    if document.get("schema") != 1 or not isinstance(document.get("reviews"), list):
        raise ProbeError(f"invalid target-miss review document: {path}")
    reviews = {}
    known_lessons = {boundary["lesson"] for boundary in BOUNDARIES}
    controlling_documents = {
        "docs/design/LESSONS_055_057_ESCAPE_CONSOLE_PLAN.md": {
            "resource-budgets-and-mandatory-probes"
        },
        "docs/design/LESSON_057_ESCAPE_CONSOLE_STRESS_PASS.md": {"gate-result"},
    }
    for review in document["reviews"]:
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
        if set(review) != required:
            raise ProbeError(f"invalid target-miss review fields: {review}")
        if review["disposition"] != "accepted-target-miss":
            raise ProbeError(f"invalid target-miss disposition: {review}")
        if review["lesson"] not in known_lessons:
            raise ProbeError(f"target-miss review names unknown lesson: {review}")
        authority_parts = review["authority"].split("#", 1)
        if (
            len(authority_parts) != 2
            or authority_parts[0] not in controlling_documents
            or authority_parts[1] not in controlling_documents[authority_parts[0]]
        ):
            raise ProbeError(
                f"target-miss review lacks controlling authority: {review}"
            )
        authority = root / authority_parts[0]
        if not authority.is_file():
            raise ProbeError(f"target-miss review authority is missing: {authority}")
        marker = (
            f"Resource-review: lesson={review['lesson']} "
            f"metric={review['metric']} observed={review['observed_bytes']} "
            f"target={review['target_bytes']} hard={review['hard_bytes']} "
            f"disposition={review['disposition']}"
        )
        text = authority.read_text(encoding="utf-8")
        outside_fence = []
        fenced = False
        fence_marker = None
        for line in text.splitlines():
            opening = re.match(r"^ {0,3}(`{3,}|~{3,})(.*)$", line)
            if not fenced and opening:
                marker_character = opening.group(1)[0]
                if marker_character == "`" and "`" in opening.group(2):
                    outside_fence.append(line)
                else:
                    fenced = True
                    fence_marker = (marker_character, len(opening.group(1)))
                    outside_fence.append(None)
            elif fenced and re.match(
                rf"^ {{0,3}}{re.escape(fence_marker[0])}"
                rf"{{{fence_marker[1]},}} *$",
                line,
            ):
                fenced = False
                fence_marker = None
                outside_fence.append(None)
            else:
                outside_fence.append(None if fenced else line)
        headings = []
        for line_number, line in enumerate(outside_fence):
            if line is None:
                continue
            heading = re.match(r"^(##+) (.+)$", line)
            if heading:
                headings.append((line_number, len(heading.group(1)), heading.group(2)))
        selected = None
        for index, heading in enumerate(headings):
            slug = re.sub(
                r"[^a-z0-9 -]", "", heading[2].strip().lower()
            ).replace(" ", "-")
            if slug != authority_parts[1]:
                continue
            end = len(outside_fence)
            level = heading[1]
            for following in headings[index + 1 :]:
                if following[1] <= level:
                    end = following[0]
                    break
            selected = outside_fence[heading[0] + 1 : end]
            break
        if selected is None or marker not in selected:
            raise ProbeError(
                f"target-miss review marker is absent from {review['authority']}: "
                f"{marker}"
            )
        key = (review["lesson"], review["metric"])
        if key in reviews:
            raise ProbeError(f"duplicate target-miss review: {key}")
        reviews[key] = review
    return reviews


def apply_reviews(boundary, measurements, gates, reviews, validated_reviews):
    measurement_names = {
        "flash": "flash_bytes",
        "static_sram": "static_sram_bytes",
        "synchronous_stack": "synchronous_stack_bytes",
        "object": "object_bytes",
    }
    limit_names = {
        "flash": ("flash_target", "flash_hard"),
        "static_sram": ("sram_target", "sram_hard"),
        "synchronous_stack": ("stack_target", "stack_hard"),
        "object": ("object_target", "object_hard"),
    }
    accepted = []
    lesson_reviews = {
        metric: review
        for (lesson, metric), review in reviews.items()
        if lesson == boundary["lesson"]
    }
    for metric, review in lesson_reviews.items():
        if metric not in measurement_names:
            raise ProbeError(
                f"unsupported reviewed metric for Lesson {boundary['lesson']}: "
                f"{metric}"
            )
        target_name, hard_name = limit_names[metric]
        expected = (
            measurements[measurement_names[metric]],
            boundary[target_name],
            boundary[hard_name],
        )
        reviewed = (
            review["observed_bytes"],
            review["target_bytes"],
            review["hard_bytes"],
        )
        if reviewed != expected or gates[metric] != "target-miss":
            raise ProbeError(
                f"stale target-miss review for Lesson {boundary['lesson']} "
                f"{metric}: reviewed {reviewed}, measured {expected}, "
                f"current disposition {gates[metric]}"
            )
        validated_reviews.add((boundary["lesson"], metric))
    for metric, disposition in tuple(gates.items()):
        if disposition != "target-miss" or metric not in measurement_names:
            continue
        review = lesson_reviews.get(metric)
        if review is None:
            continue
        gates[metric] = "reviewed-target-miss"
        accepted.append(review)
    return accepted


def boundary_available(root, boundary):
    if boundary["lesson"] != "057":
        return (root / boundary["sketch"]).is_dir()
    required = (
        root / "src/inert_escape_console.h",
        root / "src/inert_escape_console.cpp",
        root
        / "examples/Lesson057InertEscapeConsole/Lesson057InertEscapeConsole.ino",
        root / boundary["sketch"] / "EscapeConsoleMaximumComposition.ino",
    )
    return all(path.is_file() for path in required)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--arduino-cli", default="arduino-cli")
    parser.add_argument("--fqbn", default="arduino:avr:mega")
    parser.add_argument(
        "--evidence-json",
        default="build/evidence/escape-console-resource-probe.json",
    )
    parser.add_argument(
        "--require-complete",
        action="store_true",
        help="fail when the Lesson 057 maximum-composition fixture is pending",
    )
    parser.add_argument(
        "--review-file",
        default="probes/escape_console_resource_reviews.json",
    )
    arguments = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parent.parent
    evidence_path = root / arguments.evidence_json
    temporary = pathlib.Path(tempfile.mkdtemp(prefix="adk-escape-probe."))
    report = {
        "schema": 1,
        "fqbn": arguments.fqbn,
        "constants": {
            "board_sram_bytes": BOARD_SRAM,
            "call_return_bytes": CALL_RETURN_BYTES,
            "isr_reserve_bytes": ISR_RESERVE,
            "minimum_residual_sram_bytes": MINIMUM_RESIDUAL_SRAM,
        },
        "boundaries": [],
        "commands": [],
        "status": "pass",
    }
    exit_code = 0
    try:
        reviews = load_reviews(root, arguments.review_file)
        available = [
            boundary
            for boundary in BOUNDARIES
            if boundary_available(root, boundary)
        ]
        if not available:
            raise ProbeError("no escape-console lesson fixture is available")

        builds = {}
        build_errors = {}
        validated_reviews = set()
        tools = None
        for boundary in available:
            try:
                build_directory, elf_path, command = compile_sketch(
                    arguments, root, temporary, boundary
                )
            except subprocess.CalledProcessError as error:
                build_errors[boundary["lesson"]] = str(error)
                continue
            report["commands"].append(command)
            if tools is None:
                tools = {
                    "compiler": tool_beside(build_directory, "avr-g++"),
                    "nm": tool_beside(build_directory, "avr-nm"),
                    "size": tool_beside(build_directory, "avr-size"),
                    "objdump": tool_beside(build_directory, "avr-objdump"),
                    "cxxfilt": tool_beside(build_directory, "avr-c++filt"),
                }
            builds[boundary["lesson"]] = (build_directory, elf_path)

        if tools is None:
            raise ProbeError("every available linked fixture build failed")
        sizes, size_command = object_sizes(
            tools["compiler"],
            tools["nm"],
            root,
            temporary,
            any(boundary["lesson"] == "057" for boundary in available),
        )
        report["commands"].append(size_command)
        report["tools"] = {
            "arduino_cli": output(
                (arguments.arduino_cli, "version"), stderr=subprocess.STDOUT
            ).strip(),
            **{name: tool_version(path) for name, path in tools.items()},
        }

        for boundary in BOUNDARIES:
            lesson = boundary["lesson"]
            if lesson in build_errors:
                state = {
                    "lesson": lesson,
                    "status": "error",
                    "reason": build_errors[lesson],
                }
                object_bytes = sizes.get(boundary["object_symbol"])
                if object_bytes is not None:
                    state["measurements"] = {"object_bytes": object_bytes}
                    state["gates"] = {
                        "object": gate(
                            object_bytes,
                            boundary["object_target"],
                            boundary["object_hard"],
                        )
                    }
                report["boundaries"].append(state)
                report["status"] = merge_status(report["status"], "error")
                exit_code = 1
                print(f"Lesson {lesson}: linked fixture build failed")
                continue
            if lesson not in builds:
                state = {
                    "lesson": lesson,
                    "status": "pending",
                    "reason": (
                        "Lesson 057 source and maximum-composition fixture "
                        "are not both present"
                    ),
                }
                report["boundaries"].append(state)
                report["status"] = merge_status(report["status"], "pending")
                print(f"Lesson {lesson}: pending maximum-composition implementation")
                if arguments.require_complete:
                    exit_code = 1
                continue

            build_directory, elf_path = builds[lesson]
            flash, static_sram = section_sizes(tools["size"], elf_path)
            functions = linked_functions(tools["nm"], tools["cxxfilt"], elf_path)
            assign_stack(functions, stack_records(build_directory))
            graph, dynamic, unresolved, instructions = instruction_graph(
                tools["objdump"], elf_path, functions
            )
            assign_assembly_stack(functions, graph, instructions)
            synchronous_stack, stack_path, call_graph = exact_stack(
                functions, graph, dynamic, unresolved
            )
            object_bytes = sizes.get(boundary["object_symbol"])
            if object_bytes is None:
                raise ProbeError(
                    f"object-size symbol {boundary['object_symbol']} is missing"
                )
            measurements = {
                "flash_bytes": flash,
                "static_sram_bytes": static_sram,
                "synchronous_stack_bytes": synchronous_stack,
                "object_bytes": object_bytes,
            }
            gates = {
                "flash": gate(
                    flash, boundary["flash_target"], boundary["flash_hard"]
                ),
                "static_sram": gate(
                    static_sram, boundary["sram_target"], boundary["sram_hard"]
                ),
                "synchronous_stack": gate(
                    synchronous_stack,
                    boundary["stack_target"],
                    boundary["stack_hard"],
                ),
                "object": gate(
                    object_bytes,
                    boundary["object_target"],
                    boundary["object_hard"],
                ),
            }
            state = {
                "lesson": lesson,
                "measurements": measurements,
                "gates": gates,
                "stack_path": stack_path,
                "linked_call_graph": call_graph,
                "elf_sha256": sha256(elf_path),
                "map_sha256": sha256(build_directory / "firmware.map"),
                "no_lto": True,
            }
            if lesson == "057":
                clue_bytes = sizes["clueConstraintModelObjectBytes"]
                panel_bytes = sizes["faultAwareOperatorPanelObjectBytes"]
                child_lower_bound = clue_bytes + panel_bytes
                snapshot_bytes = sizes["escapeConsoleSnapshotBytes"]
                family_snapshot_bytes = sizes["escapeFamilySnapshotBytes"]
                parent_overhead = object_bytes - child_lower_bound
                if parent_overhead < 0:
                    raise ProbeError(
                        "Lesson 057 object is smaller than its value-owned children"
                    )
                state["measurements"]["owned_child_objects"] = {
                    "clue_constraint_model_bytes": clue_bytes,
                    "fault_aware_operator_panel_bytes": panel_bytes,
                    "mechanical_lower_bound_bytes": child_lower_bound,
                    "parent_state_overhead_bytes": parent_overhead,
                    "object_target_bytes": boundary["object_target"],
                    "public_value_sizes": {
                        "escape_console_snapshot_bytes": snapshot_bytes,
                        "escape_family_snapshot_bytes": family_snapshot_bytes,
                    },
                }
                residual = (
                    BOARD_SRAM
                    - static_sram
                    - synchronous_stack
                    - ISR_RESERVE
                )
                state["measurements"]["isr_reserve_bytes"] = ISR_RESERVE
                state["measurements"]["residual_sram_bytes"] = residual
                state["gates"]["residual_sram"] = (
                    "pass"
                    if residual >= MINIMUM_RESIDUAL_SRAM
                    else "hard-fail"
                )
            state["accepted_reviews"] = apply_reviews(
                boundary,
                measurements,
                state["gates"],
                reviews,
                validated_reviews,
            )
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
            if state["status"] == "hard-fail":
                report["status"] = merge_status(report["status"], "hard-fail")
                exit_code = 1
            elif state["status"] == "review-required":
                report["status"] = merge_status(report["status"], "review-required")
                exit_code = 1
            elif state["status"] == "reviewed-target-miss":
                report["status"] = merge_status(
                    report["status"], "reviewed-target-miss"
                )
            report["boundaries"].append(state)
            print(
                f"Lesson {lesson}: flash {flash} B; static SRAM "
                f"{static_sram} B; synchronous stack {synchronous_stack} B; "
                f"object {object_bytes} B ({state['status']})"
            )
        unvalidated_reviews = set(reviews) - validated_reviews
        if unvalidated_reviews:
            raise ProbeError(
                "target-miss reviews lack a successful current measurement: "
                f"{sorted(unvalidated_reviews)}"
            )
        evidence_path.parent.mkdir(parents=True, exist_ok=True)
        evidence_path.write_text(
            json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
        )
        print(f"Evidence: {evidence_path.relative_to(root)}")
        return exit_code
    except (OSError, ProbeError, subprocess.CalledProcessError) as error:
        report["status"] = "error"
        report["error"] = str(error)
        evidence_path.parent.mkdir(parents=True, exist_ok=True)
        evidence_path.write_text(
            json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
        )
        print(f"Escape-console resource probe error: {error}", file=sys.stderr)
        return 1
    finally:
        shutil.rmtree(temporary)


if __name__ == "__main__":
    sys.exit(main())
