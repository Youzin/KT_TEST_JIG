
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include	"frame.h"

uint8_t	rx_frame_buffer[256];
char	rx_array[40];

// frame structure
typedef struct {
	uint8_t	sync[2];	// 0xAA, 0XAA
	uint8_t length;		// length of frame size excetp sync
	uint8_t commnd;
	uint8_t	mode;
	uint8_t	data[];
} FRAME_T;


void	send_frame(uint8_t *frame, int size)
{
	uint8_t length;
	uint8_t sum = 0;

	send_byte(0xaa);
	send_byte(0xaa);
	length = (uint8_t) size;
	send_byte(length);

	while( size-- ) {
		send_byte(*frame);
		sum += *frame++;
	}
	send_byte(sum);
	//kprintf(("Frame %d", (int) frame[0]));
}

void	send_byte(uint8_t ch)
{
	#if 0
	USART_TxChar(ch);
	#else
	SerialPutChar(ch);
	#endif
}


int get_frame(uint8_t *buf)
{
	uint8_t	data;
	static	uint16_t	frame_ptr = 0, frame_start = 0;
	static	uint8_t 	frame_length, check_sum;
	
	while ( get_byte(&data)) {
		#if 1
		if ( !frame_ptr && !frame_start ) {
			if ( data == 0xaa ) {
			frame_length = 0;
			frame_start = 1;
                      //  printf("Frame Start.. ");
			break;
			}
			else break;
		}
		
		#endif
	      rx_frame_buffer[frame_ptr] = data;	
	   
		if ( frame_ptr == 0 ) {
			frame_length = data;
			check_sum = 0;
		}
		else if ( frame_ptr > frame_length ) {
           		frame_ptr = 0;
			frame_start = 0;
			if ( check_sum == data ) {
				memcpy(buf, rx_frame_buffer, frame_length+2);				
                //dump_memory(buf, frame_length+2); 
				return 1;
			}	
            		else {
            			//printf("Check sum Error %x \n\r", check_sum);
            			return 0;
            		}
		}
		else check_sum += data & 0xff ;		
		frame_ptr++;
		//IWDG_ReloadCounter();
	}
    return 0;
}

int	get_byte(uint8_t *data)
{
	return USART_Rx(data);
}

int	get_command()
{
	int	ret = 0;
	
	if ( get_frame(rx_frame_buffer) )	{
		ret = rx_frame_buffer[1];
	}

	return ret;
}

char *get_str(int arg)
{
	char	*str;
	int	    index;
      
    index = (arg -1) * 4 + 2;
	str   =  &rx_frame_buffer[index];

	//dprintf("Get Str: %s\n", str);
	
	return str;
}

int	get_4byte(int arg)
{
	int	iv, index;
      
    index = (arg -1) * 4 + 2;
	iv   = (rx_frame_buffer[index]) & 0xff;
	iv |= ( rx_frame_buffer[index+1] << 8) & 0xff00;
	iv |= ( rx_frame_buffer[index+2] << 16) & 0xff0000;
	iv |= ( rx_frame_buffer[index+3] << 24) & 0xff000000;
	//printf("Get Int: %x\n", iv);	
	return iv;
}

int	get_intvalue(int arg)
{
	int	iv, index;
      
    index = (arg -1) * 4 + 2;
	iv   = (rx_frame_buffer[index] << 24) & 0xff000000;
	iv |= ( rx_frame_buffer[index+1] << 16) & 0xff0000;
	iv |= ( rx_frame_buffer[index+2] << 8)  & 0xff00;
	iv |=  rx_frame_buffer[index+3] & 0xff;
	//printf("Get Int: %x\n", iv);	
	return iv;
}

float	get_floatvalue(int arg)
{
	float value;
	int	iv, index;
      
    index = (arg -1) * 4 + 2;
	iv   = (rx_frame_buffer[index] << 24) & 0xff000000;
	iv |= ( rx_frame_buffer[index+1] << 16) & 0xff0000;
	iv |= ( rx_frame_buffer[index+2] << 8)  & 0xff00;
	iv |=  rx_frame_buffer[index+3] & 0xff;

	value = ((float) iv ) / 100.0;
	//dprintf("Get F: %f, i:%d \n", value, iv);
	
	return value;
}

int	get_4byte2(int arg)
{
	int	iv, index;
      
    index = (arg -1) * 4 + 2;
	iv   = (rx_frame_buffer[index]) & 0xff;
	iv |= ( rx_frame_buffer[index+1] << 8) & 0xff00;
	iv |= ( rx_frame_buffer[index+2] << 16) & 0xff0000;
	iv |= ( rx_frame_buffer[index+3] << 24) & 0xff000000;
	//printf("Get Int: %x\n", iv);	
	return iv;
}

int	get_intvalue2(int arg)
{
	int	iv, index;
      
    index = (arg -1) * 4 + 3;
	iv   = (rx_frame_buffer[index] << 24) & 0xff000000;
	iv |= ( rx_frame_buffer[index+1] << 16) & 0xff0000;
	iv |= ( rx_frame_buffer[index+2] << 8)  & 0xff00;
	iv |=  rx_frame_buffer[index+3] & 0xff;
	//printf("Get Int: %d \n", iv);	
	return iv;
}

char *get_string2(int arg)
{
	int	count = 0, index;
	char	*str;
      
    index = (arg -1) * 4 + 3;
	str = rx_array;
	*str = rx_frame_buffer[index];
	while ( *str != 0 && count++ < 20 ) {
		//printf(" %x,", *str);
		str++; 
		index++;
		*str   = rx_frame_buffer[index];		
	}
	//printf("Get string: %s \n", rx_array);	
	return rx_array;
}

uint8_t	get_mode()
{
	return rx_frame_buffer[2];
}


uint8_t *get_data(int arg, int size)
{
	int	count = 0, index;
	char	*data;

    index = (arg - 1) * 4 + 2;
	data = rx_array;

	printf("GET DATA \n");

	while (  count++ < size ) {
		*data   = rx_frame_buffer[index];	
		printf("%02X, ", *data);
		data++;
		index++;		
	}
	printf("\n");
	return rx_array;
}



