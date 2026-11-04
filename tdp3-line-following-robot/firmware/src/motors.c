/*
 * motors.c - L298N driver.
 *
 * L298 truth table (per channel):
 *   EN   IN_a  IN_b   result
 *   PWM  1     0      forward, speed = duty
 *   PWM  0     1      reverse, speed = duty
 *   1    0     0      brake (fast motor stop)
 *   0    x     x      coast (free running)
 *
 * The enable pins are driven by TPM0 in edge-aligned PWM mode; the IN pins
 * are plain GPIO.
 */
#include "motors.h"
#include "config.h"
#include "gpio.h"
#include "mkl25z4.h"
#include "pins.h"
#include "system_mkl25z4.h"

typedef struct {
    pin_t   in_a;        /* HIGH for forward */
    pin_t   in_b;        /* HIGH for reverse */
    pin_t   en;          /* PWM enable pin */
    uint8_t en_mux;      /* ALT function that connects en to the TPM */
    uint8_t tpm_ch;
    bool    invert;
    int16_t trim_pct;
} motor_hw_t;

typedef struct {
    volatile int16_t  target;    /* set by motors_set() */
    volatile int16_t  current;   /* updated by motors_tick_1ms() */
    volatile uint16_t dwell_ms;  /* time left stopped before reversing */
} motor_state_t;

static const motor_hw_t s_hw[2] = {
    [MOTOR_LEFT] = {
        .in_a = PIN_IN1, .in_b = PIN_IN2, .en = PIN_ENA,
        .en_mux = PIN_ENA_MUX, .tpm_ch = PIN_ENA_TPM_CH,
        .invert = (LEFT_INVERT != 0), .trim_pct = LEFT_TRIM_PCT,
    },
    [MOTOR_RIGHT] = {
        .in_a = PIN_IN3, .in_b = PIN_IN4, .en = PIN_ENB,
        .en_mux = PIN_ENB_MUX, .tpm_ch = PIN_ENB_TPM_CH,
        .invert = (RIGHT_INVERT != 0), .trim_pct = RIGHT_TRIM_PCT,
    },
};

static motor_state_t s_motor[2];
static uint32_t s_pwm_period;   /* TPM counts per PWM period (= MOD + 1) */

static void set_duty_counts(uint8_t tpm_ch, uint32_t counts)
{
    if (counts > 0xFFFFu) {
        counts = 0xFFFFu;   /* CnV is 16 bits */
    }
    MOTOR_TPM->CH[tpm_ch].CnV = counts;
}

static int16_t clamp_permille(int32_t v)
{
    if (v > 1000) {
        return 1000;
    }
    if (v < -1000) {
        return -1000;
    }
    return (int16_t)v;
}

/* Drive the hardware for one motor at the given signed speed. */
static void apply(motor_id_t id, int16_t permille)
{
    const motor_hw_t *hw = &s_hw[id];
    int32_t v = hw->invert ? -(int32_t)permille : (int32_t)permille;

    if (v == 0) {
        /* Brake: both inputs low, enable fully on. */
        pin_write(&hw->in_a, false);
        pin_write(&hw->in_b, false);
        set_duty_counts(hw->tpm_ch, s_pwm_period);   /* CnV > MOD gives 100 % */
        return;
    }

    uint32_t mag = (uint32_t)(v > 0 ? v : -v);
    mag = (mag * (uint32_t)hw->trim_pct) / 100u;
    if (mag > 1000u) {
        mag = 1000u;
    }
    set_duty_counts(hw->tpm_ch, (mag * s_pwm_period) / 1000u);
    pin_write(&hw->in_a, v > 0);
    pin_write(&hw->in_b, v < 0);
}

/* One 1 ms ramp step. Returns true if the applied speed changed. */
static bool ramp_step(motor_state_t *m)
{
    if (m->dwell_ms > 0u) {
        m->dwell_ms--;
        return false;
    }

    int16_t target = m->target;
    int16_t cur = m->current;
    if (cur == target) {
        return false;
    }

    /* Reversing: ramp down to zero first. */
    int16_t goal = target;
    if ((cur > 0 && target < 0) || (cur < 0 && target > 0)) {
        goal = 0;
    }

    int32_t next = cur;
    if (next < goal) {
        next += MOTOR_RAMP_PER_MS;
        if (next > goal) {
            next = goal;
        }
    } else {
        next -= MOTOR_RAMP_PER_MS;
        if (next < goal) {
            next = goal;
        }
    }

    if (next == 0 && cur != 0 && target != 0) {
        /* Just stopped on the way to reversing: hold still for a moment. */
        m->dwell_ms = (uint16_t)MOTOR_REVERSE_DWELL_MS;
    }
    m->current = (int16_t)next;
    return true;
}

void motors_tick_1ms(void)
{
    for (int i = 0; i < 2; i++) {
        if (ramp_step(&s_motor[i])) {
            apply((motor_id_t)i, s_motor[i].current);
        }
    }
}

void motors_init(void)
{
    /* Direction pins: outputs, low. */
    for (int i = 0; i < 2; i++) {
        pin_make_output(&s_hw[i].in_a, false);
        pin_make_output(&s_hw[i].in_b, false);
        s_motor[i].target = 0;
        s_motor[i].current = 0;
        s_motor[i].dwell_ms = 0u;
    }

    /* PWM timer: choose the smallest prescaler that fits the 16-bit counter. */
    SIM_SCGC6 |= MOTOR_TPM_SCGC6_BIT;
    uint32_t ps = 0u;
    uint32_t counts = PeripheralClock / MOTOR_PWM_HZ;
    while (counts > 65536u && ps < 7u) {
        ps++;
        counts = (PeripheralClock >> ps) / MOTOR_PWM_HZ;
    }
    if (counts > 65536u) {
        counts = 65536u;
    }
    if (counts < 2u) {
        counts = 2u;
    }
    s_pwm_period = counts;

    MOTOR_TPM->SC = 0u;                    /* counter off while configuring */
    MOTOR_TPM->CNT = 0u;
    MOTOR_TPM->MOD = counts - 1u;
    for (int i = 0; i < 2; i++) {
        MOTOR_TPM->CH[s_hw[i].tpm_ch].CnSC = TPM_CnSC_MSB | TPM_CnSC_ELSB;  /* edge PWM, high-true */
        MOTOR_TPM->CH[s_hw[i].tpm_ch].CnV = 0u;                             /* 0 % = coast */
    }
    MOTOR_TPM->SC = TPM_SC_CMOD(1) | TPM_SC_PS(ps);

    /* Connect the enable pins to the timer, then hold both motors braked. */
    for (int i = 0; i < 2; i++) {
        pin_set_mux(&s_hw[i].en, s_hw[i].en_mux);
        apply((motor_id_t)i, 0);
    }
}

void motors_set(int16_t left_permille, int16_t right_permille)
{
    int16_t l = clamp_permille(left_permille);
    int16_t r = clamp_permille(right_permille);
    irq_disable();          /* so both motors start on the same tick */
    s_motor[MOTOR_LEFT].target = l;
    s_motor[MOTOR_RIGHT].target = r;
    irq_enable();
}

void motors_stop(void)
{
    motors_set(0, 0);
}

bool motors_settled(void)
{
    for (int i = 0; i < 2; i++) {
        if (s_motor[i].current != s_motor[i].target || s_motor[i].dwell_ms != 0u) {
            return false;
        }
    }
    return true;
}

int16_t motors_current(motor_id_t id)
{
    return s_motor[id].current;
}

void motors_emergency_off(void)
{
    for (int i = 0; i < 2; i++) {
        pin_make_output(&s_hw[i].en, false);   /* take EN away from the timer: low = coast */
        pin_write(&s_hw[i].in_a, false);
        pin_write(&s_hw[i].in_b, false);
    }
}
