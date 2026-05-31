"""Cold start: first launch shows Program Picker; selecting a program goes to Ready."""

import time


def test_first_launch_shows_picker(pebble_raw):
    # No program stored -> picker is the first screen.
    # Scroll down twice to reach 16:8 (index 2) and select it.
    pebble_raw.press("down")
    pebble_raw.press("down")
    pebble_raw.press("select")
    time.sleep(0.5)
    assert pebble_raw.wait_for_log("program-selected"), "Expected program-selected log"
