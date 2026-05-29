#ifndef USB_HOST_H
#define USB_HOST_H

#ifdef __cplusplus
extern "C" {
#endif

// Initialize TinyUSB host
void usb_host_init(void);

// Task to be called in the main loop
void usb_host_task(void);

#ifdef __cplusplus
}
#endif

#endif // USB_HOST_H
