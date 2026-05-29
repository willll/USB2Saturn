#ifndef INPUT_MAPPER_H
#define INPUT_MAPPER_H

#include <stdbool.h>
#include <stdint.h>

#include "saturn_out.h"

#ifdef __cplusplus
extern "C" {
#endif

// Clear all Saturn digital button state fields.
void saturn_gamepad_state_clear(volatile saturn_gamepad_state_t* state);

// Map a USB boot keyboard report to Saturn digital state.
// Returns false when the report is too short to parse.
bool saturn_map_keyboard_boot_report(uint8_t const* report, uint16_t len,
                                     volatile saturn_gamepad_state_t* state);

// Map a USB boot mouse report to Saturn digital state.
// Returns false when the report is too short to parse.
bool saturn_map_mouse_boot_report(uint8_t const* report, uint16_t len,
                                  volatile saturn_gamepad_state_t* state);

// Map a generic gamepad report used by current fallback behavior.
// Returns false when the report is too short to parse.
bool saturn_map_generic_gamepad_report(uint8_t const* report, uint16_t len,
                                       volatile saturn_gamepad_state_t* state);

// Translate USB HID boot keyboard keycode to Saturn keyboard scancode.
// Returns false when the key is unsupported.
bool saturn_map_usb_keycode_to_saturn_scancode(uint8_t usb_keycode, uint8_t* saturn_scancode);

#ifdef __cplusplus
}
#endif

#endif // INPUT_MAPPER_H
