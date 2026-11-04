/*
 * motors.h - L298N dual H-bridge driver (two DC gearmotors).
 *
 * Speeds are signed permille: +1000 = full forward, -1000 = full reverse,
 * 0 = stopped (braked). Changes are ramped in a 1 ms interrupt, and a motor
 * is always brought to a stop for MOTOR_REVERSE_DWELL_MS before it reverses
 * (the GHM-01 datasheet says the gearbox must not be reversed instantly).
 */
#ifndef MOTORS_H
#define MOTORS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    MOTOR_LEFT = 0,    /* L298N channel A: ENA, IN1, IN2, OUT1/OUT2 */
    MOTOR_RIGHT = 1,   /* L298N channel B: ENB, IN3, IN4, OUT3/OUT4 */
} motor_id_t;

void    motors_init(void);
void    motors_set(int16_t left_permille, int16_t right_permille); /* ramped */
void    motors_stop(void);                                         /* ramp to 0, then brake */
bool    motors_settled(void);           /* true once both motors have reached their targets */
int16_t motors_current(motor_id_t id);  /* speed actually being applied right now */
void    motors_tick_1ms(void);          /* call every 1 ms (SysTick does this) */
void    motors_emergency_off(void);     /* immediately force all driver inputs low */

#endif /* MOTORS_H */
