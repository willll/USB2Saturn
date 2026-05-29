#ifndef SATURN_MOUSE_PROTOCOL_H
#define SATURN_MOUSE_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SATURN_MOUSE_PROTOCOL_ID 0xE3u

typedef struct {
    int16_t delta_x;
    int16_t delta_y;
    bool left;
    bool right;
    bool middle;
    bool start;
    bool x_over;
    bool y_over;
    uint8_t last_select_state;
    uint8_t nibble_index;
} saturn_mouse_protocol_t;

void saturn_mouse_protocol_reset(saturn_mouse_protocol_t* protocol);
void saturn_mouse_protocol_reset_poll_state(saturn_mouse_protocol_t* protocol);

void saturn_mouse_protocol_set_buttons(saturn_mouse_protocol_t* protocol,
                                       bool left,
                                       bool right,
                                       bool middle,
                                       bool start);

void saturn_mouse_protocol_add_motion(saturn_mouse_protocol_t* protocol, int16_t delta_x, int16_t delta_y);

// Returns the current 4-bit nibble placed on D3..D0 according to select-line transitions.
uint8_t saturn_mouse_protocol_next_nibble(saturn_mouse_protocol_t* protocol,
                                          uint8_t s0,
                                          uint8_t s1);

#ifdef __cplusplus
}
#endif

#endif // SATURN_MOUSE_PROTOCOL_H