/*
 * path.h - a path is a list of steps, written with these macros:
 *
 *   FORWARD(mm)            drive straight forwards
 *   BACKWARD(mm)           drive straight backwards
 *   TURN_LEFT(deg)         spin on the spot, anticlockwise
 *   TURN_RIGHT(deg)        spin on the spot, clockwise
 *   WAIT(ms)               stand still
 *   DRIVE(left, right, ms) set each wheel's speed (permille) for a time,
 *                          e.g. DRIVE(600, 300, 1500) curves gently right
 */
#ifndef PATH_H
#define PATH_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    STEP_FORWARD,
    STEP_BACKWARD,
    STEP_TURN_LEFT,
    STEP_TURN_RIGHT,
    STEP_WAIT,
    STEP_DRIVE,
} step_kind_t;

typedef struct {
    step_kind_t kind;
    int32_t     amount;   /* mm, degrees or ms depending on kind */
    int16_t     left;     /* STEP_DRIVE only */
    int16_t     right;    /* STEP_DRIVE only */
} path_step_t;

#define FORWARD(mm)        { STEP_FORWARD,    (mm),  0, 0 }
#define BACKWARD(mm)       { STEP_BACKWARD,   (mm),  0, 0 }
#define TURN_LEFT(deg)     { STEP_TURN_LEFT,  (deg), 0, 0 }
#define TURN_RIGHT(deg)    { STEP_TURN_RIGHT, (deg), 0, 0 }
#define WAIT(ms)           { STEP_WAIT,       (ms),  0, 0 }
#define DRIVE(l, r, ms)    { STEP_DRIVE,      (ms),  (l), (r) }

extern const path_step_t g_demo_path[];
extern const size_t      g_demo_path_len;

void path_run(const path_step_t *steps, size_t count);

#endif /* PATH_H */
