#include 	"stm32f10x.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include 	"util.h"

#include	"rabm.h"
#include    "rtime.h"
#include 	"eeprom.h"
#include	"command.h"
//#include	"alarmComm.h"
void	system_shutdown();	// alarmcomm.c

extern ADC_VALUE adv;

extern uint16_t dac_value[2];
extern int16_t out_status, port_status;
//extern int shutdown_flag;
extern int	test_mode;
//extern unsigned long  shutdown_time;

extern byte psu_ip[];
extern int yooha_debug;
//extern byte saved_ip[],dipsw_ip[2];;

extern float	Vref;
extern int hw_version;

uint8_t dac_test_flag = 0;


/*
static final int	
CMD_SELF_TEST = 0x80, CMD_ADC_TEST = 0x81, CMD_DAC_TEST = 0x82, 
CMD_AC_TEST = 0x83, CMD_LED_TEST = 0x84, CMD_BUTTON_TEST = 0x85, 
CMD_RELAY_TEST = 0x86, CMD_EPROM_TEST = 0x87;
*/

#define CMD_ALL_TEST  0xa0

IO_STS outputs[16] = {
	{ GPIO_AC_FAIL, AC_FAIL_PIN},
	{ NULL, 0 },
};

IO_STS inputs[16] = {
	{ GPIO_RELAY1, RELAY1_PIN},
	{ GPIO_RELAY2, RELAY2_PIN},
	{ GPIO_RELAY3, RELAY3_PIN},
	{ GPIO_RELAY4, RELAY4_PIN},
	{ GPIO_RELAY5, RELAY5_PIN},
	{ GPIO_RELAY6, RELAY6_PIN},
	{ GPIO_SR_ON, SR_ON_PIN},
	{ GPIO_RECT_CON, RECT_CON_PIN},
	//{ GPIO_AC_FAIL, AC_FAIL_PIN},
	{ GPIO_PFC_TH, PFC_TH_PIN},
	//{ GPIO_PFC1, PFC1_PIN},
	//{ GPIO_PFC2, PFC2_PIN},
	//{ GPIO_PFC3, PFC3_PIN},
	//{ GPIO_PFC4, PFC4_PIN},
		
	{0, 0},
	
};

void	send_test_frame(byte *buf)
{
	int size;

	size = (int ) (buf[1]& 0xff ) + 1;
	size *= sizeof(int);

	send_frame(buf, size);
}

void rect_test_on(int mode)
{
	if ( mode ) {
		rect_on(1);
	}
	else rect_on(0);
}

void LED_test_on()
{
	dprintf("LED on!\n");

	//GPIO_SetBits(GPIO_LED_RUN , LED_RUN);
	//GPIO_SetBits(GPIO_LED_FAIL , LED_FAIL);
}

void LED_test_off()
{
	dprintf("LED off!\n");
	//GPIO_ResetBits(GPIO_LED_RUN , LED_RUN);
	//GPIO_ResetBits(GPIO_LED_FAIL , LED_FAIL);
}

void relay_test_on()
{
	//GPIO_SetBits(GPIO_RELAY, RELAY1|RELAY2|RELAY3|RELAY4|RELAY5|RELAY6);
}


void relay_test_off()
{
	//GPIO_ResetBits(GPIO_RELAY, RELAY1|RELAY2|RELAY3|RELAY4|RELAY5|RELAY6);
}

int	debug_mode_toggle()
{
	yooha_debug = 1 - yooha_debug ;
	return yooha_debug;

}

int	test_mode_toggle()
{
	test_mode = 1 - test_mode ;
	return test_mode;

}

extern uint16_t dac_value[2];

void dac_test()
{
	static int outv = 0;
	DAC_WriteCh(1, dac_value[0]);
	DAC_WriteCh(2, dac_value[1]);
	//DAC_WriteCh(3, dac_value[2]);
	//DAC_WriteCh(4, dac_value[3]);
	return;
}

void	cmd_self_test(byte cmd, byte mode)
{
      byte	buf[16];
      
	buf[0] = cmd ;	// same to the command
	buf[1] = 6;		// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0;	// dummy
	printf("Self test...\n");
}

void	cmd_chip_id(byte cmd, byte mode)
{
	byte	buf[16], wiz;
	int	*pbuf;

	buf[0] = cmd ;	// same to the command
	buf[1] = 2;		// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0;	// dummy

	wiz = IINCHIP_READ(0x003900);	// wiz = 4
	pbuf = (int *) &buf[4];
	//*pbuf = test_cs5463();
        *pbuf = CS5463_Read(0, ECS_EPSILON);			// ecs epsilon = 13			// ecs epsilon = 13
	buf[8] = wiz;
	send_test_frame(buf);
}

void	cmd_get_ip(byte cmd, byte mode)
{
	byte	buf[20];
	int	    ip, *pbuf;

	buf[0] = cmd ;	// same to the command
	buf[1] = 1;		// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0;	// dummy


	pbuf = (int *) &buf[4];
	*pbuf = get_ip_address(psu_ip);


	send_test_frame(buf);
	printf("GET IP ADDR : %02X:%02X:%02X:%02X\n", psu_ip[0], psu_ip[1], psu_ip[2], psu_ip[3]);

}


void	cmd_adc_test(byte cmd, byte mode)
{

	byte	buf[30];
	int	*pbuf;

	buf[0] = cmd ;	// same to the command
	buf[1] = 5;		// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0;	// dummy

	pbuf = (int *) &buf[4];

	*pbuf++ = adv.dcv;
	*pbuf++ = adv.dca;
	*pbuf++ = adv.rect;
	*pbuf++ = adv.batt;
	*pbuf = adv.refv;

	printf("ADC: %d %d", adv.dcv,  adv.dca);
	printf(" %d %d %d\n", adv.rect,  adv.batt, adv.refv);
	//*pbuf++ = ADC_ReadCh(RECT_T_CH);	// BAT TEMP	
	//*pbuf++ = ADC_ReadCh(10);	// OUT_V
	//*pbuf++ = ADC_ReadCh(11);	// INPUT_A
	//*pbuf++ = ADC_ReadCh(12);	// BAT_A
	//*pbuf++ = ADC_ReadCh(CONT_T_CH);	// RECT TEMP  	  

	send_test_frame(buf);

}
			
void	cmd_dac_test(byte cmd, int value)
{
	byte	buf[20], *pdata;
	int	   *pbuf;
	int  dac1, dac2;
	double  a, v;

	buf[0] = cmd ;	// same to the command
	buf[1] = 2;		// no of test result
	buf[2] = 0;	//dummy
	buf[3] = 0;	// dummy

	pdata = (byte *) &value;
	if ( value & 0x80000000) {
		dac1 =  pdata[0]  + ((pdata[1]& 0x3f) * 256);		
		dac_value[2] = (uint16_t) dac1;

		dac2 = pdata[2] + ( pdata[3]  * 256);
		dac_value[3] = (uint16_t) dac2;
		printf(" ADV: %d, %d\n", value, dac_value[2], dac_value[3]);
	}
	else {
		dac1 = pdata[0] + (pdata[1] * 256);		
		dac_value[0] = (uint16_t) dac1;

		dac2 = pdata[2] + (pdata[3] * 256);
		dac_value[1] = (uint16_t)  dac2; 
		printf(" ADV: %d, %d\n", dac_value[0], dac_value[1]);
	}

	printf("DAC: %x : %d, %d\n", value, dac1, dac2);	

	send_test_frame(buf);

	#if 0
	u16 addr;
	addr = ip_read_switch();
	printf("ADDR = 0x%04x\n", addr);
	#endif

}

void	cmd_ac_test(byte cmd, byte mode)
{
	byte	buf[20];

	float 	value1, value2;
	int		*pbuf;

	buf[0] = cmd ;	// same to the command
	buf[1] = 2;		// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0;	// dummy

	pbuf = (int *) &buf[4];

	value1 = ACV_Read();
	*pbuf++ = (int) (value1 * 100);
	value2 = ACA_Read();
	*pbuf++ = (int) (value2 * 100);

	send_test_frame(buf);

	printf("AC test : ACV = %f, ACA=%f\n", value1, value2);
	ACT_Read();

}

void	cmd_led_test(byte cmd, byte mode)
{
	byte	buf[10];

	buf[0] = cmd ;	// same to the command
	buf[1] = 0;
	buf[2] = mode;	//dummy
	buf[3] = 0; // dummy

	if ( mode == 3 ) {	// turn on rect_off
		rect_test_on(1);
	}
	else if ( mode == 2 ) {	// turn off rect_off
		rect_test_on(0);
	}
	send_test_frame(buf);
}

void	cmd_boot0_test(byte cmd, byte mode)
{
	byte	buf[10];
	int 	*pbuf;

	buf[0] = cmd ;	// same to the command
	buf[1] = 1; 	// no of test result
	buf[2] = mode;	//mode
	buf[3] = 0; // dummy

	pbuf = (int *) &buf[4];
	printf("BOOT0\n");
	if ( GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8) ) *pbuf = 1;
	else *pbuf = 0;

	send_test_frame(buf);

}


void	cmd_button_test(byte cmd, byte mode)
{
	byte	buf[10];
	int 	i;
	uint16_t *pbuf, status;
	IO_STS *port;

	buf[0] = cmd ;	// same to the command
	buf[1] = 1; 	// no of test result
	buf[2] = mode;	//mode
	buf[3] = 0; // dummy

	pbuf = (uint16_t *) &buf[4];

	status = 0;
	port = inputs;
	for ( i = 0; i < 16; i++) {
		if ( port->group == 0 ) {
			break;
		}
		if ( GPIO_ReadInputDataBit(port->group, port->pin)== 1 ) {
			status |= (1 << i);
		}
		port++;
	}

	*pbuf = status;
	printf("INPUT TEST:%x\n", status);

	send_test_frame(buf);

}


void	cmd_output_test(byte cmd, byte mode)
{
	byte	buf[10];
	int 	i;
	uint16_t *pbuf, status;
	IO_STS *port;

	buf[0] = cmd ;	// same to the command
	buf[1] = 1; 	// no of test result
	buf[2] = mode;	//mode
	buf[3] = 0; // dummy

	printf("OUTPUT Test:0x%x\n", mode);

	pbuf = (uint16_t *) &buf[4];

	if ( mode ) status = 0xffff;
	else status = 0;
	out_status = status;
	port = outputs;
	for ( i = 0; i < 16; i++) {
		if ( port->group == 0 ) {
			break;
		}
		if ( status & (0x01<<i)) {
			GPIO_ResetBits(port->group, port->pin);
		}
		else GPIO_SetBits(port->group, port->pin);
		port++;
	}

	*pbuf = status;

	
	send_test_frame(buf);
}


void	cmd_relay_test(byte cmd, byte mode)
{
	byte	buf[10];

	buf[0] = cmd ;	// same to the command
	buf[1] = 0; 	// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0; // dummy

	printf("RELAY TEST:%d\n", mode);

	if ( mode ) {
		relay_on(1);
		relay_on(2);
		relay_on(3);
		relay_on(4);	
		relay_on(5);
		relay_on(6);	
		sr_off();
		GPIO_SetBits(GPIO_RECT_ON , RECT_ON);	// rect(1)
	}
	else {
		relay_off(1);
		relay_off(2);
		relay_off(3);
		relay_off(4);	
		relay_off(5);
		relay_off(6);
		sr_on();
		GPIO_ResetBits(GPIO_RECT_ON , RECT_ON);	// rect(0);
	}
	
	send_test_frame(buf);

}

void	cmd_eprom_test(byte cmd, byte mode)
{
	byte	buf[10];
	int 	value, *pbuf;

	buf[0] = cmd ;	// same to the command
	buf[1] = 1; 	// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0; // dummy

	printf("eprom test:");
	pbuf = (int *) &buf[4];

	if ( eeprom_test() == 1)  {
		value = 1;
		printf("OK\n");
	}
	else {
		value = 0;
		printf("ERROR\n");
	}

	*pbuf = value;
	send_test_frame(buf);

}


void	cmd_eprom_erase(byte cmd, byte mode)
{
	byte	buf[30];
	int 	value, *pbuf;

	buf[0] = cmd ;	// same to the command
	buf[1] = 1; 	// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0; // dummy

	printf("EPROM erase!\n");

	set_ad_reset();
	pbuf = (int *) &buf[4];

	if ( eeprom_erase() == 1) value = 1;
	else value = 0;

	*pbuf = value;
	send_test_frame(buf);

}


void	cmd_system_shutdown(byte cmd, byte mode)
{
	byte	buf[10];

	buf[0] = cmd ;	// same to the command
	buf[1] = 0; 	// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0; // dummy

	printf("DB Shutdown\n");


	send_test_frame(buf);

	system_shutdown();


}

void	cmd_set_hw_version(byte cmd, byte mode)
{
	byte	buf[10];
	int 	version, *pbuf;

	version = get_intvalue2(1);	
	sys_set_hw_version(version);
	
	buf[0] = cmd ;	// same to the command
	buf[1] = 1; 	// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0; // dummy

	printf("HW ver:%x\n", version);
	pbuf = (int *) &buf[4];
	*pbuf = hw_version;
	send_test_frame(buf);
}

void	cmd_set_mac(byte cmd, byte mode)
{
	byte	buf[10];
	int 	*pbuf;
	unsigned char	*sn, *mac;

	mac = get_data(1, 16);	// 6+ VERSION_SIZE 	
	sys_set_mac(mac);

	buf[0] = cmd ;	// same to the command
	buf[1] = 1; 	// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0; // dummy


	pbuf = (int *) &buf[4];
	*pbuf = 1;

	send_test_frame(buf);
}


void	cmd_set_sn(byte cmd, byte mode)
{
	byte	buf[10];
	int 	*pbuf;
	unsigned char	*sn;

	sn = get_data(1, 16) + 6;
	//printf("SN:%s\n", sn);
	sys_set_sn(sn);

	
	buf[0] = cmd ;	// same to the command
	buf[1] = 1; 	// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0; // dummy


	pbuf = (int *) &buf[4];
	*pbuf = 1;

	send_test_frame(buf);
}

void	cmd_debug_mode(byte cmd, byte mode)
{
	byte	buf[20];
	int 	debug;

	debug = debug_mode_toggle();
	debug = mode;
	buf[0] = cmd ;	// same to the command
	buf[1] = 1; 	// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0; // dummy

	printf("Debug mode = %d\n", debug);
	buf[4] = ( byte )debug & 0xff;
	buf[5] = 0;
	buf[6] = 0;
	buf[7] = 0;

	send_test_frame(buf);
}


void	cmd_test_mode(byte cmd, byte mode)
{
	byte	buf[20];
	//int 	debug;

	//debug = test_mode_toggle();
	test_mode = mode;
	dac_test_flag = 0;
	buf[0] = cmd ;	// same to the command
	buf[1] = 1; 	// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0; // dummy

	//printf("Test mode = %d\n", test_mode);
	//buf[4] = ( byte ) debug& 0xff;
    buf[4] = ( byte ) mode & 0xff;
	buf[5] = 0;
	buf[6] = 0;
	buf[7] = 0; 

	send_test_frame(buf);
}


void	cmd_info_set(byte cmd, byte mode)
{
	byte	buf[20];
	char	*info;

	info = get_string2(1);

	if ( strlen(info) >= MEMO_INFO_SIZE ) info[MEMO_INFO_SIZE-1] = 0;

	sys_set_info(info);
	buf[0] = cmd ;	// same to the command
	buf[1] = 0; 	// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0; // dummy
	dprintf("memo info:%s\n", info);

	send_test_frame(buf);
}

void	cmd_out_sel(byte cmd, byte mode)
{
	printf("OUT SEL: %x\n", mode);
	if ( mode ) {
		GPIO_SetBits(GPIO_ETH_SEL, ETH_SEL_PIN);
	}
	else GPIO_ResetBits(GPIO_ETH_SEL, ETH_SEL_PIN);
}

void diagnosis(byte command) 
{
	//printf("Diagnosis Mode...");

	byte mode;
	int data;		// 4bytes

	mode = get_mode();

	switch (command ) {
		case CMD_CHIP_ID :
			cmd_chip_id(command, mode);
			break;
		case CMD_GET_IP :
			cmd_get_ip(command, mode);
			break;
		case CMD_SELF_TEST :
			cmd_self_test(command, mode);
			break;
		case CMD_ADC_TEST:
			cmd_adc_test(command, mode);
			break;
		case CMD_DAC_TEST:
			data = get_4byte(1);
			cmd_dac_test(command, data);
			break;
		case CMD_AC_TEST:
			cmd_ac_test(command, mode);
			break;
		case CMD_LED_TEST:
			cmd_led_test(command, mode);
			break;
		case CMD_BUTTON_TEST:			
			cmd_button_test(command, mode);
			break;
		case CMD_RELAY_TEST:
			cmd_relay_test(command, mode);
			break;
		case CMD_EPROM_TEST:
			cmd_eprom_test(command, mode);
			break;
		case CMD_EPROM_ERASE:
			cmd_eprom_erase(command, mode);
			break;
		case CMD_SYSTEM_SHUTDOWN:
			cmd_system_shutdown(command, mode);
			break;
		case CMD_SET_HW_VERSION:
			cmd_set_hw_version(command, mode);
			break;
		case CMD_SET_SN:
			cmd_set_sn(command, mode);
			break;
		case CMD_SET_MAC:
			cmd_set_mac(command, mode);
			break;
		case CMD_DEBUG_MODE:
			cmd_debug_mode(command, mode);
			break;

		case	CMD_INFO_SET:
			cmd_info_set(command, mode);
			break;
		case	CMD_TEST_MODE:
			cmd_test_mode(command, mode);
			break;
		case CMD_BOOT0_TEST:
			cmd_boot0_test(command,mode);
			break;
		case CMD_OUTPUT_TEST:
			cmd_output_test(command,mode);
			break;
		case CMD_OUT_SEL:
			cmd_out_sel(command, mode);
			break;

		default :
			//printf("Invalid TEST cmd : %x", command);
			break;

	}

}

