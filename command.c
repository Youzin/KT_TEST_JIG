
#include 	"stm32f10x.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include	"util.h"
#include	"command.h"
#include	"rabm.h"
#include	"rtime.h"
#include	"debug.h"

//#include	"alarmComm.h"
int	ip_write_psu_name(char *name);	// in alarmcomm.c

// 2018 0717
extern int	ip_state;
extern int	fusing_size, fusing_mod;
extern int	fusing_count;
extern int	fusing_total_size;
extern u16	operation_mode;
extern int yooha_debug;
extern uint16_t	adc_dr[];

extern float	Vref;
extern u8	saved_management_id[], saved_ip[];
extern u32	management_id;

extern LOG_HEADER log_header;
extern int log_send_size, log_sending;
extern int log_send_index;

extern RECT_STATUS		rect_sts;
extern ALARM_STATUS	alarm_sts;
extern AD_SETUP		ad_offset;
extern OP_STATUS		op_sts;
extern LOG_STATUS		log_sts;
extern SYSTEM_INFO		sys_info;
extern BAT_TEST_DATA	bat_data;
extern BAT_AUTO_TEST_INFO	auto_test;

extern	char user_psu_name[];
extern	byte 	USART_line_buffer[];
extern	byte 	psu_mac[];
extern  int forced_bat_test;


byte	rx_frame[256];
byte	rx_buffer[256];

char	rx_array[256];

F_REC_STATUS	rec_status;	// packets to send PC
F_ALARM_SETUP	alarm_set;
F_AD_SETUP		ad_set;
F_LOG_TYPE		log_frame;
F_OP_SETUP		op_frame;
F_SYSTEM_INFO	sys_frame;
F_PSU_NAME		psu_name_set;
F_DL_ACK		dl_ack_frame;


void	set_psu_name(char *name)
{
	ip_write_psu_name( name);
	send_psu_name();
}


void	set_bat_test_on()
{
	printf("%s\n", __FUNCTION__);
	
	bat_test_set_mode(1);
	bat_test_save_mode(1);
	send_op_set();
}

void	set_bat_test_off()
{
	printf("%s\n", __FUNCTION__);

	bat_test_set_mode(0);
	bat_test_save_mode(0);
	send_op_set();
}

void	set_bat_test_v(float val)
{
	//printf("%s %f\n", __FUNCTION__, val);
	op_sts.bat_test_V = val;
	printf("B SETV %f\n", op_sts.bat_test_V);

	bat_test_set_v(val);
	bat_test_save_v(val);
	send_op_set();
}

void	set_bat_test_period(int month)
{
	//printf("%s %d\n", __FUNCTION__, month);

	bat_test_set_period(month);
	bat_test_save_period(month);
	bat_test_save_time();
	send_op_set();
}

void	set_bat_test_req(int time)
{
	printf("%s %d\n", __FUNCTION__, time);
	if ( op_sts.bat_equip ) {
		printf("BAT TEST Cancelled!\n");
		return;
	}

	forced_bat_test = 1;
	bat_test_start();
#if 0 // IP bat test???
	int result;
	
	//printf("MTest REQ : %f, %d\n", op_sts.bat_test_V, time);

	result = ip_battery_test_start( (int)(op_sts.bat_test_V * 10.0), time/60);
	bat_data.result = result;
	send_bat_test_data();
#endif
}


void	set_bat_test_day(int ymd)
{
	byte *d;
	d = (byte *) &ymd;

	//printf("%s %d\n", __FUNCTION__, ymd);
	
	op_sts.month = d[2] & 0xff;
	op_sts.date = d[1] & 0xff;	
	bat_set_auto_test_day();
	bat_test_save_time();
	send_op_set();
}


void	set_bat_test_time(int time)
{
	byte *t;
	t = (byte *) &time;

	//printf("%s %d\n", __FUNCTION__, time);
	op_sts.hour = t[3] & 0xff;
	op_sts.min = t[2] & 0xff;
	bat_set_auto_test_day();
	bat_test_save_time();
	send_op_set();

}


void set_ad_iv(float value) 
{
	set_ad_set_log(LOG_IV_AD_SET, rect_sts.input_V, value);	
	rect_sts.iv_offset  = value - ACV_Read();
	ad_offset.iv = value;
	send_ad_set();	
	ad_save_offset(OFFSET_IV_ADDR, rect_sts.iv_offset);        
    value = ad_read_offset(OFFSET_IV_ADDR);
	//printf("****  IV_offset : %f \n", value);
}

void set_ad_ia(float value) 
{
	set_ad_set_log(LOG_IA_AD_SET, rect_sts.input_A, value);
	rect_sts.ia_offset  = value - ACA_Read();
	//rect_sts.ia_offset  += value - read_input_A();
	ad_offset.ia = value;
	send_ad_set();
	ad_save_offset(OFFSET_IA_ADDR, rect_sts.ia_offset);
}

void set_ad_ov(float value) 
{
	float ov;

	ov = op_sts.output_V - rect_sts.ov_set_offset;
	set_ad_set_log(LOG_OV_AD_SET, rect_sts.output_V, value);
	rect_sts.ov_offset = value -  DCV_Read();
	rect_sts.ov_set_offset = value - ov;
	//rect_sts.ov_set_offset = value - op_sts.output_V;
	ad_offset.ov = value;

	// 0816
	//op_sts.output_V = value;
	//write_output_voltage(op_sts.output_V);
	//write_eeprom(OP_OUTPUT_VALUE_ADDR  , (byte*) &op_sts.output_V, sizeof(float));
	// 0816

	send_ad_set();
	ad_save_offset(OFFSET_OV_ADDR, rect_sts.ov_offset);
	ad_save_offset(OFFSET_OV_SET_ADDR, rect_sts.ov_set_offset);

	printf("OV_offset : %5.2f, %5.2f\n", rect_sts.ov_offset,  rect_sts.ov_set_offset);
}

void set_ad_oa(float value) 
{
	set_ad_set_log(LOG_OA_AD_SET, rect_sts.output_A, value);
	rect_sts.oa_offset  = value - LDA_Read();
	//rect_sts.oa_offset += value - read_output_A();
	ad_offset.oa = value;
	send_ad_set();
	ad_save_offset(OFFSET_OA_ADDR, rect_sts.oa_offset);
}

void set_ad_ba(float value) 
{
	set_ad_set_log(LOG_BA_AD_SET, rect_sts.bat_A, value);
	rect_sts.ba_offset  = value - BATA_Read();
	ad_offset.ba = value;
	send_ad_set();
	ad_save_offset(OFFSET_BA_ADDR, rect_sts.ba_offset);
}

void set_ad_zero_output() 
{
	float offset = 0.0;
    int i = 0;
	
    while (i++ < 5) { 
      offset += (-1* LDA_Read());
      Delay(0x20);
	  IWDG_ReloadCounter();
    } 	
	rect_sts.oa_offset = offset / 5.0;

	ad_save_offset(OFFSET_OA_ADDR, rect_sts.oa_offset);
	ad_save_offset(OFFSET_VREF_ADDR, Vref);
	//printf("%s \n", __FUNCTION__);
}

void ad_zero_battery() 
{
	int	offset = 0, i;

	for ( i=0; i < 5; i++) {
		//offset += ADC_ReadCh(12);
		offset += (adc_dr[IDX_BATA] & 0xfff);
		//Delay_us(250);
	}

	ad_offset.bat_zero_offset =  offset / 5;	
	set_ad_set_log(LOG_ZB_AD_SET, 0, 0);
	rect_sts.ba_offset =  -1 * BATA_Read();
	ad_save_offset(OFFSET_BA_ADDR, rect_sts.ba_offset);
	offset_save(OFFSET_BAT_ZERO_ADDR, (byte *) & ad_offset.bat_zero_offset);
	//send_ad_set();

	//printf("Bat Zero = %d, ba = %5.2f \n", ad_offset.bat_zero_offset, rect_sts.ba_offset);
}

void	set_ad_reset()
{
	//printf("AD_reset \n");
	//set_ad_set_log(LOG_RESET_AD_SET, 0, 0);
	rect_sts.iv_offset = 0.0;
	rect_sts.ia_offset = 0.0;
	rect_sts.ov_offset = 0.0;
	rect_sts.ov_set_offset = 0.0;
	rect_sts.oa_offset = 0.0;
	rect_sts.ba_offset = 0.0;

	ad_offset.iv = 0.0;
	ad_offset.ia = 0.0;
	ad_offset.ov = 0.0;
	ad_offset.oa = 0.0;
	ad_offset.ba = 0.0;

	ad_offset.bat_zero_offset = (int ) (2.5 * 4095.0 / Vref);	
	ad_save_offset(OFFSET_IV_ADDR, 0);
	ad_save_offset(OFFSET_IA_ADDR, 0);

	ad_save_offset(OFFSET_OV_ADDR, 0);
	ad_save_offset(OFFSET_OA_ADDR, 0);
	ad_save_offset(OFFSET_BA_ADDR, 0);
	ad_save_offset(OFFSET_OV_SET_ADDR, 0);

	//printf("%s \n", __FUNCTION__);

}

void	set_ad_zero()
{
	int  offset = 0;
	//float ref;

	set_ad_reset();

	offset= 0;

	//ref = 2.5 / Vref * 4095 ;
	ad_offset.bat_zero_offset = ( int ) (offset / 10.0);

	BATA_Calibration();
	
	//printf("Vref = %5.3fV, offset = %d\n", Vref,ad_offset.bat_zero_offset );

	//ref = rect_sts.ba_offset;
	send_ad_set();

	ad_save_offset(OFFSET_BA_ADDR, rect_sts.ba_offset);
	offset_save(OFFSET_BAT_ZERO_ADDR, (byte *) & ad_offset.bat_zero_offset);
	set_ad_set_log(LOG_ZB_AD_SET, 0, 0);
	printf("Zero OFFSET = %d, B_OFFSET = %5.2f \n", ad_offset.bat_zero_offset, rect_sts.ba_offset);

}

void	set_auto_test_mode(int mode)
{
	if ( mode == 0) {
		op_sts.bat_test_mode = mode;
	}
	else op_sts.bat_test_mode = 1;
}

void	set_bat_test_data(int time)
{
	bat_data.bat_V = rect_sts.output_V;
	bat_data.bat_A = rect_sts.bat_A;
	bat_data.time  = time;
	bat_data.rate = ip_bat_test_residual() /10.0;
	bat_data.send_flag  = 1;
}

void set_alarm_ihv(float value) {

	set_alarm_set_log(LOG_IHV_SET, alarm_sts.hi_input_V, value);
	alarm_sts.hi_input_V = value;
	save_value(ALARM_ACV_HI_ADDR, value);
	send_alarm_set();

	//printf("ihv = %f", alarm_sts.hi_input_V);
}

void set_alarm_ilv(float value) {
	set_alarm_set_log(LOG_ILV_SET, alarm_sts.low_input_V, value);
	alarm_sts.low_input_V = value;
	save_value(ALARM_ACV_LOW_ADDR, value);
	send_alarm_set();
	//printf("ilv = %f", alarm_sts.low_input_V);
}

void set_alarm_ohv(float value) {
	set_alarm_set_log(LOG_OHV_SET, alarm_sts.hi_output_V, value);
	alarm_sts.hi_output_V = value;
	save_value(ALARM_DCV_HI_ADDR, value);
	send_alarm_set();
}

void set_alarm_olv(float value) {
	set_alarm_set_log(LOG_OLV_SET, alarm_sts.low_output_V, value);
	alarm_sts.low_output_V = value;
	save_value(ALARM_DCV_LOW_ADDR, value);
	send_alarm_set();
}

void set_alarm_bfv(float value) {
	set_alarm_set_log(LOG_BCF_SET, alarm_sts.low_fail_bat, value);
	alarm_sts.low_fail_bat = value;
	save_value(ALARM_BAT_FAIL_ADDR, value);
	send_alarm_set();
}

void set_alarm_bhv(float value) {
	set_alarm_set_log(LOG_BHV_SET, alarm_sts.hi_bat_V, value);
	alarm_sts.hi_bat_V = value;
	save_value(ALARM_BAT_FAIL_ADDR, value);
	send_alarm_set();
}

void set_alarm_blv(float value) {
	set_alarm_set_log(LOG_BLV_SET, alarm_sts.low_bat_V, value);
	alarm_sts.low_bat_V = value;
	save_value(ALARM_BAT_LOW_ADDR, value);
	send_alarm_set();
}


void set_alarm_bht(float value) {
	set_alarm_set_log(LOG_BHT_SET, alarm_sts.hi_bat_T, value);
	alarm_sts.hi_bat_T = value;
	save_value(ALARM_BAT_T_ADDR, value);
	send_alarm_set();
}
void set_alarm_rht(float value) {
	
	set_alarm_set_log(LOG_RHT_SET, alarm_sts.hi_rec_T, value);
	alarm_sts.hi_rec_T = value;
	save_value(ALARM_REC_T_ADDR, value);
	send_alarm_set();
}

void	set_alarm_default(){
	set_alarm_set_log(LOG_DEF_SET, 0, 0);
	alarm_set_default();
	send_alarm_set();

}


void	set_alarm_clear(int mode)
{
	byte	buf[30];

	buf[0] = SET_BQL ;	// same to the command
	buf[1] = 0; 	// no of test result
	buf[2] = mode;	//dummy
	buf[3] = 0; // dummy

	printf("CLEAR ALARM : %d\n", mode);
	send_test_frame(buf);

	if ( mode == 0 ) {
		//clear_bql();
		clear_alarms();
	}
	//else 	set_bql();

}

int	get_command()
{
	int	ret = 0;
	
	if ( get_frame(rx_frame) )	{
		ret = rx_frame[1];
		//printf("CMD : %d\n", ret);
	}

	return ret;
}

char *get_str(int arg)
{
	char	*str;
	int	    index;
      
    index = (arg -1) * 4 + 2;
	str   =  &rx_frame[index];

	//dprintf("Get Str: %s\n", str);
	
	return str;
}


int	get_4byte(int arg)
{
	int	iv, index;
      
    index = (arg -1) * 4 + 2;
	iv   = (rx_frame[index]) & 0xff;
	iv |= ( rx_frame[index+1] << 8) & 0xff00;
	iv |= ( rx_frame[index+2] << 16) & 0xff0000;
	iv |= ( rx_frame[index+3] << 24) & 0xff000000;
	//printf("Get Int: %x\n", iv);	
	return iv;

}


int	get_intvalue(int arg)
{
	int	iv, index;
      
    index = (arg -1) * 4 + 2;
	iv   = (rx_frame[index] << 24) & 0xff000000;
	iv |= ( rx_frame[index+1] << 16) & 0xff0000;
	iv |= ( rx_frame[index+2] << 8)  & 0xff00;
	iv |=  rx_frame[index+3] & 0xff;
	//printf("Get Int: %x\n", iv);	
	return iv;
}

float	get_floatvalue(int arg)
{
	float value;
	int	iv, index;
      
    index = (arg -1) * 4 + 2;
	iv   = (rx_frame[index] << 24) & 0xff000000;
	iv |= ( rx_frame[index+1] << 16) & 0xff0000;
	iv |= ( rx_frame[index+2] << 8)  & 0xff00;
	iv |=  rx_frame[index+3] & 0xff;

	value = ((float) iv ) / 100.0;
	//dprintf("Get F: %f, i:%d \n", value, iv);
	
	return value;
}

int	get_4byte2(int arg)
{
	int	iv, index;
      
    index = (arg -1) * 4 + 2;
	iv   = (rx_frame[index]) & 0xff;
	iv |= ( rx_frame[index+1] << 8) & 0xff00;
	iv |= ( rx_frame[index+2] << 16) & 0xff0000;
	iv |= ( rx_frame[index+3] << 24) & 0xff000000;
	//printf("Get Int: %x\n", iv);	
	return iv;
}


int	get_intvalue2(int arg)
{
	int	iv, index;
      
    index = (arg -1) * 4 + 3;
	iv   = (rx_frame[index] << 24) & 0xff000000;
	iv |= ( rx_frame[index+1] << 16) & 0xff0000;
	iv |= ( rx_frame[index+2] << 8)  & 0xff00;
	iv |=  rx_frame[index+3] & 0xff;


	//printf("Get Int: %d \n", iv);
	
	return iv;
}

char *get_string2(int arg)
{
	int	count = 0, index;
	char	*str;
      
    index = (arg -1) * 4 + 3;
	str = rx_array;

	*str = rx_frame[index];
	while ( *str != 0 && count++ < 20 ) {
		//printf(" %x,", *str);
		str++; 
		index++;
		*str   = rx_frame[index];	
		
	}
	//printf("Get string: %s \n", rx_array);
	
	return rx_array;
}

byte	get_mode()
{
	return rx_frame[2];
}


unsigned char *get_data(int arg, int size)
{
	int	count = 0, index;
	char	*data;

    index = (arg - 1) * 4 + 2;
	data = rx_array;

	//printf("GET DATA \n");

	while (  count++ < size ) {
		*data   = rx_frame[index];	
		//printf("%02X, ", *data);
		data++;
		index++;		
	}
	//printf("\n");
	return rx_array;

}

void	do_test_mode()
{
	int command;
	
	command = get_command();
	if ( command ) {
		rabm_process_command(command);
		//display_status();
	}
	//if ( !(time10ms % 20) ) GPIO_WriteBit(GPIO_SYSTEM_LED, SYSTEM_LED, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIO_SYSTEM_LED, SYSTEM_LED)));
}



void send_dl_ack(int16_t res)
{

	dl_ack_frame.command = FW_ACK;
	dl_ack_frame.mode = 0;
	dl_ack_frame.para = res;
	send_frame((byte *)&dl_ack_frame, (byte) sizeof(F_DL_ACK) );
}

int download_fw(byte *p)
{


	F_DOWNLOAD *dl;
	F_DL_DATA	*dd;
	//F_DL_ACK	da;

	int16_t result = 0;
	int	count = 0;
	int	restart = 0; //, size;
	byte mode;

	dl = (F_DOWNLOAD *) p;
	mode = dl->mode;

	//printf("DOWNLOAD REQ : %d \n", mode);

	switch ( mode ) {
		case DL_START:	// start fusing
			fusing_size = dl->para;			
			
			if ( ip_state == IP_FUSING_STATE  ) {
				result = 1;
				fusing_count = 0;
				ip_state = IP_IDLE_STATE;
				operation_mode = NORMAL_MODE;
				
			}
			else if ( ip_state != IP_IDLE_STATE) result = -101;
			else if ( fusing_size > ( 0x1c000 / 100 ) ) result = -111;
			else  if (operation_mode == NORMAL_MODE) {
				result = 0;
				fusing_count = 0;
				ip_state = IP_FUSING_STATE;
				operation_mode = FUSING_MODE;
				flash_erase(FUSING_ADDRESS, (fusing_size * 100 / 0x800) + 1);
			}
			else {
				result = -101;
			}

			log_write(LOG_FW_DL_START, 0, 0, 0);

			printf("Fusing Start : size = %x result = %d\n", fusing_size, result);
			break;
		case DL_DATA:	// transfer req
			//send_req = (MSG_FUSING_SEND_REQ *) (rx->data);
			//data = set_word(send_req->seq_no);
			dd = (F_DL_DATA *) dl;
			count = dd->count;
			
			
			if ( ip_state == IP_FUSING_STATE ) {
				fusing_count++;
				if ( count == fusing_count ) {
					result = 0;
					if ( count == 1 ) {
						result =  fusing_check_file(dd->data);
					}		
					else if ( count == 2 ) {	// skip this frame
						result = 0;
					}
					else if ( rs_fusing_image(fusing_count-2, dd->data) != 0) result = -201; // data from  seq = 3;

					if ( result != 0 ) {
						ip_state = IP_IDLE_STATE;
						operation_mode = NORMAL_MODE;
					}
				}
				else if ( count == (fusing_count-1) ) {
					result = 1;	
					fusing_count--;

				}
				else if (count > fusing_count ) {
					result = -113;
				}
				else if ( count < (fusing_count-1)) {
					result = -112;
				}
			}
			else {
				result = -12;			
			}	
			if ( result != 0 ) printf("Fusing Error: count = %x, res= %d\n", count, result);
			break;
		case	DL_END_START:	// end of fusing & restart
			printf("Restart, ");
			restart = 1;			
			
		case	DL_END:	// end of fusing
			printf("End Fusing !!\n");
			//end_req = (MSG_FUSING_END_REQ *) (rx->data);
			//fusing_mod = set_word( end_req->mod);		
			fusing_mod =  dl->para;

			if ( ip_state == IP_FUSING_STATE ) {
				if ( fusing_mod > 100 ) {
					result = -106;
				}
				else if ( fusing_mod < 0 ) {
					result = -105;
				}
				else  {
					result = fusing_finish(100);
					if ( result == -1)  result = -201;					
				}
			}
			else {
				result =  -12;
			}
		
			ip_state = IP_IDLE_STATE;
			operation_mode = NORMAL_MODE;
			
			log_write(LOG_FW_DL_DONE, 0, 0, 0);

			printf("Fusing End: mod = %x result = %d\n", fusing_mod, result);
			break;

		case	DL_CANCEL:	// cancel fusing
			printf("Fusing Cancelled\n");
			ip_state = IP_IDLE_STATE;
			operation_mode = NORMAL_MODE;
			result = -1;
			break;
		default :
			result = -102;
			break;		

	}



	send_dl_ack( result);

	//printf("Result = %d\n", result);

	if ( result == 0  && restart == 1)  {
		set_system_restart(50);
		//ip_reboot_noti();
	}

	if ( result != 0 ) {
		ip_state = IP_IDLE_STATE;
		operation_mode = NORMAL_MODE;		
		log_write(LOG_FW_DL_FAIL, 0, 0, 0);
	}
	return result;
}

void	rabm_process_command(int cmd)
{
	int	value1; //, value2;
	float	value;
	char	*str;

	switch (cmd) {
		case SET_ALARM_MODE:
			value1 = get_intvalue(1);
			set_alarm_mode(value1);
			break;
		case FW_DOWNLOAD:
			download_fw( &rx_frame[1]);
			break;
		case GET_REC_STATUS:
			send_rect_status();
		break;
		case	GET_ALARM_STATUS:
			send_alarm_set();
			break;
		case	GET_SYS_INFO:
			send_system_info();
			break;
		case	GET_SYS_TIME:
			send_system_time();
			break;
		case	GET_AD_SETUP:
			send_ad_set();
			break;
		case	GET_OPERATION_SETUP:
			send_op_set();
			break;
		case	GET_LOG:
			value1 = get_log_size();
			send_log_record(value1);
			break;
		case	GET_LOG_NO:
			send_log_no();
			break;
		case SET_MANAGEMENT_ID:
			value1 = get_4byte(1);
			set_management_id(value1);
			break;
		case	SET_LOG_CLEAR:
			set_log_clear();
			break;
		case	SET_ALRAM_INPUT_HI_V:
			value = get_floatvalue(1);
			set_alarm_ihv(value);
			break;
		case	SET_ALARM_INPUT_LOW_V:
			value = get_floatvalue(1);
			set_alarm_ilv(value);
			break;
		case	SET_ALARM_OUTPUT_HI_V:
			value = get_floatvalue(1);
			set_alarm_ohv(value);
			break;		
		case	SET_ALRAM_OUTPUT_LOW_V:
			value = get_floatvalue(1);
			set_alarm_olv(value);
			break;
		case	SET_ALARM_BAT_LOW_V:
			value = get_floatvalue(1);
			set_alarm_blv(value);
			break;
		case	SET_ALARM_BAT_FAIL_V:
			value = get_floatvalue(1);
			set_alarm_bfv(value);
			break;
		case	SET_ALARM_BAT_HI_T:
			value = get_floatvalue(1);
			set_alarm_bht(value);
			break;
		case	SET_ALARM_REC_HI_T:
			value = get_floatvalue(1);
			set_alarm_rht(value);
			break;		
		case SET_ALARM_DEFAULT:
			set_alarm_default();
			break;

		case	SET_AD_INPUT_V:
				value = get_floatvalue(1);
				set_ad_iv(value);
				break;
		case	SET_AD_INPUT_A:
			value = get_floatvalue(1);
			set_ad_ia(value);
			break;
		case	SET_AD_OUTPUT_V:
			value = get_floatvalue(1);
			set_ad_ov(value);
			break;		
		case	SET_AD_OUTPUT_A:
			value = get_floatvalue(1);
			set_ad_oa(value);
			break;		
		case	SET_AD_BAT_A:
			value = get_floatvalue(1);
			set_ad_ba(value);
			break;		
		case	SET_AD_OUPUT_ZERO_V:
			set_ad_zero();
			break;		
		case	SET_AD_BAT_ZERO_V:
			//set_ad_zero_battery();
			break;
		case	SET_AD_RESET:
			set_ad_reset();
			break;		
		case	SET_SYSTEM_TIME :
			value1 = get_intvalue(1);
			set_system_time(value1);
			break;			
		case	SET_SYSTEM_DATE:
			value1 = get_intvalue(1);
			set_system_date(value1);
			break;
		case	SET_T_COMP_ON:
			set_output_mode(1);
			break;
		case	SET_T_COMP_OFF:
			set_output_mode(0);
			break;
		case	SET_T_COMP_OUTPUT_V:
			value = get_floatvalue(1);
			set_output_voltage(value);
			break;
#if 0
		case	SET_BAT_CHARGE_ON:
			set_current_limit_mode(1);
			break;
		case	SET_BAT_CHARGE_OFF:			
			set_current_limit_mode(0);
			break;		
		case	SET_BAT_CHARGE_A:
			value = get_floatvalue(1);
			set_current_limit(value);
			break;
#endif
		case	SET_RELAY_NORMAL_OPEN:
			//set_relay_contact(NORMAL_OPEN);
			break;
		case	SET_RELEAY_NORMAL_CLOSE:
			//set_relay_contact(NORMAL_CLOSE);
			break;
		case	SET_BQL:
			value1 = get_intvalue(1);
			set_alarm_clear(0);
			break;

		case	SET_BAT_EQUIP:
			value1 = get_intvalue(1);
			set_bat_equip(value1);
			break;

		case GET_PSU_NAME:
			send_psu_name();
			break;
		case SET_PSU_NAME:
			str = get_str(1);
			set_psu_name(str);
			break;
		case	SET_BAT_TEST_ON:
			set_bat_test_on();
			break;
		case	SET_BAT_TEST_OFF:
			set_bat_test_off();
			break;
		case	SET_BAT_TEST_PERIOD:
			value1 = get_intvalue(1);
			set_bat_test_period(value1);
			break;
		case	SET_BAT_TEST_V:
			value = get_floatvalue(1);
			set_bat_test_v(value);
			break;
		case	SET_BAT_TEST_DAY:
			value1 = get_intvalue(1);
			set_bat_test_day(value1);
			break;
		case	SET_BAT_TEST_TIME:
			value1 = get_intvalue(1);
			set_bat_test_time(value1);
			break;
		case	SET_BAT_TEST_REQ:
			value1 = get_intvalue(1);
			set_bat_test_req(value1);
			break;
		case	SET_FLASH_IP:
			value1 = get_intvalue(1);
			set_flash_ip(value1);
			break;
		case	CMD_PING:
			value1 = get_intvalue(1);
			ping_to(value1);
			break;


		default:
			//printf("Invalid commnad: %d \n", cmd);
			diagnosis(cmd);
			break;
	}
}

void	send_system_info()
{
	//u8 mac[6];
	
	get_system_time(&sys_info);

	sys_frame.command = REPORT_SYSTEM_INFO;
	sys_frame.mode	= 2;		// report system time & version
	sys_frame.year = (byte ) ( ( sys_info.year - 2000) & 0xff);
	sys_frame.month = (byte) sys_info.month & 0xff;
	sys_frame.day = (byte) sys_info.day & 0xff;
	sys_frame.hour = (byte) sys_info.hour& 0xff;
	sys_frame.min = (byte) sys_info.min & 0xff;
	sys_frame.sec = (byte) sys_info.sec& 0xff;
	memcpy(sys_frame.hw_version, sys_info.hw_version, VERSION_SIZE);
	memcpy(sys_frame.fw_version, sys_info.fw_version, VERSION_SIZE);
	memcpy(sys_frame.serial_no, sys_info.serial_no, VERSION_SIZE);

	memcpy(sys_frame.ip_addr, sys_info.ip_addr, 4);
	memcpy(sys_frame.ip_save, saved_ip, 4);	
	sys_frame.run_id = management_id;
	sys_frame.saved_id = *(u32 *) saved_management_id;
	memcpy(sys_frame.run_mac, psu_mac, 6);
	memcpy(sys_frame.save_mac, sys_info.s_mac, 6);
	
	send_frame((byte *)&sys_frame, sizeof(F_SYSTEM_INFO));
}

void	send_system_time()
{
	get_system_time(&sys_info);

	sys_frame.command = REPORT_SYSTEM_INFO;
	sys_frame.mode	= 1;		// report system time
	sys_frame.year = (byte ) ( ( sys_info.year - 2000) & 0xff);
	sys_frame.month = (byte) sys_info.month & 0xff;
	sys_frame.day = (byte) sys_info.day & 0xff;
	sys_frame.hour = (byte) sys_info.hour& 0xff;
	sys_frame.min = (byte) sys_info.min & 0xff;
	sys_frame.sec = (byte) sys_info.sec& 0xff;
	
	send_frame((byte *)&sys_frame, 8);	// size of system time
}

void	send_log_no()
{
	int no;

	if (operation_mode == FUSING_MODE ) return;
	no = get_log_size();

	log_frame.command = REPORT_LOG;
	log_frame.index = 250;	// log no frame
	log_frame.year = (byte) ( no & 0xff);
	
	send_frame((byte *)&log_frame, sizeof(F_LOG_TYPE));

}

void	send_log_record(int no)
{

	LOG_HEADER	*log;
	int	size;
	u8	index;

	if ( log_sending ) return;


#ifdef FLASH_LOG
	log_send_size = fb_log_size;
#else

	log = &log_header;

	size = log->head - log->tail;
	if ( log->head < log->tail ) {
		size = LOG_MAX_NO + size;		
	}
	if ( size > no ) size = no;
#endif

	index = 0;
	log_send_size = size;
	log_send_index = size-1;
	log_sending = 1;



	//printf("send_log_record: %d", no);
	return;	

}

#if 0
void	send_bat_test_status()
{
	op_frame.command = REPORT_BAT_TEST;
	
	op_frame.sts = 0;
	op_frame.year = op_sts.year;
	op_frame.month = op_sts.month;
	op_frame.date = op_sts.date;
	op_frame.hour = op_sts.hour;
	op_frame.min = op_sts.min;
	op_frame.sec = op_sts.sec;
	op_frame.bat_test_V = conv(op_sts.bat_test_V);
	op_sts.bat_test_mode = op_sts.bat_test_mode;
	op_frame.bat_test_period = op_sts.bat_test_period;
	
	send_frame((byte *)&op_frame, (byte) sizeof(F_OP_SETUP) );
}
#endif

void	send_op_set()
{
	op_frame.command = REPORT_OPERATION;
	op_frame.sts = get_op_mode();		// add bat_equip
	op_frame.year = 	op_sts.year;
	op_frame.month=  auto_test.month;
	op_frame.date =  auto_test.day;
	op_frame.hour =  op_sts.hour;
	op_frame.min =  op_sts.min;
	op_frame.sec =  op_sts.sec;

	op_frame.output_V = conv(op_sts.output_V);
	op_frame.charge_A=  conv(op_sts.charge_A);
	op_frame.bat_test_period =  op_sts.bat_test_period;
	op_frame.bat_test_V =  conv(op_sts.bat_test_V);
	op_frame.bat_test_mode =  op_sts.bat_test_mode;
	
	op_frame.test_time =  op_sts.bat_test_mode;
    op_frame.rate = ip_bat_test_residual();
	
	op_frame.alarm_mode = (byte) op_sts.alarm_mode & 0xff;
	op_frame.bat_sts = (byte) (rect_sts.cell_fail & 0xff);

	send_frame((byte *)&op_frame, (byte) sizeof(F_OP_SETUP) );
	//printf("RABM:Send OP Set\n");
	//printf("%s %d %f\n", __FUNCTION__, op_frame.bat_test_V, op_sts.bat_test_V);
}

void	send_ad_set()
{
	ad_set.command = REPORT_AD;
	ad_set.sts = get_alarm_status();
	if ( op_sts.bat_equip ) ad_set.sts |= ALARM_BAT_DISCONNECT ;

	#if 0
	ad_set.iv = conv(ad_offset.iv );
	ad_set.ia=  conv(ad_offset.ia);
	ad_set.ov= conv(ad_offset.ov);
	ad_set.oa = conv(ad_offset.oa);
	ad_set.ba = conv(ad_offset.ba);
	#endif
	ad_set.iv = conv(rect_sts.iv_offset );
	ad_set.ia =  conv(rect_sts.ia_offset );
	ad_set.ov = conv(rect_sts.ov_offset );
	ad_set.oa = conv(rect_sts.oa_offset );
	ad_set.ba = conv(rect_sts.ba_offset );
	
	send_frame((byte *)&ad_set, (byte) sizeof(F_AD_SETUP) );
}

void	send_alarm_set()
{
	alarm_set.command = REPORT_ALARM;
	alarm_set.sts = get_alarm_status();

	if ( op_sts.bat_equip ) alarm_set.sts |= ALARM_BAT_DISCONNECT ;

	alarm_set.ihv = conv(alarm_sts.hi_input_V  );
	alarm_set.ilv =  conv(alarm_sts.low_input_V);
	alarm_set.ohv = conv(alarm_sts.hi_output_V);
	alarm_set.olv = conv(alarm_sts.low_output_V);
	alarm_set.bfv = conv(alarm_sts.low_fail_bat);
	alarm_set.blv = conv(alarm_sts.low_bat_V);
	alarm_set.bht = conv(alarm_sts.hi_bat_T);

	alarm_set.rht = conv(alarm_sts.hi_rec_T);
	
	send_frame((byte *)&alarm_set, (byte) sizeof(F_ALARM_SETUP) );
}


void	send_rect_status()
{
	F_REC_STATUS	sts_frame;

	RECT_STATUS	*sts;
	sts = &rect_sts;

	sts_frame.command = REPORT_STATUS;
	sts_frame.sts = get_alarm_status(&alarm_sts);

	if ( op_sts.bat_equip ) sts_frame.sts |= ALARM_BAT_DISCONNECT;


	
	sts_frame.input_V  =  conv(sts->input_V);
	sts_frame.input_A  = conv(sts->input_A );
	sts_frame.output_V  = conv(sts->output_V);
	sts_frame.output_A  = conv(sts->output_A);
	sts_frame.bat_A  = conv(sts->bat_A);
	sts_frame.bat_T  = conv(sts->bat_T);
	sts_frame.rec_T  = conv(sts->rec_T);
	sts_frame.bat_V  = conv(sts->bat_V);
	sts_frame.bat_sts  = (byte) (sts->cell_fail & 0xff);

	//printf("DCV = %5.2f, %dV\n", sts->output_V, sts_frame.output_V);
	//send_frame(tframe, 5);	
	send_frame((u8 *) &sts_frame, sizeof(F_REC_STATUS));	

}

void	send_psu_name()
{
	psu_name_set.command = REPORT_PSU_NAME;
	psu_name_set.mode = 0;
	strcpy(psu_name_set.name, user_psu_name);
	
	send_frame((byte *)&psu_name_set, (byte) sizeof(F_PSU_NAME) );
}


void	send_bat_test_data()
{
	F_BAT_TEST_DATA	frame;

	BAT_TEST_DATA	*sts;

	sts = &bat_data;

	frame.command = REPORT_BAT_TEST;
	frame.mode = get_alarm_status(&alarm_sts);
	if ( op_sts.bat_equip ) frame.mode |= ALARM_BAT_DISCONNECT ;

	frame.bat_V  =  conv(sts->bat_V);
	frame.bat_A  = conv(sts->bat_A );
	frame.rate  = conv(sts->rate);
	frame.time  = sts->time;
	frame.result  = sts->result;

	send_frame((u8 *) &frame, sizeof(F_BAT_TEST_DATA));	
	//printf("Bat Frame : %d, %d, %d, %d\n", frame.bat_V, frame.bat_A, frame.rate, frame.time);

}


void	send_current_log()
{
	send_log_record(1);
}

void	set_current_limit_max()	// MAX CHG_REF
{
	set_operation_log(LOG_CL_SET, op_sts.charge_A, 0);

    op_sts.charge_A = 0;
	//DAC_WriteCh(2, 4095);
	send_op_set();
}

void	set_alarm_mode(int mode)	// CHG_REF
{
	uint8_t eth = 0, pvalue = 0;

	printf("ALARM_MODE : %d\n",  mode);
	if ( mode == 0  ) {
		GPIO_SetBits(GPIO_ETH_SEL, ETH_SEL_PIN);
	}
	else GPIO_ResetBits(GPIO_ETH_SEL, ETH_SEL_PIN);

	//send_op_set();
}


void	set_bat_equip(int nobat)	// CHG_REF
{
	uint8_t equip = 0, pvalue = 0;

	printf("BAT ARM MASK : %d\n",  nobat);
	if ( op_sts.bat_equip ) pvalue = 1;
	if ( nobat ) equip = 1;
	
	set_operation_log(LOG_BAT_MASK, (float) pvalue, (float) equip);
	//log_write(LOG_ALARM_MODE, (uint8_t) equip, 0 , 0);	// equip =1 : MASK ON

    op_sts.bat_equip = (uint16_t) equip & 0xff;

	eeprom_write(BAT_EQUIP_ADDR, equip);
	send_op_set();
}

#if 0
void	set_current_limit(float	c)	// CHG_REF
{
	set_operation_log(LOG_CL_SET, op_sts.charge_A, c);

    op_sts.charge_A = c;
	write_current_limit(c);
	write_eeprom(OP_CURRENT_VALUE_ADDR  , (byte*) &c, sizeof(float));
	send_op_set();
}
#endif


void	set_system_time(int time)
{
	int hour, min, sec;

	hour =  time>> 24 & 0xff ;
	min =  time >> 16 & 0xff ;
	sec = time >> 8 & 0xff ;

	set_time_only(hour, min, sec);
	send_system_time();
}

void	set_system_date(int date)
{
	int year, month, day;

	year	 =  (  date >> 24 & 0xff ) + 2000;
	month =  date >> 16 & 0xff ;
	day = date >> 8 & 0xff ;

	set_date_only(year, month, day);
	send_system_time();

	//printf("Set Date %d/%d/%d", year, month, day);
}


void set_log_clear()
{
	#ifdef FLASH_LOG
	fb_log_clear();
	#else
	log_format_eeprom(&log_header);
	#endif
	
	send_log_no();
}



void	send_frame(byte *frame, int size)
{
	byte length;
	byte sum = 0;

	send_byte(0xaa);
	send_byte(0xaa);
	length = (byte) size;
	send_byte(length);

	while( size-- ) {
		send_byte(*frame);
		sum += *frame++;
	}
	send_byte(sum);
	//kprintf(("Frame %d", (int) frame[0]));
}

void	send_byte(byte ch)
{
	#if 0
	USART_TxChar(ch);
	#else
	SerialPutChar(ch);
	#endif
}

void test_rx_frame()
{
	get_frame(rx_frame);
}



int get_frame(byte *buf)
{
	byte	data;
	static	word	frame_ptr = 0, frame_start = 0;
	static	byte 	frame_length, check_sum;
	
	while ( get_byte(&data)) {

		if ( !frame_ptr && !frame_start ) {
			if ( data == 0xaa ) {
				frame_length = 0;
				frame_start = 1;
	            //printf("Frame Start.. ");
				//continue;
			}
			//else printf("FE:%x ", data);
			continue;
		}

		#if 1
		if ( frame_start == 1 ) {
			if ( data != 0xAA ){
				frame_start = 2;
			}
			else {
				//printf("AA 2nd\n");				
				continue;
			}
		}
		#endif
		
	    rx_buffer[frame_ptr] = data;	
	    //printf("FD=%x@%d\n", data, frame_ptr);

		if ( frame_ptr == 0 ) {
			if ( data >= 128 ) {
				//printf("FSE= %d\n", data);
				frame_ptr = 0;
				frame_start = 0;
				continue;
			}
			frame_length = (word) ( data & 0xff);
			check_sum = 0;
			//printf("FL= %d@%d\n", data, frame_ptr);
		}
		else if ( frame_ptr > frame_length ) {
			
           	frame_ptr = 0;
			frame_start = 0;
			if ( check_sum == data ) {
				memcpy(buf, rx_buffer, frame_length+2);				
                //dump_memory(buf, frame_length+2); 
                //printf("CS OK %x %x\n", check_sum, data);
				return 1;
			}	
    		else {
    			//printf("CSE:%x %x\n", check_sum, data);
    			continue;
    		}
		}
		else check_sum += data & 0xff ;		
		frame_ptr++;
		IWDG_ReloadCounter();
		
	}
    return 0;
}



int	get_byte(byte *data)
{

	return USART_Rx(data);
}

#if 0
int	get_line(byte *line)
{

	byte	ch;
	static	int	index = 0;

	if ( get_byte(&ch) ) {
		
		if ( ch == 0x0D ) {
                    USART_line_buffer[index] = 0;
                    memcpy(line, USART_line_buffer, index+1);
                    index = 0;
                    return(1);			
		}
		if ( index < 255 && ( ch != 0x0d && ch != 0x0a)) {
		    USART_line_buffer[index++] = ch;
		}
	}

	return 0;
}
#endif

