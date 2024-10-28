#include "LEDs.h"

static GPIO_InitTypeDef GPIO_InitStruct;

void Init_LED3(void){//ROJO
		__HAL_RCC_GPIOB_CLK_ENABLE();
		GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP; 
		GPIO_InitStruct.Pull=GPIO_PULLUP; 
		GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_VERY_HIGH; 
		
		GPIO_InitStruct.Pin=GPIO_PIN_14;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
}
void Init_LED1(void){//VERDE
	  __HAL_RCC_GPIOB_CLK_ENABLE();
		GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP; 
		GPIO_InitStruct.Pull=GPIO_PULLUP; 
		GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_VERY_HIGH; 
		
		GPIO_InitStruct.Pin=GPIO_PIN_0;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
}

void Init_LED2(void){//AZUL
	  __HAL_RCC_GPIOB_CLK_ENABLE();
		GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP; 
		GPIO_InitStruct.Pull=GPIO_PULLUP; 
		GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_VERY_HIGH; 
		
		GPIO_InitStruct.Pin=GPIO_PIN_7;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
}



void Led_verde_set(void){
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
}
void Led_verde_reset(void){
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
}

void Led_azul_set(void){
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
}
void Led_azul_reset(void){
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
}

void Led_rojo_set(void){
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
}
void Led_rojo_reset(void){
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
}
