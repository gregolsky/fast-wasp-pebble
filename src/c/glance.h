#pragma once

#include <stdbool.h>
#include <stddef.h>

// Build the launcher App Glance subtitle template for the current state.
// Returns false when there is nothing to show (no program / ready state).
bool glance_subtitle(char *buf, size_t len);

// Push the current state to the launcher glance. Call on app exit.
void glance_update(void);
