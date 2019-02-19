
#include 	"stm32f10x.h"
//#include 	"stm32_eval.h"
#include 	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include 	<stdint.h>
#include	"util.h"
#include	"rabm.h"
#include	<time.h>
#include	"eeprom.h"
#include	"alarmComm.h"
#include	"W5500\telnet.h"
#include	"W5500\loopback.h"
#include	"W5500\dhcp.h"
#include	"W5500\w5500.h"
#include	"w5500\types.h"
#include	"w5500\wiz_config.h"
#include	"w5500\socket.h"
#include	"ping.h"

#define MD5
#include	"md5.h"

//#define	 DEBUG_PRINT
#ifdef DEBUG_PRINT
#define Dprintf printf 
#else
#define Dprintf
#endif

extern int eth_flag;
extern uint32_t test_value;

void	ip_send_msg( void *msg, int length,  int dest_port);
void	system_shutdown();
void 	ip_alarm_noti();
void	check_system_restart();
void tx_udp(SOCKET s, uint16 port, UDP_PACKET *tx);
int rx_udp(SOCKET s, uint16 port, UDP_PACKET *rx);
void	set_management_id(u32 id);
int fusing_check_file(byte *data);


uint32_t my_time;

static unsigned long long rsp_timer = 0;
static int rsp_flag = 0;


extern char dhcp_state;
extern uint16_t due_state;


u16	operation_mode = NORMAL_MODE;	// NORMAL_MODE

UDP_PACKET	udp_rx, udp_tx;
//byte	tx_buffer[1500];

int		packet_header, packet_tail;
//byte	ump_mac[6] = {0x02, 0, 0, 0, 0, 0};
byte	psu_mac[6] = {0x02, 0, 0, 0, 0, 0};

byte	gw_ip[4] = {192, 168, 0, 1};
byte	subnet_mask[4] = { 255, 0, 0, 0};	// 255.255.0.0 -> 255.0.0.0

byte	ump_ip[4] = {255, 255, 255, 255};
byte	psu_ip[4] = {192, 168, 255, 255};
u16		psu_port	= 0x2000;	// D = 8192
u16		ump_port	= 0x5071;
u16		psu_mailbox = 0x2000;	// ????
u16		ump_mailbox = 0x5071;
SOCKET	psu_socket 	= 5;	// udp : RX = 5, TX = 6
SOCKET telnet_socket = 1;	// telnet : socket :  1, 2
SOCKET dhcp_socket = 3;		// DHCP : 3
SOCKET ping_socket = 4;
u16		telnet_port = 23;
uint8_t ping_addr[4] = { 192, 168, 1, 1}, ping_rx_addr[4];
uint8_t du_ip[4] = {192, 168, 16, 33};



u_char	mac_company[3] = { 0x00, 0x08, 0xdc};		// wiz_net mac code

extern	unsigned long long time10ms;
extern	ALARM_STATUS	alarm_sts;
extern  OP_STATUS		op_sts;
extern 	LOG_STATUS	log_sts;
extern	RECT_STATUS	rect_sts;

static	int	ip_tx_flag = 0;
extern	int alarm_notification_flag;

extern	u32	bat_test_time;
extern int	ip_changed_flag;
int management_id_flag = 0;

int 	ip_bat_test_time,  ip_bat_test_stop = 0;
float	ip_bat_test_V;

const	char const_hw_desc[80] = "TH;TH;SK 5G 48V Rectifier;PSU_6S;"; // R6000
char	hw_desc[244];


extern int		hw_version, fw_version;	// decimal no. 0x01020304-> v01.02.03.04
int		reboot_delay = 3000;	// 3sec unit = [ms]
byte	memo_info[128];
REPAIR_INFO	repair_info[5];
CN_INFO	CN_info[5];
int	 	ip_bat_test = 0;

SOCKET_REGS	sockets[8];

// 2014 0107 v0.70

unsigned char user_psu_name[100];
unsigned long long	shutdown_time = 0, restart_time = 0;
extern int	shutdown_flag;
extern 	int restart_flag;



// =============================================================================
//	from wiz main.

//CONFIG_MSG Config_Msg;
//CHCONFIG_TYPE_DEF Chconfig_Type_Def; 

// Configuration Network Information of W5200

uint8 Enable_DHCP = OFF;
//uint8 MAC[6] = {0x02, 0x08, 0xDC, 0x01, 0x02, 0x03};//MAC Address

#if 0
uint8 IP[4] = {192, 168, 2, 4};//IP Address
uint8 GateWay[4] = {192, 168, 2, 1};//Gateway Address
uint8 SubNet[4] = {255, 255, 0, 0};//SubnetMask Address
#endif

//TX MEM SIZE- SOCKET 0:8KB, SOCKET 1:2KB, SOCKET2-7:1KB
//RX MEM SIZE- SOCKET 0:8KB, SOCKET 1:2KB, SOCKET2-7:1KB
uint8 txsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};
uint8 rxsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};

//FOR TCP Client
//Configuration Network Information of TEST PC
uint8 Dest_IP[4] = {192, 168, 128, 2}; //DST_IP Address 
uint16 Dest_PORT = 3000; //DST_IP port

uint8 ch_status[MAX_SOCK_NUM] = { 0, };	/** 0:close, 1:ready, 2:connected */

uint8 TX_BUF[TX_RX_MAX_BUF_SIZE]; // TX Buffer for applications
uint8 RX_BUF[TX_RX_MAX_BUF_SIZE]; // RX Buffer for applications

byte	dipsw_ip[4], saved_ip[4];
byte	dipsw_mac[6], saved_mac[6];
byte	saved_management_id[4];
uint32_t	management_id = 0;


int bat_seq_no, bat_id, bat_port =0;

md5_context ctx;
char md5_output[16];
char md5_input[32];

   
extern SOCKET DHCPC_SOCK;
typedef struct {
	byte version;
	byte reserved;
	uint16_t msg_id;
	unsigned char i4[3]; //(Not used, reserved)
	unsigned char d4[3]; //(Not used, reserved)
	unsigned char o4[3];
	unsigned char rsv[3];
} DUE_REQ_T, DUE_RSP_T;

DUE_REQ_T due_req_p;
DUE_RSP_T due_rsp_p;

#define MSG_DUE_REQ	0x26d1
#define MSG_DUE_RSP	0x26d0

#define DUE_PORT	0x42F8

#define DUE_TEST	0
#define DUE_TX_REQUEST		1
#define DUE_WAIT_RSP	2
#define DUE_RX_RSP	3
#define DUE_RETRY	4
#define DUE_SUCCESS	5
#define DUE_OK		7
#define DUE_WAIT_TEST 8


#define DUE_TEST_TIME	(100 * 10)
#define DUE_NEXT_TIME	1000	// 1000msec
#define DUE_WAIT_TIME	300	// 500msec

uint16_t due_state = 0;
unsigned long long due_time;

   
// =============================================================================


unsigned char reverse_byte(unsigned char b) 
{
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

#if 0

/*******************************************************************************
* Function Name  : Delay_us
* Description    : Delay per micro second.
* Input          : time_us
* Output         : None
* Return         : None
*******************************************************************************/

void Delay_us( u8 time_us )
{
  register u8 i;
  register u8 j;
  for( i=0;i<time_us;i++ )    
  {
    for( j=0;j<5;j++ )          // 25CLK
    {
      asm("nop");       //1CLK         
      asm("nop");       //1CLK         
      asm("nop");       //1CLK         
      asm("nop");       //1CLK         
      asm("nop");       //1CLK                  
    }      
  }                              // 25CLK*0.04us=1us
}

/*******************************************************************************
* Function Name  : Delay_ms
* Description    : Delay per mili second.
* Input          : time_ms
* Output         : None
* Return         : None
*******************************************************************************/

void Delay_ms( u16 time_ms )	// 45  = 10msec
{
  register u16 i;
  for( i=0;i<time_ms;i++ )
  {
    Delay_us(250);
    Delay_us(250);
    Delay_us(250);
    //Delay_us(250);
  }
}
#endif

void Timer4_sec_ISR(void)
{
		my_time++;
}


int ctoi(char c)
{
    return c - '0' - (c > '@') * 7;
}


int flash_write(int addr, byte * data, int length)
{
	int	i;
	uint32_t *wdata;	// int

	wdata = (uint32_t *) data;
	FLASH_Unlock();
	for ( i = 0; i < (length+3)/4; i++) {
		FLASH_ProgramWord(addr, *wdata );
		if ( *(int *) addr != *wdata ){
			FLASH_Lock();
			printf("FLASH WERR:%x,%x\n", *(int *) addr, *wdata );			
			return -1;
		}		
		addr += 4;
		wdata++;
	}
	
	FLASH_Lock();
	//printf("F:%x,%x\n", *(int *)(addr-4), *(wdata-1) );
	return 0;

}

int ip_set_memo_info(byte *data)
{
	int ret;

	ret = write_eeprom(MEMO_INFO_ADDR, data, MEMO_INFO_SIZE);
	IWDG_ReloadCounter();
	dump_memory(data, MEMO_INFO_SIZE);
	return ret;
}


void	ip_read_due_ip()
{
	uint8_t ip[4];
	
	read_eeprom( IP_DUE_ADDR, ip, 4);
	if ( psu_ip[0] == 0xff || psu_ip[1] == 0 ) {
		psu_ip[0] = 192;
		psu_ip[1] = 168;	
		psu_ip[2] = 16;
		psu_ip[3] = 37;
	}
	else {
		psu_ip[0] = ip[3];
		psu_ip[1] = ip[2];	
		psu_ip[2] = ip[1];
		psu_ip[3] = ip[0];
	}
	printf("DUE ADDR %x\n", *(int*)psu_ip);
}


void	ip_save_due_ip()
{
	uint8_t ip[4];
	
	ip[3]= psu_ip[0];
	ip[2]= psu_ip[1];	
	ip[1]= psu_ip[2];
	ip[0]= psu_ip[3];
	
	write_eeprom( IP_DUE_ADDR, ip, 4);
	printf("SAVE DUE ADDR %x\n", *(int*)psu_ip);
}



void	ip_update_flash_ip(int addr)
{
	saved_ip[3] =  addr  & 0xff;;
	saved_ip[2] = ( addr >> 8 ) & 0xff;;
	saved_ip[1] = ( addr >> 16 ) & 0xff;
	saved_ip[0] = ( addr >> 24 ) & 0xff;
	set_mac_address(saved_mac, saved_ip);
	sys_write_ip_info(addr);	// save psu_ip to flash memory
	//Reset_W5200();
	//wiz_init(); 

}	



void	save_psu_ip(uint8 *ip)
{
	psu_ip[0] = *ip++;
	psu_ip[1] = *ip++;
	psu_ip[2] = *ip++;
	psu_ip[3] = *ip++;

}

void	save_gw_ip(uint8 *ip)
{
	gw_ip[0] = *ip++;
	gw_ip[1] = *ip++;
	gw_ip[2] = *ip++;
	gw_ip[3] = *ip++;

}

void	save_subnet_mask(uint8 *ip)
{
	subnet_mask[0] = *ip++;
	subnet_mask[1] = *ip++;
	subnet_mask[2] = *ip++;
	subnet_mask[3] = *ip++;

}



void	set_wiz_network()
{
	uint8 tmp_array[6];     

	setSHAR(psu_mac);
	saveSUBR(subnet_mask);
	setSUBR(subnet_mask);
	setGAR(gw_ip);
	setSIPR(psu_ip);
	
	//Set PTR and RCR register	
	setRTR(6000);
	setRCR(3);

	//Init. TX & RX Memory size
	sysinit(txsize, rxsize); 
		

	Delay_ms(1);
	getSIPR (tmp_array);
	printf("\r\nIP : %d.%d.%d.%d", tmp_array[0],tmp_array[1],tmp_array[2],tmp_array[3]);
	Delay_ms(1);
	getSUBR(tmp_array);
	printf("\r\nSN : %d.%d.%d.%d", tmp_array[0],tmp_array[1],tmp_array[2],tmp_array[3]);
	Delay_ms(1);
	getGAR(tmp_array);
	printf("\r\nGW : %d.%d.%d.%d", tmp_array[0],tmp_array[1],tmp_array[2],tmp_array[3]);
	Delay_ms(1);

	getSHAR(tmp_array);
	printf("\r\nMAC : %.2X.%.2X.%.2X.%.2X.%.2X.%.2X", tmp_array[0],tmp_array[1],tmp_array[2],tmp_array[3],tmp_array[4],tmp_array[5]);
	//printf("\r\nMAC : %.2X.%.2X.%.2X.%.2X.%.2X.%.2X", IINCHIP_READ(SHAR0+0),IINCHIP_READ(SHAR0+1),IINCHIP_READ(SHAR0+2),IINCHIP_READ(SHAR0+3),IINCHIP_READ(SHAR0+4),IINCHIP_READ(SHAR0+5));

}

void	start_udp_port()
{

	if  ( getSn_SR(psu_socket) == SOCK_CLOSED ) 
	rx_udp( psu_socket, psu_port, (UDP_PACKET *) RX_BUF);
	if  ( getSn_SR(psu_socket+1) == SOCK_CLOSED )
	tx_udp( psu_socket+1, psu_port, (UDP_PACKET *) TX_BUF);
}


void Reset_W5200(void)
{
	GPIO_ResetBits(GPIOB, WIZ_RESET);
	Delay_ms(5);  
	GPIO_SetBits(GPIOB, WIZ_RESET);
	Delay_ms(100);  
}


void	delay5()
{
	int i;

	for ( i = 0; i < 5; i++)
	{
		asm("nop");
	}

}

#if 0
u16	ip_read_switch()
{
	u16	addr1, addr;



	addr = 0xc0a88002;


	printf("DIP = %x \n", addr);
	return addr;	// 
}
#endif


#if 1

void	set_subnet_mask(byte mask[])
{
	saveSUBR(mask);
	setSUBR(mask);
}

void	set_socket_regs(SOCKET_REGS	*s)
{
	s->rMR = Sn_MR_UDP;
	s->rIR = Sn_IR_SEND_OK | Sn_IR_TIMEOUT | Sn_IR_RECV | Sn_IR_DISCON | Sn_IR_CON;	
}

byte	psu_mr = 0x00;
u16		psu_rtr = 2000;	// 200msec
u16		psu_rcr	= 8;

void  ip_init()
{
	iinchip_init();

	setMR(psu_mr);
	setGAR(gw_ip);
	set_subnet_mask(subnet_mask);
	setSHAR(psu_mac);	// psu mac address
	setSIPR(psu_ip);	// source ip address

	set_socket_regs(sockets);
}


#endif
u32	ip_next_seq_no()
{
	static int	seq_no = 0;
	int seq;

	seq = seq_no++;
	if ( seq_no == 0xffff ) seq_no = 0;
	return seq;	
}


u32	ip_get_psu_status( u32 value[], byte data[])
{
	RECT_STATUS  *status;
	int v;

	status = &rect_sts;

	v = _round ( status->input_V * 10.0);	// AC input voltagem 2Phase AC or 3 Phase AC R
	if ( v < 0 || v > 3000 ) v = 0;
	value[0] = set_int(v);
	value[1] = 0;	// dummy
	v = _round (status->output_V * 10.0);	// DC input voltage
	if ( v < 0 || v> 600 ) v = 0;
	value[2] = set_int(v);
	
	v = _round (status->output_A * 10.0) ;	// DC input current
	if ( v < 0 || v > 5000 ) v = 0;
	value[3] = set_int(v);

	if ( op_sts.bat_equip ){
	value[4] = 0;
	value[5] = 0;
	}
	else {
	v =  ip_bat_test_residual();	// battery residual rate 0~100 %
	if ( v < 0 || v > 1000 ) v = 0;
	value[4] = set_int(v);
	
	v = _round (status->bat_V * 10.0) ;	// battery residual voltage
	if ( v < 0 || v > 600 ) v = 0;
	value[5] = set_int(v);
	}


	v  = _round (status->rec_T * 10.0);	// rectifire temperature
	if ( v < -400 || v > 1200 ) v = -10000;
	value[6] = set_int(v);
	
	value[7] = 0;	// 3Phase AC S input voltage
	value[8] = 0;	// dummy
	value[9] = 0;	// 3Phase AC T input voltage
	
	if ( op_sts.bat_equip ){	
	value[10] = 0xffff;	// battery conf. (배터리미사용)
	}
	else {
	value[10] = 0;	// battery conf.
	}

	value[11] = set_int(0);	// rectifier description : 2Phase = 0, 3Phase = 1

	data[0] = 1;	// 0: not equip, 1:equip, 2:unequip
	data[1] = 0;	// 0: not equip, 1:equip, 2:unequip
	data[2] = 0;	// dummy
 	data[3] = 0;	// dummy
 	data[4] = 1;	//Battery type : LIthume fof Compact psu
	data[5] = status->bat_test;	// Baterry test status,  0: normat, 1:testing
	data[6] = 7;	//PSU_6S= 7
	data[7] = get_auto_test_mode(); // v7.0

	for( v = 8; v < 16; v++) {	// data size 16-> 24 By v0.60
		data[v] = 0;	
	}

	data[16] = 0xef;	// 76~77
	data[17] = 0xff;
	
	data[18] = 0xef;	// 78~79
	data[19] = 0xff;

	data[16] = 0xef;	// 80~81
	data[17] = 0xff;
	
	data[18] = 0xef;	// 82~83
	data[19] = 0xff;

	data[20] = 0xef;	// 84~85
	data[21] = 0xff;

	data[22] = 0xef;	// 86~87
	data[23] = 0xff;

	return 0;

}


u32	ip_get_alarm(ALARM_STATUS	*alarm)
{
	u32	status = IP_ALARM_RECT_COMPACT; // 5G (PSU_6S) IP_ALARM_RECT_COMPACT;

	//check_alarm(alarm);
	//check_power_cut(alarm);
	
	if ( alarm->input_V == ALARM_HIGH ) status |= IP_ALARM_AC_VOLTAGE;
	else if ( alarm->input_V == ALARM_LOW ) status |= IP_ALARM_AC_VOLTAGE;

	if ( alarm->output_V == ALARM_HIGH ) status |= IP_ALARM_DC_HIGH_VOLTAGE;
	else if ( alarm->output_V == ALARM_LOW ) status |= IP_ALARM_DC_LOW_VOLTAGE;

	if ( alarm->bat_T == ALARM_HIGH ) status |= IP_ALARM_REC_TEMPERATURE;
	if ( alarm->rec_T == ALARM_HIGH ) status |= IP_ALARM_REC_TEMPERATURE;


	//if ( alarm->door_open != 0 ) status |= IP_ALARM_RECT_DOOR;
	if ( op_sts.bat_equip == 0 ) {
		if ( alarm->bat_NFB != 0 || alarm->bat_disconnect != 0 )  {
			status |= IP_ALARM_BAT_DISCONNECTION;	//ALARM_BAT_NFB;
		}
		if ( alarm->bat_V == ALARM_LOW ) status |= IP_ALARM_BAT_DISCHARGE;
		if ( alarm->cell_fail != 0 ) status |= IP_ALARM_BAT_QUALITY_LOW;	//DISCHARGE; //IP_ALARM_BAT_ABNORMAL;
	}
	#if 0	// 190114
	else {
	status |= IP_ALARM_BAT_DISCONNECTION;
	}
	#endif
	
	//if ( alarm->out_NFB != 0 ) status |= ALARM_OUT_NFB;


	if ( alarm->power_cut == 1 ) status |= IP_ALARM_POWER_OUT;
	if ( alarm->module_fail !=0 ) status |= IP_ALARM_MODULE_FAIL_0;
	if ( alarm->ocp !=0 ) status |= IP_ALARM_DC_OVER_CURRENT;
	//if ( alarm->bat_bql  != 0 )  status |= IP_ALARM_BAT_QUALITY_LOW;
	
	return status;
}


int	read_repair_info()
{
	int i, rp_addr;

	rp_addr = REPAIR_INFO_ADDR ;
	for ( i= 0; i< 5; i++) {
		read_eeprom(rp_addr, (byte *) &repair_info[i], REPAIR_INFO_SIZE);
		rp_addr += REPAIR_INFO_SIZE;
		IWDG_ReloadCounter();
	}
	return 0;
}

int	save_repair_info(byte *set, int no)
{
	int	rp_addr;

	//printf("Save_Repair_info = %d\n", no);

	rp_addr = REPAIR_INFO_ADDR + REPAIR_INFO_SIZE * no;
	memcpy(&repair_info[no], set, REPAIR_INFO_SIZE);
	write_eeprom(rp_addr, (byte *) &repair_info[no], REPAIR_INFO_SIZE);

	IWDG_ReloadCounter();
	dump_memory((byte *) &repair_info[no], REPAIR_INFO_SIZE);

	return 0;

}

int	ip_set_repair_info(byte *set)
{
	REPAIR_INFO *rp;
	int set_no;
	//int save_no, i;

	rp = (REPAIR_INFO *) set;
	set_no =  atoi(rp->num)-1;
	//read_repair_info();

	if ( set_no < 0 || set_no > 4 ) return -4;
#if 0
	for (i = 0; i < 5 ; i++) {
		rp = &repair_info[i];
		//save_no = byte2int(rp->num, 0);
		save_no = atoi(rp->num);
		if ( save_no == set_no) {
			save_no = i; 
			break;
		}
	}

	if ( i >= 5 ) {
		for (i = 0; i < 5 ; i++) {
			rp = &repair_info[i];
			save_no = atoi(rp->num);
			if ( save_no == 0) {
				save_no = i;
				break;
			}
		}
	}
#endif
	save_repair_info( (byte *)rp, set_no);

	return 0;

}

int ip_battery_test_start(int voltage, int time)
{
	float test_v;
	int 	ret = 0;

	if ( time == 0 ) {
		if ( ip_bat_test == 1 ) ip_bat_test_stop = 1;
		//printf("Test stop\n");
		return 0;
	}

	ip_bat_test_stop = 0;
	test_v = voltage / 10.0;
	if ( time < 0 || time > 120 ) return -202;
	if ( test_v < 21.0 || test_v > 53.3 ) return -201;



	if ( alarm_sts.input_V || alarm_sts.output_V || alarm_sts.bat_V || alarm_sts.bat_NFB || alarm_sts.bat_disconnect ) ret = -212;
	else if ( op_sts.bat_testing == 1 ) ret = -301;

	if ( read_bat_A() >= 2.0 ) {
		ret = -211;	// under bat. charging 
	}

	if ( ret ) {
		//printf("Abort %d\n", ret);
		return ret;
	}
		
	ip_bat_test_time = time * 60;	// in sec  (time * 60)
	ip_bat_test_V = test_v;

	//write_output_voltage(ip_bat_test_V);
	//op_sts.bat_testing = 1;
	ip_bat_test = 1;

	bat_ip_test();
	//set_bat_test_log(LOG_BAT_TEST_START, 0, 0);
	//printf("BATeststart %5.2fV,%dSec\n", ip_bat_test_V, ip_bat_test_time);
	return 0;
}

int ip_get_repair_info(int start, int end, byte *set)
{

	int i, rp_addr, ret = 0, no;

	//printf("Get Repair info : start = %d, end = %d \n", start, end);
	if ( start < 1 || end > 5 || start > end ) return -4;

	rp_addr = REPAIR_INFO_ADDR + REPAIR_INFO_SIZE * (start -1);
	for ( i = start; i <= end; i++) {
		read_eeprom(rp_addr, (byte *)&repair_info[i-1], 4);
		no = atoi((char *)&repair_info[i-1]);
		if ( no > 0 && no < 6) {
			read_eeprom(rp_addr+4, (byte *)&repair_info[i-1] +4, REPAIR_INFO_SIZE -4);
			//dump_memory((byte *)&repair_info[i-1], REPAIR_INFO_SIZE);
			memcpy(set, &repair_info[i-1], REPAIR_INFO_SIZE);
			set += REPAIR_INFO_SIZE;
			ret++;
		}	
		rp_addr += REPAIR_INFO_SIZE;
		IWDG_ReloadCounter();
	}

	if (ret == 0 ) ret = -5;
	return ret;

}

int	read_cn_info()
{
	int i, cn_addr;

	cn_addr = CN_INFO_ADDR ;
	for ( i= 0; i< 5; i++) {
		read_eeprom(cn_addr, (byte *)&CN_info[i], CN_INFO_SIZE);
		cn_addr += CN_INFO_SIZE;
		//dprintf("CN info : %d \n", i+1);
		dump_memory((byte *)&CN_info[i], CN_INFO_SIZE);
		IWDG_ReloadCounter();
	}
	return 0;
}

int	save_cn_info(byte *set, int no)
{
	int	cn_addr;

	cn_addr = CN_INFO_ADDR + CN_INFO_SIZE * no;
	memcpy(&CN_info[no], set, CN_INFO_SIZE);
	dump_memory((byte *)&CN_info[no], CN_INFO_SIZE);
	write_eeprom(cn_addr, (byte *)&CN_info[no], CN_INFO_SIZE);

	IWDG_ReloadCounter();

	//dprintf("save_cn_info = %d\n", no);
	
	dump_memory(set, CN_INFO_SIZE);
	return 0;
}

int ip_set_cn_info(byte *set)
{
	CN_INFO *cn;
	int	set_no;
	//int	save_no, i;

	cn = (CN_INFO *) set;
	dump_memory((byte *)cn, CN_INFO_SIZE);

	
	set_no =  atoi(cn->num) -1;;
	if ( set_no < 0 || set_no > 4 ) return -4;

	//read_cn_info();

	#if 0
	for (i = 0; i < 5 ; i++) {
		cn = &CN_info[i];
		save_no = atoi(cn->num);
		if ( save_no == set_no) {
			save_no = i; 
			break;
		}
	}

	if ( i >= 5 ) {
		for (i = 0; i < 5 ; i++) {
			cn =  &CN_info[i];
			save_no = atoi(cn->num);
			if ( save_no == 0) {
				save_no = i;
				break;
			}
		}
	}
	#endif

	//printf("set no = %d, save no = %d\n", set_no, save_no);
	save_cn_info( set, set_no);

	return 0;
}


int ip_get_cn_info(int start, int end, byte *set)
{
	int	i, cn_addr, no, ret = 0;

	//printf("Get CN Info start = %d,end = %d\n", start, end);

	if ( start < 1 || end > 5 || start > end ) return -4;

	cn_addr = CN_INFO_ADDR + CN_INFO_SIZE * (start -1);
	for ( i = start; i <= end; i++) {
		read_eeprom(cn_addr, (byte *)&CN_info[i-1], 4);
		no = atoi((char *)&CN_info[i-1]);
		if ( no > 0 && no < 6) {
			ret++;
			read_eeprom(cn_addr+4, (byte *)&CN_info[i-1] + 4, CN_INFO_SIZE -4);
			memcpy(set, &CN_info[i-1], CN_INFO_SIZE);
			dump_memory((byte *)&CN_info[i-1], CN_INFO_SIZE);
			set += CN_INFO_SIZE;
		}	
		cn_addr += CN_INFO_SIZE;
		IWDG_ReloadCounter();
	}

	if ( ret == 0 ) ret = -5;	// no valid CN info set
	return ret;
}

/*
int ip_set_memo_info(byte *data)
{
	int ret;

	ret = write_eeprom(MEMO_INFO_ADDR, data, MEMO_INFO_SIZE);
	IWDG_ReloadCounter();
	dump_memory(data, MEMO_INFO_SIZE);
	return ret;
}
*/

int ip_get_memo_info(byte *data)
{
	int ret;
	
	ret = read_eeprom(MEMO_INFO_ADDR, data, MEMO_INFO_SIZE);
	dump_memory(data, MEMO_INFO_SIZE);

	return ret;
}

u32 byte2int(byte data[], int index)
{
	int ret;

	#ifdef BIG_ENDIAN
		ret = (data[index+0]& 0xff) << 24 | (data[index+1] & 0xff) << 16 | (data[index+2]& 0xff) << 8 | (data[index+3]&0xff) ;
	#else
		ret =  *(u32 *) (&data[index]);
	#endif
	
	//printf("data = %x, int = %x\n", *(u32 *) (&data[index]), ret );
	return ret;
}

int byte2word(byte data[], int index)
{
	int ret;
	
	#ifdef BIG_ENDIAN
		ret = (data[index+0] & 0xff) << 8 | (data[index+1] & 0xff) ;	
	#else
		ret =  *(u16 *) (& data[index]);
	#endif

	//printf("data = %x, word = %x\n", *(u16 *) (&data[index]), ret );
	return ret;
}

void	int2byte(u32 value, byte data[], int index)
{
	#ifdef BIG_ENDIAN
	 data[index+0] = (byte) ((value >> 24) & 0xff) ;
	 data[index+1] = (byte) ((value >> 16) & 0xff); 
	 data[index+2] = (byte) ((value >> 8) & 0xff); 
	 data[index+3] = (byte) (value & 0xff); 	
	#else
	  *(u32 *) ( &data[index]) = value ;
	#endif
	//printf("i2b value = %x, %x, %x, %x, %x\n", value, data[index],data[index+1], data[index+2], data[index+3]);
}

void	word2byte(int value, byte data[], int index)
{
	#ifdef BIG_ENDIAN
		 data[index] = (byte) ((value >> 8) & 0xff); 
		 data[index+1] = (byte) (value & 0xff); 	
	#else
		* (u16 *) (& data[index]) = (u16 ) (value & 0xffff);
	#endif

	//printf("w2b value = %x, %x, %x \n", value, data[index],data[index+1]);
}

u16	get_ip_port(UDP_PACKET * udp)
{
	return set_word(udp->dest_port);
}

u32	get_ip_address(byte ip[])
{
	u32 ret;

	ret = (ip[0]& 0xff) | ((ip[1] << 8) & 0xff00) | ((ip[2]<<16) & 0xff0000) | ((ip[3]<< 24) & 0xff000000);
	//return  byte2int(ip, 0);
	return ret;
}

u32	get_reboot_delay()	// what can it do for delay????
{

	return 10;
}

int	ip_get_msg_id(UDP_PACKET *rx)
{
	return byte2int(rx->data, 4);
}


int	recv_packet(UDP_PACKET *rx)
{
	rx->data = RX_BUF;
	return rx_udp( psu_socket, psu_port, rx);
}

void	send_packet(UDP_PACKET *tx)
{
	tx_udp( psu_socket+1, psu_port, tx);
}


void	ip_get_ump_address(UDP_PACKET *udp, byte *addr)
{
	 * (u32 *) &ump_ip = udp->dest_ip;
}


u16	set_word(u16 w)
{
	u16 ret;

	#ifdef	BIG_ENDIAN
		ret = ( w >> 8 & 0xff ) | ( w << 8 & 0xff00 );
	#else
		return w;
	#endif
	
	return ret;
}

int	set_int(int w)
{
	int ret;

	#ifdef	BIG_ENDIAN
	ret = ( w>>24 & 0xff ) | ( w >> 8 & 0xff00 )| ( w<<8 & 0xff0000 ) | ( w <<24 & 0xff000000 );
		return ret;
	#else
		return w;
	#endif
	
}



int	ip_state = IP_IDLE_STATE;


int	fusing_size, fusing_mod;
int	fusing_count;

int	fusing_total_size;


#if 0	// defined in RABM.c

pFunc Jump_To_Application;
uint32_t	JumpAddress;


void	check_system_restart()
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

	//NVIC_SystemReset();	// software boot function in CM3.h

}

void set_system_shutdown(){

	printf("ShutDown!\n");
	power_cut = 1;

	shutdown_flag = 1;
	shutdown_time = time10ms + 1000;	// After  10sec, shutdown the system
	ip_alarm_noti();
}


void	system_shutdown()
{
	int count = 0;


	printf("Going Shutdown!\n");
	//GPIO_SetBits(GPIO_BAT_CUTOFF, BAT_CUTOFF);
	set_shutdown_log(LOG_SHUTDOWN, 0.0, 0.0);
	write_shutdown_log(&log_sts);
	Delay_ms(100);
	while ( count++ < 100 ) {	// waiting shutdown for 10secs
		bat_cutoff_on(0);	// cutoff relay off
		rect_on(0);		// shutdown
		IWDG_ReloadCounter();
		Delay_ms(100);
	}
	
	NVIC_SystemReset();	// if shutdown fails, system restart
}
#endif

void	flash_erase(int addr, int page_size)
{	
	int	i;

	printf("FLASH ERASE : 0x%08x, %d\n", addr, page_size);
	FLASH_Unlock();
	for ( i = 0; i <= page_size; i++) {
		FLASH_ErasePage(addr);
		addr += 0x800;
	}

	FLASH_Lock();
}


void fusing_init()
{
	FLASH_Unlock();
	FLASH_ErasePage(AP_STATE);

	FLASH_ProgramWord(AP_STATE, NEW_IMAGE );
	FLASH_ProgramWord(IMAGE_STATE, OLD_IMAGE );
	FLASH_Lock();
}


int	fusing_finish(int fsize)
{
	int size, i;
	
	// IP : fsize = 200, RS : fsize  = 100
	fusing_total_size = (fusing_count-1) * fsize + fusing_mod;
	if (fusing_mod == 0) fusing_total_size += fsize;



#ifdef MD5
	char* buf;

	size = fusing_total_size - 200;	// size of MD5
	printf("Fusing verifying : tsize = %d, size = %d\n", fusing_total_size, size);
	buf = (char *) FUSING_ADDRESS;

	md5_init( &ctx);
	dprintf("MD5 starts\n");
	md5_starts(&ctx);

	dprintf("MD5 update\n");
#if 0
	while ( size > 1024 ) {
		md5_update(&ctx, buf, 1024);
		size -= 1024;
		buf += 1024;
	}
#endif	
	md5_update(&ctx, buf, size);
	dprintf("MD5 finish\n");
	md5_finish(&ctx, md5_output);
	dprintf("MD5 Free\n");
    md5_free( &ctx );

	dprintf("MD5 input/output\n");
	//dump_memory(md5_input, 16);
	//dump_memory(md5_output, 16);

    if( memcmp( md5_input, md5_output, 16 ) != 0 ) {
		printf("Fusing Verify Fail!\n");
		return -1;
    }

	//printf("F FINISH : size = %d\n", fusing_total_size);

	FLASH_Unlock();
	FLASH_ErasePage(AP_STATE);

	for(i = 0x8003800; i <  0x8003c00; i+=4) {
		FLASH_ProgramWord(i, 0x00 );
	}
	
	FLASH_ProgramWord(AP_STATE, OLD_IMAGE );
	FLASH_ProgramWord(IMAGE_STATE, NEW_IMAGE );
	FLASH_ProgramWord(IMAGE_SIZE, fusing_total_size - 200);	
	for(i = 0x8003c10; i <  0x8003400; i+= 4) {
		FLASH_ProgramWord(i, 0x00 );
	}
	
	FLASH_Lock();

	size = *(int*)IMAGE_SIZE;

	printf("F Verify : size = %d ap = %x, image = %x\n", *(int*)IMAGE_SIZE, *(int*)AP_STATE, *(int*)IMAGE_STATE);
	

	if ( size != ( fusing_total_size - 200 ) ) {
		FLASH_Unlock();
		FLASH_ErasePage(AP_STATE);
		for(i = 0x8003800; i <  0x8003c00; i+=4) {
			FLASH_ProgramWord(i, 0x00 );
		}
		FLASH_ProgramWord(IMAGE_STATE, OLD_IMAGE );
		FLASH_Lock();
		return -1;
	}


	printf("Fusing Success!\n");

#endif	
	return 0;
}

int rs_fusing_image(int seq, byte *data)
{
	int	addr;

	addr = FUSING_ADDRESS + (seq-1) * 100;
	return flash_write(addr, data, 100);
}


int fusing_image(int seq, byte *data)
{
	int	addr;

	addr = FUSING_ADDRESS + (seq-1) * 200;
	return flash_write(addr, data, 200);
}


int fusing_check_file(byte *data)
{
	const char	bin_id[] = "YOOHA_5G48V";
	unsigned char *buf, val1, val2;
	int i;

	data[sizeof(bin_id)-1] = 0;
	printf("BIN file ID : %s\n", data);
	if ( memcmp(bin_id, (char *)data, strlen(bin_id)) == 0 ) {
		printf("Valid BIN file. Download Start!\n");
		
		#ifdef MD5
		data[strlen(bin_id) + 32 + 2] = 0;
		buf = &data[strlen(bin_id) + 2];		//  add length of  line feed : 0D/0A
		printf ("MD5 : %s \n",  buf);
		for ( i=0; i < 16; i++) {
			val1 = ((ctoi(*buf++) << 4) & 0xf0);
			val2 =  (ctoi(*buf++) & 0xf);
		
			md5_input[i] = val1 + val2;
			printf("%02X:",md5_input[i]);
		}
		printf ("\n");
		#endif
		fusing_init();
		
		return 0;
	}
	else {
		printf("Invalid BIN file. Download Rejected!\n");
		return -210;		// vendor specific Error Code : Invalid Bin File
		//return 0;
	}
}


int ip_bat_test_voltage()
{
	float	a;
	//a = read_bat_V()* 10.0;
	a = read_output_V()* 10.0;
	return	_round (a);

}
int ip_bat_test_current()
{
	float	a;

	a = read_output_A() * 10.0;
	//printf("DC OUT A = %5.3f\n", a/10);
	return _round ( a );

}

int ip_bat_test_residual()
{
	float	bat_v, full;

	//bat_v = read_bat_V();
	bat_v = read_output_V();
	//printf("BAT V = %5.3f\n", bat_v);
	full = bat_v * 16.67 - 350;	// 10% = 21.6V ~~ 90% 26.4V

	if ( full < 1.0 ) full = 0.5;	// minimu 5%
	if ( full > 99.0 ) full = 100.0;	
	
	return _round ((full * 10.0));
}


void	ip_set_hw_desc(char *sn)
{
	//char str[20];
	//sprintf(str, "%d.%d.%d.%d", (version>> 24) & 0xff, (version>> 16) & 0xff, (version>> 8) & 0xff, version & 0xff);

	int size;
	
	strcpy(hw_desc, const_hw_desc);
	strcat(hw_desc, sn);

	size = strlen(hw_desc);
	hw_desc[size] = ';';
	hw_desc[size+1] = 0;
}


void	ip_alarm_noti()
{
	u32 alarm;
	int	t;
	MSG_ALARM_NOTI	noti;

	
	alarm = ip_get_alarm(&alarm_sts);
	t = _round (rect_sts.rec_T * 10.0) ;
	noti.dmb = set_word(ump_mailbox);
	noti.smb = set_word(psu_mailbox);
	
	noti.msg_id = set_int(ID_ALARM_NOTI);
	noti.result = set_word(0);	// normal
	noti.seq_no = set_word(ip_next_seq_no());
	noti.valid  = set_int(1);
	noti.alarm  = set_int(alarm);
	if ( t < -40 ) t = -1000;
	
	noti.temperature = set_int(t);

	//printf(" ALARM_NOTI : Alarm = 0x%x, T =%d \n", alarm, t);

	ip_send_msg((void *) &noti, sizeof(MSG_ALARM_NOTI), ump_port);

}


void	ip_reboot_noti()
{
	MSG_REBOOT_NOTI	noti;

	noti.dmb = set_word(0x6180);
	//noti.smb = set_word(0x6180);
	//noti.dmb = set_word(ump_mailbox);
	noti.smb = set_word(psu_mailbox);
	noti.msg_id = set_int(ID_REBOOT_NOTI);
	noti.dummy = 0;	

	printf(" REBOOT_NOTI\n");
	ip_send_msg((void *) &noti, sizeof(MSG_REBOOT_NOTI), 0x6180);
}


void	ip_send_msg( void *msg, int length,  int dest_port)
{
	UDP_PACKET	tx;

	//tx.dest_ip = get_ip_address(psu_ip); 
	tx.dest_ip = *(u32 *)ump_ip;
	tx.dest_port = dest_port;
	//tx.dest_port = ump_port;
	tx.length = length;
	tx.data = msg;

	//printf("ip_send : dip = %x, dport = %x \n", tx.dest_ip, tx.dest_port);
	send_packet(&tx);
}

#if 0
int ip_keep_alive_req( UDP_PACKET *rx)
{
	MSG_KEEP_ALIVE_REQ	*req;
	MSG_KEEP_ALIVE_ACK	ack;

	req = (MSG_KEEP_ALIVE_REQ *) (rx->data);
	//word2byte( (int) req->smb, (byte *) &ump_mailbox, 0);
	//word2byte( (int) req->dmb, (byte *) &psu_mailbox, 0);
	//ump_mailbox = set_word(req->smb);
	//psu_mailbox = set_word(req->smb);
	printf("KEEP_ALIVE_REQ : UMB = %x, PMB = %x\n", set_word(req->smb), set_word(req->smb));

	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);
	
	ack.msg_id = set_int(ID_KEEP_ALIVE_ACK);
	ack.result = set_word(0);	// normal
	ack.dummy = set_word(0);	// ????? remove set_word????

	ip_send_msg((void *) &ack, sizeof(MSG_KEEP_ALIVE_ACK), rx->dest_ip);
	return 1;
}
#endif

MSG_ALARM_RSP	s_rsp;
int s_rsp_flag = 0;


int ip_alarm_rsp( )
{

	u32 alarm;

	MSG_ALARM_RSP	rsp;

	//dprintf("ALARM_RSP : UMB = %x, PMB = %x\n", ump_mailbox, psu_mailbox);
	alarm = ip_get_alarm(&alarm_sts);

	rsp.dmb = set_word(ump_mailbox);
	rsp.smb = set_word(psu_mailbox);
	//rsp.dmb = set_word(req->smb);	//set_word(ump_mailbox);
	//rsp.smb = set_word(req->dmb);	//set_word(psu_mailbox);
	
	rsp.msg_id = set_int(ID_ALARM_RSP);
	rsp.result = set_word(0);	// normal
	rsp.seq_no = 0;	// ????? remove set_word????
	rsp.valid  = set_int(1);
	rsp.alarm  = set_int(alarm);


	ip_send_msg((void *) &rsp, sizeof(MSG_ALARM_RSP), ump_port);
	return 1;
}

int ip_alarm_req( UDP_PACKET *rx)
{

	u32 alarm;
	MSG_ALARM_REQ	*req;
	MSG_ALARM_RSP	rsp;

	ump_port =  rx->dest_port;
	req = (MSG_ALARM_REQ *) (rx->data);
	word2byte( (int) req->smb, (byte *) &ump_mailbox, 0);
	word2byte( (int) req->dmb, (byte *) &psu_mailbox, 0);
	ump_mailbox = set_word(req->smb);
	psu_mailbox = set_word(req->dmb);
	//dprintf("ALARM_REQ : UMB = %x, PMB = %x\n", ump_mailbox, psu_mailbox);
	alarm = ip_get_alarm(&alarm_sts);

	rsp.dmb = req->smb;	//set_word(ump_mailbox);
	rsp.smb = req->dmb;	//set_word(psu_mailbox);
	//rsp.dmb = set_word(req->smb);	//set_word(ump_mailbox);
	//rsp.smb = set_word(req->dmb);	//set_word(psu_mailbox);
	
	rsp.msg_id = set_int(ID_ALARM_RSP);
	rsp.result = set_word(0);	// normal
	rsp.seq_no = req->seq_no;	// ????? remove set_word????
	rsp.valid  = set_int(1);
	rsp.alarm  = set_int(alarm);

	//dprintf("Port %x, A seq = %x:%x\n", ump_port, rsp.seq_no, req->seq_no );

	ip_send_msg((void *) &rsp, sizeof(MSG_ALARM_RSP), ump_port);
	rsp_timer = time10ms + 500;
	return 1;
}

int ip_hw_info_req( UDP_PACKET *rx)
{
	//dprintf("HW_INFO_REQ : \n");

	MSG_HW_INFO_REQ	*req;
	MSG_HW_INFO_ACK	ack;

	req = (MSG_HW_INFO_REQ *) (rx->data);

	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);

	
	ack.msg_id = set_int(ID_HW_INFO_ACK);
	ack.result = set_word(0);	// normal
	ack.dummy = set_word(0);

	ack.version = set_int(hw_version);
	memcpy(ack.description, hw_desc, 244);

	ip_send_msg((byte *) &ack, sizeof(MSG_HW_INFO_ACK), rx->dest_port);

	return 1;

}

int ip_fw_info_req( UDP_PACKET *rx)
{
	//dprintf("FW_INFO_REQ : \n");

	MSG_FW_INFO_REQ	*req;
	MSG_FW_INFO_ACK	ack;

	req = (MSG_FW_INFO_REQ *) (rx->data);

	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);

	
	ack.msg_id = set_int(ID_FW_INFO_ACK);
	ack.result = set_word(0);	// normal
	ack.dummy = set_word(0);
	ack.version = set_int(fw_version);

	ip_send_msg((byte *) &ack, sizeof(MSG_FW_INFO_ACK), rx->dest_port);

	return 1;

}



int ip_fusing_req( UDP_PACKET *rx)
{
	//printf("BD FUSING_REQ : \n");

	MSG_FUSING_START_REQ	*req;
	MSG_FUSING_START_ACK	ack;
	MSG_FUSING_SEND_REQ 	*send_req;
	MSG_FUSING_END_REQ 		*end_req;
	int result = 0;
	int	flag, data = 0;
	int	restart = 0;

	req = (MSG_FUSING_START_REQ *) (rx->data);
	flag = set_word(req->flag);
	switch ( flag ) {
		case 0:	// start fusing
			fusing_size = set_word(req->size);			
			if ( ip_state == IP_FUSING_STATE  ) {
				result = 1;
				fusing_count = 0;
				ip_state = IP_IDLE_STATE;
				operation_mode = NORMAL_MODE;
				
			}
			else if ( ip_state != IP_IDLE_STATE) result = -101;
			else if ( fusing_size > ( 0x1c000 / 200 ) ) result = -111;
			else  if (operation_mode == NORMAL_MODE) {
				result = 0;
				fusing_count = 0;
				ip_state = IP_FUSING_STATE;
				operation_mode = FUSING_MODE;
				flash_erase(FUSING_ADDRESS, (fusing_size * 200 / 0x800) + 1);
			}
			else {
				result = -101;
			}
			#if 0
			if ( result ) {
				fusing_count = 0;
				ip_state = IP_IDLE_STATE;
				operation_mode = NORMAL_MODE;
			}
			#endif
			data = 0;
			printf("Fusing Start : size = %x result = %d\n", fusing_size, result);
			break;
		case 1:	// transfer req
			send_req = (MSG_FUSING_SEND_REQ *) (rx->data);
			data = set_word(send_req->seq_no);
			
			
			if ( ip_state == IP_FUSING_STATE ) {
				fusing_count++;
				if ( data == fusing_count ) {
					result = 0;
					if ( data == 1 ) {
						result =  fusing_check_file(send_req->data);
					}					
					else if ( fusing_image(fusing_count-1, send_req->data) != 0) result = -201;

					if ( result != 0 ) {
						ip_state = IP_IDLE_STATE;
						operation_mode = NORMAL_MODE;
					}
				}
				else if ( data == (fusing_count-1) ) {
					result = 1;	
					fusing_count--;

				}
				else if (data > fusing_count ) {
					result = -113;
				}
				else if ( data < (fusing_count-1)) {
					result = -112;
				}
			}
			else {
				result = -12;			
			}	
			if ( result != 0 ) printf("Fusing Error: count = %x, res= %d\n", data, result);
			break;
		case	2:	// end of fusing & restart
			printf("Restart, ");
			restart = 1;			
			
		case	4:	// end of fusing
			//printf("End Fusing !!\n");
			end_req = (MSG_FUSING_END_REQ *) (rx->data);
			fusing_mod = set_word( end_req->mod);		

			if ( ip_state == IP_FUSING_STATE ) {
				if ( fusing_mod > 200 ) {
					result = -106;
				}
				else if ( fusing_mod < 0 ) {
					result = -105;
				}
				else  {
					result = fusing_finish(200);
					if ( result == -1)  result = -201;					
				}
			}
			else {
				result =  -12;
			}
			data = flag;
			ip_state = IP_IDLE_STATE;
			operation_mode = NORMAL_MODE;

			printf("Fusing End: Flag = %d, mod = %x result = %d\n",flag, fusing_mod, result);
			break;

		case	8:	// cancel fusing
			printf("Fusing Cancelled\n");
			ip_state = IP_IDLE_STATE;
			operation_mode = NORMAL_MODE;
			data = flag;
			break;
		default :
			result = -102;
			break;		

	}


	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);

	
	ack.msg_id = set_int(ID_FUSING_ACK);
	ack.result = set_word(result);	// normal
	ack.dummy = set_word(data);

	ip_send_msg((byte *) &ack, sizeof(MSG_FUSING_START_ACK), rx->dest_port );

	//printf("Result = %d\n", result);

	if ( result == 0  && restart == 1)  {
		set_system_restart(50);
		ip_reboot_noti();
	}

	if ( result != 0 ) {
		ip_state = IP_IDLE_STATE;
		operation_mode = NORMAL_MODE;		
	}
	return result;
}

int ip_reboot_req( UDP_PACKET *rx)
{
	Dprintf("Reboot_REQ : \n");

	MSG_REBOOT_REQ	*req;
	MSG_REBOOT_ACK	ack;
	short int	result = 0;


	req = (MSG_REBOOT_REQ *) (rx->data);

	reboot_delay = get_reboot_delay();
	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);

	
	ack.msg_id = set_int(ID_REBOOT_ACK);

	if ( operation_mode != NORMAL_MODE ) result = -11;
	else result = 0;
	ack.result = set_word(result);	// normal
	ack.delay = set_word(reboot_delay);	// time unit = 1ms

	ip_send_msg((byte *) &ack, sizeof(MSG_SHUTDOWN_ACK), rx->dest_port );

	if ( result == 0 ) {
		set_system_restart((reboot_delay + 9 )/ 10);	// time unit = 10ms
		ip_reboot_noti();
	}
	
	return 1;
}


int ip_psu_status_req( UDP_PACKET *rx)
{
	Dprintf("PSU_STATUS_REQ : \n");

	MSG_PSU_STATUS_REQ	*req;
	MSG_PSU_STATUS_ACK	ack;

	req = (MSG_PSU_STATUS_REQ *) (rx->data);

	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);
	
	ack.msg_id = set_int(ID_PSU_STATUS_ACK);
	ack.result = set_int(0);	// normal
	ack.dummy = set_word(0);
	ack.length = set_int(72);
	ack.seq_no = req->seq_no;

	//printf("p seq = %x:%x\n", ack.seq_no, req->seq_no);
	ip_get_psu_status(ack.value, ack.data);     

	ip_send_msg((byte *) &ack, sizeof(MSG_PSU_STATUS_ACK), rx->dest_port );

	return 1;
}

int ip_shutdown_req( UDP_PACKET *rx)
{
	Dprintf("SHUTDOWN_REQ : \n");

	MSG_SHUTDOWN_REQ	*req;
	MSG_SHUTDOWN_ACK	ack;

	req = (MSG_SHUTDOWN_REQ *) (rx->data);

	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);

	
	ack.msg_id = set_int(ID_SHUTDOWN_ACK);
	ack.result = set_word(0);	// normal
	ack.dummy = set_word(0);

	ip_send_msg((byte *) &ack, sizeof(MSG_SHUTDOWN_ACK), rx->dest_port);

	set_system_shutdown();

	return 1;

}

int ip_memo_get_req( UDP_PACKET *rx)
{
	//dprintf("MEMO_GET_REQ : \n");

	MSG_MEMO_GET_REQ	*req;
	MSG_MEMO_GET_ACK	ack;
	int ret;

	req = (MSG_MEMO_GET_REQ *) (rx->data);

	ret = ip_get_memo_info(ack.data);

	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);
	
	ack.msg_id = set_int(ID_MEMO_GET_ACK);
	ack.result = set_word(ret);	// normal
	ack.dummy = set_word(0);	

	ip_send_msg((byte *) &ack, sizeof(MSG_MEMO_GET_ACK), rx->dest_port);
	return ret;
}


int ip_memo_set_req( UDP_PACKET *rx)
{
	//dprintf("MEMO_SET_REQ : \n");

	MSG_MEMO_SET_REQ	*req;
	MSG_MEMO_SET_ACK	ack;
	int	ret;

	req = (MSG_MEMO_SET_REQ *) (rx->data);
	ret = ip_set_memo_info(req->data);

	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);
	
	ack.msg_id = set_int(ID_MEMO_SET_ACK);
	ack.result = set_word(ret);	// normal
	ack.dummy = set_word(0);	

	ip_send_msg((byte *) &ack, sizeof(MSG_MEMO_SET_ACK), rx->dest_port );
	return ret;
}


int ip_cn_get_req( UDP_PACKET *rx)
{
	//dprintf("CN_GET_REQ : \n");

	MSG_CN_GET_REQ	*req;
	MSG_CN_GET_ACK	ack;
	int ret, result;
	int start, end;
	
	req = (MSG_CN_GET_REQ *) rx->data;
	start = set_word(req->start);
	end = set_word(req->end);

	ret = ip_get_cn_info(start, end, ack.set1);
	if ( ret < 1) result = ret;
	else result = 0;

	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);

	
	ack.msg_id = set_int(ID_CN_GET_ACK);
	ack.result = set_word(result);	// normal
	ack.set_no = set_word(ret);	

	int size = sizeof(MSG_CN_GET_ACK) - (CN_INFO_SIZE * 5);
	if ( ret > 0 ) size += (CN_INFO_SIZE * ret);
		
	ip_send_msg((byte *) &ack, size, rx->dest_port );
	return result;
}

int ip_cn_set_req( UDP_PACKET *rx)
{
	//dprintf("CN_SET_REQ : \n");

	MSG_CN_SET_REQ	*req;
	MSG_CN_SET_ACK	ack;
	int ret, result;
	
	req = (MSG_CN_SET_REQ *) rx->data;

	ret = ip_set_cn_info(req->set);
	if ( ret < 0 ) result = ret;
	else result = 0;

	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);

	
	ack.msg_id = set_int(ID_CN_SET_ACK);
	ack.result = set_word(result);	// normal
	ack.dummy = set_word(0);	

	ip_send_msg((byte *) &ack, sizeof(MSG_CN_SET_ACK), rx->dest_port );
	return result;
}


int ip_repair_get_req( UDP_PACKET *rx)
{
	

	MSG_REPAIR_GET_REQ	*req;
	MSG_REPAIR_GET_ACK	ack;

	req = (MSG_REPAIR_GET_REQ *) rx->data;

	int ret, result;
	int	start, end;
	
	start = set_word(req->start);
	end = set_word(req->end);


	ret = ip_get_repair_info(start, end, ack.set1);
	///dprintf("REPAIR_GET_REQ : Set No. = %d \n", ret);
	if ( ret < 1) result = ret;
	else result = 0;

	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);
	
	ack.msg_id = set_int(ID_REPAIR_GET_ACK);
	ack.result = set_word(result);	// normal
	ack.set_no = set_word(ret);	

	int size = sizeof(MSG_REPAIR_GET_ACK) - (REPAIR_INFO_SIZE * 5);
	if ( ret > 0 ) size += (REPAIR_INFO_SIZE * ret);

	ip_send_msg((byte *) &ack, size, rx->dest_port );

	return result;

}

int ip_repair_set_req( UDP_PACKET *rx)
{
	

	MSG_REPAIR_SET_REQ	*req;
	MSG_REPAIR_SET_ACK	ack;

	req = (MSG_REPAIR_SET_REQ *) rx->data;

	int ret, result;

	ret = ip_set_repair_info(req->set);
	//dprintf("REPAIR_SET_REQ : Set No. = %d \n", ret);
	if ( ret < 0 ) result = ret;
	else result = 0;

	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);
	
	ack.msg_id = set_int(ID_REPAIR_SET_ACK);
	ack.result = set_word(result);	// normal
	ack.dummy = set_word(0);


	ip_send_msg((byte *) &ack, sizeof(MSG_REPAIR_SET_ACK), rx->dest_port );
	//dump_memory(req->set, 128);

	return result;

}


int ip_battery_test_req( UDP_PACKET *rx)
{
	

	MSG_BATTERY_TEST_REQ	*req;
	MSG_BATTERY_TEST_ACK	ack;
	int	result, voltage, time;


	req = (MSG_BATTERY_TEST_REQ *) rx->data;
	//word2byte( req->smb, (byte *) &ump_mailbox, 0);
	//word2byte( req->dmb, (byte *) &psu_mailbox, 0);
	ump_mailbox = set_word(req->smb);
	psu_mailbox = set_word(req->dmb);
	bat_seq_no = req->seq_no;
	bat_port = rx->dest_port;
	bat_id = set_word(req->psu_id);
	voltage = set_int(req->voltage);
	time = set_int(req->time);

	Dprintf("BATTERY_TEST_REQ : psu_id = %x, v= %d, T=%d \n", bat_id, voltage, time);
	result = ip_battery_test_start(voltage, time);
	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);

	
	ack.msg_id = set_int(ID_BATTERY_TEST_ACK);
	ack.seq_no = bat_seq_no;
	ack.psu_id = set_word(bat_id);
	ack.result = set_word(result);	// normal = 0
	ack.dummy = set_word(0);


	ip_send_msg((byte *) &ack, sizeof(MSG_BATTERY_TEST_ACK), bat_port );

	return result;

}

int ip_battery_test_data(int test_time)
{
	//printf("BATTERY_TEST_DATA : \n");

	int v, a, r;
	MSG_BATTERY_TEST_DATA	ack;

	ip_bat_test = 0;
	//result = ip_battery_test_result(&ack);
	set_bat_test_data(test_time);
	v = ip_bat_test_voltage();
	a = ip_bat_test_current();
	if ( a < 0 ) a = 0;
	if (a > 3000 ) a = 3000;
	r = ip_bat_test_residual();
	ack.dmb = set_word(ump_mailbox);
	ack.smb = set_word(psu_mailbox);
	
	ack.msg_id = set_int(ID_BATTERY_TEST_DATA);
	ack.seq_no= bat_seq_no;	// normal
	ack.psu_id = set_word(bat_id);
	ack.input_voltage = set_int(v);
	ack.input_current = set_int(a);
	ack.residual_rate = set_int(r);
	ack.test_time 	= set_int(test_time);	//(test_time);
	ack.dummy = set_int(0);

	if (bat_port ==  0 ) bat_port = ump_port;
	ip_send_msg((byte *) &ack, sizeof(MSG_BATTERY_TEST_DATA), bat_port);
	Dprintf("BATTERY_TEST_DATA : %d, %d, %d, %d\n", ack.dmb, ump_mailbox, r, test_time);
	bat_port = 0;
	return 0;
}


int ip_op_info_req( UDP_PACKET *rx)
{
	

	MSG_OP_INFO_REQ	*req;
	MSG_OP_INFO_ACK	ack;

	req = (MSG_OP_INFO_REQ *) rx->data;

	int result;

	//dprintf("Operation_info_REQ\n");
	result = 0;

	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);
	
	ack.msg_id = set_int(ID_OP_INFO_ACK);
	ack.result = set_word(result);	// normal
	ack.op_mode = set_word(operation_mode);	

	ack.running_ip = *(u32 *)psu_ip;
	memcpy(ack.running_mac, psu_mac, 6);
	ack.dummy1 = 0;

	ack.saved_ip = *(u32 *)saved_ip;
	memcpy(ack.saved_mac, saved_mac, 6);
	ack.dummy2 = 0;

	
	ack.dipsw_ip = *(u32 *)dipsw_ip;
	memcpy(ack.dipsw_mac, dipsw_mac, 6);
	ack.dummy3 = 0;

	ack.dummy4 = 0;
	ack.running_mng_id = set_int(management_id);				// ksyoo 20150318 0.91
	ack.saved_mng_id = set_int(*(u32*) saved_management_id);	// ksyoo 20150318 0.91

	int size = sizeof(MSG_OP_INFO_ACK);

	ip_send_msg((byte *) &ack, size, rx->dest_port );

	return result;

}


int	ip_set_ip_addr(u32	addr)
{
	ip_changed_flag = 1;
	saved_ip[0] = (addr >> 24) & 0xff;
    saved_ip[1] = (addr >> 16) & 0xff;
    saved_ip[2] = (addr >> 8) & 0xff;
    saved_ip[3] = addr  & 0xff;
	set_mac_address(saved_mac, saved_ip);

	return 0;
}

int ip_ip_set_req( UDP_PACKET *rx)
{
	//dprintf("IP_SET_REQ : \n");

	MSG_IP_SET_REQ	*req;
	MSG_IP_SET_ACK	ack;
	int result;
	u32	ip;
	
	req = (MSG_IP_SET_REQ *) rx->data;

	ip = req->ip;
	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);

	
	ack.msg_id = set_int(ID_IP_SET_ACK);
	ack.result = set_word(0);	// normal
	ack.dummy = set_word(0);	

	ip_send_msg((byte *) &ack, sizeof(MSG_IP_SET_ACK), rx->dest_port );

	result = ip_set_ip_addr(ip);
	return result;
}


// 20140107 v0.70

int	ip_read_psu_name(unsigned char *name)
{
	int ret;

	//memset(name, 0, 100);
	ret = read_eeprom(PSU_NAME_ADDR, name, 1);

	if ( *name == 0xff || *name == 0 ) {	// initialize psu_name
		strcpy((char *) user_psu_name, const_hw_desc);
	        ret = write_eeprom(PSU_NAME_ADDR, user_psu_name, 100);	
                //printf("\nDefault PSU_NAME = %s", user_psu_name);		
	}
      
	ret = read_eeprom(PSU_NAME_ADDR, name, 100);
	name[99] = 0;
	//dump_memory(user_psu_name, 256);

	return ret;
}

int	ip_write_psu_name(char *name)
{
	int ret;
	
	name[99] = 0;
	//memset(user_psu_name, 0, 100);		// clear user psu name buffer 
	strcpy(( char *)user_psu_name, name);
	ret = write_eeprom(PSU_NAME_ADDR, user_psu_name, 100);	
	//printf("\nWRITE PSU_NAME = %x %x %x %x", user_psu_name[0],user_psu_name[1],user_psu_name[2],user_psu_name[3]);

	return ret;
}


int	ip_user_name_read_req( UDP_PACKET *rx)
{

	//dprintf("IP_USER_NAME_READ_REQ : \n");

	MSG_USER_NAME_READ_REQ	*req;
	MSG_USER_NAME_READ_ACK	ack;


	req = (MSG_USER_NAME_READ_REQ *) rx->data;

	ack.dmb = req->smb; //set_word(ump_mailbox);
	ack.smb = req->dmb; //set_word(psu_mailbox);


	ack.msg_id = set_int(ID_USER_NAME_READ_ACK);
	ack.result = set_word(0);	// normal
	ack.dummy = set_word(0);	
	memset(ack.psu_name, 0, 100);
	strcpy(ack.psu_name, (char const *) user_psu_name);

	ip_send_msg((byte *) &ack, sizeof(MSG_USER_NAME_READ_ACK), rx->dest_port );
	//dump_memory(ack.psu_name, 256);
	
	return 0;

}

int	ip_user_name_write_req( UDP_PACKET *rx)
{

	//dprintf("IP_USER_NAME_WRITE_REQ : \n");

	MSG_USER_NAME_WRITE_REQ	*req;
	MSG_USER_NAME_WRITE_ACK	ack;
	int result;

	req = (MSG_USER_NAME_WRITE_REQ *) rx->data;
	result = ip_write_psu_name(req->psu_name);
	ack.dmb = req->smb; //set_word(ump_mailbox);
	ack.smb = req->dmb; //set_word(psu_mailbox);


	ack.msg_id = set_int(ID_USER_NAME_WRITE_ACK);
	ack.result = set_word(0);	// normal
	ack.dummy = set_word(0);	
	ip_send_msg((byte *) &ack, sizeof(MSG_USER_NAME_WRITE_ACK), rx->dest_port );

	return result;

}


int	ip_bat_autotest_set_req( UDP_PACKET *rx)
{

	//dprintf("bat test flag : \n");

	MSG_BAT_AUTOTEST_SET_REQ	*req;
	MSG_BAT_AUTOTEST_SET_ACK	ack;
	int result = 0;

	req = (MSG_BAT_AUTOTEST_SET_REQ *) rx->data;
	if ( req->flag == 0 || req->flag == 1 ) {
		set_auto_test_mode(req->flag);
	}
	else result = -3; 
	
	ack.dmb = req->smb; //set_word(ump_mailbox);
	ack.smb = req->dmb; //set_word(psu_mailbox);
	ack.msg_id = set_int(ID_BAT_AUTOTEST_SET_ACK);
	ack.result = set_word(result);	// normal
	ack.dummy = set_word(0);	
	ip_send_msg((byte *) &ack, sizeof(MSG_BAT_AUTOTEST_SET_ACK), rx->dest_port );

	return result;

}



int ip_standby_reboot_req( UDP_PACKET *rx)
{
	
	MSG_STANDBY_REBOOT_REQ	*req;
	MSG_STANDBY_REBOOT_ACK	ack;
	int result;
	short	command, time;
	
	req = (MSG_STANDBY_REBOOT_REQ *) rx->data;
	command = set_word(req->command);
	time = set_word(req->standby_time);

	

	result = 0;
	switch (command) {
		case	0:
			if ( operation_mode != NORMAL_MODE ) result = -11;
			else {
				ip_reboot_noti();
				operation_mode = STANDBY_MODE;
				set_system_restart(time * 100);
			}
			break;
		case 1:
			if (operation_mode != STANDBY_MODE ) {
				result = -12;
				time = 0;
			}
			else {
				time = (restart_time - time10ms ) / 100;
			}
			break;
		case 2:
			if (operation_mode != STANDBY_MODE ) {
				result = -12;
				time = 0;
			}
			else {
				time = (restart_time - time10ms ) / 100;
				operation_mode = NORMAL_MODE;
				restart_time = 0L;
				restart_flag = 0;	// cancel standby reboot
			}
			break;
		default :
			result = -3;
			time = 0;
			break;

	}
	

	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);

	
	ack.msg_id = set_int(ID_STANDBY_REBOOT_ACK);
	ack.result = set_word(result);	// normal
	ack.remain_time = set_word(time);
	//ack.dummy = set_word(0);	

	ip_send_msg((byte *) &ack, sizeof(MSG_STANDBY_REBOOT_ACK), rx->dest_port );

	//printf("STANDBY REBOOT_REQ time= %d\n", time);


	return result;
}



int ip_management_id_set_req( UDP_PACKET *rx)
{
	Dprintf("MANAGEMENT_ID_SET_REQ : \n");

	MSG_MANAGEMENT_ID_SET_REQ	*req;
	MSG_MANAGEMENT_ID_SET_ACK	ack;
	int result = 0;
	u32	id;
	u8 buf[8];
	
	req = (MSG_MANAGEMENT_ID_SET_REQ *) rx->data;

	id = req->management_id;
	memcpy(buf, (u8 *) &req->management_id, 4);
	ack.dmb = req->smb;	//set_word(ump_mailbox);
	ack.smb = req->dmb;	//set_word(psu_mailbox);

	
	ack.msg_id = set_int(ID_MANAGEMENT_ID_SET_ACK);
	ack.result = set_word(0);	// normal
	ack.dummy = set_word(0);	

	ip_send_msg((byte *) &ack, sizeof(MSG_MANAGEMENT_ID_SET_ACK), rx->dest_port );

	Dprintf("Buf : %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
	set_management_id(id);
	return result;
}

int	process_msg(UDP_PACKET *udp)
{
	int	ret = 0;
	int	msg_id;
	
	ip_get_ump_address(udp, ump_ip);
	//tx_port = udp->dest_port;

	//ump_port = tx_port;
	//ump_port = udp->dest_port;
	msg_id = ip_get_msg_id(udp);
	//dprintf("Proceess 0x%08x msg = %x port = %x \n", udp->dest_ip, msg_id, udp->dest_port);
	switch ( msg_id) {
		case	ID_MANAGEMENT_ID_SET_REQ:
			ret = ip_management_id_set_req(udp);
			break;
		case	ID_ALARM_REQ:
			ret = ip_alarm_req(udp);
			break;
		case	ID_HW_INFO_REQ:
			ret = ip_hw_info_req(udp);
			break;
		case	ID_FW_INFO_REQ:
			ret = ip_fw_info_req(udp);
			break;
		case	ID_FUSING_REQ :
			ret = ip_fusing_req(udp);
			break;
		case	ID_REBOOT_REQ :
			ret = ip_reboot_req(udp);
			break;
		case	ID_PSU_STATUS_REQ:
			ret = ip_psu_status_req(udp);
			break;
		case	ID_SHUTDOWN_REQ:
			ret = ip_shutdown_req(udp);
			break;
		case	ID_MEMO_GET_REQ:
			ret = ip_memo_get_req(udp);
			break;
		case	ID_CN_GET_REQ:
			ret = ip_cn_get_req(udp);
			break;
		case	ID_REPAIR_GET_REQ:
			ret = ip_repair_get_req(udp);
			break;
		case	ID_MEMO_SET_REQ:
			ret = ip_memo_set_req(udp);
			break;
		case	ID_CN_SET_REQ:
			ret = ip_cn_set_req(udp);
			break;
		case	ID_REPAIR_SET_REQ:
			ret = ip_repair_set_req(udp);
			break;
		case	ID_BATTERY_TEST_REQ :
			ret = ip_battery_test_req(udp);
			break;
		case	ID_OP_INFO_REQ :
			ret = ip_op_info_req(udp);
			break;
		case	ID_IP_SET_REQ :
			ret = ip_ip_set_req(udp);
			break;
		case	ID_STANDBY_REBOOT_REQ :
			ret = ip_standby_reboot_req(udp);
			break;
		case	ID_USER_NAME_READ_REQ:
			ret = ip_user_name_read_req(udp);
			break;
		case	ID_USER_NAME_WRITE_REQ:
			ret = ip_user_name_write_req(udp);
			break;
		case	ID_BAT_AUTOTEST_SET_REQ :
			ret = ip_bat_autotest_set_req(udp);
			break;
		default:
			break;
	}

	return ret;
}


void	alarm_comm()
{
	int	length;

	if ( operation_mode == NORMAL_MODE ) {
		if ( Enable_DHCP == ON ) {
			if ( dhcp_state == STATE_DHCP_INIT ) {
				start_dhcp();
				return;
			}		
			
			check_DHCP_state(dhcp_socket );
			IWDG_ReloadCounter();
			// 190114
			//if ( dhcp_state == STATE_DHCP_INIT || dhcp_state == STATE_DHCP_DISCOVER || dhcp_state == STATE_DHCP_REQUEST ) return;
			if ( dhcp_state == STATE_DHCP_INIT || dhcp_state == STATE_DHCP_DISCOVER ) return;
		}
		else if ( Enable_DHCP == DUE_ADDR ) {
			due_process();
			//if ( due_state != DUE_SUCCESS ) return;
		}
	}
	
	ping_rx(ping_socket, ping_rx_addr);
	IWDG_ReloadCounter();
	ping_tx();
	IWDG_ReloadCounter();

	if ( Enable_DHCP == DUE_ADDR && (due_state!= DUE_OK && due_state != DUE_SUCCESS) ) return;
	
	length = recv_packet(&udp_rx);
	if ( length > 0 ) {
		process_msg(&udp_rx);

	}
	IWDG_ReloadCounter();
	if (rsp_flag == 0 )  {
		rsp_timer = time10ms + 500;
		rsp_flag = 1;
	}
	if (rsp_flag == 1  && rsp_timer< time10ms ) {
		//ip_alarm_rsp();	// periodic alarm RSP as DongAh PSU
		rsp_timer = time10ms + 500;
	}

	if ( alarm_notification_flag == 1 ) {
		ip_alarm_noti();	// send alarm notification message
		alarm_notification_flag = 0;
	}
	else if ( ip_tx_flag ) {
		send_packet( &udp_tx);
		ip_tx_flag = 0;
	}
	IWDG_ReloadCounter();
#if 1
	TELNETS(telnet_socket, telnet_port);
	TELNETS(telnet_socket+1, telnet_port);
	IWDG_ReloadCounter();
#endif	

}

void	set_gw_address(byte gw[], byte ip[], byte mask[])
{
	gw[0] = ip[0] & mask[0];
	gw[1] = ip[1] & mask[1];
	gw[2] = ip[2] & mask[2];
	gw[3] = 1;
}

void	set_due_gw(byte gw[], byte ip[], byte mask[])
{
	gw[0] = ip[0] & mask[0];
	gw[1] = ip[1] & mask[1];
	gw[2] = 16;
	gw[3] = 1;
}


void init_due_addr(uint8_t ipx1, uint8_t ipx2)
{
#if 1
	du_ip[0] = psu_ip[0] = ipx1;
	du_ip[1] = psu_ip[1] = ipx2;	
	du_ip[2] = 16;
	//psu_ip[3] = 37;
	du_ip[3] = 33;
#endif

	printf("%s:%x %x \n", __FUNCTION__, ipx1, ipx2);

	close(psu_socket);
	close(psu_socket+1);
#if 0
	psu_ip[0] = 192;
	psu_ip[1] = 168;
	psu_ip[2] = 128;
	psu_ip[3] = 2;	
#endif

	set_mac_address(psu_mac, psu_ip);
	subnet_mask[0] = 255;
	subnet_mask[1] = 255;
	subnet_mask[2] = 0;
	subnet_mask[3] = 0;	

	set_due_gw(gw_ip, psu_ip, subnet_mask);
	set_wiz_network();
	start_udp_port();

}

void	ip_addr_init()
{
	u_char addr[4];
	
	packet_header = packet_tail = 0;

	//save_management_id(0xabcdef12);
	management_id = read_management_id();
	//printf("MID:%d\n", management_id);
	if ( management_id == 0xfffffffe ) {
		Enable_DHCP = DUE_ADDR;
		#if 1
		if(!socket(dhcp_socket , Sn_MR_UDP, psu_port, 0x00))
		{
			printf("DUE FAIL\n");
		}

		#endif
		ip_read_due_ip();
		init_due_addr(psu_ip[0], psu_ip[1]);
		due_state = DUE_TEST;
		//due_state = DUE_TX_REQUEST;		
	}
	else if ( management_id != 0 ) {
		//190116 read_board_specific_id();	// to read dipsw ip addr
		//190116 read_flash_ip(addr);	// dummy addr
		dhcp_state = STATE_DHCP_INIT;
		
		init_dhcp_client(dhcp_socket, 0, 0);
		Enable_DHCP = DHCP_ON;
	}
	else {
		Enable_DHCP = DHCP_OFF;
		read_ip_address(psu_ip);
		set_mac_address(psu_mac, psu_ip);
		set_gw_address(gw_ip, psu_ip, subnet_mask);
		set_wiz_network();
		start_udp_port();
	}

	ip_state = IP_IDLE_STATE;
	operation_mode = NORMAL_MODE;
	
}

u16	read_dipsw()
{
	return 0x00;
}

u16 read_board_specific_id()
{

	dipsw_ip[0] = 192;
	dipsw_ip[1] = 168;
	dipsw_ip[2] = 128;
	dipsw_ip[3] = 2;

	set_mac_address(dipsw_mac, dipsw_ip);
	//printf("Dipsw IP: 0x%02x:0x%02x:0x%02x:0x%02x\n", dipsw_ip[0], dipsw_ip[1], dipsw_ip[2], dipsw_ip[3]);
	return 0;
}


int	read_flash_ip(byte addr[])
{
	byte flash_ip[4];

	eeprom_read_buffer(IP_EP_ADDR, flash_ip, 4);

	saved_ip[0]  = flash_ip[3];
	saved_ip[1]  = flash_ip[2];
	saved_ip[2]  = flash_ip[1];
	saved_ip[3]  = flash_ip[0];

	set_mac_address(saved_mac, saved_ip);
	//printf("saved IP: 0x%02x:0x%02x:0x%02x:0x%02x\n", saved_ip[0], saved_ip[1], saved_ip[2], saved_ip[3]);

	if ( saved_ip[0] != 192 ) { // sip == 0 || sip == 0xffffffff) {
		saved_ip[0]  =  0;
		saved_ip[1]  = 0;
		saved_ip[2]  = 0;
		saved_ip[3]  = 0;
		set_mac_address(saved_mac,saved_ip);
		return 0;
	}	
	
	addr[0] = flash_ip[3];
	addr[1] = flash_ip[2];
	addr[2] = flash_ip[1];
	addr[3] = flash_ip[0];

	return 1;
}



void read_ip_address(byte addr[])
{
	
	addr[0] = 192;
	addr[1] = 168;
	addr[2] = 128;
	addr[3] = 2;


	read_flash_ip(addr);
	
}

void	set_mac_address(byte mac[], byte ip[])
{
	mac[0] = 0x02 ;
	mac[1] = ((ip[0] >> 6) & 0x03);		//0x03; //0x00 | ((ip[0] >> 6) & 0x03);
	mac[2] = ((ip[0] << 2) & 0xfc) | ((ip[1] >> 6) & 0x03);		//0x02 ; //((ip[0] << 2) & 0xfc);
	mac[3] = ((ip[1] << 2) & 0xfc) | ((ip[2] >> 6) & 0x03);		//0xa2 ;//((ip[1] << 2) & 0xfc);
	mac[4] = ((ip[2] << 2) & 0xfc) | ((ip[3] >> 6) & 0x03);		//0x56 ;// ((ip[2] << 2) & 0xfc);
	mac[5] = ((ip[3] << 2) & 0xfc) ;	//0x08 ; //((ip[3] << 2) & 0xfc);
}

//#define	WIZ_VERSIONR	0x0039
#define	WIZ_VERSION		0x04
//#define	WIZ_PTIMERR		0x0028
#define	WIZ_PTIME		0x28


void	wiz_get_ip_address(byte ip[])
{
	getSIPR (ip);
}

void	wiz_get_mac_address(byte mac[])
{
	mac[0] = IINCHIP_READ(SHAR0+0);
	mac[1] = IINCHIP_READ(SHAR0+1);
	mac[2] = IINCHIP_READ(SHAR0+2);
	mac[3] = IINCHIP_READ(SHAR0+3);
	mac[4] = IINCHIP_READ(SHAR0+4);
	mac[5] = IINCHIP_READ(SHAR0+5);
}
void	wiz_get_gw_address(byte gw[])
{
	getGAR(gw);
}

void	wiz_get_subnet_address(byte sn[])
{
	getSUBR(sn);
}

void	wiz_get_address()
{
	byte tmp_array[6];

	Delay_ms(50);
	getSIPR (tmp_array);
	printf("\nIP : %d.%d.%d.%d", tmp_array[0],tmp_array[1],tmp_array[2],tmp_array[3]);

#if 1
	Delay_ms(50);
	printf("\r\nMAC : %.2X.%.2X.%.2X.%.2X.%.2X.%.2X", IINCHIP_READ(SHAR0+0),IINCHIP_READ(SHAR0+1),IINCHIP_READ(SHAR0+2),IINCHIP_READ(SHAR0+3),IINCHIP_READ(SHAR0+4),IINCHIP_READ(SHAR0+5));
	Delay_ms(50);
	getSUBR(tmp_array);
	printf("\r\nSN : %d.%d.%d.%d", tmp_array[0],tmp_array[1],tmp_array[2],tmp_array[3]);
	Delay_ms(50);
	getGAR(tmp_array);
	printf("\r\nGW : %d.%d.%d.%d", tmp_array[0],tmp_array[1],tmp_array[2],tmp_array[3]);
	Delay_ms(5);
#endif

}

void	wiz_test()
{
	byte	version;
	
	version = IINCHIP_READ(VERSIONR);	
	//ptime = IINCHIP_READ(0x28);
	//printf("WIZ Version = 0x%02x, Ptimer = 0x%02x\n", version, ptime);
	if ( version != WIZ_VERSION  ) {
		printf("ETH CHIP Error!\n");
	}
	else printf("ETH CHIP OK!\n");

	//wiz_get_address();
}

void check_wiznet(){
	byte	version;
	
	version = IINCHIP_READ(GAR0);	// whether it is reset or not
	//printf("WIZ VERSION : %d \n", version);
	if ( version == 0 ) {
		//if ( eth_flag > 10 ) return;
		printf("ETH Error!\n");
		Reset_W5200();
		wiz_init();		
		eth_flag = 1;
	}
	else eth_flag = 0;
}

void dhcp_reset()
{
	Dprintf("DHCP reset..\n");
	Reset_W5200();
}


u_char	num2bin(char num)
{
	if ( num > ('0'+9) || num < '0') num = 0;
	else num -= '0';

	return num;
}

void	set_system_MAC()
{
	//u_char	mac[6];

	
	//psu_mac[0] = 0x00;	// wiznet MAC code : 0x00-08-DC, TELESYS 00:17:B2
	//psu_mac[1] = 0x17;
	//psu_mac[2] = 0xb2;

	sys_read_mac();	// R6000
	//printf("MAC : %02x-%02x-%02x:", psu_mac[0], psu_mac[1], psu_mac[2]);
	//printf("%02x-%02x-%02x\n", psu_mac[3], psu_mac[4], psu_mac[5]);
}


uint32_t	get_management_id()
{
	return management_id;
}

void	set_management_id(u32 id)	// ** id shoudl be BIG Endian
{
	management_id_flag = 1;		// late saving of management id

	test_value = id;
	saved_management_id[0] = (id >> 24) & 0xff;
	saved_management_id[1] = (id >> 16) & 0xff;
	saved_management_id[2] = (id >> 8) & 0xff;
	saved_management_id[3] = id & 0xff;

	printf("S MID = %02X %02X %02X %02X %u\n", saved_management_id[0], saved_management_id[1], saved_management_id[2], saved_management_id[3], *(u32*) saved_management_id);
	printf("R MID = %x %u\n", management_id, management_id);
	
}


#if 0
void	 init_dhcp()
{
	u_char	tmpip[4];
	
	/* get IP */
	
	init_dhcp_client(0,  dhcp_reset, dhcp_reset);

	while ( 1 )
	{
		Dprintf("\r\nSerching DHCP Server...\r\n");
		if ( getIP_DHCPS() )
			break;
	}

	//Dprintf("\r\n*** THANK YOU ***\r\n"); // R6000
	tmpip[0] = 211;
	tmpip[1] = 46;
	tmpip[2] = 117;
	tmpip[3] = 79;
	
	sendto(3, (const u_char*)"for PING TEST", 13, tmpip, 5000); 
	while ( 1 ) 
	{
		check_DHCP_state( 0 );
	}

}
#endif


void send_due_req()
{
	uint8_t ip[4];

	due_req_p.version = 1;
	due_req_p.msg_id = 0;
	due_req_p.msg_id = set_word(MSG_DUE_REQ);

	due_req_p.i4[0] = 0;
	due_req_p.i4[1] = 0;
	due_req_p.i4[2] = 0;

	due_req_p.d4[0] = 0;
	due_req_p.d4[1] = 0;
	due_req_p.d4[2] = 0;

	due_req_p.o4[0] = 0;
	due_req_p.o4[1] = 0;
	due_req_p.o4[2] = 0;

	due_req_p.rsv[0] = 0;
	due_req_p.rsv[1] = 0;
	due_req_p.rsv[2] = 0;

	ip[0] = 255;
	ip[1] = 255;
	ip[2] = 255;
	ip[3] = 255;
	
	if(0 == sendto(dhcp_socket, (uint8_t *)&due_req_p, sizeof(DUE_REQ_T), ip, DUE_PORT))
	{
		printf("DUE : REQ FAIL!\n");
	}
}

uint16_t get_due_rsp() 
{
	uint16_t len, port;
	uint8_t	addr[6], ip1, ip2;
	
	if ((len = getSn_RX_RSR(dhcp_socket)) > 0)	{
		len = recvfrom(dhcp_socket, (u_char *)&due_rsp_p, len, addr, &port);
		if ( port == DUE_PORT  && due_rsp_p.msg_id == set_word(MSG_DUE_RSP)) {
			if  ( psu_ip[0] != due_rsp_p.o4[0] || psu_ip[1] != due_rsp_p.o4[1]) {
				init_due_addr(due_rsp_p.o4[0], due_rsp_p.o4[1]);
			}
			return 1;
		}
	}
	return 0;
}

void due_process()
{
	static int due_count = 0;
	
	switch (due_state) {
		case DUE_TEST:
			due_time = time10ms + DUE_NEXT_TIME;
			due_count++;
			if ( due_count > 5 ) {	// 5MIN : 30
				due_state = DUE_TX_REQUEST;
				due_count = 0;
				printf("DUE TEST TIMEOUT\n");
				break;
			}
			due_ping();
			due_state = DUE_WAIT_TEST;
			break;
		case DUE_WAIT_TEST:
		   if ( due_ping_result() ) {
		   		due_state = DUE_OK;
				printf("DUE CONNECT\n");
				break;
		   	}
		   	if ( due_time < time10ms ) {
				due_state = DUE_TEST;	
				printf("DUE PING TIMEOUT\n");
			}
			break;		
		case DUE_TX_REQUEST:
			send_due_req();
			due_time = time10ms + DUE_WAIT_TIME;
			due_state = DUE_WAIT_RSP;
			printf("DUE REQUEST\n");
			break;
		case DUE_RETRY:
			if ( due_time < time10ms ) {
				due_state = DUE_TX_REQUEST;
			}
			break;		

		case DUE_WAIT_RSP:
			if ( get_due_rsp() == 1 ) {
				due_state = DUE_SUCCESS;
				close(dhcp_socket);
				ip_save_due_ip();
				printf("DUE SUCCESS\n");
				break;
			}
			if ( due_time < time10ms ) {
				due_time = time10ms + DUE_NEXT_TIME;
				due_state = DUE_RETRY;
			}
			break;	

		case DUE_SUCCESS:
			break;
		case DUE_OK:
			break;
			
		default:
			printf("Invalide DUE state\n");
			due_state = DUE_TEST;
			break;

	}

}



