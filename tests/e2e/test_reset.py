"""Settings -> Reset wipes all data and returns to Program Picker."""

import time


def _open_settings(pebble):
    pebble.press("down")
    time.sleep(0.5)


def test_reset_clears_state(pebble):
    # Start a fast so there's state to wipe.
    pebble.press("select")
    time.sleep(0.3)
    pebble.press("select")   # stop fast
    time.sleep(0.3)

    _open_settings(pebble)

    # Navigate to "Reset Data" (third item: Vibration, Milestones, Reset Data).
    pebble.press("down")     # -> Milestones
    pebble.press("down")     # -> Reset Data
    pebble.press("select")   # arm confirm
    time.sleep(0.3)
    pebble.press("select")   # confirm reset
    time.sleep(0.5)

    assert pebble.wait_for_log("storage-reset"), "Expected storage-reset log after reset"

    # After reset the app exits; verify picker appears on fresh launch.
    pebble.reinstall()
    assert pebble.wait_for_log("state-picker"), \
        "Expected Program Picker on relaunch after reset"
