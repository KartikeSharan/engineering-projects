/*
 * debug_uart.h - debug messages over the FRDM board's USB serial port.
 *
 * UART0 (PTA1/PTA2) is wired on the board to the OpenSDA chip, which shows up
 * on your laptop as a serial port. View it with:  python tdp.py monitor
 *
 * debug_printf supports %d %i %u %x %X %c %s %% with an optional width and
 * zero padding (e.g. %5d, %04x). There is no floating point.
 */
#ifndef DEBUG_UART_H
#define DEBUG_UART_H

#include <stdint.h>

void debug_init(uint32_t baud);
void debug_putc(char c);
void debug_puts(const char *s);
void debug_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#endif /* DEBUG_UART_H */
