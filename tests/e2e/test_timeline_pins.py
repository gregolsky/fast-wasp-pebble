"""Timeline alert style (default): the phone turns alerts into pins with
reminders. JS logs `pin-add:<NotifyEvent>:<unix>` and `pin-del:<NotifyEvent>`."""

import time

H = 3600
FAST_COMPLETE, FAST_HALFWAY = 0, 3


def _start_fast(pebble):
    start = time.time()
    pebble.press("select")  # 12:12
    assert pebble.wait_for_log("fast-active")
    return start


def test_start_fast_inserts_pins(pebble):
    _start_fast(pebble)
    assert pebble.wait_for_log("pins-sent", timeout=20), "watch never sent alert times"
    assert pebble.wait_for_log(f"pin-add:{FAST_COMPLETE}:"), "goal pin missing"
    assert pebble.wait_for_log(f"pin-add:{FAST_HALFWAY}:"), "halfway pin missing"


def test_stop_fast_withdraws_pins(pebble):
    _start_fast(pebble)
    assert pebble.wait_for_log(f"pin-add:{FAST_COMPLETE}:", timeout=20)
    pebble.press("select")  # stop early
    assert pebble.wait_for_log(f"pin-del:{FAST_COMPLETE}", timeout=20), "goal pin left behind"


def test_reminder_shows_without_app_alert(pebble):
    start = _start_fast(pebble)
    assert pebble.wait_for_log(f"pin-add:{FAST_HALFWAY}:", timeout=20)
    pebble.press("back")  # exit to the watchface
    time.sleep(1)
    pebble.clear_logs()
    pebble.jump_to_just_before(start + 6 * H)
    time.sleep(20)
    pebble.screenshot(f"{pebble.platform}_reminder_halfway")
    assert not pebble.wait_for_log(f"alert:{FAST_HALFWAY}", timeout=1), \
        "full-screen alert fired in timeline mode"


def test_switch_to_full_screen_moves_pending_alerts(pebble):
    start = _start_fast(pebble)
    assert pebble.wait_for_log(f"pin-add:{FAST_HALFWAY}:", timeout=20)
    pebble.use_full_screen_alerts()
    assert pebble.wait_for_log(f"pin-del:{FAST_HALFWAY}", timeout=20), "pin not withdrawn"
    pebble.jump_to_just_before(start + 6 * H)
    assert pebble.wait_for_log(f"alert:{FAST_HALFWAY}", timeout=30), "wakeup not scheduled"


def test_switch_back_to_timeline_reinserts_pins(pebble):
    pebble.use_full_screen_alerts()
    _start_fast(pebble)
    pebble.clear_logs()
    pebble.use_timeline_alerts()
    assert pebble.wait_for_log(f"pin-add:{FAST_COMPLETE}:", timeout=20), "pins not restored"


# Not covered: a sync missed while the phone is away, repaired on next launch.
# `pebble emu-bt-connection --connected no` cuts the tool's own link to the
# emulator and it never comes back, so that path can't be driven here.


def test_relaunch_does_not_duplicate_pins(pebble):
    _start_fast(pebble)
    assert pebble.wait_for_log("pins-confirmed", timeout=20)
    time.sleep(1)  # let the first session's log lines drain
    pebble.clear_logs()
    pebble.reinstall()
    assert pebble.wait_for_log("pins-confirmed", timeout=30)
    assert not pebble.wait_for_log("pin-add:", timeout=3), "pins re-inserted on relaunch"


def test_exit_right_after_start_still_delivers_pins(pebble):
    pebble.reinstall(settle_s=0)  # relaunch: the phone script is still starting
    pebble.press("select")
    pebble.press("back")
    assert pebble.wait_for_log(f"pin-add:{FAST_COMPLETE}:", timeout=20), "pins lost on exit"
    assert pebble.wait_for_log("pins-confirmed"), "app left before the phone stored the pins"
