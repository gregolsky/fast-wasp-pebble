"""Uninstalling wipes the app's storage but the watch keeps its wakeups.
A reinstalled app must not let the old fast's alerts fire."""

import time

H = 3600
FAST_HALFWAY = 3
FIRE_TIMEOUT = 40


def test_reinstall_clears_old_fast_alerts(pebble):
    pebble.use_full_screen_alerts()
    old_start = pebble.emu_now()
    pebble.press("select")  # 12:12, old halfway at +6h
    assert pebble.wait_for_log("fast-active")

    pebble.uninstall()
    pebble.reinstall()
    pebble.set_time(old_start + H)  # new fast an hour later, as after a reinstall
    pebble.clear_logs()
    pebble.select_program(0)
    assert pebble.wait_for_log("state-ready")
    pebble.use_full_screen_alerts()
    pebble.press("select")  # new halfway at +7h
    assert pebble.wait_for_log("fast-active")

    pebble.jump_to_just_before(old_start + 6 * H)
    assert not pebble.wait_for_log(f"alert:{FAST_HALFWAY}", timeout=FIRE_TIMEOUT), \
        "old fast's halfway alert fired after reinstall"
