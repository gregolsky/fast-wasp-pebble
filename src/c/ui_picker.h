#pragma once

#include <pebble.h>

// Push the program-picker window onto the stack.
// on_select: called with the chosen program index (0..5).
void ui_picker_push(void (*on_select)(uint8_t program_idx));
