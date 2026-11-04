/*
 * main.c - TDP3 Team 7 robot firmware, FRDM-KL25Z.
 *
 * Interim demo: drive a fixed, pre-programmed path (no line following).
 *
 * LED meaning:
 *   red blinking    countdown after reset (put the robot down!)
 *   green           driving
 *   blue            finished - press RESET to run again
 *   yellow          calibration run: measuring pause
 *   fast red        crash (HardFault) - motors are switched off
 */
#include <stdbool.h>
#include "board.h"
#include "config.h"
#include "debug_uart.h"
#include "drive.h"
#include "motors.h"
#include "path.h"
#include "system_mkl25z4.h"

static const char *mode_name(int mode)
{
    switch (mode) {
    case MODE_DEMO_PATH:  return "DEMO PATH";
    case MODE_CALIBRATE:  return "CALIBRATE";
    case MODE_MOTOR_TEST: return "MOTOR TEST";
    default:              return "UNKNOWN";
    }
}

static void countdown(uint32_t ms)
{
    debug_printf("Starting in %u ms...\n", (unsigned)ms);
    uint32_t start = millis();
    while ((uint32_t)(millis() - start) < ms) {
        uint32_t t = millis() - start;
        led_set(((t / 250u) % 2u) == 0u ? LED_RED : LED_OFF);
        delay_ms(10u);
    }
}

static void run_calibration(void)
{
    debug_printf("\nCALIBRATION\n");
    debug_printf("1) Straight run: %u ms at CRUISE_SPEED=%d\n", (unsigned)CAL_RUN_MS, CRUISE_SPEED);
    led_set(LED_GREEN);
    drive_raw(CRUISE_SPEED, CRUISE_SPEED, CAL_RUN_MS);
    led_set(LED_YELLOW);
    debug_printf("   Measure the distance now. CAL_MM_PER_S = distance_mm * 1000 / %u\n", (unsigned)CAL_RUN_MS);
    debug_printf("   If it curved left, lower RIGHT_TRIM_PCT; if it curved right, lower LEFT_TRIM_PCT.\n");
    delay_ms(5000u);

    debug_printf("2) Spin right: %u ms at TURN_SPEED=%d\n", (unsigned)CAL_RUN_MS, TURN_SPEED);
    led_set(LED_GREEN);
    drive_raw(TURN_SPEED, (int16_t)-TURN_SPEED, CAL_RUN_MS);
    debug_printf("   Measure the angle turned. CAL_DEG_PER_S = angle_deg * 1000 / %u\n", (unsigned)CAL_RUN_MS);
}

static void run_motor_test(void)
{
    static const struct {
        const char *text;
        int16_t left;
        int16_t right;
        led_colour_t led;
    } tests[] = {
        { "LEFT wheel FORWARD",   500,    0, LED_GREEN },
        { "LEFT wheel BACKWARD", -500,    0, LED_MAGENTA },
        { "RIGHT wheel FORWARD",    0,  500, LED_CYAN },
        { "RIGHT wheel BACKWARD",   0, -500, LED_BLUE },
    };

    debug_printf("\nMOTOR TEST - lift the wheels off the floor.\n");
    debug_printf("'FORWARD' means the wheel turns the way that drives the robot forwards.\n");
    debug_printf("If a wheel goes the wrong way, set LEFT_INVERT / RIGHT_INVERT in config.h.\n");
    debug_printf("If nothing moves: check ENA/ENB jumpers removed, 12 V on, common GND.\n\n");

    for (unsigned i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        debug_printf("%s\n", tests[i].text);
        led_set(tests[i].led);
        drive_raw(tests[i].left, tests[i].right, 1500u);
        led_set(LED_OFF);
        delay_ms(1000u);
    }
}

int main(void)
{
    board_init();
    debug_init(DEBUG_BAUD);
    motors_init();
    board_set_tick_callback(motors_tick_1ms);

    debug_printf("\n\n=== TDP3 Team 7 - FRDM-KL25Z ===\n");
    debug_printf("Mode: %s\n", mode_name(RUN_MODE));
    if (g_clock_fallback) {
        debug_printf("WARNING: 8 MHz crystal did not start, using internal clock (%u Hz)\n",
                     (unsigned)SystemCoreClock);
    } else {
        debug_printf("Clock: %u Hz (crystal + PLL)\n", (unsigned)SystemCoreClock);
    }
    debug_printf("Cruise %d, turn %d, cal %d mm/s, %d deg/s, trim L%d%% R%d%%\n",
                 CRUISE_SPEED, TURN_SPEED, CAL_MM_PER_S, CAL_DEG_PER_S,
                 LEFT_TRIM_PCT, RIGHT_TRIM_PCT);

    countdown(START_DELAY_MS);
    led_set(LED_GREEN);
    uint32_t t0 = millis();

    if (RUN_MODE == MODE_CALIBRATE) {
        run_calibration();
    } else if (RUN_MODE == MODE_MOTOR_TEST) {
        run_motor_test();
    } else {
        path_run(g_demo_path, g_demo_path_len);
    }

    drive_stop();
    led_set(LED_BLUE);
    debug_printf("Done in %u ms. Press RESET to run again.\n", (unsigned)(millis() - t0));

    for (;;) {
        delay_ms(1000u);   /* motors stay braked; nothing else to do */
    }
}

/* Any crash: cut the motors first, then flash the red LED fast forever. */
void HardFault_Handler(void)
{
    motors_emergency_off();
    for (;;) {
        led_set(LED_RED);
        for (volatile uint32_t i = 0; i < 300000u; i++) {
        }
        led_set(LED_OFF);
        for (volatile uint32_t i = 0; i < 300000u; i++) {
        }
    }
}
