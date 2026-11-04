#include "mbed.h"

#define TMP102_SDA          D4
#define TMP102_SCL          D5
#define TMP102_I2C_ADDRESS  0x90
#define TMP102_TEMP_REG     0x00
#define TMP102_CONFIG_REG   0x01
#define BUTTON_PIN          D3

I2C tmp102_i2c(TMP102_SDA, TMP102_SCL);
InterruptIn button_interrupt(BUTTON_PIN, PullDown);

volatile bool button_pressed_flag = false;

void button_press_isr() {
    button_pressed_flag = true;
}

void TMP102_Configure_Shutdown() {
    char config_bytes[3];
    config_bytes[0] = TMP102_CONFIG_REG;
    config_bytes[1] = 0x61;
    config_bytes[2] = 0xA0;
    tmp102_i2c.write(TMP102_I2C_ADDRESS, config_bytes, 3);
}

float TMP102_OneShot_Read_Temperature() {
    char oneshot_config[3];
    oneshot_config[0] = TMP102_CONFIG_REG;
    oneshot_config[1] = 0xE1;
    oneshot_config[2] = 0xA0;
    tmp102_i2c.write(TMP102_I2C_ADDRESS, oneshot_config, 3);

    thread_sleep_for(20);

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

    button_interrupt.rise(&button_press_isr);

    printf("TMP102 Task 5 - Press button to take a reading\n");

    int reading_count = 0;

    while (1) {
        if (button_pressed_flag) {
            button_pressed_flag = false;
            reading_count++;

            float temperature_celsius = TMP102_OneShot_Read_Temperature();

            printf("Reading %2d: %d.%04d C\n",
                   reading_count,
                   (int)temperature_celsius,
                   abs((int)(temperature_celsius * 10000) % 10000));

            thread_sleep_for(500);  // debounce delay
        }
        thread_sleep_for(10);
    }
}
