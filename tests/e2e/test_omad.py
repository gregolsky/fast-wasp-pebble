"""OMAD: logging a meal closes the previous interval and opens a new one."""

import time


def _switch_to_omad(pebble):
    # From FAST/READY screen: long-press SELECT -> picker, navigate to OMAD (index 5).
    pebble.press("select", hold=True)
    time.sleep(0.5)
    pebble.select_program(5)
    time.sleep(0.5)


def test_omad_log_meal(pebble):
    _switch_to_omad(pebble)
    pebble.press("select")   # log first meal → enters OMAD active
    time.sleep(0.3)
    assert pebble.wait_for_log("omad-active"), "Expected OMAD state after first meal"
    time.sleep(0.3)
    pebble.press("select")   # log second meal (closes first interval)
    time.sleep(0.5)

    pebble.press("up")   # open Stats
    time.sleep(0.5)
    assert pebble.wait_for_log("stats-total-fasts:1"), \
        "Expected 1 OMAD interval in stats"
