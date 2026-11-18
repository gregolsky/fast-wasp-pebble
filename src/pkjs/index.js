// Copyright 2025 Grzegorz Lachowski
// SPDX-License-Identifier: Apache-2.0

// Turns the watch's alert schedule into timeline pins with reminders, so
// alerts show as native popups. Reminders fire even with the phone away.
var keys = require('message_keys');

// Same order and copy as NotifyEvent / COPY in src/c/ui_alert.c. Reminders
// show only their title, so it carries the encouragement on its own.
var ALERTS = [
  { title: 'Fast complete',     reminder: 'Fast complete! Go eat',       body: 'You did it. Open FastWasp to start eating.', icon: 'REACHED_FITNESS_GOAL' },
  { title: 'Eat window closed', reminder: 'Eating over! Next fast now',  body: 'Open FastWasp to start the next fast.',     icon: 'RESULT_DELETED' },
  { title: 'OMAD meal time',    reminder: 'Meal time! 23h fasted',       body: '23h since your last meal.',                 icon: 'DINNER_RESERVATION' },
  { title: 'Halfway',           reminder: 'Halfway there! Keep going',   body: "You've got this. Keep flying.",             icon: 'ALARM_CLOCK' },
  { title: '1 hour left',       reminder: '1 hour left! Almost there',   body: "One hour. Don't blink.",                    icon: 'ALARM_CLOCK' },
  { title: 'Ketosis',           reminder: 'Ketosis! Burning fat now',    body: 'Fat is fuel. Stay sharp.',                  icon: 'ALARM_CLOCK' },
  { title: 'Autophagy',         reminder: 'Autophagy! Cells renewing',   body: 'Deep work. Cells clearing.',                icon: 'ALARM_CLOCK' }
];

// One pin per occurrence, so earlier fasts stay in the timeline history.
// localStorage remembers the current time per alert to find its pin again.
function slot(i) { return 'fastwasp-alert-' + i; }
function pinId(i, at) { return slot(i) + '-' + at; }

function readInt32LE(bytes, off) {
  return bytes[off] | (bytes[off + 1] << 8) | (bytes[off + 2] << 16) | (bytes[off + 3] << 24);
}

function syncPins(bytes) {
  if (!bytes || bytes.length !== ALERTS.length * 4) {
    console.log('pins-bad-payload');
    return false;
  }
  var now = Date.now() / 1000;
  for (var i = 0; i < ALERTS.length; i++) {
    var at = readInt32LE(bytes, i * 4);
    var prev = Number(localStorage.getItem(slot(i))) || 0;
    if (at === prev) continue;
    // Withdraw a pin still ahead; fired ones stay in the timeline as history.
    if (prev > now) {
      Pebble.deleteTimelinePin(pinId(i, prev));
      console.log('pin-del:' + i);
    }
    localStorage.removeItem(slot(i));
    if (at > now) {
      var a = ALERTS[i];
      var icon = 'system://images/' + a.icon;
      var iso = new Date(at * 1000).toISOString();
      Pebble.insertTimelinePin({
        id: pinId(i, at),
        time: iso,
        layout: { type: 'genericPin', title: a.title, subtitle: 'FastWasp', body: a.body, tinyIcon: icon },
        reminders: [{ time: iso, layout: { type: 'genericReminder', title: a.reminder, locationName: 'FastWasp', tinyIcon: icon } }]
      });
      localStorage.setItem(slot(i), String(at));
      console.log('pin-add:' + i + ':' + at);
    }
  }
  return true;
}

// The watch may be busy (nack), so retry a few times.
function send(key, value, triesLeft) {
  var msg = {};
  msg[key] = value;
  Pebble.sendAppMessage(msg, null, function () {
    if (triesLeft > 0) setTimeout(function () { send(key, value, triesLeft - 1); }, 1000);
  });
}

// The watch sends nothing until it hears READY.
Pebble.addEventListener('ready', function () { send(keys.READY, 1, 5); });

Pebble.addEventListener('appmessage', function (e) {
  var bytes = e.payload.PINS || e.payload[keys.PINS];
  if (bytes === undefined || !syncPins(bytes)) return;
  // The phone acks before this handler runs, so echo the times once the pins
  // are stored; only then may the watch app exit.
  send(keys.SYNCED, bytes, 3);
});
