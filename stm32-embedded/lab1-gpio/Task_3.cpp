/**
 * Lab 1 Task 3 - Square Wave Generator
 * Mode 0: LED off, output low
 * Mode 1: LED blinks 1Hz, square wave 100-2000Hz from potentiometer
 * Mode 2: LED on, output low (placeholder for Task 4)
 * Pins: LED=A6, Output=D5, Mode=D10, Reset=D11, Pot=A0
 */

#include "mbed.h"

// Pin setup
DigitalOut led(A6);
DigitalOut signalOut(D5);
DigitalIn modeBtn(D10);
DigitalIn resetBtn(D11);
AnalogIn pot(A0);

// Mode tracking
int mode = 0;           // 0, 1, or 2
int lastModeBtn = 1;
int lastResetBtn = 1;

// Timing variables (each loop = 250us)
int ledCount = 0;       // counts for LED blink
int ledState = 0;       // 0=off, 1=on
int waveCount = 0;      // counts for square wave
int waveState = 0;      // 0=low, 1=high
int halfPeriod = 0;     // how many loops before toggling

int getFreq() {
    float val = pot.read();                    // 0.0 to 1.0
    return 100 + (int)(val * 1900);            // 100Hz to 2000Hz
}

void calcHalfPeriod(int freq) {
    halfPeriod = 2000 / freq;                  // 2000us half period / 250us loop = 2000/freq
    if (halfPeriod < 1) halfPeriod = 1;
}

int main() {
    modeBtn.mode(PullUp);
    resetBtn.mode(PullUp);
    
    while (1) {
        // Read buttons
        int curModeBtn = modeBtn.read();
        int curResetBtn = resetBtn.read();
        
        // Mode button press (falling edge)
        if (lastModeBtn == 1 && curModeBtn == 0) {
            mode++;
            if (mode > 2) mode = 0;
            
            if (mode == 1) {
                waveState = 0;
                signalOut = 0;
                waveCount = 0;
                calcHalfPeriod(getFreq());
            }
            if (mode == 0) {
                signalOut = 0;
                led = 0;
            }
            if (mode == 2) {
                led = 1;
                signalOut = 0;
            }
            wait_us(50000);  // debounce
        }
        
        // Reset button press
        if (lastResetBtn == 1 && curResetBtn == 0) {
            mode = 0;
            signalOut = 0;
            led = 0;
            waveCount = 0;
            ledCount = 0;
            wait_us(50000);
        }
        
        lastModeBtn = curModeBtn;
        lastResetBtn = curResetBtn;
        
        // MODE 0: IDLE
        if (mode == 0) {
            signalOut = 0;
            led = 0;
        }
        
        // MODE 1: SQUARE WAVE
        else if (mode == 1) {
            // Update frequency from pot (real time)
            calcHalfPeriod(getFreq());
            
            // Square wave generation
            waveCount++;
            if (waveCount >= halfPeriod) {
                if (waveState == 0) {
                    waveState = 1;
                    signalOut = 1;
                } else {
                    waveState = 0;
                    signalOut = 0;
                }
                waveCount = 0;
            }
            
            // LED blink at 1Hz (2000 counts * 250us = 500ms)
            ledCount++;
            if (ledCount >= 2000) {
                if (ledState == 0) {
                    ledState = 1;
                    led = 1;
                } else {
                    ledState = 0;
                    led = 0;
                }
                ledCount = 0;
            }
        }
        
        // MODE 2: PATTERN (placeholder)
        else if (mode == 2) {
            led = 1;
            signalOut = 0;
        }
        
        wait_us(250);  // 250us delay = 0.25ms
    }
}