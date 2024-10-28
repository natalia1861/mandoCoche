#ifndef __HTTP_SERVER_H
#define __HTTP_SERVER_H

#include "cmsis_os2.h" 
#include "stm32f4xx_hal.h"
#include "Driver_SPI.h"
#include <stdio.h>

#define SIZE_MSGQUEUE_LCD			3
#define SIZE_MSGQUEUE_ALERTS			3

#define FLAG_WEB	0x40

typedef struct {   
	char users[5][24];
} MSGQUEUE_USUARIOS_t;

typedef struct {
	uint8_t action;
	uint8_t id[5];
  char name[19];
  uint8_t password[4];
} MSGQUEUE_WEB_t;

typedef struct {   
	char gas[24];
	char fuego[24];
	char piezo[24];
  char pwdAlert[24];
 
} MSGQUEUE_ALERTS_t;


osMessageQueueId_t getWebQueue (void);
osMessageQueueId_t getQueueUsers (void);
osMessageQueueId_t getQueueAlerts (void);

#endif
