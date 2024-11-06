/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network:Service
 * Copyright (c) 2004-2018 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server_CGI.c
 * Purpose: HTTP Server CGI Module
 * Rev.:    V6.0.0
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE
#include "LEDs.h"
#include "http_server.h"
#include "cJSON.h"
#include "rtc.h"
//#include "Board_LED.h"                  // ::Board Support:LED

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic push
#pragma  clang diagnostic ignored "-Wformat-nonliteral"
#endif

// http_server.c
extern uint16_t AD_in (uint32_t ch);
extern uint8_t  get_button (void);

extern bool consumo;
extern char add_text[3][20+1];
extern osThreadId_t TID_Usuarios;
extern uint32_t cnt;

//rtc.c
extern MSGQUEUE_RTC_t rtc_date_time;

//colas from http_server.c
extern osMessageQueueId_t mid_MsgQueueUsers; 
extern osMessageQueueId_t mid_MsgQueueAlerts; 

MSGQUEUE_USUARIOS_t users;
MSGQUEUE_ALERTS_t alerts;

extern char delete_text[5][20+1];

//internal functions
static void reinitUsers (void);



// Local variables.
static uint8_t P2;
static uint8_t ip_addr[NET_ADDR_IP6_LEN];
static char    ip_string[40];
static char tiempo[24];
static char date[24];

static char user1[24];
static char user2[24];
static char user3[24];
static char user4[24];
static char user5[24];

static char alert1[24];
static char alert2[24];
static char alert3[24];

static char gas[24];
static char fuego[24];
static char lcd[24];


// My structure of CGI status variable.
typedef struct {
  uint8_t idx;
  uint8_t unused[3];
} MY_BUF;
#define MYBUF(p)        ((MY_BUF *)p)

// Process query string received by GET request.
void netCGI_ProcessQuery (const char *qstr) {
  netIF_Option opt = netIF_OptionMAC_Address;
  int16_t      typ = 0;
  char var[40];

  do {
    // Loop through all the parameters
    qstr = netCGI_GetEnvVar (qstr, var, sizeof (var));
    // Check return string, 'qstr' now points to the next parameter

    switch (var[0]) {
      case 'i': // Local IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_Address;       }
        else               { opt = netIF_OptionIP6_StaticAddress; }
        break;

      case 'm': // Local network mask
        if (var[1] == '4') { opt = netIF_OptionIP4_SubnetMask; }
        break;

      case 'g': // Default gateway IP address
        if (var[1] == '4') { opt = netIF_OptionIP6_DefaultGateway; }
        else               { opt = netIF_OptionIP6_DefaultGateway; }
        break;

      case 'p': // Primary DNS server IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_PrimaryDNS; }
        else               { opt = netIF_OptionIP6_PrimaryDNS; }
        break;

      case 's': // Secondary DNS server IP address
        if (var[1] == '4') { opt = netIF_OptionIP4_SecondaryDNS; }
        else               { opt = netIF_OptionIP6_SecondaryDNS; }
        break;
      
      default: var[0] = '\0'; break;
    }

    switch (var[1]) {
      case '4': typ = NET_ADDR_IP4; break;
      case '6': typ = NET_ADDR_IP6; break;

      default: var[0] = '\0'; break;
    }

    if ((var[0] != '\0') && (var[2] == '=')) {
      netIP_aton (&var[3], typ, ip_addr);
      // Set required option
      netIF_SetOption (NET_IF_CLASS_ETH, opt, ip_addr, sizeof(ip_addr));
    }
  } while (qstr);
}

// Process data received by POST request.
// Type code: - 0 = www-url-encoded form data.
//            - 1 = filename for file upload (null-terminated string).
//            - 2 = file upload raw data.
//            - 3 = end of file upload (file close requested).
//            - 4 = any XML encoded POST data (single or last stream).
//            - 5 = the same as 4, but with more XML data to follow.
void netCGI_ProcessData (uint8_t code, const char *data, uint32_t len) {
  char var[40],passw[12];
   

  if (code != 0) {
    // Ignore all other codes
    return;
  }

  P2 = 0;
  consumo = false;
  if (len == 0) {
    // No data or all items (radio, checkbox) are off
    Led_verde_reset();
    //LED_SetOut (P2);
    return;
  }
  passw[0] = 1;
  do {
    // Parse all parameters
    data = netCGI_GetEnvVar (data, var, sizeof (var));
    if (var[0] != 0) {
      // First character is non-null, string exists
//      if (strcmp (var, "led0=off") == 0) {
//        Led_verde_reset();
//        //P2 |= 0x01;
//      }
//      else if (strcmp (var, "led0=on") == 0) {
//        P2 |= 0x01;
//        Led_verde_set();
//      }
//      else if (strcmp (var, "led1=off") == 0) {
//        Led_azul_reset();
//        //P2 |= 0x02;
//      }
//      else if (strcmp (var, "led1=on") == 0) {
//        Led_azul_set();
//        P2 |= 0x02;
//      }
//      else if (strcmp (var, "led2=off") == 0) {
//        //P2 |= 0x04;
//        Led_rojo_reset();
//      }
//      else if (strcmp (var, "led2=on") == 0) {
//        P2 |= 0x04;
//        Led_rojo_set();
//      }else if (strcmp (var, "R=off") == 0) {
//        
//      }
//      else if (strcmp (var, "R=on") == 0) {
//        P2 |= 0x08;
//        
//      }else if (strcmp (var, "G=off") == 0) {
//       
//      }
//      else if (strcmp (var, "G=on") == 0) {
//        P2 |= 0x10;
//      }else if (strcmp (var, "B=off") == 0) {
//        //P2 |= 0x04;
//     
//      }
//      else if (strcmp (var, "B=on") == 0) {
//        P2 |= 0x20;
//       
//      }
//      else if (strcmp (var, "ctrl=Browser") == 0) {
//        LEDrun = false;
//				
//					Led_verde_reset();
//					Led_rojo_reset();
//					Led_azul_reset();
//				
//      }
//      if(strcmp (var, "ctrl=Running Lights") == 0){
//        LEDrun = true;
//      }else if ((strncmp (var, "pw0=", 4) == 0) ||
//               (strncmp (var, "pw2=", 4) == 0)) {
//        // Change password, retyped password
//        if (netHTTPs_LoginActive()) {
//          if (passw[0] == 1) {
//            strcpy (passw, var+4);
//          }
//          else if (strcmp (passw, var+4) == 0) {
//            // Both strings are equal, change the password
//            netHTTPs_SetPassword (passw);
//          }
//        }
//      }
      if (strncmp (var, "id=", 3) == 0) {
        strcpy (add_text[0], var+3);
        osThreadFlagsSet (TID_Usuarios, 0x01);
      }else if (strncmp (var, "nombre=", 7) == 0) {
        strcpy (add_text[1], var+7);
        osThreadFlagsSet (TID_Usuarios, 0x01);
      }else if (strncmp (var, "password=", 9) == 0) {
        strcpy (add_text[2], var+9);
        osThreadFlagsSet (TID_Usuarios, 0x01);
      }else if (strncmp (var, "pwdAlert=", 9) == 0) {
        strcpy (add_text[0], var+9);
        osThreadFlagsSet (TID_Usuarios, 0x80);
      }else if (strncmp (var, "pwdExit=", 8) == 0) {
        strcpy (add_text[0], var+8);
        osThreadFlagsSet (TID_Usuarios, 0x200);
      }else if(strcmp (var, "ctrlConsumo=Enable") == 0){
        consumo=true;
      }else if(strcmp (var, "ctrlConsumo=Disable") == 0){
        consumo=false;
      
      //else if (strcmp (var, "fuegoCheck=on") == 0) {
//        P2 |= 0x50;
//        osThreadFlagsSet (TID_Usuarios, 0x03);
//       
//      }else if (strcmp (var, "gasCheck=on") == 0) {
//        P2 |= 0x60;
//        osThreadFlagsSet (TID_Usuarios, 0x04);
//       
//      }else if (strcmp (var, "lcdCheck=on") == 0) {
//        P2 |= 0x70;
//        osThreadFlagsSet (TID_Usuarios, 0x05);
//       
//      }else if (strncmp (var, "pwdExit=",8) == 0) {
//				strcpy (add_text[0], var+8);
//        osThreadFlagsSet (TID_Usuarios, 0x06);
			}else if (strcmp (var, "user1Del=on") == 0) {
        P2 |= 0x01;
        osThreadFlagsSet (TID_Usuarios, 0x02);
       
      
			}else if (strcmp (var, "user2Del=on") == 0) {
        P2 |= 0x02;
        osThreadFlagsSet (TID_Usuarios, 0x04);
       
      }else if (strcmp (var, "user3Del=on") == 0) {
        P2 |= 0x04;
        osThreadFlagsSet (TID_Usuarios, 0x08);
       
      }else if (strcmp (var, "user4Del=on") == 0) {
        P2 |= 0x08;
        osThreadFlagsSet (TID_Usuarios, 0x10);
       
      }else if (strcmp (var, "user5Del=on") == 0) {
        P2 |= 0x10;
        osThreadFlagsSet (TID_Usuarios, 0x20);
       
      }
    }
  } while (data);
}

// Generate dynamic web data from a script line.
uint32_t netCGI_Script (const char *env, char *buf, uint32_t buflen, uint32_t *pcgi) {
  int32_t socket;
  netTCP_State state;
  NET_ADDR r_client;
  const char *lang;
  uint32_t len = 0U;
  uint8_t id;
  static uint32_t adv;
  netIF_Option opt = netIF_OptionMAC_Address;
  int16_t      typ = 0;
  
  
  
//        // Leer la entrada estándar para obtener los datos enviados desde el navegador
//    char *data1 = NULL;
//    size_t len1 = 0;
//    ssize_t read;
//    while ((read = fread(&data1, &len1, stdin)) != -1) {
//        // Procesar los datos, aquí parsearías la cadena JSON para obtener el valor de showButtonPressed
//        cJSON *root = cJSON_Parse(data);
//        if (root == NULL) {
//            printf("Error parsing JSON data\n");
//            return 1;
//        }

//        cJSON *showButtonPressedJSON = cJSON_GetObjectItemCaseSensitive(root, "showButtonPressed");
//        if (cJSON_IsBool(showButtonPressedJSON)) {
//            cJSON_bool showButtonPressed = cJSON_IsTrue(showButtonPressedJSON);
//            printf("showButtonPressed: %s\n", showButtonPressed ? "true" : "false");
//            // Aquí puedes realizar acciones basadas en el valor de showButtonPressed
//        } else {
//            printf("showButtonPressed is not a boolean\n");
//        }

//        cJSON_Delete(root);
//    }

//    // Liberar memoria
//    free(data);

	MSGQUEUE_USUARIOS_t users;
	
  switch (env[0]) {
    // Analyze a 'c' script line starting position 2
      
     case 'b':
      // LED control from 'led.cgi'
      if (env[2] == 'c') {
        // Select Control
        len = (uint32_t)sprintf (buf, &env[4], consumo ?     ""     : "selected",
                                               consumo ? "selected" :    ""     );
        break;
      }
     break;
				
    case 'l':
      // Consume CheckBox from consumo.cgi
      id = env[2] - '0';
      if (id > 0) {
        id = 0;
      }
      id = (uint8_t)(1U << id);
      len = (uint32_t)sprintf (buf, &env[4], (P2 & id) ? "checked" : "");
      break;
		
    case 'c':
      // Consume CheckBox from consumo.cgi
      id = env[2] - '0';
      if (id > 0) {
        id = 0;
      }
      id = (uint8_t)(1U << id);
      len = (uint32_t)sprintf (buf, &env[4], (P2 & id) ? "checked" : "");
      break;
      
    case 'a':
      // Consume CheckBox from consumo.cgi
      id = env[2] - '0';
      if (id > 0) {
        id = 0;
      }
      id = (uint8_t)(1U << id);
      len = (uint32_t)sprintf (buf, &env[4], (P2 & id) ? "checked" : "");
      break;

			
    case 'd':
      // Consume CheckBox from consumo.cgi
      id = env[2] - '0';
      if (id > 0) {
        id = 0;
      }
      id = (uint8_t)(1U << id);
      len = (uint32_t)sprintf (buf, &env[4], (P2 & id) ? "checked" : "");
      break;
      
    case 'e':
      // Consume CheckBox from consumo.cgi
      id = env[2] - '0';
      if (id > 0) {
        id = 0;
      }
      id = (uint8_t)(1U << id);
      len = (uint32_t)sprintf (buf, &env[4], (P2 & id) ? "checked" : "");
      break;
		case 'x':
      // control CheckBox from control.cgi
      id = env[2] - '0';
      if (id > 1) {
        id = 0;
      }
      id = (uint8_t)(1U << id);
      len = (uint32_t)sprintf (buf, &env[4], (P2 & id) ? "checked" : "");
      break;


    case 'f':
      // Usuarios Module control from 'usuarios.cgi'
      switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[4], add_text[0]);
          break;
        case '2':
          len = (uint32_t)sprintf (buf, &env[4], add_text[1]);
          break;
        case '3':
          len = (uint32_t)sprintf (buf, &env[4], add_text[2]);
          break;
      }
      break;
			
		case 'm':
      //Salida Module control from 'salida.cgi'
      switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[4], add_text[0]);
          break;
      }
      break;
			
		case 'g':
      //Salida Module control from 'salida.cgi'
      switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[4], add_text[0]);
          break;
      }
      break;

//    case 'g':
//      // AD Input from 'ad.cgi'
//      switch (env[2]) {
//        case '1':
//          adv = AD_in (0);
//          len = (uint32_t)sprintf (buf, &env[4], adv);
//          break;
//        case '2':
//          len = (uint32_t)sprintf (buf, &env[4], (double)((float)adv*3.3f)/4096);
//          break;
//        case '3':
//          adv = (adv * 100) / 4096;
//          len = (uint32_t)sprintf (buf, &env[4], adv);
//          break;
//      }
//      break;
    case 'j':
      // AD Input from 'ad.cgi'
      switch (env[2]) {
        case '1':
          len = (uint32_t)sprintf (buf, &env[4],rtc_date_time.rtc[0]);
          break;
        case '2':
          len = (uint32_t)sprintf (buf, &env[4], rtc_date_time.rtc[1]);
          break;
      }
      break;
			
    case 'h':
      // AD Input from 'ad.cgi'
		  //osMessageQueueGet(mid_MsgQueueUsers, &users, NULL, osWaitForever);
			reinitUsers();
			osThreadFlagsSet (TID_Usuarios, 0x40);
			osMessageQueueGet(mid_MsgQueueUsers, &users, NULL, 5000);
      switch (env[2]) {
        case '1':
					memcpy(user1, users.users[0], sizeof(user1));
          //sprintf(user1, "%-20s", users.users[0]);
          len = (uint32_t)sprintf (buf, &env[4],user1);
          break;
        case '2':
          memcpy(user2, users.users[1], sizeof(user2));
          len = (uint32_t)sprintf (buf, &env[4], user2);
          break;
        case '3':
          memcpy(user3, users.users[2], sizeof(user3));
          len = (uint32_t)sprintf (buf, &env[4], user3);
          break;
        case '4':
          memcpy(user4, users.users[3], sizeof(user4));
          len = (uint32_t)sprintf (buf, &env[4], user4);
          break;
        case '5':
          memcpy(user5, users.users[4], sizeof(user5));
          len = (uint32_t)sprintf (buf, &env[4], user5);
          break;				
      }
      break;		
      

     
     case 'v':
			osThreadFlagsSet (TID_Usuarios, 0x100);
			osMessageQueueGet(mid_MsgQueueAlerts, &alerts, NULL, osWaitForever);
      switch (env[2]) {
        case '1':
          sprintf(alert1, "%-24s", alerts.fuego);
          len = (uint32_t)sprintf (buf, &env[4],alert1);
          break;
        case '2':
          sprintf(alert2, "%-20s", alerts.gas);
          len = (uint32_t)sprintf (buf, &env[4], alert2);
          break;
        case '3':
          sprintf(alert3, "%-20s",  alerts.piezo);
          len = (uint32_t)sprintf (buf, &env[4], alert3);
          break;			
      }
     break;		
			
		case 'i':
      // AD Input from 'ad.cgi'
//      switch (env[2]) {
//        case '1':
//          
          len = (uint32_t)sprintf (buf, &env[1],rtc_date_time.rtc[0]);
         // break;
//        case '2':
//          HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);
//          sprintf(date, "%.2d-%.2d-%.2d",  sdatestructureget.Month, sdatestructureget.Date, 2000 + sdatestructureget.Year);
//          len = (uint32_t)sprintf (buf, &env[1], date);
//          break;
//      }
      break;

//    case 'x':
//      // AD Input from 'ad.cgx'
//      adv = AD_in (0);
//      len = (uint32_t)sprintf (buf, &env[1], adv);
//      break;
    
    case 'z':
//      strcpy(delete_text[0], "lorena1v2");
//    sprintf (users.user1, "%-20s", delete_text[0]);
      //osMessageQueueGet(mid_MsgQueueUsers, &users, NULL, osWaitForever);
//      switch (env[2]) {
//        case '1':
          //sprintf(user1, "%s",users.user1);
          len = (uint32_t)sprintf (buf, &env[1],"hola");
//          break;
//        case '2':
//          sprintf(user2, "%-20s", "prueba2");
//          len = (uint32_t)sprintf (buf, &env[1], user2);
//          break;
//        case '3':
//          sprintf(user3, "%-20s","prueba3");
//          len = (uint32_t)sprintf (buf, &env[1], user3);
//          break;
//        case '4':
//          sprintf(user4, "%-20s", "prueba4");
//          len = (uint32_t)sprintf (buf, &env[1], user4);
//          break;
//        case '5':
//          sprintf(user5, "%-20s",  "prueba5");
//          len = (uint32_t)sprintf (buf, &env[1], user5);
//          break;		
      break;

    case 'y':
      // Button state from 'button.cgx'
        len = (uint32_t)sprintf (buf, "<checkbox><id>button%c</id><on>%s</on></checkbox>",
        env[1], (get_button () & (1 << (env[1]-'0'))) ? "true" : "false");
      break;

  }
  return (len);
}

static void reinitUsers (void) {
	memset(users.users[0], '\0', sizeof(users.users));
	memset(users.users[1], '\0', sizeof(users.users));
	memset(users.users[2], '\0', sizeof(users.users));
	memset(users.users[3], '\0', sizeof(users.users));
	memset(users.users[4], '\0', sizeof(users.users));
		memset(user1, '\0', sizeof(user1));
		memset(user2, '\0', sizeof(user2));
		memset(user3, '\0', sizeof(user3));
		memset(user4, '\0', sizeof(user4));
		memset(user5, '\0', sizeof(user5));
}

#if      defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma  clang diagnostic pop
#endif
