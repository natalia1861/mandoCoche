#ifndef __TECLADO_H
#define __TECLADO_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include <stdio.h>
#include "LEDs.h"

#define FLAG_LEER_TECLA	0x01
#define FLAG_TECLA 0x04
#define QUEUE_MAX 	5

#define ROW1_PIN GPIO_PIN_2
#define ROW1_PORT GPIOD

#define ROW2_PIN GPIO_PIN_2
#define ROW2_PORT GPIOG

#define ROW3_PIN GPIO_PIN_3
#define ROW3_PORT GPIOG

#define ROW4_PIN GPIO_PIN_1
#define ROW4_PORT GPIOG

#define COL1_PIN GPIO_PIN_9
#define COL1_PORT GPIOC

#define COL2_PIN GPIO_PIN_10
#define COL2_PORT GPIOC

#define COL3_PIN GPIO_PIN_11
#define COL3_PORT GPIOC

#define COL4_PIN GPIO_PIN_12
#define COL4_PORT GPIOC




void Init_ModTEC(void);
osThreadId_t getModTECThreadID(void);
osMessageQueueId_t getMsgTEC(void);
void teclado_on (void);
void teclado_off (void);

typedef struct {                               
	char tecla;
} MSGQUEUE_TEC_t;



#endif
