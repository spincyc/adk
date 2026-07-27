import json
import pathlib
import subprocess
import tempfile
import unittest

from scripts.usb_matrix import LeaseLedger, Route, UsbIpController


class RecordingRunner:
    def __init__(self):
        self.commands = []
        self.results = []

    def answer(self, stdout="", stderr="", returncode=0):
        self.results.append(
            subprocess.CompletedProcess([], returncode, stdout, stderr),
        )

    def run(self, command):
        self.commands.append(list(command))
        if not self.results:
            raise AssertionError("unexpected command")
        return self.results.pop(0)


class UsbMatrixSecurityTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.ledgerPath = pathlib.Path(self.temporary.name) / "leases.json"
        self.runner = RecordingRunner()
        self.ledger = LeaseLedger(self.ledgerPath)
        self.controller = UsbIpController(self.runner, self.ledger)

    def tearDown(self):
        self.temporary.cleanup()

    def writeLedger(self, document):
        self.ledgerPath.write_text(
            json.dumps(document),
            encoding="utf-8",
        )

    def test_hostile_identifiers_are_rejected_before_commands(self):
        deviceNode = "node; touch /tmp/adk-usb-matrix-injection"
        busId = "2-1; reboot"

        with self.assertRaisesRegex(ValueError, "invalid device node"):
            self.controller.execute_assign(deviceNode, busId, "host-a")

        self.assertEqual(self.runner.commands, [])

    def test_discovery_failure_never_changes_the_ledger(self):
        self.runner.answer(stderr="unreachable", returncode=1)

        with self.assertRaisesRegex(RuntimeError, "unreachable"):
            self.controller.discover_remote("device-a")

        self.assertFalse(self.ledgerPath.exists())

    def test_first_assignment_command_failure_is_fail_closed(self):
        self.runner.answer(stderr="module unavailable", returncode=1)

        with self.assertRaisesRegex(RuntimeError, "module unavailable"):
            self.controller.execute_assign("device-a", "2-1", "host-a")

        self.assertEqual(self.runner.commands, [["modprobe", "vhci-hcd"]])
        self.assertFalse(self.ledgerPath.exists())

    def test_malformed_json_is_rejected_without_replacement(self):
        malformed = "{not-json"
        self.ledgerPath.write_text(malformed, encoding="utf-8")

        with self.assertRaises(json.JSONDecodeError):
            self.controller.execute_assign("device-a", "2-1", "host-a")

        self.assertEqual(self.ledgerPath.read_text(encoding="utf-8"), malformed)
        self.assertEqual(self.runner.commands, [])

    def test_missing_route_fields_are_rejected_before_commands(self):
        document = {
            "format": 1,
            "routes": [{"device_node": "device-a"}],
        }
        self.writeLedger(document)

        with self.assertRaises((TypeError, ValueError)):
            self.controller.execute_assign("device-b", "3-1", "host-b")

        self.assertEqual(self.runner.commands, [])
        self.assertEqual(json.loads(self.ledgerPath.read_text()), document)

    def test_failed_detach_is_idempotent_with_respect_to_persistence(self):
        route = Route("device-a", "2-1", "host-a", 9)
        self.ledger.assign(route)
        before = self.ledgerPath.read_bytes()
        self.runner.answer(stderr="detach timed out", returncode=1)

        with self.assertRaisesRegex(RuntimeError, "detach timed out"):
            self.controller.execute_release("host-a", 4)

        self.assertEqual(self.ledgerPath.read_bytes(), before)
        self.assertEqual(self.ledger.routes(), [route])

    def test_repeated_release_does_not_issue_a_second_detach(self):
        route = Route("device-a", "2-1", "host-a", 9)
        self.ledger.assign(route)
        self.runner.answer()

        self.controller.execute_release("host-a", 4)

        with self.assertRaisesRegex(ValueError, "no active lease"):
            self.controller.execute_release("host-a", 4)

        self.assertEqual(
            self.runner.commands,
            [["usbip", "detach", "--port", "4"]],
        )


if __name__ == "__main__":
    unittest.main()
