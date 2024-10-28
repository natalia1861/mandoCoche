/**
 ******************************************************************************
 * @file           : nak_Driver_I2C.c
 * @author         : Natalia Agüero
 * @date           : 19/10/2024
 * @brief          : Driver I2C
 ******************************************************************************
 * @attention
 * 
 * Este archivo contiene la implementación del driver I2C,
 * 
 ******************************************************************************
 */
 
#include "nak_Driver_I2C.h"

//driver I2C
extern ARM_DRIVER_I2C            	Driver_I2C1;
extern ARM_DRIVER_I2C							Driver_I2C2;
extern ARM_DRIVER_I2C							Driver_I2C3;
extern ARM_DRIVER_I2C							Driver_I2C4;
extern ARM_DRIVER_I2C							Driver_I2C5;
extern ARM_DRIVER_I2C							Driver_I2C6;


typedef struct {
	ARM_DRIVER_I2C *driver;
	osMutexId_t mutex_I2C;
	osSemaphoreId_t transfer_I2C_semaphore;
	ARM_I2C_SignalEvent_t Callback_I2C;
	bool initialized;
} I2C_Driver_t;

static I2C_Driver_t I2C_Drivers[I2C_MAX] = 	{ //NO SE DE DONDE ME LO PILLA
		{&Driver_I2C1, NULL, NULL, NULL, false},
		{&Driver_I2C2, NULL, NULL, NULL, false},
		{&Driver_I2C3, NULL, NULL, NULL, false},
		{&Driver_I2C4, NULL, NULL, NULL, false},
		{&Driver_I2C5, NULL, NULL, NULL, false},
};

int32_t I2C1_Init(I2C_used_t I2C_line) {
	//chek de que no esta inicializado
		if (I2C_Drivers[I2C_line].initialized) {
			return ARM_DRIVER_OK;
		}
		
		uint32_t status = ARM_DRIVER_OK;
    // Inicializa el controlador I2C
    status |= I2C_Drivers[I2C_line].driver->Initialize(I2C_Drivers[I2C_line].Callback_I2C);
    status |= I2C_Drivers[I2C_line].driver->PowerControl(ARM_POWER_FULL);
    status |= I2C_Drivers[I2C_line].driver->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);  // Usar velocidad rápida (400kHz) por defecto

//    // Verificar si el AS5600 responde a su dirección I2C
//    if (Driver_I2C1.MasterTransmit(AS5600_I2C_ADDRESS, NULL, 0, false) != ARM_DRIVER_OK) {
//        return ARM_DRIVER_ERROR;  // Error si no responde
//    }

		// Crear mutex para controlar el acceso al BUS I2C1
    if ((I2C_Drivers[I2C_line].mutex_I2C = osMutexNew(NULL))== NULL) {
			status |= ARM_DRIVER_ERROR;
		}			
    
    // Crear semáforo
    if ((I2C_Drivers[I2C_line].transfer_I2C_semaphore = osSemaphoreNew(1, 0, NULL))==NULL) {  // Inicialmente bloqueado (valor 0)
			status |= ARM_DRIVER_ERROR;
		}
    return status;  // El dispositivo está listo
}

//a aprtir de aqui falta!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! emter la interfaz
int32_t I2C1_Configure(I2C_configuration_t configuration) {
		uint32_t status = ARM_DRIVER_OK;

    // Inicializa el controlador I2C
    status |= Driver_I2C1.Initialize(I2C1_callback);
    status |= Driver_I2C1.PowerControl(ARM_POWER_FULL);
    status |= Driver_I2C1.Control(ARM_I2C_BUS_SPEED, configuration.speed);  // Usar velocidad rápida (400kHz)

//    // Verificar si el AS5600 responde a su dirección I2C
//    if (Driver_I2C1.MasterTransmit(AS5600_I2C_ADDRESS, NULL, 0, false) != ARM_DRIVER_OK) {
//        return ARM_DRIVER_ERROR;  // Error si no responde
//    }

    return status;  // El dispositivo está listo
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

static void timeout_callback () {
	
}

uint32_t IC1_ReadRegister (uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t* data) {
  // Adquirir el mutex antes de acceder al I2C
	osMutexAcquire(I2C1_mutex, osWaitForever);
	//adquiere el semaforo de transferencias
	osSemaphoreAcquire(transfer_I2C_semaphore, osWaitForever);
	
  // Master command
  if (drv_I2C->MasterTransmit(SLAVE_ADDRESS, &reg, 1, false) != ARM_DRIVER_OK) {
     osMutexRelease(I2C1_mutex);
     return ARM_DRIVER_ERROR;  // Error en la transmisión
  }
	/* Wait until transfer completed */
  osSemaphoreAcquire(transfer_I2C_semaphore, osWaitForever);
	
	//Slave response
	if (drv_I2C->MasterReceive(SLAVE_ADDRESS, data, 1, false) != ARM_DRIVER_OK) {
    osMutexRelease(I2C1_mutex);
    return ARM_DRIVER_ERROR;  // Error en la recepción
  }
	
  osMutexRelease(I2C1_mutex);  // Liberar el mutex después de usar el I2C

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
     return ARM_DRIVER_ERROR;  // Error en la transmisión
  }
	/* Wait until transfer completed */
  osSemaphoreAcquire(transfer_I2C_semaphore, osWaitForever);
	
	//Slave response
	if (drv_I2C->MasterReceive(SLAVE_ADDRESS, data, size, false) != ARM_DRIVER_OK) {
    osMutexRelease(I2C1_mutex);
    return ARM_DRIVER_ERROR;  // Error en la recepción
  }
  osMutexRelease(I2C1_mutex);  // Liberar el mutex después de usar el I2C
	
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
    return ARM_DRIVER_ERROR;  // Error en la transmisión
  }
	
  osMutexRelease(I2C1_mutex);  // Liberar el mutex después de usar el I2C
	
  return ARM_DRIVER_OK;
}
