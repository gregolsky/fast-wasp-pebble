#pragma once

// Timeline-pin alerts: sends the alert times to the phone (src/pkjs/index.js),
// which inserts or withdraws one pin with a reminder per alert.
void pins_init(void);

// Queue a resync of all alert times to the phone.
void pins_sync(void);

// Call cb once the phone confirms the current pin times (NULL to stop).
void pins_when_synced(void (*cb)(void));
