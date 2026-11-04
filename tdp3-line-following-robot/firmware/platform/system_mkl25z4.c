/*
 * system_mkl25z4.c - clock setup, called from Reset_Handler before main().
 *
 * Normal case (PEE mode, same as Mbed/NXP SDK for the FRDM-KL25Z):
 *   8 MHz crystal -> /2 = 4 MHz PLL reference -> x24 = 96 MHz PLL
 *   core/system clock = 96 / 2 = 48 MHz
 *   bus/flash clock   = 48 / 2 = 24 MHz
 *   TPM + UART0 clock = PLL / 2 = 48 MHz
 *
 * If the crystal or PLL fails to start (should never happen on a healthy
 * board) the chip falls back to its internal reference (FEI mode, ~21 MHz)
 * so the robot still runs, and g_clock_fallback is set so main() can warn.
 */
#include <stdbool.h>
#include <stdint.h>
#include "mkl25z4.h"
#include "system_mkl25z4.h"

uint32_t SystemCoreClock = 20971520u;  /* core clock in Hz */
uint32_t PeripheralClock = 20971520u;  /* clock feeding TPM and UART0 in Hz */
bool     g_clock_fallback = false;

#define CLOCK_TIMEOUT 200000u

static bool wait_mask(volatile uint8_t *reg, uint8_t mask, uint8_t value)
{
    for (uint32_t i = 0; i < CLOCK_TIMEOUT; i++) {
        if ((*reg & mask) == value) {
            return true;
        }
    }
    return false;
}

static void use_internal_clock(void)
{
    MCG_C6 = 0u;                      /* PLL off */
    MCG_C1 = MCG_C1_IREFS;            /* FLL from the internal 32 kHz reference */
    MCG_C2 = 0u;                      /* external oscillator off */
    (void)wait_mask(&MCG_S, MCG_S_IREFST, MCG_S_IREFST);
    (void)wait_mask(&MCG_S, MCG_S_CLKST_MASK, MCG_S_CLKST(0));

    SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV4(1); /* reset default */
    SIM_SOPT2 = (SIM_SOPT2 & ~(SIM_SOPT2_PLLFLLSEL | SIM_SOPT2_TPMSRC_MASK | SIM_SOPT2_UART0SRC_MASK))
              | SIM_SOPT2_TPMSRC(1) | SIM_SOPT2_UART0SRC(1);       /* FLL clock */

    SystemCoreClock = 20971520u;
    PeripheralClock = 20971520u;
    g_clock_fallback = true;
}

void SystemInit(void)
{
    /* Core = MCGOUT / 2, bus = core / 2. Set before speeding up. */
    SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1) | SIM_CLKDIV1_OUTDIV4(1);

    /* FEI -> FBE: start the 8 MHz crystal and run from it directly. */
    MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_EREFS0;
    OSC0_CR = 0u;
    MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(3);     /* 8 MHz / 256 for the FLL */
    MCG_C4 &= (uint8_t)~(MCG_C4_DMX32 | MCG_C4_DRST_DRS_MASK);
    MCG_C5 = MCG_C5_PRDIV0(1);                     /* PLL reference = 8 / 2 = 4 MHz */
    MCG_C6 = 0u;

    if (!wait_mask(&MCG_S, MCG_S_OSCINIT0, MCG_S_OSCINIT0) ||
        !wait_mask(&MCG_S, MCG_S_IREFST, 0u) ||
        !wait_mask(&MCG_S, MCG_S_CLKST_MASK, MCG_S_CLKST(2))) {
        use_internal_clock();
        return;
    }

    /* FBE -> PBE: enable the PLL (VDIV0 = 0 means x24 -> 96 MHz). */
    MCG_C6 = MCG_C6_PLLS | MCG_C6_VDIV0(0);
    if (!wait_mask(&MCG_S, MCG_S_PLLST, MCG_S_PLLST) ||
        !wait_mask(&MCG_S, MCG_S_LOCK0, MCG_S_LOCK0)) {
        use_internal_clock();
        return;
    }

    /* PBE -> PEE: switch the system clock over to the PLL. */
    MCG_C1 = MCG_C1_CLKS(0) | MCG_C1_FRDIV(3);
    if (!wait_mask(&MCG_S, MCG_S_CLKST_MASK, MCG_S_CLKST(3))) {
        use_internal_clock();
        return;
    }

    /* TPM and UART0 run from MCGPLLCLK / 2 = 48 MHz. */
    SIM_SOPT2 = (SIM_SOPT2 & ~(SIM_SOPT2_TPMSRC_MASK | SIM_SOPT2_UART0SRC_MASK))
              | SIM_SOPT2_PLLFLLSEL | SIM_SOPT2_TPMSRC(1) | SIM_SOPT2_UART0SRC(1);

    SystemCoreClock = 48000000u;
    PeripheralClock = 48000000u;
}
