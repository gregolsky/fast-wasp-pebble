"""Advancing the emulator clock past the target triggers wakeup relaunch."""

import time


def test_fast_complete_wakeup(pebble):
    # Start a 12:12 fast.
    pebble.press("select")  # start fast
    time.sleep(0.5)

    # Advance the emulator clock by 12 h + 30 s to trigger the wakeup.
    import subprocess
    result = subprocess.run(
        ["pebble", "emu-time-format", "--emulator", pebble.platform,
         "--time-offset", str(12 * 3600 + 30)],
        capture_output=True, text=True
    )
    # The wakeup should relaunch the app and log fast-complete.
    assert pebble.wait_for_log("fast-complete", timeout=10), \
        "Expected fast-complete log after wakeup"
    # Eating window should now be open.
    assert pebble.wait_for_log("eat-active", timeout=5), \
        "Expected eat-active log after fast-complete wakeup"
