#ifndef __LCD_H
#define __LCD_H

#include "cmsis_os2.h" 
#include "stm32f4xx_hal.h"
#include "Driver_SPI.h"
#include <stdio.h>

#define SIZE_MSGQUEUE_LCD			3

typedef struct {                               
  uint8_t nLin;
	char buf[24];
} MSGQUEUE_LCD_t;


void initModLCD(void);
osMessageQueueId_t getModLCDQueueID(void);


void Init_Th_LCD_Test(void);


#endif

