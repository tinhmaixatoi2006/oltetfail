/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/main.c
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */
//#define HC05_INIT
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "PWM.h"
#include "GPIO.h"
#include "RCC.h"
//#include "ADC.h"
#include "DMA.h"
#include "NVIC.h"
#include "I2C.h"
#include "MadgwickAHRS.h"
#include "lsm303dlhc_driver.h"
#include "l3g4200d_driver.h"


#include "USART.h"

#include <stdio.h>

extern __IO uint16_t ADCValue[2];
volatile uint16_t eredmeny,kezdet, vege;
#define TASK_LED_PRIORITY ( tskIDLE_PRIORITY + 1  )
#define TASK_PWMSET_PRIORITY ( tskIDLE_PRIORITY  + 1 )
#define TASK_BTCOMM_PRIORITY ( tskIDLE_PRIORITY  + 1 )
#define TASK_ADCREAD_PRIORITY ( tskIDLE_PRIORITY + 2 )
#define TASK_ADCSTART_PRIORITY ( tskIDLE_PRIORITY + 1 )
#define TASK_INIT_PRIORITY (tskIDLE_PRIORITY + 1)
#define TASK_SENSORREAD_PRIORITY (tskIDLE_PRIORITY + 3)
//LED villogtat�, fut-e az oprendszer
static void prvLEDTask (void *pvParameters);
//PWM be�ll�t� taszk, egyel�re mind a n�gy motorra ugyanazt
static void prvPWMSetTask (void* pvParameters);
//Azok az inicializ�l�sok futnak itt amihez kellenek m�r az oprendszer szolg�ltat�sai:)
static void prvInitTask (void* pvParameters);
// BT kommunik�ci� teszt
static void prvBTCommTask (void* pvParameters);
//itt k�pne kiolvasni a szenzorokat
static void prvSensorReadTask (void* pvParameters);

xSemaphoreHandle xADCSemaphore = NULL;
//Ez itt nem fog kelleni, �t kell �ll�tani az �j bluetooth initj�re
#ifdef HC05_INIT
const portCHAR init[]="AT+UART=38400,0,0\r\n";
#endif

int main(void)
{
	//uint16_t kezdet, vege;
	vSemaphoreCreateBinary(xADCSemaphore);
	RCC_Config();

	IO_Config();
	UART_Config();
	PWM_Config();
	DMA_Config();
	I2C_Config();
	NVIC_Config();

	DebugTimerInit();
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	xTaskCreate(prvInitTask,(signed char*)"INIT", configMINIMAL_STACK_SIZE,NULL,TASK_INIT_PRIORITY,NULL);

	vTaskStartScheduler();
  while (1)
  {



  }
}
static void prvInitTask(void* pvParameters)
{
#ifdef HC05_INIT
	uint8_t i;
	for(i=0;init[i]!='\n';i++)
	{
		xQueueSend(TransmitQueue,init+i,( portTickType )0);
	}

	xQueueSend(TransmitQueue,init+i,( portTickType )0);

	UARTStartSend();
#endif
	//inicializ�l�s
	initSensorACC();
	initSensorGyro();

	//taszk ind�t�s
	xTaskCreate(prvLEDTask,(signed char*)"LED", configMINIMAL_STACK_SIZE,NULL,TASK_LED_PRIORITY,NULL);
	xTaskCreate(prvBTCommTask,(signed char*)"BT Comm", configMINIMAL_STACK_SIZE,NULL,TASK_BTCOMM_PRIORITY,NULL);
	xTaskCreate(prvPWMSetTask,(signed char*)"PWM Set", configMINIMAL_STACK_SIZE,NULL,TASK_PWMSET_PRIORITY,NULL);
	//xTaskCreate(prvSensorReadTask,(signed char*)"Sensor Read", configMINIMAL_STACK_SIZE,NULL,TASK_SENSORREAD_PRIORITY,NULL);

	vTaskDelete(NULL);
	while(1)
	{

	}

}

uint8_t recvTemp[256];
static void prvBTCommTask (void* pvParameters)
{
	portTickType xLastWakeTime;
	uint8_t HCI_Read_Local_Name[3];
	uint8_t recvPos = 0;

	xLastWakeTime=xTaskGetTickCount();

	// Wait for BT to go shutdown
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
	vTaskDelayUntil(&xLastWakeTime,1000 * portTICK_RATE_MS);

	// BT is dead, lets boot
	GPIO_SetBits(GPIOA, GPIO_Pin_4);

	// Create init packets
	HCI_Read_Local_Name[0] = 0x00;
	HCI_Read_Local_Name[1] = 0x14;
	HCI_Read_Local_Name[2] = 0x00;

	vTaskDelayUntil(&xLastWakeTime,500 * portTICK_RATE_MS);
	UART_StartSend(HCI_Read_Local_Name, 0, 3);

	for(;;)
	{
		recvPos += UART_TryReceive(recvTemp, recvPos, -1, portMAX_DELAY);
		vTaskDelayUntil(&xLastWakeTime,100 * portTICK_RATE_MS);
	}
}

//debug gaz valtozo, bluetoothon keresztul valtoztathato
uint8_t throttle=0;
static void prvPWMSetTask (void* pvParameters)
{
	static uint8_t counter=0;
	static uint8_t gaz=1;
	portTickType xLastWakeTime;
	xLastWakeTime=xTaskGetTickCount();
	uint8_t percent=0;
	while (1)
	{
		percent=0;
		if (counter<60) counter++;
		else if ((counter<69
				+6))
		{

			percent=/*0;//*/gaz*10;
			gaz+=1;
			PWM_SetDutyCycle(percent);
			counter++;
		}
		else if (counter<90 )
			{
			counter++;
			}
		//gege szerint ez igy jo lesz
		else PWM_SetDutyCycle(0);

		//bluetooth
/*
		switch (percent/10)
		{
		case 0 :
			xQueueSend(TransmitQueue,'0',( portTickType )0);
			break;
		case 1 :
			xQueueSend(TransmitQueue,'1',( portTickType )0);
			break;
		case 2 :
			xQueueSend(TransmitQueue,'2',( portTickType )0);
			break;
		case 3 :
			xQueueSend(TransmitQueue,'3',( portTickType )0);
			break;
		case 4 :
			xQueueSend(TransmitQueue,'4',( portTickType )0);
			break;
		case 5 :
			xQueueSend(TransmitQueue,'5',( portTickType )0);
			break;
		case 6 :
			xQueueSend(TransmitQueue,'6',( portTickType )0);
			break;
		case 7 :
			xQueueSend(TransmitQueue,'7',( portTickType )0);
			break;
		case 8 :
			xQueueSend(TransmitQueue,'8',( portTickType )0);
			break;
		case 9 :
			xQueueSend(TransmitQueue,'9',( portTickType )0);
			break;
		case 10 :
			xQueueSend(TransmitQueue,'1',( portTickType )0);
			xQueueSend(TransmitQueue,'0',( portTickType )0);
			break;

		}
		xQueueSend(TransmitQueue,' ',( portTickType )0);
		UARTStartSend();
*/
		vTaskDelayUntil(&xLastWakeTime,1000 * portTICK_RATE_MS);
	}
}

static void prvLEDTask (void* pvParameters)
{
	const portCHAR teszt[]="Hello World!\r\n";
	uint8_t i;
	portTickType xLastWakeTime;
	const portTickType xFrequency = 500;
	xLastWakeTime=xTaskGetTickCount();
	for( ;; )
	{
		char c;
		if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_1))  //toggle led
		{
			GPIO_ResetBits(GPIOA, GPIO_Pin_1); //set to zero
			GPIO_ResetBits(GPIOA, GPIO_Pin_2); //set to zero
		}
		else
		{
			GPIO_SetBits(GPIOA,GPIO_Pin_1); //set to one
			GPIO_SetBits(GPIOA,GPIO_Pin_2); //set to one
		}

		c = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11);

		vTaskDelayUntil(&xLastWakeTime,xFrequency * portTICK_RATE_MS);
	}
}
//szenzor kiolvasas
static void prvSensorReadTask (void* pvParameters)
{
	portTickType xLastWakeTime;
	static uint8_t data[6];
	static uint8_t newData;
	AccAxesRaw_t AccAxes;
	MagAxesRaw_t MagAxes;
	AxesRaw_t GyroAxes;
	uint8_t status;
	xLastWakeTime=xTaskGetTickCount();
	while(1)
	{
		//400Hz-el jon az adat a gyorsulas merobol es a gyrobol.
		GetSatusReg(&status);
		if (status&0b0001000) //new data received
		{
			newData|=1;
		//	GetAccAxesRaw(&AccAxes);
			readACC(data);

			//allithato endiannes miatt olvashat� �gy is
			AccAxes.AXIS_X=((int16_t*)data)[0];
			AccAxes.AXIS_Y=((int16_t*)data)[1];
			AccAxes.AXIS_Z=((int16_t*)data)[2];

		}

		//az iranytubol csak 30Hz
		//ReadStatusM(&status);
		if (status&0x01)
		{
			readMag(data);
			MagAxes.AXIS_X=((int16_t)data[0])<<8;
			MagAxes.AXIS_X|=data[1];
			MagAxes.AXIS_Y=((int16_t)data[2])<<8;
			MagAxes.AXIS_Y|=data[3];
			MagAxes.AXIS_Z=((int16_t)data[4])<<8;
			MagAxes.AXIS_Z|=data[5];
		}
		L3G4200D_GetSatusReg(&status);
		if (status&0b00001000)
		{
			ReadGyro(data);
			newData|=2;
			GyroAxes.AXIS_X=((int16_t*)data)[0];
			GyroAxes.AXIS_Y=((int16_t*)data)[1];
			GyroAxes.AXIS_Z=((int16_t*)data)[2];
		}
		if (newData==3)
		{
			newData=0;
		}
		vTaskDelayUntil(&xLastWakeTime,100);
		float x=2;
		x=invSqrt(x);
		//x?=0.707168
	}

}

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
  GPIO_SetBits(GPIOC,GPIO_Pin_12);
  for( ;; );
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
