#ifndef __RAMB_H
#define __RAMB_H

#include	"config.h"
#include	"I2CRoutines.h"
#include	"uart.h"
#include	"alarmcomm.h"

#define TYPE_5G	0
#define TYPE_5G_NEW	1
#define TYPE_5G_C	2

#define	VERSION_SIZE	12

#define BAT_STS_CHG	0x02
#define BAT_STS_DCHG	0x01
#define BAT_STS_CHANGE		0x08
//#define BAT_STS_CONN		0x08
#define BAT_STS_TEST_FAIL	0x04
#define BAT_STS_SW		0x20
#define BTN_ALARM_CLEAR		0x0100



#define JUMPER_HW	0x40
#define JUMPER_ETH	0x80

#define ALARM_MODE_CONTACT	0
#define ALARM_MODE_ETH		1
#define ALARM_MODE_HW		2

#define	LOG_MAX_NO	201

// control BD SENSOR = 0, RECTifier sensor(external) = 13
#define CONT_T_CH	0		// 
#define RECT_T_CH	13

#define	POWER_ON_RESET	1
#define RST_RESET		2
#define	WATCHDOG_RESET	3
#define	SW_RESET		4


#define	BAT_TEST_TIME	6000			// org = 60sec
#define	BAT_CHARGE_12H		 (12*3600) //(12 * 3600 )
#define	BAT_CHARGE_7H		 (7*3600)  //( 7 * 3600 )



#define	V_REF	3.417	// DAC/ADC v_ref 
#define	OUTPUT_THRESHOLD		40.0
#define MIN_OUT_V	40.0	//40.0 @1206

#define OCP_ZERO_V	5.0


#define INPUT_HI_VOLTAGE_MAX	270.0
#define INPUT_HI_VOLTAGE_MIN	230.0
#define INPUT_LOW_VOLTAGE_MAX	210.0
#define INPUT_LOW_VOLTAGE_MIN		160.0
#define OUTPUT_HI_VOLTAGE_MAX		58.0
#define OUTPUT_HI_VOLTAGE_MIN		49.0
#define OUTPUT_LOW_VOLTAGE_MAX	53.0
#define OUTPUT_LOW_VOLTAGE_MIN	40.0
#define	OUTPUT_OVER_CURRENT		31.9	// OCP = 35.0 * 1.05

#define AC_SHUTDOWN_V		167.0 // RECOVER 175
//#define AC_LOW_SHUTDOWN_V		167.0
#define AC_HIGH_SHUTDOWN_V		273.0	// recover 265


#define	OUTPUT_MODULE_FAIL	55.2	// module fail directly

#define BAT_LOW_VOLTAGE_MAX	52.0
#define BAT_LOW_VOLTAGE_MIN	42.0

//#define BAT_CELL_FAIL_MAX	24.0
//#define BAT_CELL_FAIL_MIN	22.1
#define BAT_CELL_FAIL_V		43.0
#define	BAT_CUT_OFF_V		43.0

//#define BAT_TEMP_MAX	95.0
//#define RECT_TEMP_MAX	95.0

#define	OUTPUT_OVP_DEF	54.75	// RECOVER =  53.8

#define INPUT_HI_VOLTAGE_DEF	270.0
#define INPUT_LOW_VOLTAGE_DEF	170.0
#define OUTPUT_HI_VOLTAGE_DEF	54.3	// RECOVER 53.9V
#define OUTPUT_LOW_VOLTAGE_DEF	46.0	// RECOVER at 48.0
#define BAT_HI_VOLTAGE_DEF		54.5
#define BAT_LOW_VOLTAGE_DEF		44.0
//#define BAT_FAIL_HI_DEF			29.0
#define BAT_DISCHARGE_DEF		44.0	// RECOVER 45.0
#define BAT_FAIL_LOW_DEF		44.0	// bat. cut off voltage
#define BAT_TEMP_DEF			95.0
#define RECT_TEMP_DEF			95.0

#define RECT_TEMP_FAIL			100.0	// recover 90


#define	DCV_COEF	19.2  //11.4
#define	LDA_COEF	1.0 	//10.0
#define	BATA_COEF	1.0

#define	OUTPUT_VOLTAGE_DEF	53.3

#if 0
#define	BAT_TEST_SW_VOLTAGE	22.0	//21.0
#define	BAT_TEST_FAIL_V		23.0
#define	BAT_TEST_BQL_V		23.0
#define	BAT_TEST_LVD_V		21.0
#define	BAT_LVD_V			24.0
#define	BAT_CHARGE_A		0.35
#define	BAT_CAPACITY		(7.0 * 360)
#endif

#define	MIN_CURRENT_LIMIT	0.5
#define	MAX_CURRENT_LIMIT	3.0

#define	CHARGE_A	3.0

#define ALARM_OK	0
#define	ALARM_HIGH		0x01
#define	ALARM_LOW		0x02

#define	ALARM_BAT_HIGH		0x80
#define	ALARM_REC_HIGH		0x40
#define	ALARM_AC_NFB		0x20
#define	ALARM_INPUT_HIGH	0x10
#define	ALARM_INPUT_LOW		0x08
#define	ALARM_OUTPUT_HIGH	0x04
#define	ALARM_OUTPUT_LOW	0x02
#define	ALARM_BAT_LOW		0x01

#define	ALARM_BAT_DISCONNECT	0x0100
#define	ALARM_BAT_CELL_FAIL		0x0200
#define	ALARM_OTP			0x0400
#define	ALARM_BAT_NFB			0x0800
#define	ALARM_OVP			0x1000
//#define	ALARM_RU1			ALARM_OUT_NFB
#define	ALARM_BQL			0x2000
#define ALARM_OCP			0x2000
//#define	ALARM_OUTPUT_A		0x4000
#define ALARM_POWER_CUT		0x4000


#define	ALARM_MODULE_FAIL	0x8000


#define	EVENT_OCCUR		0x18	// exclusive value of alsrm status 00011000
#define	EVENT_CLEAR		0x06	// exclusive value of alarm status 00000110

#define	ECS_RMS_CURRENT	11
#define	ECS_RMS_VOLTAGE	12
#define ECS_INST_POWER	9
#define	ECS_REAL_POWER	10
#define	ECS_STATUS		15
#define	ECS_TEMP		19
#define	ECS_EPSILON 	13

#define	ECS_CURRENT_OFFSET	16
#define	ECS_VOLTAGE_OFFSET	17
#define	ECS_POWER_OFFSET	14

#define	ECS_EPSILON_VALUE	0x01999A

#define STATE_COUNT	5

typedef enum {
	REPORT_STATUS = 55,
	REPORT_ALARM,
	REPORT_SYSTEM_INFO,
	REPORT_OPERATION,
	REPORT_LOG,
	REPORT_AD,
	REPORT_BAT_TEST = 61,
	REPORT_PSU_NAME = 62,

} DEV_REPORT;

typedef enum {
	GET_REC_STATUS = 1,
	GET_ALARM_STATUS,
	GET_SYS_INFO,
	GET_AD_SETUP,
	GET_OPERATION_SETUP,
	GET_LOG,
	GET_LOG_NO,
	GET_SYS_TIME,
	GET_PSU_NAME,

	SET_ALRAM_INPUT_HI_V = 10,
	SET_ALARM_INPUT_LOW_V,
	SET_ALARM_OUTPUT_HI_V,
	SET_ALRAM_OUTPUT_LOW_V,
	SET_ALARM_BAT_FAIL_V,
	SET_ALARM_BAT_LOW_V,	
	SET_ALARM_BAT_HI_T,
	SET_ALARM_REC_HI_T,
	SET_ALARM_DEFAULT,

	SET_AD_INPUT_V = 20,
	SET_AD_INPUT_A,
	SET_AD_OUTPUT_V,
	SET_AD_OUTPUT_A,
	SET_AD_BAT_A,
	SET_AD_OUPUT_ZERO_V,
	SET_AD_BAT_ZERO_V,
	SET_AD_RESET,

	SET_SYSTEM_TIME = 30,
	SET_SYSTEM_DATE,

	SET_T_COMP_ON,
	SET_T_COMP_OFF,
	SET_T_COMP_OUTPUT_V,
	SET_FLASH_IP,
	
	SET_BAT_CHARGE_ON = 40,
	SET_BAT_CHARGE_OFF,
	SET_BAT_CHARGE_A,
	
	SET_RELAY_NORMAL_OPEN = 50,
	SET_RELEAY_NORMAL_CLOSE,
	
	SET_BAT_TEST_ON = 60,
	SET_BAT_TEST_OFF,
	SET_BAT_TEST_PERIOD,
	SET_BAT_TEST_V,
	SET_BAT_TEST_DAY,
	SET_BAT_TEST_TIME,
	SET_BAT_TEST_DURATION,
	SET_BAT_TEST_REQ,

	SET_LOG_CLEAR = 70,
	SET_PSU_NAME = 78,
	SET_BQL = 79,

	SET_MANAGEMENT_ID = 80,
    SET_BAT_EQUIP = 81,

	FW_DOWNLOAD = 90,
	FW_ACK,
	SET_ALARM_MODE = 95,
	
    SYSTEM_RESET = 0x89,
    SET_HW_VERSION= 0x8A,
    SET_SN = 0x8B,
    SET_TYPE,


	CMD_ADC_TEST= 0xB0,	// test or debug commands
	CMD_DAC_TEST,
    CMD_AC_TEST,
    CMD_LED_TEST,
    CMD_BUTTON_TEST,
    CMD_RELAY_TEST,	// 0xB5
    CMD_EPROM_TEST,
    CMD_EPROM_ERASE,    
    CMD_DEBUG_MODE,
    CMD_INFO_SET,
    CMD_TEST_MODE,	// 0xBA
   
    CMD_GET_IP = 0xBC,		// 0xBC
    CMD_CHIP_ID,
    CMD_BOOT0_TEST,
    CMD_SELF_TEST,
    CMD_PING = 0xc0,		// 0xC0
    CMD_SET_MAC = 0xc1,			//
    CMD_OUTPUT_TEST = 0xc2,		// 0xC2
    CMD_OUT_SEL,
} PC_COMMAND ;


#define CMD_SYSTEM_SHUTDOWN SYSTEM_RESET
#define CMD_SET_HW_VERSION 	SET_HW_VERSION
#define CMD_SET_SN 			SET_SN



#define DL_START 	1
#define	DL_CANCEL 	2
#define	DL_END		3
#define	DL_END_START 4
#define	DL_DATA		5

typedef  struct {
	byte command;
	byte mode;
	uint16_t para;
} F_DOWNLOAD;

typedef  struct {
	byte command;
	byte mode;
	uint16_t para;
} F_DL_ACK;

typedef  struct {
	byte command;
	byte mode;
	uint16_t count;
	byte data[100];
} F_DL_DATA;


typedef struct {
	byte	command;		
	//byte dummy1;
	byte	dummy2;
	u16	sts;
	int input_V;
	int	input_A;
	int   output_V;
	int	output_A;
	int	bat_A;
	int	bat_T;
	int	rec_T;
	int bat_V;
	byte bat_sts;

} F_REC_STATUS;

typedef struct {
	byte command;	
	byte dummy3;
	u16 	sts;
	
	int 	ihv;
	int	ilv;
	int 	ohv;
	int	olv;	
	int	bfv;
	int	blv;
	int	bht;
	int	rht;
	int	fail_bat;

} F_ALARM_SETUP;


typedef	struct {
	byte command;	
	byte dummy3;
	u16 	sts;
	
	int 	iv;
	int	ia;
	int	ov;
	int	oa;
	int	ba;

} F_AD_SETUP;

typedef  struct {
	byte command;
	byte mode;
	byte	year;
	byte	month;
	byte	day;
	byte	hour;
	byte	min;
	byte	sec;
	byte	hw_version[VERSION_SIZE];
	byte	fw_version[VERSION_SIZE];
	byte	serial_no[VERSION_SIZE];

	byte	ip_addr[4];
	byte	ip_save[4];
	u32		run_id;
	u32		saved_id;
	//u16		dummy;
	//byte	run_mac[6];
	//byte	save_mac[6];
	byte 	type;

} F_SYSTEM_INFO;

typedef	struct {
	byte command;
	byte	sts;		// output mode, current mode, relay contact
	byte	year;	// next battery test date & time
	byte	month;
	byte	date;
	byte	hour;
	byte	min;
	byte	sec;

	int		output_V;
	int		charge_A;
	int		bat_test_period;
	int		bat_test_V;
	int		bat_test_mode;
	int		rate;
	int		test_time;
	byte    alarm_mode;
	byte	bat_sts;


} F_OP_SETUP;

typedef	struct {
	byte	command;
	byte	index;
	
	byte	year;	// 0x00 == 2000
	byte	month;
	byte	date;
	byte	hour;
	byte	min;
	byte	sec;
	
	byte	code;
	byte	status;

	union {
			int pvalue;
			struct	 {
				byte	year;	// 0x00 == 2000
				byte	month;
				byte	date;
			} day;
			int32_t acv;
	};
	
	union {
		int cvalue;
		struct	{
			byte	hour;
			byte	min;
			byte	sec;
		} time;
		int32_t aca;
	};
	short dcv;
	short dca;
	short batv;
	short bata;
	short rect;
} F_LOG_TYPE;

typedef  struct {
	byte command;
	byte mode;
	byte	name[100];

} F_PSU_NAME;

typedef  struct {
	byte command;
	u16  mode;
	byte	dummy;
	int	bat_V;
	int	bat_A;
	int	rate;
	int	time;
	int	result;

} F_BAT_TEST_DATA;

typedef  struct {

	float	bat_V;
	float	bat_A;
	float	rate;
	int	time;
	int	result;
	int	send_flag;

} BAT_TEST_DATA;

typedef	struct {
	
	byte	year;	// next battery test date & time
	byte	month;
	byte	date;
	byte	hour;
	byte	min;
	byte	sec;

	int	output_mode;
	int	current_mode;
	int	relay_contact;
	int	bat_test_mode;

	float		output_V;
	float		charge_A;
	int		bat_test_period;
	float		bat_test_V;
	int	   bat_testing;
	byte    bat_equip;		// 20180716
	byte 	bat_sts;		//
	byte	alarm_mode;

} OP_STATUS;

typedef	struct {
	float 	input_V;
	float		input_A;
	float	 	output_V;
	float		output_A;
	float		bat_A;
	float		bat_V;
	float		bat_T;
	float		rec_T;
	float		fail_bat;
	float	 	output_set_V;

	float		ov_offset;
	float		oa_offset;
	float		ba_offset;
	float		iv_offset;
	float		ia_offset;
	float		bt_offset;
	float		rt_offset;
	float		ov_set_offset;

	int	door_open;
	int	ac_NFB;
	int	bat_NFB;
	int	out_NFB;
	int	bat_disconnect;
	int	cell_fail;
	int	bat_test;
	int	ru1;
	int	ru2;
	int	ru3;

} RECT_STATUS;

typedef	struct {
	float 	iv;
	float		ia;
	float		ov;
	float		oa;
	float		ba;
	float		bt;
	float		rt;
	float		ov_set;
	int		bat_zero_offset;
} AD_SETUP;


typedef	struct {
	int 	input_V;
	int	input_A;
	int	output_V;
	int	output_A;
	int	bat_A;
	int	bat_V;
	int	bat_T;
	int	rec_T;
	int	cell_fail;
	int	ac_NFB;
	int	bat_NFB;
	int	ovp;
	int	out_NFB;
	int	bat_disconnect;
	int module_fail;
	int ocp;
	int otp;
//	int	discharge;

	float 	hi_input_V;
	float 	hi_input_A;
	float		hi_output_V;
	float 	hi_output_A;
	float		hi_fail_bat;
	float		hi_bat_V;
	float 	hi_bat_T;
	float 	hi_rec_T;

	float 	low_input_V;
	float 	low_input_A;
	float		low_output_V;
	float 	low_output_A;
	float		low_fail_bat;
	float		low_bat_V;
	float 	low_bat_T;
	float 	low_rec_T;

	float		dcv_offset;
	float		lda_offset;
	float		bata_offset;
	float		cv_offset;
	float		aca_offst;
	int		bat_bql;
	int		power_cut;
} ALARM_STATUS;

typedef  struct {
	int	year;
	int	month;
	int	day;
	int	hour;
	int	min;
	int	sec;
	byte *hw_version;
	byte	*fw_version;
	byte	*serial_no;

	byte	*ip_addr;
	byte	s_mac[6];
	byte	type;

} SYSTEM_INFO;

typedef struct {
	char	no[4];
	char	date[10];
	char	time[9];
	char	alarm_name[20];
	char	status[10];
} LOG_MSG;

typedef	struct {
	//byte	date[4];	// 0x00 == 2000
	byte	month;
	byte	date;
	byte	hour;
	byte	min;
	byte	sec;
	byte	code;
	union {
		int	pvalue;
		struct   {
			byte	year;	// 0x00 == 2000
			byte	month;
			byte	date;
		} day;
	};
	union {
		int cvalue;
		struct  {
			byte	hour;
			byte	min;
			byte	sec;
		} time;
	};
	byte	status;
	byte	checksum;
}NEW_LOG_TYPE;

typedef	struct {
	byte	year;	// 0x00 == 2000
	byte	month;
	byte	date;
	byte	hour;
	byte	min;
	byte	sec;
	byte	code;
	byte	status;	
	union {
		int	pvalue;
		struct   {
			byte	year;	// 0x00 == 2000
			byte	month;
			byte	date;
		} day;
		int32_t acv;
	};
	union {
		int cvalue;
		struct  {
			byte	hour;
			byte	min;
			byte	sec;
		} time;
		int32_t aca;
	};
	short dcv;
	short dca;
	short batv;
	short bata;
	short rect;
	//word	checksum;
} LOG_TYPE;

typedef	struct {
	word	id;
	word	head;
	word	tail;
	word	size;
	word	checksum;
} LOG_HEADER;


typedef	struct {
	word 	log_flag;
	word	alarm_log;
	word	alarm_set_log;
	word	ad_set_log;
	word	time_set_log;
	word	operation_log;
	word	reset_log;
	word	bat_test_log;
	word	alarm_status;
	word	shutdown_log;
	float		pvalue;
	float		cvalue;
	int		pyear;
	int		pmonth;
	int		pdate;
	int		ptime;
	int		pmin;
	int		psec;
	int		cyear;
	int		cmonth;
	int		cdate;
	int		ctime;
	int		cmin;
	int		csec;
	uint8_t 	sts;
	
} LOG_STATUS;

typedef	struct {
	float		Vref;
	float		Boffset;
	float		Coffset;
	int		sign;


} OFFSET_TYPE;

#define	OFFSET_SIGN	0x12345678
#define	OFFSET_ERASE	0xffffffff
#define	OFFSET_MAX_INDEX	( PAGE_SIZE / sizeof(OFFSET_TYPE))

typedef	struct {
	int	page;
	int	index;
} EE_OFFSET_TYPE;

typedef struct {
	int month;
	int	day;
	int	hour;
	int	min;
	float	v;
	int	duration;
} BAT_AUTO_TEST_INFO;


typedef struct {
	uint32_t	dcv;
	uint32_t	dca;
	uint32_t	batv;
	uint32_t 	bata;
	uint32_t	rect;
	uint32_t 	batt;
	uint32_t 	refv;
	uint32_t    dacv;
} ADC_VALUE;


typedef struct {
	//uint16	id;
	GPIO_TypeDef* group;
	uint16_t pin;
	uint8_t bit;
	uint16_t count;		// 10ms count
	uint16_t wait;		// 10ms count
	uint16_t flag;		// 
} PORT_STS;




typedef struct {
	//uint16	id;
	GPIO_TypeDef* group;
	uint16_t pin;
} IO_STS;

typedef struct {
	uint16_t time;	// 10ms count
	uint16_t wait;	// 10ms count
	uint16_t flag;	// 

} ALARM_TIMER;

typedef struct {
	uint16_t v;
	uint16_t a;
	uint16_t	sts;
	uint16_t	prt;
	uint16_t 	dis;
	uint16_t 	soc;
	uint16_t 	hw_ver;
	uint16_t	sw_ver;
	uint16_t 	soh;
	uint16_t	temp;
	uint16_t 	fet_temp;
	uint32_t	alarm1;
	uint32_t	alarm2;
	uint16_t	c_limit;
	uint16_t	lvd;
	uint16_t 	ext;
	uint8_t		type;
} BMS_STS_T;


#define	BAT_FAIL_TIME		(5 * 60)		// 5minutes SKT-SAMSUNG  default


#define	OFFSET_IA_ADDR	EEPROM_START_ADDR
#define	OFFSET_IV_ADDR	(EEPROM_START_ADDR + sizeof(float)* 1) 	
#define	OFFSET_OA_ADDR	(EEPROM_START_ADDR + sizeof(float)* 2) 
#define	OFFSET_OV_ADDR	(EEPROM_START_ADDR + sizeof(float)* 3) 
#define	OFFSET_BA_ADDR	(EEPROM_START_ADDR + sizeof(float)* 4) 
#define	OFFSET_OV_SET_ADDR		(EEPROM_START_ADDR + sizeof(float)* 5) 
#define	OFFSET_BAT_ZERO_ADDR		(EEPROM_START_ADDR + sizeof(float)* 6) 
#define	OFFSET_VREF_ADDR			(EEPROM_START_ADDR + sizeof(int)* 7) 
#define	OFFSET_CHECKSUM_ADDR	(EEPROM_START_ADDR + sizeof(float)* 8)

#define	ALARM_ACV_HI_ADDR	(EEPROM_START_ADDR + sizeof(float)* 9) 
#define	ALARM_ACV_LOW_ADDR	(EEPROM_START_ADDR + sizeof(float)* 10) 
#define	ALARM_DCV_HI_ADDR	(EEPROM_START_ADDR + sizeof(float)* 11) 
#define	ALARM_DCV_LOW_ADDR	(EEPROM_START_ADDR + sizeof(float)* 12) 
#define	ALARM_BAT_LOW_ADDR	(EEPROM_START_ADDR + sizeof(float)* 13) 
#define	ALARM_BAT_FAIL_ADDR	(EEPROM_START_ADDR + sizeof(float)* 14) 

#define	OP_OUTPUT_VALUE_ADDR	(EEPROM_START_ADDR + sizeof(float)* 15) 
#define	OP_CURRENT_VALUE_ADDR	(EEPROM_START_ADDR + sizeof(float)* 16) 
#define	OP_BAT_TEST_PERIOD_ADDR	(EEPROM_START_ADDR + sizeof(float)* 17) 
#define	OP_BAT_TEST_VOLTAGE_ADDR	(EEPROM_START_ADDR + sizeof(float)* 18) 
#define	OP_BAT_TEST_DATE_ADDR	(EEPROM_START_ADDR + sizeof(float)* 19) 
#define	OP_BAT_TEST_TIME_ADDR	(EEPROM_START_ADDR + sizeof(float)* 20) 
#define	OP_OUTPUT_MODE_ADDR		(EEPROM_START_ADDR + sizeof(float)* 21) 
#define	OP_CURRENT_MODE_ADDR	(EEPROM_START_ADDR + sizeof(float)* 21+1) 
#define	OP_BAT_TEST_MODE_ADDR	(EEPROM_START_ADDR + sizeof(float)* 21+2) 
#define	OP_RELAY_ADDR				(EEPROM_START_ADDR + sizeof(float)* 21+3) 
#define	ALARM_BAT_T_ADDR			(EEPROM_START_ADDR + sizeof(float)* 22) 
#define	ALARM_REC_T_ADDR			(EEPROM_START_ADDR + sizeof(float)* 23) 
 
#define	OFFSET_BACKUP_ADDR		(EEPROM_START_ADDR + sizeof(float)* 24)


#define	OFFSET_BACKUP_END_ADDR				(OFFSET_BACKUP_ADDR + sizeof(float)* 24)	// +11


#define	CN_INFO_HEAD			OFFSET_BACKUP_END_ADDR
#define	CN_INFO_TAIL			(OFFSET_BACKUP_END_ADDR + 1)
#define	REPAIR_INFO_HEAD		(OFFSET_BACKUP_END_ADDR + 2)
#define	REPAIR_INFO_TAIL		(OFFSET_BACKUP_END_ADDR + 3)

#define	MEMO_INFO_ADDR			(OFFSET_BACKUP_END_ADDR + 4)
#define	MEMO_INFO_SIZE			128

#define	CN_INFO_ADDR			(MEMO_INFO_ADDR+MEMO_INFO_SIZE)
#define	CN_INFO_SIZE			124
#define	REPAIR_INFO_ADDR		(CN_INFO_ADDR + CN_INFO_SIZE * 5)
#define	REPAIR_INFO_SIZE		188
#define	REPAIR_INFO_END_ADDR	(REPAIR_INFO_ADDR + REPAIR_INFO_SIZE * 5)
#define	PSU_NAME_ADDR			REPAIR_INFO_END_ADDR
#define	PSU_NAME_END_ADDR		(PSU_NAME_ADDR + 100)

// 20140225
#define	ALARM_SAVE_ADDR			PSU_NAME_END_ADDR
#define	ALARM_BQL_ADDR			ALARM_SAVE_ADDR
#define	ALARM_OUTPUT_HIGH_ADDR			(ALARM_SAVE_ADDR + 1)
#define	ALARM_MODULE_FAIL_ADDR			(ALARM_SAVE_ADDR + 2)
#define MANAGEMENT_ID_ADDR		(ALARM_SAVE_ADDR + sizeof(int))
#define HWV_EP_ADDR				(MANAGEMENT_ID_ADDR + sizeof(int))		// 2017.2.20
#define SN_EP_ADDR				(HWV_EP_ADDR + sizeof(int))		// 2017.2.20
#define IP_EP_ADDR				(SN_EP_ADDR + 20)						// 2017.2.20
#define MAC_ADDR				(IP_EP_ADDR + sizeof(int))				
#define BAT_EQUIP_ADDR			(MAC_ADDR + 6)
#define ALARM_MODE_ADDR			(BAT_EQUIP_ADDR + 2)
#define FW_DL_ADDR				(ALARM_MODE_ADDR + 2)
#define IP_DUE_ADDR				(FW_DL_ADDR + 4)
#define MANAGEMENT_ID_SAVE		(IP_DUE_ADDR +4)


// 66 bytes space

#define	EEPROM_END				(MANAGEMENT_ID_SAVE + 66 + 4)		// MAC addr = 6bytes

// end



enum {

	LOG_HEADER_ADDR0,
	LOG_HEADER_ADDR1 = LOG_HEADER_ADDR0 + sizeof(LOG_HEADER),
	LOG_START_ADDR = LOG_HEADER_ADDR1 + sizeof(LOG_HEADER),
	LOG_END_ADDR =LOG_START_ADDR + (LOG_MAX_NO) * sizeof(LOG_TYPE),

	EEPROM_START_ADDR = (LOG_END_ADDR + 4), //0x2000,

	EEPROM_LAST_ADDR = EEPROM_END ,
	
};

//#warning EEPROM_LAST_ADDR

enum {
	LOG_FORMAT = 1,
	LOG_RESET_ALARM,
	LOG_CHECKSUM_ERROR,

	LOG_IHV,
	LOG_ILV,
	LOG_IHA, 
	LOG_ILA,
	LOG_OHV,
	LOG_OLV,
	LOG_OHA, 
	LOG_OLA,
	LOG_BDC,
	LOG_BLV,
	LOG_BHA, // CELL FAIL
	LOG_BLA, // MODULE FAIL
	LOG_BHT,
	LOG_RHT,
	LOG_ACF, 
	LOG_BCF,
	
	LOG_IHV_SET,
	LOG_ILV_SET,
	LOG_IHA_SET, 
	LOG_ILA_SET,
	LOG_OHV_SET,
	LOG_OLV_SET,
	LOG_OHA_SET, 
	LOG_OLA_SET,
	LOG_BHV_SET,
	LOG_BLV_SET,
	LOG_BHA_SET, 
	LOG_BLA_SET,
	LOG_BHT_SET,
	LOG_RHT_SET,
	LOG_ACF_SET, 
	LOG_BCF_SET,
	LOG_DEF_SET,
	
	LOG_IV_AD_SET,
	LOG_IA_AD_SET,
	LOG_OV_AD_SET,
	LOG_OA_AD_SET,
	LOG_BA_AD_SET,
	LOG_ZO_AD_SET,
	LOG_ZB_AD_SET,
	LOG_RESET_AD_SET,
	
	LOG_TIME_SET,
	LOG_DATE_SET,	

	LOG_OPERATION_SET,
	LOG_POWER_ON_RESET,
	LOG_SYSTEM_RESET,
	LOG_WATCHDOG_RESET,
	LOG_BAT_TEST_START,
	LOG_BAT_TEST_DONE,
	LOG_BAT_TEST_STOP,
	LOG_BAT_TEST_ON,
	LOG_BAT_TEST_OFF,
	LOG_BAT_TEST_V_SET,
	LOG_BAT_TEST_PERIOD,
	LOG_SHUTDOWN,
	LOG_UNKNOWN_RESET,
	LOG_BQL,

	LOG_CL_ON,
	LOG_CL_OFF,
	LOG_CL_SET,
	LOG_OVP_ON,
	LOG_OVP_OFF,	
	LOG_OVP_FAIL,
	LOG_OCP_ON,
	LOG_OCP_OFF,
	LOG_OCP_FAIL,
	LOG_OTP_ON,
	LOG_OTP_OFF,
	LOG_OTP_FAIL,
	LOG_BAT_MASK,
	LOG_AC_CUT_ON,
	LOG_AC_CUT_OFF,
	LOG_ALARM_MODE,
	LOG_FW_DL_START,
	LOG_FW_DL_DONE,
	LOG_FW_DL_FAIL,
    LOG_UNKNOWN,
};

#define LOG_NRST_RESET LOG_UNKNOWN_RESET

extern	ErrorStatus HSEStartUpStatus;
extern	RECT_STATUS	rect_sts;
extern	ALARM_STATUS	alarm_sts;
extern	SYSTEM_INFO		sys_info;
extern	int	reset_type;
extern  uint8_t SERIAL_NO[];
extern  BMS_STS_T	bms;
extern uint16_t bat_can_sts, bat_can_prt, bat_can_v, bat_can_a, bat_can_flag;



char *get_string2(int arg);

int	read_eeprom(int	addr, byte *data, int size);
int	write_eeprom(int	addr, byte *data, int size);


void	ACV_Write_Offset(float value);
void	ACA_Write_Offset(float value);

float ACA_Read();
float ACV_Read();
float	ACT_Read();

float DCV_Read();
//float	BCV_Read();


float	BATA_Read();
void	BATA_Calibration();
void	LDA_Calibration();
float	LDA_Read();

float	read_input_A();
float	read_output_A();
float	read_bat_A();
float	read_output_V();
float	read_input_V();
float	read_bat_V();
float	read_bat_T();

void	report_alarm(ALARM_STATUS	*alarm);
void	check_alarm(ALARM_STATUS	*alarm);
int		check_range(float value, float max, float min);
void	get_rect_status( RECT_STATUS *sts);
float 	get_temperature(int);

int	eeprom_test();
int	eeprom_erase();


int		write_eeprom(int	addr, byte *data, int size);
int 	read_eeprom(int addr, byte *data, int size);
void	log_init_control(int	mode);
void	log_write(word code, byte status, int pvalue, int cvalue);
int		log_read_header( LOG_HEADER *log);
int		log_checksum(LOG_HEADER *log);
void	log_format_eeprom(LOG_HEADER *log);
int		log_verify_eeprom();
void	log_get_time(LOG_TYPE *event);
void	log_reset();
int	log_read_header_at_reset( LOG_HEADER *log);
int	log_header_checksum(LOG_HEADER *log);


u16		get_alarm_status();
u16		process_alarm_status(ALARM_STATUS	*alarm);

void	alarm_set_init() ;
//void	send_log_record(int no);
float	ad_read_offset(int addr);
void	ad_save_offset(int addr, float value);
//void	set_temp_compensation();
//void	set_current_limit(float	c);
//void	set_output_voltage(float	ov);
byte	get_op_mode();

void	get_system_time(SYSTEM_INFO *sys);

void	alarm_set_default() ;
float	RET_Read();
void	write_current_limit(float	c)	;
void	write_output_voltage(float	ov)	;
void	check_sensor();
u32		get_time_counter();
int		check_battery();

void	log_set_header_checksum(LOG_HEADER *log);
void	wait(unsigned long time)	;	// wait for time * 10ms

void	fb_log_write(word code, byte status, int pvalue, int cvalue);
void	sys_read_hw_version();
void	sys_read_fw_version();


void	offset_save(int addr, byte *data);
void	save_value(int addr, float value);


void	set_alarm_set_log(word code, float pvalue, float cvalue);
void	set_ad_set_log(word code, float pvalue, float cvalue);
void	set_operation_log(word code, float pvalue, float cvalue);
void	set_bat_test_log(word code, float pvalue, float cvalue);
void	set_shutdown_log(word code, float pvalue, float cvalue);
void	set_operation_log(word code, float pvalue, float cvalue);


int	get_log_size();
void	set_output_mode(int mode);
void	set_output_voltage(float	ov);
void	set_current_limit_mode(int mode);
void	sys_set_hw_version(int version);
void	sys_set_sn(uint8_t *sn);

//void	send_test_frame(byte *buf);
void	check_saved_alarm();
void	clear_alarms();
void recovery_start(int mode);
int check_recovery_status();
int	recovery_do();
float BAT_CA();




uint32_t	read_management_id();
void 		save_management_id(uint32_t id);

int	sys_write_mac_info(u8 *mac);
void	sys_read_mac(void);

void recovery_timer_start(void);
void check_OVP(void);

void	 system_shutdown(void);
void	rect_on(int mode);
void	write_shutdown_log(LOG_STATUS  *log);
void	avg_ref(void);

void bat_test_set_v(float	value);
void bat_test_save_mode(int);
void bat_test_save_v(float);
void bat_set_auto_test_day(void);
void bat_test_save_time(void);
void sr_on(void);
void sr_off(void);
void rabm_main(void);

void WD_reset(void);
void save_alarm_mode(int mode);
int	sys_write_ip_info(int ip);
int	get_auto_test_mode();
void set_system_restart(int time);

void set_system_shutdown();
void	set_bat_test_data(int time);
void bat_test_set_mode(int 	mode);
void bat_test_save_period( int month);
void	bat_test_set_period( int month);
void	bat_test_start();
void dac_test();
uint16_t check_alarm_jumper();
uint32_t test_cs5463();
void check_bat_sts();
void check_power_cut(ALARM_STATUS	*alarm);
uint32_t	sys_read_management_id();


#endif
