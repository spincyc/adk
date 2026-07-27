#!/usr/bin/env python3
"""Dependency-free helper for the repository's durable AI journal."""

from __future__ import annotations

import argparse
import datetime as dt
import json
import os
from pathlib import Path
import socket
import sys
import time
import uuid

STATUSES = {
    "queued", "active", "verifying", "blocked", "supersession-proposed",
    "superseded", "done", "cancelled",
}
PRIORITIES = {"critical", "high", "normal", "low"}
TERMINAL = {"superseded", "done", "cancelled"}
ORDER = {"critical": 0, "high": 1, "normal": 2, "low": 3}
FEEDBACK_SEVERITIES = {"critical", "high", "normal", "low"}
FEEDBACK_DISPOSITIONS = {"accepted", "deferred", "rejected", "duplicate"}


def now() -> str:
    return dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def root() -> Path:
    candidate = Path(__file__).resolve().parents[2]
    if not (candidate / ".git").exists():
        raise SystemExit("journal helper is not inside a Git working tree")
    return candidate


def parse_document(path: Path) -> tuple[dict, str]:
    text = path.read_text(encoding="utf-8")
    lines = text.splitlines()
    if not lines or lines[0] != "---":
        raise ValueError(f"{path}: missing front matter")
    try:
        end = lines.index("---", 1)
    except ValueError as exc:
        raise ValueError(f"{path}: unterminated front matter") from exc
    data: dict = {}
    for number, line in enumerate(lines[1:end], 2):
        if not line or line.startswith("#"):
            continue
        if ":" not in line:
            raise ValueError(f"{path}:{number}: expected key: JSON-value")
        key, raw = line.split(":", 1)
        key = key.strip()
        if not key or key in data:
            raise ValueError(f"{path}:{number}: invalid or duplicate key")
        try:
            data[key] = json.loads(raw.strip())
        except json.JSONDecodeError as exc:
            raise ValueError(f"{path}:{number}: values must use JSON-compatible YAML") from exc
    body = "\n".join(lines[end + 1:]).strip() + "\n"
    return data, body


def document(data: dict, body: str) -> str:
    fields = ["---"]
    for key, value in data.items():
        fields.append(f"{key}: {json.dumps(value, ensure_ascii=False)}")
    fields.extend(["---", "", body.strip(), ""])
    return "\n".join(fields)


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(f".{path.name}.{uuid.uuid4()}.tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def task_states() -> dict[str, tuple[Path, dict]]:
    result = {}
    for path in sorted((root() / ".journal/tasks").glob("*/state.md")):
        data, _ = parse_document(path)
        task_id = data.get("task_uuid")
        if task_id in result:
            raise ValueError(f"duplicate task UUID: {task_id}")
        result[task_id] = (path, data)
    return result


def validate_uuid(value: object, label: str) -> None:
    try:
        uuid.UUID(str(value))
    except (ValueError, AttributeError, TypeError) as exc:
        raise ValueError(f"invalid {label}: {value!r}") from exc


def find_cycle(tasks: dict[str, tuple[Path, dict]]) -> list[str] | None:
    visiting: set[str] = set()
    visited: set[str] = set()

    def visit(task_id: str, path: list[str]) -> list[str] | None:
        if task_id in visiting:
            return path[path.index(task_id):] + [task_id]
        if task_id in visited:
            return None
        visiting.add(task_id)
        for dependency in tasks[task_id][1].get("hard_dependencies", []):
            if dependency not in tasks:
                continue
            cycle = visit(dependency, path + [dependency])
            if cycle:
                return cycle
        visiting.remove(task_id)
        visited.add(task_id)
        return None

    for candidate in tasks:
        cycle = visit(candidate, [candidate])
        if cycle:
            return cycle
    return None


def validate() -> None:
    journal = root() / ".journal"
    errors: list[str] = []
    try:
        repository, _ = parse_document(journal / "state.md")
        validate_uuid(repository.get("repository_uuid"), "repository_uuid")
        tasks = task_states()
        for task_id, (path, data) in tasks.items():
            validate_uuid(task_id, f"{path} task_uuid")
            if path.parent.name != task_id:
                errors.append(f"{path}: directory does not match task_uuid")
            if data.get("status") not in STATUSES:
                errors.append(f"{path}: invalid status {data.get('status')!r}")
            if data.get("priority") not in PRIORITIES:
                errors.append(f"{path}: invalid priority {data.get('priority')!r}")
            for field in ("hard_dependencies", "soft_dependencies", "related_to"):
                for reference in data.get(field, []):
                    if reference not in tasks:
                        errors.append(f"{path}: unknown {field} task {reference}")
            for field in ("parent", "discovered_by", "superseded_by"):
                reference = data.get(field)
                if reference is not None and reference not in tasks:
                    errors.append(f"{path}: unknown {field} task {reference}")
            for subtree in ("events", "decisions"):
                if not (path.parent / subtree).is_dir():
                    errors.append(f"{path.parent}: missing {subtree}/")
        cycle = find_cycle(tasks)
        if cycle:
            errors.append("hard dependency cycle: " + " -> ".join(cycle))
        seen: set[str] = set()
        decision_ids: set[str] = set()
        for kind, key in (("events", "event_uuid"), ("decisions", "decision_uuid")):
            for path in journal.glob(f"**/{kind}/*.md"):
                data, _ = parse_document(path)
                record_id = data.get(key)
                validate_uuid(record_id, f"{path} {key}")
                if path.stem != record_id:
                    errors.append(f"{path}: filename does not match {key}")
                if record_id in seen:
                    errors.append(f"{path}: duplicate record UUID {record_id}")
                seen.add(record_id)
                if kind == "decisions":
                    decision_ids.add(record_id)
                for task_id in data.get("task_ids", []):
                    if task_id not in tasks:
                        errors.append(f"{path}: unknown task {task_id}")
        feedback_ids: set[str] = set()
        for path in sorted((journal / "feedback/reports").glob("*.md")):
            data, body = parse_document(path)
            feedback_id = data.get("feedback_uuid")
            validate_uuid(feedback_id, f"{path} feedback_uuid")
            if path.stem != feedback_id:
                errors.append(f"{path}: filename does not match feedback_uuid")
            if feedback_id in seen:
                errors.append(f"{path}: duplicate record UUID {feedback_id}")
            seen.add(feedback_id)
            feedback_ids.add(feedback_id)
            validate_uuid(
                data.get("agent_instance_uuid"),
                f"{path} agent_instance_uuid",
            )
            if data.get("severity") not in FEEDBACK_SEVERITIES:
                errors.append(
                    f"{path}: invalid severity {data.get('severity')!r}"
                )
            for task_id in data.get("task_ids", []):
                if task_id not in tasks:
                    errors.append(f"{path}: unknown task {task_id}")
            affected_paths = data.get("affected_paths")
            if not isinstance(affected_paths, list) or not affected_paths:
                errors.append(f"{path}: affected_paths must be a nonempty list")
            elif not all(isinstance(item, str) and item for item in affected_paths):
                errors.append(f"{path}: affected_paths entries must be strings")
            for heading in (
                "# ",
                "## Evidence\n",
                "## Impact\n",
                "## Proposed change\n",
            ):
                if heading not in body:
                    errors.append(f"{path}: missing body heading {heading.strip()!r}")
        reviews: dict[str, tuple[Path, dict]] = {}
        superseded_reviews: set[str] = set()
        for path in sorted((journal / "feedback/reviews").glob("*.md")):
            data, body = parse_document(path)
            review_id = data.get("review_uuid")
            validate_uuid(review_id, f"{path} review_uuid")
            if path.stem != review_id:
                errors.append(f"{path}: filename does not match review_uuid")
            if review_id in seen:
                errors.append(f"{path}: duplicate record UUID {review_id}")
            seen.add(review_id)
            reviews[review_id] = (path, data)
            validate_uuid(
                data.get("agent_instance_uuid"),
                f"{path} agent_instance_uuid",
            )
            if data.get("feedback_uuid") not in feedback_ids:
                errors.append(f"{path}: unknown feedback_uuid")
            if data.get("disposition") not in FEEDBACK_DISPOSITIONS:
                errors.append(
                    f"{path}: invalid disposition {data.get('disposition')!r}"
                )
            task_id = data.get("task_uuid")
            if task_id not in tasks:
                errors.append(f"{path}: unknown task_uuid {task_id}")
            decision_id = data.get("decision_uuid")
            if data.get("disposition") == "accepted" and decision_id is None:
                errors.append(f"{path}: accepted review requires decision_uuid")
            if decision_id is not None and decision_id not in decision_ids:
                errors.append(f"{path}: unknown decision_uuid {decision_id}")
            supersedes = data.get("supersedes")
            if supersedes is not None:
                validate_uuid(supersedes, f"{path} supersedes")
                if supersedes in superseded_reviews:
                    errors.append(f"{path}: review history branches at {supersedes}")
                superseded_reviews.add(supersedes)
            for heading in ("# ", "## Rationale\n", "## Follow-up\n"):
                if heading not in body:
                    errors.append(f"{path}: missing body heading {heading.strip()!r}")
        roots: dict[str, int] = {}
        for review_id, (path, data) in reviews.items():
            supersedes = data.get("supersedes")
            if supersedes is not None:
                if supersedes not in reviews:
                    errors.append(f"{path}: unknown superseded review {supersedes}")
                elif (
                    reviews[supersedes][1].get("feedback_uuid")
                    != data.get("feedback_uuid")
                ):
                    errors.append(f"{path}: cannot supersede another report's review")
            else:
                feedback_id = data.get("feedback_uuid")
                roots[feedback_id] = roots.get(feedback_id, 0) + 1
        for feedback_id, count in roots.items():
            if count > 1:
                errors.append(
                    f"feedback {feedback_id}: multiple root reviews"
                )
        for review_id in reviews:
            chain: set[str] = set()
            candidate: str | None = review_id
            while candidate is not None and candidate in reviews:
                if candidate in chain:
                    errors.append(
                        f"feedback review cycle includes {candidate}"
                    )
                    break
                chain.add(candidate)
                candidate = reviews[candidate][1].get("supersedes")
        for path in journal.glob("leases/*/state.md"):
            data, _ = parse_document(path)
            if data.get("task_uuid") not in tasks:
                errors.append(f"{path}: lease references unknown task")
            validate_uuid(data.get("agent_instance_uuid"), f"{path} agent_instance_uuid")
    except (OSError, ValueError) as exc:
        errors.append(str(exc))
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
    print(f"journal valid: {len(tasks)} task(s)")


def render_queue(tasks: dict[str, tuple[Path, dict]]) -> str:
    groups = [
        ("Active", {"active", "verifying"}),
        ("Queued", {"queued", "supersession-proposed"}),
        ("Blocked", {"blocked"}),
        ("Terminal", TERMINAL),
    ]
    lines = [
        "---",
        "schema_version: 1",
        f"generated_at: {json.dumps(now())}",
        f"task_count: {len(tasks)}",
        "---",
        "",
        "# Work queue",
        "",
        "This file is a rebuildable view. Task `state.md` files are authoritative.",
    ]
    for heading, statuses in groups:
        lines.extend(["", f"## {heading}", ""])
        selected = [
            data for _, data in tasks.values() if data["status"] in statuses
        ]
        selected.sort(key=lambda item: (
            ORDER.get(item["priority"], 99), item["created_at"], item["task_uuid"]
        ))
        if selected:
            for data in selected:
                dependencies = data.get("hard_dependencies", [])
                suffix = f" (depends on {', '.join(dependencies)})" if dependencies else ""
                lines.append(
                    f"- `{data['task_uuid']}` — {data['title']} "
                    f"[{data['status']}, {data['priority']}]{suffix}"
                )
        else:
            lines.append("None.")
    return "\n".join(lines) + "\n"


def rebuild_queue() -> None:
    tasks = task_states()
    atomic_write(root() / ".journal/queue.md", render_queue(tasks))
    print(f"rebuilt queue for {len(tasks)} task(s)")


def new_task(args: argparse.Namespace) -> None:
    task_id = str(uuid.uuid4())
    timestamp = now()
    task = root() / ".journal/tasks" / task_id
    data = {
        "schema_version": 1,
        "task_uuid": task_id,
        "title": args.title,
        "status": "queued",
        "priority": args.priority,
        "priority_reason": args.priority_reason,
        "parent": args.parent,
        "discovered_by": args.discovered_by,
        "hard_dependencies": [],
        "soft_dependencies": [],
        "related_to": [],
        "superseded_by": None,
        "created_at": timestamp,
        "updated_at": timestamp,
    }
    body = (
        f"# Goal\n\n{args.goal}\n\n"
        "## Acceptance criteria\n\n"
        "- Refine during bounded ingestion or activation.\n\n"
        "## Original request\n\n"
        f"{args.request}\n"
    )
    atomic_write(task / "state.md", document(data, body))
    (task / "events").mkdir(parents=True)
    (task / "decisions").mkdir()
    (task / "decisions" / ".gitkeep").touch()
    event_id = str(uuid.uuid4())
    event_data = {
        "schema_version": 1,
        "event_uuid": event_id,
        "event_type": "ingestion",
        "scope": "task",
        "task_ids": [task_id],
        "agent_instance_uuid": args.agent,
        "created_at": timestamp,
    }
    event_body = (
        "# Request ingestion\n\n"
        "Created through bounded enrichment. Review the task goal, acceptance "
        "criteria, dependencies, overlap, and redactions before activation.\n\n"
        "## Original request\n\n"
        f"{args.request}\n"
    )
    atomic_write(
        task / "events" / f"{event_id}.md",
        document(event_data, event_body),
    )
    rebuild_queue()
    print(task_id)


def process_start_token(pid: int) -> str | None:
    path = Path(f"/proc/{pid}/stat")
    try:
        return path.read_text(encoding="utf-8").split()[21]
    except (OSError, IndexError):
        return None


def boot_id() -> str | None:
    try:
        return Path("/proc/sys/kernel/random/boot_id").read_text(encoding="utf-8").strip()
    except OSError:
        return None


def lock_acquire(args: argparse.Namespace) -> None:
    lock_id = acquire_lock(args.agent, args.operation, args.task)
    print(lock_id)


def acquire_lock(
    agent: str,
    operation: str,
    task_ids: list[str],
    wait_seconds: float = 0,
) -> str:
    lock = root() / ".journal/.lock"
    deadline = time.monotonic() + wait_seconds
    while True:
        try:
            lock.mkdir()
            break
        except FileExistsError:
            if time.monotonic() >= deadline:
                raise SystemExit(
                    "journal lock already exists; inspect it before recovery"
                )
            time.sleep(0.05)
    lock_id = str(uuid.uuid4())
    owner = {
        "lock_uuid": lock_id,
        "agent_instance_uuid": agent,
        "pid": os.getpid(),
        "hostname": socket.gethostname(),
        "boot_id": boot_id(),
        "process_start_token": process_start_token(os.getpid()),
        "acquired_at": now(),
        "operation": operation,
        "task_ids": task_ids,
    }
    try:
        atomic_write(lock / "owner.json", json.dumps(owner, indent=2) + "\n")
    except BaseException:
        lock.rmdir()
        raise
    return lock_id


def lock_status() -> None:
    owner = root() / ".journal/.lock/owner.json"
    if not owner.exists():
        print("unlocked")
        return
    print(owner.read_text(encoding="utf-8"), end="")


def lock_release(args: argparse.Namespace) -> None:
    release_lock(args.lock)
    print("released")


def release_lock(lock_id: str) -> None:
    lock = root() / ".journal/.lock"
    owner_path = lock / "owner.json"
    if not owner_path.exists():
        raise SystemExit("lock is absent or incompletely initialized")
    owner = json.loads(owner_path.read_text(encoding="utf-8"))
    if owner.get("lock_uuid") != lock_id:
        raise SystemExit("lock UUID mismatch; refusing release")
    owner_path.unlink()
    lock.rmdir()


def feedback_submit(args: argparse.Namespace) -> None:
    validate_uuid(args.agent, "agent_instance_uuid")
    tasks = task_states()
    for task_id in args.task:
        if task_id not in tasks:
            raise SystemExit(f"unknown task: {task_id}")
    feedback_id = str(uuid.uuid4())
    timestamp = now()
    data = {
        "schema_version": 1,
        "feedback_uuid": feedback_id,
        "severity": args.severity,
        "agent_instance_uuid": args.agent,
        "task_ids": args.task,
        "affected_paths": args.path,
        "created_at": timestamp,
    }
    body = (
        f"# {args.summary}\n\n"
        f"## Evidence\n\n{args.evidence}\n\n"
        f"## Impact\n\n{args.impact}\n\n"
        f"## Proposed change\n\n{args.proposal}\n"
    )
    lock_id = acquire_lock(
        args.agent,
        "publish-journal-feedback",
        args.task,
        wait_seconds=5,
    )
    try:
        target = root() / ".journal/feedback/reports" / f"{feedback_id}.md"
        if target.exists():
            raise SystemExit("feedback UUID collision; retry submission")
        atomic_write(target, document(data, body))
    finally:
        release_lock(lock_id)
    print(feedback_id)


def current_review(feedback_id: str) -> str | None:
    reviews: dict[str, dict] = {}
    superseded: set[str] = set()
    for path in (root() / ".journal/feedback/reviews").glob("*.md"):
        data, _ = parse_document(path)
        if data.get("feedback_uuid") == feedback_id:
            review_id = data.get("review_uuid")
            reviews[review_id] = data
            if data.get("supersedes") is not None:
                superseded.add(data["supersedes"])
    heads = set(reviews) - superseded
    if len(heads) > 1:
        raise SystemExit("feedback has branched reviews; reconcile before review")
    return next(iter(heads), None)


def feedback_review(args: argparse.Namespace) -> None:
    validate_uuid(args.agent, "agent_instance_uuid")
    validate_uuid(args.feedback, "feedback_uuid")
    if args.task not in task_states():
        raise SystemExit(f"unknown task: {args.task}")
    if args.decision is not None:
        validate_uuid(args.decision, "decision_uuid")
        decisions = {
            parse_document(path)[0].get("decision_uuid")
            for path in (root() / ".journal").glob("**/decisions/*.md")
        }
        if args.decision not in decisions:
            raise SystemExit(f"unknown decision: {args.decision}")
    lock_id = acquire_lock(
        args.agent,
        "publish-journal-feedback-review",
        [args.task],
        wait_seconds=5,
    )
    try:
        report = (
            root() / ".journal/feedback/reports" / f"{args.feedback}.md"
        )
        if not report.exists():
            raise SystemExit(f"unknown feedback report: {args.feedback}")
        if args.disposition == "accepted" and args.decision is None:
            raise SystemExit("accepted feedback requires --decision")
        review_id = str(uuid.uuid4())
        data = {
            "schema_version": 1,
            "review_uuid": review_id,
            "feedback_uuid": args.feedback,
            "disposition": args.disposition,
            "agent_instance_uuid": args.agent,
            "task_uuid": args.task,
            "decision_uuid": args.decision,
            "supersedes": current_review(args.feedback),
            "created_at": now(),
        }
        body = (
            f"# Feedback {args.disposition}\n\n"
            f"## Rationale\n\n{args.rationale}\n\n"
            f"## Follow-up\n\n{args.follow_up}\n"
        )
        target = root() / ".journal/feedback/reviews" / f"{review_id}.md"
        atomic_write(target, document(data, body))
    finally:
        release_lock(lock_id)
    print(review_id)


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(description=__doc__)
    commands = result.add_subparsers(dest="command", required=True)
    commands.add_parser("validate")
    commands.add_parser("rebuild-queue")
    create = commands.add_parser("new-task")
    create.add_argument("--title", required=True)
    create.add_argument("--goal", required=True)
    create.add_argument("--request", required=True)
    create.add_argument("--agent", required=True)
    create.add_argument("--priority", choices=sorted(PRIORITIES), default="normal")
    create.add_argument("--priority-reason", default="Default user-requested work")
    create.add_argument("--parent")
    create.add_argument("--discovered-by")
    lock = commands.add_parser("lock")
    lock_commands = lock.add_subparsers(dest="lock_command", required=True)
    acquire = lock_commands.add_parser("acquire")
    acquire.add_argument("--agent", required=True)
    acquire.add_argument("--operation", required=True)
    acquire.add_argument("--task", action="append", default=[])
    lock_commands.add_parser("status")
    release = lock_commands.add_parser("release")
    release.add_argument("--lock", required=True)
    feedback = commands.add_parser("feedback")
    feedback_commands = feedback.add_subparsers(
        dest="feedback_command",
        required=True,
    )
    submit = feedback_commands.add_parser("submit")
    submit.add_argument("--agent", required=True)
    submit.add_argument("--severity", choices=sorted(FEEDBACK_SEVERITIES), default="normal")
    submit.add_argument("--task", action="append", default=[])
    submit.add_argument("--path", action="append", required=True)
    submit.add_argument("--summary", required=True)
    submit.add_argument("--evidence", required=True)
    submit.add_argument("--impact", required=True)
    submit.add_argument("--proposal", required=True)
    review = feedback_commands.add_parser("review")
    review.add_argument("--agent", required=True)
    review.add_argument("--feedback", required=True)
    review.add_argument("--disposition", choices=sorted(FEEDBACK_DISPOSITIONS), required=True)
    review.add_argument("--task", required=True)
    review.add_argument("--decision")
    review.add_argument("--rationale", required=True)
    review.add_argument("--follow-up", required=True)
    return result


def main() -> None:
    args = parser().parse_args()
    if args.command == "validate":
        validate()
    elif args.command == "rebuild-queue":
        rebuild_queue()
    elif args.command == "new-task":
        new_task(args)
    elif args.command == "feedback" and args.feedback_command == "submit":
        feedback_submit(args)
    elif args.command == "feedback":
        feedback_review(args)
    elif args.lock_command == "acquire":
        lock_acquire(args)
    elif args.lock_command == "status":
        lock_status()
    else:
        lock_release(args)


if __name__ == "__main__":
    main()
