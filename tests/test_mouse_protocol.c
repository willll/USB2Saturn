#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "saturn_mouse_protocol.h"

static uint8_t g_toggle = 0u;

static void reset_select_toggle(void) {
    g_toggle = 0u;
}

static uint8_t next_nibble(saturn_mouse_protocol_t* p) {
    g_toggle ^= 1u;
    return saturn_mouse_protocol_next_nibble(p, 0u, g_toggle);
}

static void read_packet(saturn_mouse_protocol_t* p, uint8_t out[8]) {
    reset_select_toggle();
    for (int i = 0; i < 8; ++i) {
        out[i] = next_nibble(p);
    }
}

static void test_basic_mouse_packet_sequence(void) {
    saturn_mouse_protocol_t p;
    uint8_t n[8];

    saturn_mouse_protocol_reset(&p);
    saturn_mouse_protocol_set_buttons(&p, true, false, true, false);
    saturn_mouse_protocol_add_motion(&p, 0x12, -3);
    read_packet(&p, n);

    assert(n[0] == 0xEu);
    assert(n[1] == 0x3u);

    // Control byte: Y sign + middle + left = 0x25
    assert(n[2] == 0x2u);
    assert(n[3] == 0x5u);

    assert(n[4] == 0x1u);
    assert(n[5] == 0x2u);

    assert(n[6] == 0x0u);
    assert(n[7] == 0x3u);
}

static void test_motion_consumed_after_packet(void) {
    saturn_mouse_protocol_t p;
    uint8_t n[8];

    saturn_mouse_protocol_reset(&p);
    saturn_mouse_protocol_add_motion(&p, 5, 7);
    read_packet(&p, n);
    read_packet(&p, n);

    // Header unchanged.
    assert(n[0] == 0xEu);
    assert(n[1] == 0x3u);

    // No movement after prior packet was consumed.
    assert(n[4] == 0x0u);
    assert(n[5] == 0x0u);
    assert(n[6] == 0x0u);
    assert(n[7] == 0x0u);
}

static void test_overflow_bits_set(void) {
    saturn_mouse_protocol_t p;
    uint8_t n[8];

    saturn_mouse_protocol_reset(&p);
    saturn_mouse_protocol_add_motion(&p, 300, -300);
    read_packet(&p, n);

    // Control byte: Y over, X over, Y sign = 0xE0
    assert(n[2] == 0xEu);
    assert(n[3] == 0x0u);

    // Saturated magnitudes remain absolute values.
    assert(n[4] == 0xFu);
    assert(n[5] == 0xFu);
    assert(n[6] == 0xFu);
    assert(n[7] == 0xFu);
}

int main(void) {
    test_basic_mouse_packet_sequence();
    test_motion_consumed_after_packet();
    test_overflow_bits_set();

    puts("All mouse protocol tests passed.");
    return 0;
}