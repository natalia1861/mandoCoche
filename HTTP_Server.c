/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "main.h"

#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE
#include "LEDs.h"
#include "adc.h"
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "Board_LED.h"                  // ::Board Support:LED
#include "http_server.h"
#include "principal.h"
//#include "Board_Buttons.h"              // ::Board Support:Buttons
//#include "Board_ADC.h"                  // ::Board Support:A/D Converter
//#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
//#include "GLCD_Config.h"                // Keil.MCBSTM32F400::Board Support:Graphic LCD

// Main stack size must be multiple of 8 Bytes
#define APP_MAIN_STK_SZ (1024U)
uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};

extern uint16_t AD_in          (uint32_t ch);
extern void netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len);

uint8_t cnt;
extern uint8_t  get_button (void);


/*---------------------VARIABLES USUARIOS------------*/

#define SIZE_MSGQUEUE_USERS			5
//#define SIZE_MSGQUEUE_CONTROL			3

//from principal.c
extern osThreadId_t tid_Thread_principal;		

char add_text[3][20+1];	//texto de un user creo
char delete_text[5][20+1];	//texto para borrar los users

//colas
static osMessageQueueId_t web_queue;
osMessageQueueId_t mid_MsgQueueUsers;   
osMessageQueueId_t mid_MsgQueueAlerts; 
//osMessageQueueId_t mid_MsgQueueControl; 

static void Init_MsgQueue_Webs (void);
static void Init_MsgQueue_Alerts (void);
static void Init_MsgQueue_Web (void);

//variables locales
static MSGQUEUE_WEB_t web_msg;

static uint32_t flags = 0;
bool consumo;
char lcd_text[2][20+1];		//diria que no lo voy a usar

//funciones locales
static bool isValidHex(const char* str, size_t len);
static bool isValidDecimal(const char* str, size_t len);
static bool isValidChar(const char* str, size_t len);
static void error(char *msg);
static void reinitUser (void);


/* Thread IDs */
osThreadId_t TID_Usuarios;

/* Thread declarations */
static void Usuarios  (void *arg);
__NO_RETURN void app_main (void *arg);


///* Read analog inputs */
uint16_t AD_in (uint32_t ch) {
  int32_t val = 0;

  if (ch == 0) {
    //handler definition
		//val=ADC_getVoltage(&adchandle ,10); //get values from channel 10->ADC123_IN10
   ADC_StartConversion();
    while (ADC_ConversionDone () < 0);
   val = ADC_GetValue();
  }
  return ((uint16_t)val);
}

//* Read digital inputs */
//uint8_t get_button (void) {
  //return ((uint8_t)Buttons_GetState ());
//}

/* IP address change notification */
void netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len) {

  (void)if_num;
  (void)val;
  (void)len;

  if (option == NET_DHCP_OPTION_IP_ADDRESS) {
    /* IP address change, trigger LCD update */
    osThreadFlagsSet (TID_Usuarios, 0x08);
  }
}


//static void Timer_Callback_Servidor (void const *arg) {
//  
//  if(encendidoRojo){
//    LED_Off(2);
//    encendidoRojo=0;
//  }else{
//    LED_On(2);
//    encendidoRojo=1;
//  }
//	cont_servidor++;
//	if (cont_servidor >= 8){
//		osTimerStop(tim_id2_servidor);
//		cont_servidor =0;
//		
//	}
//	
//}
//// Example: Create and Start timers
//int Init_Timers_Servidor (void) {
//  osStatus_t status;                            // function return status

//  // Create periodic timer
//  exec2_servidor = 2U;
//  tim_id2_servidor = osTimerNew((osTimerFunc_t)&Timer_Callback_Servidor, osTimerPeriodic, &exec2_servidor, NULL);
//  if (tim_id2_servidor != NULL) {  // Periodic timer created
//		
//    if (status != osOK) {
//      return -1;
//    }
//  }
//  return NULL;
//}


//COLA
static void Init_MsgQueue_Webs (void) {
  mid_MsgQueueUsers = osMessageQueueNew(SIZE_MSGQUEUE_USERS , sizeof(MSGQUEUE_USUARIOS_t), NULL);
}
static void Init_MsgQueue_Alerts (void) {
  mid_MsgQueueAlerts = osMessageQueueNew(SIZE_MSGQUEUE_ALERTS , sizeof(MSGQUEUE_ALERTS_t), NULL);
}

//cola para principal
static void Init_MsgQueue_Web (void) {
  web_queue = osMessageQueueNew(SIZE_MSGQUEUE_USERS , sizeof(MSGQUEUE_WEB_t), NULL);
}

osMessageQueueId_t getWebQueue (void) {
	return web_queue;
}
////COLA
//void Init_MsgQueue_Control (void) {
// 
//  mid_MsgQueueControl = osMessageQueueNew(SIZE_MSGQUEUE_CONTROL , sizeof(MSGQUEUE_USUARIOS_t), NULL);

//}






static __NO_RETURN void Usuarios (void *arg) {
   (void)arg;
	Init_MsgQueue_Webs();
  Init_MsgQueue_Alerts();
	Init_MsgQueue_Web();
  //Init_MsgQueue_Control();
  
  MSGQUEUE_USUARIOS_t users;
  MSGQUEUE_ALERTS_t alerts;
  
  strcpy(delete_text[0], "lorena1");
	strcpy(delete_text[1], "lorena2");
	strcpy(delete_text[2], "lorena3");
	strcpy(delete_text[3], "lorena4");
	strcpy(delete_text[4], "lorena5");

  
  	sprintf (users.users[0], "%-20s", delete_text[0]);
		sprintf (users.users[1], "%-20s", delete_text[1]);
		sprintf (users.users[2], "%-20s", delete_text[2]);
		sprintf (users.users[3], "%-20s", delete_text[3]);
		sprintf (users.users[4], "%-20s", delete_text[4]);
    
    sprintf (alerts.gas, "%-20s", "No hay fuga de gas");
		sprintf (alerts.fuego, "%-20s", "No hay fuego");
		sprintf (alerts.piezo, "%-24s", "No se ha detectado");
    
//    sprintf (control.gas, "%s", "no se ha detectado fuga");
//		sprintf (control.fuego, "%s", "no fuego");
//		sprintf (control.lcd, "%s", "hola");

  while(1) {
    flags = osThreadFlagsWait(0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20| 0x40 | 0x80 | 0x100 | 0x200, osFlagsWaitAny, osWaitForever);
		reinitUser();
		if (flags & 0x01) { //funciona añadir usuarios
			if (isValidHex(add_text[0], 10) & isValidChar(add_text[1], strlen(add_text[1])) & isValidDecimal(add_text[2], 4)) {
				web_msg.action = 1;
				for (int i = 0; i < 5; i++) {
				char temp[3] = { add_text[0][i*2], add_text[0][i*2 + 1], '\0' };
				web_msg.id[i] = (uint8_t)strtoul(temp, NULL, 16);	//16 porque es la base del hexadecimal
				}
				// Copy add_text[1] to name (ensure it fits)
				strncpy(web_msg.name, add_text[1], sizeof(web_msg.name) - 1);
				web_msg.name[sizeof(web_msg.name) - 1] = '\0'; // Ensure null-termination

				// Convert add_text[2] (password string) to uint8_t array
				for (int i = 0; i < 4; i++) {
					char temp[3] = { add_text[2][i * 2], add_text[2][i * 2 + 1], '\0' };
					web_msg.password[i] = (uint8_t)strtoul(temp, NULL, 10); // Convert decimal string to uint8_t
				}
				if (osMessageQueuePut(web_queue, &web_msg, NULL, 50) == osOK) {
					osThreadFlagsSet (tid_Thread_principal, FLAG_WEB);
				}	
			} else {
					error("parametros incorrectos");
			}				
    } 
    if (flags & 0x02) {	//elimina user1
      web_msg.action = 3;
			if (osMessageQueuePut(web_queue, &web_msg, NULL, 50) == osOK) {
				osThreadFlagsSet (tid_Thread_principal, FLAG_WEB);
			}
    }
    if (flags & 0x04) {//elimina user2
      web_msg.action = 4;
			if (osMessageQueuePut(web_queue, &web_msg, NULL, 50) == osOK) {
				osThreadFlagsSet (tid_Thread_principal, FLAG_WEB);
			}
    }
    if (flags & 0x08) {//elimina user3
      web_msg.action = 5;
			if (osMessageQueuePut(web_queue, &web_msg, NULL, 50) == osOK) {
				osThreadFlagsSet (tid_Thread_principal, FLAG_WEB);
			}
    }
    if (flags & 0x10) {//elimina user4
      web_msg.action = 6;
			if (osMessageQueuePut(web_queue, &web_msg, NULL, 50) == osOK) {
				osThreadFlagsSet (tid_Thread_principal, FLAG_WEB);
			}
    }
    if (flags & 0x20) {//elimina user5
      web_msg.action = 7;
			if (osMessageQueuePut(web_queue, &web_msg, NULL, 50) == osOK) {
				osThreadFlagsSet (tid_Thread_principal, FLAG_WEB);
			}
    }
    if (flags & 0x40) {//show users
			web_msg.action = 2;
			if (osMessageQueuePut(web_queue, &web_msg, NULL, 50) == osOK) {
				osThreadFlagsSet (tid_Thread_principal, FLAG_WEB);
			}
    }
    if (flags & 0x80) { //funciona desactiva alarma
			web_msg.action = 8;
			if (osMessageQueuePut(web_queue, &web_msg, NULL, 50) == osOK) {
				osThreadFlagsSet (tid_Thread_principal, FLAG_WEB);
			}
    }

    if (flags & 0x100) {	//enseña alarma
			web_msg.action = 9;
			if (osMessageQueuePut(web_queue, &web_msg, NULL, 50) == osOK) {
				osThreadFlagsSet (tid_Thread_principal, FLAG_WEB);
			}
    }
    if (flags & 0x200) { //SALIR
			web_msg.action = 10;
			if (osMessageQueuePut(web_queue, &web_msg, NULL, 50) == osOK) {
				osThreadFlagsSet (tid_Thread_principal, FLAG_WEB);
			}
    }


    
//   strcpy(delete_text[0], "lorena1v2");
//    strcpy(delete_text[1], "lorena2v2");
//    strcpy(delete_text[2], "lorena3v2");
//    strcpy(delete_text[3], "lorena4v2");
//    strcpy(delete_text[4], "lorena5v2");
// sprintf (users.user1, "%-20s", delete_text[0]);
//    sprintf (users.user2, "%-20s", delete_text[1]);
//    sprintf (users.user3, "%-20s", delete_text[2]);
//    sprintf (users.user4, "%-20s", delete_text[3]);
//    sprintf (users.user5, "%-20s", delete_text[4]);
   
//    /* Send user data to Principal */
//    sprintf (users.bufUsers, "%-20s", add_text[0]);
//		sprintf (users.bufPassword, "%-20s", add_text[1]);
//		osMessageQueuePut(mid_MsgQueueUsers, &users, 0U, 0U);
//    osDelay(1000);
//    osMessageQueueGet(mid_MsgQueueUsers, &users, NULL, osWaitForever);
//    encontrado=1;

    

  }
}

//VALIDACION DE DATOS
static bool isValidHex(const char* str, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (!isxdigit((unsigned char)str[i])) {
            return false;
        }
    }
    return true;
}

static bool isValidDecimal(const char* str, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char)str[i])) {
            return false;
        }
    }
    return true;
}

static bool isValidChar(const char* str, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (!isprint((unsigned char)str[i])) {
            return false;
        }
    }
    return true;
}

static void reinitUser (void) {
	memset(&web_msg, '\0', sizeof(web_msg));
}
	

static void error(char *msg) {
	web_msg.action = 0;
	strcpy (web_msg.name, msg);
}

osMessageQueueId_t getQueueUsers (void) {
	return mid_MsgQueueUsers;
}

osMessageQueueId_t getQueueAlerts (void) {
	return mid_MsgQueueAlerts;
}

/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
__NO_RETURN void app_main (void *arg) {
  (void)arg;

  netInitialize ();
	ADC_Initialize();
  TID_Usuarios = osThreadNew (Usuarios,  NULL, NULL);
	Init_Thread_principal();
  osThreadExit();
}
