"""Settings → Reset wipes all data and returns to Program Picker."""

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

    # Navigate to "Reset Data" (row 1, second item).
    pebble.press("down")
    pebble.press("select")   # first press: arm confirm
    time.sleep(0.3)
    pebble.press("select")   # second press: confirm reset
    time.sleep(0.5)

    # After reset the app should return to picker (no saved program).
    assert pebble.wait_for_log("state-picker"), \
        "Expected Program Picker after reset"
    # Wakeup id should be -1 (no wakeup) logged.
    assert pebble.wait_for_log("storage-reset"), \
        "Expected storage-reset log after reset"
