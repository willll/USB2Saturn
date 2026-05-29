#include "usb_host.h"
#include "saturn_out.h"

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
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    // Determine the report type based on the protocol
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

    if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD) {
        // Simple keyboard mapping to Saturn Controller
        // For example: Arrows = D-Pad, Z=A, X=B, C=C, A=X, S=Y, D=Z, Enter=Start
        // This is a placeholder. A full implementation would parse report->keycode
    } else if (itf_protocol == HID_ITF_PROTOCOL_MOUSE) {
        // Mouse mapping to Saturn Mouse
    } else {
        // Generic HID / Gamepad
        // For a robust implementation, we should parse the HID report descriptor.
        // As a quick default for many standard gamepads (like Xbox 360 / Generic USB):
        // We will just try to guess based on standard layouts if parsing isn't available.

        // TODO: Add proper HID report parsing using tuh_hid_parse_report_descriptor
        // Here is a very crude hardcoded map for generic PS3/PC gamepads as a fallback.
        if (len >= 6) {
            // Very generic mapping (will vary wildly between controllers without proper report parsing)
            g_saturn_state.left = (report[3] < 128);
            g_saturn_state.right = (report[3] > 128);
            g_saturn_state.up = (report[4] < 128);
            g_saturn_state.down = (report[4] > 128);
            
            // Buttons usually on byte 5 and 6
            uint16_t buttons = report[5] | (report[6] << 8);
            g_saturn_state.a = (buttons & 0x0001); // often Cross/A
            g_saturn_state.b = (buttons & 0x0002); // often Circle/B
            g_saturn_state.x = (buttons & 0x0004); // often Square/X
            g_saturn_state.y = (buttons & 0x0008); // often Triangle/Y
            g_saturn_state.l = (buttons & 0x0010); // L1
            g_saturn_state.r = (buttons & 0x0020); // R1
            g_saturn_state.c = (buttons & 0x0040); // L2
            g_saturn_state.z = (buttons & 0x0080); // R2
            g_saturn_state.start = (buttons & 0x0200); // Start
        }
    }

    // Continue to request report
    tuh_hid_receive_report(dev_addr, instance);
}
