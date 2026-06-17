"""E2E fixture for Fast Pebble. Run inside Docker: see run-e2e.sh."""
import subprocess, time, threading, queue, os, glob, shutil, pytest

PLATFORM     = "basalt"
_pebble_bin  = shutil.which("pebble")
SDK_PY       = (os.path.join(os.path.dirname(_pebble_bin), "python3")
                if _pebble_bin else "python3")
DRIVER       = os.path.join(os.path.dirname(__file__), "driver.py")
ARTIFACT_DIR = os.path.join(os.path.dirname(__file__), "_artifacts")
ROOT_DIR     = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


def _find_pbw():
    m = glob.glob(os.path.join(ROOT_DIR, "build", "*.pbw"))
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

    @staticmethod
    def _clear_emulator_state():
        """Remove the pebble-tool emulator state file so the next install starts fresh.
        Without this, zombie PIDs left after SIGKILL make pebble-tool think old processes
        are still running and try to connect to their (dead) ports."""
        try:
            os.remove("/tmp/pb-emulator.json")
        except FileNotFoundError:
            pass

    def start(self):
        self._clear_emulator_state()
        pbw = _find_pbw()
        for attempt in range(6):
            result = subprocess.run(
                ["pebble", "install", "--emulator", self.platform, pbw],
                capture_output=True, text=True
            )
            if result.returncode == 0:
                break
            print(f"[install attempt {attempt} rc={result.returncode}]: {(result.stderr or result.stdout)[:300]}", flush=True)
            time.sleep(1)
        else:
            raise RuntimeError("pebble install failed after 6 attempts")
        subprocess.run(["pebble", "wipe"], capture_output=True)
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
        subprocess.run(["pebble", "kill", "--force"], capture_output=True)
        self._clear_emulator_state()
        time.sleep(5)  # wait for QEMU to release ports before next test

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
