
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "util.h"
#include	"rabm.h"
#include "rtime.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

extern __IO uint32_t TimeDisplay;

time_t TIME;
struct tm  rtc_time;
struct tm *ptim;

int	reset_type;

void RTC_Configuration(void);

void	set_date_only(int year, int month, int day)
{
	get_system_time(&sys_info);
	set_time(year, month, day, sys_info.hour, sys_info.min, sys_info.sec);
}

void	set_time_only(int hour, int min, int sec)
{
	get_system_time(&sys_info);
	set_time(sys_info.year, sys_info.month, sys_info.day, hour, min, sec);
}

void	set_time(int year, int month, int day, int hour, int min, int sec)
{
        ptim = &rtc_time;
	ptim->tm_year = year -1900;
	ptim->tm_mon = month -1;
	ptim->tm_mday = day;

	ptim->tm_hour = hour;
	ptim->tm_min = min;
	ptim->tm_sec = sec;

	TIME = mktime(ptim)-0x3FF36300;

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	RTC_SetCounter(TIME);
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}

void	get_system_time(SYSTEM_INFO *sys)
{
     
	TIME = RTC_GetCounter()+0x3FF36300;
	ptim = localtime(&TIME);
	
	//printf("DD/MM/YYYY HH:MM:SS %02d/%02d/%4d ",ptim->tm_mday,ptim->tm_mon+1,ptim->tm_year+1900); 
	//printf("%02d:%02d:%02d\n\r",ptim->tm_hour,ptim->tm_min,ptim->tm_sec);

	sys->year = ptim->tm_year+1900;
	sys->month = ptim->tm_mon+1;
	sys->day = ptim->tm_mday;
	sys->hour = ptim->tm_hour;
	sys->min = ptim->tm_min;
	sys->sec= ptim->tm_sec;
}


u32	get_time_counter()
{
	return RTC_GetCounter();
}
void	get_time()
{
	TIME = RTC_GetCounter()+0x3FF36300;
	ptim = localtime(&TIME);
	
	//printf("DD/MM/YYYY HH:MM:SS %02d/%02d/%4d ",ptim->tm_mday,ptim->tm_mon+1,ptim->tm_year+1900);
	//printf("%02d:%02d:%02d\n\r",ptim->tm_hour,ptim->tm_min,ptim->tm_sec);
}


 uint32_t Time_Regulate(void)
 {
   uint32_t Tmp_HH = 0xFF, Tmp_MM = 0xFF, Tmp_SS = 0xFF;
 
  // printf("\r\n==============Time Settings=====================================");
  // printf("\r\n  Please Set Hours");

 #if 0
   while (Tmp_HH == 0xFF)
   {
     Tmp_HH = USART_Scanf(23);
   }
   printf(":  %d", Tmp_HH);
   printf("\r\n  Please Set Minutes");
   while (Tmp_MM == 0xFF)
   {
     Tmp_MM = USART_Scanf(59);
   }
   printf(":  %d", Tmp_MM);
   printf("\r\n  Please Set Seconds");
   while (Tmp_SS == 0xFF)
   {
     Tmp_SS = USART_Scanf(59);
  }
   printf(":  %d", Tmp_SS);
 #endif

	Tmp_HH = 12;
 	Tmp_MM = 30;
 	Tmp_SS = 30;
 
   /* Return the value to store in RTC counter register */
   return((Tmp_HH*3600 + Tmp_MM*60 + Tmp_SS));
 }



/*******************************************************************************
* Function Name  : Time_Adjust
* Description    : Adjusts time.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Time_Adjust(void)
{
   /* Wait until last write operation on RTC registers has finished */
   RTC_WaitForLastTask();
   /* Change the current time */
   RTC_SetCounter(0x9999);
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

}

void Time_Display(uint32_t TimeVar)
{

   //printf("Time_Display\n\r");
   /* Reset RTC Counter when Time is 23:59:59 */
   if (RTC_GetCounter() == 0x0001517F)
   {
      RTC_SetCounter(0x0);
      /* Wait until last write operation on RTC registers has finished */
      RTC_WaitForLastTask();
   }

   #if 0
   /* Compute  hours */
   THH = TimeVar / 3600;
   /* Compute minutes */
   TMM = (TimeVar % 3600) / 60;
   /* Compute seconds */
   TSS = (TimeVar % 3600) % 60;
 
   printf("  Time: %0.2d:%0.2d:%0.2d\r", THH, TMM, TSS);
   #endif
}


/*******************************************************************************
* Function Name  : Time_Show
* Description    : Shows the current time (HH:MM:SS) on the Hyperterminal.
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void Time_Show(void)
{
  // printf("\n\rTime_Show \n\r");
 
   /* Infinite loop */
   while (1)
   {
     /* If 1s has been elapsed */
     if (TimeDisplay == 1)
     {
       GPIO_WriteBit(GPIOE, GPIO_Pin_12, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_12)));
       /* Display current time */
       Time_Display(RTC_GetCounter());
       
       TimeDisplay = 0;
     }
   }

}


/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : Configures the RTC.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_Configuration1(void)
{


	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	/* RTC Configuration */
	if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
	{
		/* Backup data register value is not correct or not yet programmed (when
		the first time the program is executed) */
		printf(" ***RTC INIT***");
		#if 1
		/* RTC Configuration */
		RTC_Configuration();

		/* Adjust time by values entred by the user on the hyperterminal */
		set_time(2012, 1, 1, 0,0,0);
		//Time_Adjust();
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
		#endif
	}
	else
	{
		PWR_BackupAccessCmd(ENABLE);	// *** enabel RTC and Backup area access
	}

        //reset_type = RCC->CSR;
        
      
}


void RTC_Configuration(void)
{

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	
	/* Reset Backup Domain */
	BKP_DeInit();

#if 0
	/* Enable LSI */
	RCC_LSICmd(ENABLE);
	/* Wait till LSI is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET){};
	/* Select LSI as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
#else
	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	/* Wait till LSE is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET){};
	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
#endif
	
	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);
	
#if 0
	/* Disable the Tamper Pin */
	BKP_TamperPinCmd(DISABLE); /* To output RTCCLK/64 on Tamper pin, the tamper
							   functionality must be disabled */
	/* Enable RTC Clock Output on Tamper Pin */
	BKP_RTCCalibrationClockOutputCmd(ENABLE);
#endif
	
	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	/* Enable the RTC Second */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
	/* Set RTC prescaler: set RTC period to 1sec */
#if 0
	RTC_SetPrescaler(31999); /* RTC period = RTCCLK/RTC_PR = (32.000 KHz)/(31999+1) */
#else
	RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
#endif
	
	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}


