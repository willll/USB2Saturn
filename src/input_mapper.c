#include "input_mapper.h"

#include <stddef.h>

// USB HID boot keyboard keycodes we map.
#define HID_KEY_A       0x04u
#define HID_KEY_C       0x06u
#define HID_KEY_D       0x07u
#define HID_KEY_E       0x08u
#define HID_KEY_Q       0x14u
#define HID_KEY_S       0x16u
#define HID_KEY_W       0x1Au
#define HID_KEY_X       0x1Bu
#define HID_KEY_Z       0x1Du
#define HID_KEY_ENTER   0x28u
#define HID_KEY_SPACE   0x2Cu
#define HID_KEY_RIGHT   0x4Fu
#define HID_KEY_LEFT    0x50u
#define HID_KEY_DOWN    0x51u
#define HID_KEY_UP      0x52u

static bool key_present(uint8_t const* report, uint8_t keycode) {
    // Boot keyboard report: [modifiers, reserved, key0..key5]
    for (size_t i = 2; i < 8; ++i) {
        if (report[i] == keycode) {
            return true;
        }
    }
    return false;
}

void saturn_gamepad_state_clear(volatile saturn_gamepad_state_t* state) {
    if (!state) {
        return;
    }

    state->up = false;
    state->down = false;
    state->left = false;
    state->right = false;
    state->a = false;
    state->b = false;
    state->c = false;
    state->x = false;
    state->y = false;
    state->z = false;
    state->l = false;
    state->r = false;
    state->start = false;
}

bool saturn_map_keyboard_boot_report(uint8_t const* report, uint16_t len,
                                     volatile saturn_gamepad_state_t* state) {
    if (!report || !state || len < 8) {
        return false;
    }

    saturn_gamepad_state_clear(state);

    // D-pad from arrows or WASD.
    state->up = key_present(report, HID_KEY_UP) || key_present(report, HID_KEY_W);
    state->down = key_present(report, HID_KEY_DOWN) || key_present(report, HID_KEY_S);
    state->left = key_present(report, HID_KEY_LEFT) || key_present(report, HID_KEY_A);
    state->right = key_present(report, HID_KEY_RIGHT) || key_present(report, HID_KEY_D);

    // Face/shoulder/start mapping.
    state->a = key_present(report, HID_KEY_Z);
    state->b = key_present(report, HID_KEY_X);
    state->c = key_present(report, HID_KEY_C);
    state->x = key_present(report, HID_KEY_A);
    state->y = key_present(report, HID_KEY_S);
    state->z = key_present(report, HID_KEY_D);
    state->l = key_present(report, HID_KEY_Q);
    state->r = key_present(report, HID_KEY_E);
    state->start = key_present(report, HID_KEY_ENTER) || key_present(report, HID_KEY_SPACE);

    return true;
}

bool saturn_map_mouse_boot_report(uint8_t const* report, uint16_t len,
                                  volatile saturn_gamepad_state_t* state) {
    if (!report || !state || len < 3) {
        return false;
    }

    uint8_t buttons = report[0];
    int8_t dx = (int8_t) report[1];
    int8_t dy = (int8_t) report[2];
    int8_t wheel = 0;
    const int8_t deadzone = 8;

    if (len >= 4) {
        wheel = (int8_t) report[3];
    }

    // Keep this mapping deterministic for now:
    // - Mouse movement drives D-pad directions.
    // - L/R/M mouse buttons map to A/B/C.
    // - Wheel maps to L/R shoulder taps.
    state->left = (dx < -deadzone);
    state->right = (dx > deadzone);
    state->up = (dy < -deadzone);
    state->down = (dy > deadzone);

    state->a = ((buttons & 0x01u) != 0u);
    state->b = ((buttons & 0x02u) != 0u);
    state->c = ((buttons & 0x04u) != 0u);

    state->l = (wheel > 0);
    state->r = (wheel < 0);

    // Mouse mode does not currently map these fields.
    state->x = false;
    state->y = false;
    state->z = false;
    state->start = false;

    return true;
}

bool saturn_map_generic_gamepad_report(uint8_t const* report, uint16_t len,
                                       volatile saturn_gamepad_state_t* state) {
    if (!report || !state || len < 7) {
        return false;
    }

    // Very generic fallback mapping (descriptor-unaware).
    state->left = (report[3] < 128u);
    state->right = (report[3] > 128u);
    state->up = (report[4] < 128u);
    state->down = (report[4] > 128u);

    {
        uint16_t buttons = (uint16_t) report[5] | ((uint16_t) report[6] << 8);
        state->a = ((buttons & 0x0001u) != 0u);
        state->b = ((buttons & 0x0002u) != 0u);
        state->x = ((buttons & 0x0004u) != 0u);
        state->y = ((buttons & 0x0008u) != 0u);
        state->l = ((buttons & 0x0010u) != 0u);
        state->r = ((buttons & 0x0020u) != 0u);
        state->c = ((buttons & 0x0040u) != 0u);
        state->z = ((buttons & 0x0080u) != 0u);
        state->start = ((buttons & 0x0200u) != 0u);
    }

    return true;
}
