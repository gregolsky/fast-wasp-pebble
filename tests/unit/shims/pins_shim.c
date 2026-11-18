// Host stand-in for src/c/pins.c: counts resync requests instead of messaging the phone.
#include "pins.h"

int g_pins_sync_count;

void pins_init(void) {}
void pins_sync(void) { g_pins_sync_count++; }
