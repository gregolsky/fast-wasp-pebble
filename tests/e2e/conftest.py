"""E2E fixture for Fast Pebble. Run inside Docker: see run-e2e.sh."""
import subprocess, time, threading, queue, os, glob, pytest

PLATFORM     = "basalt"
SDK_PY       = os.path.expanduser("~/.local/share/uv/tools/pebble-tool/bin/python3")
DRIVER       = os.path.join(os.path.dirname(__file__), "driver.py")
ARTIFACT_DIR = os.path.join(os.path.dirname(__file__), "_artifacts")


def _find_pbw():
    root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    m = glob.glob(os.path.join(root, "build", "*.pbw"))
    if not m:
        raise FileNotFoundError("No .pbw in build/ -- run pebble build first.")
    return m[0]


def pytest_addoption(parser):
    parser.addoption("--platform", default=PLATFORM, choices=["basalt"],
                     help="Pebble platform (basalt only; diorite/emery QEMU broken)")


@pytest.fixture(scope="session")
def platform(request):
    return request.config.getoption("--platform")


class PebbleEmulator:
    def __init__(self, plat):
        self.platform = plat
        self._log_proc = None
        self._log_q    = queue.Queue()
        self._log_hist = []
        os.makedirs(ARTIFACT_DIR, exist_ok=True)

    # ── Lifecycle ─────────────────────────────────────────────────────────────

    def start(self):
        subprocess.run(["pebble", "wipe", "--emulator", self.platform],
                       capture_output=True)
        subprocess.run(["pebble", "install", "--emulator", self.platform,
                        _find_pbw()], check=True, capture_output=True)
        time.sleep(3)
        self._start_logs()

    def stop(self):
        if self._log_proc:
            self._log_proc.terminate()
            try:
                self._log_proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self._log_proc.kill()
            self._log_proc = None
        subprocess.run(["pebble", "kill"], capture_output=True)

    def reinstall(self):
        """Reinstall (relaunch) the app without wiping storage."""
        subprocess.run(["pebble", "install", "--emulator", self.platform,
                        _find_pbw()], capture_output=True)
        time.sleep(2)

    # ── Input ─────────────────────────────────────────────────────────────────

    def press(self, button, hold=False):
        cmd = [SDK_PY, DRIVER, self.platform, button.lower()]
        if hold:
            cmd += ["--hold-ms", "800"]
        subprocess.run(cmd, check=True)
        time.sleep(0.35)

    def select_program(self, idx):
        """Navigate from picker position 0 down to idx, then press select."""
        for _ in range(idx):
            self.press("down")
        self.press("select")

    # ── Observation ──────────────────────────────────────────────────────────

    def _start_logs(self):
        self._log_proc = subprocess.Popen(
            ["pebble", "logs", "--emulator", self.platform],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, bufsize=1
        )
        self._log_hist.clear()

        def _reader():
            for line in self._log_proc.stdout:
                self._log_hist.append(line)
                self._log_q.put(line)

        threading.Thread(target=_reader, daemon=True).start()

    def wait_for_log(self, s, timeout=10.0):
        if any(s in l for l in self._log_hist):
            return True
        deadline = time.time() + timeout
        while time.time() < deadline:
            try:
                if s in self._log_q.get(timeout=0.5):
                    return True
            except queue.Empty:
                pass
        return False

    def screenshot(self, name):
        path = os.path.join(ARTIFACT_DIR, f"{name}.png")
        subprocess.run(["pebble", "screenshot", "--emulator", self.platform, path],
                       capture_output=True)
        return path


@pytest.fixture(scope="function")
def pebble_raw(platform):
    """Fresh emulator with picker visible -- no program pre-selected."""
    emu = PebbleEmulator(platform)
    emu.start()
    yield emu
    emu.stop()


@pytest.fixture(scope="function")
def pebble(platform):
    """Fresh emulator with 12:12 selected, app in READY state."""
    emu = PebbleEmulator(platform)
    emu.start()
    emu.select_program(0)
    assert emu.wait_for_log("state-ready"), \
        "Fixture setup: expected state-ready after selecting 12:12"
    yield emu
    emu.stop()
