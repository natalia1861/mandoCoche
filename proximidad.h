#ifndef __SR501_H
#define __SR501_H

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include "cmsis_os2.h"
#include "stdbool.h"

#define FLAG_PROXIMIDAD	0x10

void Init_Prox(void);
#endif
