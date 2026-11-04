/*
 * gpio.h - tiny helpers for driving pins described by pin_t (see pins.h).
 */
#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>
#include "mkl25z4.h"
#include "pins.h"

static inline void pin_clock_on(const pin_t *p)
{
    SIM_SCGC5 |= (1u << p->sim_scgc5_bit_index);
}

/* Route the pin to a peripheral (mux = ALTn number from the datasheet). */
static inline void pin_set_mux(const pin_t *p, uint32_t mux)
{
    pin_clock_on(p);
    p->port->PCR[p->pin] = PORT_PCR_MUX(mux);
}

static inline void pin_write(const pin_t *p, bool high)
{
    if (high) {
        p->gpio->PSOR = (1u << p->pin);
    } else {
        p->gpio->PCOR = (1u << p->pin);
    }
}

/* Configure as a GPIO output. The level is set before the pin starts
 * driving so there is no glitch. */
static inline void pin_make_output(const pin_t *p, bool initial_high)
{
    pin_set_mux(p, 1u);              /* ALT1 = GPIO on every KL25Z pin */
    pin_write(p, initial_high);
    p->gpio->PDDR |= (1u << p->pin);
}

static inline void pin_make_input(const pin_t *p, bool pull_up)
{
    pin_clock_on(p);
    p->port->PCR[p->pin] = PORT_PCR_MUX(1u) | PORT_PCR_PE | (pull_up ? PORT_PCR_PS : 0u);
    p->gpio->PDDR &= ~(1u << p->pin);
}

static inline bool pin_read(const pin_t *p)
{
    return (p->gpio->PDIR & (1u << p->pin)) != 0u;
}

#endif /* GPIO_H */
