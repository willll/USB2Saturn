#include "saturn_out.h"
#include "saturn_keyboard_protocol.h"
#include "saturn_mouse_protocol.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

// Define the GPIO pins mapped to the Saturn port
#define PIN_S0  0
#define PIN_S1  1

#define PIN_D0  2
#define PIN_D1  3
#define PIN_D2  4
#define PIN_D3  5

volatile saturn_gamepad_state_t g_saturn_state = {0};

static volatile saturn_peripheral_mode_t g_peripheral_mode = SATURN_PERIPHERAL_GAMEPAD;
static saturn_keyboard_protocol_t g_keyboard_protocol;
static saturn_mouse_protocol_t g_mouse_protocol;

static uint16_t saturn_build_game_key_word(void) {
    uint16_t game_key = 0xFFFFu;

    if (g_saturn_state.right) { game_key &= (uint16_t)~(1u << 15); }
    if (g_saturn_state.left)  { game_key &= (uint16_t)~(1u << 14); }
    if (g_saturn_state.down)  { game_key &= (uint16_t)~(1u << 13); }
    if (g_saturn_state.up)    { game_key &= (uint16_t)~(1u << 12); }
    if (g_saturn_state.start) { game_key &= (uint16_t)~(1u << 11); }
    if (g_saturn_state.a)     { game_key &= (uint16_t)~(1u << 10); }
    if (g_saturn_state.c)     { game_key &= (uint16_t)~(1u << 9); }
    if (g_saturn_state.b)     { game_key &= (uint16_t)~(1u << 8); }
    if (g_saturn_state.r)     { game_key &= (uint16_t)~(1u << 7); }
    if (g_saturn_state.x)     { game_key &= (uint16_t)~(1u << 6); }
    if (g_saturn_state.y)     { game_key &= (uint16_t)~(1u << 5); }
    if (g_saturn_state.z)     { game_key &= (uint16_t)~(1u << 4); }
    if (g_saturn_state.l)     { game_key &= (uint16_t)~(1u << 3); }

    return game_key;
}

// Pre-calculate the output bitmasks for speed
// The data pins are contiguous from GPIO 2 to GPIO 5, so a mask shift is easy.
// A pressed button is LOW (0), unpressed is HIGH (1).
static inline void update_data_lines(uint8_t s0, uint8_t s1) {
    uint32_t data_val = 0;

    if (g_peripheral_mode == SATURN_PERIPHERAL_KEYBOARD) {
        data_val = saturn_keyboard_protocol_next_nibble(&g_keyboard_protocol,
                                                        saturn_build_game_key_word(),
                                                        s0,
                                                        s1);

        // Shift data_val to the correct GPIO base (PIN_D0)
        gpio_put_masked(0xFu << PIN_D0, ((uint32_t)data_val) << PIN_D0);
        return;
    }

    if (g_peripheral_mode == SATURN_PERIPHERAL_MOUSE) {
        data_val = saturn_mouse_protocol_next_nibble(&g_mouse_protocol, s0, s1);
        gpio_put_masked(0xFu << PIN_D0, ((uint32_t)data_val) << PIN_D0);
        return;
    }

    if (s1 == 1 && s0 == 0) {
        // D3=Right, D2=Left, D1=Down, D0=Up
        data_val |= (!g_saturn_state.right) ? (1 << 3) : 0;
        data_val |= (!g_saturn_state.left)  ? (1 << 2) : 0;
        data_val |= (!g_saturn_state.down)  ? (1 << 1) : 0;
        data_val |= (!g_saturn_state.up)    ? (1 << 0) : 0;
    } else if (s1 == 0 && s0 == 1) {
        // D3=Start, D2=A, D1=C, D0=B
        data_val |= (!g_saturn_state.start) ? (1 << 3) : 0;
        data_val |= (!g_saturn_state.a)     ? (1 << 2) : 0;
        data_val |= (!g_saturn_state.c)     ? (1 << 1) : 0;
        data_val |= (!g_saturn_state.b)     ? (1 << 0) : 0;
    } else if (s1 == 0 && s0 == 0) {
        // D3=R, D2=X, D1=Y, D0=Z
        data_val |= (!g_saturn_state.r)     ? (1 << 3) : 0;
        data_val |= (!g_saturn_state.x)     ? (1 << 2) : 0;
        data_val |= (!g_saturn_state.y)     ? (1 << 1) : 0;
        data_val |= (!g_saturn_state.z)     ? (1 << 0) : 0;
    } else { // s1 == 1 && s0 == 1
        // D3=L, D2=1, D1=1, D0=1
        data_val |= (!g_saturn_state.l)     ? (1 << 3) : 0;
        data_val |= (1 << 2);
        data_val |= (1 << 1);
        data_val |= (1 << 0);
    }

    // Shift data_val to the correct GPIO base (PIN_D0)
    uint32_t mask = (0xF << PIN_D0);
    uint32_t values = (data_val << PIN_D0);
    
    // Apply to GPIO outputs
    gpio_put_masked(mask, values);
}

// IRQ handler for when S0 or S1 changes state
static void saturn_gpio_callback(uint gpio, uint32_t events) {
    // Read the current state of S0 and S1
    uint8_t s0 = gpio_get(PIN_S0);
    uint8_t s1 = gpio_get(PIN_S1);
    
    update_data_lines(s0, s1);
}

void saturn_out_init(void) {
    // Initialize output pins (D0-D3)
    gpio_init(PIN_D0);
    gpio_init(PIN_D1);
    gpio_init(PIN_D2);
    gpio_init(PIN_D3);
    
    gpio_set_dir(PIN_D0, GPIO_OUT);
    gpio_set_dir(PIN_D1, GPIO_OUT);
    gpio_set_dir(PIN_D2, GPIO_OUT);
    gpio_set_dir(PIN_D3, GPIO_OUT);

    // Initial state (high = no buttons pressed)
    gpio_put_masked(0xF << PIN_D0, 0xF << PIN_D0);

    // Initialize input pins (S0, S1)
    gpio_init(PIN_S0);
    gpio_init(PIN_S1);
    
    gpio_set_dir(PIN_S0, GPIO_IN);
    gpio_set_dir(PIN_S1, GPIO_IN);
    
    // Pull-ups aren't strictly necessary if driven by Saturn, but good practice
    gpio_pull_up(PIN_S0);
    gpio_pull_up(PIN_S1);

    // Setup interrupts on any edge for both S0 and S1
    gpio_set_irq_enabled_with_callback(PIN_S0, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &saturn_gpio_callback);
    gpio_set_irq_enabled(PIN_S1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    
    saturn_keyboard_protocol_reset(&g_keyboard_protocol);
    saturn_mouse_protocol_reset(&g_mouse_protocol);
    saturn_keyboard_reset_state();
    saturn_mouse_reset_state();

    // Call once to set initial state
    update_data_lines(gpio_get(PIN_S0), gpio_get(PIN_S1));
}

void saturn_set_peripheral_mode(saturn_peripheral_mode_t mode) {
    g_peripheral_mode = mode;
    saturn_keyboard_protocol_reset_poll_state(&g_keyboard_protocol);
    saturn_mouse_protocol_reset_poll_state(&g_mouse_protocol);
}

saturn_peripheral_mode_t saturn_get_peripheral_mode(void) {
    return g_peripheral_mode;
}

bool saturn_keyboard_push_scancode(uint8_t scancode) {
    return saturn_keyboard_protocol_push_make(&g_keyboard_protocol, scancode);
}

bool saturn_keyboard_push_break_scancode(uint8_t scancode) {
    return saturn_keyboard_protocol_push_break(&g_keyboard_protocol, scancode);
}

void saturn_keyboard_release_key(void) {
    saturn_keyboard_protocol_release_last(&g_keyboard_protocol);
}

void saturn_keyboard_set_locks(bool caps_lock, bool num_lock, bool scroll_lock) {
    saturn_keyboard_protocol_set_locks(&g_keyboard_protocol, caps_lock, num_lock, scroll_lock);
}

void saturn_keyboard_reset_state(void) {
    saturn_keyboard_protocol_reset(&g_keyboard_protocol);
}

void saturn_mouse_submit_report(uint8_t buttons, int8_t dx, int8_t dy) {
    saturn_mouse_protocol_set_buttons(&g_mouse_protocol,
                                      (buttons & 0x01u) != 0u,
                                      (buttons & 0x02u) != 0u,
                                      (buttons & 0x04u) != 0u,
                                      false);
    saturn_mouse_protocol_add_motion(&g_mouse_protocol, dx, dy);
}

void saturn_mouse_reset_state(void) {
    saturn_mouse_protocol_reset(&g_mouse_protocol);
}
