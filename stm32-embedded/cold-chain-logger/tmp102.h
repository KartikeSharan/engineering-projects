/*
    tmp102.h
    Header file for the TMP102 temperature sensor driver.
    Cold-Chain Transport Condition Logger
    UESTC 2004 Embedded Processors - University of Glasgow

    The TMP102 is an I2C temperature sensor. This driver configures
    it in continuous conversion mode and reads the temperature in
    degrees Celsius. Pin and register definitions match the working
    Lab 4 setup (SDA on D4, SCL on D5, slave address 0x90).
*/

#ifndef TMP102_H
#define TMP102_H

#include "mbed.h"

// I2C pins on the NUCLEO-L432KC (same as Lab 4)
#define TMP102_SDA          D4
#define TMP102_SCL          D5

// TMP102 slave address (ADD0 pin tied to GND)
#define TMP102_I2C_ADDRESS  0x90

// TMP102 internal register addresses
#define TMP102_TEMP_REG     0x00
#define TMP102_CONFIG_REG   0x01

// Set up the TMP102 in continuous conversion mode. Call once at startup.
void TMP102_Init(void);

// Read the current temperature in degrees Celsius.
float TMP102_ReadTemperature(void);

// Put the sensor into shutdown mode to save power.
void TMP102_Shutdown(void);

// Bring the sensor back into continuous conversion mode.
void TMP102_Wakeup(void);

#endif
