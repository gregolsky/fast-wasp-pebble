"""Short-press Select during eating window starts a new fast."""

import time


def _get_to_eating_window(pebble):
    pebble.press("select")
    time.sleep(0.3)
    pebble.press("select")
    time.sleep(0.5)


def test_restart_fast_from_eating_window(pebble):
    _get_to_eating_window(pebble)
    assert pebble.wait_for_log("eat-active")

    pebble.press("select")   # Start Fasting Now
    time.sleep(0.5)
    assert pebble.wait_for_log("fast-active"), \
        "Expected new fast to start after Select during eating window"
