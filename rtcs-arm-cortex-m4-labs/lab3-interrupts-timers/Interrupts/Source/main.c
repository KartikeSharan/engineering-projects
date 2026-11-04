#include <stdlib.h>
#include <stdint.h>
#include "../Drivers/platform.h"
#include "stm32f4xx_hal.h"

/*
LED3-PD13-ORANGE
LED4-PD12-GREEN
LED5-PD14-RED
LED6-PD15-BLUE
*/

int MAIN_Output=0;
//int ISR_Output=0;
int count=0;

void Buttons_ISR(void){
	//ISR_Output=1;
	MCU_Output_Set(P_DBG_ISR,MCU_HIGH);
	count++;
	MCU_Output_Set(P_DBG_ISR,MCU_LOW);
	//ISR_Output=0;
}

int main(void)
{
	Board_Initialization();
	LED_Initialization();
	MCU_Output_Initialization();
	Buttons_Initialization();
	//Buttons_Interrupt(&EXTI0_IRQHandler);
	Buttons_Interrupt(&Buttons_ISR);
	
	__enable_irq();
	
	while (1) {
		MCU_Output_toggle(P_DBG_MAIN);
		//MAIN_Output=!MAIN_Output;
		
		GPIO_LED_Set(LED6,(count&1)>>0);
		GPIO_LED_Set(LED5,(count&2)>>1);
		GPIO_LED_Set(LED3,(count&4)>>2);
		GPIO_LED_Set(LED4,(count&8)>>3);
	}
}

// *******************************ARM University Program Copyright © ARM Ltd 2014*************************************   
