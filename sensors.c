#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "sensors.h"
#include "temp.h"
#include "gas.h"
#include "consumo.h"
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
#define SIZE_MSGQUEUE_SENSORS 3
//from principal.c
extern osThreadId_t tid_Thread_principal;

//hilo
osThreadId_t tid_Thread_sensor;                        // thread id
static void medir_sensores (void *argument);                   // thread function

//timer medidas
static osTimerId_t tim_id_10seg;	//timer medidas cada 10 segundos
static void Timer_Callback_10seg (void);

//QUEUE
static osMessageQueueId_t sensors_queue;
static osMessageQueueId_t temp_queue;
static osMessageQueueId_t gas_queue;
static osMessageQueueId_t adc_queue;

int Init_Sensores_peligro (void) {
	initModTemp(); //inicializa temperatura
	Init_ModGas(); //inicializa gas
	Init_ModADC();
	
	tim_id_10seg = osTimerNew((osTimerFunc_t)&Timer_Callback_10seg, osTimerPeriodic, NULL, NULL);
	osTimerStart (tim_id_10seg, 10000);
	
  tid_Thread_sensor = osThreadNew(medir_sensores, NULL, NULL);
  if (tid_Thread_sensor == NULL) {
    return(-1);
  }
  return(0);
}
 
void medir_sensores (void *argument) {
	uint32_t flags;
	MSGQUEUE_SENSORS_t sensors_msg;
	MSGQUEUE_ADC_t adc_msg;
  while (1) {
		osThreadFlagsWait(FLAG_MEDIR, osFlagsWaitAny, osWaitForever);
		leer_temp();
		leer_gas();
		leer_consumo();
		flags = osThreadFlagsWait(TEMP_FLAG | GAS_FLAG | ADC_FLAG, osFlagsWaitAll, osWaitForever);
		if (flags & TEMP_FLAG) {
			if (osMessageQueueGet (temp_queue, &sensors_msg.temp, NULL, 500)==osOK) {
				if (osMessageQueuePut (sensors_queue, &sensors_msg, NULL, 500)==osOK){
					osThreadFlagsSet (tid_Thread_principal, SENSOR_FLAG);
				}
			}
		}
		if (flags & GAS_FLAG) {
			if (osMessageQueueGet (gas_queue, &sensors_msg.gas, NULL, 500)==osOK) {
				if (osMessageQueuePut (sensors_queue, &sensors_msg, NULL, 500)==osOK) {
					osThreadFlagsSet (tid_Thread_principal, SENSOR_FLAG);
				}
			}
		}
		if (flags & ADC_FLAG) {
			if (osMessageQueueGet (adc_queue, &adc_msg, NULL, 500)==osOK) {
				sensors_msg.consumo = adc_msg.consumo;
				sensors_msg.pres = adc_msg.pres;
				if (osMessageQueuePut (sensors_queue, &sensors_msg, NULL, 500)==osOK) {
					osThreadFlagsSet (tid_Thread_principal, SENSOR_FLAG);
				}
			}
		}
  }
}

static void Timer_Callback_10seg (void) {	//cada 10 seg se toman medidas
	osThreadFlagsSet(tid_Thread_sensor, FLAG_MEDIR);
}

static void Init_MsgQueue_Sensors (void) {
  sensors_queue = osMessageQueueNew(SIZE_MSGQUEUE_SENSORS , sizeof(MSGQUEUE_SENSORS_t), NULL);
	temp_queue = getTempQueueID();
	gas_queue = getGasQueueID();
	adc_queue = getMsgADCID();
}

osMessageQueueId_t getSensorsQueue(void){
	return sensors_queue;
}

void Init_mod_sensors(void) {
	Init_Sensores_peligro();
	Init_MsgQueue_Sensors();
}
