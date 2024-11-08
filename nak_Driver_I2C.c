/**
 ******************************************************************************
 * @file           : nak_Driver_I2C.c
 * @author         : Natalia Ag�ero
 * @date           : 19/10/2024
 * @brief          : Driver I2C
 ******************************************************************************
 * @attention
 * 
 * Este archivo contiene la implementaci�n del driver I2C,
 * 
 ******************************************************************************
 */
 
#include "nak_Driver_I2C.h"
#include <stdio.h>
#include "RTE_Device.h"

//definition of I2C depending if its active or not

I2C_DriverConfig_t Drivers_I2C [I2C_LINE_MAX] = {
    [I2C_LINE_1] = {
        .name = "I2C1",
        .initialized = false
    },
    [I2C_LINE_2] = {
        .name = "I2C2",
        .initialized = false
    },
    [I2C_LINE_3] = {
        .name = "I2C3",
        .initialized = false
    }
};

#if defined (RTE_I2C1) && (RTE_I2C1 == 1)
    #define USE_I2C1
    extern ARM_DRIVER_I2C Driver_I2C1; 
    static void I2C1_callback(uint32_t event);
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
    extern ARM_DRIVER_I2C							Driver_I2C3;
    I2C_Driver_t I2C2_Driver = {"I2C3"};
    static void I2C3_callback(uint32_t event);
    I2C_DriverConfig_t I2C3_DriverConfig = {"I2C3", NULL, NULL, I2C3_callback};

#endif


int32_t I2C_Init_All (void) {
    int32_t status = ARM_DRIVER_OK;
		#ifdef USE_I2C1
            Drivers_I2C[I2C_LINE_1].driver = &Driver_I2C1;
            Drivers_I2C[I2C_LINE_1].callback_I2C = I2C1_callback;
            Drivers_I2C[I2C_LINE_1].initialized = false;
			status = I2C_Init(I2C_LINE_1);
		#endif
		#ifdef USE_I2C2
            Drivers_I2C[I2C_LINE_2].driver = &Driver_I2C2;
            Drivers_I2C[I2C_LINE_2].callback_I2C = I2C2_callback;
            Drivers_I2C[I2C_LINE_2].initialized = false;
			status = I2C_Init(I2C_LINE_2);
		#endif
		#ifdef USE_I2C3
            Drivers_I2C[I2C_LINE_3].driver = &Driver_I2C3;
            Drivers_I2C[I2C_LINE_3].callback_I2C = I2C3_callback;
            Drivers_I2C[I2C_LINE_3].initialized = false;
			status = I2C_Init(I2C_LINE_3);
		#endif	
    return status;
}


int32_t I2C_Init(I2C_LINE I2C_line) {
	//check de que no esta inicializado
		if (Drivers_I2C[I2C_line].initialized) {
			return ARM_DRIVER_OK;
		}
		
		int32_t status = ARM_DRIVER_OK;
        
        // Inicializa el controlador I2C
        Drivers_I2C[I2C_line].driver = &Driver_I2C1;
        status |= Drivers_I2C[I2C_line].driver->Initialize(Drivers_I2C[I2C_line].callback_I2C);
        status |= Drivers_I2C[I2C_line].driver->PowerControl(ARM_POWER_FULL);
        status |= Drivers_I2C[I2C_line].driver->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);  // Usar velocidad r�pida (400kHz) por defecto

//    // Verificar si el AS5600 responde a su direcci�n I2C
//    if (Driver_I2C1.MasterTransmit(AS5600_I2C_ADDRESS, NULL, 0, false) != ARM_DRIVER_OK) {
//        return ARM_DRIVER_ERROR;  // Error si no responde
//    }
		char name [20];
		sprintf(name, "%s%s", "mutex ", Drivers_I2C[I2C_line].name);
		const osMutexAttr_t i2c_mutex_attr = {
			name
		};
		
		// Crear mutex para controlar el acceso al BUS I2C
        if ((Drivers_I2C[I2C_line].mutex_I2C = osMutexNew(&i2c_mutex_attr))== NULL) {
			status |= ARM_DRIVER_ERROR;
		}			
    
    // Crear sem�foro
        if ((Drivers_I2C[I2C_line].transfer_I2C_semaphore = osSemaphoreNew(1, 1, NULL))==NULL) {  // Inicialmente bloqueado (valor 0)
            status |= ARM_DRIVER_ERROR;
		}
		
		if (status == ARM_DRIVER_OK) {
			Drivers_I2C[I2C_line].initialized = true;
		}
		
    return status;  // El dispositivo est� listo
}

int32_t I2C_Configure(I2C_LINE I2C_line, I2C_configuration_t configuration) {
    uint32_t status = ARM_DRIVER_OK;

    // Inicializa el controlador I2C
    status |= Drivers_I2C[I2C_line].driver->Initialize(I2C1_callback);
    status |= Drivers_I2C[I2C_line].driver->PowerControl(ARM_POWER_FULL);
    status |= Drivers_I2C[I2C_line].driver->Control(ARM_I2C_BUS_SPEED, configuration.speed);  // Usar velocidad r�pida (400kHz) por defecto

//    // Verificar si el AS5600 responde a su direcci�n I2C
//    if (Driver_I2C1.MasterTransmit(AS5600_I2C_ADDRESS, NULL, 0, false) != ARM_DRIVER_OK) {
//        return ARM_DRIVER_ERROR;  // Error si no responde
//    }
    return status;  // El dispositivo est� listo
}

#ifdef USE_I2C1
void I2C1_callback(uint32_t event) {
    uint8_t status = 0;
	if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
        status = 1;
    osSemaphoreRelease(Drivers_I2C[I2C_LINE_1].transfer_I2C_semaphore);
  } else {
      status = 2;
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

int32_t I2C_ReadRegister (I2C_LINE I2C_line, uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t* data) {
  // Adquirir el mutex antes de acceder al I2C
	osMutexAcquire(Drivers_I2C[I2C_line].mutex_I2C, osWaitForever);
	//adquiere el semaforo de transferencias
	osSemaphoreAcquire(Drivers_I2C[I2C_line].transfer_I2C_semaphore, osWaitForever);
	
  // Master command
  if (Drivers_I2C[I2C_line].driver->MasterTransmit(SLAVE_ADDRESS, &reg, 1, false) != ARM_DRIVER_OK) {
     osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
     return ARM_DRIVER_ERROR;  // Error en la transmisi�n
  }
	/* Wait until transfer completed */
  osSemaphoreAcquire(Drivers_I2C[I2C_line].transfer_I2C_semaphore, osWaitForever);
	
	//Slave response
	if (Drivers_I2C[I2C_line].driver->MasterReceive(SLAVE_ADDRESS, data, 1, false) != ARM_DRIVER_OK) {
    osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
    return ARM_DRIVER_ERROR;  // Error en la recepci�n
  }
	
  osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);  // Liberar el mutex despu�s de usar el I2C

	return ARM_DRIVER_OK;
}


int32_t I2C_ReadRegisters (I2C_LINE I2C_line, uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t* data, uint8_t size) {
  uint32_t status;
    
    // Adquirir el mutex antes de acceder al I2C
	osMutexAcquire(Drivers_I2C[I2C_line].mutex_I2C, osWaitForever);
	//adquiere el semaforo de transferencias
	osSemaphoreAcquire(Drivers_I2C[I2C_line].transfer_I2C_semaphore, osWaitForever);
	
  // Master command
    status = Drivers_I2C[I2C_line].driver->MasterTransmit(SLAVE_ADDRESS, &reg, 1, false);
  if (status != ARM_DRIVER_OK) {
     osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
     return ARM_DRIVER_ERROR;  // Error en la transmisi�n
  }
	/* Wait until transfer completed */
  osSemaphoreAcquire(Drivers_I2C[I2C_line].transfer_I2C_semaphore, osWaitForever);
	
	//Slave response
	if (Drivers_I2C[I2C_line].driver->MasterReceive(SLAVE_ADDRESS, data, size, false) != ARM_DRIVER_OK) {
    osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
    return ARM_DRIVER_ERROR;  // Error en la recepci�n
  }
  osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);  // Liberar el mutex despu�s de usar el I2C
	
	return ARM_DRIVER_OK;
}
int32_t I2C_WriteRegister (I2C_LINE I2C_line, uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t data) {
	uint8_t aux [2] = {reg, data};
	
	// Adquirir el mutex antes de acceder al I2C
	osMutexAcquire(Drivers_I2C[I2C_line].mutex_I2C, osWaitForever);
	//adquiere el semaforo de transferencias
	osSemaphoreAcquire(Drivers_I2C[I2C_line].transfer_I2C_semaphore, osWaitForever);
	
	//Master command
  if (Drivers_I2C[I2C_line].driver->MasterTransmit(SLAVE_ADDRESS, aux, 2, false) != ARM_DRIVER_OK) {
    osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
    return ARM_DRIVER_ERROR;  // Error en la transmisi�n
  }
	
  osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);  // Liberar el mutex despu�s de usar el I2C
	
  return ARM_DRIVER_OK;
}
