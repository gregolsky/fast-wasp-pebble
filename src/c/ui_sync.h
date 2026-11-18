#pragma once

// Shown when leaving the app before the phone has confirmed the latest alert
// times: the phone drops the app's script on exit, so old timeline pins would
// stay and fire. Exits once the phone confirms, or after a warning.
void ui_sync_push(void);
