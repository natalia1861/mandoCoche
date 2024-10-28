#ifndef _ZUM_H
#define _ZUM_H

#include "cmsis_os2.h"  
#include "stm32f4xx_hal.h"

#define FLAG_ALARMA	0x01
#define FLAG_ALERTA 0x02
#define FLAG_ZSTOP 0x04


void initModZumbador(void);
osThreadId_t getModPWMThreadID(void);

#endif
