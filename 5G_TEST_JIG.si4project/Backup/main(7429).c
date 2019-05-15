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

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <stdio.h>
#include	"rabm.h"


extern int fw_version; 

 int main(void)
{

	RCC_Configuration();
	NVIC_Configuration();
	
	
	GPIO_Configuration();
	//GPIO_ResetBits( GPIO_DC_REF_ENABLE, DC_REF_ENABLE);	
	//Delay_ms(100);

	//while(1) ;
	
	I2C_Configuration();



	//ADC_Configuration();	
	SPI_Configuration();
	Timer4_Init();
	UART_Configuration();	

	RTC_Configuration1();

	RCC_ClearFlag();
	EXTI1_Config();
	DMA_Configuration();
	ADC_Configuration_scanmode();
	Reset_W5200();	// WIZ_RESET not available

	//printf("\n\n====================================\n");
	printf(" TH-5G PSU \n");
  	printf("YooHa ELEC. @ %s %s\n\r", __DATE__, __TIME__);
	printf("FW %08X \n\r", fw_version);
  	printf("====================================\n");

	rabm_main();

}






