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
#include <stdio.h>
#include "RTE_Device.h"

//definition of I2C depending if its active or not

#if defined (RTE_I2C1) && (RTE_I2C1 == 1)
    #define USE_I2C1
    extern ARM_DRIVER_I2C							Driver_I2C1; 
    static void I2C1_callback(uint32_t event);
    I2C_DriverConfig_t I2C1_DriverConfig = {"I2C1", NULL, NULL, I2C1_callback};
#endif

#if defined (RTE_I2C2) && (RTE_I2C2 == 1)
    #define USE_I2C2
    extern ARM_DRIVER_I2C							Driver_I2C2;
    I2C_Driver_t I2C2_Driver = {"I2C2";
    static void I2C2_callback(uint32_t event);
    I2C_DriverConfig_t I2C2_DriverConfig = {"I2C2", NULL, NULL, I2C2_callback};

#endif

#if defined (RTE_I2C3) && (RTE_I2C3 == 1)
    #define I2C2
    extern ARM_DRIVER_I2C							Driver_I2C2;
    I2C_Driver_t I2C2_Driver = {"I2C3"};
    static void I2C3_callback(uint32_t event);
    I2C_DriverConfig_t I2C3_DriverConfig = {"I2C3", NULL, NULL, I2C3_callback};

#endif


//callback functions


int32_t I2C_Init_All (void) {
    int32_t status = ARM_DRIVER_OK;
		#ifdef USE_I2C1
			status = I2C_Init(I2C1_DriverConfig);
		#endif
		#ifdef USE_I2C2
				status = I2C_Init(I2C1_Driver);
		#endif
		#ifdef USE_I2C3
				status = I2C_Init(I2C1_Driver);
		#endif	
    return status;
}


int32_t I2C_Init(I2C_DriverConfig_t I2C_Driver) {
	//check de que no esta inicializado
		if (I2C_Driver.initialized) {
			return ARM_DRIVER_OK;
		}
		
		int32_t status = ARM_DRIVER_OK;
        
        // Inicializa el controlador I2C
        status |= I2C_Driver.driver->Initialize(I2C_Driver.callback_I2C);
        status |= I2C_Driver.driver->PowerControl(ARM_POWER_FULL);
        status |= I2C_Driver.driver->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);  // Usar velocidad rápida (400kHz) por defecto

//    // Verificar si el AS5600 responde a su dirección I2C
//    if (Driver_I2C1.MasterTransmit(AS5600_I2C_ADDRESS, NULL, 0, false) != ARM_DRIVER_OK) {
//        return ARM_DRIVER_ERROR;  // Error si no responde
//    }
		char name [20];
		sprintf(name, "%s%s", "mutex ", I2C_Driver.name);
		const osMutexAttr_t i2c_mutex_attr = {
			name
		};
		
		// Crear mutex para controlar el acceso al BUS I2C
        if ((I2C_Driver.mutex_I2C = osMutexNew(&i2c_mutex_attr))== NULL) {
			status |= ARM_DRIVER_ERROR;
		}			
    
    // Crear semáforo
        if ((I2C_Driver.transfer_I2C_semaphore = osSemaphoreNew(1, 0, NULL))==NULL) {  // Inicialmente bloqueado (valor 0)
            status |= ARM_DRIVER_ERROR;
		}
		
		if (status == ARM_DRIVER_OK) {
			I2C_Driver.initialized = true;
		}
		
    return status;  // El dispositivo está listo
}

int32_t I2C1_Configure(I2C_DriverConfig_t I2C_Driver, I2C_configuration_t configuration) {
    uint32_t status = ARM_DRIVER_OK;

    // Inicializa el controlador I2C
    status |= I2C_Driver.driver->Initialize(I2C_Driver.callback_I2C);
    status |= I2C_Driver.driver->PowerControl(ARM_POWER_FULL);
    status |= I2C_Driver.driver->Control(ARM_I2C_BUS_SPEED, configuration.speed);  // Usar velocidad rápida (400kHz) por defecto

//    // Verificar si el AS5600 responde a su dirección I2C
//    if (Driver_I2C1.MasterTransmit(AS5600_I2C_ADDRESS, NULL, 0, false) != ARM_DRIVER_OK) {
//        return ARM_DRIVER_ERROR;  // Error si no responde
//    }
    return status;  // El dispositivo está listo
}

#ifdef USE_I2C1
void I2C1_callback(uint32_t event) {
	if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
    osSemaphoreRelease(I2C1_DriverConfig.transfer_I2C_semaphore);
  } else {
		//errores
	}
}
#endif
#ifdef USE_I2C2
void I2C2_callback(uint32_t event) {
	if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
    osSemaphoreRelease(I2C2_Driver.transfer_I2C_semaphore);
  } else {
		//errores
	}
}
#endif
#ifdef USE_I2C3
void I2C3_callback(uint32_t event) {
	if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
    osSemaphoreRelease(I2C3_Driver.transfer_I2C_semaphore);
  } else {
		//errores
	}
}
#endif

int32_t I2C_ReadRegister (I2C_DriverConfig_t I2C_Driver, uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t* data) {
  // Adquirir el mutex antes de acceder al I2C
	osMutexAcquire(I2C_Driver.mutex_I2C, osWaitForever);
	//adquiere el semaforo de transferencias
	osSemaphoreAcquire(I2C_Driver.transfer_I2C_semaphore, osWaitForever);
	
  // Master command
  if (I2C_Driver.driver->MasterTransmit(SLAVE_ADDRESS, &reg, 1, false) != ARM_DRIVER_OK) {
     osMutexRelease(I2C_Driver.mutex_I2C);
     return ARM_DRIVER_ERROR;  // Error en la transmisión
  }
	/* Wait until transfer completed */
  osSemaphoreAcquire(I2C_Driver.transfer_I2C_semaphore, osWaitForever);
	
	//Slave response
	if (I2C_Driver.driver->MasterReceive(SLAVE_ADDRESS, data, 1, false) != ARM_DRIVER_OK) {
    osMutexRelease(I2C_Driver.mutex_I2C);
    return ARM_DRIVER_ERROR;  // Error en la recepción
  }
	
  osMutexRelease(I2C_Driver.mutex_I2C);  // Liberar el mutex después de usar el I2C

	return ARM_DRIVER_OK;
}


int32_t I2C_ReadRegisters (I2C_DriverConfig_t I2C_Driver, uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t* data, uint8_t size) {
  // Adquirir el mutex antes de acceder al I2C
	osMutexAcquire(I2C_Driver.mutex_I2C, osWaitForever);
	//adquiere el semaforo de transferencias
	osSemaphoreAcquire(I2C_Driver.transfer_I2C_semaphore, osWaitForever);
	
  // Master command
  if (I2C_Driver.driver->MasterTransmit(SLAVE_ADDRESS, &reg, 1, false) != ARM_DRIVER_OK) {
     osMutexRelease(I2C_Driver.mutex_I2C);
     return ARM_DRIVER_ERROR;  // Error en la transmisión
  }
	/* Wait until transfer completed */
  osSemaphoreAcquire(I2C_Driver.transfer_I2C_semaphore, osWaitForever);
	
	//Slave response
	if (I2C_Driver.driver->MasterReceive(SLAVE_ADDRESS, data, size, false) != ARM_DRIVER_OK) {
    osMutexRelease(I2C_Driver.mutex_I2C);
    return ARM_DRIVER_ERROR;  // Error en la recepción
  }
  osMutexRelease(I2C_Driver.mutex_I2C);  // Liberar el mutex después de usar el I2C
	
	return ARM_DRIVER_OK;
}
int32_t I2C_WriteRegister (I2C_DriverConfig_t I2C_Driver, uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t data) {
	uint8_t aux [2] = {reg, data};
	
	// Adquirir el mutex antes de acceder al I2C
	osMutexAcquire(I2C_Driver.mutex_I2C, osWaitForever);
	//adquiere el semaforo de transferencias
	osSemaphoreAcquire(I2C_Driver.transfer_I2C_semaphore, osWaitForever);
	
	//Master command
  if (I2C_Driver.driver->MasterTransmit(SLAVE_ADDRESS, aux, 2, false) != ARM_DRIVER_OK) {
    osMutexRelease(I2C_Driver.mutex_I2C);
    return ARM_DRIVER_ERROR;  // Error en la transmisión
  }
	
  osMutexRelease(I2C_Driver.mutex_I2C);  // Liberar el mutex después de usar el I2C
	
  return ARM_DRIVER_OK;
}
