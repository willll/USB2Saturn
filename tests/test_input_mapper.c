#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "input_mapper.h"

static void expect_all_clear(volatile saturn_gamepad_state_t* state) {
    assert(!state->up);
    assert(!state->down);
    assert(!state->left);
    assert(!state->right);
    assert(!state->a);
    assert(!state->b);
    assert(!state->c);
    assert(!state->x);
    assert(!state->y);
    assert(!state->z);
    assert(!state->l);
    assert(!state->r);
    assert(!state->start);
}

static void test_clear_state(void) {
    volatile saturn_gamepad_state_t state = {
        .up = true, .down = true, .left = true, .right = true,
        .a = true, .b = true, .c = true,
        .x = true, .y = true, .z = true,
        .l = true, .r = true, .start = true
    };

    saturn_gamepad_state_clear(&state);
    expect_all_clear(&state);
}

static void test_mouse_mapping_buttons_and_motion(void) {
    volatile saturn_gamepad_state_t state = {0};
    uint8_t report[] = {
        0x03, // left + right mouse buttons
        20,   // dx right
        (uint8_t) -30, // dy up
        1     // wheel up
    };

    bool ok = saturn_map_mouse_boot_report(report, (uint16_t) sizeof(report), &state);
    assert(ok);

    assert(state.right);
    assert(!state.left);
    assert(state.up);
    assert(!state.down);

    assert(state.a);
    assert(state.b);
    assert(!state.c);

    assert(state.l);
    assert(!state.r);

    assert(!state.x);
    assert(!state.y);
    assert(!state.z);
    assert(!state.start);
}

static void test_mouse_mapping_deadzone(void) {
    volatile saturn_gamepad_state_t state = {0};
    uint8_t report[] = {
        0x00,
        5,            // within deadzone
        (uint8_t) -7, // within deadzone
        0
    };

    bool ok = saturn_map_mouse_boot_report(report, (uint16_t) sizeof(report), &state);
    assert(ok);

    assert(!state.left);
    assert(!state.right);
    assert(!state.up);
    assert(!state.down);
}

static void test_mouse_invalid_report(void) {
    volatile saturn_gamepad_state_t state = {0};
    uint8_t short_report[] = {0x01, 0x02};

    bool ok = saturn_map_mouse_boot_report(short_report, (uint16_t) sizeof(short_report), &state);
    assert(!ok);
    expect_all_clear(&state);
}

static void test_keyboard_mapping_arrows_and_buttons(void) {
    volatile saturn_gamepad_state_t state = {0};
    uint8_t report[8] = {
        0x00,
        0x00,
        0x52, // Up
        0x50, // Left
        0x1D, // Z => Saturn A
        0x1B, // X => Saturn B
        0x06, // C => Saturn C
        0x28  // Enter => Start
    };

    bool ok = saturn_map_keyboard_boot_report(report, (uint16_t) sizeof(report), &state);
    assert(ok);

    assert(state.up);
    assert(!state.down);
    assert(state.left);
    assert(!state.right);

    assert(state.a);
    assert(state.b);
    assert(state.c);
    assert(state.start);
}

static void test_keyboard_mapping_wasd_shoulders(void) {
    volatile saturn_gamepad_state_t state = {0};
    uint8_t report[8] = {
        0x00,
        0x00,
        0x1A, // W => up
        0x07, // D => right and Saturn Z
        0x14, // Q => L
        0x08, // E => R
        0x16, // S => down and Saturn Y
        0x04  // A => left and Saturn X
    };

    bool ok = saturn_map_keyboard_boot_report(report, (uint16_t) sizeof(report), &state);
    assert(ok);

    assert(state.up);
    assert(state.down);
    assert(state.left);
    assert(state.right);

    assert(state.x);
    assert(state.y);
    assert(state.z);
    assert(state.l);
    assert(state.r);
}

static void test_keyboard_invalid_report(void) {
    volatile saturn_gamepad_state_t state = {0};
    uint8_t short_report[] = {0x00, 0x00, 0x52};

    bool ok = saturn_map_keyboard_boot_report(short_report, (uint16_t) sizeof(short_report), &state);
    assert(!ok);
    expect_all_clear(&state);
}

static void test_generic_gamepad_mapping(void) {
    volatile saturn_gamepad_state_t state = {0};
    uint8_t report[] = {
        0x00, 0x00, 0x00,
        0,      // axis x = left
        255,    // axis y = down
        0x03,   // buttons low byte (A + B)
        0x02    // buttons high byte (Start)
    };

    bool ok = saturn_map_generic_gamepad_report(report, (uint16_t) sizeof(report), &state);
    assert(ok);

    assert(state.left);
    assert(!state.right);
    assert(!state.up);
    assert(state.down);

    assert(state.a);
    assert(state.b);
    assert(state.start);
}

static void test_generic_gamepad_short_report_rejected(void) {
    volatile saturn_gamepad_state_t state = {0};
    uint8_t report[] = {0, 0, 0, 127, 127, 0xFF}; // len 6 (invalid)

    bool ok = saturn_map_generic_gamepad_report(report, (uint16_t) sizeof(report), &state);
    assert(!ok);
    expect_all_clear(&state);
}

int main(void) {
    test_clear_state();
    test_keyboard_mapping_arrows_and_buttons();
    test_keyboard_mapping_wasd_shoulders();
    test_keyboard_invalid_report();
    test_mouse_mapping_buttons_and_motion();
    test_mouse_mapping_deadzone();
    test_mouse_invalid_report();
    test_generic_gamepad_mapping();
    test_generic_gamepad_short_report_rejected();

    puts("All input mapper tests passed.");
    return 0;
}
