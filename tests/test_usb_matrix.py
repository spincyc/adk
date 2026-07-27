import pathlib
import subprocess
import tempfile
import unittest

from scripts.usb_matrix import LeaseLedger, Route, UsbIpController


class FakeRunner:
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


class UsbIpControllerTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        path = pathlib.Path(self.temporary.name) / "leases.json"
        self.runner = FakeRunner()
        self.ledger = LeaseLedger(path)
        self.controller = UsbIpController(self.runner, self.ledger)

    def tearDown(self):
        self.temporary.cleanup()

    def test_remote_discovery_is_read_only(self):
        self.runner.answer("Exportable USB devices\n")

        output = self.controller.discover_remote("device-a")

        self.assertEqual(output, "Exportable USB devices\n")
        self.assertEqual(
            self.runner.commands,
            [["usbip", "list", "--remote", "device-a"]],
        )
        self.assertEqual(self.ledger.routes(), [])

    def test_dry_run_plan_has_no_side_effect(self):
        route, commands = self.controller.plan_assign(
            "device-a",
            "2-1",
            "host-a",
        )

        self.assertEqual(route.generation, 1)
        self.assertEqual(
            commands[-1],
            [
                "usbip",
                "attach",
                "--remote",
                "device-a",
                "--busid",
                "2-1",
            ],
        )
        self.assertEqual(self.runner.commands, [])
        self.assertEqual(self.ledger.routes(), [])

    def test_status_names_temporary_authority(self):
        route = Route("device-a", "2-1", "host-a", 3)
        self.ledger.assign(route)

        status = self.controller.temporary_status()

        self.assertEqual(status["authority"], "temporary-phase-one-ledger")
        self.assertEqual(status["routes"], [route.__dict__])
        self.assertEqual(self.runner.commands, [])

    def test_successful_assignment_records_exclusive_lease(self):
        self.runner.answer()
        self.runner.answer()

        route = self.controller.execute_assign("device-a", "2-1", "host-a")

        self.assertEqual(route.generation, 1)
        self.assertEqual(self.ledger.routes(), [route])

    def test_device_cannot_be_assigned_twice(self):
        self.ledger.assign(Route("device-a", "2-1", "host-a", 4))

        with self.assertRaisesRegex(ValueError, "device already"):
            self.controller.execute_assign("device-a", "2-1", "host-b")

        self.assertEqual(self.runner.commands, [])

    def test_host_cannot_receive_two_devices(self):
        self.ledger.assign(Route("device-a", "2-1", "host-a", 4))

        with self.assertRaisesRegex(ValueError, "host already"):
            self.controller.execute_assign("device-b", "3-2", "host-a")

        self.assertEqual(self.runner.commands, [])

    def test_failed_attach_does_not_create_lease(self):
        self.runner.answer()
        self.runner.answer(stderr="attach failed", returncode=1)

        with self.assertRaisesRegex(RuntimeError, "attach failed"):
            self.controller.execute_assign("device-a", "2-1", "host-a")

        self.assertEqual(self.ledger.routes(), [])

    def test_release_detaches_before_removing_lease(self):
        active = Route("device-a", "2-1", "host-a", 7)
        self.ledger.assign(active)
        self.runner.answer()

        released = self.controller.execute_release("host-a", 3)

        self.assertEqual(released, active)
        self.assertEqual(self.ledger.routes(), [])
        self.assertEqual(
            self.runner.commands,
            [["usbip", "detach", "--port", "3"]],
        )

    def test_failed_detach_preserves_lease(self):
        active = Route("device-a", "2-1", "host-a", 7)
        self.ledger.assign(active)
        self.runner.answer(stderr="busy", returncode=1)

        with self.assertRaisesRegex(RuntimeError, "busy"):
            self.controller.execute_release("host-a", 3)

        self.assertEqual(self.ledger.routes(), [active])


if __name__ == "__main__":
    unittest.main()
