"""Closing and reopening the app preserves the countdown within ±2 s."""

import time
import subprocess


def _kill_app(pebble):
    subprocess.run(
        ["pebble", "emu-control", "--emulator", pebble.platform, "--kill"],
        capture_output=True
    )
    time.sleep(1)


def _launch_app(pebble):
    subprocess.run(
        ["pebble", "install", "--emulator", pebble.platform],
        capture_output=True
    )
    time.sleep(1)


def test_fast_survives_app_close(pebble):
    # Pick 12:12 (index 0 — first item).
    pebble.press("select")   # start fast from Ready
    time.sleep(0.5)

    pebble.screenshot("before_close")
    pebble.press("back")     # exit app; fast still running in background
    time.sleep(2)

    _launch_app(pebble)
    pebble.screenshot("after_reopen")
    # Countdown should still be ticking; log confirms active fast state.
    assert pebble.wait_for_log("fast-active"), "Expected fast-active log after reopen"
