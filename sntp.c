#include "sntp.h"

extern RTC_HandleTypeDef RtcHandle;
//Variables:
uint32_t s = 0;
struct tm tiempo_SNTP;
//#define SNTP_CLIENT_NTP_SERVER  "217.79.179.106"
static void time_callback (uint32_t seconds, uint32_t seconds_fraction) ;
const NET_ADDR4 ntp_server = {NET_ADDR_IP4,0,150,214,94,5};

void get_time (void) {
  if (netSNTPc_GetTime ((NET_ADDR*) &ntp_server, time_callback) == netOK) {
    //printf ("SNTP request sent.\n");
	//	osDelay(3000);
  } else {
  // printf ("SNTP not ready or bad parameters.\n");
  }
}

static void time_callback (uint32_t seconds, uint32_t seconds_fraction) {
	s = seconds;
	RTC_DateTypeDef SNTPdatestructure;
  RTC_TimeTypeDef SNTPtimestructure;
  if (s == 0) {
    printf ("Server not responding or bad response.\n");
  } else {
    tiempo_SNTP = *localtime(&s);
		SNTPdatestructure.Year = tiempo_SNTP.tm_year-100;
		SNTPdatestructure.WeekDay = tiempo_SNTP.tm_wday;
		SNTPdatestructure.Month = tiempo_SNTP.tm_mon+1;
		SNTPdatestructure.Date = tiempo_SNTP.tm_mday;
		HAL_RTC_SetDate(&RtcHandle,&SNTPdatestructure,RTC_FORMAT_BIN);
  
		SNTPtimestructure.Hours = tiempo_SNTP.tm_hour+3;
		SNTPtimestructure.Minutes = tiempo_SNTP.tm_min;
		SNTPtimestructure.Seconds = tiempo_SNTP.tm_sec;
	  HAL_RTC_SetTime(&RtcHandle, &SNTPtimestructure, RTC_FORMAT_BIN);
  }
}

