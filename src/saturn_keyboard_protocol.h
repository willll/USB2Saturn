#ifndef SATURN_KEYBOARD_PROTOCOL_H
#define SATURN_KEYBOARD_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SATURN_KEYBOARD_PROTOCOL_ID 0x34u
#define SATURN_KEYBOARD_QUEUE_SIZE 16u

typedef struct {
    uint8_t scancode;
    bool is_break;
} saturn_keyboard_event_t;

typedef struct {
    saturn_keyboard_event_t queue[SATURN_KEYBOARD_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t last_scancode;
    bool caps_lock;
    bool num_lock;
    bool scroll_lock;
    uint8_t last_select_state;
    uint8_t nibble_index;
} saturn_keyboard_protocol_t;

void saturn_keyboard_protocol_reset(saturn_keyboard_protocol_t* protocol);
void saturn_keyboard_protocol_reset_poll_state(saturn_keyboard_protocol_t* protocol);

void saturn_keyboard_protocol_set_locks(saturn_keyboard_protocol_t* protocol,
                                        bool caps_lock,
                                        bool num_lock,
                                        bool scroll_lock);

bool saturn_keyboard_protocol_push_make(saturn_keyboard_protocol_t* protocol, uint8_t scancode);
bool saturn_keyboard_protocol_push_break(saturn_keyboard_protocol_t* protocol, uint8_t scancode);
void saturn_keyboard_protocol_release_last(saturn_keyboard_protocol_t* protocol);

// Returns the current 4-bit nibble placed on D3..D0 according to select-line transitions.
uint8_t saturn_keyboard_protocol_next_nibble(saturn_keyboard_protocol_t* protocol,
                                             uint16_t game_key,
                                             uint8_t s0,
                                             uint8_t s1);

#ifdef __cplusplus
}
#endif

#endif // SATURN_KEYBOARD_PROTOCOL_H
