#include "mbed.h"
#include <cmath>   // for atan2f, sqrtf, fabsf

// --------------------------------------------------
// SPI Pin Definitions
// --------------------------------------------------
#define ADXL_MOSI A6
#define ADXL_MISO A5
#define ADXL_SCLK A4
#define ADXL_CS   D12

// --------------------------------------------------
// LED Pin Definitions
// --------------------------------------------------
DigitalOut led_x_red1(D0);    // LXR1 ""“ mild -X tilt
DigitalOut led_x_red2(D1);    // LXR2 ""“ strong -X tilt
DigitalOut led_x_grn1(D11);   // LXG1 ""“ strong +X tilt
DigitalOut led_x_grn2(D9);    // LXG2 ""“ mild +X tilt

DigitalOut led_y_red1(A2);    // LYR1 ""“ mild -Y tilt
DigitalOut led_y_red2(A0);    // LYR2 ""“ strong -Y tilt
DigitalOut led_y_grn1(D6);    // LYG1 ""“ strong +Y tilt
DigitalOut led_y_grn2(A3);    // LYG2 ""“ mild +Y tilt

// --------------------------------------------------
// ADXL345 Register Addresses
// --------------------------------------------------
#define POWER_CTL_REG   0x2D
#define DATA_FORMAT_REG 0x31
#define BW_RATE_REG     0x2C
#define DATAX0_REG      0x32

// --------------------------------------------------
// Tilt thresholds (in g)
// --------------------------------------------------
#define DEAD_ZONE_THRESHOLD  0.05f   // below this "†’ all LEDs off
#define MILD_TILT_THRESHOLD  0.35f   // mild tilt (1 LED per axis)
#define STRONG_TILT_THRESHOLD 0.65f  // strong tilt (2 LEDs per axis)

// --------------------------------------------------
// SPI and Chip Select
// --------------------------------------------------
SPI adxl345_spi(ADXL_MOSI, ADXL_MISO, ADXL_SCLK);
DigitalOut chip_select(ADXL_CS);

// --------------------------------------------------
// Write to ADXL345 register
// --------------------------------------------------
void ADXL345_Write(char reg_address, char reg_value) {
    chip_select = 0;
    adxl345_spi.write(reg_address);
    adxl345_spi.write(reg_value);
    chip_select = 1;
}

// --------------------------------------------------
// Read multiple bytes from ADXL345
// --------------------------------------------------
void ADXL345_Read(char start_address, char* data_buffer, int byte_count) {
    char read_address = 0x80 | 0x40 | start_address; // read + multi-byte flags
    chip_select = 0;
    adxl345_spi.write(read_address);
    for (int byte_index = 0; byte_index < byte_count; byte_index++) {
        data_buffer[byte_index] = adxl345_spi.write(0x00);
    }
    chip_select = 1;
}

// --------------------------------------------------
// Initialise ADXL345
// --------------------------------------------------
void ADXL345_SPI_Initialise() {
    adxl345_spi.format(8, 3);          // 8-bit data, SPI mode 3
    adxl345_spi.frequency(2000000);    // 2 MHz clock
    chip_select = 1;                   // deselect device initially

    // FULL_RES, right-justified, Â±16g range, 4-wire SPI
    ADXL345_Write(DATA_FORMAT_REG, 0x0B);

    // Measurement mode, link cleared, auto-sleep disabled, normal power
    ADXL345_Write(POWER_CTL_REG, 0x08);

    // 25 Hz output data rate, normal power mode
    ADXL345_Write(BW_RATE_REG, 0x08);
}

// --------------------------------------------------
// Turn off all LEDs
// --------------------------------------------------
void all_leds_off() {
    led_x_red1 = 0; led_x_red2 = 0;
    led_x_grn1 = 0; led_x_grn2 = 0;
    led_y_red1 = 0; led_y_red2 = 0;
    led_y_grn1 = 0; led_y_grn2 = 0;
}

// --------------------------------------------------
// Task 5: Combined X+Y LED control
//
// Strategy:
//   1. Compute 2D tilt magnitude from X and Y components
//   2. If below dead zone "†’ all LEDs off
//   3. Compute tilt angle using atan2(Y, X)
//   4. Project the vector onto each axis to determine
//      how much of the tilt is in X and how much in Y
//   5. Drive each axis's LEDs based on their projected
//      component, with mild/strong intensity levels
//
// This means diagonal tilts light up BOTH X and Y LEDs
// simultaneously, which is the key difference from Task 4.
// --------------------------------------------------
void combined_led_control(float x_accel_g, float y_accel_g) {

    // Step 1: compute total tilt magnitude
    float tilt_magnitude = sqrtf(x_accel_g * x_accel_g + y_accel_g * y_accel_g);

    // Step 2: dead zone check ""“ flat on table "†’ all off
    if (tilt_magnitude < DEAD_ZONE_THRESHOLD) {
        all_leds_off();
        return;
    }

    // Step 3: compute angle of tilt vector (radians, -Ï" to +Ï")
    float tilt_angle_rad = atan2f(y_accel_g, x_accel_g);

    // Step 4: project vector onto X and Y axes
    // These are simply the original readings, but we re-derive
    // them from magnitude Ã— cos/sin for clarity
    float x_component = tilt_magnitude * cosf(tilt_angle_rad); // same as x_accel_g
    float y_component = tilt_magnitude * sinf(tilt_angle_rad); // same as y_accel_g

    // Step 5a: X-axis LED control based on X component
    float abs_x = fabsf(x_component);

    if (x_component < -STRONG_TILT_THRESHOLD) {
        // Strong tilt toward -X: both red LEDs on
        led_x_red1 = 1; led_x_red2 = 1;
        led_x_grn1 = 0; led_x_grn2 = 0;
    }
    else if (x_component < -MILD_TILT_THRESHOLD) {
        // Mild tilt toward -X: one red LED
        led_x_red1 = 1; led_x_red2 = 0;
        led_x_grn1 = 0; led_x_grn2 = 0;
    }
    else if (x_component > STRONG_TILT_THRESHOLD) {
        // Strong tilt toward +X: both green LEDs on
        led_x_grn1 = 1; led_x_grn2 = 1;
        led_x_red1 = 0; led_x_red2 = 0;
    }
    else if (x_component > MILD_TILT_THRESHOLD) {
        // Mild tilt toward +X: one green LED
        led_x_grn2 = 1; led_x_grn1 = 0;
        led_x_red1 = 0; led_x_red2 = 0;
    }
    else {
        // X near-zero (mostly Y-axis tilt)
        led_x_red1 = 0; led_x_red2 = 0;
        led_x_grn1 = 0; led_x_grn2 = 0;
    }

    // Step 5b: Y-axis LED control based on Y component
    float abs_y = fabsf(y_component);

    if (y_component < -STRONG_TILT_THRESHOLD) {
        led_y_red1 = 1; led_y_red2 = 1;
        led_y_grn1 = 0; led_y_grn2 = 0;
    }
    else if (y_component < -MILD_TILT_THRESHOLD) {
        led_y_red1 = 1; led_y_red2 = 0;
        led_y_grn1 = 0; led_y_grn2 = 0;
    }
    else if (y_component > STRONG_TILT_THRESHOLD) {
        led_y_grn1 = 1; led_y_grn2 = 1;
        led_y_red1 = 0; led_y_red2 = 0;
    }
    else if (y_component > MILD_TILT_THRESHOLD) {
        led_y_grn2 = 1; led_y_grn1 = 0;
        led_y_red1 = 0; led_y_red2 = 0;
    }
    else {
        led_y_red1 = 0; led_y_red2 = 0;
        led_y_grn1 = 0; led_y_grn2 = 0;
    }
}

// --------------------------------------------------
// Main Program
// --------------------------------------------------
int main() {
    char raw_data[6];
    int16_t x_raw, y_raw, z_raw;
    float x_accel_g, y_accel_g, z_accel_g;

    printf("ADXL345 Task 5 starting...\n");
    ADXL345_SPI_Initialise();
    printf("Initialisation complete\n");

    all_leds_off();

    while (1) {
        // Read 6 bytes starting at DATAX0 (X0, X1, Y0, Y1, Z0, Z1)
        ADXL345_Read(DATAX0_REG, raw_data, 6);

        // Combine bytes into 16-bit two's complement values
        x_raw = (raw_data[1] << 8) | raw_data[0];
        y_raw = (raw_data[3] << 8) | raw_data[2];
        z_raw = (raw_data[5] << 8) | raw_data[4];

        // Convert to g (3.9 mg per LSB in FULL_RES Â±16g mode)
        x_accel_g = x_raw * 0.0039f;
        y_accel_g = y_raw * 0.0039f;
        z_accel_g = z_raw * 0.0039f;

        printf("X=%5dmg  Y=%5dmg  Z=%5dmg\n",
               (int)(x_accel_g * 1000),
               (int)(y_accel_g * 1000),
               (int)(z_accel_g * 1000));

        // Drive LEDs based on combined tilt vector
        combined_led_control(x_accel_g, y_accel_g);

        thread_sleep_for(150);   // 150 ms update rate
    }
}
