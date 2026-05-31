"""Settings -> Reset wipes all data and returns to Program Picker."""

import time


def _open_settings(pebble):
    # From Ready: long-press Select -> Program Picker, then long-press Select -> Settings.
    pebble.press("select", hold=True)
    time.sleep(0.5)
    pebble.press("select", hold=True)
    time.sleep(0.5)


def test_reset_clears_state(pebble):
    # Start a fast so there's state to wipe.
    pebble.press("select")
    time.sleep(0.3)
    pebble.press("select")   # stop fast
    time.sleep(0.3)

    _open_settings(pebble)

    # Navigate to "Reset Data" (second item) and confirm twice.
    pebble.press("down")
    pebble.press("select")   # arm confirm
    time.sleep(0.3)
    pebble.press("select")   # confirm reset
    time.sleep(0.5)

    assert pebble.wait_for_log("storage-reset"), "Expected storage-reset log after reset"

    # App exits after stack pop; reinstall to verify picker appears on relaunch.
    pebble.reinstall()
    assert pebble.wait_for_log("state-picker"), \
        "Expected Program Picker on relaunch after reset"
