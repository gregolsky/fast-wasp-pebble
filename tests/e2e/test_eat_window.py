"""Long-press Select during eating window ends it without starting a new fast."""

import time


def _get_to_eating_window(pebble):
    pebble.press("select")   # start fast
    time.sleep(0.3)
    pebble.press("select")   # stop fast -> eating window opens
    time.sleep(0.5)


def test_end_eating_window_returns_to_ready(pebble):
    _get_to_eating_window(pebble)
    assert pebble.wait_for_log("eat-active"), "Expected eating window to open"

    pebble.press("select", hold=True)  # End Eating Window
    time.sleep(0.5)
    assert pebble.wait_for_log("state-ready"), "Expected ready state after ending eat window"
