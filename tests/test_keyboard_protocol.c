#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "saturn_keyboard_protocol.h"

static uint8_t g_toggle = 0u;

static void reset_select_toggle(void) {
    g_toggle = 0u;
}

static uint8_t next_nibble(saturn_keyboard_protocol_t* p, uint16_t game_key) {
    // Alternate select lines so each call advances one nibble.
    g_toggle ^= 1u;
    return saturn_keyboard_protocol_next_nibble(p, game_key, 0u, g_toggle);
}

static void read_packet(saturn_keyboard_protocol_t* p, uint16_t game_key, uint8_t out[12]) {
    reset_select_toggle();
    for (int i = 0; i < 12; ++i) {
        out[i] = next_nibble(p, game_key);
    }
}

static void test_make_packet_sequence(void) {
    saturn_keyboard_protocol_t p;
    uint8_t n[12];

    saturn_keyboard_protocol_reset(&p);
    saturn_keyboard_protocol_set_locks(&p, true, false, false);
    assert(saturn_keyboard_protocol_push_make(&p, 0x1Cu));

    read_packet(&p, 0xFFFFu, n);

    // 0x34 header
    assert(n[0] == 0x3u);
    assert(n[1] == 0x4u);

    // Game-key compatibility bytes (0xFF, 0xF8)
    assert(n[2] == 0xFu);
    assert(n[3] == 0xFu);
    assert(n[4] == 0xFu);
    assert(n[5] == 0x8u);

    // Status: Caps lock on, Make event set.
    assert(n[6] == 0x4u);
    assert(n[7] == 0xEu);

    // Scancode 0x1C
    assert(n[8] == 0x1u);
    assert(n[9] == 0xCu);

    // Tail nibbles are fixed.
    assert(n[10] == 0x0u);
    assert(n[11] == 0x1u);
}

static void test_break_packet_sequence(void) {
    saturn_keyboard_protocol_t p;
    uint8_t n[12];

    saturn_keyboard_protocol_reset(&p);
    assert(saturn_keyboard_protocol_push_break(&p, 0x1Cu));

    read_packet(&p, 0xFFFFu, n);

    // Break event in low status nibble (0b0111).
    assert(n[6] == 0x0u);
    assert(n[7] == 0x7u);
    assert(n[8] == 0x1u);
    assert(n[9] == 0xCu);
}

static void test_event_consumed_after_12th_nibble(void) {
    saturn_keyboard_protocol_t p;
    uint8_t n[12];

    saturn_keyboard_protocol_reset(&p);
    assert(saturn_keyboard_protocol_push_make(&p, 0x1Cu));

    read_packet(&p, 0xFFFFu, n);

    // Next packet has no pending event: status low nibble should be 0x6.
    for (int i = 0; i < 8; ++i) {
        n[i] = next_nibble(&p, 0xFFFFu);
    }

    assert(n[6] == 0x0u);
    assert(n[7] == 0x6u);
}

static void test_modifier_make_break_packets(void) {
    saturn_keyboard_protocol_t p;
    uint8_t n[12];

    // Official duplicated modifier scancodes from ST-TECH-45:
    // LAlt=0x11, RAlt=0x17, LShift=0x12, RShift=0x59, LCtrl=0x14, RCtrl=0x18.
    const uint8_t modifiers[] = {0x11u, 0x17u, 0x12u, 0x59u, 0x14u, 0x18u};

    saturn_keyboard_protocol_reset(&p);

    for (size_t i = 0; i < (sizeof(modifiers) / sizeof(modifiers[0])); ++i) {
        uint8_t sc = modifiers[i];

        assert(saturn_keyboard_protocol_push_make(&p, sc));
        read_packet(&p, 0xFFFFu, n);

        // Make status nibble and scancode.
        assert(n[6] == 0x0u);
        assert(n[7] == 0xEu);
        assert(n[8] == ((sc >> 4) & 0x0Fu));
        assert(n[9] == (sc & 0x0Fu));

        assert(saturn_keyboard_protocol_push_break(&p, sc));
        read_packet(&p, 0xFFFFu, n);

        // Break status nibble and scancode.
        assert(n[6] == 0x0u);
        assert(n[7] == 0x7u);
        assert(n[8] == ((sc >> 4) & 0x0Fu));
        assert(n[9] == (sc & 0x0Fu));
    }
}

int main(void) {
    test_make_packet_sequence();
    test_break_packet_sequence();
    test_event_consumed_after_12th_nibble();
    test_modifier_make_break_packets();

    puts("All keyboard protocol tests passed.");
    return 0;
}
