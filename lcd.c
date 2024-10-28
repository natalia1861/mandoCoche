#include "lcd.h"
#include "Arial12x12.h"
/*--------------------------VARIABLES----------------------------*/
#define TRANSFER_COMPLETE 0x02
extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;
uint8_t positionL1=0;
uint8_t positionL2=0;

unsigned char buffer[512];

osThreadId_t tid_Th_LCD;
osMessageQueueId_t mid_MsgQueueLCD;     
osThreadId_t tid_Th_LCD_Test;

/*-------------------PROTOTIPOS DE FUNCIONES---------------------*/
static void delay(uint32_t n_microsegundos);
static void mySPI_callback(uint32_t event);
static void LCD_wr_cmd(unsigned char cmd);
static void LCD_wr_data(unsigned char data);
static void clearBuffer(void);
static void LCD_update(void);
static void LCD_update_L1(void);
static void LCD_update_L2(void);
static void LCD_symbolToLocalBuffer_L1(uint8_t symbol);
static void LCD_symbolToLocalBuffer_L2(uint8_t symbol);
static void LCD_symbolToLocalBuffer(uint8_t line, uint8_t symbol);
static void Init_MsgQueue_LCD (void);
static void Init_Thread_LCD (void) ;
static void Thread_LCD (void *argument);

/*---------------------------FUNCIONES---------------------------*/
static void delay(uint32_t n_microsegundos){
	// Configurar y arrancar el timer para generar un evento
	// pasados n_microsegundos
	TIM_HandleTypeDef htim7; //tim7_clk=APB1_timer_clock=20MHz
	
	__HAL_RCC_TIM7_CLK_ENABLE(); 
	
	htim7.Instance= TIM7;        
	htim7.Init.Prescaler=84-1; // 84MHz /84 = 1MHz --> 1us
	htim7.Init.Period=n_microsegundos-1; // 1MHz/n_usegundos=1us*n_usegundos

	HAL_TIM_Base_Init(&htim7); 
	__HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
	HAL_TIM_Base_Start(&htim7); 
	// Esperar a que se active el flag del registro de Match
	// correspondiente
	while(!__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE));
	//Borrar el flag
	__HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
	// Parar el Timer y ponerlo a 0 para la siguiente llamada a
	// la función
	HAL_TIM_Base_Stop(&htim7); 
	HAL_TIM_Base_DeInit(&htim7); 
}

static void mySPI_callback(uint32_t event){
    switch (event)
    {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
        osThreadFlagsSet(tid_Th_LCD, TRANSFER_COMPLETE);
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
static void LCD_reset(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	//INICIALIZACIÓN Y CONFIGURACIÓN DEL SPI
  Driver_SPI1.Initialize(mySPI_callback);
	Driver_SPI1.PowerControl(ARM_POWER_FULL);
	Driver_SPI1.Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), 2000000);
  //CONFIGURACIÓN DE PINES DE SALIDA PARA LCD
  /*Enable clock to GPIO*/
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
	/*Config pins*/
  GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP; 
	GPIO_InitStruct.Pull=GPIO_PULLUP; 
	GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_VERY_HIGH; 
   
  /*Init Pins */
	//LCD_RESET --> PA6
	GPIO_InitStruct.Pin=GPIO_PIN_6;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6,GPIO_PIN_SET);
	//LCD_A0 --> PF13
	GPIO_InitStruct.Pin=GPIO_PIN_13;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13,GPIO_PIN_SET);
	//LCD_CS_N --> PD14		
	GPIO_InitStruct.Pin=GPIO_PIN_14;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);		
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14,GPIO_PIN_SET);
	
	/*Señal de reset*/
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6,GPIO_PIN_RESET);
	delay(2); //mínimo 1
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6,GPIO_PIN_SET);
	
	/*Espera a fin del proceso*/
	delay(1000);
}

static void LCD_wr_data(unsigned char data){
		// Seleccionar CS = 0;
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET);
		// Seleccionar A0 = 1; => a 1 desde inicio
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13,GPIO_PIN_SET);
		// Escribir un dato (data) usando la función SPIDrv->Send(…);
		SPIdrv->Send(&data,sizeof(data));
		// Esperar a que se libere el bus SPI;
		osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
		// Seleccionar CS = 1;
	  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET);
}

static void LCD_wr_cmd(unsigned char cmd){
	// Seleccionar CS = 0;
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET);
	// Seleccionar A0 = 0;
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13,GPIO_PIN_RESET);
	// Escribir un comando (cmd) usando la función SPIDrv->Send(…);
		SPIdrv->Send(&cmd,sizeof(cmd));
	// Esperar a que se libere el bus SPI;
		osThreadFlagsWait(TRANSFER_COMPLETE, osFlagsWaitAny, osWaitForever);
	// Seleccionar CS = 1;
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET);
}

static void LCD_init(void){
	LCD_wr_cmd(0xAE); //Display off
	LCD_wr_cmd(0xA2); //Fija el valor de la relación de la tensión de polarización del LCD a 1/9
	LCD_wr_cmd(0xA0); //El direccionamiento de la RAM de datos del display es la normal
	LCD_wr_cmd(0xC8); //El scan en las salidas COM es el normal
	LCD_wr_cmd(0x22); //Fija la relación de resistencias interna a 2
	LCD_wr_cmd(0x2F); //Power on
	LCD_wr_cmd(0x40); //Display empieza en la línea 0
	LCD_wr_cmd(0xAF); //Display ON
	LCD_wr_cmd(0x81); //Contraste
	LCD_wr_cmd(0x0F); //Valor Contraste
	LCD_wr_cmd(0xA4); //Display all points normal
	LCD_wr_cmd(0xA6); //LCD Display normal
	
	clearBuffer();
	LCD_update();
}

static void clearBuffer(void){
	int i=0;
	for(i=0;i<512;i++){
		buffer[i]=0x00;
	}
}


static void LCD_update(void){
	int i;
	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB0); // Página 0
	for(i=0;i<128;i++){
		LCD_wr_data(buffer[i]);
	}
	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB1); // Página 1
	for(i=128;i<256;i++){
		LCD_wr_data(buffer[i]);
	}
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB2); //Página 2
	for(i=256;i<384;i++){
		LCD_wr_data(buffer[i]);
	}
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB3); // Pagina 3
	for(i=384;i<512;i++){
		LCD_wr_data(buffer[i]);
	}
	
	clearBuffer();
	positionL1=0;
	positionL2=0;
}
static void LCD_update_L1(void){
	int i;
	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB0); // Página 0
	for(i=0;i<128;i++){
		LCD_wr_data(buffer[i]);
	}
	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB1); // Página 1
	for(i=128;i<256;i++){
		LCD_wr_data(buffer[i]);
	}
	
	clearBuffer();
	positionL1=0;
}
static void LCD_update_L2(void){
	int i;
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB2); //Página 2
	for(i=256;i<384;i++){
		LCD_wr_data(buffer[i]);
	}
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB3); // Pagina 3
	for(i=384;i<512;i++){
		LCD_wr_data(buffer[i]);
	}
	
	clearBuffer();
	positionL2=0;
}

static void LCD_symbolToLocalBuffer_L1(uint8_t symbol){
		uint8_t i, valuel, value2;
		uint16_t offset=0;

		offset=25* (symbol - ' ');
		for (i=0; i<12; i++){
			valuel=Arial12x12[offset+i*2+1];
			value2=Arial12x12[offset+i*2+2];

			buffer[i+positionL1]=valuel;
			buffer[i+128+positionL1]=value2;
		}
		positionL1=positionL1+Arial12x12 [offset];
		
		clearBuffer();
	}
	
static void LCD_symbolToLocalBuffer_L2(uint8_t symbol){
		uint8_t i, valuel, value2;
		uint16_t offset=0;

		offset=25* (symbol - ' ');
		for (i=0; i<12; i++){
			valuel=Arial12x12[offset+i*2+1];
			value2=Arial12x12[offset+i*2+2];

			buffer[i+positionL2+256]=valuel;
			buffer[i+128+positionL2+256]=value2;
		}
		positionL2=positionL2+Arial12x12 [offset];
	}
static void LCD_symbolToLocalBuffer(uint8_t line, uint8_t symbol) {
		uint8_t i, valuel, value2;
		uint16_t offset=0;

		offset=25* (symbol - ' ');

		if(line==1){
			for (i=0; i<12; i++){
				valuel=Arial12x12[offset+i*2+1];
				value2=Arial12x12[offset+i*2+2];

				buffer[i+positionL1]=valuel;
				buffer[i+128+positionL1]=value2;
			}
			positionL1=positionL1+Arial12x12 [offset];
		}else{
			for (i=0; i<12; i++){
				valuel=Arial12x12[offset+i*2+1];
				value2=Arial12x12[offset+i*2+2];

				buffer[i+positionL2+256]=valuel;
				buffer[i+128+positionL2+256]=value2;
			}
			positionL2=positionL2+Arial12x12 [offset];
		}

	}
	
static void LCD_LineaToLocalBuffer(char linea[30], uint8_t nLin){
		int i;
		i=0;
		while(linea[i]!=0){
			LCD_symbolToLocalBuffer(nLin,linea[i]);
			i++;
		}
}

//cola
static void Init_MsgQueue_LCD (void) {
  mid_MsgQueueLCD = osMessageQueueNew(SIZE_MSGQUEUE_LCD , sizeof(MSGQUEUE_LCD_t), NULL);
}

static void Init_Thread_LCD (void) {
  tid_Th_LCD = osThreadNew(Thread_LCD, NULL, NULL);
}
static void Thread_LCD (void *argument) {
	MSGQUEUE_LCD_t localObject;
  LCD_reset();
	LCD_init();
  while (1) {
		osMessageQueueGet(mid_MsgQueueLCD, &localObject, NULL, osWaitForever);
		LCD_LineaToLocalBuffer(localObject.buf, localObject.nLin);
		if(localObject.nLin==1){
			LCD_update_L1();
		}else if(localObject.nLin==2){
			LCD_update_L2();
		}
		
	}
}
osMessageQueueId_t getModLCDQueueID(void){
	return mid_MsgQueueLCD;
}	
	/*---------------------------MODULO----------------------------------*/
void initModLCD(void){
	Init_MsgQueue_LCD();
	Init_Thread_LCD();
} 
	
