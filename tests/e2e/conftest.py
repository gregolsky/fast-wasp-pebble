"""
Pebble emulator fixture for Fast Pebble e2e tests.

Usage:
    pytest tests/e2e --platform=emery
    pytest tests/e2e --platform=diorite

Prerequisites:
    - Pebble SDK (Core Devices fork) installed and on PATH
    - `pebble build` has already run (produces build/*.pbw)
    - `libpebble2` pip package for emulator control

Each test gets a fresh emulator with wiped persistent storage.
"""

import subprocess
import time
import glob
import os
import pytest

# ── CLI option ────────────────────────────────────────────────────────────────

def pytest_addoption(parser):
    parser.addoption(
        "--platform", action="store", default="emery",
        choices=["emery", "diorite"],
        help="Pebble emulator platform (default: emery)"
    )


@pytest.fixture(scope="session")
def platform(request):
    return request.config.getoption("--platform")


# ── Emulator helper ──────────────────────────────────────────────────────────

class PebbleEmulator:
    """Thin wrapper around the pebble CLI for driving the emulator."""

    BUTTON_MAP = {
        "up":     "up",
        "select": "select",
        "down":   "down",
        "back":   "back",
    }

    def __init__(self, platform: str):
        self.platform = platform
        self._proc = None
        self._artifact_dir = os.path.join(
            os.path.dirname(__file__), "_artifacts"
        )
        os.makedirs(self._artifact_dir, exist_ok=True)

    # ── Lifecycle ─────────────────────────────────────────────────────────────

    def start(self):
        self._proc = subprocess.Popen(
            ["pebble", "emulator", "--platform", self.platform],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        time.sleep(3)  # give emulator time to boot

    def stop(self):
        if self._proc:
            self._proc.terminate()
            self._proc.wait(timeout=5)
            self._proc = None

    def wipe(self):
        subprocess.run(
            ["pebble", "wipe", "--emulator", self.platform],
            check=True, capture_output=True
        )

    def install(self, pbw_path: str):
        subprocess.run(
            ["pebble", "install", "--emulator", self.platform, pbw_path],
            check=True, capture_output=True
        )

    # ── Input ─────────────────────────────────────────────────────────────────

    def press(self, button: str, hold: bool = False):
        """Send a button press (short or long) to the emulator."""
        btn = self.BUTTON_MAP[button.lower()]
        if hold:
            subprocess.run(
                ["pebble", "emu-control", "--emulator", self.platform,
                 "--button", btn, "--long"],
                check=True, capture_output=True
            )
        else:
            subprocess.run(
                ["pebble", "emu-control", "--emulator", self.platform,
                 "--button", btn],
                check=True, capture_output=True
            )
        time.sleep(0.3)

    # ── Time control ──────────────────────────────────────────────────────────

    def set_time(self, epoch: int):
        """Advance the emulator's clock to the given Unix epoch seconds."""
        subprocess.run(
            ["pebble", "emu-time-format", "--emulator", self.platform,
             "--time", str(epoch)],
            check=True, capture_output=True
        )
        time.sleep(0.5)

    # ── Observation ──────────────────────────────────────────────────────────

    def screenshot(self, name: str) -> str:
        """Take a screenshot and save to _artifacts/; returns the path."""
        path = os.path.join(self._artifact_dir, f"{name}.png")
        subprocess.run(
            ["pebble", "screenshot", "--emulator", self.platform, path],
            check=True, capture_output=True
        )
        return path

    def wait_for_log(self, substring: str, timeout: float = 5.0) -> bool:
        """
        Poll the pebble log for a line containing `substring`.
        Returns True if found within `timeout` seconds, False otherwise.
        We rely on APP_LOG lines emitted by the app for key state transitions.
        """
        deadline = time.time() + timeout
        while time.time() < deadline:
            result = subprocess.run(
                ["pebble", "logs", "--emulator", self.platform, "--count", "50"],
                capture_output=True, text=True
            )
            if substring in result.stdout:
                return True
            time.sleep(0.25)
        return False


# ── Fixtures ─────────────────────────────────────────────────────────────────

def _find_pbw() -> str:
    root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    matches = glob.glob(os.path.join(root, "build", "*.pbw"))
    if not matches:
        raise FileNotFoundError(
            "No .pbw found in build/. Run `make build` first."
        )
    return matches[0]


@pytest.fixture(scope="function")
def pebble(platform):
    """Per-test emulator: start, wipe, install, yield, stop."""
    emu = PebbleEmulator(platform)
    emu.start()
    emu.wipe()
    emu.install(_find_pbw())
    time.sleep(1)  # let the app launch
    yield emu
    emu.stop()
