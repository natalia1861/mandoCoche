#ifndef __SENSORS_H
#define __SENSORS_H

#include "cmsis_os2.h"

#define FLAG_MEDIR 0x01
#define SENSOR_FLAG 0x02

typedef struct {
	float temp;
	float gas;
	float pres;//Value from piezoelectric
  float consumo;
} MSGQUEUE_SENSORS_t;

int Init_Sensores_peligro (void);
osMessageQueueId_t getSensorsQueue(void);

#endif
