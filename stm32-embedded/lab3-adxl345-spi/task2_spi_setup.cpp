#include "mbed.h"

// --------------------------------------------------
// SPI Pin Definitions
// --------------------------------------------------
#define SPI_MOSI A6
#define SPI_MISO A5
#define SPI_SCLK A4
#define SPI_CS   D12

// --------------------------------------------------
// ADXL345 Register Addresses
// --------------------------------------------------
#define DEVID_REG       0x00
#define POWER_CTL_REG   0x2D
#define DATA_FORMAT_REG 0x31
#define BW_RATE_REG     0x2C

// --------------------------------------------------
// SPI Object
// --------------------------------------------------
SPI adxl345_spi(SPI_MOSI, SPI_MISO, SPI_SCLK);

// Chip Select Pin
DigitalOut cs(SPI_CS);

// --------------------------------------------------
// Function: Write to ADXL345 Register
// --------------------------------------------------
void ADXL345_Write(unsigned char address, unsigned char value)
{
    cs = 0;                        // Select device
    adxl345_spi.write(address);   // Send register address
    adxl345_spi.write(value);     // Send data
    cs = 1;                        // Deselect device
}

// --------------------------------------------------
// Function: Initialise ADXL345
// --------------------------------------------------
void ADXL345_SPI_Initialise()
{
    // SPI format:
    // 8-bit data, Mode 3
    adxl345_spi.format(8, 3);

    // 2 MHz SPI clock
    adxl345_spi.frequency(2000000);

    cs = 1;

    // --------------------------------------------------
    // DATA_FORMAT Register (0x31)
    //
    // FULL_RES = 1
    // Justify = 0 (right justified)
    // Range = 11 (+/-16g)
    // SPI = 0 (4-wire SPI)
    //
    // Binary: 00001011 = 0x0B
    // --------------------------------------------------
    ADXL345_Write(DATA_FORMAT_REG, 0x0B);

    // --------------------------------------------------
    // POWER_CTL Register (0x2D)
    //
    // Link = 0
    // AUTO_SLEEP = 0
    // Measure = 1
    // Sleep = 0
    //
    // Binary: 00001000 = 0x08
    // --------------------------------------------------
    ADXL345_Write(POWER_CTL_REG, 0x08);

    // --------------------------------------------------
    // BW_RATE Register (0x2C)
    //
    // Normal mode
    // Output data rate = 8 Hz
    //
    // Binary: 00001000 = 0x08
    // --------------------------------------------------
    ADXL345_Write(BW_RATE_REG, 0x08);
}

// --------------------------------------------------
// Main Program
// --------------------------------------------------
int main()
{
    printf("Initialising ADXL345...\n");

    ADXL345_SPI_Initialise();

    printf("ADXL345 Initialised Successfully\n");

    while (1)
    {
        // Do nothing for Task 2
    }
}
