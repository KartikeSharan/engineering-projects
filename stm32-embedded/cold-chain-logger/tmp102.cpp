/*
    tmp102.cpp
    TMP102 temperature sensor driver.
    Cold-Chain Transport Condition Logger
    UESTC 2004 Embedded Processors - University of Glasgow

    This is built directly on the Lab 4 Task 1 code that was tested
    and confirmed working on the board. The sensor is configured once
    in continuous conversion mode and then read whenever needed. The
    12-bit temperature register gives 0.0625 degrees C per bit.
*/

#include "tmp102.h"

// I2C bus object for the TMP102 (global, matches working Lab 4 code)
I2C tmp102_i2c(TMP102_SDA, TMP102_SCL);

/*
    TMP102_Init
    Sets the I2C clock and writes the config register to put the
    sensor into continuous conversion mode. Called once at startup.
*/
void TMP102_Init(void)
{
    tmp102_i2c.frequency(100000);

    char config_bytes[3];
    config_bytes[0] = TMP102_CONFIG_REG;
    config_bytes[1] = 0x60;   // 12-bit, continuous conversion
    config_bytes[2] = 0xA0;   // default rate and alert settings
    tmp102_i2c.write(TMP102_I2C_ADDRESS, config_bytes, 3);
}

/*
    TMP102_ReadTemperature
    Reads the two-byte temperature register and converts it to
    degrees Celsius. Returns the temperature as a float.
*/
float TMP102_ReadTemperature(void)
{
    char pointer_byte = TMP102_TEMP_REG;
    char raw_bytes[2];

    // Point to the temperature register then read 2 bytes
    tmp102_i2c.write(TMP102_I2C_ADDRESS, &pointer_byte, 1);
    tmp102_i2c.read(TMP102_I2C_ADDRESS, raw_bytes, 2);

    // First byte is the upper 8 bits, second byte upper 4 bits = 12-bit value
    int16_t raw_temp = (raw_bytes[0] << 4) | (raw_bytes[1] >> 4);

    // Convert two's complement if the temperature is negative
    if (raw_temp > 2047) {
        raw_temp -= 4096;
    }

    // Each LSB is 0.0625 degrees C
    return raw_temp * 0.0625f;
}

/*
    TMP102_Shutdown
    Sets the SD bit in the config register so the sensor stops
    converting and drops to a very low current draw. Used to save
    power when readings are not needed.
*/
void TMP102_Shutdown(void)
{
    char config_bytes[3];
    config_bytes[0] = TMP102_CONFIG_REG;
    config_bytes[1] = 0x61;   // same as continuous but SD bit set
    config_bytes[2] = 0xA0;
    tmp102_i2c.write(TMP102_I2C_ADDRESS, config_bytes, 3);
}

/*
    TMP102_Wakeup
    Clears the SD bit so the sensor resumes continuous conversion.
    A short delay lets the first conversion complete before reading.
*/
void TMP102_Wakeup(void)
{
    char config_bytes[3];
    config_bytes[0] = TMP102_CONFIG_REG;
    config_bytes[1] = 0x60;   // SD bit cleared = continuous
    config_bytes[2] = 0xA0;
    tmp102_i2c.write(TMP102_I2C_ADDRESS, config_bytes, 3);

    // Wait for one conversion cycle to finish (about 26ms for 12-bit)
    thread_sleep_for(30);
}
