"""Edit-start-time: ±15 min nudge commits or refuses correctly."""

import time


def _start_fast(pebble):
    pebble.press("select")
    time.sleep(0.3)


def test_edit_start_minus_15_ok(pebble):
    _start_fast(pebble)
    pebble.press("up")       # open edit-start screen
    time.sleep(0.3)
    pebble.press("down")     # offset = -15 min
    pebble.press("select")   # commit
    time.sleep(0.5)
    assert pebble.wait_for_log("edit-start-ok"), "Expected commit to succeed"


def test_edit_start_future_refused(pebble):
    _start_fast(pebble)
    pebble.press("up")
    time.sleep(0.3)
    # Press Up many times to overshoot into the future.
    for _ in range(20):
        pebble.press("up")
    pebble.press("select")
    time.sleep(0.5)
    assert pebble.wait_for_log("edit-start-future"), \
        "Expected future-time commit to be refused"
