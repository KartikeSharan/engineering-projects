/*
 * board.c - SysTick 1 ms timebase and the on-board RGB LED.
 */
#include <stdbool.h>
#include "board.h"
#include "gpio.h"
#include "mkl25z4.h"
#include "pins.h"
#include "system_mkl25z4.h"

static volatile uint32_t s_ms;
static volatile tick_callback_t s_tick_cb;

static const pin_t s_led_red   = PIN_LED_RED;
static const pin_t s_led_green = PIN_LED_GREEN;
static const pin_t s_led_blue  = PIN_LED_BLUE;

void SysTick_Handler(void)
{
    s_ms++;
    tick_callback_t cb = s_tick_cb;
    if (cb) {
        cb();
    }
}

void board_init(void)
{
    /* LEDs are active low: start with them off (pin high). */
    pin_make_output(&s_led_red, true);
    pin_make_output(&s_led_green, true);
    pin_make_output(&s_led_blue, true);

    SYST_CSR = 0u;
    SYST_RVR = (SystemCoreClock / 1000u) - 1u;
    SYST_CVR = 0u;
    SYST_CSR = SYST_CSR_CLKSOURCE | SYST_CSR_TICKINT | SYST_CSR_ENABLE;
}

void board_set_tick_callback(tick_callback_t cb)
{
    s_tick_cb = cb;
}

uint32_t millis(void)
{
    return s_ms;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = s_ms;
    while ((uint32_t)(s_ms - start) < ms) {
        wait_for_interrupt();   /* sleep until the next SysTick */
    }
}

void led_set(led_colour_t colour)
{
    bool r = (colour == LED_RED)   || (colour == LED_YELLOW) || (colour == LED_MAGENTA) || (colour == LED_WHITE);
    bool g = (colour == LED_GREEN) || (colour == LED_YELLOW) || (colour == LED_CYAN)    || (colour == LED_WHITE);
    bool b = (colour == LED_BLUE)  || (colour == LED_CYAN)   || (colour == LED_MAGENTA) || (colour == LED_WHITE);
    pin_write(&s_led_red, !r);
    pin_write(&s_led_green, !g);
    pin_write(&s_led_blue, !b);
}
