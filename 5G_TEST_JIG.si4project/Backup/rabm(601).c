
/*
***************************************************************

	THIS RECTIFIER and Battery Manager program

	YOOHA.  2012.1.27
	written by ksyoo

	R5.0 ( SAMSUNG SPEC 0.8, 0.9, 0.91 )

	48V for SKT

***************************************************************
*/

//#define	BAT_TEST 1


//2017 5.16 ovp check delay(5sec) due to overshoot at start up in low temperature
//2017 5.18 current limit 3.0 -> 4.5

void A1_isr(int edge);

#define AC_10MS

#include 	"stm32f10x.h"
#include 	<stdio.h>
#include 	<stdlib.h>
#include	<string.h>
#include    <time.h>

#include 	"util.h"
#include    "I2CRoutines.h"
#include	"rabm.h"
#include	"command.h"
#include 	"eeprom.h"
#include	"alarmComm.h"

#if 0
#define FLASH_LOG
#endif

#if 0
#define HIGH_TEMP_TEST
#endif

#define RECOVER_OVP		1
#define RECOVER_FAIL	2
#define	RECOVER_USER	3
#define	RECOVER_FORCED	4
#define	RECOVER_OCP		5
#define	RECOVER_AC		6
#define	RECOVER_OTP		7
#define	RECOVER_OVP_RESET		8



#define REC_IDLE	0
#define REC_MIN		1
#define REC_WAIT1	2
#define REC_ON		3
#define REC_WAIT2	4
#define REC_RAMP	5
#define REC_WAIT3	6
#define REC_END		7
#define REC_FAIL	8
#define REC_FORCED	9
#define REC_OCP_FAIL	10
#define REC_OCP		11
#define REC_OVP_RESET		12
#define REC_OVP		13
#define REC_OVP_FAIL 14
#define REC_OVP_END		15

#define RECOVER_FORCED_TIME	500	// 5secs
#define RECOVER_OCP_TIME	1000	// 10secs
#define RECOVER_OVP_RESET_TIME	9000	// 9000, 90secs = 1min30sec
#define OVP_CLEAR_TIME	180000	// 180000 = 30Min, (  1min= 6000 )

void	avg_ref_adc();

extern IO_STS outputs[];
extern char dhcp_state ;

extern uint8_t Enable_DHCP;
extern uint16_t	adc_dr[];
extern ADC_VALUE adv;

extern uint8_t rect_restart, A1_wait;
extern	volatile unsigned long long	time10ms;
extern	unsigned char user_psu_name[];
extern int current_limit_flag;
extern volatile unsigned int time3ms;
extern u16 operation_mode;
extern unsigned long long A1_off_time ;

uint32_t test_value;

uint16_t AC_cutoff_flag = 0, A1_recovery = 0;
uint16_t A1_flag = 0, A1_count = 0, A1_mode = 1, A1_rise = 0;
unsigned long long A1_time = 0;
uint16_t adc_ready = 0;

int temp_3ms;
//extern int pi_control_flag;


//int	debug;
int eth_flag = 0;
extern F_LOG_TYPE		log_frame;

RECT_STATUS		rect_sts;
ALARM_STATUS	alarm_sts;
AD_SETUP		ad_offset;
OP_STATUS		op_sts;
LOG_STATUS		log_sts;
SYSTEM_INFO		sys_info;
BAT_TEST_DATA	bat_data;

float save_test_V;
float	Vref = V_REF ;
float	DCV_value = 0.0;

int	rect_on_flag = 0;
int	log_sending = 0, log_send_size, log_send_index;
int	alarm_notification_flag = 0;

int	eeprom_OK	= 0;
int system_shutdown_flag = 0;
int OVP_off_ready = 0, OVP_on_ready = 0;
int OTP_flag = 0;
int valid_flag = 0;

int	I2C_dev_config(I2C_TypeDef *I2Cx, byte addr);
void diagnosis(byte command);
//void bat_cutoff_on(int mode);
void check_wiznet();
void system_shutdown_48();
void bat_test_start();


BAT_AUTO_TEST_INFO	auto_test;

//int bat_test_flag  = 0;
int	 log_flag = 0;
unsigned long time_tmp, ovp_timer;
int	test_mode = 0;
uint16_t ovp_enable;

LOG_HEADER	log_header;
word	log_id = 0xc001;
int		log_header_no;
uint16_t	system_alarm_status = 0;


LOG_TYPE	log_event;


//int bat_lvd_flag = 0;
//int	power_back_time;
//
//tatic int	blackout_flag = 0;
//static int	blackout_time;

static	int ovp_lock = 0, fail_lock = 0;
static int ovp_count = 0, ovp_flag = 0;;
static int ocp_flag = 0, ocp_count = 0, ocp_lock = 0;;
static int ac_shutdown_flag = 0;

int fail_on_ready = 0;	// 0330


extern int ip_bat_test;
extern	int ip_bat_test_time, ip_bat_test_stop;
extern float ip_bat_test_V;

int	ip_changed_flag = 0;
extern int	management_id_flag;
extern u8	saved_management_id[];
extern u32	management_id;
extern u8	psu_mac[];


//int	relay_mode = RELAY_SAMSUNG ;
//u16	relay_alarm[6] ;
//byte	rx_frame_buffer[256];
unsigned LED_blink_time = 50;	// 0.5sec

uint8_t	HW_VERSION[20] ;
uint8_t	FW_VERSION[20] ;
uint8_t	SERIAL_NO[20]  ;

#define SKT48V_SN	"YY12345678"



int		hw_version = 0x01000000;
int		fw_version = 0x01000000;	// 48V 1.0.0


//byte 	USART_line_buffer[256];
//char	rx_array[40];

// average measurement values
float	batA[5] = {0.0, 0.0, 0.0, 0.0, 0.0},
		dcA[5] =  {0.0, 0.0, 0.0, 0.0, 0.0},
		dcV[20] =  {0.0, 0.0, 0.0, 0.0, 0.0};

#define MAX_VREF 32
int vref[MAX_VREF];
int vref_ind = 0, vref_flag = 0, vref_sum = 0;

float dsc_current;
int dsc_flag = 0;
float	batS = 0.0, dcS = 0.0, dcvS = 0.0;
unsigned long long	auto_test_time;
extern int shutdown_flag;
extern unsigned long long  shutdown_time, restart_time;

extern byte psu_ip[];
extern int yooha_debug;
extern byte saved_ip[],dipsw_ip[];

int bat_relay_on = 0;

#define BUF_SIZE  (OFFSET_CHECKSUM_ADDR - EEPROM_START_ADDR + 4)

uint8_t	buf[BUF_SIZE];

typedef	struct {
	float	avg;
	float	sum;
	u16	count;
} AVG;

AVG		aiv, aia, aoa, aov, aba, abv;
AVG		abt, art;
AVG		bca;

typedef struct {
	int mode;
	float	out;
	int	state;
	unsigned long long timer;
} RECOVERY;

RECOVERY	recover;
static uint8_t 	bat_sw_state = 0;
//static uint8_t ovp_clear_flag = 0;
u16	relay_alarm[4];
unsigned long long ovp_clear_time ;

pFunc Jump_To_Application;
 uint32_t	 JumpAddress;

 int shutdown_flag = 0;
 int restart_flag = 0;
 uint16_t	recover_vout = 0;

 int ACV_value = 100, acv_flag = 0, acv_count = 0;

int 	bat_cell_fail = 0;
unsigned long 	bat_cell_fail_time = 0;
u32	bat_test_begin_time, bat_test_end_time;
u32	bat_test_time = 0x7fffffff;
int forced_bat_test = 0;
uint16_t check_status_flag = 0;
//uint16_t alarm_mode = 0;	// 0 = PORT alarm, 1 = Eth alarm




PORT_STS	port_sts[16] = {

		{ NULL, },

	};

ALARM_TIMER	atimer[16];

uint16_t	port_status = 0;
uint16_t	out_status = 0;
uint16_t dac_value[4];


void display_status()
{
	printf("\nIN :0x%04x ", port_status);
	printf("OUT :0x%04x\n", out_status);

	printf("REFADC : %d\n", adv.dca);

	//printf("REFADC : %d %d %d %d\n", adv.dcv, adv.dca, adv.batv, adv.bata);
	//printf("ADC (RT BT refV) : %d %d %d %d\n", adv.rect, adv.batt, adv.refv, adv.dacv);

	//printf("DAC :%d, %d\n", dac_value[0], dac_value[1]);

}

void set_output_status()
{
	int i ;
	IO_STS *port;

	port = outputs;
	for ( i = 0; i < 8; i++) {
		if ( port->group == 0 ) {
			break;
		}
		if ( out_status & (0x01<<i)) {
			GPIO_SetBits(port->group, port->pin);
		}
		else GPIO_ResetBits(port->group, port->pin);
		port++;
	}
}

uint16_t read_alarm_mode()
{
	uint16_t jumper;
	
	jumper = check_alarm_jumper();
	if ( jumper == 0 ) {
		jumper = (uint16_t)	(eeprom_read(ALARM_MODE_ADDR) & 0xff);
		if (jumper != 1 ) jumper = 0;
	}
	
	return jumper;	
}


void  write_alarm_mode()
{
	eeprom_write(ALARM_MODE_ADDR, (uint8_t) op_sts.alarm_mode & 0xff);
}

void set_alarm_port(int mode)
{
	if ( (mode & 0x01) == 0  ) {
		GPIO_WriteBit(GPIO_ETH_SEL, ETH_SEL_PIN, (BitAction) 1);
		printf("ALARM PORT : CONTACT\n");
	}
	else {
		GPIO_WriteBit(GPIO_ETH_SEL, ETH_SEL_PIN, (BitAction) 0);
		printf("ALARM PORT : ETHERNET\n");
	}
}


void save_alarm_mode(int mode)
{
	if ( mode == 0  ) {
		op_sts.alarm_mode  = 0;
		
	}
	else {
		op_sts.alarm_mode  = 1;
	}

	set_alarm_port(op_sts.alarm_mode);
	write_alarm_mode();
}	

void init_alarm_mode()
{
	op_sts.alarm_mode = read_alarm_mode();

	set_alarm_port(op_sts.alarm_mode);	
}

 void	 check_system_restart()
 {
	 if ( restart_flag == 1  && restart_time < time10ms ) {
		 NVIC_SystemReset();
	 }
 
 
	 if ( shutdown_flag == 1  && shutdown_time < time10ms ) {
		 system_shutdown();
	 }
	 
 }
 
 void set_system_restart(int time){
 
	 printf("Restart!\n");
 
	 restart_flag = 1;
	 restart_time = time10ms + time;
 
	 //NVIC_SystemReset();	 // software boot function in CM3.h
 
 }
 
 void set_system_shutdown(){
 
	 printf("ShutDown!\n");
 
	 shutdown_flag = 1;
	 shutdown_time = time10ms + 1000;	 // After  10sec, shutdown the system
 }
 
 
void	 system_shutdown()
{
	int count = 0;
	
	printf("Going Shutdown!\n");
	//GPIO_SetBits(GPIO_BAT_CUTOFF, BAT_CUTOFF);
	set_shutdown_log(LOG_SHUTDOWN, 0.0, 0.0);
	write_shutdown_log(&log_sts);
	Delay_ms(100);
	#if 0
	while ( count++ < 35 ) {	 // waiting shutdown for 10secs

	 rect_on(0);	 // shutdown
	 IWDG_ReloadCounter();
	 Delay_ms(100);
	}
	#endif
	 
	 NVIC_SystemReset(); // if shutdown fails, system restart
}

void	relay_on(u16	no)
{
	//GPIO_SetBits(GPIO_RELAY, RELAY1 << (no -1));
	switch (no) {
		case 1:
			GPIO_SetBits(GPIO_RELAY1, RELAY1_PIN);
			break;
		case 2:
			GPIO_SetBits(GPIO_RELAY2, RELAY2_PIN);
			break;
		case 3:
			GPIO_SetBits(GPIO_RELAY3, RELAY3_PIN);
			break;
		case 4:
			GPIO_SetBits(GPIO_RELAY4, RELAY4_PIN);
			break;
		case 5:
			GPIO_SetBits(GPIO_RELAY5, RELAY5_PIN);
			break;
		case 6:
			GPIO_SetBits(GPIO_RELAY6, RELAY6_PIN);
			break;
		default:
			printf("INVALUD RELAY NO= %d\n",no); 
			break;
	}
	//printf("Relay on: %d\n", no);
}

void	relay_off(u16	no)
{
	//GPIO_ResetBits(GPIO_RELAY, RELAY1 << (no -1));
	switch (no) {
		case 1:
			GPIO_ResetBits(GPIO_RELAY1, RELAY1_PIN);
			break;
		case 2:
			GPIO_ResetBits(GPIO_RELAY2, RELAY2_PIN);
			break;

		case 3:
			GPIO_ResetBits(GPIO_RELAY3, RELAY3_PIN);
			break;
		case 4:
			GPIO_ResetBits(GPIO_RELAY4, RELAY4_PIN);
			break;
		case 5:
			GPIO_ResetBits(GPIO_RELAY5, RELAY5_PIN);
			break;
		case 6:
			GPIO_ResetBits(GPIO_RELAY6, RELAY6_PIN);
			break;		
		default:
			printf("INVALUD RELAY NO= %d\n",no); 
			break;
	}

}

void	relay_set_off()
{
	relay_off(1);
	relay_off(2);
	relay_off(3);
	relay_off(4);
}

void	relay_set_alarm(u16 alarm)
{

	if ( op_sts.bat_equip ) alarm &= ~ALARM_BAT_DISCONNECT;

	if ( alarm & relay_alarm[0] ) {
		relay_on(1);
	}
	else relay_off(1);

	if ( alarm & relay_alarm[1] ) {
		relay_on(2);
	}
	else relay_off(2);

	if ( alarm & relay_alarm[2] ) {
		relay_on(3);
	}
	else relay_off(3);

	if ( alarm & relay_alarm[3] ) {
		relay_on(4);
	}
	else relay_off(4);

}



void	init_relay_mode()
{
	//op_sts.relay_contact = relay_read_contact();

	//printf("RELAY MODE (SLN SK = 012 4) %d, ", relay_mode);
	relay_set_off();

	relay_alarm[0] = ALARM_INPUT_HIGH|ALARM_INPUT_LOW;;
	relay_alarm[1] = ALARM_OUTPUT_HIGH | ALARM_OUTPUT_LOW;
	relay_alarm[2] = ALARM_REC_HIGH | ALARM_BAT_HIGH;
	relay_alarm[3] = ALARM_BAT_LOW | ALARM_BAT_CELL_FAIL | ALARM_BAT_DISCONNECT;	
}

#if 0
void	LED_run(int mode)
{

	if ( !mode) {
		#if 0
		//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Pin = LED_RUN;  // USART1 RX, BAT_SW
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		#endif
		GPIO_SetBits(GPIO_LED_RUN , LED_RUN);
	}
	else {
		#if 0
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Pin = LED_RUN;  
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		#endif
		GPIO_ResetBits(GPIO_LED_RUN , LED_RUN);
	}
}

void LED_power()
{
	//static int on = 0;
	GPIO_ToggleBits(GPIO_LED_FAIL , LED_FAIL);
}
	
void	LED_fail(int mode)
{

	if ( !mode)  {

		GPIO_SetBits(GPIO_LED_FAIL , LED_FAIL);
	}
	else {
		GPIO_ResetBits(GPIO_LED_FAIL , LED_FAIL);
	}
}


void	LED_run_blink()
{
	static uint16_t toggle = 0;

	LED_run(toggle);
	toggle = 1 - toggle;

	//GPIO_WriteBit(GPIO_LED_RUN, LED_RUN, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIO_LED_RUN, LED_RUN)));
}
void	LED_fail_blink()
{
	static uint16_t toggle = 0;

	LED_fail(toggle);
	toggle = 1 - toggle;

	//GPIO_WriteBit(GPIO_LED_FAIL, LED_FAIL, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIO_LED_FAIL, LED_FAIL)));
}


void	LED_alarm(u16 alarm)
{
	if ( op_sts.bat_equip ) alarm &= ~(ALARM_BAT_CELL_FAIL | ALARM_BAT_DISCONNECT );

	// 20140225
	if ( ovp_lock ) {
	//if ( alarm_sts.power_cut ) {
		if ( !(time10ms % 60) ) {
			LED_fail_blink();
		}
		LED_run(0);
		return;
	}
	// end
	alarm = ( !alarm) ? 0 : 1;
	LED_run( 1 - alarm );
	LED_fail(alarm);	
} 
#endif


void	rect_on(int mode)
{	
	printf("RectOn : %d\n", mode);

	rect_on_flag = mode;
	if ( mode == 0 ) {
		DAC_WriteCh(1,0);
		GPIO_ResetBits(GPIO_RECT_ON , RECT_ON);
	}
	else if ( !ocp_lock ) GPIO_SetBits(GPIO_RECT_ON , RECT_ON);
}

void	set_default_eeprom()
{
	int i ;
	for ( i = EEPROM_START_ADDR; i < EEPROM_LAST_ADDR; i++) {
		eeprom_write(i, 0);
		if ( eeprom_read(0) == 0 ) {
			printf("waddr %x \n ", i);
			break;
		}
	}
	alarm_set_default();
}

int eeprom_wr_test()
{
	uint32_t i;
	uint8_t rd, bk;

	printf("EEPROM WR TEST...\n");

	for ( i = 0; i < 0x4000; i++) {
		bk = eeprom_read(i);
		eeprom_write(i, (uint8_t)( i & 0xff) );
		rd = eeprom_read(i);
		eeprom_write(i, bk);
		printf("EEPROM WR : %x, %x\n", i, rd);
		#if 0
		if ( rd != (i & 0xff)){
			printf("EEPROM WR : %x, %x\n", i, rd);
			//return 0;
		}
		#endif
	}
	printf("EEPROM WR OK\n");

	return 1;
}


int	eeprom_test()
{
	byte data, rd;
	int	addr;

	eeprom_OK = 1;	// for test


	addr = get_time_counter() % 1024;	// Access random address 
	//addr = addr * 2; //+ 2048;	

	addr = EEPROM_END + 2;

	data = eeprom_read(addr);
	eeprom_write(addr, (byte) (addr & 0xff));	
	rd = eeprom_read(addr);	
	eeprom_write(addr, data);
	printf("Write = %x Read= 0x%02x",(byte) (addr & 0xff), rd );
	if (  rd == (byte) (addr & 0xff) ) {
		#if 0
		eeprom_write(0x3fff, 0x5a);		// 0x3fff = the last address of FRAM2
		rd = eeprom_read(0x3fff);	
		if ( rd == 0x5a) return 1;
		#endif
		return 1;
	}
	eeprom_OK = 0;
	return 0;

}


int	eeprom_erase()
{
	int i ;
	
	for (i = MEMO_INFO_ADDR; i <= EEPROM_END; i++) {
		eeprom_write(i, 0);	
		if ( eeprom_read(i) != 0 )  {
			printf("WRITE ERROR : %x@%x \n", eeprom_read(i), i);
			return 0;
		}
		IWDG_ReloadCounter();
	}

	return 1;
}


int	offset_clear()
{
	int i;

	printf("Clear offset:%x -> %x, %x\n",EEPROM_START_ADDR, OFFSET_CHECKSUM_ADDR,  OFFSET_BACKUP_ADDR);

	memset(buf, 0, BUF_SIZE);
	eeprom_write_buffer(EEPROM_START_ADDR, buf, BUF_SIZE);
	for ( i = EEPROM_START_ADDR; i <= OFFSET_CHECKSUM_ADDR; i++) {
		//eeprom_write(i, 0);	
		#if 1
		if ( eeprom_read(i) != 0 )  {
			printf("WRITE ERROR : %x@%x \n", eeprom_read(i), i);
			break;
		}
		#endif		
	}	
	eeprom_write_buffer( OFFSET_BACKUP_ADDR, buf, BUF_SIZE);
    return 0;
}

int	offset_verify()
{
	int	i;
	byte data;
	word checksum, c1, c2, c3;

	if ( eeprom_read(OFFSET_IA_ADDR) == 0xff )  {
		offset_clear();
		return 0;
	}
	//offset_clear();
	
	checksum = eeprom_read(OFFSET_CHECKSUM_ADDR);
	c1 = 0;
	for ( i = EEPROM_START_ADDR; i < OFFSET_CHECKSUM_ADDR; i++) {
		data = eeprom_read(i);
		c1 += data;
	}
	c1 &= 0xff;

	c2 = 0;
	//printf("S2:");
	for ( i = OFFSET_BACKUP_ADDR; i < (OFFSET_CHECKSUM_ADDR- EEPROM_START_ADDR)+OFFSET_BACKUP_ADDR; i++) {
		data = eeprom_read(i);
		c2 += data;
	}
	c2 &= 0xff;
	c3 = eeprom_read((OFFSET_CHECKSUM_ADDR- EEPROM_START_ADDR)+ OFFSET_BACKUP_ADDR);
	 printf("\nOFFSET Checksum : %x, c1 = %x, c2 = %x, c3 = %x\n", checksum, c1, c2, c3);

	 if ( checksum == c1 && c1== c2 && c2 == c3) return 1;
	 if ( (checksum == 0 || checksum == 0xff) && c2 == c3) {
	 	c1 = 0;
		for ( i = 0; i < (OFFSET_BACKUP_ADDR-EEPROM_START_ADDR); i++) {
			data = eeprom_read(i+OFFSET_BACKUP_ADDR);
			eeprom_write(i+EEPROM_START_ADDR, data);	
			c1 += data;
		}
		eeprom_write(i, c2);

		c1 &= 0xff;
		printf("Checksum ERROR- BLOCK 1 %x  %x\n", c1, c2);
		return 1;
	 }

	 else if ( checksum == c1 &&  ( c2 !=c3  ||  c1 != c2) ) {

	 	c2 = 0;
		for ( i = 0; i < (OFFSET_BACKUP_ADDR-EEPROM_START_ADDR); i++) {
			data = eeprom_read(i+EEPROM_START_ADDR);
			eeprom_write(i+OFFSET_BACKUP_ADDR, data);	
			c2 += data;
		}
		eeprom_write(i+OFFSET_BACKUP_ADDR, c1);

		c2 &= 0xff;
		printf("Checksum ERROR- BLOCK 2: %x, %x\n", c1, c2);
		return 1;
	 }

	 else offset_clear();

	 return 0;

}


byte cs_back[20];
void offset_save_size(int addr, byte *data, int size)
{
	word checksum;
	int i;
	byte *p;

	p = data;
	read_eeprom(addr, cs_back, size);
	checksum = eeprom_read(OFFSET_CHECKSUM_ADDR);

	for ( i = 0; i < size; i++) {
		checksum -= cs_back[0]&0xff;
		checksum += *p & 0xff;
		p++;
	}

	checksum &= 0xff;
	write_eeprom(addr, data, size);
	eeprom_write(OFFSET_CHECKSUM_ADDR, checksum);

	write_eeprom(addr + OFFSET_BACKUP_ADDR - EEPROM_START_ADDR, data, size);
	eeprom_write((OFFSET_CHECKSUM_ADDR-EEPROM_START_ADDR) + OFFSET_BACKUP_ADDR, checksum );
}


void	offset_save(int addr, byte *data)
{
	byte	old[4];
	word checksum;

	read_eeprom(addr, old, sizeof(float));
	checksum = eeprom_read(OFFSET_CHECKSUM_ADDR);

	checksum -= old[0]&0xff;
	checksum -= old[1]&0xff;
	checksum -= old[2]&0xff;
	checksum -= old[3]&0xff;

	checksum += *data&0xff;
	checksum += *(data+1)&0xff;
	checksum += *(data+2)&0xff;
	checksum += *(data+3)&0xff;

	checksum &= 0xff;
	write_eeprom(addr, data, sizeof(float));
	eeprom_write(OFFSET_CHECKSUM_ADDR, checksum);

	write_eeprom(addr + OFFSET_BACKUP_ADDR - EEPROM_START_ADDR, data, sizeof(float));
	eeprom_write((OFFSET_CHECKSUM_ADDR-EEPROM_START_ADDR)+ OFFSET_BACKUP_ADDR, checksum );
}

void reset_pheri()
{

	I2C_reset();

	RCC_APB1PeriphResetCmd(0xffffffff, ENABLE);
	RCC_APB2PeriphResetCmd(0xffffffff, ENABLE);

	Delay_ms(50);	

	RCC_APB1PeriphResetCmd(0xffffffff, DISABLE);
	RCC_APB2PeriphResetCmd(0xffffffff, DISABLE);

	Delay_ms(100);

	NVIC_SystemReset();	// if shutdown fails, system restar
}

void	init_log()
{
	int	mode;

	printf("EEPROM start %x, end = %x\n", EEPROM_START_ADDR, EEPROM_LAST_ADDR);
	//if ( EEPROM_LAST_ADDR > 0x1fff ) printf("******* EEPROM OVERFLOW *******\n");

		if ( eeprom_test() == 0 )  {
			printf(" : EEPROM ERROR\n");
			reset_pheri();
		}
		else printf(" : EEPROM OK\n");
	mode = log_verify_eeprom();
	log_flag = mode;		// log_flag :1 = Normal, 0= format, 2: checksum Error
	log_init_control(mode);
	log_sts.alarm_status = 0;
	log_sts.log_flag = 0;

	log_sending = 0;
}


int	log_verify_eeprom()
{
	int	ret ;

	ret = log_read_header_at_reset(&log_header);

	if ( ret != 1 ) {	// invalid eepro, format eeprom
		log_format_eeprom(&log_header);
		log_read_header(&log_header);
		//set_default_eeprom();
	}

	return ret;;
}

void	log_init_control(int	mode)
{
	log_read_header(&log_header);
	log_reset();
	switch(mode) {
		case 0:		// formatted
			log_write( LOG_FORMAT, 1, 0, 0);
			break;			
		case 2:		// check sum error
			log_write( LOG_CHECKSUM_ERROR, 1, 0, 0);
			break;
		default:
			break;
	}
}


int	read_eeprom(int	addr, uint8_t *data, int size)
{
	eeprom_read_buffer( addr, data, size);
	return 0;
}

int	write_eeprom(int	addr, uint8_t *data, int size)
{
	//int a, s;
	//a = addr;
	//s = size;

	while ( size-- > 0 ) {
		eeprom_write( addr++, *data++);
		//if ( addr >= OFFSET_BACKUP_END_ADDR ) printf("WR FAULT: %x, %d", a, s);
	}
	return 0;
}



void	log_write(word code, uint8_t status, int pvalue, int cvalue)
{

	LOG_HEADER	*log;
	LOG_TYPE	*event;
	int	addr;

	log = &log_header;
	event = &log_event;

	event->code = (byte) code & 0xff;
	event->status = status;
	event->pvalue = pvalue;
	event->cvalue = cvalue;

	if ( code == LOG_BHA ) {
		event->pvalue = (int) rect_sts.cell_fail;
	}
	else if ( (code >= LOG_IHV && code <= LOG_BCF )|| (code >= LOG_BAT_TEST_START && code <= LOG_SHUTDOWN ) ||
		(code >= LOG_OVP_ON && code <= LOG_OTP_FAIL )) {
		event->acv = (short) (rect_sts.input_V * 100.0);
		event->aca = (short) (rect_sts.input_A * 100.0);
		event->dcv = (short) (rect_sts.output_V * 100.0);
		event->dca = (short) (rect_sts.output_A * 100.0);
		event->batv = (short) (rect_sts.bat_V * 100.0);
		event->bata = (short) (rect_sts.bat_A * 100.0);
		event->rect = (short) (rect_sts.rec_T * 100.0);
	}
	else {
		event->dcv = (short) (rect_sts.output_V * 100.0);
		event->dca = (short) (rect_sts.output_A * 100.0);
		event->batv = (short) (rect_sts.bat_V * 100.0);
		event->bata = (short) (rect_sts.bat_A * 100.0);
		event->rect = (short) (rect_sts.rec_T * 100.0);
	}

	IWDG_ReloadCounter();

	//printf("STS %5.2f, %5.2f\n",  event->batv, event->bata);


	log_get_time(event);	
	

	//if ( read_output_V() < 15.0 )  return;	// DON'T WRITE LOG WHEN POWER FADE OUT

	
	addr = (int) log->head * sizeof(LOG_TYPE)+LOG_START_ADDR;	

	log->id = log_id;
	log->head++;
	if ( log->head == LOG_MAX_NO ) log->head = 0;

	
	if ( log->size < LOG_MAX_NO ) log->size++;
	if ( log->head == log->tail ) {	
		log->tail++;
		if ( log->tail == LOG_MAX_NO ) log->tail = 0;	// overwrap the first record
	}

	log_set_header_checksum(log);

	//dump_memory((byte *) event, sizeof(LOG_TYPE));
	write_eeprom(addr,(byte *)event, sizeof(LOG_TYPE));
	if ( log_header_no == 0 )
			write_eeprom(LOG_HEADER_ADDR0, (byte *)log, sizeof(LOG_HEADER));
	else 	write_eeprom(LOG_HEADER_ADDR1, (byte *)log, sizeof(LOG_HEADER));

	log_header_no  = 1 - log_header_no;
	
	//read_eeprom(LOG_HEADER_ADDR, (byte *) log, sizeof(LOG_HEADER));
	//dump_memory((byte *) log, sizeof(LOG_HEADER));

}

int	log_read_header( LOG_HEADER *log)
{
	int	cs_ok;



	if ( log_header_no == 0 ) {
		read_eeprom(LOG_HEADER_ADDR0, (byte *)log, sizeof(LOG_HEADER));		
	}
	else  {
		read_eeprom(LOG_HEADER_ADDR1, (byte *)log, sizeof(LOG_HEADER));
	}
	cs_ok = log_header_checksum(log);


	if ( cs_ok  == 0 ) {
		log_header_no = 1- log_header_no;
		read_eeprom(LOG_HEADER_ADDR1, (byte *)log, sizeof(LOG_HEADER));
		cs_ok = log_header_checksum(log);
		if ( cs_ok == 0 ) {
			printf("LOG Header ERROR!!!\n");
                     //log_format_eeprom(log);   
			return 0;
		}
	}
	
	//printf("LOG Header ID = %x\n", log_header.id);
	//dump_memory((byte *)log, sizeof(LOG_HEADER));
	return 1;

}


void	log_set_header_checksum(LOG_HEADER *log)
{
	byte *data;
        int i ;
	
	log->checksum = 0;
	data = (byte *) log;
	for ( i = 0; i< sizeof(LOG_HEADER) -2; i++) {
		log->checksum += (word) (*data++ & 0xff);
	}	
}

int	log_header_checksum(LOG_HEADER *log)
{
	int 		i;
	byte		*data ;
	word	cs;

	data = (byte *) log;
	cs = 0;
	for ( i = 0; i< sizeof(LOG_HEADER)-2; i++) {
		cs += (word) (*data++ & 0xff);	
	}

	if ( cs == log->checksum) return 1;
	else return 0;

}

int	log_select_header(LOG_HEADER *log1, LOG_HEADER *log2)
{

	if ( log1->head != 0  && log1->head >= log2->head ) return 0;
	else if ( log1->head== 0 &&  log2->head > 1 ) return 0;
	return 1;

}

void	log_copy_header(LOG_HEADER *log, LOG_HEADER *log1)
{
	log->id = log1->id;
	log->head = log1->head;
	log->tail = log1->tail;
	log->size = log1->size;
	log->checksum= log1->checksum;

}

int	log_read_header_at_reset( LOG_HEADER *log)
{
	int	cs_ok, cs_ok1;
	LOG_HEADER	log1;

	
	read_eeprom(LOG_HEADER_ADDR0, (byte *)log, sizeof(LOG_HEADER));
	if ( log->id == 0xffff ){
		printf("LOG HEADER 0xffff\n");
		return 0;	// may be eeprom erased
	}
	
	cs_ok = log_header_checksum(log);
	read_eeprom(LOG_HEADER_ADDR1, (byte *) &log1, sizeof(LOG_HEADER));
	cs_ok1 = log_header_checksum(&log1);

	if ( cs_ok == 1  && cs_ok1 == 1 ) {
		log_header_no = log_select_header(log, &log1);
	}
	else if (cs_ok== 1 ) {
		log_header_no = 0;
		log_copy_header( &log1, log);
	}
	else if (cs_ok1 == 1 ) {
		log_header_no = 1;
		log_copy_header( log, &log1);
	}
	else  {
		if ( log->id != log_id ) return 2;
		printf("LOG HEADER ERROR at Reset \n");
		return 0;
	}	

	if ( log->id != log_id ) return 2;
	printf("LOG Header ID = %x\n", log_header.id);
	//dump_memory((byte *)log, sizeof(LOG_HEADER));
	return 1;

}


void	log_read_record(LOG_HEADER *log, LOG_TYPE *record, int index)
{
	index += log->tail;
	if ( index >= LOG_MAX_NO ) {
		index -= LOG_MAX_NO;
	}
	read_eeprom(LOG_START_ADDR + index*sizeof(LOG_TYPE), (byte *) record, sizeof(LOG_TYPE));
	//printf("Read record : %d \n\r", index);
	//dump_memory((byte *) record, sizeof(LOG_TYPE));


}

void	log_format_eeprom(LOG_HEADER *log)
{

	log->checksum = 0;
	log->head = 0;
	log->tail = 0;
	log->id = log_id;
	log->size = 0;

	printf("Format : Header %x\n", log->id);


	log_set_header_checksum(log);	
    log_header_no = 0;
	write_eeprom(LOG_HEADER_ADDR0, (byte *)log, sizeof(LOG_HEADER));
	write_eeprom(LOG_HEADER_ADDR1, (byte *)log, sizeof(LOG_HEADER));
	//read_eeprom(LOG_HEADER_ADDR0, (byte *) log, sizeof(LOG_HEADER));
	//dump_memory((byte *) log, sizeof(LOG_HEADER));
	//printf("Format : set default\n");
	//set_default_eeprom();
	printf("Format : Done\n");

}

void	log_get_time(LOG_TYPE *event)
{
	time_t	TIME;
	struct tm *ptim;

	TIME = RTC_GetCounter()+0x3FF36300;
	ptim = localtime(&TIME);

	event->year = (byte) (( ptim->tm_year+1900 ) - 2000);
	event->month = ptim->tm_mon+1;
	event->date = ptim->tm_mday;
	event->hour = ptim->tm_hour;
	event->min = ptim->tm_min;
	event->sec = ptim->tm_sec;	
	
	//printf("Date/Time  %02d/%02d/%4d ",ptim->tm_mday,ptim->tm_mon+1,ptim->tm_year+1900);
	//printf("%02d:%02d:%02d\n\r",ptim->tm_hour,ptim->tm_min,ptim->tm_sec);
}

void	log_reset()
{
	int code;

	printf("Log Reset:%x\n", reset_type);
	
	if ( reset_type & RESET_POWERON ) {
		code = LOG_POWER_ON_RESET;
	}
	else if ( reset_type & RESET_SWRESET ) {
		code = LOG_SYSTEM_RESET;
	}
	else if ( reset_type & RESET_IWDG ) {
		code = LOG_WATCHDOG_RESET;
	}
	else if ( reset_type & RESET_NRST ) {
		code  = LOG_NRST_RESET;		// LOG_UNKNOWN_RESET
	}
	else code = LOG_RESET_ALARM;


	log_write(code, 0, 0, 0);
	log_sts.log_flag = 1;

	
}



void write_alarm_log()
{
	static	u16		change = 0;
	u16		status;
	
	byte		code, event;
	static	u16	pstatus;
	static unsigned long		alarm_time;
	static int cell_fail = 0;

	status = get_alarm_status();

	if ( change != 0 ){
		//if ( (alarm_time + 60) > time10ms && pstatus != status ) change = 0;
		//else if ( (alarm_time + 60) < time10ms ) {
			
			status = pstatus;
			log_sts.alarm_log  = 1;		
			log_sts.alarm_status = status;		
			alarm_notification_flag = 1;

	#if 0
			if ( change & ALARM_OCP ) {
				if ( status & ALARM_OCP ) {
					if ( ocp_lock ) log_write(LOG_OCP_FAIL, EVENT_OCCUR, 0, 0);
					else log_write(LOG_OCP_ON, EVENT_OCCUR, 0, 0);		
				}
				else  log_write(LOG_OCP_OFF, EVENT_CLEAR, 0, 0);
			}
	#endif

			
			if ( change & ALARM_BAT_HIGH ) {
				code = LOG_BHT;
				if ( status & ALARM_BAT_HIGH ) 				
					event = EVENT_OCCUR;			
				else  event = EVENT_CLEAR;
				
				log_write(code, event, 0, 0);
			}
			if ( change & ALARM_REC_HIGH ) {
				code = LOG_RHT;
				if ( status & ALARM_REC_HIGH ) event = EVENT_OCCUR;
				else  event = EVENT_CLEAR;
				log_write(code, event, 0, 0);
			}
			if ( change & ALARM_AC_NFB ) {
				code = LOG_ACF;
				if ( status & ALARM_AC_NFB ) event = EVENT_OCCUR;
				else  event = EVENT_CLEAR;
				log_write(code, event, 0, 0);
			}
			if ( change & ALARM_INPUT_HIGH ) {
				code = LOG_IHV;
				if ( status & ALARM_INPUT_HIGH )  event = EVENT_OCCUR;
				else  event = EVENT_CLEAR;
				log_write(code, event, 0, 0);
			}
			if ( change & ALARM_INPUT_LOW ) {
				code = LOG_ILV;
				if ( status & ALARM_INPUT_LOW )  event = EVENT_OCCUR;
				else  event = EVENT_CLEAR;
				log_write(code, event, 0, 0);
			}
			if ( change & ALARM_OUTPUT_HIGH ) {
				code = LOG_OHV;
				if ( status & ALARM_OUTPUT_HIGH ) {
					event = EVENT_OCCUR;
					//mode = ALARM_HIGH;
				}
				else  {
					event = EVENT_CLEAR;
					//mode = 0;
					
				}
				log_write(code, event, 0, 0);				
				//write_eeprom(ALARM_OUTPUT_HIGH_ADDR,(char *) &mode, 1); //20140226
				
			}
			if ( change & ALARM_OUTPUT_LOW ) {
				code = LOG_OLV;
				if ( status & ALARM_OUTPUT_LOW )  event = EVENT_OCCUR;
				else  event = EVENT_CLEAR;
				log_write(code, event, 0, 0);
			}
			if ( change & ALARM_BAT_LOW) {
				code = LOG_BLV;
				if ( status & ALARM_BAT_LOW) event = EVENT_OCCUR;
				else  event = EVENT_CLEAR;
				log_write(code, event, 0, 0);
			}		
			if ( change & (ALARM_BAT_DISCONNECT ) ) {
				code = LOG_BDC;
				if ( status & ALARM_BAT_DISCONNECT ) event = EVENT_OCCUR;
				else  event = EVENT_CLEAR;
				log_write(code, event, 0, 0);
				if ( op_sts.bat_equip ) alarm_notification_flag = 0;
			}
			#if 0
			if ( change & ALARM_BAT_NFB) {
				code = LOG_BCF;
				if ( status & ALARM_BAT_NFB ) event = EVENT_OCCUR;
				else  {
					event = EVENT_CLEAR;
					rect_sts.bat_disconnect = 0;
				}
				log_write(code, event, 0, 0);
			}
			#endif

			if ( change & ALARM_OVP ) {
				if ( status & ALARM_OVP ) log_write(LOG_OVP_ON, EVENT_OCCUR, 0, 0);		
				else  log_write(LOG_OVP_OFF, EVENT_CLEAR, 0, 0);
			}

			if ( change & ALARM_OTP ) {
				if ( status & ALARM_OTP ) log_write(LOG_OTP_ON, EVENT_OCCUR, 0, 0); 	
				else  log_write(LOG_OTP_OFF, EVENT_CLEAR, 0, 0);
			}
#if 0
			if ( change & ALARM_BAT_CELL_FAIL ) {
				code = LOG_BHA;
				if ( status & ALARM_BAT_CELL_FAIL  ) event = EVENT_OCCUR;
				else  event = EVENT_CLEAR;
				log_write(code, event, 0, 0);
			}
#endif			
			#if 0 // def SKT_27V

			if ( change & ALARM_BQL ) {
				code = LOG_BQL;
				if ( status & ALARM_BQL  )  {
					event = EVENT_OCCUR;
					mode = 1;
				}
				else  {
					event = EVENT_CLEAR;
					mode = 0;
				}
				log_write(code, event, 0, 0);
				write_eeprom(ALARM_BQL_ADDR,(char *) &mode, 1); //20140226
			}

			
			if ( change & ALARM_OUTPUT_A) {
				if ( status & ALARM_OUTPUT_A  ) event = EVENT_OCCUR;
				else  event = EVENT_CLEAR;
				#if 0
				code = LOG_BLA;
				log_write(code, event, 0, 0);
				#endif
			}
			#endif
			// 20140225
			if ( change & ALARM_MODULE_FAIL ) {
				if ( status & ALARM_MODULE_FAIL  )  {
					event = EVENT_OCCUR;
					//shutdown_flag = 1;
					//shutdown_time = time10ms + 1000;	// 10sec to sutdown
					//mode = 1;
				}
				else   {
					event = EVENT_CLEAR;
					//shutdown_flag = 0;
					//mode = 0;
				}
				#if 1
				code = LOG_BLA;
				log_write(code, event, 0, 0);
				#endif
				//write_eeprom(ALARM_MODULE_FAIL_ADDR,(char *) &mode, 1);
			}
			// end
			change = 0;
		//}
	}
	
	else if ( log_sts.alarm_status != status ) {
		change = log_sts.alarm_status ^ status;
		pstatus = status;
		alarm_time = time10ms;
	}
	//else log_sts.alarm_log  = 0;

	if ( cell_fail != rect_sts.cell_fail ) {
		if ( rect_sts.cell_fail != BAT_STS_TEST_FAIL) {
			code = LOG_BHA;
			if ( rect_sts.cell_fail & ( BAT_STS_CHG | BAT_STS_DCHG | BAT_STS_CHANGE)) event = EVENT_OCCUR;
			else  event = EVENT_CLEAR;
			log_write(code, event, 0, 0);
		}
	}
	cell_fail = rect_sts.cell_fail;

}


void	write_alarm_set_log(LOG_STATUS *log)
{
	log_write(log->alarm_set_log, log->alarm_status, (int) log->pvalue, (int)log->cvalue );
	log->alarm_set_log  = 0;
}

void	write_ad_set_log(LOG_STATUS  *log)
{
	log_write(log->ad_set_log, log->alarm_status, (int)log->pvalue, (int)log->cvalue );
	log->ad_set_log = 0;
}

void write_time_set_log(LOG_STATUS  *log)
{
	log_write(LOG_TIME_SET , log->alarm_status, (int)log->pvalue, (int)log->cvalue );
	log->time_set_log = 0;
}

void write_operation_log(LOG_STATUS  *log)
{
	if ( log->operation_log == LOG_ALARM_MODE ) {
		log_write(log->operation_log, log->sts, (int)log->pvalue, (int)log->cvalue );
	}
	else log_write(log->operation_log, log->alarm_status, (int)log->pvalue, (int)log->cvalue );
	log->operation_log = 0;
}


void	write_bat_test_log(LOG_STATUS  *log)
{
	log_write(log->bat_test_log, log->alarm_status, (int)log->pvalue, (int)log->cvalue );
	log->bat_test_log = 0;
}

void	write_shutdown_log(LOG_STATUS  *log)
{
	log_write(log->shutdown_log, log->alarm_status,(int) log->pvalue, (int)log->cvalue );
	log->shutdown_log = 0;
}

void	write_log()
{
	LOG_STATUS	*log;

	log = &log_sts;

	if ( log->log_flag ) {		
		if ( log->alarm_set_log) write_alarm_set_log( log);
		if ( log->ad_set_log) write_ad_set_log(log);
		if ( log->time_set_log) write_time_set_log(log);
		if ( log->operation_log) write_operation_log(log);	
		if ( log->bat_test_log) write_bat_test_log(log);	
		if ( log->shutdown_log) write_shutdown_log(log);
		//log->log_flag = 0;
		//send_log_no();
	}
	write_alarm_log();
	if ( log->alarm_log || log->log_flag )  {
		send_log_no();
		log->alarm_log = 0;
		log->log_flag = 0;
	}

}

#if 1
float Thr[] = {346941, 172707, 90168, 49165, 27892, 16406, 9974, 6250, 4026, 2660, 1800, 1244, 877, 623, 460, 341, 257, 196};

float Thb[] = { 95772, 55803, 33684, 21003, 13490, 8904, 6025, 4172, 2950, 2028, 1561, 1165, 883, 678, 528, 384, 304};
#endif


void	ACV_Write_Offset(float value)	// mV
{

	uint32_t	offset;

	offset = dec2bin( value);

	CS5463_Write(0, ECS_VOLTAGE_OFFSET, offset );

	//printf("Offset %f, %x \n\r", value, offset);

}

void	ACA_Write_Offset(float value)	// mA
{

	uint32_t	offset;

	offset = dec2bin( value);

	CS5463_Write(0, ECS_CURRENT_OFFSET, offset );

	//printf("Offset %f, %x \n\r", value, offset);

}

float	ACV_Read()
{
	u32 acv;
	float	acv_f, div;

	if ( acv_flag ) return 0.0;

	acv = CS5463_Read(0, ECS_RMS_VOLTAGE );
    ACV_value = acv;
	acv_f = (float ) (acv * ( 20.0 / 4.0 * 84.333));	 //  30ohm->15ohm, 20:1-> 10:1, 20181211
	
	div = ( float ) 0x1000000;
	acv_f =  acv_f / div;

	//acv_f = 1.14*acv_f  -  26.8;	// 167: 170 ~ 281:270


#if 1
	if ( acv_f < 195.0 ) acv_f = 1.025*acv_f  -  9.55;	
	else if ( acv_f < 220.0 ) acv_f = 1.046*acv_f  -  13.92;
	else if ( acv_f < 245.0 ) acv_f = 1.098*acv_f  -  25.42;
	else acv_f = 1.22*acv_f  -  55.365;
#endif


	//printf("ACV : %d, %f\n\r", acv, acv_f);

	return acv_f;
}

float	ACA_Read()
{
	u32 aca;
	double	acv_f;

	if ( acv_flag ) return 0.0;
	
	aca = CS5463_Read(0, ECS_RMS_CURRENT );
	acv_f = (double) (  aca * ( 1000 / 4 /30));	// 2000:1
	acv_f = acv_f  / 0x1000000;
#if 0
	acv_f = acv_f *1.3668 + 0.02793;
	acv_f = acv_f * 1.776+ 0.063;
#endif

	acv_f = acv_f * 2.427+ 0.5;


	return (float) acv_f;
}


float	ACT_Read()
{
	u32 aca;
	float	act;
	
	aca = CS5463_Read(0, ECS_TEMP );
	aca = aca >> 10;
	act = (float)   aca / 64.0  ;	// 2000:1

	printf("ACT : 0x%x, %f\n", aca, act);

	//printf("ACT Epsilon: 0x%x\n", aca);
	if ( aca != ECS_EPSILON_VALUE )printf("### ACT READ ERROR!!!\n");
	return act;
}


#define ECS_RESET_START 0
#define ECS_RESET_END	1
#define ECS_SYNC		2
#define ECS_CHECK		3

uint32_t test_cs5463()
{
	uint32_t ret; 
	
	ret = CS5463_Read(0, ECS_EPSILON);
	printf("CS5463 Epsilon: 0x%x\n", ret);

	return ret;
}

uint32_t	check_CS5463()
{

	static uint8_t state = 0;
#if 0
	u32 aca, sts;

	aca = CS5463_Read(0, ECS_TEMP);
	printf("ACT Epsilon: 0x%x\n", aca);

	sts = CS5463_Read(0, ECS_STATUS);
	printf("RESET CS5463 : 0x%x\n", sts);
	

#endif

	if ( acv_flag )  {
		state = ECS_RESET_START;
		return 0;
	}

#if 0
	switch (state ) {
		case ECS_RESET_START:
			CS5463_RESET_LOW;
			state = ECS_RESET_END;
			break;
		case ECS_RESET_END:
			CS5463_RESET_HIGH;
			state = ECS_SYNC;
			break;
		case ECS_SYNC:
			CS5463_Sync();
			state = ECS_CHECK;
			CS5463_Command(0xE8);
			break;
		case ECS_CHECK:
			printf("REC CS5463 : %5.2f\n", rect_sts.input_V);
			if ( ACV_value != 0 ) {
				acv_flag = 1;
				acv_count = 0;
			}
			state = ECS_RESET_START;
			break;

		default:
			break;

	}
#endif
	#if 1
	if ( ACV_value == 0 ) {
		SPI_Configuration();		
		CS5463_init();
		printf("REC CS5463 : %5.2f\n", rect_sts.input_V);
		acv_flag = 1;
		acv_count = 0;
		//Delay_ms(500);
		//sr_on();
		return 0;
	}
	#endif
	return 1;
}

uint32_t	check_CS5463_init()
{

#if 0
	u32 aca, sts;

	aca = CS5463_Read(0, ECS_TEMP);
	printf("ACT Epsilon: 0x%x\n", aca);

	sts = CS5463_Read(0, ECS_STATUS);
	printf("RESET CS5463 : 0x%x\n", sts);
	

#endif


	if ( ACV_value == 0 ) {
		SPI_Configuration();		
		CS5463_init();
		printf("REC CS5463 INIT : %5.2f\n", rect_sts.input_V);
		acv_flag = 1;
		acv_count = 0;
		//Delay_ms(200);
		//sr_on();
		return 0;
	}

	return 1;
}


float Ep_Read()
{
	u32 acv;
	float	acv_f, div;

	acv = CS5463_Read(0, 13 );
	acv_f = (float ) acv;
	div = ( float ) 0x800000;
	acv_f =  acv_f * 1000.0 / div;
	
	//kprintf(("Ep: %x, %f\n\r", acv, acv_f));
	//return ( 123.45);
	return acv_f;

}

float	BCV_Read()
{

	float	dcv;

	//v = ADC_ReadCh(14);
	if ( adv.batv > adv.dcv ) adv.batv = adv.dcv;
	dcv = (float) (adv.batv / 10.0);

	dcv = dcv * Vref / 4095.0;

	dcv = 22.8884 * dcv + 0.1554;


	if (dcv < 10.0 ) dcv = 0.0;

	//dcv = 0.987 * dcv + 0.37;

	//printf("DCV : %5.3f\n\r",  dcv);
	//return (123.45);
	return dcv;
	
}


float read_dcv_value()
{
	float	dcv;

	dcv = (float) (adv.dcv / 10.0);
	//dcv =  DCV_COEF * ( (float) v * Vref / 4095.0/10.0)-0.05; // 10.8 = 27V / 2.5V
	dcv = dcv * Vref / 4095.0/10.0;
	dcv = 22.8884 * dcv + 0.1554;

	return dcv + rect_sts.ov_offset;	

}

void read_dcv()
{

	float	dcv;


	//GPIO_SetBits(GPIOB , GPIO_Pin_10);
	//v = ADC_ReadCh(10);
		
	if (Vref < 2.5 ) return;
			
		dcv = (float) (adv.dcv / 10.0) * Vref / 4095.0;
		
		dcv = 22.8884 * dcv + 0.1554;

		DCV_value = dcv;
		//printf("DCV %5.3f %5.3f %5.3f %5.3f\n", dcv, dcv+rect_sts.ov_offset, Vref, rect_sts.ov_offset);

}

float	DCV_Read()
{

	read_dcv();

	return DCV_value;

}

// ACS712 20A coffeficient


//#if 1 // defined (SKT) || defined(SKT_27V)
#if 0
float	sT_a[] = {0.0169, 0.0083, -0.0163};
float	sT_b[] = {39.93, 40.02, 40.92 };

float	oT_a[] = {-0.139, -0.1, -0.05};
float	oT_b[] = {2517.424, 2502.5, 2498.25 };
#endif

void	LDA_Calibration()
{
        
    rect_sts.oa_offset = -1 * LDA_Read();
}

float	LDA_Read()
{
	float	v;


    v =  (float) ( adv.dca /10.0) * Vref / 4095.0  ;	// ACS output voltage 0A = 2.5V


	v = v - 1.25 ;  // 0(A) currnt at 2.5V
	v = LDA_COEF * ( (v * 1000 ) / 20.0) ;

	v = 1.0294*v - 0.075;

	//printf("LCD : %5.2F %5.2F\n",adv.dca, v);
	
	return v;
}

#if 0

#define MAX_BATA	10
float bat[MAX_BATA];
int ba_ind = 0;
int ba_flag = 0;
float	ba_sum = 0;

float BAT_CA()
{
	float	vm;
	float	ba;

	vm = (float) ADC_ReadCh(12);
	//printf("BAT_CA : %5.2F\n", vm);
	vm =  vm * Vref / 4095.0  ;	// ACS output voltage 0A = 2.5V

	//vm = vm - 2.5 ;  // 0(A) currnt at 2.5V	
    vm = BATA_COEF * ( ((vm - 2.5) * 1000.0) / 40.0) ;
	


	if ( ba_flag == 0 ) {
		bat[ba_ind] = vm;
		ba_sum += vm;
		ba = ba_sum / (ba_ind+1);
	}
	else {
		ba_sum -= bat[ba_ind];
		bat[ba_ind] = vm;
		ba_sum += vm;
		ba = ba_sum / MAX_BATA;
	}
	
	ba_ind++;
	if (ba_ind >= MAX_BATA ) {
		ba_flag = 1;	
		ba_ind = 0;
	}	
	
	return ba;;
}
#endif

int bat_offset;
float	BATA_Read()
{
	float	v, vm;
	float	sT, offsetT;

	
	static	int index = 0, no = 0;


	vm = (float) ( adv.bata / 10.0);
	bat_offset = (int) vm;
	v =  vm * Vref / 4095.0  ;	// ACS output voltage 0A = 2.5V

# if 0
	if ( bT  <  25  ) i = 0;
	else if ( bT < 85 ) i = 1;
	else i = 2;

	sT = sT_a[i] * bT + sT_b[i];	
	offsetT = 2500 - (oT_a[i] * bT + oT_b[i]);
#endif
	sT = 40.0;
	offsetT = 0;
	v = v - 2.5 ;  // 0(A) currnt at 2.5V
    v = BATA_COEF * ( (v * 1000.0  + offsetT) / sT) ;
	v = 1.0294*v - 0.075;
	
	batS -= batA[index];
	batA[index] = v;
	batS += v;	
	index++;	
	if ( batS != batS ) {	// check for NAN
		batS = 0.0;
	}	
	if ( !no ) v = batS /index;
	else v = batS / 5;	
	if ( index > 4 ) {
		index = 0;
		no = 1;
	}
	//printf("BAT : %5.2F\n", v);


	return v;

}

void	BATA_Calibration()
{

    rect_sts.ba_offset = -1 * BATA_Read();
	ad_offset.bat_zero_offset  = bat_offset;
}


void get_Vref()
{
	int offset = 0, i;
	
	for ( i=0; i < 5; i++) {
		offset += ADC_ReadCh(17);
		Delay(0x10);
	}
	Vref = 1.2 * 4095.0 * 5.0 / offset ;
}



float	BATT_Read()
{
	float 	value;
	float		thr, temp;
	float 	a, b;
	int 	tx,  ti;	


	//t = ADC_ReadCh(CONT_T_CH) & 0xffffff;		// control bd. temp sensor

	value = (float) (adv.batt/10.0) ;   //* 1.215;
	if ( value < 1.0) value = 1.0;
	thr = 1500.0 * 4095.0/value - 1600.0;

	for ( ti = 0; ti < 18; ti++) {
		if ( thr > Thr[ti] ) break;
	}
	tx = (ti- 6) * 10;

	if ( ti ==  0 ) {
		a  =  10  / (Thr[1] - Thr[0]);
		b = tx - Thr[0] * a;	
	}
	else {
		a  =  10  / (Thr[ti] - Thr[ti-1]);
		b = tx - Thr[ti-1] * a;	
	}
	temp = a * thr + b;
	//printf("BATT_TEMP thr = %3.2f, T  %3.2f\n\r", thr, temp);

	if ( temp < -50.0 ) temp = -100.0;
	return temp;

}



float	RET_Read()
{
	float 	value;
	float		thr, temp;
	float 	a, b;
	int 		tx,  ti;	

	//t = ADC_ReadCh(RECT_T_CH) & 0xffffff;	// external temp. sensor


	value = (float) (adv.rect / 10.0);   //* 1.215;
	if ( value < 1.0) value = 1.0;
	thr = 1500.0 * 4095.0/value - 1600.0;

	for ( ti = 0; ti < 18; ti++) {
		if ( thr > Thr[ti] ) break;
	}
	tx = (ti- 6) * 10;

	if ( ti ==  0 ) {
		a  =  10  / (Thr[1] - Thr[0]);
		b = tx - Thr[0] * a;	
	}
	else {
		a  =  10  / (Thr[ti] - Thr[ti-1]);
		b = tx - Thr[ti-1] * a;	
	}
	temp = a * thr + b;
	//printf("RECT_TEMP thr = %3.2f, T  %3.2f\n\r", thr, temp);

	if ( temp < -50.0 ) temp = -100.0;
#if 0	
	else if ( temp > 85 ) temp = temp * 1.15;
	else if ( temp > 65 ) temp = temp * 1.1;
#endif	
	return temp;
}



float	read_input_V()
{
	return ACV_Read() + rect_sts.iv_offset;

}

float	read_input_A()
{
	return ACA_Read() + rect_sts.ia_offset;

}

float	read_output_V()
{
	return DCV_Read() + rect_sts.ov_offset;

}

float	read_bat_V()
{
#if 1
	float bat_v;
	bat_v = BCV_Read(); // + rect_sts.ov_offset ;
	return bat_v + rect_sts.ov_offset;	// 180814  offsetº¸Á¤ ;
#endif	

	//return rect_sts.output_V;
}


float	read_output_A()
{
#if 0
	if ( rect_sts.bat_disconnect == 0 && dsc_flag ) {
		return  (-1 *dsc_current)- 4.4;
	}
#endif
	float oa = LDA_Read() + rect_sts.oa_offset;

	if ( rect_on_flag == 0 || oa < 0.0 ) return 0.0;
	return oa;
}

float	read_bat_A()
{
	float ba;

	ba = BATA_Read() + rect_sts.ba_offset;
	if ( ba < 0.0 ) {
		if ( ba > -4.1 ) ba = 0.0;
		else ba = 1.0869 * ba + 4.456;
	}

#if 0
	if ( ba < 0 )  {
		dsc_current = ba;
		dsc_flag = 1;
		ba = 0.0;
	}
	else dsc_flag = 0;
#endif	
	return ba;

}

float	read_bat_T()
{
	return BATT_Read() + rect_sts.bt_offset;
}

float	read_rec_T()
{
	return RET_Read() + rect_sts.rt_offset;
}



int	read_bat_NFB()
{
	return GPIO_ReadInputDataBit(GPIO_DI_BAT_SW, DI_BAT_SW);
}


int	check_range(float value, float max, float min)
{
	int ret = 0;

	if ( value >= max) ret = ALARM_HIGH;
	else if (value <= min ) ret = ALARM_LOW;
	return ret;
}

int	alarm_input_V(ALARM_STATUS *alarm)
{
	float hi_offset = 0.0, low_offset = 0.0;
	
	if ( alarm->input_V &  ALARM_HIGH ) hi_offset = -5.0;
	else if ( alarm->input_V &  ALARM_LOW ) low_offset = 5.0;
	return check_range (rect_sts.input_V, alarm->hi_input_V + hi_offset, alarm->low_input_V + low_offset  );
}

int	check_OVP_shutdown()
{
	

	if (ovp_lock ) return 0;
	
	if (  rect_sts.output_V > OUTPUT_OVP_DEF ) {		
		OVP_on_ready = 1;
	}	
	else {
		if ( ovp_flag == 1 ) {		// 2017 0620
			if (  rect_sts.output_V <= (OUTPUT_OVP_DEF - 0.5)) {
				OVP_on_ready = 0;
			}
		}
		else OVP_on_ready = 0;		// 2017 0620
	}

	//printf("OVP READY : %d\n", OVP_on_ready);
    return 0;
}


int	alarm_output_V(ALARM_STATUS *alarm)
{
	float hi_offset = 0.0, low_offset = 0.0;
	
	if ( alarm->output_V &  ALARM_HIGH ) {
		hi_offset = -0.4;
		if ( ovp_lock ) return ALARM_HIGH;
	}
	else if ( alarm->output_V &  ALARM_LOW ) low_offset = 2.0;
	return check_range (rect_sts.output_V , alarm->hi_output_V+ hi_offset, alarm->low_output_V+ low_offset  );
}

int	alarm_module_fail(ALARM_STATUS *alarm)
{
	static float t_offset = 0.0;
	float	hi_t;
	

	hi_t = RECT_TEMP_FAIL; 	// 100
	//hi_t = alarm_sts.hi_rec_T + 5;
	if ( rect_sts.rec_T  >= ( hi_t + t_offset)) {	 // 0330  || rect_sts.bat_T >= ( hi_t + t_offset)) {
		fail_on_ready = 1;
		t_offset = -12.0;	// recover from 88
	}
	else {		
		fail_on_ready = 0;
		t_offset = 0.0;
		//over_t = 0;
	}

	return ovp_lock;
}


int	alarm_output_A(ALARM_STATUS *alarm)
{
#if 0	
	float hi_offset = 0.0;

	if ( alarm->output_A ) hi_offset = -0.5;
	if ( rect_sts.output_A > ((OUTPUT_OVER_CURRENT * 1.05) + hi_offset )) return 1;
	else 
#endif			
	return 0;	
}


int	alarm_bat_V(ALARM_STATUS *alarm)
{
	float hi_offset = 0.0, low_offset = 0.0;
	
	if ( alarm->bat_V &  ALARM_HIGH ) hi_offset = -0.5;
	else if ( alarm->bat_V &  ALARM_LOW ) low_offset = 1.0;
	
	return check_range (rect_sts.bat_V , alarm->hi_bat_V+ hi_offset, alarm->low_bat_V+low_offset);
}

int	alarm_bat_T(ALARM_STATUS *alarm)
{
	float hi_offset = 0.0, low_offset = 0.0;
	
	if ( alarm->bat_T &  ALARM_HIGH ) hi_offset = -10.0;
	else if ( alarm->bat_T &  ALARM_LOW ) low_offset = 5.0;
	return check_range (rect_sts.bat_T, alarm->hi_bat_T + hi_offset, alarm->low_bat_T+low_offset);
}

int	alarm_rec_T(ALARM_STATUS *alarm)
{
	float hi_offset = 0.0, low_offset = 0.0;
	
	if ( alarm->rec_T &  ALARM_HIGH ) hi_offset = -7.0;	// 5
	else if ( alarm->rec_T &  ALARM_LOW ) low_offset = 10.0;
	
	return check_range (rect_sts.rec_T , alarm->hi_rec_T+hi_offset, alarm->low_rec_T+low_offset);
}


int	get_log_size()
{

#ifdef FLASH_LOG
	return fb_log_size;
#endif

	LOG_HEADER	*log;
	int	size;

	log = &log_header;

	size = log->head - log->tail;
	if ( log->head < log->tail ) {
		size = LOG_MAX_NO + size;		
	}

	return size;

}

void	init_avg(AVG * a)
{
	a->avg = 0.0;
	a->sum = 0.0;
	a->count = 0;
}

void	init_all_avg()
{
	init_avg(&aia);
	init_avg(&aiv);
	init_avg(&aoa);
	init_avg(&aov);
	init_avg(&aba);
	init_avg(&abt);
	init_avg(&art);
	init_avg(&bca);
}

float 	get_avg(AVG * a , float value)
{
	if ( a->count < 20 )  {	
		a->count ++;	
	}
       else {
       	 a->sum -= a->avg;	 	 
        }
       a->sum +=  value;
  	a->avg  = a->sum / (float) a->count;
	return a->avg;
}

void	get_rect_status( RECT_STATUS *sts)
{
	sts->output_V = read_output_V();

	sts->input_V = read_input_V();
	sts->input_A = read_input_A();
	sts->output_A = read_output_A();
	sts->bat_A = read_bat_A();
	sts->bat_T = read_bat_T();
	sts->rec_T = read_rec_T();


	sts->bat_V = read_bat_V();

}




uint16_t check_alarm_jumper()
{
	static uint16_t pjmp = 0;
	uint16_t jmp = 0;
	
	if ( (port_status & JUMPER_HW)== 0 ) {
		if ( (port_status & JUMPER_ETH) == 0 ) jmp = 3;	// ETH/HW
		else jmp =  2;	// RELAY/HW
	}

	if (jmp != pjmp ) {
		set_alarm_mode(jmp);
	}
	pjmp = jmp;
	return jmp;	// SW SELECT
}

uint16_t	get_port_status()
{
	return port_status;
}

uint16_t check_port_status()
{
	int i, no;
	static uint16_t pstatus = 0;
	uint16_t status = 0, change;
	PORT_STS *port;

	port = port_sts;
	for ( i = 0; i < 16; i++) {
		if ( port->group == 0 ) {
			no = i;
			break;
		}
		port->bit = GPIO_ReadInputDataBit(port->group, port->pin);
		if ( port->bit == 1 ) {
			status |= (1 << i);
		}
		port++;
	}

	change = pstatus ^ status;
	//printf("status = %x, %x %x\n",change, pstatus, status);

	port = port_sts;
	for ( i = 0; i < no; i++) {
		if ( change & ( 1 << i )) {
			port->count = 0;
			port->flag = 1;
		}
		else if ( port->flag ) {
			port->count++;
			if (  port->count > port->wait ) {
				port_status &= ~(1 << i);
				port_status |=	(pstatus & (1 << i)); 
				port->flag = 0;
			}
		}		
		port++;
	}
	pstatus = status;
	return port_status;


}

void alarm_timer_init()
{
	int i ;
	
	for ( i = 0; i < 16; i ++) {
		atimer[i].time = 0;
		atimer[i].wait = 4;
		atimer[i].flag = 0;
	}
}


u16	get_alarm_status()
{
	return system_alarm_status;

}



u16	process_alarm_status(ALARM_STATUS	*alarm)
{
	u16	status = 0,  i;
	//static int bat_nfb = 0;
	static uint16_t pstatus = 0, change = 0;

	//check_sensor();
	check_alarm(alarm);
	check_OVP_shutdown();

	
	if ( alarm->input_V == ALARM_HIGH ) status |= ALARM_INPUT_HIGH;
	else if ( alarm->input_V == ALARM_LOW ) status |= ALARM_INPUT_LOW;

	if ( alarm->output_V == ALARM_HIGH ) status |= ALARM_OUTPUT_HIGH;
	else if ( alarm->output_V == ALARM_LOW ) status |= ALARM_OUTPUT_LOW;

	if ( op_sts.bat_equip == 0 ) {
	if ( alarm->bat_V == ALARM_LOW && alarm->bat_NFB == 0 ) status |= ALARM_BAT_LOW;
	}
	//if ( alarm->bat_T == ALARM_HIGH ) status |= ALARM_BAT_HIGH;
	if ( alarm->rec_T == ALARM_HIGH ) status |= ALARM_REC_HIGH;

#if 1
	
	//if ( alarm->bat_NFB != 0 )  {
	//	status |= ALARM_BAT_NFB;
	//}
	if ( alarm->bat_disconnect != 0 ) {
		status |= ALARM_BAT_DISCONNECT;
	}
	
	if ( !op_sts.bat_equip && alarm->cell_fail != 0 ) {
		status |= ALARM_BAT_CELL_FAIL;
	}
#endif

	//if ( alarm->output_A != 0 ) status |= ALARM_OUTPUT_A;
	if ( alarm->module_fail != 0 ) status |= ALARM_MODULE_FAIL;
	if ( alarm->ovp != 0 ) status |= ALARM_OVP;
	if ( alarm->ocp != 0 ) status |= ALARM_OCP;
	if ( alarm->otp !=0 ) status |= ALARM_OTP;
	if ( alarm->power_cut !=0 ) status |= ALARM_POWER_CUT;

	change = pstatus ^ status;
	//printf("status = %x, %x %x\n",change, pstatus, status);

	for ( i = 0; i < 16; i++) {
			if ( change & ( 1 << i )) {
				atimer[i].time = 0;
				atimer[i].flag = 1;
			}
			else if (atimer[i].flag) {
				if ( atimer[i].time > atimer[i].wait  ) {
					system_alarm_status &=  ~(1 << i);
					system_alarm_status |=  (pstatus & (1 << i)); 
					atimer[i].flag = 0;
				}
				atimer[i].time++;
			}

	}
	pstatus = status;

	return system_alarm_status;

}

byte	get_op_mode()
{
	byte sts = 0;
	
	if ( op_sts.output_mode )  sts |= 0x01;
	if ( op_sts.current_mode )  sts |= 0x02;
	if ( op_sts.relay_contact )  sts |= 0x04;
	if ( op_sts.bat_test_mode )  sts |= 0x08;

	if ( op_sts.bat_equip )  sts |= 0x80;	// 20180716

	return sts;
}

float		read_value(int addr)
{
	float value;
	
	read_eeprom(addr, (u8 *) & value, sizeof(float));
	return value;
}

int	read_output_mode()
{
	return eeprom_read(OP_OUTPUT_MODE_ADDR );
}

int	read_current_limit_mode()
{
	
	return eeprom_read(OP_CURRENT_MODE_ADDR );
}

float	read_output_voltage()
{
	float	value;

	read_eeprom(OP_OUTPUT_VALUE_ADDR, (u8 *) & value, sizeof(float));


	return value;
}

float	read_current_limit()
{
	float	value;

	read_eeprom(OP_CURRENT_VALUE_ADDR, (u8 *) & value, sizeof(float));

	return value;
}


int	read_bat_test_mode()
{
	return eeprom_read(OP_BAT_TEST_MODE_ADDR );
}

float	read_bat_test_voltage()
{
	float	value;

	read_eeprom(OP_BAT_TEST_VOLTAGE_ADDR, (u8 *) & value, sizeof(float));

	if (value < 40.0 || value > 54.0 ) value = 46.0;

	return value;
}

int	read_bat_test_period()
{
	int	value;

	read_eeprom(OP_BAT_TEST_PERIOD_ADDR, (u8 *) & value, sizeof(int));

	return value;
}

void	read_next_test_time()
{
	u8	data[8];

	//printf("%s\n", __FUNCTION__);
	read_eeprom(OP_BAT_TEST_DATE_ADDR, data, 8);

	op_sts.year = data[0];
	op_sts.month = data[1];
	op_sts.date = data[2];
	
	op_sts.hour = data[4];
	op_sts.min = data[5];
	op_sts.sec = data[6];

	//======================================
	// 2.20 AM3, 5.20 AM3, 8.20 AM3, 11.20 AM3
	//======================================

	if ( op_sts.month < 1 || op_sts.month > 12 ) op_sts.month = 1;
	if ( op_sts.date < 1 || op_sts.date > 31 ) op_sts.date = 7;

	bat_set_auto_test_day();

	#if 0
	get_system_time(&sys_info);
	op_sts.year = 0;
	op_sts.date = 20;
	op_sts.hour = 3;
	op_sts.min = 0;
	op_sts.sec = 0;


	if ( sys_info.hour > 3 && sys_info.day >= 20 ) {
		if ( sys_info.month == 2 ) op_sts.month = 5;
		else if ( sys_info.month == 5 ) op_sts.month = 8;
		else if ( sys_info.month == 8 ) op_sts.month = 11;
		else if ( sys_info.month == 11 ) op_sts.month = 2;		
	}
	else {
		if ( sys_info.month <= 2 ) op_sts.month = 2;
		else if ( sys_info.month <= 5 ) op_sts.month = 5;
		else if ( sys_info.month <= 8 ) op_sts.month = 8;
		else if ( sys_info.month <= 11 ) op_sts.month = 11;
		else op_sts.month = 2;		
	}
	#endif
}

void	write_output_mode(int mode)
{
	op_sts.output_mode = mode;
	eeprom_write(OP_OUTPUT_MODE_ADDR  , (u8) mode);
}


void	set_output_mode(int mode)
{
	if ( mode == 0 ) write_output_voltage(op_sts.output_V);
	write_output_mode(mode);
	send_op_set();
}

#if 0
void	write_current_limit_mode(int mode)
{
	
	if ( mode == 0 ) DAC_WriteCh(2, 4095);
	else write_current_limit(op_sts.charge_A);	
}

void	set_current_limit_mode(int mode)
{
	word code;
	
	op_sts.current_mode = mode;
	if ( mode == 0 ) code = LOG_CL_OFF;
	else  code = LOG_CL_ON;
	set_operation_log(code, 0, 0);
	write_current_limit_mode(mode);
	eeprom_write(OP_CURRENT_MODE_ADDR  , (u8) mode);
	send_op_set();

}
#endif

void	write_output_voltage(float	ov)	// DC_REF, same to the REC_REF
{
	float		value, svalue;
    //printf(" Output vol : %5.2f, %5.2f \n", ov, rect_sts.ov_set_offset);


	ov = ov - rect_sts.ov_set_offset;

	//ov = ov  * 7 / 13;
	//svalue = 0.326 * ov -  (6.875);
	//svalue = 0.22*ov - 3.84;		// 0622
	#if 0
	if ( ov < 50 ) {
		svalue = 0.1704* ov - 6.627;
	}
	else svalue = 0.1766 * ov - 6.939;
	#endif
	svalue = 0.1677 * ov -6.4792;

	if ( svalue < 0.1 ) svalue = 0.0;
	value = svalue * 4095.0 / Vref;
	
	//printf(" Output vol %5.3f: %d, %5.3f \n", ov, (int)value, svalue);
	DAC_WriteCh(1, (int) value);
	
}

void	set_output_voltage(float	ov)	// DC_REF, same to the REC_REF
{
	op_sts.output_V = ov;
	write_output_voltage(ov);
	write_eeprom(OP_OUTPUT_VALUE_ADDR  , (byte*) &ov, sizeof(float));
	send_op_set();	
	printf("SET Output V: %5.2f\n", ov);
}

void	update_output_voltage()	// DC_REF, same to the REC_REF
{

	if (  op_sts.bat_testing || rect_sts.bat_test  ) {
		return;
	}

	if ( rect_on_flag == 0 ) {
		DAC_WriteCh(1, 0);
	}
	else

	//if ( op_sts.output_mode == 0  ) {
		write_output_voltage(op_sts.output_V);
	
		//printf("OT=%f5.3\n", op_sts.output_V);		
	//}
}

#define COF_A	1.0294
#define COF_B	0.070	//	0.075
#define COF_C	(2.5*25 * COF_A -0.15)
//#define COF_C	(2.5*25 + COF_B)

#if 0

void	write_current_limit(float	c)	// CHG_REF
{
	float		value, svalue;
 
	//svalue =  (c -0.2)* 0.04;  //+ ad_offset.ba;
	if ( c > 2.95 ) c = 4.5;

	//printf("Current Limit: %5.2f\n", c);

	#if 1 
	c = c - rect_sts.ba_offset;
	svalue = ( c + COF_C) / 25 / COF_A;
	value = svalue * 4095 / Vref;	// + ad_offset.bat_zero_offset;
	
	if ( value < ad_offset.bat_zero_offset )  value = ad_offset.bat_zero_offset;
	#endif

	//value = c * 4095 / Vref;
	DAC_WriteCh(2, (int) value);

}
#endif

void	set_alarm_mode_log(word code, uint8_t sts)
{
	//printf("OP LOG : %x, %5.2f, %5.2f\n", code, pvalue, cvalue);
	log_sts.operation_log = code;
	log_sts.sts = sts;
	log_sts.log_flag = 1;
}


void	set_alarm_set_log(word code, float pvalue, float cvalue)
{
	log_sts.alarm_set_log = code;
	log_sts.pvalue = conv(pvalue);
	log_sts.cvalue = conv(cvalue);
	log_sts.log_flag = 1;
}

void	set_ad_set_log(word code, float pvalue, float cvalue)
{
	log_sts.ad_set_log = code;
	log_sts.pvalue = conv(pvalue);
	log_sts.cvalue = conv(cvalue);
	log_sts.log_flag = 1;
}


void	set_operation_log(word code, float pvalue, float cvalue)
{
	//printf("OP LOG : %x, %5.2f, %5.2f\n", code, pvalue, cvalue);
	log_sts.operation_log = code;
	log_sts.pvalue = conv(pvalue);
	log_sts.cvalue = conv(cvalue);
	log_sts.log_flag = 1;
}

void	set_bat_test_log(word code, float pvalue, float cvalue)
{
	log_sts.bat_test_log = code;
	log_sts.pvalue = conv(pvalue);
	log_sts.cvalue = conv(cvalue);
	log_sts.log_flag = 1;
}

void	set_shutdown_log(word code, float pvalue, float cvalue)
{
	log_sts.shutdown_log = code;
	log_sts.pvalue = conv(pvalue);
	log_sts.cvalue = conv(cvalue);
	log_sts.log_flag = 1;
}


void	save_value(int addr, float value)
{
	u8	*buf;

	buf = (byte *) &value;
	write_eeprom(addr, buf, sizeof(float));
	//printf("ALARM SET : %x %5.2f", addr, value);
}



void	check_saved_alarm()
{
	uint8_t alarm[4];
#if 1	// 190115

	read_eeprom(ALARM_SAVE_ADDR, alarm, 4);

    ovp_count = (int) alarm[0];
	ovp_lock = alarm_sts.output_V = alarm[1];
	//fail_lock = alarm_sts.module_fail = alarm[2];
	fail_lock  = alarm[2];
#endif
	//ovp_count = 0;
	//ovp_lock = 0;
	fail_lock = 0;

	printf("Alarm : COUNT = %x, OVP=%x,  OTP=%x\n", ovp_count, ovp_lock, fail_lock);


}




void	ad_save_offset(int addr, float value)
{
	u8	*buf;

	buf = (byte *) &value;
	offset_save(addr, buf);
}


float	ad_read_offset(int addr)
{
	float	value;	

	read_eeprom(addr, (u8 *) &value, sizeof(float));
	if ( value == -0 ) value = 0.0;
	//printf("AD offset= %5.2f \n", value);
	return value;
}

#if 0

int	get_auto_test_mode()
{
	return op_sts.bat_test_mode;
}


void bat_test_save_mode(int 	mode)
{
	eeprom_write(OP_BAT_TEST_MODE_ADDR,  mode);
}

void bat_test_save_v(float	value)
{
	u8	*buf;

	buf = (byte *) &value;
	write_eeprom(OP_BAT_TEST_VOLTAGE_ADDR, buf, sizeof(float));
}

void bat_test_save_period( int month)
{
	//eeprom_write(OP_BAT_TEST_PERIOD_ADDR, (byte * ) &month, sizeof(int));
      write_eeprom(OP_BAT_TEST_PERIOD_ADDR, (byte * ) &month, sizeof(int));
}

void bat_test_save_time()
{
	u8	buf[8];

	buf[0]= op_sts.year ;
	buf[1]= op_sts.month ;
	buf[2]= op_sts.date ;
	buf[3] = 0;
	buf[4]= op_sts.hour ;
	buf[5]= op_sts.min ;
	buf[6]= op_sts.sec ;
	
	write_eeprom(OP_BAT_TEST_DATE_ADDR, buf, 8);
}

void bat_test_set_mode(int 	mode)
{
	op_sts.bat_test_mode = mode;
	
	if ( mode ) {
		set_bat_test_log(LOG_BAT_TEST_ON,  0, 0);
	}
	else set_bat_test_log(LOG_BAT_TEST_OFF,  0, 0);	
}

void bat_test_set_v(float	value)
{
	float	pvalue;

	pvalue = op_sts.bat_test_V;
	op_sts.bat_test_V = value;
	auto_test.v = op_sts.bat_test_V;
	
	set_bat_test_log(LOG_BAT_TEST_V_SET,  pvalue, value);

	
}

void bat_update_auto_test_day()
{
	get_system_time(&sys_info);

	auto_test.month = sys_info.month + op_sts.bat_test_period;
	if (auto_test.month > 12) {
			auto_test.month -= 12;
	}
	auto_test.day = op_sts.date;
	auto_test.hour = op_sts.hour;
	auto_test.min = op_sts.min;
	
	if ( auto_test.month == 2  && auto_test.day > 28 ) {
		auto_test.day = 28;
	}
	else if ( (auto_test.month == 4 || auto_test.month == 6 || auto_test.month == 9 || auto_test.month == 11) && auto_test.day > 30 ) {
		auto_test.day= 30;
	}


}


void bat_set_auto_test_day()
{
	get_system_time(&sys_info);

	auto_test.month = op_sts.month;
	if ( op_sts.month < sys_info.month) {
		auto_test.month += op_sts.bat_test_period;
	}
	else if ( op_sts.month ==  sys_info.month ) {
		if ( op_sts.date < sys_info.day )   {
			auto_test.month += op_sts.bat_test_period;
		}
		else if ( op_sts.date == sys_info.day ) {
			if ( op_sts.hour < sys_info.hour)  {
				auto_test.month += op_sts.bat_test_period;
			}
			else if( op_sts.hour == sys_info.hour ) {
		  		if ( op_sts.min <= sys_info.min ) {
		  			auto_test.month += op_sts.bat_test_period;
		  		}
			}
		}
	}
	
	
	if (auto_test.month > 12) {
			auto_test.month -= 12;
	}
	auto_test.day = op_sts.date;
	auto_test.hour = op_sts.hour;
	auto_test.min = op_sts.min;
	
	if ( auto_test.month == 2  && auto_test.day > 28 ) {
		auto_test.day = 28;
	}
	else if ( (auto_test.month == 4 || auto_test.month == 6 || auto_test.month == 9 || auto_test.month == 11) && op_sts.date > 30 ) {
		auto_test.day= 30;
	}

	auto_test.v = op_sts.bat_test_V;
	//printf("auto Test : %d/%d,%d:%d\n", auto_test.month, auto_test.day, auto_test.hour, auto_test.min);
}



void	bat_test_set_period( int month)
{
	int pvalue;
	
	pvalue = op_sts.bat_test_period ;
	op_sts.bat_test_period = month;
	set_bat_test_log(LOG_BAT_TEST_PERIOD,  pvalue, month);
}






u32	get_bat_test_time()	// secs
{
	bat_test_end_time = get_time_counter();
	//printf("BAT TEST end time = %d, %d\n", bat_test_end_time, bat_test_end_time - bat_test_begin_time);
	return ((bat_test_end_time - bat_test_begin_time)) ; 
}


int	bat_test_condition()
{
	if ( alarm_sts.input_V || alarm_sts.output_V || alarm_sts.bat_V || alarm_sts.bat_NFB || alarm_sts.bat_disconnect ) return 0;
	return 1;
}
#endif

void	set_flash_ip(int ip)
{
	dprintf("SET FLASH IP ADDR = %x \n", ip);	
	ip_update_flash_ip(ip);
}


int	get_door_state()
{	
	//printf("Invalid %s\n", __FUNCTION__);
	return 0;
}


int	get_AC_NFB()
{
	//printf("Invalid %s\n", __FUNCTION__);
	return 0;
	
}

#if 0
int	get_BAT_NFB()
{

#if 0
	static uint8_t 	state_flag = 0;
	static	uint8_t count = 0;	
	static unsigned long long bat_time = 0;;
	
	if ( GPIO_ReadInputDataBit(GPIO_DI_BAT_SW, DI_BAT_SW) ) {		// BAT SW OFF
		if ( bat_sw_state == 0) {		
			state_flag = 0;
			if( count++ > 3) {
				bat_sw_state = 1;
				count = 0;
				//BAT_CON_ON();
			}
		}
		else count = 0;
	}
	else {	// BAT SW ON
		if ( bat_sw_state == 1)  {
			
			if ( count++ > 3) {
				bat_sw_state =0;
				count = 0;
				state_flag = 1;
				bat_time = time10ms + 20;
			}
		}
		else count = 0;
	}	
	
#if 1
	if ( state_flag == 1 ) {
		if ( bat_time <= time10ms ) {
			//BAT_CON_OFF();
			state_flag = 0;
		}
	}	
#endif
#endif

	if ( port_status & BAT_STS_SW ) {
		bat_sw_state = 1;
	}
	else bat_sw_state = 0;

	return bat_sw_state;

}

int	get_bat_test_sw()
{
#if 0
	static int 	state = 0;
	static	int count = 0;

	//printf("check Bat Test SW ON\n");
	if ( GPIO_ReadInputDataBit(GPIO_BAT_TEST_SW, BAT_TEST_SW) ) {
		
		if ( state == 0 ) {
			if(  count++ > STATE_COUNT) {
				state =1;
				clear_alarms();
				//printf("Bat Test SW ON\n");
			}
		}
		else count = 0;
	}
	else {
		if ( state == 1 )  {
			if ( count++ > STATE_COUNT ) {
				state = 0;
				//printf("Bat Test SW OFF\n");
			}
		}
		else count = 0;
	}	
	return state;
#endif

	if ( port_status & BAT_STS_TEST ) {
		return 1;
	}
	else return 0;

}
#endif

void	clear_alarms()
{
	int mode = 0;

	printf("CL ALM:%x %x %d\n", ovp_lock, fail_lock, alarm_sts.module_fail);

	if ( ovp_lock || fail_lock ) {
		system_shutdown_flag = 0;
		alarm_sts.output_V = 0;
		alarm_sts.module_fail = 0;
		fail_lock = 0;
		ovp_lock = 0;
		ovp_count = 0;
		ovp_flag = 0;
		fail_on_ready = 0;
		//bat_cutoff_on(1);
		//write_eeprom(ALARM_SAVE_ADDR,(char *) &mode, sizeof(int));

		recovery_start(RECOVER_OVP_RESET);
		recover.timer = time10ms + 50;
	}
	write_eeprom(ALARM_SAVE_ADDR,(uint8_t *) &mode, 4);
	ovp_count = 0;
}


void	bat_test_sw_start()
{
	clear_alarms();	// 20140225
	printf("Clear Module fail\n");

}


void check_bat_sts()
{
	//op_sts.bat_sts = (uint8_t)  (port_status & 0xff);
	static  uint16_t btn_save = BTN_ALARM_CLEAR;;

#if 0	// Enable BAT SW


	if ( port_status & BAT_STS_SW ) {
		//printf("BAT SW ON\n");
		rect_sts.bat_disconnect = 0;
	}
	else  {
		rect_sts.bat_disconnect = 1;
	}
	
#endif


	//rect_sts.cell_fail |= port_status & (BAT_STS_CHG|BAT_STS_DCHG);
	if ( port_status & BAT_STS_CHG) {
		rect_sts.cell_fail |= BAT_STS_CHG;
	}
	else rect_sts.cell_fail &= ~BAT_STS_CHG;
	
	if ( port_status & BAT_STS_DCHG) {
		rect_sts.cell_fail |= BAT_STS_DCHG;
	}
	else rect_sts.cell_fail &= ~BAT_STS_DCHG;

	if ( !(port_status & BAT_STS_CHANGE)) {
		rect_sts.cell_fail |= BAT_STS_CHANGE;
	}
	else rect_sts.cell_fail &= ~BAT_STS_CHANGE;

		
	if ( (port_status & (BAT_STS_CHG | BAT_STS_DCHG | BAT_STS_CHANGE)) == (BAT_STS_CHG | BAT_STS_DCHG | BAT_STS_CHANGE)) {
		rect_sts.bat_disconnect = 1;
		rect_sts.cell_fail = BAT_STS_CHG | BAT_STS_DCHG ;
	}
	else rect_sts.bat_disconnect = 0;

	op_sts.bat_sts = (byte) (rect_sts.cell_fail & 0xff);

	if ( (btn_save != (port_status & BTN_ALARM_CLEAR)) && !btn_save ) {
		clear_alarms();
	}
	btn_save = port_status & BTN_ALARM_CLEAR;
}
#if 0
void check_power_cut(ALARM_STATUS	*alarm)
{
	alarm->power_cut = 0;

	if (ocp_lock || alarm_sts.ovp || alarm_sts.otp ) {
		alarm->power_cut = 1;
	}
	if (  ac_shutdown_flag	&& rect_sts.input_V >= (AC_HIGH_SHUTDOWN_V - 8) ) {
		alarm->power_cut = 1;
	}
}
#endif

void	check_alarm(ALARM_STATUS	*alarm)
{
	if ( acv_flag == 0 &&  ACV_value != 0 ) alarm->input_V = alarm_input_V(alarm);

	if ( rect_sts.bat_test == 0 && op_sts.bat_testing == 0 ) {
		alarm->output_V = alarm_output_V(alarm);
		alarm->bat_V = alarm_bat_V(alarm);
	}
	alarm->module_fail = alarm_module_fail(alarm);	// POWER_OUT
	//alarm->bat_T = alarm_bat_T(alarm);
	alarm->rec_T = alarm_rec_T(alarm);


	//alarm->bat_NFB =  rect_sts.bat_NFB;
	alarm->cell_fail =  rect_sts.cell_fail;	
	alarm->bat_disconnect = rect_sts.bat_disconnect;

	// ( bat_relay_on )	alarm->bat_disconnect = rect_sts.bat_disconnect;
	//se alarm->bat_disconnect = 1;	// 20140314
	
	//alarm->output_A  = alarm_output_A(alarm);
	if (ocp_lock || alarm_sts.ovp || alarm_sts.otp  || ac_shutdown_flag  ) {
		alarm->power_cut = 1;
	}
	else alarm->power_cut = 0;
	#if 0
	if (  ac_shutdown_flag  && rect_sts.input_V >= (AC_HIGH_SHUTDOWN_V - 8) ) {
		alarm->power_cut = 1;
	}
	#endif
}



void	ovp_protect()
{
	uint8_t alarm = 1;

	//bat_cutoff_on(0);	// cutoff relay off
	
	ovp_count++;
	ovp_flag = 1;
	if ( ovp_count >= 3 ) { 	//|| rect_sts.output_V >= OUTPUT_MODULE_FAIL )  {
		ovp_lock = 1;
		write_eeprom(ALARM_OUTPUT_HIGH_ADDR, &alarm, 1);
		ovp_count = 3;
		alarm_sts.module_fail = 1;
		printf("MODULE FAIL\n");
	}
	else 	{
		#if 1
		//recovery_timer_start();
		alarm =  (char ) ovp_count & 0xff;
		write_eeprom(ALARM_BQL_ADDR, &alarm, 1);
		recovery_start(RECOVER_OVP_RESET);
		#else
		alarm =  (char ) ovp_count & 0xff;
		write_eeprom(ALARM_BQL_ADDR, &alarm, 1);
		log_write(LOG_OVP_ON, EVENT_CLEAR, 0, 0);
		rect_on(0);
		Delay_ms(4500);
		printf("***OVP RESET: %d\n", ovp_count);
		NVIC_SystemReset();	// if shutdown fails, system restart

		#endif
		printf("OVP protect %d: %d\n", ovp_count, ovp_lock);
	}
	alarm_sts.ovp = 1;
	alarm_sts.output_V = ALARM_HIGH;
	rect_on(0);


}


void	bat_cutoff_on(int mode)
{
	// mode = 1, relay ON
	// mode = 0, relay OFF

	if ( mode) {
		bat_relay_on = 1;
		//alarm_sts.bat_lvd = 0;
		if ( system_shutdown_flag == 1 )  {
			return;
		}
		//GPIO_SetBits(GPIO_BAT_CUTOFF, BAT_CUTOFF);
	}
	else  {
		bat_relay_on = 0;
		//GPIO_ResetBits(GPIO_BAT_CUTOFF, BAT_CUTOFF);
	}
	
	//printf("BAT relay %x\n", bat_relay_on);
}

void	check_battery_cutoff()
 {
 	if (  alarm_sts.input_V  && alarm_sts.output_V   ) {
 		if ( read_output_V() < alarm_sts.low_fail_bat  )	bat_cutoff_on(0);
 	}
	else if (  alarm_sts.input_V == 0 &&  !ovp_lock && !fail_lock ) bat_cutoff_on(1);
}


void	ad_set_init()
{
	int value;

	ad_offset.iv = 0;
	ad_offset.ia = 0;
	ad_offset.ov = 0;
	ad_offset.oa= 0;
	ad_offset.ba = 0;	
	ad_offset.ov_set = 0;

	if ( offset_verify() == 0 ) {	// set default values
		ad_offset.bat_zero_offset = (int) (2.5 * 4095 / Vref);				
	}
	else {
		read_eeprom(OFFSET_BAT_ZERO_ADDR, (u8 *) &value, sizeof(int));
		ad_offset.bat_zero_offset  = value;
		if (  value < (2.5 * 4095 / Vref * 0.85) || value > (2.5 * 4095 /Vref * 1.15)) {
			//printf("Invalid bat_zero = %d \n", ad_offset.bat_zero_offset);			
			ad_offset.bat_zero_offset = (int)  (2.5 * 4095 / Vref);
		}		
	}
	printf("Vref = %5.3f, bat_zero = %d \n", Vref, ad_offset.bat_zero_offset);

}	


void alarm_set_default() 
{
	alarm_sts.hi_input_V = INPUT_HI_VOLTAGE_DEF;
	alarm_sts.hi_output_V = OUTPUT_HI_VOLTAGE_DEF;
	alarm_sts.low_input_V= INPUT_LOW_VOLTAGE_DEF;
	alarm_sts.low_output_V = OUTPUT_LOW_VOLTAGE_DEF;
	alarm_sts.low_fail_bat =	BAT_FAIL_LOW_DEF;
	alarm_sts.low_bat_V = BAT_LOW_VOLTAGE_DEF;
	alarm_sts.hi_bat_V = BAT_HI_VOLTAGE_DEF;
	//alarm_sts.hi_fail_bat = BAT_FAIL_HI_DEF;
	alarm_sts.hi_bat_T	= BAT_TEMP_DEF;
	alarm_sts.hi_rec_T	= RECT_TEMP_DEF;
	alarm_sts.low_bat_T	= -40.0;
	alarm_sts.low_rec_T	= -40.0;	
	save_value(ALARM_ACV_HI_ADDR, alarm_sts.hi_input_V);
	save_value(ALARM_ACV_LOW_ADDR, alarm_sts.low_input_V);
	//log_read_header(&log_header);
	save_value(ALARM_DCV_HI_ADDR, alarm_sts.hi_output_V);	
	save_value(ALARM_DCV_LOW_ADDR, alarm_sts.low_output_V);
	//log_read_header(&log_header);
	save_value(ALARM_BAT_FAIL_ADDR, alarm_sts.low_fail_bat);
	save_value(ALARM_BAT_LOW_ADDR, alarm_sts.low_bat_V );
	//log_read_header(&log_header);
	save_value(ALARM_BAT_T_ADDR, alarm_sts.hi_bat_T);
	save_value(ALARM_REC_T_ADDR, alarm_sts.hi_rec_T );
}

#if 1
void alarm_value_check()
{	
	if (alarm_sts.hi_input_V < INPUT_HI_VOLTAGE_MIN ||
		alarm_sts.low_input_V	< INPUT_LOW_VOLTAGE_MIN ||
		alarm_sts.hi_output_V < OUTPUT_HI_VOLTAGE_MIN ||	
		alarm_sts.low_output_V <  OUTPUT_LOW_VOLTAGE_MIN ||
		alarm_sts.low_fail_bat 	<  ( BAT_FAIL_LOW_DEF - 5) ||
		alarm_sts.low_bat_V 	< ( BAT_LOW_VOLTAGE_DEF -5) ||
		alarm_sts.hi_rec_T 	< 10 ||
		alarm_sts.hi_bat_T < 10 ) {
			alarm_set_default();
		}
	else if (alarm_sts.hi_input_V > INPUT_HI_VOLTAGE_MAX ||
		alarm_sts.low_input_V	> INPUT_LOW_VOLTAGE_MAX ||
		alarm_sts.hi_output_V > OUTPUT_HI_VOLTAGE_MAX ||	
		alarm_sts.low_output_V >  OUTPUT_LOW_VOLTAGE_MAX ||
		alarm_sts.low_fail_bat 	>  ( BAT_FAIL_LOW_DEF + 10) ||
		alarm_sts.low_bat_V 	> ( BAT_LOW_VOLTAGE_DEF + 10 ) ||
		alarm_sts.hi_rec_T 	> 120 ||
		alarm_sts.hi_bat_T > 120 ) {
			alarm_set_default();
		}	
}
#endif

void	alarm_set_init() 
{
	if ( eeprom_read(ALARM_ACV_HI_ADDR) == 0xff ) {
		alarm_set_default();
		//return;
	}
	else {
		alarm_sts.hi_input_V 	= read_value(ALARM_ACV_HI_ADDR);
		alarm_sts.low_input_V	= read_value(ALARM_ACV_LOW_ADDR);
		alarm_sts.hi_output_V = read_value(ALARM_DCV_HI_ADDR);	
		alarm_sts.low_output_V = read_value(ALARM_DCV_LOW_ADDR);
		alarm_sts.low_fail_bat 	= read_value(ALARM_BAT_FAIL_ADDR);
		alarm_sts.low_bat_V 	= read_value(ALARM_BAT_LOW_ADDR);
		alarm_sts.hi_bat_T 	= read_value(ALARM_BAT_T_ADDR);
		alarm_sts.hi_rec_T 	= read_value(ALARM_REC_T_ADDR);

		alarm_sts.hi_bat_V =	BAT_HI_VOLTAGE_DEF;
		//alarm_sts.hi_fail_bat = BAT_FAIL_HI_DEF;
		alarm_sts.low_bat_T	= -40.0;
		alarm_sts.low_rec_T	= -40.0;
	}
#if 1
	alarm_value_check();
#endif
	alarm_sts.input_V = 0;
	alarm_sts.output_V = 0;
	alarm_sts.input_A = 0;
	alarm_sts.output_A = 0;
	alarm_sts.input_V = 0;
	alarm_sts.bat_V = 0;
	alarm_sts.bat_A = 0;
	alarm_sts.bat_T = 0;
	alarm_sts.rec_T = 0;;
	alarm_sts.cell_fail = 0;
	alarm_sts.ac_NFB = 0;
	alarm_sts.bat_NFB = 0;
	alarm_sts.ovp = 0;
	alarm_sts.out_NFB = 0;
	alarm_sts.bat_disconnect = 0;
	alarm_sts.bat_bql = 0;
	//alarm_sts.bat_lvd = 0;
}


void	operation_set_default()
{
	int i;
	

	//printf("%s\n", __FUNCTION__);
	op_sts.output_mode = 0;
	op_sts.relay_contact = 0;
	op_sts.bat_test_mode = 0;
	op_sts.bat_test_period = 1;		// 3Months
	op_sts.bat_test_V = 46.0;
	op_sts.output_V = OUTPUT_VOLTAGE_DEF;
	op_sts.current_mode = 1;
	op_sts.charge_A = CHARGE_A;
	op_sts.bat_equip = 0;

	op_sts.month = sys_info.month;
	op_sts.date = 7;
	op_sts.hour = 3;
	op_sts.min = 0;
	for ( i = 0; i < (OP_RELAY_ADDR - OP_BAT_TEST_PERIOD_ADDR + 1); i++) {
		eeprom_write(OP_BAT_TEST_PERIOD_ADDR+i, 0);
	}
	write_eeprom(OP_OUTPUT_VALUE_ADDR  , (byte*) &op_sts.output_V, sizeof(float));
	write_eeprom(OP_CURRENT_VALUE_ADDR  , (byte*) &op_sts.charge_A , sizeof(float));
	write_eeprom(OP_BAT_TEST_VOLTAGE_ADDR  , (byte*) &op_sts.bat_test_V , sizeof(float));

	op_sts.bat_equip = 0;
	eeprom_write(BAT_EQUIP_ADDR, 0);
	op_sts.alarm_mode = ALARM_MODE_CONTACT;
	eeprom_write(ALARM_MODE_ADDR, 0);

}

void	operation_init()
{
	//printf("%s\n", __FUNCTION__);

	if ( eeprom_read(OP_OUTPUT_MODE_ADDR ) == 0xff ) {
		operation_set_default();
		return;
	}

	op_sts.bat_equip = (int) eeprom_read(BAT_EQUIP_ADDR) & 0xff;
	if ( op_sts.bat_equip != 1 ) op_sts.bat_equip = 0;

	op_sts.output_mode = read_output_mode();
	op_sts.current_mode = 1;	// always 1, read_current_limit_mode();
	//op_sts.relay_contact = relay_read_contact();

	op_sts.output_V = read_output_voltage();
	op_sts.charge_A = CHARGE_A;     //read_current_limit();
	
	op_sts.bat_test_mode = read_bat_test_mode();
	//op_sts.bat_test_mode = 0;		// 20140312 disable battery auto test
	op_sts.bat_test_period = read_bat_test_period();
	op_sts.bat_test_V = read_bat_test_voltage();

	if (op_sts.bat_test_period < 1  ) op_sts.bat_test_period = 1;
	read_next_test_time();
	//if (op_sts.bat_test_V < (BAT_TEST_FAIL_V - 2) ) op_sts.bat_test_V = BAT_TEST_FAIL_V;

	if (( op_sts.output_V < OUTPUT_LOW_VOLTAGE_DEF || op_sts.output_V >= 58.1 )  )	// 54.5
	{
			op_sts.output_V = OUTPUT_VOLTAGE_DEF;
			write_eeprom(OP_OUTPUT_VALUE_ADDR  , (byte*) &op_sts.output_V, sizeof(float));
	}
		
	if (( op_sts.charge_A < MIN_CURRENT_LIMIT || op_sts.charge_A > MAX_CURRENT_LIMIT )	 ) {
			op_sts.current_mode = 1;	// always 1
			op_sts.charge_A = CHARGE_A;
			write_eeprom(OP_CURRENT_VALUE_ADDR  , (byte*) &op_sts.charge_A , sizeof(float));
	}

	#if 0
	if ( op_sts.current_mode  ) {		
		write_current_limit(op_sts.charge_A);
	}
	else  {
	       //op_sts.charge_A = 2.0;
	       DAC_WriteCh(2, 4095);
	}	
	#endif

	//init_alarm_mode();

	bat_data.send_flag = 0;
	bat_data.result = 0;
}

void	sys_read_fw_version()
{		
	sprintf((char *)FW_VERSION, "%d.%d.%d.%d", (fw_version>> 24) & 0xff, (fw_version>> 16) & 0xff, (fw_version>> 8) & 0xff, fw_version & 0xff);

}

void	sys_read_hw_version()
{
	int	 version;

	//version = *(int *)HW_VERSION_ADDRESS;
	eeprom_read_buffer( HWV_EP_ADDR, (uint8_t *) &version, 4);
	if ( version != 0xffffffff ) hw_version = version;
	if ( hw_version == 0 ) hw_version = 0x01000000;

	printf("\nHW ver=%x ", hw_version);
	sprintf((char *)HW_VERSION, "%d.%d.%d.%d", (hw_version>> 24) & 0xff, (hw_version>> 16) & 0xff, (hw_version>> 8) & 0xff, hw_version & 0xff);

}

void	sys_read_mac()
{
	int mac_addr;
	uint8_t  mac_1, mac[6];

	mac_addr = MAC_ADDR;

	mac_1 = eeprom_read(mac_addr);
	if (  mac_1 == 0xff ) {
			mac[0] = MAC_PREFIX1;
			mac[1] = MAC_PREFIX2;
			mac[2] = MAC_PREFIX3;
			mac[3] = 0x11;
			mac[4] = 0x22;
			mac[5] = 0x33;
	}
	else {
		eeprom_read_buffer(mac_addr, mac, 6);	
		//mac[0] = MAC_PREFIX1;
		//mac[1] = MAC_PREFIX2;
		//mac[2] = MAC_PREFIX3;
	}
	memcpy ( sys_info.s_mac, mac, 6);
	memcpy ( psu_mac, mac, 6);
	//printf("RD MAC = %x %x %x \n", sys_info.s_mac[3], sys_info.s_mac[4], sys_info.s_mac[5]);

}


void	sys_read_sn()
{
	int sn_addr;
	uint8_t  isn;



	sn_addr =  SN_EP_ADDR;

	isn = eeprom_read( sn_addr);
	//printf("SN1: %x \n", isn);
	if (  isn == 0xff ) strcpy((char *)SERIAL_NO, SKT48V_SN);
	else {
		eeprom_read_buffer( sn_addr, SERIAL_NO, 20);
	}
	SERIAL_NO[10]= 0;

	ip_set_hw_desc((char *)SERIAL_NO);

	//printf("SN : %s\n", SERIAL_NO);
}


int	sys_write_hw_version(int version)
{
	printf("WR HW ver. = %x \n", version);
	write_eeprom( HWV_EP_ADDR, (u8 *) &version, 4);	
	return 0;
}

int	sys_write_sn(char *sn)
{

	
	printf("WR SN = %s \n", sn);
	//eeprom_write_buffer( SN_EP_ADDR, sn, 20);
	write_eeprom(SN_EP_ADDR, (uint8_t *) sn, 20);

	return 0;
}

#if 0
int	sys_write_hw_info(int version, char *sn)
{
	//printf("SET HW ver. = %x \n", version);
	#if 0
	FLASH_Unlock();

	FLASH_ErasePage(HW_VERSION_ADDRESS);
	IWDG_ReloadCounter();
	FLASH_ProgramWord(HW_VERSION_ADDRESS, version );
	flash_write(SN_ADDRESS, sn, 20);

	FLASH_Lock();
	#endif


	eeprom_write_buffer( HWV_EP_ADDR, (u8 *) &version, 4);	
	eeprom_write_buffer( SN_EP_ADDR, (u8 *) &sn, 20);	
	
	return 0;
}

#endif


int	sys_write_ip_info(int ip)
{
	//printf("SET IP ADDR = %x \n", ip);
	int eip;
	
	#if 0
	FLASH_Unlock();

	FLASH_ErasePage(HW_VERSION_ADDRESS);
	IWDG_ReloadCounter();
	FLASH_ProgramWord(HW_VERSION_ADDRESS, hw_version );
	FLASH_ProgramWord(IP_ADDRESS, ip );
	flash_write(SN_ADDRESS, SERIAL_NO, 20);	

	FLASH_Lock();
	#endif

	if ( Enable_DHCP == DHCP_OFF ) {
	write_eeprom( IP_EP_ADDR, (uint8_t *) &ip, 4);
		eeprom_read_buffer(IP_EP_ADDR, (uint8_t *) eip, 4);
	}
	else if ( Enable_DHCP == DUE_ADDR ) {
		write_eeprom( IP_DUE_ADDR, (uint8_t *) &ip, 4);
		eeprom_read_buffer(IP_DUE_ADDR, (uint8_t *) eip, 4);
	}
	sys_info.ip_addr = psu_ip;
	printf("SET IP = %x %x\n", ip, eip);

	return 0;
}

int	sys_write_mac_info(uint8_t *mac)
{
	printf("MACOUI:%x %x %x\n", mac[0], mac[1], mac[2]);
	printf("MACNIC:%x %x %x \n", mac[3], mac[4], mac[5]);
	write_eeprom( MAC_ADDR, mac, 6);


	return 0;
}


void	sys_set_info(uint8_t *info)
{
	ip_set_memo_info(info);
	//dprintf("Sys_set_info: %s \n", info);
}


void	sys_set_hw_version(int version)
{		
	//sys_read_sn(SERIAL_NO);	
	sys_write_hw_version(version);
	sys_read_hw_version();
}


void	sys_set_mac(uint8_t *mac)
{
	sys_write_mac_info(mac);
	sys_read_mac();
}

uint8_t ch2no(uint8_t d1)
{
	uint8_t no;
	
	if ( d1 >= 'A' && d1 <= 'F') {
		no = d1 - 'A' + 10;
	}
	else if ( d1 >= '0' && d1 <= '9' ) {
		no = d1 -'0';
	}
	else no = 0x0f;
	//printf("c2n %x %x\n", d1, no);
	return no;
}

uint8_t sn2no(uint8_t d1, uint8_t d2)
{
	return ( ch2no(d1) * 16 + ch2no(d2));
}

void	sys_set_sn(uint8_t *sn)
{
	uint8_t mac[6];
	uint32_t y, m, l, s;
	
	//hw_version = sys_read_hw_version();
	sys_write_sn(sn);
	sys_read_sn();


	mac[0] = sn[0];
	mac[1] = sn[1];
	mac[2] = 0;
	y = (atoi(mac) - 18) & 0x0f; 
	mac[0] = sn[2];
	mac[1] = sn[3];
	mac[2] = 0;
	m = atoi(mac) & 0x0f; 
	y = y * 16 + m;

	mac[0] = sn[4];
	mac[1] = sn[5];
	mac[2] = 0;
	l = atoi(mac) ; 


	mac[0] = sn[6];
	mac[1] = sn[7];
	mac[2] = sn[8];
	mac[3] = sn[9];
	mac[4] = 0;
	s = atoi(mac) ; 
	l = ((l << 5 ) & 0xf0) + ((s >> 8) & 0x1f);
	s &= 0xff;

	mac[0] = MAC_PREFIX1;
	mac[1] = MAC_PREFIX2;
	mac[2] = MID_PREFIX;
	
	mac[3] = y & 0xff;
	mac[4] = l & 0xff;
	mac[5] = s & 0xff;

	sys_write_mac_info(mac);
	sys_read_mac();
	m = (mac[2] << 24) + (mac[3]<< 16 ) + (mac[4] << 8) + mac[5];
	set_management_id(m);

}


void	sys_info_init()
{
	get_system_time(&sys_info);
	//strcpy (SERIAL_NO, __TIME__ );
	//strcpy (FW_VERSION, __DATE__ );
	sys_read_fw_version();
	sys_read_hw_version();
	sys_read_sn();	
	sys_read_mac();
	sys_read_management_id();
	//printf("HW ver. = %x \n", version);
	
	//sprintf(FW_VERSION, "%d.%d.%d.%d", (fw_version>> 24) & 0xff, (fw_version>> 16) & 0xff, (fw_version>> 8) & 0xff, fw_version & 0xff);
	sys_info.hw_version = HW_VERSION;
	sys_info.fw_version = FW_VERSION;
	sys_info.serial_no = SERIAL_NO;

	sys_info.ip_addr = psu_ip;

}

uint32_t	sys_read_management_id()
{
	uint32_t	id;
	uint8_t  change;

	read_eeprom(MANAGEMENT_ID_SAVE, &change, 1);
	read_eeprom(MANAGEMENT_ID_ADDR, (uint8_t *)&id, sizeof(int));
	if ( change != 0x69 ) {
		id = (MID_PREFIX << 24) + (psu_mac[3] << 16) + (psu_mac[4] << 8) + psu_mac[5];
	}

	management_id = id;
	*(uint32_t *)saved_management_id = id;
	printf("S MID=%u,%08X\n", id, id);

	return id;
}


uint32_t	read_management_id()
{
	return management_id;
}

void	save_management_id(uint32_t id)
{
	uint8_t  save = 0x69;
	
	write_eeprom(MANAGEMENT_ID_ADDR, (uint8_t *)&id, sizeof(int));
	write_eeprom(MANAGEMENT_ID_SAVE, &save, 1);
	*(uint32_t *)saved_management_id = id;

}

void	rect_status_init()
{
	rect_sts.rt_offset = 0;
	rect_sts.bt_offset = 0;

	
	if ( (eeprom_read(OFFSET_IV_ADDR)== 0xff) && (eeprom_read(OFFSET_IV_ADDR+1) == 0xff)) {	// eeprom error
		rect_sts.iv_offset = 0;
		rect_sts.ia_offset = 0;
		rect_sts.ov_offset = 0;
		rect_sts.oa_offset = 0;
		rect_sts.ba_offset = 0;
		rect_sts.ov_set_offset = 0;
	}
	else {
		rect_sts.iv_offset = ad_read_offset( OFFSET_IV_ADDR);
		rect_sts.ia_offset = ad_read_offset( OFFSET_IA_ADDR);
		rect_sts.ov_offset = ad_read_offset( OFFSET_OV_ADDR);
		rect_sts.oa_offset = ad_read_offset( OFFSET_OA_ADDR);
		rect_sts.ba_offset = ad_read_offset( OFFSET_BA_ADDR);
		rect_sts.ov_set_offset = ad_read_offset(OFFSET_OV_SET_ADDR);
	}

	if ( rect_sts.ba_offset < -5.0 || rect_sts.ba_offset > 5.0 ) rect_sts.ba_offset = 0.0;
	if ( rect_sts.oa_offset < -5.0 || rect_sts.oa_offset > 5.0 ) rect_sts.oa_offset = 0.0;

	
	rect_sts.bat_test = 0;
	printf("OFFSET DcV = %5.3f, DcA_ = %5.3f, BatA = %5.3f\n", rect_sts.ov_offset, rect_sts.oa_offset, rect_sts.ba_offset);
	printf("OFFSET AcV = %5.3f, AcA_ = %5.3f, ov= %5.3f\n", rect_sts.iv_offset, rect_sts.ia_offset, rect_sts.ov_set_offset);

	//printf("SAVED OFFSET  BA: %5.2f, OA = %5.2f  \n",rect_sts.ba_offset, rect_sts.oa_offset);
}

#if 0
void	wait_for_init()
{
	int i,k = 0,j = 0 ;

	for ( i = 0; i < 9999999; i++) {
		k = i * j++;	
	}
}
#endif

void slow_start()
{
	float out = MIN_OUT_V;

	while ( 1 ) {		
		//printf("OUT %5.2fV\n", out);
		//if ( read_output_V() >= alarm_sts.output_V ) break;
		avg_ref();
		out += 0.3;
	#if 1
		if ( out >= (op_sts.output_V - 0.3) ) {
			out = op_sts.output_V - 0.3;
			write_output_voltage(out);
			break;
		}
	#endif
		write_output_voltage(out);
		recover_vout = 3;
		Delay_ms(50);	// 10msec
	}
	//out = read_dcv_value();
	printf("SLOW START %5.2fV \n", out );

	#if 0	// 2017 3.14
	//if ( read_output_V() < OCP_ZERO_V ) {
	out = read_dcv_value();
	if ( out < OCP_ZERO_V ) {
			printf("RECT OCP %5.2fA\n", read_output_A());
			rect_on(0);	
			ocp_flag = 0;
			log_write(LOG_OCP_ON, EVENT_OCCUR, 0, 0);
	}
#endif
	return;

}

void wait_normal_voltage()
{
	int	timeout = 20;	// wait for 500msec at most
	while ( timeout--) {
		if ( read_output_V() > OUTPUT_THRESHOLD ) return;
		wait(5);	// wait for 50msec		
	}
}

void ovp_reset()
{
	//char alarm;
	
	//alarm =  (char ) ovp_count & 0xff;
	//write_eeprom(ALARM_BQL_ADDR, &alarm, 1);
	//log_write(LOG_OVP_ON, EVENT_CLEAR, 0, 0);
	//rect_on(0);
	//Delay_ms(4500);
	printf("***OVP RESET: %d\n", ovp_count);
	NVIC_SystemReset(); // if shutdown fails, system restart

}

void recovery_init()
{
	recover.state = REC_IDLE;
	recover.mode = 0;
	recover.timer = 0;

	ocp_flag = 0;
	ocp_count = 0;
	ocp_lock = 0;
	ac_shutdown_flag = 0;
}

void	avg_ref_adc()	// averaging Vref
{
	uint32_t ref;
	
	ref = ADC_ReadCh(17);		// adjust Vref continuously
	
	//printf("%s:%d\n", __FUNCTION__, __LINE__);
	if ( vref_flag == 0 ) {
		vref[vref_ind] = ref;
		vref_sum += ref;
		//ref = vref_sum / MAX_VREF;
	}
	else {
		vref_sum -= vref[vref_ind];
		vref[vref_ind] = ref;
		vref_sum += ref;
		ref = vref_sum / MAX_VREF;
		Vref = 1.2 * 4095.0 / (float) ref ;
	}
	
	vref_ind++;
	if ( vref_ind >= MAX_VREF ) {
		vref_ind = 0;
		vref_flag = 1;
	}


}


void	avg_ref()	// averaging Vref
{
	uint32_t ref;
	
	//ref = ADC_ReadCh(17);		// adjust Vref continuously
	//ref = adc_dr[5];
	ref = adv.refv / 10;
	//printf("%s:%d\n", __FUNCTION__, __LINE__);
	if ( vref_flag == 0 ) {
		vref[vref_ind] = ref;
		vref_sum += ref;
		//ref = vref_sum / MAX_VREF;
	}
	else {
		vref_sum -= vref[vref_ind];
		vref[vref_ind] = ref;
		vref_sum += ref;
		ref = vref_sum / MAX_VREF;
		Vref = 1.2 * 4095.0 / (float) ref ;
	}
	
	vref_ind++;
	if ( vref_ind >= MAX_VREF ) {
		vref_ind = 0;
		vref_flag = 1;
	}


}

void	rabm_init()
{

    float outv;	
	int count = 0;
	int eps;


	for ( count = 0 ; count < 40; count++) {
		avg_ref_adc();
		check_port_status();
		Delay_ms(4);		
	}
	count = 0;
	
	ADC_Configuration_scanmode();

	init_log();
    
	dhcp_state = 0 ;  // 0 = STATE_DHCP_INIT;
	//check_wiznet();	//init_relay_mode();

	adc_ready = 1;
	ad_set_init();
	//offset_verify();
	sys_info_init();	// ksyoo 20150324	
	
	alarm_set_init();
	rect_status_init();
	operation_init();
	check_saved_alarm();		// 20140312 disble, 20142026 check output_hi, module_fail
	recovery_init();

	init_relay_mode();
	init_alarm_mode();
	
	wiz_init();



	Delay_ms(1700);

	if ( !ovp_lock && !fail_lock ) {
		write_output_voltage(MIN_OUT_V);
		Delay_ms(10);	// 2700 = 2sec
		//DAC_WriteCh(1, 0);
		rect_on(1); 
		Delay_ms(50);	//	900
		slow_start();
	}
	else rect_on(0);
	Delay_ms(150);	// 450  = 1sec
	outv = read_dcv_value();
	if (ovp_lock ) { // || fail_lock)  {
		alarm_sts.module_fail = 1;
	}
	else if ( fail_lock) {
		//alarm_sts.module_fail = 1;
		alarm_sts.module_fail = 0;
	}
	else if ( ovp_count ) {
		alarm_sts.ovp = 1;
		alarm_sts.output_V = ALARM_HIGH;
		recovery_start(RECOVER_OVP);

	}

skip_output:
	//CS5463_init();
	
	eps = CS5463_Read(0, ECS_EPSILON);
	printf("ACT Epsilon: 0x%x\n", eps);
	check_CS5463_init();
	Delay_ms(200);

	test_mode = 0;
	ip_read_psu_name(user_psu_name);

	//power_back_time = get_time_counter();
	if ( GPIO_ReadInputDataBit(GPIO_DI_BAT_SW, DI_BAT_SW) ) {		// BAT SW OFF
		bat_sw_state  = 0;	
	}
	else bat_sw_state = 1;
}




void recovery_timer_start()
{
#if 0
	if ( alarm_sts.input_V != 0 ) {
		return;
	}
#endif
	if ( recover.mode == 0 ){
		recover.mode = RECOVER_FORCED;
		recover.timer = time10ms + RECOVER_FORCED_TIME;	
		recover.state = REC_FORCED;
		ovp_flag = 0;
		alarm_sts.ovp = 0;
		//log_write(LOG_OVP_OFF, EVENT_CLEAR, 0, 0);
		printf("RECOVERY Timer START ");
	}
}

void recovery_start(int mode)
{
#if 0
	if ( rect_sts.input_V < 50.0 ) {
		return;
	}
#endif
	
	if ( recover.mode == 0 ) {
		recover.mode = mode;
		if ( recover.mode == RECOVER_OVP_RESET ) {
			recover.state = REC_OVP_RESET;
			recover.timer = time10ms + RECOVER_OVP_RESET_TIME;
			rect_on(0);
		}
		else if ( recover.mode == RECOVER_OCP) {
			recover.state = REC_OCP;
			recover.timer = time10ms + RECOVER_OCP_TIME;
		}
		else if ( recover.mode == RECOVER_OVP) {
			recover.state = REC_OVP;
			//recover.timer = time10ms + RECOVER_OCP_TIME;
		}
		else {		
			recover.state = REC_IDLE;
		}
		printf("RECOVERY START:%d\n", recover.mode); //: %5.2fV, %5.2fT\n", recover.mode, rect_sts.output_V, rect_sts.rec_T);
	}
	//else printf("RECOVERY START FAIL %d : %5.2fV, %5.2fT\n", recover.mode, rect_sts.output_V, rect_sts.rec_T);`
}

int	recovery_do()
{
	uint8_t alarm = 0;
	float v, offset;
	static unsigned long long time;

	if ( recover.mode == 0 ) return 0;

#if 1
	v = read_output_V();
	offset = 0.0;
	
	if ( v  > (OUTPUT_OVP_DEF-offset)  ) {		
		if ( recover.state == REC_FORCED ) {
			if ( recover.timer  <= time10ms ) {
				recover.state = REC_FAIL;
				return 1;
			}			
		}
		else if ( recover.state != REC_OVP && recover.state != REC_OVP_RESET) {
			recover.state = REC_FAIL;
		}
	}
	
#endif

	switch ( recover.state ) {
		case REC_OVP:
			offset = 1.0;
			if ( v  >= (OUTPUT_OVP_DEF-offset)  ) {
				recover.state = REC_OVP_FAIL;
			}
			else recover.state = REC_OVP_END;
			break;
		case REC_OVP_FAIL:
			recover.mode = 0;
			recover.state = REC_IDLE;
			log_write(LOG_OVP_FAIL, EVENT_OCCUR, 0, 0);
			if ( !ovp_lock) ovp_protect();
			alarm_notification_flag = 1;
			printf("REC OVP FAIL : %5.2f\n", v);
			break;
		case REC_OVP_END:
			alarm = 0;
			write_eeprom(ALARM_OUTPUT_HIGH_ADDR, &alarm, 1);
			recover.mode = 0;			
			recover.state = REC_IDLE;
			log_write(LOG_OVP_OFF, EVENT_CLEAR, 0, 0);
			alarm_sts.ovp = 0;
			//ovp_clear_flag = 1;
			ovp_clear_time = time10ms + OVP_CLEAR_TIME; // 180000;	// wait for 180sec (3min)
			alarm_notification_flag = 1;
			printf("REC OVP OK : %5.2f\n", v);
			break;
		case REC_OVP_RESET:
			if ( recover.timer  <= time10ms ) {
				//ovp_reset();
				recover.state = REC_IDLE;				
			}
			break;	
		case REC_FORCED:
		case REC_OCP:
			if ( (recover.timer ) <= time10ms ) {
				recover.state = REC_IDLE;
				
			}
			break;
		case REC_IDLE:		
			
			recover.state = REC_MIN;
			recover.out = MIN_OUT_V;
			write_output_voltage(recover.out);
			time = time10ms + 10 ;	// 0907: 100->10
			printf("REC MIN : %5.2f\n", recover.out);
			break;
		case REC_MIN:
			if (time < time10ms ) {
				recover.state = REC_ON;
			}
			break;
		case REC_ON:
			write_output_voltage(recover.out);
			 rect_on(1);
			 recover.state = REC_WAIT1;
			 time = time10ms + 25 ;
			 printf("REC ON : %5.2f\n", recover.out);
			 break;
		case REC_WAIT1:
			if (time < time10ms ) {
				recover.state = REC_RAMP;

			}
			break;
		case REC_RAMP:
			#ifndef HIGH_TEMP_TEST
			if ( v < OCP_ZERO_V ) {
				recover.state = REC_OCP_FAIL;
				return 1;	
			}
			#endif
			
			recover.out += 0.3;
			if ( recover.out >= ( op_sts.output_V - 0.3) ) {  // 20170705 OUTPUT_VOLTAGE_DEF ) {
				recover.out = ( op_sts.output_V - 0.3);
				recover.state = REC_END;
			}
			else {
				recover.state = REC_WAIT2;
				time = time10ms + 5 ;
			}
			write_output_voltage(recover.out);		
			//printf("REC ON : %5.2f\n", recover.out);
			break;
		case REC_WAIT2:
			if (time <= time10ms ) {
				 time = time10ms + 4 ;	// 5
				recover.state = REC_RAMP;
			}
			break;
		case REC_END:
			recover.state = REC_IDLE;			
			if ( recover.mode == RECOVER_OVP_RESET || recover.mode == RECOVER_USER ) {
				write_eeprom(ALARM_OUTPUT_HIGH_ADDR, &alarm, 1);			
				ovp_flag = 0;
				alarm_sts.ovp = 0;
				ovp_lock = 0;
			}
			if ( recover.mode == RECOVER_FAIL || recover.mode == RECOVER_USER){
				write_eeprom(ALARM_MODULE_FAIL_ADDR, &alarm, 1);				
			}
			if ( recover.mode == RECOVER_OCP ) {
				ocp_lock= 0; ocp_flag = 0; ocp_count = 0;
			}
			if ( recover.mode == RECOVER_OTP ) {
				ocp_lock= 0; ocp_flag = 0; ocp_count = 0; OTP_flag = 0;
			}
			else {
				ovp_lock = 0; fail_lock= 0; alarm_sts.module_fail = 0;				
			}

			ac_shutdown_flag = 0;
			OTP_flag = 0;
			alarm_sts.ovp = 0;
			//alarm_sts.ocp = 0;	// 190115
			printf("REC end %d : %5.2fV, %5.2fT\n",recover.mode, rect_sts.output_V, rect_sts.rec_T);
			recover.mode = 0;	
			recover_vout = 3;
			break;
		case REC_FAIL:
			ac_shutdown_flag = 0;
			recover.mode = 0;
			printf("REC FAIL : %5.2f\n", v);
			recover.state = REC_IDLE;
			if ( !ovp_lock) ovp_protect();
			log_write(LOG_OVP_FAIL, EVENT_OCCUR, 0, 0);
			break;
		case REC_OCP_FAIL:
			ac_shutdown_flag = 0;
			ocp_flag = 0;
			recover.mode = 0;
			printf("REC OCP FAIL : %5.2f %d\n", v, ocp_count);
			recover.state = REC_IDLE;
			//log_write(LOG_OVP_FAIL, EVENT_OCCUR, 0, 0);
			break;
		default:
			break;
	}
	return 1;
}

void check_OCP()
{

	if ( recover.mode != 0 || alarm_sts.input_V || ovp_lock || fail_lock || OTP_flag  || A1_mode == 0 ) return;

	//printf("checkOCP %5.2f %d %d ",rect_sts.output_V, ocp_flag, ocp_lock);
	if (  rect_sts.output_V < OCP_ZERO_V  && ocp_flag == 0 && ocp_lock == 0 && A1_mode == 1  ) {
		rect_on(0);
		alarm_sts.ocp = 1;
		if ( ocp_count++ < 10 )	{
			if ( ocp_count == 1 ) log_write(LOG_OCP_ON, EVENT_OCCUR, 0, 0);
			ocp_flag = 1;
			recovery_start(RECOVER_OCP);
		}
		else {
			ocp_lock = 1;
			log_write(LOG_OCP_FAIL, EVENT_OCCUR, 0, 0);
		}
	}
	else {
		if ( alarm_sts.ocp && ocp_lock == 0 ) {
			log_write(LOG_OCP_OFF, EVENT_CLEAR, 0, 0);
			alarm_sts.ocp = 0;
		}
	}

}


void check_OVP()
{
	static int OVP_count = 0; 
	static int fail_on_count = 0, fail_off_count = 0;
	//char	alarm = 1;

	//if ( ovp_lock || ocp_flag || fail_lock ) return;
	if ( ovp_lock || ocp_flag  ) return;

	if ( alarm_sts.ovp == 1 &&  ovp_count < 3 ) {
		if ( recover.mode == 0) {
			printf("OVP check RECOVER\n");
			recovery_start(RECOVER_OVP_RESET);
			//return;
		}
	}

	if ( !fail_lock ) {
		if ( OVP_on_ready == 1 ) {
			//printf("ovp flag : %d\n", ovp_flag);
			if ( !ovp_flag && OVP_count++ >= 5 ) {	// OVP_count : 3->5 2017 5.30
				ovp_protect();				
				OVP_count = 0;
			}
			//ovp_off_count = 0;
		}
		
		else  {		
			#if 0
			if ( ovp_flag && ovp_off_count++ >= 10 ) {
				ovp_flag = 0;				
				recovery_start(RECOVER_OVP);
				//write_eeprom(ALARM_OUTPUT_HIGH_ADDR, &alarm, 0);
			}
			#endif
			OVP_count = 0;
		}
	}

	#if 1
    if ( fail_on_ready == 1 ) {
		if ( !fail_lock ) {
			if ( fail_on_count++ >= 5 ) {
				//bat_cutoff_on(0);	// cutoff relay off				
				//write_eeprom(ALARM_MODULE_FAIL_ADDR, &alarm, 1);
				printf("OTP : %5.2fV, %5.2fT\n", rect_sts.output_V, rect_sts.rec_T);
				rect_on(0);
				alarm_sts.otp = 1;
				fail_lock = 1;
			}
		}
		fail_off_count = 0;
	}
	else {
		#if 1
		if ( fail_lock ) {
			if ( fail_off_count++ >= 10 ) {
				alarm_sts.otp = 0;
				recovery_start(RECOVER_OTP);	
				printf("OTP recover\n");
				fail_lock = 0;
			}			
		}
		#endif
		fail_on_count = 0;
		
	}	
	#endif
}


void ac_recovery_start()
{
	if ( recover.mode == 0 ) {
		recover.mode = RECOVER_AC;		
		recover.state = REC_IDLE;
		printf("AC REC START %d : %5.2fV, %5.2fT\n", recover.mode, rect_sts.output_V, rect_sts.rec_T);
	}
}

void check_ac_input() //uint16_t changes)
{
	//return;

	//if ( recover.mode != 0 ) return;
	if ( ovp_lock || ocp_flag || fail_lock ) return;
	if ( acv_flag == 1 || ACV_value == 0 ) return;
		
	if ( ac_shutdown_flag == 0 && (  rect_sts.input_V < AC_SHUTDOWN_V || rect_sts.input_V >= AC_HIGH_SHUTDOWN_V)) {	// No low AC shutdown : rect_sts.input_V < AC_SHUTDOWN_V ||
			ac_shutdown_flag = 1;
			printf("AC SHUTDOWN:");
			rect_on(0);
			recover.mode = 0;
			recover.state = REC_IDLE;
			return;
	}

	if ( ac_shutdown_flag == 1 &&  rect_sts.input_V > (AC_SHUTDOWN_V + 8.0) && rect_sts.input_V <= (AC_HIGH_SHUTDOWN_V - 8.0)) {	// No : ( rect_sts.input_V > (AC_SHUTDOWN_V + 8.0) AC 175.0V
			//if (  alarm_sts.bat_V || alarm_sts.bat_NFB || alarm_sts.bat_disconnect ) return;			
		ac_shutdown_flag = 0;
		ac_recovery_start();
	}	
}

int ref = 0, count = 0;

void vref_init()
{
	int i;

	for ( i = 0 ; i < MAX_VREF; i++) {
		vref[i] = 0;
	}
	vref_ind = 0;
	vref_sum = 0;
	vref_flag = 0;

}

void sr_on()
{
	printf("SR ON\n");
	GPIO_WriteBit(GPIO_SR_ON, SR_ON_PIN, (BitAction) 0);
	//GPIO_WriteBit(GPIO_WD, WD_PIN, (BitAction) 0);
}

void sr_off()
{
	printf("SR OFF\n");
	GPIO_WriteBit(GPIO_SR_ON, SR_ON_PIN, (BitAction) 1);
	//GPIO_WriteBit(GPIO_WD, WD_PIN, (BitAction) 1);

}

int	get_auto_test_mode()
{
	return op_sts.bat_test_mode;
}



void bat_test_save_mode(int 	mode)
{
	eeprom_write(OP_BAT_TEST_MODE_ADDR,  mode);
}

void bat_test_save_v(float	value)
{
	u8	*buf;

	buf = (byte *) &value;
	write_eeprom(OP_BAT_TEST_VOLTAGE_ADDR, buf, sizeof(float));
}

void bat_test_save_period( int month)
{
	//eeprom_write(OP_BAT_TEST_PERIOD_ADDR, (byte * ) &month, sizeof(int));
      write_eeprom(OP_BAT_TEST_PERIOD_ADDR, (byte * ) &month, sizeof(int));
}

void bat_test_save_time()
{
	u8	buf[8];

	buf[0]= op_sts.year ;
	buf[1]= op_sts.month ;
	buf[2]= op_sts.date ;
	buf[3] = 0;
	buf[4]= op_sts.hour ;
	buf[5]= op_sts.min ;
	buf[6]= op_sts.sec ;
	
	write_eeprom(OP_BAT_TEST_DATE_ADDR, buf, 8);
}

void bat_test_set_mode(int 	mode)
{
	op_sts.bat_test_mode = mode;
	
	if ( mode ) {
		set_bat_test_log(LOG_BAT_TEST_ON,  0, 0);
	}
	else set_bat_test_log(LOG_BAT_TEST_OFF,  0, 0);

	
}

void bat_test_set_v(float	value)
{
	float	pvalue;

	//printf("%s %f\n", __FUNCTION__, value);
	printf("BAT SETV %f\n", op_sts.bat_test_V);

	pvalue = op_sts.bat_test_V;
	op_sts.bat_test_V = value;
	auto_test.v = op_sts.bat_test_V;
	
	set_bat_test_log(LOG_BAT_TEST_V_SET,  pvalue, value);

	
	
}

void bat_update_auto_test_day()
{
	get_system_time(&sys_info);

	auto_test.month = sys_info.month + op_sts.bat_test_period;
	if (auto_test.month > 12) {
			auto_test.month -= 12;
	}
	auto_test.day = op_sts.date;
	auto_test.hour = op_sts.hour;
	auto_test.min = op_sts.min;
	
	if ( auto_test.month == 2  && auto_test.day > 28 ) {
		auto_test.day = 28;
	}
	else if ( (auto_test.month == 4 || auto_test.month == 6 || auto_test.month == 9 || auto_test.month == 11) && auto_test.day > 30 ) {
		auto_test.day= 30;
	}


}


void bat_set_auto_test_day()
{

	//printf("%s\n", __FUNCTION__);


	get_system_time(&sys_info);

	//printf("op:%d sys:%d", auto_test.month , sys_info.month);
	auto_test.month = op_sts.month;
	//printf("op:%d sys:%d", auto_test.month , sys_info.month);
	if ( op_sts.month < sys_info.month) {
		while( auto_test.month < sys_info.month) {
			//printf("op:%d sys:%d", auto_test.month , sys_info.month);
			auto_test.month += op_sts.bat_test_period;
		}
		op_sts.month = auto_test.month;
	}
	//else 
	if ( op_sts.month ==  sys_info.month ) {
		if ( op_sts.date < sys_info.day )   {
			auto_test.month += op_sts.bat_test_period;
		}
		else if ( op_sts.date == sys_info.day ) {
			if ( op_sts.hour < sys_info.hour)  {
				auto_test.month += op_sts.bat_test_period;
			}
			else if( op_sts.hour == sys_info.hour ) {
		  		if ( op_sts.min <= sys_info.min ) {
		  			auto_test.month += op_sts.bat_test_period;
		  		}
			}
		}
	}

	op_sts.month = auto_test.month;
	
	if (auto_test.month > 12) {
			auto_test.month -= 12;
	}
	auto_test.day = op_sts.date;
	auto_test.hour = op_sts.hour;
	auto_test.min = op_sts.min;
	
	if ( auto_test.month == 2  && auto_test.day > 28 ) {
		auto_test.day = 28;
	}
	else if ( (auto_test.month == 4 || auto_test.month == 6 || auto_test.month == 9 || auto_test.month == 11) && op_sts.date > 30 ) {
		auto_test.day= 30;
	}

	auto_test.v = op_sts.bat_test_V;
	bat_test_save_time();
	printf("Auto Test : %d/%d,%d:%d\n", auto_test.month, auto_test.day, auto_test.hour, auto_test.min);
}

void	bat_test_set_period( int month)
{
	int pvalue;
	
	pvalue = op_sts.bat_test_period ;
	op_sts.bat_test_period = month;
	set_bat_test_log(LOG_BAT_TEST_PERIOD,  pvalue, month);
}

u32	get_bat_test_time()	// secs
{
	bat_test_end_time = get_time_counter();
	//printf("BAT TEST end time = %d, %d\n", bat_test_end_time, bat_test_end_time - bat_test_begin_time);
	return ((bat_test_end_time - bat_test_begin_time)) ; 
}


int	bat_test_condition()
{
	if ( alarm_sts.input_V || alarm_sts.output_V || alarm_sts.bat_V || alarm_sts.bat_NFB || alarm_sts.bat_disconnect ) return 0;
	return 1;
}


void	bat_auto_test_start()
{
	bat_test_start();
	bat_update_auto_test_day();
}

void bat_ip_test_start()
{
	save_test_V = op_sts.bat_test_V;
	op_sts.bat_test_V = ip_bat_test_V;
	printf("IP TEST V %5.1f\n", op_sts.bat_test_V);
}

void bat_ip_test_end()
{
	op_sts.bat_test_V = save_test_V ;
}


void bat_ip_test()
{
	printf("BATTEST:%d,%5.1f\n", ip_bat_test_time, ip_bat_test_V);
	set_bat_test_log(LOG_BAT_TEST_START, 1, 0);
	//op_sts.bat_testing = 1;
	auto_test_time = time10ms + ip_bat_test_time * 100;
}

void	bat_test_start()
{
	sr_off();
	//alarm_sts.cell_fail = 0;
	set_bat_test_log(LOG_BAT_TEST_START, 0, 0);
	op_sts.bat_testing = 1;
	auto_test_time = time10ms + BAT_TEST_TIME;		
}

int	bat_test_end()
{
	float bat_v;
	
	bat_v = rect_sts.output_V;


	if ( ip_bat_test ) {
		if ( ip_bat_test_stop ) {
			set_bat_test_log(LOG_BAT_TEST_STOP,  0, 0);
			bat_ip_test_end();
			ip_bat_test = 0;
			printf("IP STOP\n");
			return 1;
		}
		if ( bat_v > op_sts.bat_test_V ) {		
			if ( auto_test_time  <= time10ms ) {
				set_bat_test_log(LOG_BAT_TEST_DONE, 0, 0);	
				rect_sts.cell_fail &= ~BAT_STS_TEST_FAIL;
				printf("IP DONE: %d\n", op_sts.bat_testing);
				ip_battery_test_data(ip_bat_test_time);
				bat_ip_test_end();
				return 1;
			}
		}
		else {
			set_bat_test_log(LOG_BAT_TEST_STOP,  1, 0);
			printf("IP CELL FAIL %5.2f\n", bat_v);
			rect_sts.cell_fail |= BAT_STS_TEST_FAIL;;
			ip_battery_test_data(ip_bat_test_time);
			bat_ip_test_end();
			return 1;
		}
		return 0;
	}

	
	if ( forced_bat_test ) {
		if ( bat_v > op_sts.bat_test_V ) {		
			if ( auto_test_time  <= time10ms ) {
				set_bat_test_log(LOG_BAT_TEST_DONE, 0, 0);	
				rect_sts.cell_fail = 0;
				printf("DONE: %d\n", op_sts.bat_testing);
				forced_bat_test = 0;
				return 1;
			}
			return 0;
		}
		else {
			set_bat_test_log(LOG_BAT_TEST_STOP,  1, 0);
			printf("CELL FAIL %5.2f\n", bat_v);
			rect_sts.cell_fail |= BAT_STS_TEST_FAIL;
			forced_bat_test = 0;
			return 1;
		}
		
	}

	if ( op_sts.bat_test_mode  ) {	
		if ( bat_v > op_sts.bat_test_V ) {		
			if ( auto_test_time  <= time10ms ) {
				set_bat_test_log(LOG_BAT_TEST_DONE, 0, 0);	
				rect_sts.cell_fail &= ~BAT_STS_TEST_FAIL;
				printf("OP DONE: %d\n", op_sts.bat_testing);
				return 1;
			}
		}
		else {
			set_bat_test_log(LOG_BAT_TEST_STOP,  1, 0);
			printf("OP CELL FAIL %5.2f\n", bat_v);
			rect_sts.cell_fail |= BAT_STS_TEST_FAIL;
			return 1;
		}
	}
	else if ( op_sts.bat_test_mode == 0  ) {
		set_bat_test_log(LOG_BAT_TEST_STOP,  0, 0);
		printf("STOP\n");
		return 1;		
	}	

	return 0;
}


void	check_bat_test()
{
	static int bat_testing = 0;
	static unsigned long long	bat_wait_time ;

	if ( op_sts.bat_equip ) {		
		//printf("BAT ALARM MASK!!!");
		return;
	}


	float charge_a;
	charge_a = rect_sts.bat_A;

	if ( alarm_sts.input_V || alarm_sts.output_V || 
		 alarm_sts.module_fail || alarm_sts.bat_NFB || 
		  alarm_sts.bat_disconnect || charge_a > 3.0 )  {

		if ( bat_testing == 0 && op_sts.bat_testing == 0) return;
		if ( bat_testing != 5 ) {
			set_bat_test_log(LOG_BAT_TEST_STOP,  2, 0);
			rect_sts.bat_test = 0;
			write_output_voltage(op_sts.output_V);	// return to settup voltage
			op_sts.bat_testing = 0;
			bat_wait_time = time10ms + 100;
			printf("BAT TEST STOP \n");
			bat_testing = 5;
			return ;
		}
	}
	
	if (bat_testing == 0 ) {
		if ( op_sts.bat_testing ||  ip_bat_test ) {	// forced battery test
			//printf("BAT TEST %d\n", bat_testing);
			bat_testing = 4;
			bat_wait_time = time10ms + 100;
            if ( ip_bat_test ) bat_ip_test_start();
			return;
		}
		if ( op_sts.bat_test_mode == 0 ) return;
		get_system_time(&sys_info);
		
		#if 1
		if ( sys_info.month == auto_test.month &&
			sys_info.day   == auto_test.day &&
			sys_info.hour  == auto_test.hour &&
			sys_info.min   == auto_test.min ) {
			
			bat_test_begin_time = get_time_counter();
			bat_auto_test_start();
			bat_wait_time = time10ms + 100;
			bat_testing = 4;
			//printf("BAT AUTO TEST Time : %d\n",bat_test_begin_time);
		}

		#endif

	}
	else if ( bat_testing == 4 ) {
		if ( bat_wait_time  <= time10ms ) {
			//printf("BAT TEST %d\n", bat_testing);
			write_output_voltage(op_sts.bat_test_V - 0.5);
			rect_sts.bat_test = 1;
			bat_testing = 1;
		}
	}
	else if (bat_testing == 1 ) {
		 if ( bat_test_end() == 1 ) {
		 	//printf("BAT TEST %d\n", bat_testing);
		 	bat_testing  = 2;	
			write_output_voltage(op_sts.output_V);	// return to settup voltage
			rect_sts.bat_test = 0;
			bat_wait_time = time10ms + 100;
		 }
	}
	else if ( bat_testing == 2 ) {
		if ( bat_wait_time  <= time10ms ) {
			//printf("BAT TEST %d\n", bat_testing);
			op_sts.bat_testing = 0;	
			rect_sts.bat_test = 0;
			sr_on();
			bat_testing = 0;
		}
	}
	else if ( bat_testing == 5 ) {
		if ( bat_wait_time  <= time10ms ) {
			//printf("BAT TEST %d\n", bat_testing);
			rect_sts.bat_test = 0;
			op_sts.bat_testing = 0;	
			sr_on();
			bat_testing = 0;
		}
	}
	else  {
		printf ("BAT test ERROR");
		bat_testing = 0;
		rect_sts.bat_test = 0;
	}

}


void	rabm_main()
{
	int	command;
	u16	alarm, alarm_save;

	int count, rec;
	//extern u16	operation_mode;
	char no;
	u16 ac_int, temp;

     
	alarm_timer_init();
	vref_init();
	//rabm_init();
	//set_alarm_port(0);
	ADC_Configuration_scanmode();
	adc_ready = 1;


	avg_ref(); //get_Vref();
	
#if 0
	IWDG_WriteAccessCmd(0x5555);
	IWDG_SetPrescaler( IWDG_Prescaler_32);
	IWDG_SetReload(0xfff);	// 3276.8 msec
	IWDG_Enable();
#endif	
	time3ms = 0;

	time_tmp = time10ms;
	printf("MAIN LOOP");

	while (1) {
		if ( time_tmp == time10ms) {
			continue;
		}
		time_tmp = time10ms;

			avg_ref();
			do_test_mode();
			dac_test();
			//set_output_status();
			check_port_status();
			//get_rect_status(&rect_sts);
			IWDG_ReloadCounter();

			if ( !(time10ms % 20 ) ) {
				display_status();
			}

			if ( A1_flag == 1 ) {
				printf("INT A1\n");
				A1_flag = 0;
			}		
	}

}

void	wait(unsigned long time)	//1// time * 10ms
{
	time = time + time10ms;
	while (1) {
		if ( time < time10ms ) break;
	}
}


void WD_reset()
{

	//GPIO_ToggleBits(GPIO_WD, WD_PIN);
	GPIO_WriteBit(GPIO_WD, WD_PIN, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIO_WD, WD_PIN)));
	
}


