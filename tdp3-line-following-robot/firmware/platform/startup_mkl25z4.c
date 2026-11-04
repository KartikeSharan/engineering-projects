/*
 * startup_mkl25z4.c - vector table, flash configuration field and reset
 * handler for the MKL25Z128VLK4.
 *
 * You should not need to edit this file. Interrupt handlers are declared
 * "weak", so to use an interrupt just write a function with the same name
 * (e.g. void PIT_IRQHandler(void) { ... }) anywhere in src/.
 */
#include <stdint.h>
#include "mkl25z4.h"

/* Symbols defined by the linker script (platform/mkl25z128.ld). */
extern uint32_t __StackTop;
extern uint32_t _sidata;   /* start of .data initial values in flash */
extern uint32_t _sdata;    /* start of .data in RAM */
extern uint32_t _edata;    /* end of .data in RAM */
extern uint32_t _sbss;     /* start of .bss */
extern uint32_t _ebss;     /* end of .bss */

int  main(void);
void SystemInit(void);

void Reset_Handler(void);
void Default_Handler(void);

#define WEAK_ALIAS __attribute__((weak, alias("Default_Handler")))

/* Cortex-M0+ core exceptions */
void NMI_Handler(void)        __attribute__((weak));
void HardFault_Handler(void)  WEAK_ALIAS;
void SVC_Handler(void)        WEAK_ALIAS;
void PendSV_Handler(void)     WEAK_ALIAS;
void SysTick_Handler(void)    WEAK_ALIAS;

/* KL25Z peripheral interrupts (IRQ 0..31) */
void DMA0_IRQHandler(void)        WEAK_ALIAS;
void DMA1_IRQHandler(void)        WEAK_ALIAS;
void DMA2_IRQHandler(void)        WEAK_ALIAS;
void DMA3_IRQHandler(void)        WEAK_ALIAS;
void FTFA_IRQHandler(void)        WEAK_ALIAS;
void LVD_LVW_IRQHandler(void)     WEAK_ALIAS;
void LLW_IRQHandler(void)         WEAK_ALIAS;
void I2C0_IRQHandler(void)        WEAK_ALIAS;
void I2C1_IRQHandler(void)        WEAK_ALIAS;
void SPI0_IRQHandler(void)        WEAK_ALIAS;
void SPI1_IRQHandler(void)        WEAK_ALIAS;
void UART0_IRQHandler(void)       WEAK_ALIAS;
void UART1_IRQHandler(void)       WEAK_ALIAS;
void UART2_IRQHandler(void)       WEAK_ALIAS;
void ADC0_IRQHandler(void)        WEAK_ALIAS;
void CMP0_IRQHandler(void)        WEAK_ALIAS;
void TPM0_IRQHandler(void)        WEAK_ALIAS;
void TPM1_IRQHandler(void)        WEAK_ALIAS;
void TPM2_IRQHandler(void)        WEAK_ALIAS;
void RTC_IRQHandler(void)         WEAK_ALIAS;
void RTC_Seconds_IRQHandler(void) WEAK_ALIAS;
void PIT_IRQHandler(void)         WEAK_ALIAS;
void USB0_IRQHandler(void)        WEAK_ALIAS;
void DAC0_IRQHandler(void)        WEAK_ALIAS;
void TSI0_IRQHandler(void)        WEAK_ALIAS;
void MCG_IRQHandler(void)         WEAK_ALIAS;
void LPTMR0_IRQHandler(void)      WEAK_ALIAS;
void PORTA_IRQHandler(void)       WEAK_ALIAS;
void PORTD_IRQHandler(void)       WEAK_ALIAS;

typedef void (*vector_t)(void);

/* 16 core entries + 32 peripheral entries = 48 words (0x000 - 0x0BF). */
__attribute__((section(".isr_vector"), used))
const vector_t g_vector_table[48] = {
    (vector_t)(uintptr_t)&__StackTop,   /* initial stack pointer */
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    0, 0, 0, 0, 0, 0, 0,                /* reserved */
    SVC_Handler,
    0, 0,                               /* reserved */
    PendSV_Handler,
    SysTick_Handler,
    /* IRQ 0 .. 31 */
    DMA0_IRQHandler, DMA1_IRQHandler, DMA2_IRQHandler, DMA3_IRQHandler,
    Default_Handler,                    /* 4: reserved */
    FTFA_IRQHandler, LVD_LVW_IRQHandler, LLW_IRQHandler,
    I2C0_IRQHandler, I2C1_IRQHandler, SPI0_IRQHandler, SPI1_IRQHandler,
    UART0_IRQHandler, UART1_IRQHandler, UART2_IRQHandler,
    ADC0_IRQHandler, CMP0_IRQHandler,
    TPM0_IRQHandler, TPM1_IRQHandler, TPM2_IRQHandler,
    RTC_IRQHandler, RTC_Seconds_IRQHandler, PIT_IRQHandler,
    Default_Handler,                    /* 23: reserved */
    USB0_IRQHandler, DAC0_IRQHandler, TSI0_IRQHandler, MCG_IRQHandler,
    LPTMR0_IRQHandler,
    Default_Handler,                    /* 29: reserved */
    PORTA_IRQHandler, PORTD_IRQHandler,
};

/*
 * Flash configuration field at 0x400 - 0x40F.
 * DO NOT CHANGE: the last word sets FSEC = 0xFE (chip UNSECURED) and leaves
 * FOPT at its default. A wrong value here can lock the chip.
 */
__attribute__((section(".flash_config"), used))
const uint32_t g_flash_config[4] = {
    0xFFFFFFFFu,   /* backdoor key 0-3 */
    0xFFFFFFFFu,   /* backdoor key 4-7 */
    0xFFFFFFFFu,   /* FPROT0-3: no flash protection */
    0xFFFFFFFEu,   /* FSEC = 0xFE (unsecured), FOPT = 0xFF, FEPROT, FDPROT */
};

void Reset_Handler(void)
{
    /* The COP watchdog is enabled out of reset and would reset the chip
     * after ~1 s. SIM_COPC is write-once, so disable it straight away. */
    SIM_COPC = 0u;

    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }
    for (dst = &_sbss; dst < &_ebss; ) {
        *dst++ = 0u;
    }

    SystemInit();   /* clocks: see system_mkl25z4.c */
    (void)main();

    for (;;) {
        /* main() should never return */
    }
}

/*
 * PTA4 (Arduino D4) is the NMI pin out of reset. If it is pulled low while
 * the board boots, an NMI fires. Returning here simply ignores it; the
 * firmware then reconfigures PTA4 as a normal GPIO.
 */
void NMI_Handler(void)
{
}

void Default_Handler(void)
{
    for (;;) {
        /* Unexpected interrupt. Attach a debugger to see where we are. */
    }
}
