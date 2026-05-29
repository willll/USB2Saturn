#include "usb_host.h"
#include "saturn_out.h"
#include "input_mapper.h"

#include "bsp/board_api.h"
#include "tusb.h"

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
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    // Determine the report type based on the protocol
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

    if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD) {
        // Boot keyboard report mapping to Saturn digital controls.
        if (!saturn_map_keyboard_boot_report(report, len, &g_saturn_state)) {
            saturn_gamepad_state_clear(&g_saturn_state);
        }
    } else if (itf_protocol == HID_ITF_PROTOCOL_MOUSE) {
        // Boot mouse report mapping to Saturn digital controls.
        // This provides usable mouse-driven movement/buttons while preserving
        // the existing Saturn digital output protocol implementation.
        if (!saturn_map_mouse_boot_report(report, len, &g_saturn_state)) {
            saturn_gamepad_state_clear(&g_saturn_state);
        }
    } else {
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
