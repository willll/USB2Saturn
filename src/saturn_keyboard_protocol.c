#include "saturn_keyboard_protocol.h"

#include <string.h>

static bool queue_is_empty(saturn_keyboard_protocol_t const* protocol) {
    return protocol->head == protocol->tail;
}

static bool queue_is_full(saturn_keyboard_protocol_t const* protocol) {
    return (uint8_t)((protocol->tail + 1u) % SATURN_KEYBOARD_QUEUE_SIZE) == protocol->head;
}

static bool queue_push(saturn_keyboard_protocol_t* protocol, uint8_t scancode, bool is_break) {
    if (queue_is_full(protocol)) {
        return false;
    }

    protocol->queue[protocol->tail].scancode = scancode;
    protocol->queue[protocol->tail].is_break = is_break;
    protocol->tail = (uint8_t)((protocol->tail + 1u) % SATURN_KEYBOARD_QUEUE_SIZE);
    return true;
}

static saturn_keyboard_event_t queue_current(saturn_keyboard_protocol_t const* protocol) {
    saturn_keyboard_event_t ev = {0};
    if (!queue_is_empty(protocol)) {
        ev = protocol->queue[protocol->head];
    }
    return ev;
}

static void queue_pop(saturn_keyboard_protocol_t* protocol) {
    if (!queue_is_empty(protocol)) {
        protocol->head = (uint8_t)((protocol->head + 1u) % SATURN_KEYBOARD_QUEUE_SIZE);
    }
}

static uint8_t lock_nibble(saturn_keyboard_protocol_t const* protocol) {
    uint8_t n = 0u;
    if (protocol->caps_lock) {
        n |= 0x04u;
    }
    if (protocol->num_lock) {
        n |= 0x02u;
    }
    if (protocol->scroll_lock) {
        n |= 0x01u;
    }
    return n;
}

static uint8_t status_low_nibble(saturn_keyboard_protocol_t const* protocol) {
    if (queue_is_empty(protocol)) {
        return 0x06u;
    }

    return queue_current(protocol).is_break ? 0x07u : 0x0Eu;
}

static uint8_t packet_nibble(saturn_keyboard_protocol_t const* protocol, uint16_t game_key, uint8_t nibble_index) {
    saturn_keyboard_event_t ev = queue_current(protocol);
    uint8_t data1 = (uint8_t)(game_key >> 8);
    uint8_t data2 = (uint8_t)((game_key & 0xF8u) | 0x00u); // KB type bits must be 000.
    uint8_t data3 = (uint8_t)((lock_nibble(protocol) << 4) | status_low_nibble(protocol));
    uint8_t data4 = ev.scancode;

    switch (nibble_index % 12u) {
    case 0: return (SATURN_KEYBOARD_PROTOCOL_ID >> 4) & 0x0Fu;
    case 1: return SATURN_KEYBOARD_PROTOCOL_ID & 0x0Fu;
    case 2: return (data1 >> 4) & 0x0Fu;
    case 3: return data1 & 0x0Fu;
    case 4: return (data2 >> 4) & 0x0Fu;
    case 5: return data2 & 0x0Fu;
    case 6: return (data3 >> 4) & 0x0Fu;
    case 7: return data3 & 0x0Fu;
    case 8: return (data4 >> 4) & 0x0Fu;
    case 9: return data4 & 0x0Fu;
    case 10: return 0x0u;
    default: return 0x1u;
    }
}

void saturn_keyboard_protocol_reset(saturn_keyboard_protocol_t* protocol) {
    if (!protocol) {
        return;
    }

    memset(protocol, 0, sizeof(*protocol));
    protocol->last_select_state = 0xFFu;
}

void saturn_keyboard_protocol_reset_poll_state(saturn_keyboard_protocol_t* protocol) {
    if (!protocol) {
        return;
    }

    protocol->last_select_state = 0xFFu;
    protocol->nibble_index = 0u;
}

void saturn_keyboard_protocol_set_locks(saturn_keyboard_protocol_t* protocol,
                                        bool caps_lock,
                                        bool num_lock,
                                        bool scroll_lock) {
    if (!protocol) {
        return;
    }

    protocol->caps_lock = caps_lock;
    protocol->num_lock = num_lock;
    protocol->scroll_lock = scroll_lock;
}

bool saturn_keyboard_protocol_push_make(saturn_keyboard_protocol_t* protocol, uint8_t scancode) {
    if (!protocol) {
        return false;
    }

    protocol->last_scancode = scancode;
    return queue_push(protocol, scancode, false);
}

bool saturn_keyboard_protocol_push_break(saturn_keyboard_protocol_t* protocol, uint8_t scancode) {
    if (!protocol) {
        return false;
    }

    protocol->last_scancode = scancode;
    return queue_push(protocol, scancode, true);
}

void saturn_keyboard_protocol_release_last(saturn_keyboard_protocol_t* protocol) {
    if (!protocol || protocol->last_scancode == 0u) {
        return;
    }

    (void)queue_push(protocol, protocol->last_scancode, true);
}

uint8_t saturn_keyboard_protocol_next_nibble(saturn_keyboard_protocol_t* protocol,
                                             uint16_t game_key,
                                             uint8_t s0,
                                             uint8_t s1) {
    uint8_t state;
    uint8_t nibble;

    if (!protocol) {
        return 0u;
    }

    state = (uint8_t)((s1 << 1) | s0);

    if (state != protocol->last_select_state) {
        nibble = packet_nibble(protocol, game_key, protocol->nibble_index % 12u);
        if ((protocol->nibble_index % 12u) == 11u) {
            queue_pop(protocol);
        }
        protocol->nibble_index++;
        protocol->last_select_state = state;
        return nibble;
    }

    if (protocol->nibble_index == 0u) {
        return packet_nibble(protocol, game_key, 0u);
    }

    return packet_nibble(protocol, game_key, (uint8_t)((protocol->nibble_index - 1u) % 12u));
}
