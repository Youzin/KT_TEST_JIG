#ifndef __UTIL_H
#define __UTIL_H


#include <stdint.h>		// for int types

typedef uint8_t byte; 
typedef uint16_t word;

//typedef uint8_t u8; 
//typedef uint16_t u16; 
//typedef uint32_t u32;

#define _round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define	kprintf(a)	//printf(a)


void	dprintf(char *format, ...);
void 	Delay(uint32_t nCount);
void	sys_error(int no);

uint32_t	dec2bin(float dec);
void  	dump_memory(byte *buf, int size);
int	conv( float value);
void Delay_ms( uint16_t time_ms );
void Delay_us( uint8_t time_us );



#endif

