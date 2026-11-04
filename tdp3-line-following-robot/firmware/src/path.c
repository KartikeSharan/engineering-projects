/*
 * path.c - THE INTERIM DEMO ROUTE. Edit g_demo_path to match the course.
 *
 * The default is a 1 m x 0.5 m rectangle that finishes where it started,
 * which makes it easy to see how good the calibration is.
 * Calibrate first (see config.h), otherwise distances and angles are guesses.
 */
#include "path.h"
#include "board.h"
#include "debug_uart.h"
#include "drive.h"

const path_step_t g_demo_path[] = {
    FORWARD(1000),
    TURN_RIGHT(90),
    FORWARD(500),
    TURN_RIGHT(90),
    FORWARD(1000),
    TURN_RIGHT(90),
    FORWARD(500),
    TURN_RIGHT(90),
};

const size_t g_demo_path_len = sizeof(g_demo_path) / sizeof(g_demo_path[0]);

static void print_time(void)
{
    uint32_t ms = millis();
    debug_printf("[%4u.%03u s] ", (unsigned)(ms / 1000u), (unsigned)(ms % 1000u));
}

void path_run(const path_step_t *steps, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        const path_step_t *s = &steps[i];
        print_time();
        debug_printf("step %u/%u: ", (unsigned)(i + 1u), (unsigned)count);

        switch (s->kind) {
        case STEP_FORWARD:
            debug_printf("FORWARD %d mm (%u ms)\n", (int)s->amount, (unsigned)drive_mm_to_ms(s->amount));
            drive_forward_mm(s->amount);
            break;
        case STEP_BACKWARD:
            debug_printf("BACKWARD %d mm (%u ms)\n", (int)s->amount, (unsigned)drive_mm_to_ms(s->amount));
            drive_forward_mm(-s->amount);
            break;
        case STEP_TURN_LEFT:
            debug_printf("TURN_LEFT %d deg (%u ms)\n", (int)s->amount, (unsigned)drive_deg_to_ms(s->amount));
            drive_turn_deg(-s->amount);
            break;
        case STEP_TURN_RIGHT:
            debug_printf("TURN_RIGHT %d deg (%u ms)\n", (int)s->amount, (unsigned)drive_deg_to_ms(s->amount));
            drive_turn_deg(s->amount);
            break;
        case STEP_WAIT:
            debug_printf("WAIT %d ms\n", (int)s->amount);
            delay_ms((uint32_t)s->amount);
            break;
        case STEP_DRIVE:
            debug_printf("DRIVE left=%d right=%d for %d ms\n", (int)s->left, (int)s->right, (int)s->amount);
            drive_raw(s->left, s->right, (uint32_t)s->amount);
            break;
        default:
            debug_printf("unknown step, skipped\n");
            break;
        }
    }
}
