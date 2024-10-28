#ifndef __NA_SPI_DRIVER_H
#define __NA_SPI_DRIVER_H

#include "Driver_SPI.h"
#include "stm32f4xx_hal.h"

typedef struct {
	GPIO_TypeDef  *pin;
	GPIO_InitTypeDef *port;
} GPIO_PIN_PORT_t;

#endif
/*					END OF FILE					*/