#ifndef __GAS_H
#define __GAS_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include <stdio.h>

#define GAS_FLAG 												0x08

typedef struct{
  float gas;
} MSGQUEUE_GAS_t;


void Init_ModGas(void);
void leer_gas (void);
osMessageQueueId_t getGasQueueID(void);


#endif
