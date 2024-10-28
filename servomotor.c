#include "servomotor.h"

/*----------------------------------------------------------------------------
 *      Thread 'Servomotor.c': Sample thread
 *---------------------------------------------------------------------------*/
 
 /*
 Datos tecnicos:
 - El servomor funciona con una frecuencia de 50Hz
 - Para controlarlo, se va cambiando el periodo del pulso
Izquierda - 2ms (180º) Centro - 1.5ms (90º) Izquierda 1ms (0º)
 */
 
#define left_duty             		80
#define right_duty              	40
#define stop_duty									60
 
 //thread stuff
static osThreadId_t TID_Servo;                        // thread id
static void Thread_servo (void *argument);                   // thread function

//PWM
static TIM_HandleTypeDef htim1;  //estructura tim1 PWM mode
static TIM_OC_InitTypeDef sConfigOC; //config tim1 PWM mode
static GPIO_InitTypeDef GPIO_InitStruct; //estructura config pin salida TIM1

//variables
static uint16_t dutyCycle; //ciclo de trabajo

//timer
static osTimerId_t id_timer;
static void timer_callback (void *arg);
static void init_timer(void);
static void start_timer(void);

//internal functions
static void initTim1PWM(void);
static void reinitTIM1PWM (uint8_t duty);
static void initPinPE9(void);
static void stop_move(void);
static void move_left(void);
static void move_right(void);
static void open_door (void);
static void close_door(void);

osThreadId_t Init_ThServo (void) {
  TID_Servo = osThreadNew(Thread_servo, NULL, NULL);
  if (TID_Servo == NULL) {
    return(NULL);
  }
  return(TID_Servo);
}
 
static void Thread_servo (void *argument) {
  static uint32_t flags;
  initTim1PWM();
	initPinPE9();
  init_timer();
  while (1) {
    flags = osThreadFlagsWait(CLOSE_FLAG | OPEN_FLAG | STOP_FLAG, osFlagsWaitAny, osWaitForever);
    if (flags == CLOSE_FLAG) {
				close_door();
    } else if (flags == OPEN_FLAG) {
				open_door();
    } else if (flags == STOP_FLAG) {
				stop_move();
		}
  }
}

/*
Funcion que inicializa el timer 1 para PWM
APB2 = 168MHz - SystemCoreClock
*/
static void initTim1PWM(void) { //funcion config tim1 PWM
  __HAL_RCC_TIM1_CLK_ENABLE();
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 4199;    //168MHz/4200=40000
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 799;         //40000/800 = 50Hz
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_PWM_Init(&htim1);
  
  //Como el period es 80, pulse valdrá entre 0 y 800
  //50Hz = 20ms
  //20ms/800 = 0.025ms cada numero de pulse
  //Para conseguir: 1ms, pulse tendra que valer 1/0.025 = 40				- izquierda
  //                1.5 ms, pulse tendra que valer 1.5/0.025=60			- stop
  //                2 ms, pulse tendra que valer 2/0.025=80					- derecha
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = stop_duty;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  dutyCycle = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_1);
}

static void reinitTIM1PWM (uint8_t duty) {
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	sConfigOC.Pulse = duty;			//quizas es duty-1
	HAL_TIM_PWM_Init(&htim1);
	HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  dutyCycle = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_1);
}
static void initPinPE9(void){ //Pin salida PE9 TIM1
  __HAL_RCC_GPIOE_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

static void stop_move(void) {
	reinitTIM1PWM(stop_duty);
}

static void move_left(void) {
	reinitTIM1PWM(left_duty);	
}

static void move_right(void) {
	reinitTIM1PWM(right_duty);
}

static void open_door (void) {
	//moves servo from 0º to 180º
	move_left();
	start_timer();
}

static void close_door(void) {
	//move servo from 180º to 0º
	move_right();
	start_timer();
}

//permite parar el servo
//usaremos velocidad constante del servo, asi que con un timer pararemos de moverlo
static void start_timer(void) {
  osTimerStart(id_timer, 333);
}

static void init_timer(void) {
  id_timer = osTimerNew(timer_callback, osTimerOnce, NULL, NULL);
}

static void timer_callback (void *arg) {
	osThreadFlagsSet(TID_Servo, STOP_FLAG);
}