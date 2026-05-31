"""Long-press Select during eating window opens picker (END action was removed)."""

import time


def _get_to_eating_window(pebble):
    pebble.press("select")   # start fast
    time.sleep(0.3)
    pebble.press("select")   # stop fast -> eating window opens
    time.sleep(0.5)


def test_long_press_from_eating_opens_picker(pebble):
    _get_to_eating_window(pebble)
    assert pebble.wait_for_log("eat-active"), "Expected eating window to open"

    pebble.press("select", hold=True)   # opens picker
    time.sleep(0.5)
    pebble.press("back")                # dismiss picker
    assert pebble.wait_for_log("eat-active", timeout=3), \
        "Expected still in eating state after dismissing picker"
