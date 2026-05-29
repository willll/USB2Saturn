#include "saturn_mouse_protocol.h"

#include <string.h>

static void accumulate_axis(int16_t* delta, bool* overflow, int16_t step) {
    int32_t combined;

    if (!delta || !overflow) {
        return;
    }

    combined = (int32_t)(*delta) + (int32_t)step;

    if (combined > 255) {
        *delta = 255;
        *overflow = true;
        return;
    }

    if (combined < -255) {
        *delta = -255;
        *overflow = true;
        return;
    }

    *delta = (int16_t)combined;
}

static uint8_t control_byte(saturn_mouse_protocol_t const* protocol) {
    uint8_t value = 0;

    if (!protocol) {
        return 0;
    }

    if (protocol->left) {
        value |= 0x01u;
    }
    if (protocol->right) {
        value |= 0x02u;
    }
    if (protocol->middle) {
        value |= 0x04u;
    }
    if (protocol->start) {
        value |= 0x08u;
    }
    if (protocol->delta_x < 0) {
        value |= 0x10u;
    }
    if (protocol->delta_y < 0) {
        value |= 0x20u;
    }
    if (protocol->x_over) {
        value |= 0x40u;
    }
    if (protocol->y_over) {
        value |= 0x80u;
    }

    return value;
}

static uint8_t axis_byte(int16_t delta) {
    if (delta < 0) {
        return (uint8_t)(-delta);
    }

    return (uint8_t)delta;
}

static uint8_t packet_nibble(saturn_mouse_protocol_t const* protocol, uint8_t nibble_index) {
    uint8_t data0 = SATURN_MOUSE_PROTOCOL_ID;
    uint8_t data1 = control_byte(protocol);
    uint8_t data2 = axis_byte(protocol->delta_x);
    uint8_t data3 = axis_byte(protocol->delta_y);

    switch (nibble_index % 8u) {
    case 0: return (data0 >> 4) & 0x0Fu;
    case 1: return data0 & 0x0Fu;
    case 2: return (data1 >> 4) & 0x0Fu;
    case 3: return data1 & 0x0Fu;
    case 4: return (data2 >> 4) & 0x0Fu;
    case 5: return data2 & 0x0Fu;
    case 6: return (data3 >> 4) & 0x0Fu;
    default: return data3 & 0x0Fu;
    }
}

static void consume_packet(saturn_mouse_protocol_t* protocol) {
    if (!protocol) {
        return;
    }

    protocol->delta_x = 0;
    protocol->delta_y = 0;
    protocol->x_over = false;
    protocol->y_over = false;
}

void saturn_mouse_protocol_reset(saturn_mouse_protocol_t* protocol) {
    if (!protocol) {
        return;
    }

    memset(protocol, 0, sizeof(*protocol));
    protocol->last_select_state = 0xFFu;
}

void saturn_mouse_protocol_reset_poll_state(saturn_mouse_protocol_t* protocol) {
    if (!protocol) {
        return;
    }

    protocol->last_select_state = 0xFFu;
    protocol->nibble_index = 0u;
}

void saturn_mouse_protocol_set_buttons(saturn_mouse_protocol_t* protocol,
                                       bool left,
                                       bool right,
                                       bool middle,
                                       bool start) {
    if (!protocol) {
        return;
    }

    protocol->left = left;
    protocol->right = right;
    protocol->middle = middle;
    protocol->start = start;
}

void saturn_mouse_protocol_add_motion(saturn_mouse_protocol_t* protocol, int16_t delta_x, int16_t delta_y) {
    if (!protocol) {
        return;
    }

    accumulate_axis(&protocol->delta_x, &protocol->x_over, delta_x);
    accumulate_axis(&protocol->delta_y, &protocol->y_over, delta_y);
}

uint8_t saturn_mouse_protocol_next_nibble(saturn_mouse_protocol_t* protocol,
                                          uint8_t s0,
                                          uint8_t s1) {
    uint8_t state;
    uint8_t nibble;

    if (!protocol) {
        return 0u;
    }

    state = (uint8_t)((s1 << 1) | s0);

    if (state != protocol->last_select_state) {
        nibble = packet_nibble(protocol, protocol->nibble_index % 8u);
        if ((protocol->nibble_index % 8u) == 7u) {
            consume_packet(protocol);
        }
        protocol->nibble_index++;
        protocol->last_select_state = state;
        return nibble;
    }

    if (protocol->nibble_index == 0u) {
        return packet_nibble(protocol, 0u);
    }

    return packet_nibble(protocol, (uint8_t)((protocol->nibble_index - 1u) % 8u));
}