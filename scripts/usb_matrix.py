#!/usr/bin/env python3
"""Dry-run-first Linux USB/IP adapter for the phase-one matrix experiment.

LeaseLedger is a single-process prototype only. It is intentionally
incompatible with the authoritative fenced-epoch model in
research/usb_matrix. Do not deploy or combine the two until the documented
durable controller-process bridge exists.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import re
import subprocess
import sys
from dataclasses import dataclass
from typing import Protocol, Sequence


class CommandRunner(Protocol):
    def run(self, command: Sequence[str]) -> subprocess.CompletedProcess[str]:
        """Run one command without invoking a shell."""


@dataclass(frozen=True)
class Route:
    device_node: str
    bus_id: str
    host_node: str
    generation: int


class SubprocessRunner:
    def run(self, command: Sequence[str]) -> subprocess.CompletedProcess[str]:
        try:
            return subprocess.run(
                command,
                check=False,
                capture_output=True,
                text=True,
                timeout=10,
            )
        except subprocess.TimeoutExpired as error:
            raise RuntimeError("usbip command timed out") from error


class LeaseLedger:
    """Temporary phase-one ledger; not a production lease authority."""

    def __init__(self, path: pathlib.Path):
        self._path = path

    def routes(self) -> list[Route]:
        if not self._path.exists():
            return []

        document = json.loads(self._path.read_text(encoding="utf-8"))
        return [Route(**route) for route in document.get("routes", [])]

    def assign(self, requested: Route) -> list[Route]:
        routes = self.routes()
        for route in routes:
            if route.device_node == requested.device_node and route.bus_id == requested.bus_id:
                raise ValueError("device already has an exclusive lease")
            if route.host_node == requested.host_node:
                raise ValueError("host already has an exclusive lease")

        routes.append(requested)
        self._write(routes)
        return routes

    def release(self, host_node: str) -> Route:
        routes = self.routes()
        matches = [route for route in routes if route.host_node == host_node]
        if not matches:
            raise ValueError("host has no active lease")

        released = matches[0]
        self._write([route for route in routes if route != released])
        return released

    def next_generation(self) -> int:
        generations = [route.generation for route in self.routes()]
        return max(generations, default=0) + 1

    def _write(self, routes: list[Route]) -> None:
        self._path.parent.mkdir(parents=True, exist_ok=True)
        temporary = self._path.with_suffix(self._path.suffix + ".tmp")
        document = {
            "format": 1,
            "routes": [route.__dict__ for route in routes],
        }
        temporary.write_text(
            json.dumps(document, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        temporary.replace(self._path)


class UsbIpController:
    def __init__(self, runner: CommandRunner, ledger: LeaseLedger):
        self._runner = runner
        self._ledger = ledger

    def discover_local(self) -> str:
        return self._read(["usbip", "list", "--local"])

    def discover_remote(self, device_node: str) -> str:
        validate_node(device_node, "device node")
        return self._read(["usbip", "list", "--remote", device_node])

    def active_ports(self) -> str:
        return self._read(["usbip", "port"])

    def temporary_status(self) -> dict[str, object]:
        return {
            "authority": "temporary-phase-one-ledger",
            "routes": [route.__dict__ for route in self._ledger.routes()],
        }

    def plan_export(self, bus_id: str) -> list[list[str]]:
        validate_bus_id(bus_id)
        return [
            ["modprobe", "usbip-host"],
            ["usbip", "bind", "--busid", bus_id],
        ]

    def plan_assign(self, device_node: str, bus_id: str, host_node: str) -> tuple[Route, list[list[str]]]:
        validate_node(device_node, "device node")
        validate_bus_id(bus_id)
        validate_node(host_node, "host node")
        route = Route(
            device_node=device_node,
            bus_id=bus_id,
            host_node=host_node,
            generation=self._ledger.next_generation(),
        )
        commands = [
            ["modprobe", "vhci-hcd"],
            ["usbip", "attach", "--remote", device_node, "--busid", bus_id],
        ]
        return route, commands

    def execute_export(self, bus_id: str) -> None:
        self._run_all(self.plan_export(bus_id))

    def execute_assign(self, device_node: str, bus_id: str, host_node: str) -> Route:
        route, commands = self.plan_assign(device_node, bus_id, host_node)
        self._assert_available(route)
        self._run_all(commands)
        self._ledger.assign(route)
        return route

    def execute_release(self, host_node: str, port: int) -> Route:
        validate_node(host_node, "host node")
        if port < 0:
            raise ValueError("port must be zero or greater")
        routes = self._ledger.routes()
        matches = [route for route in routes if route.host_node == host_node]
        if not matches:
            raise ValueError("host has no active lease")

        self._run_all([["usbip", "detach", "--port", str(port)]])
        return self._ledger.release(host_node)

    def _assert_available(self, requested: Route) -> None:
        for route in self._ledger.routes():
            same_device = (
                route.device_node == requested.device_node
                and route.bus_id == requested.bus_id
            )
            if same_device:
                raise ValueError("device already has an exclusive lease")
            if route.host_node == requested.host_node:
                raise ValueError("host already has an exclusive lease")

    def _read(self, command: list[str]) -> str:
        completed = self._runner.run(command)
        if completed.returncode != 0:
            raise RuntimeError(completed.stderr.strip() or "usbip command failed")
        return completed.stdout

    def _run_all(self, commands: list[list[str]]) -> None:
        for command in commands:
            completed = self._runner.run(command)
            if completed.returncode != 0:
                raise RuntimeError(completed.stderr.strip() or "usbip command failed")


def print_plan(commands: list[list[str]]) -> None:
    for command in commands:
        print(" ".join(command))


def validate_node(value: str, label: str) -> None:
    if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9_.-]{0,252}", value):
        raise ValueError(f"invalid {label}")


def validate_bus_id(value: str) -> None:
    if not re.fullmatch(r"[0-9]+-[0-9]+(?:\.[0-9]+)*", value):
        raise ValueError("invalid USB bus ID")


def parse_arguments(arguments: Sequence[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Discover and route Linux USB/IP devices with exclusive leases.",
    )
    parser.add_argument(
        "--ledger",
        type=pathlib.Path,
        default=pathlib.Path("build/usb-matrix/leases.json"),
    )
    subparsers = parser.add_subparsers(dest="action", required=True)

    subparsers.add_parser("discover-local")
    remote = subparsers.add_parser("discover-remote")
    remote.add_argument("device_node")
    subparsers.add_parser("ports")
    subparsers.add_parser("status")

    export = subparsers.add_parser("export")
    export.add_argument("bus_id")
    export.add_argument("--execute", action="store_true")

    assign = subparsers.add_parser("assign")
    assign.add_argument("device_node")
    assign.add_argument("bus_id")
    assign.add_argument("host_node")
    assign.add_argument("--execute", action="store_true")

    release = subparsers.add_parser("release")
    release.add_argument("host_node")
    release.add_argument("--port", type=int, required=True)
    release.add_argument("--execute", action="store_true")
    return parser.parse_args(arguments)


def main(arguments: Sequence[str] | None = None) -> int:
    options = parse_arguments(arguments or sys.argv[1:])
    controller = UsbIpController(SubprocessRunner(), LeaseLedger(options.ledger))

    try:
        if options.action == "discover-local":
            print(controller.discover_local(), end="")
        elif options.action == "discover-remote":
            print(controller.discover_remote(options.device_node), end="")
        elif options.action == "ports":
            print(controller.active_ports(), end="")
        elif options.action == "status":
            print(json.dumps(controller.temporary_status(), indent=2, sort_keys=True))
        elif options.action == "export":
            commands = controller.plan_export(options.bus_id)
            if options.execute:
                controller.execute_export(options.bus_id)
            else:
                print_plan(commands)
        elif options.action == "assign":
            route, commands = controller.plan_assign(
                options.device_node,
                options.bus_id,
                options.host_node,
            )
            if options.execute:
                route = controller.execute_assign(
                    options.device_node,
                    options.bus_id,
                    options.host_node,
                )
                print(json.dumps(route.__dict__, sort_keys=True))
            else:
                print_plan(commands)
                print(f"# exclusive lease generation {route.generation}")
        elif options.action == "release":
            validate_node(options.host_node, "host node")
            if options.port < 0:
                raise ValueError("port must be zero or greater")
            command = [["usbip", "detach", "--port", str(options.port)]]
            if options.execute:
                route = controller.execute_release(options.host_node, options.port)
                print(json.dumps(route.__dict__, sort_keys=True))
            else:
                print_plan(command)
        return 0
    except (OSError, RuntimeError, ValueError, json.JSONDecodeError) as error:
        print(f"usb-matrix: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
