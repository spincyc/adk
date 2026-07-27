#!/usr/bin/env python3
"""Validate the immutable input and metadata for a non-publishing release gate."""

import argparse
import pathlib
import re
import subprocess
import sys


ROOT = pathlib.Path(__file__).resolve().parents[1]


def git(*arguments: str) -> str:
    result = subprocess.run(
        ["git", *arguments],
        cwd=ROOT,
        check=True,
        text=True,
        stdout=subprocess.PIPE,
    )
    return result.stdout.strip()


def properties(path: pathlib.Path) -> dict[str, str]:
    values: dict[str, str] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if line and not line.startswith("#"):
            key, separator, value = line.partition("=")
            if not separator or not key or not value:
                raise ValueError(f"invalid metadata line: {line}")
            values[key] = value
    return values


def validate(reference: str) -> list[str]:
    errors: list[str] = []
    resolved = git("rev-parse", "--verify", f"{reference}^{{commit}}")
    head = git("rev-parse", "HEAD")
    if resolved != head:
        errors.append(f"release ref resolves to {resolved}, not HEAD {head}")

    status = git("status", "--porcelain=v1", "--untracked-files=all")
    if status:
        errors.append("release gate requires a clean working tree")

    metadata = properties(ROOT / "library.properties")
    version = metadata.get("version", "")
    if not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+", version):
        errors.append(f"invalid library version: {version!r}")

    changelog = (ROOT / "CHANGELOG.md").read_text(encoding="utf-8")
    if f"## {version} — experimental" not in changelog:
        errors.append(f"CHANGELOG has no experimental {version} release heading")

    required_exports = ("CHANGELOG.md", "LICENSE", "README.md", "examples", "src")
    # `git archive` is binary; feed it directly to tar for policy validation.
    listing = subprocess.run(
        ["git", "archive", "--format=tar", reference],
        cwd=ROOT,
        check=True,
        stdout=subprocess.PIPE,
    )
    names = set(
        subprocess.run(
            ["tar", "-tf", "-"],
            cwd=ROOT,
            check=True,
            input=listing.stdout,
            stdout=subprocess.PIPE,
        )
        .stdout.decode("utf-8")
        .splitlines()
    )
    for required in required_exports:
        if required not in names and not any(
            name.startswith(f"{required}/") for name in names
        ):
            errors.append(f"release archive is missing {required}")
    for excluded in ("build", "doc", "docs", "legacy", "mk", "research", "scripts", "site", "tests"):
        if excluded in names or any(name.startswith(f"{excluded}/") for name in names):
            errors.append(f"release archive unexpectedly contains {excluded}")

    if not errors:
        print(f"Release metadata and archive policy pass for {resolved} ({version}).")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--ref", default="HEAD")
    arguments = parser.parse_args()
    try:
        errors = validate(arguments.ref)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"release check failed: {error}", file=sys.stderr)
        return 1
    for error in errors:
        print(f"release check failed: {error}", file=sys.stderr)
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
