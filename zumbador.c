#include "zumbador.h"


//hilo
osThreadId_t tid_Th_zumbador;

//variables lcoales
static TIM_HandleTypeDef tim2;
static osTimerId_t tim_id_PWM;  
static uint8_t cont;

//funciones locales
static void Init_Thread_Zumbador (void);
static void Thread_Zumbador (void *argument);                   
static void initTimer_PWM(void);
static void initZumbador(void);
static void Timer_Callback (void const *arg);
static void Init_Timer_ZUMB (void);                          
static void Re_Init_Timer_alarma(void); 
static void Re_Init_Timer_alerta(void);

//hilo
static void Init_Thread_Zumbador (void) {
  tid_Th_zumbador= osThreadNew(Thread_Zumbador, NULL, NULL);
}


static void Thread_Zumbador (void *argument) {
  uint32_t flags = 0;
  Init_Timer_ZUMB();
  while (1) {
		flags = osThreadFlagsWait(FLAG_ALARMA | FLAG_ALERTA | FLAG_ZSTOP ,osFlagsWaitAny,osWaitForever);
    if(flags & FLAG_ALARMA){
			Re_Init_Timer_alarma();	
		}
		if(flags & FLAG_ALERTA){	
			Re_Init_Timer_alerta();
		}
		if(flags & FLAG_ZSTOP){
			osTimerStop(tim_id_PWM);
		}
		osThreadYield();                            
  }
}


static void initTimer_PWM(void){
	TIM_OC_InitTypeDef TIM_Channel_InitStruct;
	
	__HAL_RCC_TIM2_CLK_ENABLE();

	tim2.Instance=TIM2;
	tim2.Init.Prescaler=99;
	tim2.Init.CounterMode=TIM_COUNTERMODE_UP;
	tim2.Init.Period= 419; //2kHz
	tim2.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_PWM_Init(&tim2);
	
	TIM_Channel_InitStruct.OCMode = TIM_OCMODE_PWM1;
	TIM_Channel_InitStruct.Pulse =209;
	TIM_Channel_InitStruct.OCPolarity=TIM_OCPOLARITY_HIGH;
	TIM_Channel_InitStruct.OCFastMode= TIM_OCFAST_DISABLE;

	HAL_TIM_PWM_ConfigChannel(&tim2,&TIM_Channel_InitStruct,TIM_CHANNEL_4);
}

static void initZumbador(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Alternate=GPIO_AF1_TIM2;
	GPIO_InitStruct.Pull=GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
	
}

//TIMER DEL ZUMBADOR
//We turn start and stop the signal according to the frquency set(dependes on wether it is ALARMA or ALERTA)
static void Timer_Callback (void const *arg) {
	  if(cont==1){
			HAL_TIM_PWM_Stop(&tim2, TIM_CHANNEL_4);
			cont=0;
		}else{
			HAL_TIM_PWM_Start(&tim2,TIM_CHANNEL_4);
			cont=1;
		}
}
 
static void Init_Timer_ZUMB (void) {
  tim_id_PWM = osTimerNew((osTimerFunc_t)&Timer_Callback, osTimerPeriodic, NULL, NULL);
}

static void Re_Init_Timer_alarma(void){
	osTimerStart(tim_id_PWM, 400);
  HAL_TIM_PWM_Start(&tim2,TIM_CHANNEL_4);
  cont=1;
}

static void Re_Init_Timer_alerta(void){
	osTimerStart(tim_id_PWM,5);
  HAL_TIM_PWM_Start(&tim2,TIM_CHANNEL_4);
	cont=1;
	//flag=osThreadFlagsWait(ZSTOP, osFlagsWaitAny, osWaitForever);
}
osThreadId_t getModPWMThreadID(void){
	return tid_Th_zumbador;
}

/*--------------------MÓDULO---------------------*/
void initModZumbador(void){
	initTimer_PWM();
	initZumbador();
	Init_Thread_Zumbador();
}

