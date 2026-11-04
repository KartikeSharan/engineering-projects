/*
 * config.h - the numbers you will actually tune.
 *
 * Speeds are in "permille" of full PWM: 1000 = 100 %, 600 = 60 %.
 */
#ifndef CONFIG_H
#define CONFIG_H

/* ---- What the robot does when it powers up ----------------------------
 * You can also choose this without editing the file:
 *     python tdp.py flash --build --mode calibrate
 */
#define MODE_DEMO_PATH   0   /* drive the path in path.c (the interim demo) */
#define MODE_CALIBRATE   1   /* drive straight 2 s, then spin 2 s, so you can measure */
#define MODE_MOTOR_TEST  2   /* each wheel forwards/backwards in turn (wheels off the floor!) */

#ifndef RUN_MODE
#define RUN_MODE MODE_DEMO_PATH
#endif

/* ---- Timing ------------------------------------------------------------ */
#define START_DELAY_MS        3000u  /* after reset, red LED blinks, then it goes */
#define SETTLE_MS              250u  /* stopped pause between path steps */

/* ---- Speeds (permille) ------------------------------------------------- */
#define CRUISE_SPEED           600   /* straight lines */
#define TURN_SPEED             450   /* spinning on the spot */

/* ---- Calibration: measure these with RUN_MODE = MODE_CALIBRATE ----------
 * CAL_MM_PER_S  = distance travelled in the 2 s straight run (mm) / 2
 * CAL_DEG_PER_S = angle turned in the 2 s spin (degrees) / 2
 * The values below are placeholders until you measure the real robot.
 * Re-check them when the battery voltage changes a lot: open-loop timing
 * drifts as the battery runs down.
 */
#define CAL_MM_PER_S           300   /* at CRUISE_SPEED */
#define CAL_DEG_PER_S          180   /* at TURN_SPEED */
#define CAL_RUN_MS            2000u  /* length of each calibration run */

/* ---- Straight-line trim ----------------------------------------------------
 * Two motors never match exactly. If the robot curves LEFT the right wheel is
 * faster, so lower RIGHT_TRIM_PCT a few percent (e.g. 100 -> 96), and vice versa.
 */
#define LEFT_TRIM_PCT          100
#define RIGHT_TRIM_PCT         100

/* ---- Motor direction ---------------------------------------------------------
 * If a wheel spins the wrong way in MODE_MOTOR_TEST, set its INVERT to 1
 * instead of swapping the motor wires.
 */
#define LEFT_INVERT              0
#define RIGHT_INVERT             0

/* ---- Motor driver behaviour ------------------------------------------------ */
#define MOTOR_PWM_HZ         10000u  /* PWM on ENA/ENB. 1k-20k is fine for the L298 */
#define MOTOR_RAMP_PER_MS        4   /* permille per ms: 0 -> 100 % in 250 ms */
#define MOTOR_REVERSE_DWELL_MS 100u  /* the GHM-01 gearmotor must stop before reversing */

/* ---- Debug serial (USB virtual COM port via OpenSDA) ----------------------- */
#define DEBUG_BAUD          115200u

#endif /* CONFIG_H */
