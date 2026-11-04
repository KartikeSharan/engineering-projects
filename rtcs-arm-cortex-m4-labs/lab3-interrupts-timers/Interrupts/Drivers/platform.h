#include "stm32f4xx_hal.h"
#include <stdint.h>

#define LED_ON GPIO_PIN_SET
#define LED_OFF GPIO_PIN_RESET
#define MCU_HIGH GPIO_PIN_SET
#define MCU_LOW GPIO_PIN_RESET
#define PortA GPIOA
#define PortB GPIOB
#define PortC GPIOC
#define PortD GPIOD

#define BUTTONS_COUNT                  (1)
#define BUTTONS_USER_MASK              (1<<0)
#define BUTTONS_USER_PORT               GPIOA
#define BUTTONS_USER_PIN                GPIO_PIN_0

//extern EXTI_ConfigTypeDef EXTI_Config_InitStruct;
//extern EXTI_HandleTypeDef EXTI_Handle_InitStruct;

typedef enum {
	LED3 = GPIO_PIN_13,
	LED4 = GPIO_PIN_12,
	LED5 = GPIO_PIN_14,
	LED6 = GPIO_PIN_15,
	P_DBG_ISR=GPIO_PIN_9,
	P_DBG_MAIN=GPIO_PIN_8,
	B1 = GPIO_PIN_0,
} Pin;

typedef enum {
	OUT_PP=GPIO_MODE_OUTPUT_PP,  
	IN=GPIO_MODE_INPUT,   
} PinMode;

typedef enum {
	VH=GPIO_SPEED_FREQ_VERY_HIGH 
} PinSpeed;

typedef enum {
	PU=GPIO_PULLUP,
	PD=GPIO_PULLDOWN
} PinPP;

void Board_Initialization(void);
int32_t LED_Initialization (void);
int32_t MCU_Output_Initialization (void);
void MCU_Output_Set(uint16_t Pin, GPIO_PinState State);
void MCU_Output_toggle(uint16_t Pin);
void GPIO_LED_Set(uint16_t Pin, GPIO_PinState State);
int32_t  Buttons_Initialization   (void);
int32_t  Buttons_Uninitialize (void);
int32_t Buttons_Interrupt (void (*callback)(void));
void EXTI0_IRQHandler(void);
uint32_t Buttons_GetState     (void);
uint32_t Buttons_GetCount     (void);
void delay_ms(unsigned int ms);
void delay_us(unsigned int us);
__asm void delay_cycles(unsigned int cycles);

