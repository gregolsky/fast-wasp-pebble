"""Cold start: first launch shows Program Picker; selecting a program goes to Ready."""

import time


def test_first_launch_shows_picker(pebble):
    # No program stored -> picker should be the first screen.
    # Scroll down twice and select 16:8 (index 2).
    pebble.press("down")
    pebble.press("down")
    pebble.press("select")
    time.sleep(0.5)
    # App logs the selected program on transition.
    assert pebble.wait_for_log("program-selected"), "Expected program-selected log"
