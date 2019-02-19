#ifndef __RTIME_H

void	set_time(int year, int month, int day, int hour, int min, int sec);
void	set_date_only(int year, int month, int day);

void	set_time_only(int hour, int min, int sec);
void	get_system_time(SYSTEM_INFO *sys);
u32		get_time_counter();
void	get_time();


void RTC_Configuration(void);


#endif
