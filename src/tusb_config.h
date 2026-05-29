#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU                OPT_MCU_RP2040 // Also works for RP2350
#endif

#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_HOST

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS                 OPT_OS_NONE
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG              1
#endif

/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(4)))
#endif

//--------------------------------------------------------------------
// HOST CONFIGURATION
//--------------------------------------------------------------------

// Size of buffer to hold descriptors and other data used for enumeration
#define CFG_TUH_ENUMERATION_BUFSIZE 256

#define CFG_TUH_HUB                 1

// Max number of nodes supported by a hub
#define CFG_TUH_DEVICE_MAX          (CFG_TUH_HUB ? 4 : 1)

// Enable HID host
#define CFG_TUH_HID                 4

// Number of endpoint descriptors per hub
#define CFG_TUH_ENDPOINT_MAX        16

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CONFIG_H_ */
