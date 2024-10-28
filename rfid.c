#include "rfid.h"
#include "string.h"

//flags and hilos
#define TRANSFER_COMPLETE 					0x01              //flag que se utiliza en SPI para saber que una transferencia se ha completado
#define QUEUE_MAX										10                //maximo numero de elementos en la cola

//SPI init
extern ARM_DRIVER_SPI Driver_SPI2;
static ARM_DRIVER_SPI* SPIdrv = &Driver_SPI2;

//thread
static osThreadId_t TID_RC522;
extern osThreadId_t tid_Thread_principal;
static osMessageQueueId_t mid_MsgQueueMFRC522;
static void Th_rfid (void *argument);

//timer
static osTimerId_t id_timer_medidas;
static void timer_medidas_callback (void *arg);
static void init_timer_medidas(void);

//local functions
static void SPI_callback(uint32_t event);
static void TM_MFRC522_InitSPI(void);
static void TM_MFRC522_Init (void);
static TM_MFRC522_Status_t TM_MFRC522_Check(uint8_t* id);
static void TM_MFRC522_WriteRegister(uint8_t addr, uint8_t val);
static uint8_t TM_MFRC522_ReadRegister(uint8_t addr);
static void TM_MFRC522_SetBitMask(uint8_t reg, uint8_t mask);
static void TM_MFRC522_ClearBitMask(uint8_t reg, uint8_t mask);
static void TM_MFRC522_AntennaOn(void);
static void TM_MFRC522_AntennaOff(void);
static void TM_MFRC522_Reset(void);
static TM_MFRC522_Status_t TM_MFRC522_Request(uint8_t reqMode, uint8_t* TagType);
static TM_MFRC522_Status_t TM_MFRC522_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen);
static TM_MFRC522_Status_t TM_MFRC522_Anticoll(uint8_t* serNum);
static void TM_MFRC522_CalculateCRC(uint8_t*  pIndata, uint8_t len, uint8_t* pOutData);
static uint8_t TM_MFRC522_SelectTag(uint8_t* serNum);
static TM_MFRC522_Status_t TM_MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t* Sectorkey, uint8_t* serNum);
static TM_MFRC522_Status_t TM_MFRC522_Read(uint8_t blockAddr, uint8_t* recvData);
static TM_MFRC522_Status_t TM_MFRC522_Write(uint8_t blockAddr, uint8_t* writeData);
static void TM_MFRC522_Halt(void);

//FUNCION QUE INICIALIZA RFID
void Init_ThMFRC522 (void) {
	mid_MsgQueueMFRC522 = osMessageQueueNew(QUEUE_MAX, sizeof(MSGQUEUE_RC522_t), NULL);
	TID_RC522 = osThreadNew(Th_rfid, NULL, NULL); 
}

static void Th_rfid (void *argument) {
	static uint32_t flags = 0;
	static MSGQUEUE_RC522_t msg;
	static uint8_t CardID[5];
  static TM_MFRC522_Status_t status;
//inicializacion
  TM_MFRC522_Init();
  init_timer_medidas();
  det_rfid_on();
  while (1) {
		flags = osThreadFlagsWait(RFID_READID, osFlagsWaitAny, osWaitForever);
			if (flags == RFID_READID) {
				status = TM_MFRC522_Check(CardID);
				if (status == MI_OK) {
					memcpy(msg.id, CardID, 5);
					if (osMessageQueuePut(mid_MsgQueueMFRC522 , &msg, 0U, 0U)) {
						osThreadFlagsSet(tid_Thread_principal, RFID_READID);
					}
        } else if (status == MI_ERR) {
          //ERROR GESTIONAR
        }
    }
  }
}

osMessageQueueId_t getRFIDQueue (void) {
	return mid_MsgQueueMFRC522;
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
  
//  /*Reset*/   //SPI_MISO -- SPI_B_MISO  PB4
//  __HAL_RCC_GPIOB_CLK_ENABLE();
//  GPIO_InitStruct_RFID.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct_RFID.Pull = GPIO_PULLUP;
//  GPIO_InitStruct_RFID.Speed = GPIO_SPEED_FREQ_HIGH;
//  GPIO_InitStruct_RFID.Pin = GPIO_PIN_4;
//  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_RFID);
//  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
  
  /*SPI*/   
  SPIdrv->Initialize(SPI_callback);
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
        osThreadFlagsSet(TID_RC522, TRANSFER_COMPLETE);
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

static void TM_MFRC522_Init (void) {
	TM_MFRC522_InitSPI();
	TM_MFRC522_Reset();
	osDelay(50);
	TM_MFRC522_WriteRegister(MFRC522_REG_T_MODE, 0x8D);
	TM_MFRC522_WriteRegister(MFRC522_REG_T_PRESCALER, 0x3E);
	TM_MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_L, 30);           
	TM_MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_H, 0);

	/* 48dB gain */
	TM_MFRC522_WriteRegister(MFRC522_REG_RF_CFG, 0x70);
	
	TM_MFRC522_WriteRegister(MFRC522_REG_TX_AUTO, 0x40);
	TM_MFRC522_WriteRegister(MFRC522_REG_MODE, 0x3D);

	TM_MFRC522_AntennaOn();		//Open the antenna
	
}

/*
Funcion que se utiliza para verificar la existencia de una tarjeta RFID en el campo de lectura
del lector MFRRC522 y guarda su id en el puntero especificado
id - puntero a un area de memoria de 5 bytes donde se almacenará el ID de la tarjeta detectada
*/
static TM_MFRC522_Status_t TM_MFRC522_Check(uint8_t* id) {
	TM_MFRC522_Status_t status;
	//Find cards, return card type
	status = TM_MFRC522_Request(PICC_REQIDL, id);		//envia solicitud para detectar tarjetas RFID y obtener su tipo
	if (status == MI_OK) {	//si se detecto correctamente una tarjeta
		//Card detected
		//Anti-collision, return card serial number 4 bytes
		status = TM_MFRC522_Anticoll(id);	//por si acaso hay colision
	}
	TM_MFRC522_Halt();			//Command card into hibernation 
	// asegura que la tarjeta ete lista para futuras operciones

	return status;
}

//funcion innecesaria que podemos hacer fuera
bool TM_MFRC522_Compare(uint8_t* CardID, uint8_t* CompareID) {
	uint8_t i;
	for (i = 0; i < 5; i++) {
		if (CardID[i] != CompareID[i]) {
			return false;
		}
	}
	return true;
}

static void TM_MFRC522_WriteRegister(uint8_t addr, uint8_t val) {
	static uint8_t tx_data[2];
	//CS low
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	//send address and data
	tx_data[0] = (addr << 1) & 0x7E; 	//asegura que se reserva espacio para el bit de W/R
	tx_data[1] = val;
	SPIdrv->Send(tx_data, 2);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
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
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	SPIdrv->Receive(&rx_data, 1);
	osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	
	//CS high
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	return rx_data;	
}

static void TM_MFRC522_SetBitMask(uint8_t reg, uint8_t mask) {
	TM_MFRC522_WriteRegister(reg, TM_MFRC522_ReadRegister(reg) | mask);
}

static void TM_MFRC522_ClearBitMask(uint8_t reg, uint8_t mask){
	TM_MFRC522_WriteRegister(reg, TM_MFRC522_ReadRegister(reg) & (~mask));
} 

static void TM_MFRC522_AntennaOn(void) {
	uint8_t temp;

	temp = TM_MFRC522_ReadRegister(MFRC522_REG_TX_CONTROL);
	if (!(temp & 0x03)) {
		TM_MFRC522_SetBitMask(MFRC522_REG_TX_CONTROL, 0x03);
	}
}

static void TM_MFRC522_AntennaOff(void) {
	TM_MFRC522_ClearBitMask(MFRC522_REG_TX_CONTROL, 0x03);
}

static void TM_MFRC522_Reset(void) {
	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_RESETPHASE);
}

/*
Envia un comando de solicitud al chip MFRC522 para buscar tarjetas RFID en el area de alcance
y obtiene el tipo de tarjeta
reqMode - comando (PICC_REQIDL p.ej busca una tarjeta)
TagType - id de la tarjeta detectada
*/
static TM_MFRC522_Status_t TM_MFRC522_Request(uint8_t reqMode, uint8_t* TagType) {
	TM_MFRC522_Status_t status;  
	uint16_t backBits;			//The received data bits

	TM_MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x07);		//TxLastBists = BitFramingReg[2..0]	???
	//me ralla no se si tendria que ser un 0x00 porque con un 7 indicas que se tienen que mandar 7 bits!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	TagType[0] = reqMode;
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

	if ((status != MI_OK) || (backBits != 0x10)) {    
		status = MI_ERR;
	}

	return status;
}

/*
Funcion que se utiliza para enviar comandos al lector RFID y recibir datos de la tarjeta
PCD_AUTHENT: autenticar la tarjeta usando la clave A o B (mirar funcion auth)
PCD_TRANSCEIVE: operacion de transmision y recepcion simultanea
*/
static TM_MFRC522_Status_t TM_MFRC522_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen) {
	TM_MFRC522_Status_t status = MI_ERR;
	uint8_t irqEn = 0x00;			//para MFRC522_REG_COMM_IE_N
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint16_t i;

	switch (command) {
		case PCD_AUTHENT: {
			irqEn = 0x12;					//IdleIRQ enable (allows idle interrupt request (indicated by bit IdleIRQ) to be propagated to pin IRQ
														//ErrIEn enable (allows the error interrrupt request (indicated by bit ErrIRq) to be propagated to pin IRQ
			//no transmitter interrup request, no receiver interrupt request, no high or low alert interrupt request, no timer interrupt request
			waitIRq = 0x10;			//espera interrupciones específicas	
			break;
		}
		case PCD_TRANSCEIVE: {
			irqEn = 0x77;					//todos los anteriores, menos el high alert interrupt
			waitIRq = 0x30;
			break;
		}
		default:
			break;
	}

	//IRQ pin funciona en open drain (DivlEnReg set to 00)
	TM_MFRC522_WriteRegister(MFRC522_REG_COMM_IE_N, irqEn | 0x80);	//signal pin IRQ inverted with respecto to bit IRq in the registrer Status1Reg
	TM_MFRC522_ClearBitMask(MFRC522_REG_COMM_IRQ, 0x80);	//Set1 - '0' marked bits in register CommIRqReg are clear
	TM_MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);	//FluschBuffer - '1' clears internal FIFO-buffer's read and write-pointer and the bit BufferOvfl in reg ErrReg inmediately (indica desbordamiento en FIFO)

	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_IDLE);		//no action to clear BitFramingReg

	//Writing data to the FIFO
	for (i = 0; i < sendLen; i++) {   
		TM_MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, sendData[i]);    //se meten los datos en la fifo
	}

	//Execute the command
	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, command);
	if (command == PCD_TRANSCEIVE) {    
		TM_MFRC522_SetBitMask(MFRC522_REG_BIT_FRAMING, 0x80);		//StartSend=1,transmission of data starts  
	}   

	//Waiting to receive data to complete
	i = 2000;	//i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms???
	do {
		//CommIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		n = TM_MFRC522_ReadRegister(MFRC522_REG_COMM_IRQ);
		i--;
	} while ((i!=0) && !(n&0x01) && !(n&waitIRq));

	TM_MFRC522_ClearBitMask(MFRC522_REG_BIT_FRAMING, 0x80);			//StartSend=0

	if (i != 0)  {
		if (!(TM_MFRC522_ReadRegister(MFRC522_REG_ERROR) & 0x1B)) {		//no ERROR - buffer lleno, colision, paridad, protocolos (mirar) NO MIRA CRC
			status = MI_OK;
			if (n & irqEn & 0x01) {   //timer irq? - '1' when timer decrements the TimerValue Register to zero
				status = MI_NOTAGERR;			
			}

			if (command == PCD_TRANSCEIVE) {
				n = TM_MFRC522_ReadRegister(MFRC522_REG_FIFO_LEVEL);	//mira el numero de bits en la fifo
				lastBits = TM_MFRC522_ReadRegister(MFRC522_REG_CONTROL) & 0x07;	//shows the number of valid bits in the last received byte. If 0, whole byte is valid
				printf("n = %d, lastBits = %d\n", n, lastBits);
        if (lastBits) {   
					*backLen = (n - 1) * 8 + lastBits;   //calcula la longitud total de los datos recibidos
				} else {   														//(n - 1) resta 1 al numero total de bytes recbidos y lo *8 para obtener el numero de bits total
					*backLen = n * 8;   									//luego agrega los bits validos (+lastBits)
				}																			//si es 0, todo el byte es valido, por lo que se simplifica, n*8

				if (n == 0) {							//si se reciben 0 bytes, se pone en 1	para evitar errores posteriores (divisiones por 0 y eso)					   
					n = 1;    
				}
				if (n > MFRC522_MAX_LEN) {   
					n = MFRC522_MAX_LEN;   
				}

				//Reading the received data in FIFO
				for (i = 0; i < n; i++) {   
					backData[i] = TM_MFRC522_ReadRegister(MFRC522_REG_FIFO_DATA);    
				}
         printf("backLen = %d\n", *backLen);
			}
		} else {   
			status = MI_ERR;  
		}
	}
printf("status = %d\n", status);
	return status;
}

/*
Funcion que permite realizar anticolision de tarjetas
devuelve MI_OK si se recibió bien el ID
*/
static TM_MFRC522_Status_t TM_MFRC522_Anticoll(uint8_t* serNum) {
	TM_MFRC522_Status_t status;
	uint8_t i;
	uint8_t serNumCheck = 0;
	uint16_t unLen;

	//mandamos 8 bits - BitFramingReg[2..0] = TxLastBits, con 0 indica que se mandan 8
	TM_MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x00);		//TxLastBists = BitFramingReg[2..0]
	
	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);
	//mandamos un 0x93 y un 0x20

	if (status == MI_OK) {
		//Check card serial number
		for (i = 0; i < 4; i++) {   
			serNumCheck ^= serNum[i];
		}
		if (serNumCheck != serNum[i]) {   
			status = MI_ERR;    
		}
	}
	return status;
} 

static void TM_MFRC522_CalculateCRC(uint8_t*  pIndata, uint8_t len, uint8_t* pOutData) {
	uint8_t i, n;

	TM_MFRC522_ClearBitMask(MFRC522_REG_DIV_IRQ, 0x04);			//CRCIrq = 0
	TM_MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);			//Clear the FIFO pointer
	//Write_MFRC522(CommandReg, PCD_IDLE);

	//Writing data to the FIFO	
	for (i = 0; i < len; i++) {   
		TM_MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, *(pIndata+i));   
	}
	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_CALCCRC);

	//Wait CRC calculation is complete
	i = 0xFF;
	do {
		n = TM_MFRC522_ReadRegister(MFRC522_REG_DIV_IRQ);
		i--;
	} while ((i!=0) && !(n&0x04));			//CRCIrq = 1

	//Read CRC calculation result
	pOutData[0] = TM_MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_L);
	pOutData[1] = TM_MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_M);
}

/*
Selecciona una tarjeta para comenzar una transmision de datos
(por ejemplo escribir dentro de un bloque)
serNum - id de la tarjeta
*/
static uint8_t TM_MFRC522_SelectTag(uint8_t* serNum) {
	uint8_t i;
	TM_MFRC522_Status_t status;
	uint8_t size;
	uint16_t recvBits;
	uint8_t buffer[9]; 

	buffer[0] = PICC_SElECTTAG;
	buffer[1] = 0x70;
	for (i = 0; i < 5; i++) {
		buffer[i+2] = *(serNum+i);
	}
	TM_MFRC522_CalculateCRC(buffer, 7, &buffer[7]);		//??
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

	if ((status == MI_OK) && (recvBits == 0x18)) {   
		size = buffer[0]; 
	} else {   
		size = 0;    
	}

	return size;
}

/*
Permite autenticarse para que te permita escribir en los bloques de la tarjeta
MFRC522_Auth(PICC_AUTHENT1A,24,KEY,serNum); 
24 - bloque donde escribes
Key - codigo necesario para que te permita escribir
serNum - datos que escribes
*/
static TM_MFRC522_Status_t TM_MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t* Sectorkey, uint8_t* serNum) {
	TM_MFRC522_Status_t status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[12]; 

	//Verify the command block address + sector + password + card serial number
	buff[0] = authMode;
	buff[1] = BlockAddr;
	for (i = 0; i < 6; i++) {    
		buff[i+2] = *(Sectorkey+i);   
	}
	for (i=0; i<4; i++) {    
		buff[i+8] = *(serNum+i);   
	}
	status = TM_MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);

	if ((status != MI_OK) || (!(TM_MFRC522_ReadRegister(MFRC522_REG_STATUS2) & 0x08))) {   
		status = MI_ERR;   
	}

	return status;
}

/*
Te permite leer dentro de un bloque de datos
status = MFRC522_Read( 24, R);
24 - bloque de donde se lee
R - buffer que guarda los datos
*/
static TM_MFRC522_Status_t TM_MFRC522_Read(uint8_t blockAddr, uint8_t* recvData) {
	TM_MFRC522_Status_t status;
	uint16_t unLen;

	recvData[0] = PICC_READ;
	recvData[1] = blockAddr;
	TM_MFRC522_CalculateCRC(recvData,2, &recvData[2]);
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

	if ((status != MI_OK) || (unLen != 0x90)) {
		status = MI_ERR;
	}

	return status;
}

/*
Te permite escribir dentro de un bloque de datos
status = MFRC522_Write((uint8_t)24 , W);
24 - bloque donde escribes
w - buffer de los datos que escribes
*/
static TM_MFRC522_Status_t TM_MFRC522_Write(uint8_t blockAddr, uint8_t* writeData) {
	TM_MFRC522_Status_t status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[18]; 

	buff[0] = PICC_WRITE;
	buff[1] = blockAddr;
	TM_MFRC522_CalculateCRC(buff, 2, &buff[2]);
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

	if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) {   
		status = MI_ERR;   
	}

	if (status == MI_OK) {
		//Data to the FIFO write 16Byte
		for (i = 0; i < 16; i++) {    
			buff[i] = *(writeData+i);   
		}
		TM_MFRC522_CalculateCRC(buff, 16, &buff[16]);
		status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) {   
			status = MI_ERR;   
		}
	}

	return status;
}

/*
Pone una tarjeta en hibernacion para parar la transferencia de datos de la misma
(por ejemplo tras escribir o leer de un bloque)
*/
static void TM_MFRC522_Halt(void) {
	uint16_t unLen;
	uint8_t buff[4]; 

	buff[0] = PICC_HALT;
	buff[1] = 0;
	TM_MFRC522_CalculateCRC(buff, 2, &buff[2]);

	TM_MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}

//permite detectar periodicamente tarjetas
void det_rfid_on(void) {
  osTimerStart(id_timer_medidas, 300);
}

static void init_timer_medidas(void) {
  id_timer_medidas = osTimerNew(timer_medidas_callback, osTimerPeriodic, NULL, NULL);
}

static void timer_medidas_callback (void *arg) {
	osThreadFlagsSet(TID_RC522, RFID_READID);
}

void det_rfid_off(void) {
  osTimerStop(id_timer_medidas);
}
