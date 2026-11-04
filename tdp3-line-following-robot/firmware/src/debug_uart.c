/*
 * debug_uart.c - UART0 transmit-only debug output (blocking).
 */
#include <stdarg.h>
#include <stdbool.h>
#include "debug_uart.h"
#include "mkl25z4.h"
#include "system_mkl25z4.h"

void debug_init(uint32_t baud)
{
    SIM_SCGC4 |= SIM_SCGC4_UART0;
    SIM_SCGC5 |= SIM_SCGC5_PORTA;
    /* UART0 clock source was selected in SystemInit (PeripheralClock). */

    PORTA->PCR[1] = PORT_PCR_MUX(2);   /* PTA1 = UART0_RX */
    PORTA->PCR[2] = PORT_PCR_MUX(2);   /* PTA2 = UART0_TX */

    UART0_C2 = 0u;                     /* transmitter/receiver off while configuring */

    /* Pick the oversampling ratio (4..32) and divider with the smallest error. */
    uint32_t clk = PeripheralClock;
    uint32_t best_osr = 16u, best_sbr = 1u, best_err = 0xFFFFFFFFu;
    for (uint32_t osr = 4u; osr <= 32u; osr++) {
        uint32_t sbr = (clk + (baud * osr) / 2u) / (baud * osr);
        if (sbr == 0u || sbr > 0x1FFFu) {
            continue;
        }
        uint32_t actual = clk / (osr * sbr);
        uint32_t err = (actual > baud) ? (actual - baud) : (baud - actual);
        if (err <= best_err) {             /* on a tie, prefer more oversampling */
            best_err = err;
            best_osr = osr;
            best_sbr = sbr;
        }
    }

    UART0_C4 = (uint8_t)((best_osr - 1u) & 0x1Fu);
    if (best_osr < 8u) {
        UART0_C5 |= UART0_C5_BOTHEDGE;     /* required for low oversampling ratios */
    } else {
        UART0_C5 &= (uint8_t)~UART0_C5_BOTHEDGE;
    }
    UART0_BDH = (uint8_t)((best_sbr >> 8) & 0x1Fu);
    UART0_BDL = (uint8_t)(best_sbr & 0xFFu);
    UART0_C1 = 0u;                         /* 8 data bits, no parity, 1 stop bit */
    UART0_C2 = UART0_C2_TE | UART0_C2_RE;
}

void debug_putc(char c)
{
    if (c == '\n') {
        debug_putc('\r');
    }
    while ((UART0_S1 & UART0_S1_TDRE) == 0u) {
    }
    UART0_D = (uint8_t)c;
}

void debug_puts(const char *s)
{
    while (*s) {
        debug_putc(*s++);
    }
}

static void put_unsigned(uint32_t value, uint32_t base, bool upper, int width, char pad)
{
    char buf[12];
    int n = 0;
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    do {
        buf[n++] = digits[value % base];
        value /= base;
    } while (value != 0u && n < (int)sizeof(buf));
    while (width > n) {
        debug_putc(pad);
        width--;
    }
    while (n > 0) {
        debug_putc(buf[--n]);
    }
}

void debug_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    while (*fmt) {
        char c = *fmt++;
        if (c != '%') {
            debug_putc(c);
            continue;
        }
        char pad = ' ';
        int width = 0;
        if (*fmt == '0') {
            pad = '0';
            fmt++;
        }
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt++ - '0');
        }
        while (*fmt == 'l') {      /* int and long are both 32-bit here */
            fmt++;
        }
        char spec = *fmt;
        if (spec == '\0') {
            break;
        }
        fmt++;
        switch (spec) {
        case 'd':
        case 'i': {
            int v = va_arg(ap, int);
            unsigned int mag = (v < 0) ? (unsigned int)(-(v + 1)) + 1u : (unsigned int)v;
            if (v < 0) {
                if (pad == '0') {
                    debug_putc('-');
                    put_unsigned(mag, 10u, false, width - 1, pad);
                } else {
                    /* count digits so the sign sits next to the number */
                    unsigned int t = mag;
                    int digits = 1;
                    while (t >= 10u) {
                        t /= 10u;
                        digits++;
                    }
                    for (int i = digits + 1; i < width; i++) {
                        debug_putc(' ');
                    }
                    debug_putc('-');
                    put_unsigned(mag, 10u, false, 0, pad);
                }
            } else {
                put_unsigned(mag, 10u, false, width, pad);
            }
            break;
        }
        case 'u':
            put_unsigned(va_arg(ap, unsigned int), 10u, false, width, pad);
            break;
        case 'x':
            put_unsigned(va_arg(ap, unsigned int), 16u, false, width, pad);
            break;
        case 'X':
            put_unsigned(va_arg(ap, unsigned int), 16u, true, width, pad);
            break;
        case 'c':
            debug_putc((char)va_arg(ap, int));
            break;
        case 's': {
            const char *s = va_arg(ap, const char *);
            debug_puts(s ? s : "(null)");
            break;
        }
        case '%':
            debug_putc('%');
            break;
        default:
            debug_putc('%');
            debug_putc(spec);
            break;
        }
    }
    va_end(ap);
}
