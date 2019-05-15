
// defined in IAR compiler optios
//#define	SKT_27V
//#define 	SKT
//#define 	KT
//

#define	VERSION_2	// defined 2013 2.8 by ksyoo

#ifdef TEST_MODE
#warning	TEST mode
#endif
#ifdef KT_OLD
#warning	KT_OLD version
#elif defined KT
#warning	KT_3RU version
#elif defined(SKT)
#warning	SKT_48V version
#elif defined(SKT_27V)
#warning	SKT_27V version
#endif



#define BIG_ENDIAN	1


int		USART_Rx(unsigned char *ch);
void 	USART_TxChar(unsigned char ch);
int		USART_TxBuffer_Size();
void		USART_TxNData(unsigned char *buf, int size);
uint8_t		USART_Scanf(uint32_t value);
void		GPIOA_Configuration();
void SerialPutChar(uint8_t );


void RCC_Configuration(void);
void NVIC_Configuration(void);
void GPIO_Configuration(void);
void I2C_Configuration(void);
void SPI_Configuration(void);
void Timer4_Init(void);
void	UART_Configuration(void);
void 	RTC_Configuration1(void);
void	RCC_ClearFlag(void);
void	EXTI1_Config(void);
void	DMA_Configuration(void);
void	ADC_Configuration_scanmode(void);
void	Reset_W5200(void);
void read_adc();
void Init_RxMes(CanRxMsg *RxMessage);


/* IO map
PA0 : BAT temperature, ADC
PA1 : AC FAIL, GPIO input, ext. pull up, interrupt
PA2 : RECT ON, GPIO output, ext. pull up or down
PA3 : DI_BAT_SW, GPIO input, 
PA4 : DC_REF, DAC output
PA5 : SR on/off, GPIO output, external pull down
PA6 : CS5463 reset, GPIO output, external pull up
PA7 : CS5463 CS, GPIO output, ext. pull up
PA8 : POWER LED, GPIO output, ext. pull down
PA9 : UART TX
PA10 : UART RX
PA11 : FAIL LED, GPIO output, ext. pull up
PA12 : RUN LED, GPIO output, ext. pull up
PA13 : SWDIO
PA14 : SWCLK
PA15 : Watch dog reset, GPIO output

PB0 : 
PB1 : 
PB2 : BOOT1, ext. pull down
PB3 : Relay3, GPIO output, ext. pull down
PB4 : Relay4, GPIO output, ext. pull down
PB5 : 
PB6 : I2C SCL
PB6 : I2C SDA
PB8 : BOOT0 SW, GPIO input, ext. pull down
PB9 :  wiznet reset, GPIO output, pull up
PB10 : 
PB11 : wizet interrupt, GPIO output, pull up
PB12 : WIZ SPI NSS
PB13 : SPI SCLK
PB14 : SPI MISO
PB15 : SPI MOSI

PC0 : DCV input, ADC
PC1 : DCA input, ADC
PC2 : BAT. current , ADC
PC3 : RECT, tempreratur, ADC
PC4 : Bat. temperature, ADC
PC5 : Ref, DAC input, ADC
PC6 :
PC7 : BAT_NFB, GPIO input, open collector
PC8 : BAT CHG status, GPIO input, open collector
PC9 : BAT DCHG status, GPIO input, open collector
PC10 : BAT_PWN, GPIO output, pull down
PC11 : Eth/Alarm select, GPIO output, ext. pulldown
PC12 : RELAY 1, GPIO output, ext. pulldown
PC13 :
PC14 : OSC
PC15: OSC

PD0 : OSCIN
PD1 : OSCOUT
PD2 : Relay2, GPIO output, ext. pulldown

*/


#define ADC_DCV		10
#define ADC_BATV	14
#define ADC_DCA		11
#define ADC_BATA	12
#define ADC_RECT	13
#define ADC_BATT	0


#define	GPIO_AC_FAIL		GPIOA
#define	AC_FAIL_PIN		GPIO_Pin_1

#if 0
#define	GOUT_LED_POWER	GPIOA
#define	LED_POWER_PIN	GPIO_Pin_8


#define	GPIO_LED_RUN	GPIOA
#define	LED_RUN	GPIO_Pin_11

#define	GPIO_LED_FAIL	GPIOA
#define	LED_FAIL	GPIO_Pin_12
#endif

#define	GPIO_RECT_ON	GPIOA
#define	RECT_ON		GPIO_Pin_2

#define	GPIO_DI_BAT_SW		GPIOA
#define	DI_BAT_SW		GPIO_Pin_3

#define GPIO_WD			GPIOA
#define WD_PIN 			GPIO_Pin_15





#define	NORMAL_OPEN	0
#define	NORMAL_CLOSE	1

//#define	RELAY_SAMSUNG	0
//#define	RELAY_LG		1
//#define	RELAY_NSN		2

#define 	RELAY_SKT		4

#define GPIO_PFC_TH	GPIOB
#define PFC_TH_PIN	GPIO_Pin_0
#define GPIO_PFC1	GPIOB
#define PFC1_PIN	GPIO_Pin_12
#define GPIO_PFC2	GPIOB
#define PFC2_PIN	GPIO_Pin_13
#define GPIO_PFC3	GPIOB
#define PFC3_PIN	GPIO_Pin_14
#define GPIO_PFC4	GPIOB
#define PFC4_PIN	GPIO_Pin_15

#define GPIO_TP5	GPIOB
#define TP5_PIN	GPIO_Pin_1


#define GPIO_RELAY1 GPIOC
#define RELAY1_PIN	GPIO_Pin_12
#define GPIO_RELAY2 GPIOD
#define RELAY2_PIN	GPIO_Pin_2
#define GPIO_RELAY3 GPIOB
#define RELAY3_PIN	GPIO_Pin_3
#define GPIO_RELAY4 GPIOB
#define RELAY4_PIN	GPIO_Pin_4
#define GPIO_RELAY5 GPIOB
#define RELAY5_PIN	GPIO_Pin_5
#define GPIO_RELAY6 GPIOB
#define RELAY6_PIN	GPIO_Pin_8

//#define	GPIO_BAT_TEST_SW	GPIOB
//#define	BAT_TEST_SW	GPIO_Pin_8

#define GPIO_RECT_CON	GPIOC
#define RECT_CON_PIN		GPIO_Pin_3
#define GPIO_LOAD_ON	GPIOC
#define LOAD_ON_PIN		GPIO_Pin_4

#define GPIO_BAT_NFB	GPIOC
#define BAT_NFB_PIN 	GPIO_Pin_7
#define GPIO_BAT_CHG	GPIOC
#define BAT_CHG_PIN 	GPIO_Pin_8
#define GPIO_BAT_DCHG	GPIOC
#define BAT_DCHG_PIN 	GPIO_Pin_9
#define GPIO_BAT_PWR	GPIOC
#define BAT_PWR_PIN 	GPIO_Pin_10
#define GPIO_ETH_SEL	GPIOC
#define ETH_SEL_PIN		GPIO_Pin_11


#define GPIO_SR_ON	GPIOC		//GPIOC //GPIOA
#define SR_ON_PIN 	GPIO_Pin_6 //GPIO_Pin_13 //

//#define relay_on(no) GPIO_WriteBit(GPIO_RELAY, RELAY1 << (no -1), 1)
//#define relay_off(no) GPIO_WriteBit(GPIO_RELAY, RELAY1 << (no -1), 0)
#define	relay_status(no)	GPIO_ReadOutputDataBit(GPIO_RELAY,RELAY1 << (no -1))

void	relay_toggle(int no);
void	CS5463_Sync();
void	CS5463_Reset();
uint32_t	CS5463_Write(int page, int addr, uint32_t data);
uint32_t CS5463_Read(int page, int addr);
uint32_t CS5463_Command(uint8_t command);
void	CS5463_init();

#define FRAM1_ADDR		0xa0
#define FRAM2_ADDR		0xa2
//#define	EEPROM_ADDR	0xa0
#define	EEPROM_R_WAIT	(0x8ffff)
#define	EEPROM_W_WAIT	(0x7ffff)


#define	EEPROM_T_WAIT	0x500 	//(0xfff )
#define	EEPROM_E_WAIT 0x1000	// (0x1fff)


#define RESET_POWERON	0x08000000
#define RESET_NRST		0x04000000
#define RESET_SWRESET	0x10000000
#define RESET_IWDG		0x20000000


#define CS5463_CS_ON	GPIO_WriteBit(GPIOA, GPIO_Pin_7, (BitAction)0)
#define	CS5463_CS_OFF	GPIO_WriteBit(GPIOA, GPIO_Pin_7, (BitAction)1)
#define	CS5463_RESET_LOW	GPIO_WriteBit(GPIOA, GPIO_Pin_6, (BitAction)0)
#define	CS5463_RESET_HIGH	GPIO_WriteBit(GPIOA, GPIO_Pin_6, (BitAction)1)

//#define BAT_CON_ON()		{ GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction)1);}
//#define BAT_CON_OFF()		{ GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction)0);}

#define IDX_DCV 6
#define IDX_DCA 0
#define IDX_RECT 1
#define IDX_BATT 2
#define IDX_BATV 3
#define IDX_DACV 4
#define IDX_REFV 5
#define IDX_BATA 7



void	wiz_test();
void	wiz_init();
void 	iinchip_init(void);
uint8_t eeprom_read(uint32_t ReadAddr);
void	eeprom_write(uint32_t addr, uint8_t data);
void	eeprom_write_buffer(uint32_t addr, uint8_t *buf, int size);
uint8_t eeprom_read_buffer(uint32_t ReadAddr, uint8_t *buf, int size);
int ADC_ReadCh(int ch);
void	set_flash_ip(int ip);
void	ip_update_flash_ip(int ip);
void Reset_W5200(void);
void	set_alarm_mode(int mode);
int	EEPROM_SendAddr(uint8_t addr, uint32_t rw );
int	EEPROM_SendData(uint8_t data);

void DAC_WriteCh(int ch, int value);
int DAC_ReadCh(int ch);

void DAC_I2C_write(int ch, int value);
int DAC_I2C_read(int ch);



void init_adc_dr();


#define	enable() __set_PRIMASK(0)
#define	disable()	__set_PRIMASK(1)

void	dprintf(char *, ...);

#define RCC_APB2Periph_GPIO_CAN1	RCC_APB2Periph_GPIOB
#define GPIO_Remapping_CAN 			GPIO_Remap1_CAN1
#define GPIO_CAN					GPIOA
#define GPIO_Pin_CAN_RX			GPIO_Pin_11
#define GPIO_Pin_CAN_TX			GPIO_Pin_12
#define CANx	CAN1

#define CAN_READ_BMS		0x1800c001	// host = 0x00, bms = 0x01
#define CAN_RSP_BMS			0x1000c080

void Init_TxMessage();


