#include "rtc.h"

//hilo princiapal.c
extern osThreadId_t tid_Thread_principal;

//funciones internas
static void RTC_Set_Time_SNTP(void);
static void get_sntp_time (void);
static void sntp_time_callback (uint32_t time, uint32_t seconds_fraction);
static void RTC_set_Time(uint8_t day, uint8_t month, uint8_t year, uint8_t hora, uint8_t minutos, uint8_t segundos);
static void RTC_getTime_Date(void);
static void Init_alarma(void);

//estructuras
static RTC_HandleTypeDef hrtc;
static RTC_DateTypeDef fech;
static RTC_TimeTypeDef hor;
static RTC_AlarmTypeDef alarma;

//variables locales
volatile bool alarm_enabled = true;
struct tm horaSNTP;

//variable global
MSGQUEUE_RTC_t rtc_date_time;

//timer
static osTimerId_t tim_id_3min;	//timer sincronizacion cada 3 min
static void Timer_Callback_3min (void);

//funcion que inicializa el rtc
void RTC_init(void) {
  /*##-1- Configure the RTC peripheral #######################################*/
  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follows:
      - Hour Format    = Format 24
      - Asynch Prediv  = Value according to source clock
      - Synch Prediv   = Value according to source clock
      - OutPut         = Output Disable
      - OutPutPolarity = High Polarity
      - OutPutType     = Open Drain */ 
	//__HAL_RCC_RTC_ENABLE();
  hrtc.Instance = RTC; 
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  hrtc.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  __HAL_RTC_RESET_HANDLE_STATE(&hrtc);
  if (HAL_RTC_Init(&hrtc) != HAL_OK) {
    /* Initialization Error */
    
  }
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
	
	tim_id_3min = osTimerNew((osTimerFunc_t)&Timer_Callback_3min, osTimerPeriodic, NULL, NULL);
	get_sntp_time();
	Init_alarma();
	osTimerStart(tim_id_3min, 180000);
}

//funcion que establece el rtc a una hora determinada
void RTC_set_Time(uint8_t day, uint8_t month, uint8_t year, uint8_t hora, uint8_t minutos, uint8_t segundos) {   //pone la hora a enero las 20:20:20
	fech.Year=year;
	fech.Month=month;
	fech.Date=day;
	HAL_RTC_SetDate(&hrtc,&fech,RTC_FORMAT_BIN);
	
	hor.Hours=hora;
	hor.Minutes=minutos;
  hor.Seconds=segundos;
  hor.TimeFormat=RTC_HOURFORMAT_24;
	hor.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  hor.StoreOperation = RTC_STOREOPERATION_RESET;
	HAL_RTC_SetTime(&hrtc,&hor,RTC_FORMAT_BIN);
}

void RTC_getTime_Date(void) {   //recibe la fecha y la hora y lo escribe por las lineas del lcd
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;
  HAL_RTC_GetTime(&hrtc,&stimestructureget,RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc,&sdatestructureget,RTC_FORMAT_BIN);
  
  sprintf(rtc_date_time.rtc[0],"%.2d:%.2d:%.2d",stimestructureget.Hours, stimestructureget.Minutes,stimestructureget.Seconds);
  sprintf(rtc_date_time.rtc[1],"%.2d/%.2d/%.2d",sdatestructureget.Date, sdatestructureget.Month,2000+sdatestructureget.Year);
  
	osThreadFlagsSet(tid_Thread_principal, FLAG_RTC);
}

//SNTP
static void RTC_Set_Time_SNTP(void) {	//pone la hora del servidor de sntp
	fech.Year=horaSNTP.tm_year+1900-2000;	//espera la hora desde 1900, pero al parecer esperamos recibirla en formato a partir de los 2000
	fech.Month=horaSNTP.tm_mon+1;
	fech.Date=horaSNTP.tm_mday;
	HAL_RTC_SetDate(&hrtc,&fech,RTC_FORMAT_BIN);
	hor.Hours=horaSNTP.tm_hour+1;
	hor.Minutes=horaSNTP.tm_min;
  hor.Seconds=horaSNTP.tm_sec;
  hor.TimeFormat=RTC_HOURFORMAT_24;
	hor.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  hor.StoreOperation = RTC_STOREOPERATION_RESET;
	HAL_RTC_SetTime(&hrtc,&hor,RTC_FORMAT_BIN);
}

static void get_sntp_time (void) {
	netSNTPc_GetTime (NULL, sntp_time_callback);
}

static void sntp_time_callback (uint32_t time, uint32_t seconds_fraction) {
	uint32_t hora;
	if (time==0) {
		//ERROR
		RTC_set_Time(0,0,0,0,0,0);
	} else {
		hora = time;
		horaSNTP = *localtime(&hora);
		RTC_Set_Time_SNTP();
	}
}

static void Timer_Callback_3min (void) {	//cada 3 min se sincroniza con sntp
	 get_sntp_time();								
}


void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc) {
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  /*##-1- Enables the PWR Clock and Enables access to the backup domain ###################################*/
  /* To change the source clock of the RTC feature (LSE, LSI), You have to:
     - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
     - Enable write access using HAL_PWR_EnableBkUpAccess() function before to 
       configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and 
       __HAL_RCC_BACKUPRESET_RELEASE().
     - Configure the needed RTc clock source */
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  
  //##-2- Configure LSE as RTC clock source ###################################/
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { 
    //Error_Handler();
  }
  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) { 
    //Error_Handler();
  }
  
  //##-3- Enable RTC peripheral Clocks #######################################/
  /* Enable RTC Clock */
  __HAL_RCC_RTC_ENABLE();
}

//alarma la usamos para actualizar el lcd
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
	//puedes poner un flag o variable global, pero timers no
	if (alarm_enabled) {
		RTC_getTime_Date();
	}
	//osThreadFlagsSet (tid_ThAlarm, FLAG_ALARMA);
}

void RTC_Alarm_IRQHandler(void) {
  HAL_RTC_AlarmIRQHandler(&hrtc);
}

static void Init_alarma (void){
//	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
//	alarma.Alarm = RTC_ALARM_A;
//	alarma.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
//	alarma.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
//	alarma.AlarmTime.StoreOperation = RTC_STOREOPERATION_SET;

//	alarma.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS|RTC_ALARMMASK_MINUTES;
//	alarma.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
//	alarma.AlarmDateWeekDay = 1;
//	alarma.AlarmTime.Seconds = seg_alarm;
//	
//	HAL_RTC_SetAlarm_IT(&hrtc, &alarma, RTC_FORMAT_BIN);
	
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
	alarma.Alarm = RTC_ALARM_A;
	alarma.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
	alarma.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	alarma.AlarmTime.StoreOperation = RTC_STOREOPERATION_SET;
	alarma.AlarmSubSecondMask = 0;
	
	alarma.AlarmMask = RTC_ALARMMASK_ALL;
	alarma.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	alarma.AlarmDateWeekDay = 1;
	
	HAL_RTC_SetAlarm_IT(&hrtc, &alarma, RTC_FORMAT_BIN);
}

void stopAlarma (void) {
	alarm_enabled = false;
}

void restartAlarm (void) {
	alarm_enabled = true;
}
