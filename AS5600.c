/**
 ******************************************************************************
 * @file           : AS5600.c
 * @author         : Natalia Ag�ero
 * @date           : 19/10/2024
 * @brief          : AS5600 sensor
 ******************************************************************************
 * @attention
 * 
 * Este archivo contiene la implementaci�n del driver para el sensor AS5600,
 * utilizado para la medici�n del �ngulo en el sistema de direcci�n del coche.
 * 
 ******************************************************************************
 */
 
#include "AS5600.h"

int32_t AS5600_Init(I2C_used_t I2C_line) {
		

int32_t I2C1_Configure(I2C_configuration_t configuration) {
		uint32_t status = ARM_DRIVER_OK;

    // Inicializa el controlador I2C
    status |= Driver_I2C1.Initialize(I2C1_callback);
    status |= Driver_I2C1.PowerControl(ARM_POWER_FULL);
    status |= Driver_I2C1.Control(ARM_I2C_BUS_SPEED, configuration.speed);  // Usar velocidad r�pida (400kHz)

//    // Verificar si el AS5600 responde a su direcci�n I2C
//    if (Driver_I2C1.MasterTransmit(AS5600_I2C_ADDRESS, NULL, 0, false) != ARM_DRIVER_OK) {
//        return ARM_DRIVER_ERROR;  // Error si no responde
//    }

    return status;  // El dispositivo est� listo
}

static void I2C1_callback(uint32_t event){
    switch (event) {
			case ARM_I2C_EVENT_TRANSFER_DONE:
			//todo okay    
			break;
			case ARM_I2C_EVENT_TRANSFER_INCOMPLETE:
        /*  Occurs together with ARM_I2C_EVENT_TRANSFER_DONE when 
						less data is transferred then requested. */
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
			case ARM_I2C_EVENT_ADDRESS_NACK:
        /*  Occurs in master mode when address is not acknowledged from slave.*/
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
			break;
			default:
			//others errors
			
			break;
		}
		//siempre se libera el samaforo
		osSemaphoreRelease(transfer_I2C_semaphore);  // Notificar transferencia completa
}

uint32_t IC1_ReadRegister (uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t* data) {
  // Adquirir el mutex antes de acceder al I2C
	osMutexAcquire(I2C1_mutex, osWaitForever);
	//adquiere el semaforo de transferencias
	osSemaphoreAcquire(transfer_I2C_semaphore, osWaitForever);
	
  // Master command
  if (drv_I2C->MasterTransmit(SLAVE_ADDRESS, &reg, 1, false) != ARM_DRIVER_OK) {
     osMutexRelease(I2C1_mutex);
     return ARM_DRIVER_ERROR;  // Error en la transmisi�n
  }
	/* Wait until transfer completed */
  osSemaphoreAcquire(transfer_I2C_semaphore, osWaitForever);
	
	//Slave response
	if (drv_I2C->MasterReceive(SLAVE_ADDRESS, data, 1, false) != ARM_DRIVER_OK) {
    osMutexRelease(I2C1_mutex);
    return ARM_DRIVER_ERROR;  // Error en la recepci�n
  }
	
  osMutexRelease(I2C1_mutex);  // Liberar el mutex despu�s de usar el I2C

	return ARM_DRIVER_OK;
}


uint32_t IC1_ReadRegisters (uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t* data, uint8_t size) {
  // Adquirir el mutex antes de acceder al I2C
	osMutexAcquire(I2C1_mutex, osWaitForever);
	//adquiere el semaforo de transferencias
	osSemaphoreAcquire(transfer_I2C_semaphore, osWaitForever);
	
  // Master command
  if (drv_I2C->MasterTransmit(SLAVE_ADDRESS, &reg, 1, false) != ARM_DRIVER_OK) {
     osMutexRelease(I2C1_mutex);
     return ARM_DRIVER_ERROR;  // Error en la transmisi�n
  }
	/* Wait until transfer completed */
  osSemaphoreAcquire(transfer_I2C_semaphore, osWaitForever);
	
	//Slave response
	if (drv_I2C->MasterReceive(SLAVE_ADDRESS, data, size, false) != ARM_DRIVER_OK) {
    osMutexRelease(I2C1_mutex);
    return ARM_DRIVER_ERROR;  // Error en la recepci�n
  }
  osMutexRelease(I2C1_mutex);  // Liberar el mutex despu�s de usar el I2C
	
	return ARM_DRIVER_OK;
}
uint32_t IC1_WriteRegister (uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t data) {
	uint8_t aux [2] = {reg, data};
	
	// Adquirir el mutex antes de acceder al I2C
	osMutexAcquire(I2C1_mutex, osWaitForever);
	//adquiere el semaforo de transferencias
	osSemaphoreAcquire(transfer_I2C_semaphore, osWaitForever);
	
	//Master command
  if (drv_I2C->MasterTransmit(SLAVE_ADDRESS, aux, 2, false) != ARM_DRIVER_OK) {
    osMutexRelease(I2C1_mutex);
    return ARM_DRIVER_ERROR;  // Error en la transmisi�n
  }
	
  osMutexRelease(I2C1_mutex);  // Liberar el mutex despu�s de usar el I2C
	
  return ARM_DRIVER_OK;
}
