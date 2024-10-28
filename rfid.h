#ifndef __THRFID_H
#define __THRFID_H

#include "Driver_SPI.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file

typedef enum {
	MI_OK = 0,
	MI_NOTAGERR,
	MI_ERR
} TM_MFRC522_Status_t;

typedef struct{
  uint8_t id[5];
} MSGQUEUE_RC522_t;

//flags e hilos 
#define RFID_READID 								0x20
#define RFID_STOP                   0x04
#define RFID_START                  0x08

/* MFRC522 Commands for CommandReg*/
#define PCD_IDLE										0x00   //NO action; Cancel the current command
#define PCD_AUTHENT									0x0E   //Authentication Key
#define PCD_RECEIVE									0x08   //Receive Data
#define PCD_TRANSMIT								0x04   //Transmit data
#define PCD_TRANSCEIVE							0x0C   //Transmit and receive data,
#define PCD_RESETPHASE							0x0F   //Reset
#define PCD_CALCCRC									0x03   //CRC Calculate

/* Mifare_One card command word */
#define PICC_REQIDL									0x26   // find the antenna area does not enter hibernation
#define PICC_ANTICOLL								0x93   // anti-collision
#define PICC_SElECTTAG							0x93   // election card
#define PICC_READ										0x30   // Read Block
#define PICC_WRITE									0xA0   // write block
#define PICC_HALT										0x50   // Sleep

/* MFRC522 Registers */
//Page 0: Command and Status
#define MFRC522_REG_COMMAND					0x01 
#define MFRC522_REG_COMM_IE_N				0x02  
#define MFRC522_REG_COMM_IRQ				0x04 
#define MFRC522_REG_DIV_IRQ					0x05
#define MFRC522_REG_ERROR						0x06 
#define MFRC522_REG_STATUS2					0x08 
#define MFRC522_REG_FIFO_DATA				0x09
#define MFRC522_REG_FIFO_LEVEL			0x0A
#define MFRC522_REG_CONTROL					0x0C
#define MFRC522_REG_BIT_FRAMING			0x0D

//Page 1: Command 
#define MFRC522_REG_MODE						0x11
#define MFRC522_REG_TX_CONTROL			0x14
#define MFRC522_REG_TX_AUTO					0x15

//Page 2: CFG   
#define MFRC522_REG_CRC_RESULT_M		0x21
#define MFRC522_REG_CRC_RESULT_L		0x22
#define MFRC522_REG_RF_CFG					0x26
#define MFRC522_REG_T_MODE					0x2A
#define MFRC522_REG_T_PRESCALER			0x2B
#define MFRC522_REG_T_RELOAD_H			0x2C
#define MFRC522_REG_T_RELOAD_L			0x2D


#define MFRC522_MAX_LEN							16

/*
Funcion que inicializa RFID, devuelve el id de la cola donde metera las id.
En cuanto se detecte una tarjeta, avisará al hilo principal con una flag, RFID_READID
*/
void Init_ThMFRC522 (void);
void det_rfid_on(void);
void det_rfid_off(void);
bool TM_MFRC522_Compare(uint8_t* CardID, uint8_t* CompareID);
osMessageQueueId_t getRFIDQueue(void);

#endif
