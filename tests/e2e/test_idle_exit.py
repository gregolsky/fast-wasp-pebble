"""With no button pressed for 30 s, the app goes back to the watchface."""

import time


def test_idle_exits_after_30s_and_presses_postpone_it(pebble):
    pebble.clear_logs()
    pebble.press("select")  # start a fast; restarts the countdown
    time.sleep(20)
    assert not pebble.wait_for_log("idle-exit", timeout=0.1), "left while still in use"
    pebble.press("down")    # open Settings
    time.sleep(15)
    pebble.press("down")    # scroll the menu: still counts as use
    time.sleep(20)          # 35 s after opening Settings, 20 s after the scroll
    assert not pebble.wait_for_log("idle-exit", timeout=0.1), "scrolling did not postpone the exit"
    assert pebble.wait_for_log("idle-exit", timeout=20), "app did not leave after 30 s idle"
