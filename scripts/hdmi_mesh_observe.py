#!/usr/bin/env python3
"""Print deterministic observations from the synthetic HDMI mesh fixture."""

import argparse
import sys


ROUTE_ID = "route:camera-to-wall"
SOURCE_ID = "input:camera-a"
DESTINATION_ID = "output:wall-center"


def printRoutes() -> None:
    print("fixture synthetic")
    print("route                   input           output              state   epoch")
    print(f"{ROUTE_ID:<23} {SOURCE_ID:<15} {DESTINATION_ID:<19} active  4")


def printRoute(source: str, destination: str) -> int:
    if source != SOURCE_ID or destination != DESTINATION_ID:
        print("synthetic fixture has no matching route", file=sys.stderr)
        return 2

    print("fixture synthetic")
    print(f"route {ROUTE_ID}")
    print(f"input {SOURCE_ID}")
    print(f"output {DESTINATION_ID}")
    print("state active")
    print("epoch 4")
    print("profile profile:wall-4k60")
    print("contract pinned")
    return 0


def printTrace(route: str) -> int:
    if route != ROUTE_ID:
        return unknownRoute(route)

    print("fixture synthetic")
    print(f"route {ROUTE_ID}")
    print("tick  event          state")
    print("10    route-request  reading-edid")
    print("11    edid-confirm   asserting-hpd")
    print("12    hpd-confirm    training")
    print("13    link-confirm   active")
    return 0


def printCrc(route: str) -> int:
    if route != ROUTE_ID:
        return unknownRoute(route)

    print("fixture synthetic")
    print(f"route {ROUTE_ID}")
    print("pattern synthetic-ramp")
    print("frames 120")
    print("mismatches 0")
    print("result model-only")
    return 0


def printLatency(route: str) -> int:
    if route != ROUTE_ID:
        return unknownRoute(route)

    print("fixture synthetic")
    print(f"route {ROUTE_ID}")
    print("samples 120")
    print("minimum-us 1950")
    print("median-us 2050")
    print("maximum-us 2200")
    print("result model-only")
    return 0


def unknownRoute(route: str) -> int:
    print(f"synthetic fixture has no route {route}", file=sys.stderr)
    return 2


def parseArguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Inspect a deterministic HDMI mesh model; never touches hardware."
    )
    command = parser.add_subparsers(dest="command", required=True)

    command.add_parser("routes")

    route = command.add_parser("route")
    route.add_argument("--source", required=True)
    route.add_argument("--destination", required=True)

    for name in ("trace", "crc", "latency"):
        routeCommand = command.add_parser(name)
        routeCommand.add_argument("--route", required=True)

    return parser.parse_args()


def main() -> int:
    arguments = parseArguments()

    if arguments.command == "routes":
        printRoutes()
        return 0
    if arguments.command == "route":
        return printRoute(arguments.source, arguments.destination)
    if arguments.command == "trace":
        return printTrace(arguments.route)
    if arguments.command == "crc":
        return printCrc(arguments.route)
    if arguments.command == "latency":
        return printLatency(arguments.route)

    return 2


if __name__ == "__main__":
    raise SystemExit(main())
