"""Wakeups fire for real: jump the emulator clock to just before each
scheduled alert and wait for the alert screen to log `alert:<NotifyEvent>`."""

import time

from test_omad import _switch_to_omad

H = 3600
FIRE_TIMEOUT = 30

# NotifyEvent values from src/c/notify.h
FAST_COMPLETE, OMAD_TARGET, FAST_HALFWAY = 0, 2, 3


def _start_fast(pebble):
    start = time.time()  # emulator clock starts at host time
    pebble.press("select")
    assert pebble.wait_for_log("fast-active")
    return start


def _expect_alert(pebble, event):
    assert pebble.wait_for_log(f"alert:{event}", timeout=FIRE_TIMEOUT), \
        f"alert {event} did not fire"


def test_halfway_fires(pebble):
    start = _start_fast(pebble)  # 12:12
    pebble.jump_to_just_before(start + 6 * H)
    _expect_alert(pebble, FAST_HALFWAY)
    pebble.screenshot("alert_halfway")


def test_fast_complete_fires(pebble):
    start = _start_fast(pebble)
    pebble.jump_to_just_before(start + 12 * H)
    _expect_alert(pebble, FAST_COMPLETE)
    pebble.screenshot("alert_fast_complete")
    pebble.press("select")  # dismiss alert
    assert pebble.wait_for_log("fast-complete")


def test_milestones_off_skips_halfway(pebble):
    pebble.press("down")    # Settings
    pebble.press("down")    # Milestones row
    pebble.press("select")  # toggle off
    pebble.press("back")
    start = _start_fast(pebble)
    pebble.jump_to_just_before(start + 6 * H)
    assert not pebble.wait_for_log(f"alert:{FAST_HALFWAY}", timeout=FIRE_TIMEOUT), \
        "halfway fired with milestones off"
    pebble.jump_to_just_before(start + 12 * H)  # control: goal still fires
    _expect_alert(pebble, FAST_COMPLETE)


def test_omad_first_meal_target_fires(pebble):
    _switch_to_omad(pebble)
    meal = time.time()
    pebble.press("select")  # first meal
    assert pebble.wait_for_log("omad-active")
    pebble.jump_to_just_before(meal + 23 * H)
    _expect_alert(pebble, OMAD_TARGET)
    pebble.screenshot("alert_omad")


def test_wakeup_relaunches_closed_app(pebble):
    start = _start_fast(pebble)
    pebble.press("back")    # exit app
    time.sleep(1)
    pebble.clear_logs()
    pebble.jump_to_just_before(start + 12 * H)
    assert pebble.wait_for_log("app-version:", timeout=FIRE_TIMEOUT), "app not relaunched"
    _expect_alert(pebble, FAST_COMPLETE)
