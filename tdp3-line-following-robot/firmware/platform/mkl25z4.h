/*
 * mkl25z4.h - minimal register map for the NXP MKL25Z128VLK4 (FRDM-KL25Z).
 *
 * Only the peripherals this project uses are defined. Addresses and bit
 * positions are from the KL25 Sub-Family Reference Manual (KL25P80M48SF0RM).
 * If you need another peripheral (ADC, PIT, I2C ...) add it here in the same
 * style, checking the address against the reference manual.
 */
#ifndef MKL25Z4_H
#define MKL25Z4_H

#include <stdint.h>

#define REG32(addr) (*(volatile uint32_t *)(addr))
#define REG8(addr)  (*(volatile uint8_t *)(addr))

/* ------------------------------------------------------------------ SIM -- */
#define SIM_SOPT2    REG32(0x40048004u)
#define SIM_SCGC4    REG32(0x40048034u)
#define SIM_SCGC5    REG32(0x40048038u)
#define SIM_SCGC6    REG32(0x4004803Cu)
#define SIM_CLKDIV1  REG32(0x40048044u)
#define SIM_COPC     REG32(0x40048100u)

#define SIM_SOPT2_PLLFLLSEL       (1u << 16)
#define SIM_SOPT2_TPMSRC(x)       (((uint32_t)(x) & 3u) << 24)
#define SIM_SOPT2_TPMSRC_MASK     (3u << 24)
#define SIM_SOPT2_UART0SRC(x)     (((uint32_t)(x) & 3u) << 26)
#define SIM_SOPT2_UART0SRC_MASK   (3u << 26)

#define SIM_SCGC4_UART0           (1u << 10)
#define SIM_SCGC5_PORTA           (1u << 9)
#define SIM_SCGC5_PORTB           (1u << 10)
#define SIM_SCGC5_PORTC           (1u << 11)
#define SIM_SCGC5_PORTD           (1u << 12)
#define SIM_SCGC5_PORTE           (1u << 13)
#define SIM_SCGC6_TPM0            (1u << 24)
#define SIM_SCGC6_TPM1            (1u << 25)
#define SIM_SCGC6_TPM2            (1u << 26)

#define SIM_CLKDIV1_OUTDIV1(x)    (((uint32_t)(x) & 0xFu) << 28)
#define SIM_CLKDIV1_OUTDIV4(x)    (((uint32_t)(x) & 0x7u) << 16)

/* ------------------------------------------------------------------ MCG -- */
#define MCG_C1       REG8(0x40064000u)
#define MCG_C2       REG8(0x40064001u)
#define MCG_C4       REG8(0x40064003u)
#define MCG_C5       REG8(0x40064004u)
#define MCG_C6       REG8(0x40064005u)
#define MCG_S        REG8(0x40064006u)

#define MCG_C1_CLKS(x)            ((uint8_t)(((x) & 3u) << 6))
#define MCG_C1_FRDIV(x)           ((uint8_t)(((x) & 7u) << 3))
#define MCG_C1_IREFS              ((uint8_t)(1u << 2))
#define MCG_C2_RANGE0(x)          ((uint8_t)(((x) & 3u) << 4))
#define MCG_C2_EREFS0             ((uint8_t)(1u << 2))
#define MCG_C4_DMX32              ((uint8_t)(1u << 7))
#define MCG_C4_DRST_DRS_MASK      ((uint8_t)(3u << 5))
#define MCG_C5_PRDIV0(x)          ((uint8_t)((x) & 0x1Fu))
#define MCG_C6_PLLS               ((uint8_t)(1u << 6))
#define MCG_C6_VDIV0(x)           ((uint8_t)((x) & 0x1Fu))
#define MCG_S_LOCK0               ((uint8_t)(1u << 6))
#define MCG_S_PLLST               ((uint8_t)(1u << 5))
#define MCG_S_IREFST              ((uint8_t)(1u << 4))
#define MCG_S_CLKST_MASK          ((uint8_t)(3u << 2))
#define MCG_S_CLKST(x)            ((uint8_t)(((x) & 3u) << 2))
#define MCG_S_OSCINIT0            ((uint8_t)(1u << 1))

/* ------------------------------------------------------------------ OSC -- */
#define OSC0_CR      REG8(0x40065000u)

/* ----------------------------------------------------------------- PORT -- */
typedef struct {
    volatile uint32_t PCR[32];      /* 0x00 - 0x7C pin control registers */
    volatile uint32_t GPCLR;        /* 0x80 */
    volatile uint32_t GPCHR;        /* 0x84 */
    uint32_t          RESERVED0[6]; /* 0x88 - 0x9C */
    volatile uint32_t ISFR;         /* 0xA0 */
} PORT_Type;

#define PORTA ((PORT_Type *)0x40049000u)
#define PORTB ((PORT_Type *)0x4004A000u)
#define PORTC ((PORT_Type *)0x4004B000u)
#define PORTD ((PORT_Type *)0x4004C000u)
#define PORTE ((PORT_Type *)0x4004D000u)

#define PORT_PCR_MUX(x)           (((uint32_t)(x) & 7u) << 8)
#define PORT_PCR_DSE              (1u << 6)   /* high drive strength */
#define PORT_PCR_PE               (1u << 1)   /* pull enable */
#define PORT_PCR_PS               (1u << 0)   /* 1 = pull-up, 0 = pull-down */

/* ----------------------------------------------------------------- GPIO -- */
typedef struct {
    volatile uint32_t PDOR;   /* 0x00 data output */
    volatile uint32_t PSOR;   /* 0x04 set output */
    volatile uint32_t PCOR;   /* 0x08 clear output */
    volatile uint32_t PTOR;   /* 0x0C toggle output */
    volatile uint32_t PDIR;   /* 0x10 data input */
    volatile uint32_t PDDR;   /* 0x14 data direction (1 = output) */
} GPIO_Type;

#define GPIOA ((GPIO_Type *)0x400FF000u)
#define GPIOB ((GPIO_Type *)0x400FF040u)
#define GPIOC ((GPIO_Type *)0x400FF080u)
#define GPIOD ((GPIO_Type *)0x400FF0C0u)
#define GPIOE ((GPIO_Type *)0x400FF100u)

/* ------------------------------------------------------------------ TPM -- */
typedef struct {
    volatile uint32_t CnSC;   /* channel status and control */
    volatile uint32_t CnV;    /* channel value (compare / duty) */
} TPM_Channel_Type;

typedef struct {
    volatile uint32_t SC;               /* 0x00 status and control */
    volatile uint32_t CNT;              /* 0x04 counter */
    volatile uint32_t MOD;              /* 0x08 modulo (period - 1) */
    TPM_Channel_Type  CH[6];            /* 0x0C - 0x38 */
    uint32_t          RESERVED0[5];     /* 0x3C - 0x4C */
    volatile uint32_t STATUS;           /* 0x50 */
    uint32_t          RESERVED1[12];    /* 0x54 - 0x80 */
    volatile uint32_t CONF;             /* 0x84 */
} TPM_Type;

#define TPM0 ((TPM_Type *)0x40038000u)
#define TPM1 ((TPM_Type *)0x40039000u)
#define TPM2 ((TPM_Type *)0x4003A000u)

#define TPM_SC_PS(x)              ((uint32_t)(x) & 7u)
#define TPM_SC_CMOD(x)            (((uint32_t)(x) & 3u) << 3)
#define TPM_CnSC_MSB              (1u << 5)
#define TPM_CnSC_MSA              (1u << 4)
#define TPM_CnSC_ELSB             (1u << 3)
#define TPM_CnSC_ELSA             (1u << 2)

/* ---------------------------------------------------------------- UART0 -- */
#define UART0_BDH    REG8(0x4006A000u)
#define UART0_BDL    REG8(0x4006A001u)
#define UART0_C1     REG8(0x4006A002u)
#define UART0_C2     REG8(0x4006A003u)
#define UART0_S1     REG8(0x4006A004u)
#define UART0_D      REG8(0x4006A007u)
#define UART0_C4     REG8(0x4006A00Au)
#define UART0_C5     REG8(0x4006A00Bu)

#define UART0_C2_TE               ((uint8_t)(1u << 3))
#define UART0_C2_RE               ((uint8_t)(1u << 2))
#define UART0_S1_TDRE             ((uint8_t)(1u << 7))
#define UART0_C5_BOTHEDGE         ((uint8_t)(1u << 1))

/* -------------------------------------------------- Cortex-M0+ SysTick -- */
#define SYST_CSR     REG32(0xE000E010u)
#define SYST_RVR     REG32(0xE000E014u)
#define SYST_CVR     REG32(0xE000E018u)

#define SYST_CSR_ENABLE           (1u << 0)
#define SYST_CSR_TICKINT          (1u << 1)
#define SYST_CSR_CLKSOURCE        (1u << 2)   /* 1 = processor clock */

static inline void irq_disable(void) { __asm volatile ("cpsid i" ::: "memory"); }
static inline void irq_enable(void)  { __asm volatile ("cpsie i" ::: "memory"); }
static inline void wait_for_interrupt(void) { __asm volatile ("wfi"); }

#endif /* MKL25Z4_H */
