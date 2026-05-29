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
#define HID_KEY_ESCAPE  0x29u
#define HID_KEY_BSPACE  0x2Au
#define HID_KEY_TAB     0x2Bu
#define HID_KEY_SPACE   0x2Cu
#define HID_KEY_MINUS   0x2Du
#define HID_KEY_EQUAL   0x2Eu
#define HID_KEY_LBRACE  0x2Fu
#define HID_KEY_RBRACE  0x30u
#define HID_KEY_BSLASH  0x31u
#define HID_KEY_SCOLON  0x33u
#define HID_KEY_QUOTE   0x34u
#define HID_KEY_GRAVE   0x35u
#define HID_KEY_COMMA   0x36u
#define HID_KEY_DOT     0x37u
#define HID_KEY_SLASH   0x38u
#define HID_KEY_CAPS    0x39u
#define HID_KEY_F1      0x3Au
#define HID_KEY_F2      0x3Bu
#define HID_KEY_F3      0x3Cu
#define HID_KEY_F4      0x3Du
#define HID_KEY_F5      0x3Eu
#define HID_KEY_F6      0x3Fu
#define HID_KEY_F7      0x40u
#define HID_KEY_F8      0x41u
#define HID_KEY_F9      0x42u
#define HID_KEY_F10     0x43u
#define HID_KEY_F11     0x44u
#define HID_KEY_F12     0x45u
#define HID_KEY_PSCR    0x46u
#define HID_KEY_SCROLL  0x47u
#define HID_KEY_PAUSE   0x48u
#define HID_KEY_INSERT  0x49u
#define HID_KEY_HOME    0x4Au
#define HID_KEY_PGUP    0x4Bu
#define HID_KEY_DELETE  0x4Cu
#define HID_KEY_END     0x4Du
#define HID_KEY_PGDN    0x4Eu
#define HID_KEY_RIGHT   0x4Fu
#define HID_KEY_LEFT    0x50u
#define HID_KEY_DOWN    0x51u
#define HID_KEY_UP      0x52u
#define HID_KEY_NUMLOCK 0x53u

#define HID_KEY_1       0x1Eu
#define HID_KEY_2       0x1Fu
#define HID_KEY_3       0x20u
#define HID_KEY_4       0x21u
#define HID_KEY_5       0x22u
#define HID_KEY_6       0x23u
#define HID_KEY_7       0x24u
#define HID_KEY_8       0x25u
#define HID_KEY_9       0x26u
#define HID_KEY_0       0x27u
#define HID_KEY_B       0x05u
#define HID_KEY_F       0x09u
#define HID_KEY_G       0x0Au
#define HID_KEY_H       0x0Bu
#define HID_KEY_I       0x0Cu
#define HID_KEY_J       0x0Du
#define HID_KEY_K       0x0Eu
#define HID_KEY_L       0x0Fu
#define HID_KEY_M       0x10u
#define HID_KEY_N       0x11u
#define HID_KEY_O       0x12u
#define HID_KEY_P       0x13u
#define HID_KEY_R       0x15u
#define HID_KEY_T       0x17u
#define HID_KEY_U       0x18u
#define HID_KEY_V       0x19u
#define HID_KEY_Y       0x1Cu

static bool key_present(uint8_t const* report, uint8_t keycode) {
    // Boot keyboard report: [modifiers, reserved, key0..key5]
    for (size_t i = 2; i < 8; ++i) {
        if (report[i] == keycode) {
            return true;
        }
    }
    return false;
}

static void saturn_apply_button_bits(volatile saturn_gamepad_state_t* state, uint16_t buttons) {
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

static void saturn_apply_hat_dpad(volatile saturn_gamepad_state_t* state, uint8_t hat) {
    // Standard HID hat: 0..7 directions, 8 (or 0x0F) = neutral.
    switch (hat & 0x0Fu) {
    case 0x00u: state->up = true; break;
    case 0x01u: state->up = true; state->right = true; break;
    case 0x02u: state->right = true; break;
    case 0x03u: state->down = true; state->right = true; break;
    case 0x04u: state->down = true; break;
    case 0x05u: state->down = true; state->left = true; break;
    case 0x06u: state->left = true; break;
    case 0x07u: state->up = true; state->left = true; break;
    default: break;
    }
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
    if (!report || !state) {
        return false;
    }

    // Prefer the analog layout for longer descriptor-unaware reports.
    // Keep the compact 3-byte layout for simple digital/hat-only devices.
    if (len >= 7) {
        return saturn_map_analog_gamepad_report(report, len, state);
    }

    return saturn_map_digital_gamepad_report(report, len, state);
}

bool saturn_map_digital_gamepad_report(uint8_t const* report, uint16_t len,
                                       volatile saturn_gamepad_state_t* state) {
    if (!report || !state || len < 3) {
        return false;
    }

    saturn_gamepad_state_clear(state);

    {
        uint16_t buttons = (uint16_t) report[0] | ((uint16_t) report[1] << 8);
        saturn_apply_button_bits(state, buttons);
    }

    saturn_apply_hat_dpad(state, report[2]);
    return true;
}

bool saturn_map_analog_gamepad_report(uint8_t const* report, uint16_t len,
                                      volatile saturn_gamepad_state_t* state) {
    // Descriptor-unaware fallback used by many generic USB pads.
    // byte3/4 are often LX/LY centered at 128.
    const uint8_t analog_low = 96u;
    const uint8_t analog_high = 160u;

    if (!report || !state || len < 7) {
        return false;
    }

    saturn_gamepad_state_clear(state);

    state->left = (report[3] < analog_low);
    state->right = (report[3] > analog_high);
    state->up = (report[4] < analog_low);
    state->down = (report[4] > analog_high);

    {
        uint16_t buttons = (uint16_t) report[5] | ((uint16_t) report[6] << 8);
        saturn_apply_button_bits(state, buttons);
    }

    return true;
}

bool saturn_map_usb_keycode_to_saturn_scancode(uint8_t usb_keycode, uint8_t* saturn_scancode) {
    if (!saturn_scancode) {
        return false;
    }

    switch (usb_keycode) {
    case HID_KEY_A: *saturn_scancode = 0x1Cu; return true;
    case HID_KEY_B: *saturn_scancode = 0x32u; return true;
    case HID_KEY_C: *saturn_scancode = 0x21u; return true;
    case HID_KEY_D: *saturn_scancode = 0x23u; return true;
    case HID_KEY_E: *saturn_scancode = 0x24u; return true;
    case HID_KEY_F: *saturn_scancode = 0x2Bu; return true;
    case HID_KEY_G: *saturn_scancode = 0x34u; return true;
    case HID_KEY_H: *saturn_scancode = 0x33u; return true;
    case HID_KEY_I: *saturn_scancode = 0x43u; return true;
    case HID_KEY_J: *saturn_scancode = 0x3Bu; return true;
    case HID_KEY_K: *saturn_scancode = 0x42u; return true;
    case HID_KEY_L: *saturn_scancode = 0x4Bu; return true;
    case HID_KEY_M: *saturn_scancode = 0x3Au; return true;
    case HID_KEY_N: *saturn_scancode = 0x31u; return true;
    case HID_KEY_O: *saturn_scancode = 0x44u; return true;
    case HID_KEY_P: *saturn_scancode = 0x4Du; return true;
    case HID_KEY_Q: *saturn_scancode = 0x15u; return true;
    case HID_KEY_R: *saturn_scancode = 0x2Du; return true;
    case HID_KEY_S: *saturn_scancode = 0x1Bu; return true;
    case HID_KEY_T: *saturn_scancode = 0x2Cu; return true;
    case HID_KEY_U: *saturn_scancode = 0x3Cu; return true;
    case HID_KEY_V: *saturn_scancode = 0x2Au; return true;
    case HID_KEY_W: *saturn_scancode = 0x1Du; return true;
    case HID_KEY_X: *saturn_scancode = 0x22u; return true;
    case HID_KEY_Y: *saturn_scancode = 0x35u; return true;
    case HID_KEY_Z: *saturn_scancode = 0x1Au; return true;

    case HID_KEY_1: *saturn_scancode = 0x16u; return true;
    case HID_KEY_2: *saturn_scancode = 0x1Eu; return true;
    case HID_KEY_3: *saturn_scancode = 0x26u; return true;
    case HID_KEY_4: *saturn_scancode = 0x25u; return true;
    case HID_KEY_5: *saturn_scancode = 0x2Eu; return true;
    case HID_KEY_6: *saturn_scancode = 0x36u; return true;
    case HID_KEY_7: *saturn_scancode = 0x3Du; return true;
    case HID_KEY_8: *saturn_scancode = 0x3Eu; return true;
    case HID_KEY_9: *saturn_scancode = 0x46u; return true;
    case HID_KEY_0: *saturn_scancode = 0x45u; return true;

    case HID_KEY_ENTER:  *saturn_scancode = 0x5Au; return true;
    case HID_KEY_ESCAPE: *saturn_scancode = 0x76u; return true;
    case HID_KEY_BSPACE: *saturn_scancode = 0x66u; return true;
    case HID_KEY_TAB:    *saturn_scancode = 0x0Du; return true;
    case HID_KEY_SPACE:  *saturn_scancode = 0x29u; return true;
    case HID_KEY_MINUS:  *saturn_scancode = 0x4Eu; return true;
    case HID_KEY_EQUAL:  *saturn_scancode = 0x4Eu; return true;
    case HID_KEY_LBRACE: *saturn_scancode = 0x5Bu; return true;
    case HID_KEY_RBRACE: *saturn_scancode = 0x5Du; return true;
    case HID_KEY_BSLASH: *saturn_scancode = 0x51u; return true;
    case HID_KEY_SCOLON: *saturn_scancode = 0x4Cu; return true;
    case HID_KEY_QUOTE:  *saturn_scancode = 0x52u; return true;
    case HID_KEY_GRAVE:  *saturn_scancode = 0x54u; return true;
    case HID_KEY_COMMA:  *saturn_scancode = 0x41u; return true;
    case HID_KEY_DOT:    *saturn_scancode = 0x49u; return true;
    case HID_KEY_SLASH:  *saturn_scancode = 0x4Au; return true;

    case HID_KEY_CAPS:   *saturn_scancode = 0x58u; return true;
    case HID_KEY_NUMLOCK:*saturn_scancode = 0x77u; return true;
    case HID_KEY_SCROLL: *saturn_scancode = 0x7Eu; return true;

    case HID_KEY_F1:  *saturn_scancode = 0x01u; return true;
    case HID_KEY_F2:  *saturn_scancode = 0x03u; return true;
    case HID_KEY_F3:  *saturn_scancode = 0x04u; return true;
    case HID_KEY_F4:  *saturn_scancode = 0x05u; return true;
    case HID_KEY_F5:  *saturn_scancode = 0x06u; return true;
    case HID_KEY_F6:  *saturn_scancode = 0x09u; return true;
    case HID_KEY_F7:  *saturn_scancode = 0x0Au; return true;
    case HID_KEY_F8:  *saturn_scancode = 0x0Bu; return true;
    case HID_KEY_F9:  *saturn_scancode = 0x0Cu; return true;
    case HID_KEY_F10: *saturn_scancode = 0x0Du; return true;
    case HID_KEY_F11: *saturn_scancode = 0x78u; return true;
    case HID_KEY_F12: *saturn_scancode = 0x07u; return true;

    case HID_KEY_UP:    *saturn_scancode = 0x89u; return true;
    case HID_KEY_DOWN:  *saturn_scancode = 0x8Au; return true;
    case HID_KEY_LEFT:  *saturn_scancode = 0x86u; return true;
    case HID_KEY_RIGHT: *saturn_scancode = 0x8Du; return true;

    case HID_KEY_INSERT: *saturn_scancode = 0x81u; return true;
    case HID_KEY_HOME:   *saturn_scancode = 0x87u; return true;
    case HID_KEY_PGUP:   *saturn_scancode = 0x8Bu; return true;
    case HID_KEY_DELETE: *saturn_scancode = 0x85u; return true;
    case HID_KEY_END:    *saturn_scancode = 0x88u; return true;
    case HID_KEY_PGDN:   *saturn_scancode = 0x8Cu; return true;
    case HID_KEY_PSCR:   *saturn_scancode = 0x84u; return true;
    case HID_KEY_PAUSE:  *saturn_scancode = 0x82u; return true;
    default:
        return false;
    }
}
