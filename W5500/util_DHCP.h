/*
*
@file		util.h
*/

#ifndef _UTIL_H_
#define _UTIL_H_

#define TCNT0_VALUE	0xD7		/**< 5ms at 8MHz */
#define TCNT2_VALUE	0x87		/**< 15ms at 8MHz */

#define MAX_TIMER0_CNT	0	// MAX Timer0 Handler Count
#define MAX_TIMER2_CNT	1	// MAX Timer2 Handler Count
#define MAX_TIMER_CNT	(MAX_TIMER0_CNT+MAX_TIMER2_CNT)	// Max Timer Handler

#define DHCP_CHECK_TIMER2	0


#define NO_USE_UTIL_FUNC



extern u_short swaps(u_int i);
extern u_long swapl(u_long l);



#endif /* _UTIL_H */

