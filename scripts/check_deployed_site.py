#!/usr/bin/env python3

"""Check the public artifacts required for the newest published lesson."""

from __future__ import annotations

import argparse
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Callable
from urllib.error import HTTPError, URLError
from urllib.parse import urljoin, urlsplit
from urllib.request import Request, urlopen

if __package__:
    from scripts.publication import PublicationConfigError
    from scripts.publication import PublishedLesson
    from scripts.publication import resolve_latest_publication
else:
    from publication import PublicationConfigError
    from publication import PublishedLesson
    from publication import resolve_latest_publication


DEFAULT_TIMEOUT_SECONDS = 10.0
DEFAULT_RETRIES = 2
DEFAULT_RETRY_DELAY_SECONDS = 0.25
MAX_RESPONSE_BYTES = 2 * 1024 * 1024
MAX_PDF_RESPONSE_BYTES = 50 * 1024 * 1024


@dataclass(frozen=True)
class Check:
    path: str
    description: str
    prefix: bytes | None = None
    markers: tuple[bytes, ...] = ()
    max_response_bytes: int = MAX_RESPONSE_BYTES


REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
BUILD_CONFIG = REPOSITORY_ROOT / "mk/config.mk"


def configured_publication() -> PublishedLesson:
    """Resolve the repository's current publication boundary."""
    try:
        source = BUILD_CONFIG.read_text(encoding="utf-8")
    except (OSError, UnicodeError) as exception:
        raise PublicationConfigError(
            f"cannot read lesson configuration: {exception}"
        ) from None
    return resolve_latest_publication(source)


def deployment_checks(publication: PublishedLesson) -> tuple[Check, ...]:
    """Build stable artifact checks for one resolved publication."""
    lesson = publication.number.encode()
    return (
        Check(
            "",
            "landing page",
            markers=(b"Lessons 001\xe2\x80\x93" + lesson, b"Planned course"),
        ),
        Check(
            publication.lesson_page,
            f"Lesson {publication.number} page",
            markers=(
                b"Lesson " + lesson,
                b"Status:",
                b"Energy class:",
                b"Safety boundary:",
            ),
        ),
        Check(
            publication.pdf_path,
            f"Lesson {publication.number} PDF",
            b"%PDF-",
            max_response_bytes=MAX_PDF_RESPONSE_BYTES,
        ),
        Check(
            publication.sketch_path,
            f"Lesson {publication.number} Arduino example",
            markers=(b"#include <Adk.h>",),
        ),
    )


def canonical_example(publication: PublishedLesson) -> Path:
    """Return the audited checkout path for a publication's sketch."""
    return (
        REPOSITORY_ROOT
        / "examples"
        / publication.example
        / f"{publication.example}.ino"
    )


def normalize_base_url(raw_base_url: str) -> str:
    """Return a directory-like HTTP(S) or file URL suitable for urljoin."""
    parsed = urlsplit(raw_base_url)
    if parsed.scheme not in {"https", "file"}:
        raise ValueError("base URL must use https or file")
    if parsed.scheme == "https" and not parsed.netloc:
        raise ValueError("HTTPS base URL must include a host")
    if parsed.username is not None or parsed.password is not None:
        raise ValueError("base URL must not contain credentials")
    if parsed.query or parsed.fragment:
        raise ValueError("base URL must not include a query or fragment")
    return raw_base_url.rstrip("/") + "/"


def read_url(url: str, timeout: float, max_response_bytes: int) -> bytes:
    """Fetch one URL, bounding both the request time and response size."""
    if urlsplit(url).scheme == "file" and urlsplit(url).path.endswith("/"):
        url = urljoin(url, "index.html")
    request = Request(url, headers={"User-Agent": "adk-deployment-check/1"})
    with urlopen(request, timeout=timeout) as response:
        requested = urlsplit(url)
        effective = urlsplit(response.geturl())
        if requested.scheme != effective.scheme or requested.netloc != effective.netloc:
            raise OSError("response redirected outside the deployment origin")
        if requested.path.endswith("/"):
            path_matches = effective.path.startswith(requested.path)
        else:
            path_matches = effective.path == requested.path
        if not path_matches:
            raise OSError("response redirected outside the requested artifact path")
        status = getattr(response, "status", None)
        if status is not None and status != 200:
            raise OSError(f"unexpected HTTP status {status}")
        data = response.read(max_response_bytes + 1)
    if len(data) > max_response_bytes:
        raise OSError(f"response exceeds {max_response_bytes} bytes")
    return data


def validate_response(
    check: Check,
    data: bytes,
    canonical_sketch: Path,
) -> str | None:
    if not data:
        return "response is empty"
    if check.prefix is not None and not data.startswith(check.prefix):
        return f"response does not begin with {check.prefix!r}"
    for marker in check.markers:
        if marker not in data:
            return f"response is missing {marker!r}"
    if check.path.endswith(".ino"):
        try:
            data.decode("utf-8")
        except UnicodeDecodeError:
            return "Arduino example is not valid UTF-8"
        try:
            canonical = canonical_sketch.read_bytes()
        except OSError as exception:
            return f"cannot read canonical Arduino example: {exception}"
        if data != canonical:
            return "Arduino example differs from the audited checkout"
    return None


def check_deployment(
    base_url: str,
    *,
    retries: int = DEFAULT_RETRIES,
    timeout: float = DEFAULT_TIMEOUT_SECONDS,
    retry_delay: float = DEFAULT_RETRY_DELAY_SECONDS,
    fetch: Callable[[str, float, int], bytes] = read_url,
    sleep: Callable[[float], None] = time.sleep,
    publication: PublishedLesson | None = None,
) -> list[str]:
    """Return deterministic, human-readable failures for the deployed site."""
    if retries < 0:
        raise ValueError("retries must be nonnegative")
    if timeout <= 0:
        raise ValueError("timeout must be positive")
    if retry_delay < 0:
        raise ValueError("retry delay must be nonnegative")

    normalized = normalize_base_url(base_url)
    if publication is None:
        publication = configured_publication()
    checks = deployment_checks(publication)
    canonical_sketch = canonical_example(publication)
    errors: list[str] = []
    for check in checks:
        url = urljoin(normalized, check.path)
        failure = ""
        for attempt in range(retries + 1):
            try:
                data = fetch(url, timeout, check.max_response_bytes)
                validation_error = validate_response(check, data, canonical_sketch)
                if validation_error is None:
                    failure = ""
                    break
                failure = validation_error
            except (HTTPError, URLError, OSError, TimeoutError) as exception:
                failure = str(exception)
            if attempt < retries:
                sleep(retry_delay)
        if failure:
            errors.append(
                f"{check.description} ({url}) failed after "
                f"{retries + 1} attempt(s): {failure}"
            )
    return errors


def parse_arguments(arguments: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "base_url",
        help="deployed site base URL, including any project path",
    )
    parser.add_argument(
        "--retries",
        type=int,
        default=DEFAULT_RETRIES,
        help=f"retries after the first attempt (default: {DEFAULT_RETRIES})",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=DEFAULT_TIMEOUT_SECONDS,
        help=f"per-request timeout in seconds (default: {DEFAULT_TIMEOUT_SECONDS:g})",
    )
    parser.add_argument(
        "--retry-delay",
        type=float,
        default=DEFAULT_RETRY_DELAY_SECONDS,
        help=(
            "delay between attempts in seconds "
            f"(default: {DEFAULT_RETRY_DELAY_SECONDS:g})"
        ),
    )
    return parser.parse_args(arguments)


def main(arguments: list[str]) -> int:
    options = parse_arguments(arguments)
    try:
        errors = check_deployment(
            options.base_url,
            retries=options.retries,
            timeout=options.timeout,
            retry_delay=options.retry_delay,
        )
    except ValueError as exception:
        print(f"deployment check: {exception}", file=sys.stderr)
        return 2

    if errors:
        for error in errors:
            print(f"deployment check: {error}", file=sys.stderr)
        return 1
    print(f"deployment check passed: {normalize_base_url(options.base_url)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
