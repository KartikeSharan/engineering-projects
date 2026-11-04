# Embedded C/C++ on STM32 (Embedded Processors, UESTC 2026)

NUCLEO-L432KC (Arm Cortex-M4F) with Mbed OS 6 in Keil Studio. Module mark: 82%.

* **`cold-chain-logger/`**: final project, a pair project with Elyas Omar. A transport logger that sounds a 2 kHz buzzer alarm if the temperature leaves 2 to 8 °C, or the unit is dropped (over 2 g) or tipped. Register-level drivers for the TMP102 (I2C) and ADXL345 (SPI), an event log, and a state machine that only samples while in transport. During testing both sensors failed together; testing each on its own traced it to two solder bridges on the board (SB16, SB18) joining the SPI and I2C pins, so the SPI bus was moved to free pins.
* **`lab1-gpio/`**: Lab 1 tasks.
* **`reports/`**: Lab 2 and Lab 3 reports.
