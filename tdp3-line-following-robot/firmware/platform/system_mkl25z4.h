#ifndef SYSTEM_MKL25Z4_H
#define SYSTEM_MKL25Z4_H

#include <stdbool.h>
#include <stdint.h>

extern uint32_t SystemCoreClock;   /* core clock in Hz (48 MHz normally) */
extern uint32_t PeripheralClock;   /* TPM / UART0 clock in Hz (48 MHz normally) */
extern bool     g_clock_fallback;  /* true if the crystal/PLL failed to start */

void SystemInit(void);

#endif
