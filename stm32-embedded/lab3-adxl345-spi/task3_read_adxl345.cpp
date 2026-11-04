#include "mbed.h"

// --------------------------------------------------
// SPI Pin Definitions (NUCLEO-L432KC)
// --------------------------------------------------
#define ADXL_MOSI A6
#define ADXL_MISO A5
#define ADXL_SCLK A4
#define ADXL_CS D12

// --------------------------------------------------
// ADXL345 Register Addresses
// --------------------------------------------------
#define POWER_CTL_REG 0x2D
#define DATA_FORMAT_REG 0x31
#define BW_RATE_REG 0x2C
#define DATAX0_REG 0x32

// --------------------------------------------------
// SPI Object
// --------------------------------------------------
SPI adxl345_spi(ADXL_MOSI, ADXL_MISO, ADXL_SCLK);

// Chip Select
DigitalOut cs(ADXL_CS);

// --------------------------------------------------
// Write to ADXL345 Register
// --------------------------------------------------
void ADXL345_Write(char address, char value)
{
    cs = 0;
    adxl345_spi.write(address);
    adxl345_spi.write(value);
    cs = 1;
}

// --------------------------------------------------
// Read Multiple Bytes from ADXL345
// --------------------------------------------------
void ADXL345_Read(char startAddress, char* buffer, int size)
{
    // Read bit + multi-byte bit
    char address = 0x80 | 0x40 | startAddress;

    cs = 0;
    adxl345_spi.write(address);

    for (int i = 0; i < size; i++)
    {
        buffer[i] = adxl345_spi.write(0x00);
    }
    cs = 1;
}

// --------------------------------------------------
// Initialise ADXL345
// --------------------------------------------------
void ADXL345_SPI_Initialise()
{
    // SPI format: 8-bit data, SPI mode 3
    adxl345_spi.format(8, 3);

    // SPI frequency = 2 MHz
    adxl345_spi.frequency(2000000);

    // CS inactive high
    cs = 1;

    // DATA_FORMAT: FULL_RES + +/-16g
    ADXL345_Write(DATA_FORMAT_REG, 0x0B);

    // POWER_CTL: Measurement mode
    ADXL345_Write(POWER_CTL_REG, 0x08);

    // BW_RATE: 25 Hz output rate
    ADXL345_Write(BW_RATE_REG, 0x08);
}

// --------------------------------------------------
// Main Program
// --------------------------------------------------
int main()
{
    char data[6];
    int16_t xRaw, yRaw, zRaw;
    float x, y, z;

    printf("ADXL345 Starting...\n");

    // Initialise accelerometer
    ADXL345_SPI_Initialise();

    printf("Initialisation Complete\n");

    while (1)
    {
        // Read 6 bytes starting from DATAX0
        ADXL345_Read(DATAX0_REG, data, 6);

        // Combine bytes into signed 16-bit values
        xRaw = (data[1] << 8) | data[0];
        yRaw = (data[3] << 8) | data[2];
        zRaw = (data[5] << 8) | data[4];

        // Convert to acceleration in g (3.9 mg per LSB for ±16g mode)
        x = xRaw * 0.0039f;
        y = yRaw * 0.0039f;
        z = zRaw * 0.0039f;

        // Print in milli-g
        printf("X = %d mg   Y = %d mg   Z = %d mg\n", 
               (int)(x * 1000), (int)(y * 1000), (int)(z * 1000));

        // Delay 200 ms
        thread_sleep_for(200);
    }
   
}
