#include "mbed.h"

// --------------------------------------------------
// SPI Pin Definitions
// --------------------------------------------------
#define ADXL_MOSI A6
#define ADXL_MISO A5
#define ADXL_SCLK A4
#define ADXL_CS D12

// --------------------------------------------------
// LED Pin Definitions (from Figure Task 1)
// --------------------------------------------------
// X-axis LEDs
DigitalOut lxr1(D0);   // LXR1
DigitalOut lxr2(D1);   // LXR2
DigitalOut lxg1(D11);  // LXG1
DigitalOut lxg2(D9);   // LXG2

// Y-axis LEDs
DigitalOut lyr1(A2);   // LYR1
DigitalOut lyr2(A0);   // LYR2
DigitalOut lyg1(D6);   // LYG1
DigitalOut lyg2(A3);   // LYG2

// --------------------------------------------------
// ADXL345 Register Addresses
// --------------------------------------------------
#define POWER_CTL_REG   0x2D
#define DATA_FORMAT_REG 0x31
#define BW_RATE_REG     0x2C
#define DATAX0_REG      0x32

// --------------------------------------------------
// SPI Object and Chip Select
// --------------------------------------------------
SPI adxl345_spi(ADXL_MOSI, ADXL_MISO, ADXL_SCLK);
DigitalOut cs(ADXL_CS);

// --------------------------------------------------
// Write to ADXL345 Register
// --------------------------------------------------
void ADXL345_Write(char address, char value) {
    cs = 0;
    adxl345_spi.write(address);
    adxl345_spi.write(value);
    cs = 1;
}

// --------------------------------------------------
// Read Multiple Bytes from ADXL345
// --------------------------------------------------
void ADXL345_Read(char startAddress, char* buffer, int size) {
    char address = 0x80 | 0x40 | startAddress;  // read + multi-byte
    cs = 0;
    adxl345_spi.write(address);
    for (int i = 0; i < size; i++) {
        buffer[i] = adxl345_spi.write(0x00);
    }
    cs = 1;
}

// --------------------------------------------------
// Initialise ADXL345
// --------------------------------------------------
void ADXL345_SPI_Initialise() {
    adxl345_spi.format(8, 3);       // 8-bit, mode 3
    adxl345_spi.frequency(2000000); // 2 MHz
    cs = 1;

    ADXL345_Write(DATA_FORMAT_REG, 0x0B);  // FULL_RES, Â±16g
    ADXL345_Write(POWER_CTL_REG, 0x08);    // Measurement mode
    ADXL345_Write(BW_RATE_REG, 0x08);      // 25 Hz output rate
}

// --------------------------------------------------
// Turn off all LEDs (useful before setting new state)
// --------------------------------------------------
void all_leds_off() {
    lxr1 = 0; lxr2 = 0; lxg1 = 0; lxg2 = 0;
    lyr1 = 0; lyr2 = 0; lyg1 = 0; lyg2 = 0;
}

// --------------------------------------------------
// Control LEDs based on X and Y values (in g)
// --------------------------------------------------
void control_leds(float x_g, float y_g) {
    // --- X-axis control (mutually exclusive) ---
    if (x_g < -0.5f) {
        // X less than -0.5g "†’ LXR2 on, all other X LEDs off
        lxr2 = 1; lxr1 = 0; lxg1 = 0; lxg2 = 0;
    }
    else if (x_g < -0.05f) {
        // X between -0.5 and -0.05 "†’ LXR1 on
        lxr1 = 1; lxr2 = 0; lxg1 = 0; lxg2 = 0;
    }
    else if (x_g > 0.5f) {
        // X greater than 0.5g "†’ LXG1 on
        lxg1 = 1; lxr1 = 0; lxr2 = 0; lxg2 = 0;
    }
    else if (x_g > 0.05f) {
        // X between 0.05 and 0.5 "†’ LXG2 on
        lxg2 = 1; lxr1 = 0; lxr2 = 0; lxg1 = 0;
    }
    else {
        // -0.05 "‰¤ X "‰¤ 0.05 "†’ all X LEDs off
        lxr1 = 0; lxr2 = 0; lxg1 = 0; lxg2 = 0;
    }

    // --- Y-axis control (mutually exclusive) ---
    if (y_g < -0.5f) {
        lyr2 = 1; lyr1 = 0; lyg1 = 0; lyg2 = 0;
    }
    else if (y_g < -0.05f) {
        lyr1 = 1; lyr2 = 0; lyg1 = 0; lyg2 = 0;
    }
    else if (y_g > 0.5f) {
        lyg1 = 1; lyr1 = 0; lyr2 = 0; lyg2 = 0;
    }
    else if (y_g > 0.05f) {
        lyg2 = 1; lyr1 = 0; lyr2 = 0; lyg1 = 0;
    }
    else {
        lyr1 = 0; lyr2 = 0; lyg1 = 0; lyg2 = 0;
    }
}

// --------------------------------------------------
// Main Program
// --------------------------------------------------
int main() {
    char data[6];
    int16_t xRaw, yRaw, zRaw;
    float x, y, z;

    printf("ADXL345 Starting...\n");
    ADXL345_SPI_Initialise();
    printf("Initialisation Complete\n");

    all_leds_off();  // start with all LEDs off

    while (1) {
        // Read 6 bytes from DATAX0 register
        ADXL345_Read(DATAX0_REG, data, 6);

        xRaw = (data[1] << 8) | data[0];
        yRaw = (data[3] << 8) | data[2];
        zRaw = (data[5] << 8) | data[4];

        // Convert to g (3.9 mg per LSB for Â±16g mode)
        x = xRaw * 0.0039f;
        y = yRaw * 0.0039f;
        z = zRaw * 0.0039f;

        // Print values (optional, for debugging)
        printf("X = %d mg   Y = %d mg   Z = %d mg\n",
               (int)(x * 1000), (int)(y * 1000), (int)(z * 1000));

        // Update LEDs based on X and Y tilt
        control_leds(x, y);

        thread_sleep_for(200);  // 200 ms delay
    }
}
