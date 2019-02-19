#ifndef __COMMNAD_H
#define __COMMNAD_H

int	get_command();
char *get_str(int arg);
int	get_4byte(int arg);
int	get_intvalue(int arg);
float	get_floatvalue(int arg);
int	get_intvalue2(int arg);
char *get_string2(int arg);
byte	get_mode();

int     get_frame(byte *buf);
int		get_byte(byte *data);
int	get_line(byte *line);
unsigned char *get_data(int arg, int size);


void	do_test_mode();




void	send_system_info();
void	send_system_time();
void	send_log_no();
void	send_log_record(int no);
void	send_op_set();
void	send_ad_set();
void	send_alarm_set();
void	send_rect_status();
void	send_psu_name();
void	send_bat_test_data();
void	send_current_log();

void	set_current_limit_max();
void	set_current_limit(float	c);
void	set_system_time(int time);
void	set_system_date(int date);
void set_log_clear();
void	set_ad_reset();
void	set_bat_equip(int nobat);




void  	send_frame(byte *frame, int size);
void  	send_byte(byte ch);
void test_rx_frame();



//void	set_output_voltage(float	ov);
//byte	get_op_mode();

void	set_date_only(int year, int month, int day);
void	set_time_only(int hour, int min, int sec);
void	set_bat_test_data(int time);
void	set_auto_test_mode(int mode);
//int		get_auto_test_mode();
//void	send_test_frame(byte *buf);

void	rabm_process_command(int cmd);

#endif

