


/**
  ******************************************************************************
  * @file    cinfig.c 
  * @author  ksyoo
  * @version
  * @date    
  * @brief   i/o configuration
  ******************************************************************************

  */  

/* Includes ------------------------------------------------------------------*/
#include 	"stm32f10x.h"
#include 	<stdio.h>
#include 	<stdarg.h>
#include	"util.h"
#include	 "stm32f10x_exti.h"
#include	 "stm32f10x_adc.h"
#include	 "stm32f10x_dma.h"

#include "io.h"
#include	"rabm.h"



void	DAC_Configuration();

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
 __IO uint32_t TimeDisplay = 0;

uint8_t fram_addr = 0xa0;
uint8_t dac_addr[2] = {0x1C, 0x1A};
uint8_t rect_restart;

#if 1
 uint8_t TxBuffer1[TxBufferSize1] ;
//uint8_t TxBuffer2[TxBufferSize2] ;
 uint8_t RxBuffer1[RxBufferSize1];
// uint8_t RxBuffer2[RxBufferSize2];
 __IO uint8_t TxCounter1 = 0x00;
 __IO uint8_t TxCounter2 = 0x00;
 __IO uint8_t RxCounter1 = 0x00; 
 __IO uint8_t RxCounter2 = 0x00;
 
#endif
ErrorStatus HSEStartUpStatus;

extern int	reset_type;
extern uint16_t adc_ready;

extern unsigned int	rx_head, rx_tail, tx_head, tx_tail;

extern float Vref;
extern int	eeprom_OK;


USART_InitTypeDef USART_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;



/* Private function prototypes -----------------------------------------------*/
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/* Private functions ---------------------------------------------------------*/



uint16_t	adc_dr[10];
uint32_t    dst, src;
ADC_InitTypeDef           ADC_InitStructure;
DMA_InitTypeDef           DMA_InitStructure;


void DMA_Configuration_dualmode(void)
{
	/* DMA1 Channel1 configuration ----------------------------------------------*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_dr;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 8;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	/* Enable DMA1 Channel1 ADC1 */
	//DMA_Cmd(DMA1_Channel1, ENABLE);

}


void ADC_Configuration_dualmode(void)
{
  	GPIO_InitTypeDef GPIO_InitStructure;

	init_adc_dr();
	ADC_DeInit(ADC1);

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1, ENABLE);

  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed =  0; //GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
  	GPIO_InitStructure.GPIO_Speed = 0; // GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
  	GPIO_InitStructure.GPIO_Speed =  0; //GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
  GPIO_Init(GPIOA, &GPIO_InitStructure);

	
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  //ADC_DataAlign_Right; 
	ADC_InitStructure.ADC_NbrOfChannel = 7;
	ADC_Init(ADC1, &ADC_InitStructure);
	/* ADC1 regular channels configuration */ 

	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_7Cycles5); // dcv
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_7Cycles5); // dca
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_7Cycles5);	//bca
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_7Cycles5);	// rect
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 5, ADC_SampleTime_7Cycles5);	// batt

	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 6, ADC_SampleTime_7Cycles5); // batv
	ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 7, ADC_SampleTime_7Cycles5); // pc5
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 8, ADC_SampleTime_7Cycles5); // ref
	
	ADC_Cmd(ADC1, ENABLE);

	ADC_TempSensorVrefintCmd(ENABLE);
	/* Enable ADC1 reset calibaration register */   
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));
	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));
	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	//ADC_DMACmd(ADC1, ENABLE);

	//adc_ready = 1;

}



void DMA_Configuration(void)
{
	/* DMA1 Channel1 configuration ----------------------------------------------*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_dr;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 6;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	/* Enable DMA1 Channel1 ADC1 */
	//DMA_Cmd(DMA1_Channel1, ENABLE);

}

int ADC_Read_Chs()
{
	DMA1_Channel1->CCR |= DMA_CCR1_EN;
	ADC1->CR2 |= CR2_DMA_Set;
	ADC1->CR2 |= CR2_ADON_Set;
	ADC1->CR2 |= CR2_EXTTRIG_SWSTART_Set;
	//while (!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));	// do not use
	//while ( !DMA_GetFlagStatus(DMA1_FLAG_TC1)) ;
	while ( !(DMA1->ISR & DMA1_FLAG_TC1) );
	return ADC1->DR;
}


void read_adc()
{
	static int flag = 0;
	
	if ( flag ) while ( !(DMA1->ISR & DMA1_FLAG_TC1) );

	DMA_Cmd(DMA1_Channel1, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);

	DMA1_Channel1->CCR |= DMA_CCR1_EN;
	ADC1->CR2 |= CR2_DMA_Set;
	ADC1->CR2 |= CR2_ADON_Set;
	ADC1->CR2 |= CR2_EXTTRIG_SWSTART_Set;
	//while (!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));	// do not use
	//while ( !DMA_GetFlagStatus(DMA1_FLAG_TC1)) ;

	
	// while ( !(DMA1->ISR & DMA1_FLAG_TC1) );
	flag = 1;
}


void DMA_test()
{

	DMA_Cmd(DMA1_Channel1, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);

	ADC_Read_Chs();

	printf("ADC=%u,%u %u %u %u %u\n", adc_dr[0], adc_dr[1], adc_dr[2], adc_dr[3], adc_dr[4], adc_dr[5]);

}


void init_adc_dr()
{
	int i = 0;
	
	for ( i= 0; i < 10; i++) {
		adc_dr[i] = 0;
	}

}

void ADC_Configuration_scanmode(void)
{
  	GPIO_InitTypeDef GPIO_InitStructure;

	init_adc_dr();
	ADC_DeInit(ADC1);

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1, ENABLE);

  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed =  0; //GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
  GPIO_Init(GPIOC, &GPIO_InitStructure);
#if 0
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
  	GPIO_InitStructure.GPIO_Speed = 0; // GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
  	GPIO_InitStructure.GPIO_Speed =  0; //GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
  GPIO_Init(GPIOA, &GPIO_InitStructure);

	
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  //ADC_DataAlign_Right; 
	ADC_InitStructure.ADC_NbrOfChannel = 3;
	ADC_Init(ADC1, &ADC_InitStructure);
	/* ADC1 regular channels configuration */ 

	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_28Cycles5); // dcv
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_28Cycles5); // dca
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 3, ADC_SampleTime_28Cycles5);	// batt

	
	ADC_Cmd(ADC1, ENABLE);

	ADC_TempSensorVrefintCmd(ENABLE);
	/* Enable ADC1 reset calibaration register */   
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));
	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));
	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	//ADC_DMACmd(ADC1, ENABLE);

	//adc_ready = 1;

}
	



void EXTI1_Config(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
	GPIO_InitTypeDef	 GPIO_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

  /* Enable GPIOA clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  
  /* Configure PA.01 pin as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Enable AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

  /* Connect EXTI0 Line to PA.01 pin */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);

  /* Configure EXTI1 line */
  EXTI_InitStructure.EXTI_Line = EXTI_Line1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set EXTI0 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void Timer4_Init(void)
{
  /* Enable the TIM4 Interrupt */
  NVIC_InitTypeDef NVIC_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;

  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;          
  TIM_TimeBaseStructure.TIM_Prescaler = 0; //36Mhz
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0; //appended
  
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

  /* Prescaler configuration */
  TIM_PrescalerConfig(TIM4, 49, TIM_PSCReloadMode_Immediate); //36Mhz/5 = 7.2Mhz

  /* Output Compare Timing Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable; //added
  //TIM_OCInitStructure.TIM_Channel = TIM_Channel_1;          //removed
  TIM_OCInitStructure.TIM_Pulse = 1440;  //200us
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    
  TIM_OC1Init(TIM4, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Disable);
  
  /* TIM IT enable */
  TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);

  /* TIM4 enable counter */
  TIM_Cmd(TIM4, ENABLE);
}



void	 GPIOA_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_DeInit(GPIOA);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_AFIO, ENABLE);

#if 0
	if ( reset_type & RESET_SWRESET || reset_type & RESET_IWDG ) {
		 GPIO_Write(GPIOA, 0x00|RECT_ON );
		 rect_restart = 0;
	}
	else {
		GPIO_Write(GPIOA, 0x00| SR_ON_PIN );
		rect_restart = 1;
	}
#endif	
	GPIO_Write(GPIOA, 0x00 );

#if 0
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6|GPIO_Pin_7;	// A6:ECS_RESET, A7:ECS_CS
	GPIO_InitStructure.GPIO_Pin |=  GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif

/* 
	gpio2 : rect on, 5: SR on/off, 6: ECS_RESET, 7:ECS_CS, 8: POWER LED, 11: FAIL LED, 12:RUN LED, 15: WATHC DOG RESET

*/
	GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_15;	// rect_on, rect_enl
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

#if 0
	GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_11 |GPIO_Pin_12;	//  led_run, led_fail
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif


	/* Configure USART1 Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Configure USART1 Rx as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ;  // USART1 RX, BAT_SW
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;  // AC_FAIL, RECT_ON
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	// Configure DAC output
	GPIO_InitStructure.GPIO_Speed =  GPIO_Speed_2MHz; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;	//A4:DC_REF
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);

		// Configure	ADC123_IN0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	// A0:rect_TEMP
	GPIO_InitStructure.GPIO_Speed =  GPIO_Speed_2MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}


void	GPIOB_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_DeInit(GPIOB);
	/* Enable GPIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_Write(GPIOB, 0x00);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8;	// jumper select
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;	// jumper select
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure USART3  Rx as input floating 	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	*/
	
}

void	GPIOD_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_DeInit(GPIOD);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD ,ENABLE);  // enabel APB2 clcok	


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init( GPIOD, &GPIO_InitStructure);

}

void	GPIOC_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_DeInit(GPIOC);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC| RCC_APB2Periph_AFIO,ENABLE);  // enabel APB2 clcok	

	GPIO_Write(GPIOC, ETH_SEL_PIN);


	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 ;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_12 ;		
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
		
	/* Configure PC.04 (Channel14)
	as analog input ----------------------------------------------------------*/
	// 0:dcv, 1: LDA+, 2:BCA+, 3:REC_TEMP, 4: BAT V
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed =  GPIO_Speed_2MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	#if 0
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed =  GPIO_Speed_2MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	#endif
}


void	AFIO_Configuration()
{
	u32   temp;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE);
	temp = AFIO->MAPR & AFIO_MAPR_SWJ_CFG;	
	AFIO->MAPR = temp | AFIO_MAPR_SWJ_CFG_JTAGDISABLE;	// Disable JTAG/SW-DP
}

void GPIO_Configuration(void)
{
	
	AFIO_Configuration();
	GPIOA_Configuration();
	DAC_Configuration();		// fast init
	GPIOB_Configuration();
	
	GPIOC_Configuration();
	GPIOD_Configuration();

	#if 0
	GPIO_WriteBit(GPIO_LED_POWER , LED_POWER_PIN, (BitAction) 0);
	GPIO_WriteBit(GPIO_LED_RUN , LED_RUN, (BitAction) 0);
	GPIO_WriteBit(GPIO_LED_FAIL , LED_FAIL, (BitAction) 0);
	#endif
}

u32	DAC_ReadCh(u32 ch)
{
	if ( ch == 1 ) {
		return DAC->DOR1;
	}
	else if ( ch == 2 ) {
		return DAC->DOR2;		
	}
	else if (ch == 3 || ch == 4 ) {
		return DAC_I2C_read(ch);
	}
	else sys_error(__LINE__);
        return -1;
}


void	DAC_WriteCh(u32 ch, int	value)
{

	static int pvalue = 0xffff;

	if ( pvalue == value ) return;
	
	if ( value < 1 ) value = 0;
	else if ( value > 4095 ) value = 4095;
	
	if ( ch == 1 ) {
		DAC->DHR12R1 = value;
		DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
		//DAC_write(value);
		//printf("DAC:%d\n",  value);
		//printf("DAC:%d, %d\n", DAC_read(), value);
	}
	else if ( ch == 2 ) {
		DAC->DHR12R2 = value;
		DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG2;
	}
		else if (ch == 3 || ch == 4 ) {
		DAC_I2C_write(ch, value);
	}
	else sys_error(__LINE__);
}

void	DAC_Configuration()
{
	DAC_DeInit();
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_DAC, ENABLE);

	DAC->CR |= DAC_CR_TSEL1 | DAC_CR_TEN1 | DAC_CR_EN1;	//DAC1  sw trigger and enable
	DAC->CR |= DAC_CR_TSEL2 | DAC_CR_TEN2 | DAC_CR_EN2;	//DAC2  sw trigger and enable

	DAC_WriteCh(1, 0);		// 0V, default DC_REF
	DAC_WriteCh(2, 0);	// charge current off mode : MAX
	//GPIO_ResetBits( GPIOA, GPIO_Pin_1 );
}

#if 0
void	Reset_ADC10()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//GPIO_DeInit(GPIOC);

	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC| RCC_APB2Periph_AFIO,ENABLE);  // enabel APB2 clcok	



	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	// ADC10	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOC, GPIO_Pin_10);	
	/* Configure PC.04 (Channel14)
	as analog input ----------------------------------------------------------*/
	// 0:dcv, 1: LDA+, 2:BCA+, 3:REC_TEMP
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
}
#endif

int ADC_ReadCh(int ch)
{
	ADC_InitTypeDef ADC_InitStructure;
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  //ADC_DataAlign_Right; 
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	/* ADC1 regular channels configuration */ 
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_28Cycles5);    // 28 ~ 239
	//  ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5); 
	ADC_Cmd(ADC1, ENABLE);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while (!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));
	ADC_Cmd(ADC1, DISABLE);
	return ADC_GetConversionValue(ADC1);
}


void ADC_Configuration(void)
{
	ADC_InitTypeDef ADC_InitStructure;

	ADC_DeInit(ADC1);

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1, ENABLE);
	
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  //ADC_DataAlign_Right; 
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	/* ADC1 regular channels configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_28Cycles5);    // 239Cycles
	//  ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5); 
	ADC_Cmd(ADC1, ENABLE);

	ADC_TempSensorVrefintCmd(ENABLE);
	/* Enable ADC1 reset calibaration register */   
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));
	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));
	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

#if 1
void RCC_Configuration(void)
{
	RCC_ClocksTypeDef RCC_Clocks;

  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
  {
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);

    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1);

    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);	// org = RCC_HCLK_Div2
    
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); // 72MHz / 6 = 12Mz

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* PLLCLK = 8MHz * 9 = 72 MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

    /* Enable PLL */
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }

  /* Enable GPIOA, GPIOB and AFIO clocks */
  	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1|RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD, ENABLE);
  	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
   reset_type = RCC->CSR;

#if 0   
// Enable SysTick
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency/1000);	//  1/10 = 100ms, 1/100 =  10ms, 1 / 1000 = 1ms
#endif

}
#endif

void USART1_Init(void)
{
    USART_InitTypeDef USART_InitStructure;
 
    /* USARTx configuration ------------------------------------------------------*/
    /* USARTx configured as follow:
     - BaudRate = 115200 baud  
     - Word Length = 8 Bits
     - One Stop Bit
     - No parity
     - Hardware flow control disabled (RTS and CTS signals)
     - Receive and transmit enabled
     */

	USART_DeInit(USART1);

	/* Enable GPIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 

	/* USART configuration */
    USART_InitStructure.USART_BaudRate   = 115200; //57600;	
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits   = USART_StopBits_1;
    USART_InitStructure.USART_Parity     = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl
                                         = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(USART1, USART_IT_TC, ENABLE);		
	/* Enable USART */
	USART_Cmd(USART1, ENABLE);
	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

}

#if 0
void USART3_Init(void)
{
    USART_InitTypeDef USART_InitStructure;
 
    /* USARTx configuration ------------------------------------------------------*/
    /* USARTx configured as follow:
     - BaudRate = 115200 baud  
     - Word Length = 8 Bits
     - One Stop Bit
     - No parity
     - Hardware flow control disabled (RTS and CTS signals)
     - Receive and transmit enabled
     */	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); 
	/* USART configuration */

	USART_InitStructure.USART_BaudRate   = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;
	USART_InitStructure.USART_Parity     = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(USART3, USART_IT_TC, ENABLE);		
	/* Enable USART */
	USART_Cmd(USART3, ENABLE);
	//USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

}
#endif

void	UART_Configuration()
{ 
	USART1_Init();
	//USART3_Init();
}

void SerialPutChar(uint8_t c)
{

#if 1
  USART_SendData(USART1, c);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
#else
	USART_TxChar(c);

#endif
}

void Serial_PutString(uint8_t *s)
{
  while (*s != '\0')
  {
    SerialPutChar(*s);
    s++;
  }
}



int	USART_Rx(unsigned char *ch)
{
	if ( rx_tail != rx_head ) {
		*ch = RxBuffer1[rx_tail++];
		if ( rx_tail == RxBufferSize1 ) rx_tail = 0;
		return 1;
	}
	return 0;
}


void USART_TxChar(unsigned char ch)
{
	TxBuffer1[tx_head++] = ch;
	if ( tx_head == TxBufferSize1 ) tx_head = 0;
	if (USART_GetFlagStatus(USART1, USART_FLAG_TXE) != RESET) {
		USART_SendData(USART1, TxBuffer1[tx_tail++]);		
		if(tx_tail == TxBufferSize1 ) tx_tail = 0;
	}
}

int	USART_TxBuffer_Size()
{
	if (tx_head >=  tx_tail ) return tx_head - tx_tail;
	else return tx_head + TxBufferSize1 - tx_tail;

}

void	USART_TxNData(unsigned char *buf, int size)
{

	while ( USART_TxBuffer_Size() < size ) {}
	while( size-- ) {
		TxBuffer1[tx_head++] = *buf++;
		if ( tx_head == TxBufferSize1 ) tx_head = 0;
	}

	if (USART_GetFlagStatus(USART1, USART_FLAG_TXE) != RESET) {
		USART_SendData(USART1, TxBuffer1[tx_tail++]);		
		if(tx_tail == TxBufferSize1 ) tx_tail = 0;
	}
}


void NVIC_Configuration(void)
{
   NVIC_InitTypeDef NVIC_InitStructure;

   //printf("NVIC_Config.. \n\r");
 
   /* Configure one bit for preemption priority */
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
 
   /* Enable the RTC Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#if 0

	/* Enable the USART3 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


#endif


	/* 1 bit for pre-emption priority, 3 bits for subpriority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/* Configure and enable SPI_MASTER interrupt -------------------------------*/
	NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


 
      /* 1 bit for pre-emption priority, 3 bits for subpriority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	#if 0
    NVIC_SetPriority(I2C2_EV_IRQn, 0x00);
    NVIC_EnableIRQ(I2C2_EV_IRQn);

    NVIC_SetPriority(I2C2_ER_IRQn, 0x01); 
    NVIC_EnableIRQ(I2C2_ER_IRQn);
	#endif


#ifdef  VECT_TAB_RAM
   /* Set the Vector Table base location at 0x20000000 */
   NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
   /* Set the Vector Table base location at 0x08000000 */

	#if 1 // IAP
   	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x4000);	// for IAP 0-> 0x8000
   	#else
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);	// for IAP 0-> 0x8000
   	#endif
#endif


  
 }




void I2C_Configuration(void)
{
	I2C_InitTypeDef  I2C_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	I2C_DeInit(I2C1);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);


	I2C_ITConfig(I2C1, I2C_IT_EVT|I2C_IT_BUF|I2C_IT_ERR, DISABLE);

	/* Configure I2C1 pins: SCL and SDA */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* I2C configuration */
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = fram_addr; // I2C1_SLAVE_ADDRESS7;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 200000; //I2C_Speed =200Khz;

	/* I2C Peripheral Enable */
	I2C_Cmd(I2C1, ENABLE);
	/* Apply I2C configuration after enabling it */
	I2C_Init(I2C1, &I2C_InitStructure);

}



void I2C_reset()
{	
	int i;
	
	I2C_Cmd(I2C1, DISABLE );
	
	/* Configure I2C1 pins: SCL and SDA */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;	// sda= pb7, SCL= pb6
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOB , GPIO_Pin_7);
	Delay_us(50);
	GPIO_ResetBits(GPIOB , GPIO_Pin_6);
	Delay_us(100);
	for ( i = 0; i < 9 ; i++) {
		GPIO_SetBits(GPIOB , GPIO_Pin_6);
		Delay_us(50);
		GPIO_SetBits(GPIOB , GPIO_Pin_7);
		Delay_us(200);
		GPIO_ResetBits(GPIOB , GPIO_Pin_6);
		Delay_us(50);
		GPIO_ResetBits(GPIOB , GPIO_Pin_7);
		Delay_us(200);
	}
	I2C_Configuration();
	I2C_SoftwareResetCmd(I2C1, ENABLE);
	Delay_us(200);
	I2C_SoftwareResetCmd(I2C1, DISABLE);
	

}

#if 1
void DAC_I2C_write(int ch, int value)
{
		static	int timeout = 0;
		uint8_t wdata[2];
	
		//if ( !eeprom_OK  ) return;
	
	
		wdata[0] = (byte) ((value >> 8 ) & 0x0f);	// normal operation, 12 bit data
		wdata[1] = (byte) (value & 0xff);

	
		__disable_irq();
	
		I2C_Cmd(I2C1, ENABLE);
	
	
		/* Send STRAT condition */
		I2C_GenerateSTART(I2C1, ENABLE);
		/* Test on EV5 and clear it */
	
		timeout = 0;
		while( timeout++ < EEPROM_W_WAIT && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) ;
	
		if ( timeout >= EEPROM_W_WAIT ) {
			I2C_GenerateSTART(I2C1, ENABLE);
		}
	
		timeout = 0;
		while( timeout++ < EEPROM_W_WAIT && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) ;
		
	
		if ( timeout >= EEPROM_W_WAIT ) {
			goto EEPROM_WRITE_ERROR;  
		}	
	
		/* Send EEPROM address for write */
		if ( EEPROM_SendAddr( dac_addr[ch], 0)!= 0 ) {
				goto EEPROM_WRITE_ERROR;
		} 
	
		//I2C_Cmd(I2C1, ENABLE);
	
		if ( EEPROM_SendData( wdata[0])!= 0 ) {
			goto EEPROM_WRITE_ERROR;
		} 
		if ( EEPROM_SendData( wdata[1])!= 0 ) {
			goto EEPROM_WRITE_ERROR;
		} 
	
		/* Send STOP condition */
	
	EEPROM_WRITE_ERROR:
		//Delay(EEPROM_T_WAIT);
		I2C_GenerateSTOP(I2C1, ENABLE);
		Delay(EEPROM_T_WAIT);
		I2C_Cmd(I2C1, DISABLE);
		Delay(EEPROM_E_WAIT);
		__enable_irq();
	
		return ;

}


int DAC_I2C_read(int ch)
{
	static int timeout;
	uint32_t ret;
	uint8_t  rdata[2];
	uint8_t	*buf;
	uint8_t size;


	buf = rdata;
	 __disable_irq();


	 I2C_Cmd(I2C1, ENABLE);
	 /* Send STRAT condition */
	 I2C_GenerateSTART(I2C1, ENABLE);
	 /* Test on EV5 and clear it */
	 
	 timeout = 0;
	 while( timeout++ < EEPROM_R_WAIT && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) ;
	 
	 if ( timeout >= EEPROM_R_WAIT ) {
		goto EEPROM_READ_ERROR;
	 }
	 

	/* Send EEPROM address for read */
	if ( EEPROM_SendAddr(dac_addr[ch], 1)!= 0 ) goto EEPROM_READ_ERROR;

	size = 2;
	while(size ) {
	    if(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
	    {
	      if(size == 2)
	      {
	        /* Disable Acknowledgement */
	        I2C_AcknowledgeConfig(I2C1, DISABLE);
	      }

	      if(size == 1)
	      {
	        /* Send STOP Condition */
	        I2C_GenerateSTOP(I2C1, ENABLE);
	      }

	      /* Read a byte from the EEPROM */
	      *buf = I2C_ReceiveData(I2C1);

	      /* Point to the next location where the byte read will be saved */
	      buf++;

	      /* Decrement the read bytes counter */
	      size--;
	    }
	}

	ret = (rdata[0] & 0x0f) * 256 + rdata[1];

	Delay(EEPROM_T_WAIT);
	I2C_Cmd(I2C1, DISABLE);
	Delay(EEPROM_E_WAIT);
	__enable_irq();

	return ret;

EEPROM_READ_ERROR:
	I2C_GenerateSTOP(I2C1, ENABLE);
	Delay(EEPROM_T_WAIT);
	I2C_Cmd(I2C1, DISABLE);
	Delay(EEPROM_E_WAIT);
	__enable_irq();

	return 0xffff;

}

#endif
int	EEPROM_SendAddr(uint8_t addr, uint32_t rw )
{
	int timeout = 0;

	if ( rw  == 1 ) {
		I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Receiver);
		while( timeout++ < 0x1ffff && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	}
	else  {
		I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Transmitter);
		while( timeout++ < 0x1ffff && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	}
	
	if ( timeout >= 0xffff ) {
		return 0xff;   
	}
	return 0;

}


int	EEPROM_SendData(uint8_t data)
{
	int timeout = 0;
	
	I2C_SendData(I2C1, data);

	while( timeout++ < 0x1ffff && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) ;
	
	if ( timeout >= 0x1ffff ) {
		return 0xff;   
	}
	return 0;
}

uint8_t eeprom_read_buffer(uint32_t ReadAddr, uint8_t *buf, int size)
{

	static int timeout;
	
	uint8_t	wdata[3];

	if ( !eeprom_OK  ) return 0xff;
	
	#if 0
	if ( ReadAddr >= 0x2000 ) {
		ReadAddr -= 0x2000;
		fram_addr = FRAM2_ADDR;
	}
	else 
	#endif
	fram_addr = FRAM1_ADDR;


	wdata[0] = (byte) ((ReadAddr >> 8 ) & 0xff);
	wdata[1] = (byte) ( ReadAddr & 0xff);

	 __disable_irq();

	 I2C_Cmd(I2C1, ENABLE);
	 /* Send STRAT condition */
	 I2C_GenerateSTART(I2C1, ENABLE);
	 /* Test on EV5 and clear it */

	 if ( size == 1 ) I2C_AcknowledgeConfig(I2C1, DISABLE);
	 else  I2C_AcknowledgeConfig(I2C1, ENABLE);
	 timeout = 0;
	 while( timeout++ < EEPROM_R_WAIT && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) ;
	 
	 if ( timeout >= EEPROM_R_WAIT ) {
		goto EEPROM_READ_ERROR;
	 }
	 
	 /* Send EEPROM address for write */
	 if ( EEPROM_SendAddr(fram_addr, 0)!= 0 ) goto EEPROM_READ_ERROR;
	 
	 //I2C_Cmd(I2C1, ENABLE);
	 if (EEPROM_SendData(wdata[0]) != 0) goto EEPROM_READ_ERROR ;
	 if (EEPROM_SendData(wdata[1]) != 0) goto EEPROM_READ_ERROR ;
 
	/* Send STRAT condition a second time */
	I2C_GenerateSTART(I2C1, ENABLE);

	/* Test on EV5 and clear it */
	timeout = 0;
	while( timeout++ < EEPROM_R_WAIT && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	if ( timeout >= EEPROM_R_WAIT ) {
		goto EEPROM_READ_ERROR;
	 }

	/* Send EEPROM address for read */
	if ( EEPROM_SendAddr(fram_addr, 1)!= 0 ) goto EEPROM_READ_ERROR;


#if 1
  /* While there is data to be read */
  while(size)
  {
    /* Test on EV7 and clear it */
    if(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
    {
      if(size == 2)
      {
        /* Disable Acknowledgement */
        I2C_AcknowledgeConfig(I2C1, DISABLE);
      }

      if(size == 1)
      {
        /* Send STOP Condition */
        I2C_GenerateSTOP(I2C1, ENABLE);
      }

      /* Read a byte from the EEPROM */
      *buf = I2C_ReceiveData(I2C1);

      /* Point to the next location where the byte read will be saved */
      buf++;

      /* Decrement the read bytes counter */
      size--;
    }
  }
#endif	
	
	Delay(EEPROM_T_WAIT);
	I2C_Cmd(I2C1, DISABLE);
	Delay(EEPROM_E_WAIT);
	__enable_irq();

	return 0;

EEPROM_READ_ERROR:
	I2C_GenerateSTOP(I2C1, ENABLE);
	Delay(EEPROM_T_WAIT);
	I2C_Cmd(I2C1, DISABLE);
	Delay(EEPROM_E_WAIT);
	__enable_irq();

	return 0xff;
}


void	eeprom_write_buffer(uint32_t addr, uint8_t *buf, int size)
{
	static  int timeout = 0;
	uint8_t	wdata[3];

	if ( !eeprom_OK  ) return ;
#if 0
	if ( addr >= 0x2000 ) {
		addr -= 0x2000;
		fram_addr = FRAM2_ADDR;
	}
	else 
#endif		
	fram_addr = FRAM1_ADDR;

	wdata[0] = (byte) ((addr >> 8 ) & 0xff);
	wdata[1] =  (byte) (addr & 0xff);

	__disable_irq();


	I2C_Cmd(I2C1, ENABLE);


	/* Send STRAT condition */
	I2C_GenerateSTART(I2C1, ENABLE);
	/* Test on EV5 and clear it */

	timeout = 0;
	while( timeout++ < EEPROM_W_WAIT && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) ;

	if ( timeout >= EEPROM_W_WAIT ) {
		I2C_GenerateSTART(I2C1, ENABLE);
	}

	timeout = 0;
	while( timeout++ < EEPROM_W_WAIT && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) ;

	if ( timeout >= EEPROM_W_WAIT ) {
		goto EEPROM_WRITE_ERROR;  
	}	

	/* Send EEPROM address for write */
	if ( EEPROM_SendAddr( fram_addr, 0)!= 0 ) {
			goto EEPROM_WRITE_ERROR;
	} 

	//I2C_Cmd(I2C1, ENABLE);

	if ( EEPROM_SendData( wdata[0])!= 0 ) {
		goto EEPROM_WRITE_ERROR;
	} 
	if ( EEPROM_SendData( wdata[1])!= 0 ) {
		goto EEPROM_WRITE_ERROR;
	} 
	while (size-- ) {
		if ( EEPROM_SendData( *buf++)!= 0 ) {
			goto EEPROM_WRITE_ERROR;
		}  
	}

	/* Send STOP condition */

EEPROM_WRITE_ERROR:

	//Delay(EEPROM_T_WAIT);
	I2C_GenerateSTOP(I2C1, ENABLE);
	Delay(EEPROM_T_WAIT);
	I2C_Cmd(I2C1, DISABLE);
	__enable_irq();
	Delay(EEPROM_E_WAIT);

}


uint8_t eeprom_read(uint32_t addr)
{
	uint8_t rdata;
	static int timeout;

	uint8_t	wdata[3];

	if ( !eeprom_OK  ) return 0xff;
#if 0
	if ( addr >= 0x2000 ) {
		addr -= 0x2000;
		fram_addr = FRAM2_ADDR;
	}
	else 
#endif
	fram_addr = FRAM1_ADDR;

	wdata[0] =(byte ) ((addr >> 8 ) & 0xff);
	wdata[1] = (byte) ( addr & 0xff);

	 __disable_irq();


	 I2C_Cmd(I2C1, ENABLE);
	 /* Send STRAT condition */
	 I2C_GenerateSTART(I2C1, ENABLE);
	 /* Test on EV5 and clear it */
	 
	 timeout = 0;
	 while( timeout++ < EEPROM_R_WAIT && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) ;
	 
	 if ( timeout >= EEPROM_R_WAIT ) {
		goto EEPROM_READ_ERROR;
	 }
	 
	 /* Send EEPROM address for write */
	 if ( EEPROM_SendAddr(fram_addr, 0)!= 0 ) goto EEPROM_READ_ERROR;
	 
	 //I2C_Cmd(I2C1, ENABLE);
	 if (EEPROM_SendData(wdata[0]) != 0) goto EEPROM_READ_ERROR ;
	 if (EEPROM_SendData(wdata[1]) != 0) goto EEPROM_READ_ERROR ;
 
	/* Send STRAT condition a second time */
	I2C_GenerateSTART(I2C1, ENABLE);

	/* Test on EV5 and clear it */
	 timeout = 0;
	while( timeout++ < EEPROM_R_WAIT && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
 	if ( timeout >= EEPROM_R_WAIT ) {
		goto EEPROM_READ_ERROR;
	 }

	/* Send EEPROM address for read */
	if ( EEPROM_SendAddr(fram_addr, 1)!= 0 ) goto EEPROM_READ_ERROR;
	I2C_AcknowledgeConfig(I2C1, DISABLE);
	I2C_GenerateSTOP(I2C1, ENABLE);

	 timeout = 0;
	while( timeout++ < EEPROM_R_WAIT && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));


	if ( timeout >= EEPROM_R_WAIT ) {
		goto EEPROM_READ_ERROR;
	 }
	 
	rdata = I2C_ReceiveData(I2C1);
	
	Delay(EEPROM_T_WAIT);
	I2C_Cmd(I2C1, DISABLE);
		__enable_irq();
	Delay(EEPROM_E_WAIT);

	return rdata;

EEPROM_READ_ERROR:
	I2C_GenerateSTOP(I2C1, ENABLE);
	Delay(EEPROM_T_WAIT);
	I2C_Cmd(I2C1, DISABLE);
		__enable_irq();
	Delay(EEPROM_E_WAIT);

	return 0xff;
}


void	eeprom_write(uint32_t addr, uint8_t data)
{
	static  int timeout = 0;
	uint8_t	wdata[3];

	if ( !eeprom_OK  ) return;

#if 0
	if ( addr >= 0x2000 ) {
		addr -= 0x2000;
		fram_addr = FRAM2_ADDR;
		printf("
	}
	else 
#endif
	fram_addr = FRAM1_ADDR;


	wdata[0] = (byte) ((addr >> 8 ) & 0xff);
	wdata[1] = (byte) (addr & 0xff);
	wdata[2] = data;

	__disable_irq();

	I2C_Cmd(I2C1, ENABLE);


	/* Send STRAT condition */
	I2C_GenerateSTART(I2C1, ENABLE);
	/* Test on EV5 and clear it */

	timeout = 0;
	while( timeout++ < EEPROM_W_WAIT && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) ;

	if ( timeout >= EEPROM_W_WAIT ) {
		I2C_GenerateSTART(I2C1, ENABLE);
	}

	timeout = 0;
	while( timeout++ < EEPROM_W_WAIT && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) ;
	

	if ( timeout >= EEPROM_W_WAIT ) {
		goto EEPROM_WRITE_ERROR;  
	}	

	/* Send EEPROM address for write */
	if ( EEPROM_SendAddr( fram_addr, 0)!= 0 ) {
			goto EEPROM_WRITE_ERROR;
	} 

	//I2C_Cmd(I2C1, ENABLE);

	if ( EEPROM_SendData( wdata[0])!= 0 ) {
		goto EEPROM_WRITE_ERROR;
	} 
	if ( EEPROM_SendData( wdata[1])!= 0 ) {
		goto EEPROM_WRITE_ERROR;
	} 
	if ( EEPROM_SendData( data)!= 0 ) {
		goto EEPROM_WRITE_ERROR;
	}  

	/* Send STOP condition */

EEPROM_WRITE_ERROR:
	//Delay(EEPROM_T_WAIT);
	I2C_GenerateSTOP(I2C1, ENABLE);
	Delay(EEPROM_T_WAIT);
	I2C_Cmd(I2C1, DISABLE);
	Delay(EEPROM_E_WAIT);
	__enable_irq();

	return ;
}

#if 0

void WIZ_SPI_Init(void)
{
	SPI_InitTypeDef   SPI_InitStructure;

	  /* SPI Config -------------------------------------------------------------*/
	  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	  SPI_InitStructure.SPI_CRCPolynomial = 7;

	  SPI_Init(SPI1, &SPI_InitStructure);
	  
	  /* Enable SPI */
	  SPI_Cmd(SPI1, ENABLE);

}
#endif

#if 1
void SPI_Configuration(void)
{
	SPI_InitTypeDef   SPI_InitStructure;


	SPI_I2S_DeInit(SPI2);
	// SPI2 Config
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;	// org = 64 -> 4
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);
	/* Enable SPI2 */
	SPI_Cmd(SPI2, ENABLE);


	GPIO_SetBits(GPIOB, GPIO_Pin_12);
	CS5463_CS_OFF;	//GPIO_WriteBit(GPIOA, GPIO_Pin_7, 1)
	//CS5463_Sync();
	
}
#endif




unsigned int	CS5463_page = 0;

u8	SPI_SendByte(u8 data)
{
	uint8_t	rdr;

	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE)==RESET);
	SPI_I2S_SendData(SPI2, data); //read device ID 0x3B
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE)==RESET);
	rdr = SPI_I2S_ReceiveData(SPI2);

	return rdr;
}




unsigned int CS5463_Command(unsigned char cmd)
{
	uint32_t	ret;

	CS5463_CS_ON;
	ret = SPI_SendByte(cmd);
	CS5463_CS_OFF;
	return ret;
}


unsigned int CS5463_Read_(unsigned char ucAddr)
{
    unsigned int ret;
	int i;
	uint8_t	rtemp[4], temp;

	CS5463_CS_ON;
	temp = (ucAddr << 1) & 0x3e;


    rtemp[3] = SPI_SendByte(temp);		

	for ( i= 0; i < 3; i++) {
	    rtemp[i] = SPI_SendByte(0xff);	// null data to read 3bytes
	}
	CS5463_CS_OFF;

	
	//printf("SPI RD_temp1 = %x  %x %x %x\n\r", temp, rtemp[0], rtemp[1], rtemp[2]);
	ret = ((rtemp[0]<< 16) & 0xff0000) | ((rtemp[1]<< 8) & 0xff00) |(rtemp[2] & 0xff);
	return ret;
}

unsigned int CS5463_Write_(unsigned char ucAddr, u32 ucValue)
{
	int i;
	uint8_t	temp[4], temp1;

	CS5463_CS_ON;

	temp1 = (ucAddr << 1) & 0x3e | 0x40;
	temp[0] = ( ucValue >> 16 )& 0xff;
	temp[1] = ( ucValue >> 8 ) & 0xff;
	temp[2] = ucValue & 0xff;

	//printf("SPI WR_data = %x  %x %x %x\n\r", temp1, temp[0], temp[1], temp[2]);

	temp1 = SPI_SendByte( temp1);
	for ( i= 0; i < 3; i++) {
	    temp[i] = SPI_SendByte(temp[i]);	// null data to read 3bytes
	}

	CS5463_CS_OFF;

	//printf("SPI WR_temp = %x  %x %x %x\n\r", temp1, temp[0], temp[1], temp[2]);
	//ret = ((temp[0]<< 16) & 0xff0000) | ((temp[1]<< 8) & 0xff00) |(temp[2] & 0xff);

	return temp1;
}

void	CS5463_Reset()
{
	volatile int i = 0;
	CS5463_CS_OFF;
	CS5463_RESET_HIGH;
	CS5463_RESET_LOW;
	Delay_ms(1);

	CS5463_RESET_HIGH;
	Delay_ms(1);
}


void	CS5463_Sync()
{
	volatile int i = 0;

	CS5463_CS_OFF;
	i++;
	i++;
	i++;
	i++;
	CS5463_CS_ON;
	//CS5463_RESET_LOW;
	//while ( i++ < 100) {}
	Delay_ms(1);
	CS5463_Command(0xff);
	CS5463_Command(0xff);
	CS5463_Command(0xff);
	CS5463_Command(0xff);

	CS5463_Command(0xfe);

}


uint32_t	CS5463_Read(int page, int addr)
{
	if ( page != CS5463_page ) {
		CS5463_Write_(31, page);
		CS5463_page = page;
	}
	return CS5463_Read_( (u8) addr);
}

uint32_t	CS5463_Write(int page, int addr, uint32_t data)
{
	if ( page != CS5463_page ) {
		CS5463_Write_(31, page);
		CS5463_page = page;
	}
	return CS5463_Write_( (u8) addr, data);
}


void	CS5463_init()
{
	//Delay_ms(300);
	CS5463_Reset();
	Delay_ms(1);
	CS5463_Command(0xE8);
	Delay_ms(200);
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */

#if 1
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */

  #if 1
  USART_SendData(USART1, (uint8_t) ch);

  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {}

  #else

  USART_TxChar(ch);

  #endif

  return ch;
}

#endif

/**
  * @brief  Gets numeric values from the hyperterminal.
  * @param  None
  * @retval None
  */
uint8_t USART_Scanf(uint32_t value)
{
  uint32_t index = 0;
  uint32_t tmp[2] = {0, 0};

  while (index < 2)
  {
    /* Loop until RXNE = 1 */
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
    {}
    tmp[index++] = (USART_ReceiveData(USART1));
    if ((tmp[index - 1] < 0x30) || (tmp[index - 1] > 0x39))
    {
     // printf("\n\rPlease enter valid number between 0 and 9");
      index--;
    }
  }
  /* Calculate the Corresponding value */
  index = (tmp[1] - 0x30) + ((tmp[0] - 0x30) * 10);
  /* Checks */
  if (index > value)
  {
   // printf("\n\rPlease enter valid number between 0 and %d", value);
    return 0xFF;
  }
  return index;
}

#if 0
void	 relay_toggle(int no)
{
	if ( relay_status(no) ) relay_off(no);
	else relay_on(no);

}
#endif

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
  printf("Asset Fail: %s@%d\n\r");
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

