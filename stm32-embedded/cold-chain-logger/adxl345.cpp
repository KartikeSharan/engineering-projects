/*
    adxl345.cpp
    ADXL345 accelerometer driver using SPI.
    Cold-Chain Transport Condition Logger
    UESTC 2004 Embedded Processors - University of Glasgow

    This is built on the Lab 3 code that was tested and confirmed
    working on the board. It uses 4-wire SPI in mode 3 at 2MHz,
    FULL_RES mode at +/-16g, and reads all three axes in one burst
    starting from the DATAX0 register.
*/

#include "adxl345.h"
#include <cmath>

// SPI bus and chip select for the ADXL345
SPI        adxl345_spi(ADXL_MOSI_PIN, ADXL_MISO_PIN, ADXL_SCLK_PIN);
DigitalOut adxl345_cs(ADXL_CS_PIN);

/*
    writeReg
    Writes a single byte to one ADXL345 register.
*/
static void writeReg(char address, char value)
{
    adxl345_cs = 0;                  // select the device
    adxl345_spi.write(address);      // send the register address
    adxl345_spi.write(value);        // send the value
    adxl345_cs = 1;                  // deselect the device
}

/*
    readRegs
    Reads several bytes in a row from the ADXL345. The read flag (0x80)
    and multi-byte flag (0x40) are set in the address byte, as required
    by the datasheet for a multi-byte read.
*/
static void readRegs(char start_address, char* buffer, int num_bytes)
{
    char address = 0x80 | 0x40 | start_address;

    adxl345_cs = 0;
    adxl345_spi.write(address);

    for (int i = 0; i < num_bytes; i++) {
        buffer[i] = adxl345_spi.write(0x00);  // clock out each byte
    }

    adxl345_cs = 1;
}

/*
    ADXL345_Init
    Sets up the SPI format and configures the ADXL345 for measurement
    mode at +/-16g FULL_RES. Call once at startup.
*/
void ADXL345_Init(void)
{
    adxl345_spi.format(8, 3);          // 8-bit data, SPI mode 3
    adxl345_spi.frequency(2000000);    // 2MHz clock
    adxl345_cs = 1;                    // chip select idle high

    // DATA_FORMAT: FULL_RES, +/-16g range
    writeReg(ADXL_DATA_FORMAT_REG, 0x0B);

    // POWER_CTL: measurement mode on
    writeReg(ADXL_POWER_CTL_REG, 0x08);

    // BW_RATE: 25Hz output data rate, normal power
    writeReg(ADXL_BW_RATE_REG, 0x08);
}

/*
    ADXL345_ReadAcceleration
    Reads the six data bytes (two per axis) in one burst and converts
    each axis to g. Returns the result in an AccelData_t.
*/
AccelData_t ADXL345_ReadAcceleration(void)
{
    char raw[6];
    AccelData_t accel;

    // Burst read 6 bytes starting at DATAX0
    readRegs(ADXL_DATAX0_REG, raw, 6);

    // Combine the low and high byte for each axis (little-endian)
    int16_t x_raw = (int16_t)((raw[1] << 8) | raw[0]);
    int16_t y_raw = (int16_t)((raw[3] << 8) | raw[2]);
    int16_t z_raw = (int16_t)((raw[5] << 8) | raw[4]);

    // Scale the raw counts into g
    accel.x_g = x_raw * ADXL_SCALE_FACTOR;
    accel.y_g = y_raw * ADXL_SCALE_FACTOR;
    accel.z_g = z_raw * ADXL_SCALE_FACTOR;

    return accel;
}

/*
    ADXL345_GetMagnitude
    Works out the overall acceleration magnitude. Using the magnitude
    means a shock is detected no matter which way the board is facing.
*/
float ADXL345_GetMagnitude(AccelData_t accel)
{
    return sqrtf(accel.x_g * accel.x_g +
                 accel.y_g * accel.y_g +
                 accel.z_g * accel.z_g);
}

/*
    ADXL345_IsShockDetected
    Returns true if the magnitude is above the shock threshold.
*/
bool ADXL345_IsShockDetected(AccelData_t accel)
{
    return ADXL345_GetMagnitude(accel) > ADXL_SHOCK_THRESHOLD_G;
}
