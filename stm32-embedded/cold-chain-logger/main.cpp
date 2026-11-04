/*
    main.cpp
    Cold-Chain Transport Condition Logger
    UESTC 2004 Embedded Processors - University of Glasgow

    Monitors temperature and handling conditions during transport
    using a TMP102 temperature sensor and an ADXL345 accelerometer.
    Events of interest are logged and can be reported over serial
    when the system is inspected.

    Hardware connections:
        TMP102  - SDA: D4, SCL: D5 (I2C)
        ADXL345 - MOSI: D11, MISO: D12, SCLK: D13, CS: D10 (SPI)
        Green LED  - D3  (on  = conditions OK / system ready)
        Red LED    - D2  (on  = breach detected)
        Buzzer     - D9  (beep = a breach event occurred, PWM)
        Button     - A0  (press to start/stop transport, prints report)

    Temperature limits are 2-8 degrees C, based on the WHO cold-chain
    standard for vaccines and pharmaceutical products.

    The shock threshold is 2.0g of vector magnitude. Normal handling
    and vehicle vibration stays below about 0.5g, so 2g indicates a
    drop or impact.

    Power-aware design: the system only samples the sensors while in
    the TRANSPORT or ALERT states, sleeps between samples using the
    RTOS sleep call rather than busy-waiting, and uses an interrupt
    for the button instead of polling it continuously.

    Note on printf: the Mbed minimal printf does not support %f, so
    all floating-point values are split into integer parts before
    printing (for example 24 and 25 to show 24.25).
*/

#include "mbed.h"
#include "tmp102.h"
#include "adxl345.h"
#include "event_log.h"

// Temperature limits in degrees C (WHO cold-chain standard)
#define TEMP_UPPER_LIMIT   30.0f
#define TEMP_LOWER_LIMIT   20.0f

// Time between sensor samples during transport (milliseconds)
#define SAMPLE_INTERVAL_MS  2000

// Number of OK readings in a row needed before clearing an alert.
// Stops the system bouncing out of ALERT on a single good reading.
#define READINGS_TO_CLEAR_ALERT  3

// Output peripheral pins
#define GREEN_LED_PIN   D3
#define RED_LED_PIN     D2
#define BUZZER_PIN      D9
#define BUTTON_PIN      A0

// Peripheral objects
DigitalOut  green_led(GREEN_LED_PIN);
DigitalOut  red_led(RED_LED_PIN);
PwmOut      buzzer(BUZZER_PIN);          // PWM so it works with passive buzzers too
InterruptIn mode_button(BUTTON_PIN);

// Current operating state. volatile because the ISR can change it.
volatile SystemState_t current_state = STATE_IDLE;

// Set by the button ISR; handled in the main loop.
volatile bool button_pressed = false;

// Counts consecutive in-range readings (used to clear an alert).
int consecutive_ok_readings = 0;

/*
    updateIndicators
    Drives the LEDs and buzzer to match the current situation. Called
    after each sensor check and whenever the state changes.
*/
void updateIndicators(bool temp_breach, bool shock_detected)
{
    if (current_state == STATE_IDLE) {
        // Idle: green on so the user knows the board is powered
        green_led = 1;
        red_led   = 0;
        buzzer    = 0.0f;          // PWM off

    } else if (temp_breach || shock_detected || current_state == STATE_ALERT) {
        // A breach is active: red on with a short buzzer beep
        green_led = 0;
        red_led   = 1;

        // Generate a 2kHz tone for the beep. Setting the PWM period to
        // 500us gives 2kHz, and a 50% duty cycle drives the buzzer.
        // This works for both active and passive buzzers.
        buzzer.period_us(500);     // 2 kHz tone
        buzzer.write(0.5f);        // 50% duty cycle = on
        thread_sleep_for(150);     // beep length
        buzzer.write(0.0f);        // tone off

    } else {
        // Transport running normally: green on, no alarm
        green_led = 1;
        red_led   = 0;
        buzzer    = 0.0f;          // PWM off
    }
}

/*
    onButtonPress (ISR)
    Runs on a button press. Only sets a flag because printf and sleep
    are not safe to call inside an interrupt handler.
*/
void onButtonPress(void)
{
    button_pressed = true;
}

/*
    handleButton
    Processes a button press in the main loop. The first press starts
    transport monitoring; the next press ends it, prints the full log,
    and returns the system to idle.
*/
void handleButton(void)
{
    button_pressed = false;

    if (current_state == STATE_IDLE) {
        // Start monitoring
        current_state = STATE_TRANSPORT;
        consecutive_ok_readings = 0;
        EventLog_Record(EVENT_STATE_CHANGE, 0.0f, current_state);
        printf("\n[SYSTEM] Transport started. Now monitoring...\n");

    } else {
        // End monitoring: print the report then return to idle
        printf("\n[SYSTEM] Transport ended. Printing report...\n");
        EventLog_PrintAll();
        current_state = STATE_IDLE;
        EventLog_Record(EVENT_STATE_CHANGE, 0.0f, current_state);
        printf("[SYSTEM] Back to idle. Press button to start again.\n");
        updateIndicators(false, false);
    }
}

/*
    checkTemperature
    Reads the TMP102, compares it against the limits, and logs any
    breach. Returns true if the temperature is currently out of range.
*/
bool checkTemperature(void)
{
    TMP102_Wakeup();                          // wake sensor for one reading
    float temp = TMP102_ReadTemperature();    // take the reading
    TMP102_Shutdown();                        // shut down to save power until next sample

    printf("[TEMP] %d.%02d C", (int)temp, abs((int)(temp * 100)) % 100);

    if (temp > TEMP_UPPER_LIMIT) {
        printf(" - TOO HIGH (limit: %d.%01d C)\n",
               (int)TEMP_UPPER_LIMIT, abs((int)(TEMP_UPPER_LIMIT * 10)) % 10);
        EventLog_Record(EVENT_TEMP_HIGH, temp, current_state);
        consecutive_ok_readings = 0;
        if (current_state == STATE_TRANSPORT) {
            current_state = STATE_ALERT;
        }
        return true;

    } else if (temp < TEMP_LOWER_LIMIT) {
        printf(" - TOO LOW (limit: %d.%01d C)\n",
               (int)TEMP_LOWER_LIMIT, abs((int)(TEMP_LOWER_LIMIT * 10)) % 10);
        EventLog_Record(EVENT_TEMP_LOW, temp, current_state);
        consecutive_ok_readings = 0;
        if (current_state == STATE_TRANSPORT) {
            current_state = STATE_ALERT;
        }
        return true;

    } else {
        printf(" - OK\n");
        consecutive_ok_readings++;

        // Clear the alert once enough good readings are seen in a row
        if (current_state == STATE_ALERT &&
            consecutive_ok_readings >= READINGS_TO_CLEAR_ALERT) {
            EventLog_Record(EVENT_TEMP_OK, temp, current_state);
            current_state = STATE_TRANSPORT;
            printf("[SYSTEM] Temperature back in range. Returning to transport mode.\n");
        }
        return false;
    }
}

/*
    checkMotion
    Reads the ADXL345 and checks for a shock. Returns true if a shock
    was detected on this reading.
*/
bool checkMotion(void)
{
    AccelData_t accel = ADXL345_ReadAcceleration();
    float magnitude   = ADXL345_GetMagnitude(accel);

    printf("[ACCEL] X: %d.%02dg  Y: %d.%02dg  Z: %d.%02dg  Mag: %d.%02dg",
           (int)accel.x_g, abs((int)(accel.x_g * 100)) % 100,
           (int)accel.y_g, abs((int)(accel.y_g * 100)) % 100,
           (int)accel.z_g, abs((int)(accel.z_g * 100)) % 100,
           (int)magnitude, abs((int)(magnitude * 100)) % 100);

    if (ADXL345_IsShockDetected(accel)) {
        printf(" - SHOCK DETECTED\n");
        EventLog_Record(EVENT_SHOCK, magnitude, current_state);
        if (current_state == STATE_TRANSPORT) {
            current_state = STATE_ALERT;
        }
        return true;
    }

    printf("\n");
    return false;
}

/*
    main
    Initialises every module then runs the monitoring loop.
*/
int main(void)
{
    printf("\n================================\n");
    printf(" Cold-Chain Transport Logger\n");
    printf(" UESTC 2004 - Univ of Glasgow\n");
    printf("================================\n");

    // Set up the sensors and the log
    TMP102_Init();
    printf("[INIT] TMP102 ready\n");

    ADXL345_Init();
    printf("[INIT] ADXL345 ready\n");

    EventLog_Init();
    EventLog_Record(EVENT_SYSTEM_START, 0.0f, current_state);
    printf("[INIT] Event log ready\n");

    // Set up the button interrupt (pull-down, triggers on press)
    mode_button.rise(&onButtonPress);

    // Start in idle with the green LED on
    updateIndicators(false, false);

    printf("\nReady. Press button to start transport monitoring.\n");
    printf("Temp range: %d.%01d to %d.%01d C | Shock threshold: %d.%01dg\n\n",
           (int)TEMP_LOWER_LIMIT, abs((int)(TEMP_LOWER_LIMIT * 10)) % 10,
           (int)TEMP_UPPER_LIMIT, abs((int)(TEMP_UPPER_LIMIT * 10)) % 10,
           (int)ADXL_SHOCK_THRESHOLD_G, abs((int)(ADXL_SHOCK_THRESHOLD_G * 10)) % 10);

    // Main loop
    while (1) {

        // Handle a button press if the ISR flagged one
        if (button_pressed) {
            handleButton();
        }

        // Sample the sensors only while monitoring
        if (current_state == STATE_TRANSPORT || current_state == STATE_ALERT) {

            bool temp_breach  = checkTemperature();
            bool shock_active = checkMotion();

            updateIndicators(temp_breach, shock_active);

            // Print a summary every 10 events to track progress
            if (EventLog_GetCount() > 0 && EventLog_GetCount() % 10 == 0) {
                EventLog_PrintSummary();
            }

            // Sleep between samples (saves power, avoids flooding serial)
            thread_sleep_for(SAMPLE_INTERVAL_MS);

        } else {
            // Idle: sleep most of the time and just watch for the button
            thread_sleep_for(200);
        }
    }
}
