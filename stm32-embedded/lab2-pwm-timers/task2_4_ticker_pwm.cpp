#include "mbed.h"

// Pin definitions (using the labels from your lab manual)
PwmOut pwm_output(D6);           // PWM signal output (1kHz)
DigitalOut indicator_led(D5); // LED indicator for duty cycle state

// Application constants
const float PWM_FREQUENCY = 1000.0f;       // 1 kHz
const float PWM_PERIOD = 1.0f / PWM_FREQUENCY; // 0.001 seconds (1ms)
const float DUTY_CYCLE_HIGH = 0.70f;    // 70% HIGH state
const float DUTY_CYCLE_LOW = 0.30f;     // 30% HIGH state
const float TOGGLE_INTERVAL = 2.0f;    // Switch duty cycle every 2 seconds

// Global state tracking (volatile because it is modified in an interrupt context)
volatile bool is_high_duty_cycle = true;

/*
 * Interrupt Service Routine for the Ticker.
 * This function is called automatically every 'TOGGLE_INTERVAL' seconds.
 * It switches the PWM duty cycle and updates the indicator LED.
 */
void toggle_duty_cycle() {
    if (is_high_duty_cycle) {
        // Switch from 70% to 30% duty cycle
        pwm_output.write(DUTY_CYCLE_LOW);
        indicator_led = 0; // Turn LED OFF for 30% duty cycle
        is_high_duty_cycle = false;
    } else {
        // Switch from 30% to 70% duty cycle
        pwm_output.write(DUTY_CYCLE_HIGH);
        indicator_led = 1; // Turn LED ON for 70% duty cycle
        is_high_duty_cycle = true;
    }
}

int main() {
   // 1. Configure the PWM signal's base frequency (Period)
   pwm_output.period(PWM_PERIOD); // Set period to 1ms

    // 2. Set the initial state
    pwm_output.write(DUTY_CYCLE_HIGH); // Start with 70% duty cycle
    indicator_led = 1;          // Indicator LED ON
    is_high_duty_cycle = true;       // Initialize state variable

    // 3. Attach the interrupt function to the Ticker
    Ticker duty_ticker;
    duty_ticker.attach(&toggle_duty_cycle, TOGGLE_INTERVAL);

    // 4. Main loop is idle. The Ticker interrupt handles all changes.
    while (1) {
        // The processor can sleep or do other non-critical tasks here.
        // A small sleep helps reduce power consumption.
        wait_us(1000000); // 1 second delay
    }
}
