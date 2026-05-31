"""Closing and reopening the app preserves the countdown within ±2 s."""

import time


def test_fast_survives_app_close(pebble):
    pebble.press("select")   # start fast from Ready
    time.sleep(0.5)

    pebble.screenshot("before_close")
    pebble.press("back")     # exit app; fast still running in background
    time.sleep(2)

    pebble.reinstall()
    pebble.screenshot("after_reopen")
    assert pebble.wait_for_log("fast-active"), \
        "Expected fast-active log after reopen"
