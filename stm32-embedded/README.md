# Embedded C/C++ on STM32 (Embedded Processors, UESTC 2026)

NUCLEO-L432KC (Arm Cortex-M4F) with Mbed OS 6 in Keil Studio Cloud. Module mark: 82%.

| Folder | Contents |
|---|---|
| `cold-chain-logger/` | Final project (pair project with Elyas Omar). A transport logger that sounds a 2 kHz buzzer alarm if the temperature leaves 2 to 8 °C, or the unit is dropped (over 2 g) or tipped. Drivers for the TMP102 (I2C) and ADXL345 (SPI), an event log, and a state machine that only samples while in transport. During testing both sensors failed together; testing each on its own traced it to two solder bridges on the board (SB16, SB18) joining the SPI and I2C pins, so the SPI bus was moved to free pins. |
| `lab1-gpio/` | Lab 1 tasks |
| `lab2-pwm-timers/` | Task 2.4: a 1 kHz PWM output whose duty cycle a Ticker interrupt switches between 70% and 30% every 2 s |
| `lab3-adxl345-spi/` | ADXL345 accelerometer over SPI: bus set-up (Task 2), reading the axes (Task 3), driving LEDs from the readings (Task 4) and the bonus task (Task 5) |
| `lab4-tmp102-i2c/` | TMP102 temperature sensor over I2C: continuous conversion (Task 1), averaging to reduce error (Task 2), shutdown mode (Task 3), current measurement with and without shutdown mode (Task 4), and one-shot conversions triggered by a push-button interrupt (Task 5) |
| `reports/` | Lab 2 and Lab 3 reports |

## Where the lab code came from

Keil Studio Cloud closed with the end of Mbed, taking the original projects with it. The Lab 2 to 4 programs were recovered from the code listings in my lab reports (Lab 2 PDF, Lab 3 and Lab 4 Word documents). Curly quotes from Word were straightened; nothing else was changed. Lab 2's Part 1 code wasn't printed as text in its report, so it isn't here. The recovered code hasn't been recompiled.

## Known issues (found while recovering the code)

* **Lab 4 Task 5:** after starting a one-shot conversion the code waits 20 ms before reading, but the TMP102 datasheet gives a typical conversion time of 26 ms (35 ms maximum), so a reading can return the previous result. Waiting 35 ms, or polling the OS bit until the conversion finishes, fixes it.
* **Lab 4, all tasks:** temperatures between 0 and -1 °C print without their minus sign, because the whole-number part is printed with `(int)`, which rounds towards zero.
* **Lab 4 Task 3:** in shutdown mode the sensor doesn't convert, so the loop re-reads the last stored value. This is intended (the output says "cached"), and Task 5 shows the one-shot fix.
