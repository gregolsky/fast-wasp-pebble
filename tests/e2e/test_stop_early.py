"""Stopping early records the fast in stats."""

import time


def test_stop_early_appears_in_stats(pebble):
    pebble.press("select")   # start fast
    time.sleep(0.5)
    pebble.press("select")   # stop fast early
    time.sleep(0.5)

    pebble.press("up")   # open Stats
    time.sleep(0.5)

    assert pebble.wait_for_log("stats-total-fasts:1"), \
        "Expected 1 fast in stats after stopping early"
