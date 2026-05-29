#ifndef SATURN_OUT_H
#define SATURN_OUT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Sega Saturn Gamepad State
// 1 means pressed, 0 means unpressed
typedef struct {
    bool up;
    bool down;
    bool left;
    bool right;
    bool a;
    bool b;
    bool c;
    bool x;
    bool y;
    bool z;
    bool l;
    bool r;
    bool start;
} saturn_gamepad_state_t;

typedef enum {
    SATURN_PERIPHERAL_GAMEPAD = 0,
    SATURN_PERIPHERAL_KEYBOARD = 1,
    SATURN_PERIPHERAL_MOUSE = 2,
} saturn_peripheral_mode_t;

// Global state accessed by the output protocol
extern volatile saturn_gamepad_state_t g_saturn_state;

// Select which Saturn peripheral behavior is currently emulated.
void saturn_set_peripheral_mode(saturn_peripheral_mode_t mode);
saturn_peripheral_mode_t saturn_get_peripheral_mode(void);

// Push a Saturn keyboard scancode (make code) into the transmit queue.
bool saturn_keyboard_push_scancode(uint8_t scancode);

// Push a Saturn keyboard scancode release (break code event) into the transmit queue.
bool saturn_keyboard_push_break_scancode(uint8_t scancode);

// Release currently held keyboard data byte (used on key release).
void saturn_keyboard_release_key(void);

// Update keyboard lock indicators encoded in the status register.
void saturn_keyboard_set_locks(bool caps_lock, bool num_lock, bool scroll_lock);

// Reset keyboard emulation state and clear queued keycodes.
void saturn_keyboard_reset_state(void);

// Update the Saturn mouse packet state from a USB boot mouse report.
// dx/dy are relative deltas; buttons use USB boot bit order (L/R/M in bits 0..2).
void saturn_mouse_submit_report(uint8_t buttons, int8_t dx, int8_t dy);

// Reset mouse emulation state and clear accumulated motion.
void saturn_mouse_reset_state(void);

// Initialize the GPIO pins for the Saturn protocol
void saturn_out_init(void);

#ifdef __cplusplus
}
#endif

#endif // SATURN_OUT_H
