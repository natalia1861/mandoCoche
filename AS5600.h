#ifndef __AS5600_H
#define __AS5600_H

/**
 ******************************************************************************
 * @file           : AS5600.h
 * @author         : Natalia Agüero
 * @date           : 19/10/2024
 * @brief          : Definiciones y funciones del driver AS5600 para la dirección del coche
 ******************************************************************************
 * @attention
 * 
 * Este archivo contiene las definiciones, prototipos y macros para el manejo
 * del sensor AS5600, utilizado para la lectura del ángulo en el control de 
 * dirección del coche.
 * 
 ******************************************************************************
 */

#include "nak_Driver_I2C.h"
#include "cmsis_os2.h"

#define AS5600_I2C_Line 						I2C1_Line
#define AS5600_I2C_ADDRESS					0x36
#define AS5600_ANGLE_REG    				0x0E

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

//int32_t I2C1_Init(void);
//int32_t I2C1_Configure(I2C_configuration_t configuration);


#endif
