/*
 * board.h - millisecond timebase and the on-board RGB LED.
 */
#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

typedef enum {
    LED_OFF = 0,
    LED_RED,
    LED_GREEN,
    LED_BLUE,
    LED_YELLOW,   /* red + green */
    LED_CYAN,     /* green + blue */
    LED_MAGENTA,  /* red + blue */
    LED_WHITE,
} led_colour_t;

typedef void (*tick_callback_t)(void);

void     board_init(void);                         /* LEDs + 1 kHz SysTick */
void     board_set_tick_callback(tick_callback_t cb); /* run cb every 1 ms (in the interrupt) */
uint32_t millis(void);                             /* ms since start-up */
void     delay_ms(uint32_t ms);
void     led_set(led_colour_t colour);

#endif /* BOARD_H */
