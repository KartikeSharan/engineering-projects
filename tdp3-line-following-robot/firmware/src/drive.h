/*
 * drive.h - robot-level moves built on the motor driver.
 *
 * Every move starts and ends with the robot stopped, so moves are repeatable
 * and the calibration (config.h) applies to each one.
 */
#ifndef DRIVE_H
#define DRIVE_H

#include <stdint.h>

uint32_t drive_mm_to_ms(int32_t mm);    /* straight-line time at CRUISE_SPEED */
uint32_t drive_deg_to_ms(int32_t deg);  /* spin time at TURN_SPEED */

void drive_forward_mm(int32_t mm);      /* negative = backwards */
void drive_turn_deg(int32_t deg);       /* positive = right (clockwise from above) */
void drive_raw(int16_t left, int16_t right, uint32_t ms);  /* any speeds, for curves */
void drive_stop(void);                  /* ramp down and wait until stopped */

#endif /* DRIVE_H */
