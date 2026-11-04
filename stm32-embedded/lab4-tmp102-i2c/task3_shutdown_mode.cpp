#include "mbed.h"

#define TMP102_SDA          D4
#define TMP102_SCL          D5
#define TMP102_I2C_ADDRESS  0x90
#define TMP102_TEMP_REG     0x00
#define TMP102_CONFIG_REG   0x01

I2C tmp102_i2c(TMP102_SDA, TMP102_SCL);

void TMP102_Configure_Shutdown() {
    char config_bytes[3];
    config_bytes[0] = TMP102_CONFIG_REG;
    config_bytes[1] = 0x61;   // SD bit = 1, shutdown mode
    config_bytes[2] = 0xA0;
    tmp102_i2c.write(TMP102_I2C_ADDRESS, config_bytes, 3);
}

float TMP102_Read_Temperature() {
    char pointer_byte = TMP102_TEMP_REG;
    char raw_bytes[2];
    tmp102_i2c.write(TMP102_I2C_ADDRESS, &pointer_byte, 1);
    tmp102_i2c.read(TMP102_I2C_ADDRESS, raw_bytes, 2);

    int16_t raw_temp = (raw_bytes[0] << 4) | (raw_bytes[1] >> 4);
    if (raw_temp > 2047) raw_temp -= 4096;
    return raw_temp * 0.0625f;
}

int main() {
    tmp102_i2c.frequency(100000);
    TMP102_Configure_Shutdown();
    printf("TMP102 Task 3 - Shutdown Mode Enabled\n");
    printf("Measure current with ammeter now\n");

    while (1) {
        float temperature_celsius = TMP102_Read_Temperature();
        printf("Temperature (cached): %d.%04d C\n",
               (int)temperature_celsius,
               abs((int)(temperature_celsius * 10000) % 10000));
        thread_sleep_for(1000);
    }
}
