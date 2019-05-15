/**
  ******************************************************************************
  * @file    RTC/Calendar/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
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

#define AC_10MS


/* Includes ------------------------------------------------------------------*/
#include 	"stm32f10x_it.h"
#include	"uart.h"
#include 	"stm32f10x_exti.h"
#include 	"config.h"
#include	"rabm.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup RTC_Calendar
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern __IO uint32_t TimeDisplay;

extern uint8_t TxBuffer1[]; 
extern uint8_t TxBuffer2[]; 
extern uint8_t RxBuffer1[];
extern uint8_t RxBuffer2[];
extern __IO uint8_t TxCounter1;
extern __IO uint8_t TxCounter2;
extern __IO uint8_t RxCounter1; 
extern __IO uint8_t RxCounter2;
extern uint8_t NbrOfDataToTransfer1;
extern uint8_t NbrOfDataToTransfer2;
extern uint8_t NbrOfDataToRead1;
extern uint8_t NbrOfDataToRead2;

extern uint16_t AC_cutoff_flag;
unsigned long long A1_off_time = 0;

extern uint16_t	A1_flag, A1_count, A1_rise, A1_mode, A1_recovery;
uint16_t  A1_start_time = 0,  A1_start = 0, A1_wait = 0;

extern uint16_t check_status_flag;
extern uint16_t	adc_dr[], adc_ready;

volatile unsigned long long	time10ms = 0;
volatile unsigned int time3ms = 0;



ADC_VALUE adv, ad_temp;

unsigned long	dhcp_time;		// 1 sec timer
unsigned char	dhcp_timer_flag = 0;
uint16_t bat_can_sts = 0,bat_can_prt = 0, bat_can_v, bat_can_a, bat_can_flag = 0;
CanRxMsg RxMessage;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSV_Handler exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	//GPIO_WriteBit(GPIOA, GPIO_Pin_8, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_8)));

}

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

/**
  * @brief  This function handles RTC global interrupt request.
  * @param  None
  * @retval None
  */
void RTC_IRQHandler(void)
{
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
  {
    /* Clear the RTC Second interrupt */
    RTC_ClearITPendingBit(RTC_IT_SEC);

    /* Toggle LED1 */
    //STM_EVAL_LEDToggle(LED1);

    /* Enable time update */
    TimeDisplay = 1;

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    
  }

}


#if 1

/**
  * @brief  This function handles USARTy global interrupt request.
  * @param  None
  * @retval None
  */


unsigned int	rx_head = 0, rx_tail = 0, tx_head = 0, tx_tail = 0;
unsigned int	rx_overrun;

void USART1_IRQHandler(void)
{

  __IO uint16_t status;

  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
  {

   
    /* Read one byte from the receive data register */
    RxBuffer1[rx_head++] = USART_ReceiveData(USART1);
	
    if(rx_head == RxBufferSize1)
    {
      /* Disable the USARTy Receive interrupt */
      //USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
      rx_head = 0;
    }
	if ( rx_head == rx_tail ) {
		rx_overrun = 1;
		if ( rx_tail++ ==  RxBufferSize1 ) rx_tail = 0;
	}
  }
#if 0  
  if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)
  {   
    /* Write one byte to the transmit data register */
	if ( tx_head != tx_tail ) {
	    USART_SendData(USART1, TxBuffer1[tx_tail++]);

	    if(tx_tail == TxBufferSize1)
	    {
	      /* Disable the USARTy Transmit interrupt */
	      //USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
		  tx_tail = 0;
	    }    		
  	}
	else {
		
		USART1->SR &= ~USART_FLAG_TC ;

	}
  }
#endif
}

/**
  * @brief  This function handles USARTz global interrupt request.
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{

#if 0
  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
  {
    /* Read one byte from the receive data register */
    RxBuffer2[RxCounter2++] = USART_ReceiveData(USART2);

    if( RxCounter2 == RxBufferSize2)
    {
      /* Disable the USARTz Receive interrupt */
      USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
    }
  }
  
  if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
  {   
    /* Write one byte to the transmit data register */
    USART_SendData(USART2, TxBuffer2[TxCounter2++]);

    if(TxCounter2 == TxBufferSize2)
    {
      /* Disable the USARTz Transmit interrupt */
      USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
    }
  }
#endif  
}
#endif

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

/**
  * @}
  */ 


/**
  * @brief  This function handles I2C1 Event interrupt request.
  * @param  None
  * @retval : None
  */
void I2C1_EV_IRQHandler(void)
{



}

/**
  * @}
  */

/**
  * @brief  This function handles I2C1 Event interrupt request.
  * @param  None
  * @retval : None
  */
void I2C2_EV_IRQHandler(void)
{



}
/**
  * @}
  */

/**
  * @brief  This function handles I2C2 Error interrupt request.
  * @param  None
  * @retval : None
  */
void I2C2_ER_IRQHandler(void)
{

}



/**
  * @brief  This function handles I2C1 Error interrupt request.
  * @param  None
  * @retval : None
  */
void I2C1_ER_IRQHandler(void)
{

}


/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : This function handles TIM4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler(void)
{	
  u16 rcapture;
  static u32 tmcnt=0,  i = 0;
  uint8_t level;


  if(TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)
  {

		
    rcapture = TIM_GetCapture1(TIM4);
	TIM_SetCompare1(TIM4, rcapture + 1440); //200us
    tmcnt++;

    if ( !(tmcnt % 10 ) ) time10ms ++;

    if(tmcnt>= 1000 )
    {   // 1sec
     //GPIO_WriteBit(GPIOA, GPIO_Pin_8, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_8)));

	  tmcnt =0;
	  //Timer4_sec_ISR();
    }

	time3ms++;
	if (time3ms >= 3 ) time3ms = 0;

	__disable_interrupt();
        
	if (  !(tmcnt % 10) && adc_ready ){		
		
		if (i == 0 ) {
			ad_temp.dcv = ad_temp.dca = ad_temp.batv = 0;
			ad_temp.bata = ad_temp.rect = ad_temp.batt = ad_temp.refv = 0;
			ad_temp.dacv = 0;
		}

		if ( i < 10 ) {
			ad_temp.dcv += adc_dr[IDX_DCV]; 
			ad_temp.dca += adc_dr[IDX_DCA];
			ad_temp.batv += adc_dr[IDX_BATV];
			//ad_temp.bata += adc_dr[IDX_BATA];
			ad_temp.rect += adc_dr[IDX_RECT];
			ad_temp.batt += adc_dr[IDX_BATT];
			ad_temp.dacv += adc_dr[IDX_DACV];
			ad_temp.refv += adc_dr[IDX_REFV];			
		}
		i++;

		if ( i == 10 ) {
			adv.dcv = ad_temp.dcv; 
			adv.dca = ad_temp.dca;
			adv.batv = ad_temp.batv;
			adv.bata = ad_temp.bata;
			adv.rect = ad_temp.rect;
			adv.batt = ad_temp.batt;
			adv.refv = ad_temp.refv;	
			adv.dacv = ad_temp.dacv;
			i = 0;
			check_status_flag = 1;
		}		
		read_adc();
	}

	__enable_interrupt();
        
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
  }
}


void EXTI1_IRQHandler(void)
{

  if(EXTI_GetITStatus(EXTI_Line1) != RESET)
  {

	A1_flag = 1;
    /* Clear the  EXTI line 1 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
#if  0
	static int c= 0;
	if (c == 0 ) {
		relay_on(6);
	}
	else relay_off(6);
	c = 1-c;
#endif

  CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
  if ((RxMessage.ExtId == CAN_READ_BMS ))	// FOR LOOPBACK TEST
  //if (RxMessage.ExtId == CAN_RSP_BMS )   // 0x1000c080))		// CAN RX CODE
  { 	
		//bat_can_v =  *(uint16_t *) &RxMessage.Data[0];
		//bat_can_a =  *(uint16_t *) &RxMessage.Data[2];
	    //bat_can_sts = *(uint16_t *) &RxMessage.Data[4];
		//bat_can_prt = *(uint16_t *) &RxMessage.Data[6];
		bat_can_flag = 1;
	  }

  CAN_ClearITPendingBit(CAN1,CAN_IT_FMP0);  
}





/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
