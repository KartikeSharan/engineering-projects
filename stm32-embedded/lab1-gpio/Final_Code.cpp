 /**
 * Programmable Signal Controller (PSC)
 * 
 * This program implements a three-mode signal generator:
 * - Mode 0: Idle mode - no output, LED off
 * - Mode 1: Square wave generator (100-2000 Hz controlled by potentiometer)
 * - Mode 2: Pattern generator with specific timing sequence
 * 
 * Hardware connections:
 * - LED: A6
 * - Signal Output: D5
 * - Mode Button: D10
 * - Reset Button: D11
 * - Potentiometer: A0
 * 
 * The system uses software-based timing with non-blocking delays
 * to maintain responsiveness to button inputs.
 */

#include "mbed.h"

// Pin definitions
DigitalOut led(A6);           // Status indicator LED
DigitalOut signalOut(D5);     // Signal output pin
DigitalIn modeButton(D10);    // Button to cycle through modes
DigitalIn resetButton(D11);   // Button to reset to mode 0
AnalogIn potentiometer(A0);   // Potentiometer for frequency control

// System constants
const int MODE_COUNT = 3;           // Total number of operating modes
const unsigned long BLINK_INTERVAL_MS = 500;  // 500ms for 1Hz blinking (on 500ms, off 500ms)
const int PATTERN_STEPS = 4;        // Number of steps in pattern generator

// Timing constants for pattern generator (in milliseconds)
const int PATTERN_TIMES[PATTERN_STEPS] = {10, 20, 40, 80};
const bool PATTERN_STATES[PATTERN_STEPS] = {1, 0, 1, 0};  // HIGH, LOW, HIGH, LOW

// Mode definitions
enum SystemMode {
    MODE_IDLE = 0,      // Mode 0: Idle - LED off, output low
    MODE_SQUARE = 1,    // Mode 1: Square wave generator
    MODE_PATTERN = 2    // Mode 2: Pattern generator
};

// Global variables
SystemMode currentMode = MODE_IDLE;      // Current operating mode
bool lastModeButtonState = 1;             // Previous state of mode button (pull-up: 1 = not pressed)
bool lastResetButtonState = 1;            // Previous state of reset button

// Mode 1 (Square wave) variables
unsigned long lastSquareToggleTime = 0;   // Last time square wave output changed
bool squareWaveState = 0;                 // Current square wave output state

// Mode 2 (Pattern) variables
int currentPatternStep = 0;               // Current step in the pattern sequence
unsigned long lastPatternStepTime = 0;    // Last time pattern step changed

// Mode 0 (Idle) LED blink tracking
unsigned long lastLedBlinkTime = 0;       // Last time LED blinked for idle mode
bool ledBlinkState = 0;                   // Current LED state for idle blinking

// Function prototypes
void checkButtons();
float getFrequencyFromPotentiometer();
unsigned long calculateHalfPeriodMicroseconds(float frequency);
void updateSquareWave();
void updatePatternGenerator();
void updateLedIndicator();

int main() {
    // Configure pull-up resistors for buttons
    modeButton.mode(PullUp);
    resetButton.mode(PullUp);
    
    // Initialize outputs to known states
    led = 0;          // LED off
    signalOut = 0;    // Signal output low
    
    // Main program loop
    while (1) {
        // Check for button presses (non-blocking)
        checkButtons();
        
        // Execute behaviour based on current mode
        switch (currentMode) {
            case MODE_IDLE:
                // No signal output, keep output low
                signalOut = 0;
                break;
                
            case MODE_SQUARE:
                // Generate square wave with frequency controlled by potentiometer
                updateSquareWave();
                break;
                
            case MODE_PATTERN:
                // Generate the predefined pattern sequence
                updatePatternGenerator();
                break;
        }
        
        // Update LED indicator based on current mode
        updateLedIndicator();
        
        // Small delay to prevent overwhelming the CPU
        // This non-blocking delay ensures button responsiveness
        wait_us(100);  // 100 microseconds delay
    }
}

/**
 * Check both buttons and handle mode transitions
 * Uses edge detection to ensure single press per button push
 */
void checkButtons() {
    // Read current button states (buttons are active low: 0 = pressed)
    bool currentModeButtonState = modeButton.read();
    bool currentResetButtonState = resetButton.read();
    
    // Check mode button - detect falling edge (press)
    if (lastModeButtonState == 1 && currentModeButtonState == 0) {
        // Mode button pressed - cycle to next mode
        currentMode = (SystemMode)((currentMode + 1) % MODE_COUNT);
        
        // Reset mode-specific states when entering new mode
        if (currentMode == MODE_SQUARE) {
            squareWaveState = 0;
            signalOut = 0;
            lastSquareToggleTime = us_ticker_read();  // Get current time in microseconds
        } else if (currentMode == MODE_PATTERN) {
            currentPatternStep = 0;
            signalOut = PATTERN_STATES[0];
            lastPatternStepTime = us_ticker_read();
        }
    }
    
    // Check reset button - detect falling edge (press)
    if (lastResetButtonState == 1 && currentResetButtonState == 0) {
        // Reset button pressed - return to mode 0 (Idle)
        currentMode = MODE_IDLE;
        
        // Reset outputs
        signalOut = 0;
        squareWaveState = 0;
        currentPatternStep = 0;
    }
    
    // Save current states for next iteration
    lastModeButtonState = currentModeButtonState;
    lastResetButtonState = currentResetButtonState;
}

/**
 * Read potentiometer and convert to frequency
 * @return frequency in Hz (range: 100 Hz to 2000 Hz)
 */
float getFrequencyFromPotentiometer() {
    const float MIN_FREQUENCY = 100.0f;   // 100 Hz at minimum pot position
    const float MAX_FREQUENCY = 2000.0f;  // 2000 Hz at maximum pot position
    
    // Read potentiometer (0.0 to 1.0)
    float potValue = potentiometer.read();
    
    // Linear mapping from pot value to frequency
    float frequency = MIN_FREQUENCY + (potValue * (MAX_FREQUENCY - MIN_FREQUENCY));
    
    return frequency;
}

/**
 * Calculate half period in microseconds for square wave generation
 * @param frequency - desired frequency in Hz
 * @return half period in microseconds
 */
unsigned long calculateHalfPeriodMicroseconds(float frequency) {
    // Total period = 1/frequency seconds
    // Half period = period / 2
    // Convert to microseconds: multiply by 1,000,000
    unsigned long halfPeriodUs = (unsigned long)(1000000.0f / (frequency * 2.0f));
    
    // Ensure minimum half period to avoid excessive CPU usage
    if (halfPeriodUs < 100) {
        halfPeriodUs = 100;  // Minimum 100 microseconds (5 kHz max effective)
    }
    
    return halfPeriodUs;
}

/**
 * Generate square wave in Mode 1
 * Toggles output based on frequency determined by potentiometer
 */
void updateSquareWave() {
    // Get current frequency from potentiometer
    float frequency = getFrequencyFromPotentiometer();
    
    // Calculate half period for this frequency
    unsigned long halfPeriodUs = calculateHalfPeriodMicroseconds(frequency);
    
    // Get current time in microseconds
    unsigned long currentTimeUs = us_ticker_read();
    
    // Check if it's time to toggle the output
    if ((currentTimeUs - lastSquareToggleTime) >= halfPeriodUs) {
        // Toggle square wave state
        squareWaveState = !squareWaveState;
        signalOut = squareWaveState;
        
        // Update last toggle time
        lastSquareToggleTime = currentTimeUs;
    }
}

/**
 * Generate pattern sequence in Mode 2
 * Sequence: HIGH 10ms, LOW 20ms, HIGH 40ms, LOW 80ms (repeats)
 */
void updatePatternGenerator() {
    // Get current time in microseconds
    unsigned long currentTimeUs = us_ticker_read();
    
    // Calculate elapsed time since last step change
    unsigned long elapsedTimeUs = currentTimeUs - lastPatternStepTime;
    unsigned long stepDurationUs = PATTERN_TIMES[currentPatternStep] * 1000;  // Convert ms to us
    
    // Check if current step duration has elapsed
    if (elapsedTimeUs >= stepDurationUs) {
        // Move to next pattern step
        currentPatternStep = (currentPatternStep + 1) % PATTERN_STEPS;
        
        // Update output for new step
        signalOut = PATTERN_STATES[currentPatternStep];
        
        // Reset timer for new step
        lastPatternStepTime = currentTimeUs;
    }
}

/**
 * Update LED based on current system mode
 * Mode 0: LED off
 * Mode 1: Blink at 1 Hz (500ms on, 500ms off)
 * Mode 2: LED on constantly
 */
void updateLedIndicator() {
    unsigned long currentTimeMs = us_ticker_read() / 1000;  // Convert to milliseconds
    
    switch (currentMode) {
        case MODE_IDLE:
            led = 0;  // LED off in idle mode
            break;
            
        case MODE_SQUARE:
            // Blink LED at 1 Hz (500ms on, 500ms off)
            if ((currentTimeMs % BLINK_INTERVAL_MS) < (BLINK_INTERVAL_MS / 2)) {
                led = 1;  // On for first half of interval
            } else {
                led = 0;  // Off for second half
            }
            break;
            
        case MODE_PATTERN:
            led = 1;  // LED constantly on in pattern mode
            break;
    }
}