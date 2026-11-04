#include "platform.h"
#include "stm32f4xx_hal.h"

uint32_t prioritygroup;
uint32_t priority;
EXTI_ConfigTypeDef EXTI_Config_InitStruct;
EXTI_HandleTypeDef EXTI_Handle_InitStruct;

static void (*Button_Callback)(void);

void Board_Initialization(void){
	HAL_Init();
}

int32_t LED_Initialization (void) {
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOD_CLK_ENABLE();

  /* Configure GPIO pins: PD12 PD13 PD14 PD15 */
  GPIO_InitStruct.Pin   = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	GPIO_LED_Set(LED3,LED_OFF);
	GPIO_LED_Set(LED4,LED_OFF);
	GPIO_LED_Set(LED5,LED_OFF);
	GPIO_LED_Set(LED6,LED_OFF);

  return 0;
}

int32_t MCU_Output_Initialization (void) {
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOD_CLK_ENABLE();

  /* Configure GPIO pins: PD12 PD13 PD14 PD15 */
  GPIO_InitStruct.Pin   = P_DBG_ISR | P_DBG_MAIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  return 0;
}

void GPIO_LED_Set(uint16_t Pin, GPIO_PinState State){
		HAL_GPIO_WritePin(GPIOD,Pin,State);
}


void MCU_Output_Set(uint16_t Pin, GPIO_PinState State){
		HAL_GPIO_WritePin(GPIOD,Pin,State);
}

void MCU_Output_toggle(uint16_t Pin){
		HAL_GPIO_TogglePin(GPIOD,Pin);
}

int32_t Buttons_Initialization (void) {
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOA_CLK_ENABLE();

  /* Configure GPIO pin: PA0 (USER) */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  return 0;
}

int32_t Buttons_Uninitialize (void) {

  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);

  return 0;
}

int32_t Buttons_Interrupt (void (*callback)(void)) {

	
	EXTI_Config_InitStruct.Line=EXTI_LINE_0;
	EXTI_Config_InitStruct.Mode=EXTI_MODE_INTERRUPT;
	EXTI_Config_InitStruct.Trigger=EXTI_TRIGGER_RISING;
	
	EXTI_Handle_InitStruct.Line=EXTI_LINE_0;
	EXTI_Handle_InitStruct.RisingCallback=callback;
	//EXTI_Handle_InitStruct.FallingCallback=callback;
	
	HAL_EXTI_SetConfigLine(&EXTI_Handle_InitStruct,&EXTI_Config_InitStruct);
	HAL_EXTI_RegisterCallback(&EXTI_Handle_InitStruct, HAL_EXTI_RISING_CB_ID, callback);
	HAL_EXTI_IRQHandler(&EXTI_Handle_InitStruct);
	//__enable_irq();
	
	Button_Callback=callback;
	
	prioritygroup = NVIC_GetPriorityGrouping(); // will return 5
	priority = NVIC_EncodePriority(prioritygroup, 1, 1 ); // Pri=1 , SubPri=1
	NVIC_SetPriority(EXTI0_IRQn, priority);
	NVIC_EnableIRQ(EXTI0_IRQn);
	
	return 0;
	
	}

void EXTI0_IRQHandler(void){
	HAL_EXTI_ClearPending(&EXTI_Handle_InitStruct,EXTI_TRIGGER_RISING);
	Button_Callback();
}

uint32_t Buttons_GetState (void) { 
  uint32_t val = 0;

  if (HAL_GPIO_ReadPin(BUTTONS_USER_PORT, BUTTONS_USER_PIN) == GPIO_PIN_SET) {
    val |= BUTTONS_USER_MASK;
  }

  return val;
}

uint32_t Buttons_GetCount (void) {
  return BUTTONS_COUNT;
}

void delay_ms(unsigned int ms) {
	unsigned int max_step = 1000 * (UINT32_MAX / SystemCoreClock);
	unsigned int max_sleep_cycles = max_step * (SystemCoreClock / 1000);
	while (ms > max_step) {
		ms -= max_step;
		delay_cycles(max_sleep_cycles);
	}
	delay_cycles(ms * (SystemCoreClock / 1000));
}

void delay_us(unsigned int us) {
	unsigned int max_step = 1000000 * (UINT32_MAX / SystemCoreClock);
	unsigned int max_sleep_cycles = max_step * (SystemCoreClock / 1000000);
	while (us > max_step) {
		us -= max_step;
		delay_cycles(max_sleep_cycles);
	}
	delay_cycles(us * (SystemCoreClock / 1000000));
}

__asm void delay_cycles(unsigned int cycles) {
	LSRS r0, #2
	BEQ done
loop
	SUBS r0, #1
#if __CORTEX_M == 3 || __CORTEX_M == 4
	NOP
#endif
	BNE loop
done
	BX lr
}

