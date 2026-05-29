#ifndef SATURN_OUT_H
#define SATURN_OUT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Sega Saturn Gamepad State
// 1 means pressed, 0 means unpressed
typedef struct {
    bool up;
    bool down;
    bool left;
    bool right;
    bool a;
    bool b;
    bool c;
    bool x;
    bool y;
    bool z;
    bool l;
    bool r;
    bool start;
} saturn_gamepad_state_t;

// Global state accessed by the output protocol
extern volatile saturn_gamepad_state_t g_saturn_state;

// Initialize the GPIO pins for the Saturn protocol
void saturn_out_init(void);

#ifdef __cplusplus
}
#endif

#endif // SATURN_OUT_H
