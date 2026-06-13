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


def test_no_spurious_eat_complete_on_entry(pebble):
    """Overtime must not trigger immediately when eating window opens."""
    _get_to_eating_window(pebble)
    time.sleep(2)  # let a couple of ticks fire
    assert pebble.wait_for_log("eat-active"), "Expected eating window to open"
    assert not any("eat-complete" in l for l in pebble._log_hist), \
        "eat-complete logged immediately after eating window opened — overtime at t=0"
