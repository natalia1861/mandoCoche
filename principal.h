#ifndef __PRIN_H
#define __PRIN_H

#include "cmsis_os2.h" 
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define TIMER30S				0x100
#define ALERTA     0x200
#define ALARMA			0x400

int Init_Thread_principal (void);


#endif
