/*
*
@file		util.c
@brief	The utility functions for AVREVB. (AVR-GCC Compiler)
*
*/

#include <string.h>

#include "types.h"
#include "util_DHCP.h"

u_short swaps(u_int i)
{
	u_short ret=0;
	ret = (i & 0xFF) << 8;
	ret |= ((i >> 8)& 0xFF);
	return ret;	
}

u_long swapl(u_long l)
{
	u_long ret=0;
	ret = (l & 0xFF) << 24;
	ret |= ((l >> 8) & 0xFF) << 16;
	ret |= ((l >> 16) & 0xFF) << 8;
	ret |= ((l >> 24) & 0xFF);
	return ret;
}

/**
@brief	This function initialize AVR timer.
*/
void init_timer(void)
{

		
}


/**
@brief	Register the timer handler
*/
void set_timer(
	u_int timer, 			/**< timer Handler Number */
	void (*handler)(void)	/**< user specific function to be called by timer interrupt */
	) 
{

}


/**
@brief	Unregister Timer Handler
*/
void kill_timer(
	u_int timer	/**< user specific function to be called by timer interrupt */
	) 
{

}


