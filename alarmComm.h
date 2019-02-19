#ifndef __ALARMCOMM_H
#define __ALARMCOMM_H

#include	"util.h"
//#define byte unsigned char

#define	REQ

#if 0
#define	ID_KEEP_ALIVE_REQ 	0x00401011
#define	ID_KEEP_ALIVE_ACK 	0x00401811
#endif

#define MID_PREFIX	0x41	// COMPANY_CODE(0x40) + RECT_TYPE (0x1 = 1.2KW)

#define MAC_PREFIX1	0x00
#define MAC_PREFIX2 0xFE
#define MAC_PREFIX3	MID_PREFIX

#define	ID_ALARM_NOTI	0x00481252
#define	ID_ALARM_REQ	0x00421251
#define	ID_ALARM_RSP	0x00421215
#define	ID_HW_INFO_REQ	0x00401000
#define	ID_HW_INFO_ACK	0x00401800
#define	ID_FW_INFO_REQ	0x00401001
#define	ID_FW_INFO_ACK	0x00401801
#define	ID_FUSING_REQ 	0x00401010
#define	ID_FUSING_ACK 	0x00401810
#define	ID_REBOOT_REQ	0x00401002
#define	ID_REBOOT_ACK	0x00401802
#define	ID_REBOOT_NOTI	0x00401004


#define	ID_PSU_STATUS_REQ 	0x0045003b
#define	ID_PSU_STATUS_ACK 	0x0045003c
#define	ID_SHUTDOWN_REQ 	0x00401005
#define	ID_SHUTDOWN_ACK 	0x00401805

#define	ID_MEMO_GET_REQ 	0x00401007
#define	ID_MEMO_GET_ACK 	0x00401807
#define	ID_MEMO_SET_REQ 	0x00401008
#define	ID_MEMO_SET_ACK 	0x00401808

#define	ID_CN_GET_REQ 	0x00401009
#define	ID_CN_GET_ACK 	0x00401809
#define	ID_CN_SET_REQ 	0x0040100A
#define	ID_CN_SET_ACK 	0x0040180A

#define	ID_REPAIR_GET_REQ 	0x0040100B
#define	ID_REPAIR_GET_ACK 	0x0040180B
#define	ID_REPAIR_SET_REQ 	0x0040100C
#define	ID_REPAIR_SET_ACK 	0x0040180C



#define	ID_BATTERY_TEST_REQ 	0x0040100F
#define	ID_BATTERY_TEST_ACK 	0x1040100F
#define	ID_BATTERY_TEST_DATA 	0x1040100E

#define ID_OP_INFO_REQ		0x00401011
#define ID_OP_INFO_ACK		0x00401811

#define ID_IP_SET_REQ		0x00401012
#define ID_IP_SET_ACK		0x00401812


#define ID_STANDBY_REBOOT_REQ		0x00401003
#define ID_STANDBY_REBOOT_ACK		0x00401803


// 20140107	v0.70

#define ID_USER_NAME_READ_REQ		0x00401013
#define ID_USER_NAME_READ_ACK		0x00401813
#define ID_USER_NAME_WRITE_REQ		0x00401014
#define ID_USER_NAME_WRITE_ACK		0x00401814
#define ID_BAT_AUTOTEST_SET_REQ		0x00401015
#define ID_BAT_AUTOTEST_SET_ACK		0x00401815

// 2015 3.18	v0.91
#define ID_MANAGEMENT_ID_SET_REQ		0x00401016
#define ID_MANAGEMENT_ID_SET_ACK		0x00401816


// DATA 12

#define	IP_ALARM_AC_VOLTAGE			0x01000000
#define	IP_ALARM_DC_HIGH_VOLTAGE	0x02000000
#define	IP_ALARM_DC_LOW_VOLTAGE		0x04000000
#define	IP_ALARM_DC_OVER_CURRENT	0x20000000
#define	IP_ALARM_BAT_DISCONNECTION	0x40000000
#define	IP_ALARM_POWER_OUT			0x80000000

// Data 13
#define	IP_ALARM_MODULE_FAIL_0		0x010000
#define	IP_ALARM_MODULE_FAIL_1		0x020000
#define	IP_ALARM_RECT_SMALL			0x400000
#define	IP_ALARM_RECT_MIDDLE		0x800080
#define	IP_ALARM_RECT_COMPACT		0x18000000	// rect 5_type = 1, rect 6_type =1, rect type =  0 and others = 0



//Data 14
#define	IP_ALARM_SURGE_FAIL			0x0100
#define	IP_ALARM_BAT_DISCHARGE		0x0200
#define	IP_ALARM_BAT_TEMPERATURE	0x0000
#define	IP_ALARM_REC_TEMPERATURE	0x0800
#define	IP_ALARM_BAT_ABNORMAL		0x2000
#define	IP_ALARM_BAT_QUALITY_LOW	0x4000
#define	IP_ALARM_BAT_HEATER			0x8000

// DATA 15
#define	IP_ALARM_RECT_FAN			0x01
#define	IP_ALARM_RECT_FIRE			0x02
#define	IP_ALARM_RECT_FLOOD			0x04
#define	IP_ALARM_RECT_DOOR			0x08


#define	ALARM_DEFAULT	(IP_ALARM_RECT_SMALL)

#define	BOOT_ADDRESS		0x08000000
#define	FUSING_ADDRESS		0x08020000	// image save address
#define	APPLICATION_ADDRESS	0x08004000	// application start address
#define	AP_STATE			(APPLICATION_ADDRESS - 0x0400)
#define	IMAGE_STATE			(AP_STATE + 4)
#define	IMAGE_SIZE			(AP_STATE + 8)

#if 0		// 2017 2. move to eprom
#define	HW_VERSION_ADDRESS	0x0803fc00	// int hw_version
#define	SN_ADDRESS			0x0803fc40	// char serial_no[20]
#define IP_ADDRESS			0x0803fc60	// ip address
#endif

#define	OLD_IMAGE	0x89abcdef
#define	NEW_IMAGE	0x01234567


#define IP_IDLE_STATE	0
#define	IP_FUSING_STATE	1
#define	IP_BOOT_STATE	2
#define	IP_SHUTDOWN_STATE	3
#define	IP_BATTERY_TEST_STATE	4

#define	NORMAL_MODE	0
#define	FUSING_MODE	1
#define	STANDBY_MODE	2


#define DUE_ADDR	0x03
#define DHCP_OFF 0
#define DHCP_ON	 1

typedef void (*pFunc)(void);

typedef struct {
	unsigned short dst_mailbox;	// 16bits, job id of target
	unsigned short src_mailbox;	// 16bits mailbox that wait for ack
} IPC_HEADER;


typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	var1;
	u16	var2;
	u32	var[20];
} MSG_DATA;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	seq_no;
	u16	dummy;
}	MSG_KEEP_ALIVE_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	dummy;
}	MSG_KEEP_ALIVE_ACK;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	seq_no;
	u32	valid;
	u32	alarm;
	u32	temperature;
}	MSG_ALARM_NOTI;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	seq_no;
	u16	dummy;
}	MSG_ALARM_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	seq_no;
	u32	valid;
	u32	alarm;
}	MSG_ALARM_RSP;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	seq_no;
	u16	dummy;
}	MSG_HW_INFO_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	dummy;
	u32	version;
	byte	description[244];
}	MSG_HW_INFO_ACK;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	dummy;
}	MSG_FW_INFO_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	dummy;
	u32	version;
}	MSG_FW_INFO_ACK;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	flag;
	u16	size;	// real image size = 200 * size  ex size = 0 @ 1-199, size = 1 @200-399
}	MSG_FUSING_START_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	dummy;
}	MSG_FUSING_START_ACK;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	flag;
	u16	seq_no;	// real image size = 200 * size  ex size = 0 @ 1-199, size = 1 @200-399
	byte	data[200];
}	MSG_FUSING_SEND_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	seq_no;
}	MSG_FUSING_SEND_ACK;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	flag;
	u16	mod;	// real image size = 200 * size  ex size = 0 @ 1-199, size = 1 @200-399
}	MSG_FUSING_END_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	action;
}	MSG_FUSING_END_ACK;


typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	dummy;
}	MSG_REBOOT_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	delay;
}	MSG_REBOOT_ACK;



typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	seq_no;
	u16	dummy;
}	MSG_PSU_STATUS_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	result;
	u16	dummy;
	u16	seq_no;
	u32	length;
	u32 value[12];
	u8	data[24];	//v.60  16->24	20140107
}	MSG_PSU_STATUS_ACK;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	dummy;
}	MSG_SHUTDOWN_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	dummy;
}	MSG_SHUTDOWN_ACK;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	dummy;
}	MSG_MEMO_GET_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	dummy;
	byte	data[128];
}	MSG_MEMO_GET_ACK;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	dummy;
	byte	data[128];
}	MSG_MEMO_SET_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	dummy;
}	MSG_MEMO_SET_ACK;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	start;
	u16	end;
}	MSG_CN_GET_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	set_no;
	byte	set1[124];
	byte	set2[124];
	byte	set3[124];
	byte	set4[124];
	byte	set5[124];
}	MSG_CN_GET_ACK;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	dummy;
	byte	set[124];
}	MSG_CN_SET_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	dummy;
}	MSG_CN_SET_ACK;


typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	start;
	u16	end;
}	MSG_REPAIR_GET_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	set_no;
	byte	set1[188];
	byte	set2[188];
	byte	set3[188];
	byte	set4[188];
	byte	set5[188];
}	MSG_REPAIR_GET_ACK;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	dummy;
	byte	set[188];
}	MSG_REPAIR_SET_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	dummy;
}	MSG_REPAIR_SET_ACK;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	seq_no;
	u16	psu_id;
	u32	voltage;	// 210~270 ( 21.0~27.0V)
	u32	time;		// 0~120(minutes)
	u32	dummy;
}	MSG_BATTERY_TEST_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	seq_no;
	u16 psu_id;
	u16	result;
	u16	dummy;
}	MSG_BATTERY_TEST_ACK;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	seq_no;
	u16 psu_id;
	u32	input_voltage;	// 0~600 (0.0~60.0V)
	u32	input_current;	// 0~3000 (0.0~300.0[A])
	u32	residual_rate;	// 0~1000 (0.0 ~ 100.0%)
	u32	test_time;		// 0~7200 sec
	u32	dummy;
}	MSG_BATTERY_TEST_DATA;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	dummy;
}	MSG_REBOOT_NOTI;


typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	dummy;
}	MSG_OP_INFO_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	op_mode;
	u32		running_ip;
	byte	running_mac[6];
	u16		dummy1;	
	
	u32		saved_ip;
	byte	saved_mac[6];
	u16		dummy2;
	
	u32		dipsw_ip;
	byte	dipsw_mac[6];
	u16		dummy3;
	
	u32	dummy4;			
	u32 running_mng_id;		// 0.91
	u32 saved_mng_id;
	
}	MSG_OP_INFO_ACK;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	dummy;
	u32	ip;
}	MSG_IP_SET_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	dummy;
}	MSG_IP_SET_ACK;


typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	dummy;
	u16	command;
	u16	standby_time;	// sec
	u16	dummy1;
}	MSG_STANDBY_REBOOT_REQ;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	remain_time;	// sec
}	MSG_STANDBY_REBOOT_ACK;


// 20140107	v0.70

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	dummy;
}	MSG_USER_NAME_READ_REQ;		// v0.63 

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	dummy;
	byte	psu_name[100];			// string ended by NULL
}	MSG_USER_NAME_READ_ACK;		//v0.70

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	dummy;
	byte	psu_name[100];			// v0.70 string ended by NULL
}	MSG_USER_NAME_WRITE_REQ;		// v0.63 

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	dummy;
}	MSG_USER_NAME_WRITE_ACK;		//v0.63


typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	dummy;
	byte	flag;
	byte	dummy1;
	u16		dummy2;
}	MSG_BAT_AUTOTEST_SET_REQ;		// v0.70

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	dummy;
}	MSG_BAT_AUTOTEST_SET_ACK;		//v0.70


typedef	struct {
	char num[4];
	char	cn_num[4];
	char	date[12];
	char	company[12];
	char	post[12];
	char	engineer[12];
	char	issue[24];
	char	contents[44];
}	CN_INFO;

typedef	struct {
	char 	num[4];
	char	req_date[12];
	char	req_company[12];
	char	req_post[12];
	char	req_engineer[12];
	char	req_complain[24];
	char	rep_date[12];
	char	rep_engineer[12];
	char	rep_result[44];
	char	rep_contents[44];
}	REPAIR_INFO;

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u32	dummy;
	u32	management_id;
}	MSG_MANAGEMENT_ID_SET_REQ;		// v0.91

typedef struct {
	u16	dmb;
	u16	smb;
	u32	msg_id;
	u16	result;
	u16	dummy;
}	MSG_MANAGEMENT_ID_SET_ACK;



#define	MAX_PACKET_BUFFER	10
#define	PACKET_BUFFER_SIZE	1500

#if 0
typedef struct {
	u16 dst_mailbox;
	u16	src_mailbox;
	byte	*pbuf;
} PAY_LOAD;

typedef struct {
	u16 dst_mailbox;
	u16	src_mailbox;
	u32	msg_id;
	u16	var1;
	u16	var2;
	u8	*data;
} MESSAGE;



typedef struct {
	u16 dst_mailbox;
	u16	src_mailbox;
	byte	data[PACKET_BUFFER_SIZE];
} PAY_LOAD_BUFFER;

#endif

typedef struct {
	u32	dest_ip;
	u16	dest_port;
	u32	length;
	byte	*data;
} UDP_PACKET;

typedef	struct	{
	byte	rMR;
	byte	rCR;
	byte	rIR;
	byte	rSR;
	u16		rPORT_;
	byte	rDHAR[6];
	byte	rDIPR[4];
	u16		rDPORT;
	u16		rMSS;
	byte	rPROTO;
	byte	rTOS;
	byte	rTTL;
	u16		rRXMEM_SIZE;
	u16		rTXMEM_SIZE;
	u16		rTX_FSR;
	u16		rTX_RD;
	u16		rTX_WR;
	u16		rRX_RSR;
	u16		rRX_RD;
	byte	rIMR;
	u16		rFRAG;	
} SOCKET_REGS;


#define UDP_HEADER_LENGTH	16

void 	read_ip_address(byte address[]);
void	set_mac_address(byte mac[], byte ip[]);
void	ip_addr_init();
void	set_gw_address(byte gw[], byte ip[], byte mask[]);
u32 byte2int(byte data[], int index);
int ip_set_cn_info(byte *set);
u16	set_word(u16 w);
void system_restart();
int	fusing_finish();
int fusing_image(int seq, byte *data);
int ip_bat_test_residual();
//int atoi(char *);
u16	ip_read_switch();
int ip_battery_test_data(int test_time);
void	ip_set_hw_desc(char *sn);
int ip_set_memo_info(byte *data);

int	ip_set_ip_addr(u32	addr);


int	set_int(int w);
u16	set_word(u16 w);
int ip_read_psu_name(unsigned char *);
u16	read_dipsw();
int ip_battery_test_start(int voltage, int time);

void	 init_dhcp();
void	set_wiz_network();
u16 read_board_specific_id();
void	set_management_id(u32 id);
int flash_write(int addr, byte * data, int length);
void	alarm_comm();
void	check_system_restart();

void	set_auto_test_mode(int mode);
int	read_flash_ip(byte addr[]);
void	flash_erase(int addr, int page_size);
int fusing_check_file(byte *data);
int rs_fusing_image(int seq, byte *data);

void due_process();
void init_due_addr(uint8_t ipx1, uint8_t ipx2);
void rewrite_psu_name();
void	ip_save_due_ip();
void	ip_read_due_ip();



//extern uint32_t management_id;
#endif

