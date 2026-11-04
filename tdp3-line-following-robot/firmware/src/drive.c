/*
 * drive.c - open-loop (timed) moves.
 *
 * The ramp-up at the start of a move and the ramp-down at the end are the
 * same shape, so the distance lost while speeding up is regained while
 * slowing down: distance ~= speed x time, as long as the move is longer than
 * the ramp (250 ms at the default settings).
 */
#include "drive.h"
#include "board.h"
#include "config.h"
#include "motors.h"

#define STOP_TIMEOUT_MS 3000u

uint32_t drive_mm_to_ms(int32_t mm)
{
    uint32_t a = (uint32_t)(mm < 0 ? -mm : mm);
    return (a * 1000u + (uint32_t)CAL_MM_PER_S / 2u) / (uint32_t)CAL_MM_PER_S;
}

uint32_t drive_deg_to_ms(int32_t deg)
{
    uint32_t a = (uint32_t)(deg < 0 ? -deg : deg);
    return (a * 1000u + (uint32_t)CAL_DEG_PER_S / 2u) / (uint32_t)CAL_DEG_PER_S;
}

void drive_stop(void)
{
    motors_stop();
    uint32_t start = millis();
    while (!motors_settled() && (uint32_t)(millis() - start) < STOP_TIMEOUT_MS) {
        delay_ms(1u);
    }
}

void drive_raw(int16_t left, int16_t right, uint32_t ms)
{
    motors_set(left, right);
    delay_ms(ms);
    drive_stop();
    delay_ms(SETTLE_MS);
}

void drive_forward_mm(int32_t mm)
{
    int16_t speed = (mm >= 0) ? CRUISE_SPEED : -CRUISE_SPEED;
    drive_raw(speed, speed, drive_mm_to_ms(mm));
}

void drive_turn_deg(int32_t deg)
{
    /* Spin on the spot: wheels in opposite directions. */
    int16_t s = (deg >= 0) ? TURN_SPEED : -TURN_SPEED;
    drive_raw(s, (int16_t)-s, drive_deg_to_ms(deg));
}
