#include "mbed.h"

// ------------------- PIN SETUP -------------------
DigitalOut myLed(A6);           // LED anode to A6, cathode to GND
DigitalIn modeButton(D10);      // Mode button - pressed = 0, not pressed = 1
DigitalIn resetButton(D11);     // Reset button - pressed = 0, not pressed = 1

// ------------------- VARIABLES -------------------
int currentMode = 0;            // 0 = Idle, 1 = Blinking, 2 = Always ON
int lastModeButtonState = 1;    // Previous state of mode button (1 = not pressed)
int lastResetButtonState = 1;   // Previous state of reset button

// Variables for blinking in Mode 1
int ledState = 0;               // 0 = OFF, 1 = ON
int blinkCounter = 0;           // Counts how many 1ms delays have passed

// ------------------- MAIN PROGRAM -------------------
int main() {
    // Enable internal pull-up resistors so buttons read 1 when not pressed
    modeButton.mode(PullUp);
    resetButton.mode(PullUp);
    
    // Start with LED off
    myLed = 0;
    
    // Main loop - runs forever
    while (1) {
        
        // ----- READ BUTTON STATES -----
        int currentModeButton = modeButton.read();
        int currentResetButton = resetButton.read();
        
        // ----- CHECK IF MODE BUTTON WAS JUST PRESSED -----
        // Looking for falling edge: was 1, now 0
        if (lastModeButtonState == 1 && currentModeButton == 0) {
            // Mode button pressed - cycle to next mode
            currentMode = currentMode + 1;
            if (currentMode > 2) {
                currentMode = 0;    // Wrap around: 2 -> 0
            }
            
            // Reset blink timing when entering Mode 1
            if (currentMode == 1) {
                blinkCounter = 0;
                ledState = 0;
                myLed = 0;
            }
            
            // Wait 50ms for button to stop bouncing
            wait_us(50000);
        }
        
        // ----- CHECK IF RESET BUTTON WAS JUST PRESSED -----
        if (lastResetButtonState == 1 && currentResetButton == 0) {
            // Reset button pressed - go to Mode 0 (Idle)
            currentMode = 0;
            myLed = 0;
            blinkCounter = 0;
            ledState = 0;
            wait_us(50000);
        }
        
        // Save current button states for next loop iteration
        lastModeButtonState = currentModeButton;
        lastResetButtonState = currentResetButton;
        
        // ----- CONTROL LED BASED ON CURRENT MODE -----
        
        if (currentMode == 0) {
            // MODE 0: LED stays OFF
            myLed = 0;
        }
        else if (currentMode == 1) {
            // MODE 1: LED blinks at 1 Hz (500ms ON, 500ms OFF)
            // We do this by counting how many 1ms delays have happened
            blinkCounter = blinkCounter + 1;
            
            // 500 counts of 1ms = 500ms
            if (blinkCounter >= 500) {
                // Time to toggle the LED
                if (ledState == 0) {
                    ledState = 1;   // Turn ON
                    myLed = 1;
                } else {
                    ledState = 0;   // Turn OFF
                    myLed = 0;
                }
                blinkCounter = 0;   // Reset counter for next half-cycle
            }
        }
        else if (currentMode == 2) {
            // MODE 2: LED stays ON constantly
            myLed = 1;
        }
        
        // Wait 1ms before checking everything again
        // This is a SHORT non-blocking delay (less than 5ms as required)
        // It makes the button response feel instant to humans
        wait_us(1000);
        
    } // end while(1)
}
