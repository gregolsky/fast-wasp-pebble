#pragma once

#include <pebble.h>
#include "notify.h"

// Push a full-screen alert modal for the given wakeup event.
// SELECT and BACK both dismiss it.
void ui_alert_push(NotifyEvent event);
