#ifndef __RTC_H
#define __RTC_H

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include "cmsis_os2.h"
#include <time.h>
#include "rl_net.h"

/* Defines related to Clock configuration */
#define RTC_ASYNCH_PREDIV  127   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   255 /* LSE as RTC clock */
#define ENCENDERTIMALARM 0x00440

#define FLAG_RTC			0x01

typedef struct {                               
  char rtc[2][20+1];
} MSGQUEUE_RTC_t;

void RTC_init(void);
osMessageQueueId_t getRTCQueue (void);

#endif
