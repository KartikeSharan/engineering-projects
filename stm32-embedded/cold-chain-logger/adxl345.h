/*
    adxl345.h
    Header file for the ADXL345 accelerometer driver.
    Cold-Chain Transport Condition Logger
    UESTC 2004 Embedded Processors - University of Glasgow

    The ADXL345 is a 3-axis accelerometer that uses SPI. This driver
    sets it up in FULL_RES mode at +/-16g and reads the three axes.
    Pin definitions match the working Lab 3 setup.
*/

#ifndef ADXL345_H
#define ADXL345_H

#include "mbed.h"

// SPI pins on the NUCLEO-L432KC.
// IMPORTANT: these use the dedicated SPI1 pins (D11/D12/D13) instead
// of A4/A5/A6. On this board A4 (PA5) and A5 (PA6) are tied to the
// I2C pins D5/D4 (PB6/PB7) through solder bridges SB16/SB18, so using
// A4/A5 for SPI stops the TMP102 I2C from working. D11/D12/D13 are on
// port B and do not clash with the I2C lines.
#define ADXL_MOSI_PIN   D11   // PB5
#define ADXL_MISO_PIN   D12   // PB4
#define ADXL_SCLK_PIN   D13   // PB3
#define ADXL_CS_PIN     D10   // PA11 (GPIO chip select)

// ADXL345 register addresses (from the datasheet)
#define ADXL_POWER_CTL_REG      0x2D
#define ADXL_DATA_FORMAT_REG    0x31
#define ADXL_BW_RATE_REG        0x2C
#define ADXL_DATAX0_REG         0x32

// In FULL_RES mode the scale is 3.9mg per LSB (from the datasheet)
#define ADXL_SCALE_FACTOR       0.0039f

// Shock threshold in g. Normal handling and vehicle vibration stays
// below about 0.5g, so a magnitude above 2g indicates rough handling
// such as a drop or impact (consistent with ASTM D4169 transport tests).
#define ADXL_SHOCK_THRESHOLD_G  1.2f

// Holds one set of 3-axis acceleration readings in g
typedef struct {
    float x_g;
    float y_g;
    float z_g;
} AccelData_t;

// Set up the ADXL345 in measurement mode. Call once at startup.
void ADXL345_Init(void);

// Read the X, Y and Z acceleration values. Returns them in g.
AccelData_t ADXL345_ReadAcceleration(void);

// Work out the overall vector magnitude sqrt(x^2 + y^2 + z^2).
float ADXL345_GetMagnitude(AccelData_t accel);

// Returns true if the magnitude is above the shock threshold.
bool ADXL345_IsShockDetected(AccelData_t accel);

#endif
