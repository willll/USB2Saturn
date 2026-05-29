#include "usb_host.h"
#include "saturn_out.h"
#include "input_mapper.h"

#include "bsp/board_api.h"
#include "tusb.h"

static uint8_t g_prev_kbd_report[8] = {0};
static bool g_caps_lock = false;
static bool g_num_lock = false;
static bool g_scroll_lock = false;

static bool report_has_key(uint8_t const* report, uint8_t keycode) {
    for (uint8_t i = 2; i < 8; ++i) {
        if (report[i] == keycode) {
            return true;
        }
    }
    return false;
}

static uint8_t first_new_key(uint8_t const* report, uint8_t const* prev_report) {
    for (uint8_t i = 2; i < 8; ++i) {
        uint8_t key = report[i];
        if (key == 0u) {
            continue;
        }
        if (!report_has_key(prev_report, key)) {
            return key;
        }
    }
    return 0u;
}

static uint8_t first_released_key(uint8_t const* report, uint8_t const* prev_report) {
    for (uint8_t i = 2; i < 8; ++i) {
        uint8_t key = prev_report[i];
        if (key == 0u) {
            continue;
        }
        if (!report_has_key(report, key)) {
            return key;
        }
    }
    return 0u;
}

static uint8_t modifier_to_saturn_scancode(uint8_t mod_bit) {
    switch (mod_bit) {
    case 0: return 0x14u; // Left Ctrl
    case 1: return 0x12u; // Left Shift
    case 2: return 0x11u; // Left Alt
    case 3: return 0x1Fu; // Left GUI
    case 4: return 0x18u; // Right Ctrl
    case 5: return 0x59u; // Right Shift
    case 6: return 0x17u; // Right Alt
    case 7: return 0x27u; // Right GUI
    default: return 0u;
    }
}

static void emit_modifier_transitions(uint8_t modifiers, uint8_t prev_modifiers) {
    uint8_t changed = (uint8_t)(modifiers ^ prev_modifiers);
    for (uint8_t bit = 0; bit < 8; ++bit) {
        uint8_t mask = (uint8_t)(1u << bit);
        if ((changed & mask) == 0u) {
            continue;
        }

        uint8_t sc = modifier_to_saturn_scancode(bit);
        if (sc == 0u) {
            continue;
        }

        if ((modifiers & mask) != 0u) {
            saturn_keyboard_push_scancode(sc);
        } else {
            saturn_keyboard_push_break_scancode(sc);
        }
    }
}

// HID generic report processing
// We will look for standard Gamepad / Keyboard / Mouse usage
// For simplicity, this basic implementation maps standard HID gamepad buttons.

void usb_host_init(void) {
    board_init();
    tusb_init();
}

void usb_host_task(void) {
    tuh_task();
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    // Request to receive report
    tuh_hid_receive_report(dev_addr, instance);
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    // Reset state if needed
    (void) dev_addr;
    (void) instance;
    saturn_gamepad_state_clear(&g_saturn_state);
    saturn_set_peripheral_mode(SATURN_PERIPHERAL_GAMEPAD);
    saturn_keyboard_reset_state();
    saturn_mouse_reset_state();
    for (uint8_t i = 0; i < 8; ++i) {
        g_prev_kbd_report[i] = 0u;
    }
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    // Determine the report type based on the protocol
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

    if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD) {
        saturn_set_peripheral_mode(SATURN_PERIPHERAL_KEYBOARD);

        // Keep game-key compatibility while keyboard mode is active.
        if (!saturn_map_keyboard_boot_report(report, len, &g_saturn_state)) {
            saturn_gamepad_state_clear(&g_saturn_state);
        } else {
            uint8_t saturn_code = 0u;
            uint8_t modifiers = report[0];
            uint8_t prev_modifiers = g_prev_kbd_report[0];
            uint8_t usb_new_key = first_new_key(report, g_prev_kbd_report);
            uint8_t usb_released_key = first_released_key(report, g_prev_kbd_report);

            emit_modifier_transitions(modifiers, prev_modifiers);

            if (usb_new_key != 0u && saturn_map_usb_keycode_to_saturn_scancode(usb_new_key, &saturn_code)) {
                saturn_keyboard_push_scancode(saturn_code);

                // Toggle lock state bits on lock-key press.
                if (usb_new_key == 0x39u) { // Caps Lock
                    g_caps_lock = !g_caps_lock;
                } else if (usb_new_key == 0x53u) { // Num Lock
                    g_num_lock = !g_num_lock;
                } else if (usb_new_key == 0x47u) { // Scroll Lock
                    g_scroll_lock = !g_scroll_lock;
                }

                saturn_keyboard_set_locks(g_caps_lock, g_num_lock, g_scroll_lock);
            }

            if (usb_released_key != 0u && saturn_map_usb_keycode_to_saturn_scancode(usb_released_key, &saturn_code)) {
                saturn_keyboard_push_break_scancode(saturn_code);
            }

            for (uint8_t i = 0; i < 8; ++i) {
                g_prev_kbd_report[i] = report[i];
            }
        }
    } else if (itf_protocol == HID_ITF_PROTOCOL_MOUSE) {
        saturn_set_peripheral_mode(SATURN_PERIPHERAL_MOUSE);
        saturn_gamepad_state_clear(&g_saturn_state);

        if (!report || len < 3) {
            saturn_mouse_reset_state();
        } else {
            saturn_mouse_submit_report(report[0], (int8_t)report[1], (int8_t)report[2]);
        }
    } else {
        saturn_set_peripheral_mode(SATURN_PERIPHERAL_GAMEPAD);
        // Generic HID / Gamepad
        // For a robust implementation, we should parse the HID report descriptor.
        // As a quick default for many standard gamepads (like Xbox 360 / Generic USB):
        // We will just try to guess based on standard layouts if parsing isn't available.

        // TODO: Add proper HID report parsing using tuh_hid_parse_report_descriptor
        // Here is a very crude hardcoded map for generic PS3/PC gamepads as a fallback.
        if (!saturn_map_generic_gamepad_report(report, len, &g_saturn_state)) {
            saturn_gamepad_state_clear(&g_saturn_state);
        }
    }

    // Continue to request report
    tuh_hid_receive_report(dev_addr, instance);
}
