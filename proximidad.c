#include "proximidad.h"

//principal.h
extern osThreadId_t tid_Thread_principal;

//INTERNAL VARIABLES
static GPIO_InitTypeDef GPIO_InitStruct;
volatile bool proximidad_enable = true;

//FUNCTIONS

void Init_Prox(){
		 HAL_NVIC_EnableIRQ(EXTI4_IRQn);

		__HAL_RCC_GPIOA_CLK_ENABLE();
  	GPIO_InitStruct.Pin=GPIO_PIN_4;

	  GPIO_InitStruct.Mode=GPIO_MODE_IT_FALLING; 
		GPIO_InitStruct.Pull=GPIO_NOPULL; 
		GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_VERY_HIGH; 
		
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void EXTI4_IRQHandler(void){
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if(GPIO_Pin==GPIO_PIN_4){
		if (proximidad_enable) {
    osThreadFlagsSet(tid_Thread_principal, FLAG_PROXIMIDAD);
		}
  }
}

