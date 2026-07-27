#!/usr/bin/env python3
"""Deterministic checks for the synthetic HDMI observation CLI."""

import pathlib
import subprocess
import sys


SCRIPT = pathlib.Path(__file__).with_name("hdmi_mesh_observe.py")
ROUTE = "route:camera-to-wall"


def run(*arguments: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [sys.executable, str(SCRIPT), *arguments],
        check=False,
        capture_output=True,
        text=True,
    )


def requireSuccess(result: subprocess.CompletedProcess[str], evidence: str) -> None:
    if result.returncode != 0:
        raise AssertionError(f"{evidence}: {result.stderr}")
    if not result.stdout.startswith("fixture synthetic\n"):
        raise AssertionError(f"{evidence}: fixture identity is missing")


def main() -> int:
    routes = run("routes")
    requireSuccess(routes, "routes")
    if ROUTE not in routes.stdout or "active  4" not in routes.stdout:
        raise AssertionError("routes: stable route snapshot is missing")

    route = run(
        "route",
        "--source",
        "input:camera-a",
        "--destination",
        "output:wall-center",
    )
    requireSuccess(route, "route")
    if "contract pinned\n" not in route.stdout:
        raise AssertionError("route: profile policy is missing")

    trace = run("trace", "--route", ROUTE)
    requireSuccess(trace, "trace")
    if "10    route-request  reading-edid" not in trace.stdout:
        raise AssertionError("trace: ordered request is missing")
    if trace.stdout != run("trace", "--route", ROUTE).stdout:
        raise AssertionError("trace: repeated output differs")

    crc = run("crc", "--route", ROUTE)
    requireSuccess(crc, "crc")
    if "result model-only\n" not in crc.stdout:
        raise AssertionError("crc: model boundary is missing")

    latency = run("latency", "--route", ROUTE)
    requireSuccess(latency, "latency")
    if "minimum-us 1950\n" not in latency.stdout:
        raise AssertionError("latency: bounded fixture value is missing")

    unknown = run("trace", "--route", "route:unknown")
    if unknown.returncode == 0 or unknown.stdout:
        raise AssertionError("unknown route: expected a clean failure")
    if unknown.stderr != "synthetic fixture has no route route:unknown\n":
        raise AssertionError("unknown route: unstable diagnostic")

    print("HDMI mesh observation CLI tests passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
