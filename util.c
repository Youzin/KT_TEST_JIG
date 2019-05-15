
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>	// for dprintf

#include "util.h"


int yooha_debug = 0;
void	dprintf(char *format, ...)
{
	if (yooha_debug == 1 ) {
		//printf("DEBUG: ");
		
		va_list argptr;		
		va_start(argptr, format);
		vfprintf(stdout, format, argptr);
		va_end(argptr);		
	}
}

int	conv( float value)
{
	return _round(value * 100);
}


void Delay(uint32_t nCount)
{
    for(; nCount != 0; nCount--);
}

void	sys_error(int no)
{
	printf("\nSYS Error : %d", no);
}


uint32_t	dec2bin(float dec)	// dec should be less tha 1
{
	float x;
	int i;
	uint32_t bin = 0;
	uint32_t	mask = 0x00800000;

	if ( dec < 0 ) bin |= mask;
	mask >>= 1;
	x = dec;
	for ( i = 0; i < 23; i++) {
		x = x * 2;
		if ( x >= 1 ) {
			x = x - 1;
			bin |= mask;
		}
		if ( x == 0) break;
		mask = mask >> 1;
	}

	return bin;
}


void  dump_memory(byte *buf, int size)
{
  byte *data;
  int   i;

  if ( yooha_debug == 0 ) return;
  
  data = buf;  
  while ( size > 16 ) {
    printf("\n\r%08x :", buf);
    for ( i = 0 ; i < 16; i++) {
      printf(" %02x", *data++);
    }
    printf("\n\r");
    size -= 16;
  }
  
  if ( size == 0 ) return;
  
  printf("\n\r%08x :", buf);
  for ( i = 0 ; i < size; i++) {
    printf(" %02x", *data++);
  }
  printf("\n\r");
  
}

void Delay_us(uint8_t time_us )
{
  register uint8_t i;
  register uint8_t j;
  
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


void Delay_ms(uint16_t time_ms )
{
  register uint16_t i;
  for( i=0;i<time_ms;i++ )
  {
    Delay_us(250);
    Delay_us(250);
    Delay_us(250);
    //Delay_us(250);
  }
}


