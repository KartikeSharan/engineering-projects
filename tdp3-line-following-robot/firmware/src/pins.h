/*
 * pins.h - EVERY wire between the FRDM-KL25Z and the rest of the robot.
 *
 * If you move a wire on the breadboard/PCB, change it here and nowhere else.
 *
 * The motor driver uses Arduino header pins D2-D7 (header J1), in the same
 * order as the L298N input header (ENA, IN1, IN2, IN3, IN4, ENB), so a
 * 6-way jumper/ribbon goes straight across.
 *
 *   FRDM pin   KL25Z pin   Function                L298N    L298 IC pin
 *   --------   ---------   ---------------------   ------   -----------
 *   D2         PTD4        PWM  (TPM0 channel 4)   ENA      6
 *   D3         PTA12       GPIO                    IN1      5
 *   D4         PTA4        GPIO                    IN2      7
 *   D5         PTA5        GPIO                    IN3      10
 *   D6         PTC8        GPIO                    IN4      12
 *   D7         PTC9        PWM  (TPM0 channel 5)   ENB      11
 *   GND        GND         common ground           GND      8
 *
 * D0/D1 (PTA1/PTA2) are UART0, which goes to the USB serial port used for
 * debug messages. Leave them unconnected.
 *
 * Left motor  -> L298N OUT1/OUT2 (channel A)
 * Right motor -> L298N OUT3/OUT4 (channel B)
 */
#ifndef PINS_H
#define PINS_H

#include "mkl25z4.h"

typedef struct {
    PORT_Type *port;
    GPIO_Type *gpio;
    uint8_t    pin;
    uint8_t    sim_scgc5_bit_index; /* bit in SIM_SCGC5 for this port's clock */
} pin_t;

/* Ports: SIM_SCGC5 bit 9 = PORTA, 10 = PORTB, 11 = PORTC, 12 = PORTD */
#define PIN_PTA(n) { PORTA, GPIOA, (n), 9u }
#define PIN_PTB(n) { PORTB, GPIOB, (n), 10u }
#define PIN_PTC(n) { PORTC, GPIOC, (n), 11u }
#define PIN_PTD(n) { PORTD, GPIOD, (n), 12u }

/* ---- L298N motor driver --------------------------------------------- */
/* Channel A = LEFT motor */
#define PIN_ENA        PIN_PTD(4)    /* D2  */
#define PIN_ENA_MUX    4u            /* PTD4 ALT4 = TPM0_CH4 */
#define PIN_ENA_TPM_CH 4u
#define PIN_IN1        PIN_PTA(12)   /* D3  */
#define PIN_IN2        PIN_PTA(4)    /* D4  (also the NMI pin at reset) */

/* Channel B = RIGHT motor */
#define PIN_IN3        PIN_PTA(5)    /* D5  */
#define PIN_IN4        PIN_PTC(8)    /* D6  */
#define PIN_ENB        PIN_PTC(9)    /* D7  */
#define PIN_ENB_MUX    3u            /* PTC9 ALT3 = TPM0_CH5 */
#define PIN_ENB_TPM_CH 5u

/* Both enable pins are on TPM0, so they share one PWM frequency. */
#define MOTOR_TPM      TPM0
#define MOTOR_TPM_SCGC6_BIT SIM_SCGC6_TPM0

/* ---- On-board RGB LED (active low) ------------------------------------ */
#define PIN_LED_RED    PIN_PTB(18)
#define PIN_LED_GREEN  PIN_PTB(19)
#define PIN_LED_BLUE   PIN_PTD(1)

#endif /* PINS_H */
