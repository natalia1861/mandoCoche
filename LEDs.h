#ifndef __LED_H
#define __LED_H

#include "stm32f4xx_hal.h"

void Init_LED1(void); //VERDE
void Init_LED2(void); //AZUL
void Init_LED3(void); //ROJO

void Led_azul_reset(void);
void Led_verde_reset(void);
void Led_rojo_reset(void);

void Led_verde_set(void);
void Led_azul_set(void);
void Led_rojo_set(void);
	



#endif
