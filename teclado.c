#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "teclado.h"
#include "stdbool.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
#define MAX_DIGITOS 4

//thread
static osThreadId_t tid_ThreadTeclado;                        // thread id    
static void ThreadTeclado (void *argument);                   // thread function

//cola
static osMessageQueueId_t mid_MsgQueueTEC; 

//principal.c
extern osThreadId_t tid_Thread_principal;

//variable con princial
volatile bool teclado_enable = false;

//FUNCIONES

static char teclado_read(void);
static int Init_ThreadTeclado (void);
static void ThreadTeclado (void *argument);

//timer
static osTimerId_t tim_id_250ms;
static void Timer_Callback_250ms (void);
 
static int Init_ThreadTeclado (void) {
 	mid_MsgQueueTEC = osMessageQueueNew(QUEUE_MAX, sizeof(MSGQUEUE_TEC_t), NULL);
  tid_ThreadTeclado = osThreadNew(ThreadTeclado, NULL, NULL);
  if (tid_ThreadTeclado == NULL) {
    return(-1);
  }
 
  return(0);
}

static void teclado_init(void){
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	/*GPIO PORTS CLOCK ENABLE*/
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	
	GPIO_InitStruct.Pin=ROW1_PIN;
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(ROW1_PORT,&GPIO_InitStruct);
	
	GPIO_InitStruct.Pin=ROW2_PIN;
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(ROW2_PORT,&GPIO_InitStruct);
	
	GPIO_InitStruct.Pin=ROW3_PIN;
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(ROW3_PORT,&GPIO_InitStruct);
	
	GPIO_InitStruct.Pin=ROW4_PIN;
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(ROW4_PORT,&GPIO_InitStruct);
	
	/*Initialized RESET*/
	HAL_GPIO_WritePin(ROW1_PORT,ROW1_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ROW2_PORT,ROW2_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ROW3_PORT,ROW3_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ROW4_PORT,ROW4_PIN,GPIO_PIN_RESET);
	
	GPIO_InitStruct.Pin=COL1_PIN;
	GPIO_InitStruct.Mode=GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull=GPIO_PULLUP;
	HAL_GPIO_Init(COL1_PORT,&GPIO_InitStruct);
	
	GPIO_InitStruct.Pin=COL2_PIN;
	GPIO_InitStruct.Mode=GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull=GPIO_PULLUP;
	HAL_GPIO_Init(COL2_PORT,&GPIO_InitStruct);
	
	GPIO_InitStruct.Pin=COL3_PIN;
	GPIO_InitStruct.Mode=GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull=GPIO_PULLUP;
	HAL_GPIO_Init(COL3_PORT,&GPIO_InitStruct);
	
	GPIO_InitStruct.Pin=COL4_PIN;
	GPIO_InitStruct.Mode=GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull=GPIO_PULLUP;
	HAL_GPIO_Init(COL4_PORT,&GPIO_InitStruct);
	
//		/*Initialized RESET*/
//	HAL_GPIO_WritePin(COL1_PORT,COL1_PIN,GPIO_PIN_SET);
//	HAL_GPIO_WritePin(COL2_PORT,COL2_PIN,GPIO_PIN_SET);
//	HAL_GPIO_WritePin(COL3_PORT,COL3_PIN,GPIO_PIN_SET);
//	HAL_GPIO_WritePin(COL4_PORT,COL4_PIN,GPIO_PIN_SET);
//	
}

static char teclado_read(void){
	char letras[4][4]={{'1','2','3','A'},
										 {'4','5','6','B'},
										 {'7','8','9','C'},
										 {'*','0','#','D'}};
	
	int i=0;
	char valorTecla=0;
	
	for(i=0;i<4;i++){
		if(i==0){
			HAL_GPIO_WritePin(ROW2_PORT,ROW2_PIN,GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW3_PORT,ROW3_PIN,GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW4_PORT,ROW4_PIN,GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW1_PORT,ROW1_PIN,GPIO_PIN_RESET);
			
			osDelay(10); //Bounce time <5ms
			while(HAL_GPIO_ReadPin(COL1_PORT,COL1_PIN)==0){
				valorTecla=letras[0][0]; //1
			}
			while(HAL_GPIO_ReadPin(COL2_PORT,COL2_PIN)==0){
				valorTecla=letras[0][1]; //2
			}
			while(HAL_GPIO_ReadPin(COL3_PORT,COL3_PIN)==0){
				valorTecla=letras[0][2]; //3
			}
			while(HAL_GPIO_ReadPin(COL4_PORT,COL4_PIN)==0){
				valorTecla=letras[0][3]; //A
			}
		}
		if(i==1){
			HAL_GPIO_WritePin(ROW1_PORT,ROW1_PIN,GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW3_PORT,ROW3_PIN,GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW4_PORT,ROW4_PIN,GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW2_PORT,ROW2_PIN,GPIO_PIN_RESET);
			
			osDelay(10); //Bounce time <5ms
			while(HAL_GPIO_ReadPin(COL1_PORT,COL1_PIN)==0){
				valorTecla=letras[1][0]; //4
			}
			while(HAL_GPIO_ReadPin(COL2_PORT,COL2_PIN)==0){
				valorTecla=letras[1][1]; //5
			}
			while(HAL_GPIO_ReadPin(COL3_PORT,COL3_PIN)==0){
				valorTecla=letras[1][2]; //6
			}
			while(HAL_GPIO_ReadPin(COL4_PORT,COL4_PIN)==0){
				valorTecla=letras[1][3]; //B
			}
		}
		if(i==2){
			HAL_GPIO_WritePin(ROW1_PORT,ROW1_PIN,GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW2_PORT,ROW2_PIN,GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW4_PORT,ROW4_PIN,GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW3_PORT,ROW3_PIN,GPIO_PIN_RESET);
			
			osDelay(10); //Bounce time <5ms
			while(HAL_GPIO_ReadPin(COL1_PORT,COL1_PIN)==0){
				valorTecla=letras[2][0]; //7
			}
			while(HAL_GPIO_ReadPin(COL2_PORT,COL2_PIN)==0){
				valorTecla=letras[2][1]; //8
			}
			while(HAL_GPIO_ReadPin(COL3_PORT,COL3_PIN)==0){
				valorTecla=letras[2][2]; //9
			}
			while(HAL_GPIO_ReadPin(COL4_PORT,COL4_PIN)==0){
				valorTecla=letras[2][3]; //C
			}
		}
		if(i==3){
			HAL_GPIO_WritePin(ROW1_PORT,ROW1_PIN,GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW2_PORT,ROW2_PIN,GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW3_PORT,ROW3_PIN,GPIO_PIN_SET);
			HAL_GPIO_WritePin(ROW4_PORT,ROW4_PIN,GPIO_PIN_RESET);
			
			osDelay(10); //Bounce time <5ms
			while(HAL_GPIO_ReadPin(COL1_PORT,COL1_PIN)==0){
				valorTecla=letras[3][0]; //*
			}
			while(HAL_GPIO_ReadPin(COL2_PORT,COL2_PIN)==0){
				valorTecla=letras[3][1]; //0
			}
			while(HAL_GPIO_ReadPin(COL3_PORT,COL3_PIN)==0){
				valorTecla=letras[3][2]; //#
			}
			while(HAL_GPIO_ReadPin(COL4_PORT,COL4_PIN)==0){
				valorTecla=letras[3][3]; //D
			}
		}
	}
	return valorTecla;
}
 
static void ThreadTeclado (void *argument) {
	static uint32_t flags = 0;
	char teclaPulsada=0;
	MSGQUEUE_TEC_t localObject;
 
	tim_id_250ms = osTimerNew((osTimerFunc_t)&Timer_Callback_250ms, osTimerPeriodic, NULL, NULL);
	//lo optimo seria utilizar interrupciones
  while (1){
		osThreadFlagsWait(FLAG_LEER_TECLA, osFlagsWaitAny, osWaitForever);		
		teclaPulsada=teclado_read();
		if(teclaPulsada!=0){
			localObject.tecla = teclaPulsada;
			osMessageQueuePut(mid_MsgQueueTEC, &localObject, NULL, 1000);
			osThreadFlagsSet(tid_Thread_principal, FLAG_TECLA);
		}
  }
}

void Init_ModTEC(void){
	teclado_init();	 //keyboard initialization
	Init_ThreadTeclado();
}

osThreadId_t getModTECThreadID(void){
 return tid_ThreadTeclado;
}

osMessageQueueId_t getMsgTEC(void){
	return mid_MsgQueueTEC;
}

//timer
static void Timer_Callback_250ms (void) {	
	osThreadFlagsSet(tid_ThreadTeclado, FLAG_LEER_TECLA);
}

void teclado_on (void) {
	osTimerStart (tim_id_250ms, 250);
}

void teclado_off (void) {
	osTimerStop(tim_id_250ms);
}

