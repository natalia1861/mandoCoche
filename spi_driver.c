#include "spi_driver.h"

//SPI init
extern ARM_DRIVER_SPI Driver_SPI2;
static ARM_DRIVER_SPI* SPIdrv = &Driver_SPI2;

void Configure_SPI (GPIO_PIN_PORT_t cs) {
	
	
}

static void TM_MFRC522_InitSPI(void){
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  static GPIO_InitTypeDef GPIO_InitStruct_RFID;
	
  /*CS*/    //SPI_CS -- SPI_B_NSS       PB12
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct_RFID.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_RFID.Pull = GPIO_PULLUP;
  GPIO_InitStruct_RFID.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_RFID.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_RFID);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
  
  /*SPI*/   
//  SPIdrv->Initialize(SPI_callback);
  SPIdrv-> PowerControl(ARM_POWER_FULL);
  SPIdrv-> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS (8), 20000000);
//  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
//  osDelay(1);
//  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
//  osDelay(1000);
}

static void SPI_callback(uint32_t event){
    switch (event) {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
        //osThreadFlagsSet(TID_RC522, TRANSFER_COMPLETE);
        break;
    case ARM_SPI_EVENT_DATA_LOST:
        /*  Occurs in slave mode when data is requested/sent by master
            but send/receive/transfer operation has not been started
            and indicates that data is lost. Occurs also in master mode
            when driver cannot transfer data fast enough. */
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    case ARM_SPI_EVENT_MODE_FAULT:
        /*  Occurs in master mode when Slave Select is deactivated and
            indicates Master Mode Fault. */
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    }
}

static void TM_MFRC522_WriteRegister(uint8_t addr, uint8_t val) {
	static uint8_t tx_data[2];
	//CS low
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	//send address and data
	tx_data[0] = (addr << 1) & 0x7E; 	//asegura que se reserva espacio para el bit de W/R
	tx_data[1] = val;
	SPIdrv->Send(tx_data, 2);
	//osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	//CS high
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

static uint8_t TM_MFRC522_ReadRegister(uint8_t addr) {
	uint8_t rx_data;
	//CS low
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	//send address and data
	uint8_t tx_data = ((addr << 1) & 0x7E) | 0x80; 	//asegura que se reserva espacio para el bit de W/R
	//SPIdrv->Transfer(&tx_data, &rx_data, 1);
	//osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	
	SPIdrv->Send(&tx_data, 1);
	//osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	SPIdrv->Receive(&rx_data, 1);
	//osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	
	//CS high
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	return rx_data;	
}