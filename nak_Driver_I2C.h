#ifndef __nak_Driver_I2C_H
#define __nak_Driver_I2C_H

/**
 ******************************************************************************
 * @file           : nak_Driver_I2C.h
 * @author         : Natalia Agüero
 * @date           : 19/10/2024
 * @brief          : Definiciones y funciones del driver I2C
 ******************************************************************************
 * @attention
 * 
 * 
 ******************************************************************************
 */

#include "Driver_I2C.h"
#include "cmsis_os2.h"

#define BUS_I2C_100K_SPEED							ARM_I2C_BUS_SPEED_STANDARD
#define BUS_I2C_400K_SPEED							ARM_I2C_BUS_SPEED_FAST
#define BUS_I2C_1M_SPEED								ARM_I2C_BUS_SPEED_FAST_PLUS
#define BUS_I2C_3_4_M_SPEED							ARM_I2C_BUS_SPEED_HIGH 

//#define AS5600_I2C_ADDRESS					0x36
//#define AS5600_ANGLE_REG    				0x0E

#define FLAG_TRANSFER_COMPLETE					0x01

typedef enum {
	I2C1_Line,
	I2C2_Line,
	I2C3_Line,
	I2C4_Line,
	I2C5_Line,
	I2C6_Line,
	I2C_MAX,
} I2C_used_t;

//hacer algo parecido para gestion de errores

//typedef enum {
//	ARM_DRIVER_OK,
//	ARM_DRIVER_ERROR,
//	ARM_DRIVER_ERROR_BUSY,
//	ARM_DRIVER_ERROR_TIMEOUT ,
//	ARM_DRIVER_ERROR_UNSUPPORTED,
//	ARM_DRIVER_ERROR_PARAMETER,
//	ARM_DRIVER_ERROR_SPECIFIC
//} status_error_codes_t;
 
typedef struct {
	uint32_t speed;
} I2C_configuration_t;

//int32_t I2C1_Init(void);
//int32_t I2C1_Configure(I2C_configuration_t configuration);


#endif
