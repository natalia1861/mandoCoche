#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "principal.h"
#include "rtc.h"
#include "lcd.h"
#include "sensors.h"
#include "temp.h"
#include "gas.h"
#include "teclado.h"
#include "http_server.h"
#include "rfid.h"
#include "flash.h"
#include "servomotor.h"
#include "proximidad.h"
#include "rfid.h"
#include "zumbador.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
 #define TEMP_REF	35
 #define GAS_REF  20
 
//hilo
osThreadId_t tid_Thread_principal;                        // thread id
static void Thread_principal (void *argument);                   // thread function

//variables globales
extern volatile bool proximidad_enable;

//colas
static osMessageQueueId_t lcd_queue;
static osMessageQueueId_t sensors_queue;
static osMessageQueueId_t gas_queue;
static osMessageQueueId_t web_queue;
static osMessageQueueId_t flash_put_queue;
static osMessageQueueId_t flash_get_queue;
static osMessageQueueId_t users_web_queue;
static osMessageQueueId_t alerts_web_queue;
static osMessageQueueId_t teclado_queue;
static osMessageQueueId_t rfid_queue;

//id hilo
static osThreadId_t tid_flash;
static osThreadId_t tid_servo;
static osThreadId_t tid_zumbador;

//messages
extern MSGQUEUE_RTC_t rtc_date_time;
static MSGQUEUE_LCD_t lcd_msg;
static MSGQUEUE_SENSORS_t sensors_msg;
static MSGQUEUE_GAS_t gas_msg;
static MSGQUEUE_WEB_t web_msg;
static MSGQUEUE_FLASH_t flash_msg;
static MSGQUEUE_USUARIOS_t users_web_msg;
static MSGQUEUE_ALERTS_t alerts_web_msg;
static MSGQUEUE_TEC_t teclado_msg;
static MSGQUEUE_RC522_t rfid_msg;

//internal functions
static void initThreads(void);
static void initQueues(void);
void test_flash_users (void *argument);
void test_flash_events (void *argument);
static void reinitMsgs (void);
static bool comprobarFuego (void);
static bool comprobarGas (void);

//variables locales
static uint8_t id_users[5][5];
static uint8_t password[4];
static uint8_t index_tecla = 0;
static volatile bool tim_30seg_expired = false;

//timer 30seg
static osTimerId_t tim_id_30seg;
static void timer_callback (void *arg);
static void start_timer_30seg(void);
static void stop_timer_30seg(void);

int Init_Thread_principal (void) {
  tid_Thread_principal = osThreadNew(Thread_principal, NULL, NULL);
  if (tid_Thread_principal == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_principal (void *argument) {
	uint32_t flags;
	uint8_t var = 0;
	initThreads();
	initQueues();

	sprintf(lcd_msg.buf,"processing threads");
	lcd_msg.nLin=1;
	osMessageQueuePut(lcd_queue, &lcd_msg, NULL, 50);
	
	tim_id_30seg = osTimerNew(timer_callback, osTimerOnce, NULL, NULL);
  while (1) {
		flags = osThreadFlagsWait(FLAG_RTC | SENSOR_FLAG | FLAG_WEB | FLAG_PROXIMIDAD | ALERTA | ALARMA, osFlagsWaitAny, osWaitForever);
		reinitMsgs();
     if (flags & FLAG_RTC) {
			  sprintf(lcd_msg.buf,"%d", var++);
				lcd_msg.nLin=1;
				osMessageQueuePut(lcd_queue, &lcd_msg, NULL, 50);
				sprintf(lcd_msg.buf, "%-20s",rtc_date_time.rtc[1]);
				lcd_msg.nLin=2;
				osMessageQueuePut(lcd_queue, &lcd_msg, NULL, 50);
		 }
		 if (flags & SENSOR_FLAG) {
				osMessageQueueGet (sensors_queue, &sensors_msg, NULL, 50);
			 //gestionar como mirarlo (aparece temperatra, gas, piezo y consumo
//			 	sprintf(lcd_msg.buf,"%-20.2f", sensors_msg.temp);
//				lcd_msg.nLin=1;
//				osMessageQueuePut(lcd_queue, &lcd_msg, NULL, 50);
		 }
		 if (flags & FLAG_PROXIMIDAD) {
			 tim_30seg_expired = false;
			 teclado_msg.tecla = '0';
			 index_tecla = 0;
			 //ACTIVA RFID
			 det_rfid_on();
			 //desactiva proximidad durante todo el proceso
			 proximidad_enable = false;
			 //tienes 20 segundos para pasar la tarjeta
			 osThreadFlagsWait (RFID_READID, osFlagsWaitAny, 20000);
			 //si se detecta tarjeta, se comprueba que exista
			 if (osMessageQueueGet (rfid_queue, &rfid_msg, NULL, 500) == osOK) {
					flash_msg.action = 7;
					memcpy(flash_msg.id, rfid_msg.id, 5);
					if (osMessageQueuePut (flash_put_queue, &flash_msg, NULL, 500) == osOK) {
						if (osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 2000)==osOK) {	//en caso de que exista, se coge el mensaje
							//se espera durante 30 segundos obtener la contraseña
							teclado_on();
							start_timer_30seg();
							while (teclado_msg.tecla != '#' | tim_30seg_expired==false) {		//no se haya confirmado la contraseña o se haya pasado el tiempo
								if (osMessageQueueGet(teclado_queue, &teclado_msg, NULL, 30000)==osOK) {	//se coge una tecla
									if (index_tecla == 3) {
										//error contraseña completa
									} else {
										password[index_tecla] = teclado_msg.tecla;
									}
								}
							}
							if (password == flash_msg.password) {//si contraseña ok, se abre servo, y tras 10 segundos se cierra
								osThreadFlagsSet (tid_servo, OPEN_FLAG);
								
							} else {	//si no se entra a modo alerta (tanto si la contraseña es erronea como si no se ha metido contraseña)
								osThreadFlagsSet (tid_Thread_principal, ALERTA);
							}
						}
					}
			 } //si no se detecta tarjeta o se completan todas las acciones
			 det_rfid_off();	//desactiva rfid
			 proximidad_enable = true;	//habilita proximidad de nuevo
			 stop_timer_30seg();	//para el timer
		 }
		 if (flags & ALERTA) {
			 osThreadFlagsSet (tid_zumbador, FLAG_ALERTA);
		 }
		 
		 if (flags & ALARMA) {
			 osThreadFlagsSet (tid_zumbador, FLAG_ALARMA);
		 }
		 if (flags & FLAG_WEB) {
			 osMessageQueueGet (web_queue, &web_msg, NULL, 1000);
			 switch (web_msg.action) {
				 case 0:	//error
					sprintf(lcd_msg.buf, "%-20s",web_msg.name);
					lcd_msg.nLin=2;
					osMessageQueuePut(lcd_queue, &lcd_msg, NULL, 500);
				 break;
				 case 1:	//add user
					flash_msg.action = 1;
					memcpy(flash_msg.id, web_msg.id, 5);
					strcpy (flash_msg.name, web_msg.name);
					memcpy(flash_msg.password, web_msg.password, 4);
					if (osMessageQueuePut (flash_put_queue, &flash_msg, NULL, 500) == osOK) {
						osThreadFlagsSet(tid_flash, ADD_USER);
					}
				 break;
				 case 2: //getAllUsers
					flash_msg.action = 3;
					if (osMessageQueuePut (flash_put_queue, &flash_msg, NULL, 500) == osOK) {
					 if (osMessageQueueGet (flash_get_queue, &flash_msg, NULL, 5000) == osOK) {		//si se encuentra el primer usuario
						strcpy (users_web_msg.users[0], flash_msg.name);
						for (int i = 1; i < flash_msg.numUsers; i++) {		//se añaden los demas
							if (osMessageQueueGet (flash_get_queue, &flash_msg, NULL, 5000) == osOK) {
							strcpy (users_web_msg.users[i], flash_msg.name);
							}
						}
						osMessageQueuePut(users_web_queue, &users_web_msg, NULL, 2000);		//se añade a la cola de web users
						
					} else {
						sprintf(lcd_msg.buf, "Error. No User");
						lcd_msg.nLin=2;
						osMessageQueuePut(lcd_queue, &lcd_msg, NULL, 50);
						osMessageQueuePut(users_web_queue, &users_web_msg, NULL, 2000);		//se añade a la cola de web users vacia
					}
				} else {
					sprintf(lcd_msg.buf, "Error Add User");
					lcd_msg.nLin=2;
					osMessageQueuePut(lcd_queue, &lcd_msg, NULL, 50);
					osMessageQueuePut(users_web_queue, &users_web_msg, NULL, 2000);		//se añade a la cola de web users vacia
				}
					osThreadFlagsClear (FLAG_WEB);
				 break;
//				 case 3:
//					flash_msg.action = 3;
//					if (osMessageQueuePut (flash_put_queue, &flash_msg, NULL, 500) == osOK) {		//mandamos accion de gerAll
//					 if (osMessageQueueGet (flash_get_queue, &flash_msg, NULL, 5000) == osOK) {		//si se encuentra el primer usuario, se elimina
//						 flash_msg.action = 2;	//accion de eliminar usuario
//						 if (osMessageQueuePut (flash_put_queue, &flash_msg, NULL, 5000) == osOK) {
//							 //todo ok
//						 } else {
//							sprintf(lcd_msg.buf, "Error. Delete user");
//							lcd_msg.nLin=2;
//							osMessageQueuePut(lcd_queue, &lcd_msg, NULL, 50);
//						 }
//					 }
//					}
//				 break;
//				case 4:
//					flash_msg.action = 3;
//					if (osMessageQueuePut (flash_put_queue, &flash_msg, NULL, 500) == osOK) {		//mandamos accion de gerAll
//					 if (osMessageQueueGet (flash_get_queue, &flash_msg, NULL, 5000) == osOK) {		//si se encuentra el primer usuario, se elimina
//							if (osMessageQueueGet (flash_get_queue, &flash_msg, NULL, 5000) == osOK) {	//si se encuentra al segundo
//							flash_msg.action = 2;	//accion de eliminar usuario
//						 if (osMessageQueuePut (flash_put_queue, &flash_msg, NULL, 5000) == osOK) {	//se elimina
//							 //todo ok
//						 } else {
//							sprintf(lcd_msg.buf, "Error. Delete user");
//							lcd_msg.nLin=2;
//							osMessageQueuePut(lcd_queue, &lcd_msg, NULL, 50);
//						 }
//					 }
//					}
//				 break;
					
				case 3: 	//elimina usuarios
				case 4:
				case 5:
				case 6:
				case 7:
					flash_msg.action = 3;
					if (osMessageQueuePut (flash_put_queue, &flash_msg, NULL, 500) == osOK) {
					 if (osMessageQueueGet (flash_get_queue, &flash_msg, NULL, 5000) == osOK) {		//si se encuentra el primer usuario
						memcpy (id_users[0], flash_msg.id, sizeof(id_users[0]));
						for (int i = 1; i < flash_msg.numUsers; i++) {		//se añaden los demas
							if (osMessageQueueGet (flash_get_queue, &flash_msg, NULL, 5000) == osOK) {
							memcpy (id_users[i], flash_msg.id, sizeof(id_users[i]));
							}
						}
						//eliminamos el que sea
						flash_msg.action = 2;
						memcpy (flash_msg.id, id_users[web_msg.action-3], sizeof(flash_msg.id));
						if (osMessageQueuePut (flash_put_queue, &flash_msg, NULL, 500)) {
							//todo ok
						} else {
							sprintf(lcd_msg.buf, "Error. Delete user");
							lcd_msg.nLin=2;
							osMessageQueuePut(lcd_queue, &lcd_msg, NULL, 50);
						}
					 }
				  }
				 break;
				case 8:
						osThreadFlagsSet (tid_zumbador, FLAG_ZSTOP);
					break;
				case 9:	//enseña alertas
					flash_msg.action = 6;
						//medidas
					if (comprobarFuego) {
						memcpy (alerts_web_msg.fuego, "Se detecto fuego", sizeof(alerts_web_msg.fuego));
					} else {
						memcpy (alerts_web_msg.fuego, "No hay fuego", sizeof(alerts_web_msg.fuego));
					}
					if (comprobarGas) {
						memcpy (alerts_web_msg.fuego, "Se detecto gas", sizeof(alerts_web_msg.fuego));
					} else {
						memcpy (alerts_web_msg.fuego, "No hay gas", sizeof(alerts_web_msg.fuego));
					}
					//FALTA PIEZO
					break;
			}
		}
	}
}

static void initQueues(void) {
	lcd_queue = getModLCDQueueID();
	sensors_queue = getSensorsQueue();
	gas_queue = getGasQueueID();
	web_queue = getWebQueue();
	users_web_queue = getQueueUsers();
	alerts_web_queue = getQueueAlerts();
	flash_get_queue = getOutputQueue();
	flash_put_queue = getInputQueue();
	teclado_queue = getMsgTEC();
	rfid_queue = getRFIDQueue();
}

static void initThreads(void) {
	tid_flash = Init_ThFlash();		//flash
	RTC_init();										//RTC
	initModLCD();									//LCD
	initModZumbador();						//zumbador
	Init_Sensores_peligro();			//temp y gas (falta piezo)
	Init_Prox();									//prximidad
	Init_ModTEC();								//teclado
	Init_ThMFRC522();							//rfid
	tid_servo = Init_ThServo();		//servo
	tid_zumbador = getModPWMThreadID();
	osDelay(10000);
}

static void reinitMsgs (void) {
	for (int i = 0; i < 5; i++) {
		memset(users_web_msg.users[i], '\0', sizeof(users_web_msg.users));
	}
}

static bool comprobarFuego (void) {
	if (sensors_msg.temp > TEMP_REF) {
		return true;
	}
	return false;
}

static bool comprobarGas (void) {
		if (gas_msg.gas > GAS_REF) {
		return true;
	}
	return false;
}

static void timer_callback (void *arg) {
	tim_30seg_expired = true;
}
static void start_timer_30seg(void) {
	osTimerStart (tim_id_30seg, 30000);
}
static void stop_timer_30seg(void) {
	osTimerStop(tim_id_30seg);
}
void test_flash_users (void *argument) {
	initThreads();
	initQueues();
	uint32_t flags;
  while (1) {
		add_user1(); //natalia
		add_user2(); //ainara
		add_user3(); //lorena
		//add the same 3 users to see if they are really added (must not)
		add_user1();
		add_user2();
		add_user3();
		osDelay (5000);
		//we get all the users, to see if there are only 3 new users
		flash_msg.action = 3;
		osMessageQueuePut(flash_put_queue, &flash_msg, NULL, 3000);
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000); //natalia
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000); //ainara
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000); //lorena
    
    //osMessageQueueGet(flash_queue, &user, NULL, osWaitForever);
		//osMessageQueueGet(flash_queue, &user, NULL, osWaitForever);
		//we delete user 1
    uint8_t id[5];
    id[0] = 0xD3;
    id[1] = 0xB0;
    id[2] = 0x02;
    id[3] = 0xF8;
    id[4] = 0x99;
	
    memcpy(flash_msg.id, id, 5);
		flash_msg.action = 2;
		osMessageQueuePut(flash_put_queue, &flash_msg, NULL, 3000);
		//we get the 2 remain users
		flash_msg.action = 3;
		osMessageQueuePut(flash_put_queue, &flash_msg, NULL, 3000);
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000); //ainara
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000); //lorena
		//we add user 4 (it must be in position 1)
		add_user4(); //amaya
		osDelay(2000);
		//we get all the users, to see if there are 3 again
		flash_msg.action = 3;
		osMessageQueuePut(flash_put_queue, &flash_msg, NULL, 3000);
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000); //amaya
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000); //ainara
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000); //lorena
		//we get 1 user information
	id[0] = 0xD5;
	id[1] = 0xB2;
	id[2] = 0x02;
	id[3] = 0xF8;
	id[4] = 0x99;
		memcpy(flash_msg.id, id, 5);
		flash_msg.action = 7;
		osMessageQueuePut(flash_put_queue, &flash_msg, NULL, 3000);
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000); //amaya
    //we add user 1
		add_user1();
		osDelay(2000);
    flash_msg.action = 3;
		osMessageQueuePut(flash_put_queue, &flash_msg, NULL, 3000);
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000); //amaya
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000); //ainara
    osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000); //lorena
    osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000); //natalia
		//we wait
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000);
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000);
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000);
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000);
//    osMessageQueueGet(rfid_queue, &msg_rfid, NULL, osWaitForever);
//    mensaje_Recibido = true;
  }
}

void test_flash_events (void *argument) {
	initThreads();
	initQueues();
  uint32_t flags;
  while (1) {
    add_user1();
		osDelay(1000);
    add_event1();
		osDelay(1000);
    add_event1();
		osDelay(1000);
	  flash_msg.action = 6;	//getAllEvents
		osMessageQueuePut(flash_put_queue, &flash_msg, NULL, 3000);
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000);
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000);

		flash_msg.action = 5;	//deleteAllEvents
		osMessageQueuePut(flash_put_queue, &flash_msg, NULL, 3000);

		//add1event
    add_event1();
    osDelay(1000);
		flash_msg.action = 6;	//getAllEvents
		osMessageQueuePut(flash_put_queue, &flash_msg, NULL, 3000);
    osMessageQueueGet(flash_get_queue, &flash_msg, NULL, 3000);
		osMessageQueueGet(flash_get_queue, &flash_msg, NULL, osWaitForever);
  }
}
