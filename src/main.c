#include <stdio.h>
#include "pico/stdlib.h"
#include "usb_host.h"
#include "saturn_out.h"

int main() {
    // Initialize standard I/O for debugging (optional)
    stdio_init_all();
    
    printf("USB2Saturn Starting...\n");

    // Initialize Saturn output protocol (GPIOs and interrupts)
    saturn_out_init();

    // Initialize USB Host (TinyUSB)
    usb_host_init();

    printf("Initialization Complete. Entering main loop.\n");

    while (1) {
        // Run the TinyUSB host task
        // This will trigger callbacks when HID reports arrive
        usb_host_task();
        
        // The Saturn protocol is handled asynchronously by GPIO interrupts
        // so we don't need to do anything else here except sleep briefly if we wanted to
        // save power, but tinyusb task should run continuously.
    }

    return 0;
}
