/**
  ******************************************************************************
  * @file    EEPROM_Emulation/src/eeprom.c 
  * @author  MCD Application Team
  * @version V3.1.0
  * @date    07/27/2009
  * @brief   This file provides all the EEPROM emulation firmware functions.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 
/** @addtogroup EEPROM_Emulation
  * @{
  */ 

/* Includes ------------------------------------------------------------------*/
#include 	"stm32f10x.h"
#include 	<stdio.h>
#include "RABM.h"
#include "eeprom.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Global variable used to store variable value in read sequence */


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static FLASH_Status EE_Format(void);
static uint16_t EE_FindValidPage(uint8_t Operation);
static uint16_t EE_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data);
static uint16_t EE_PageTransfer(uint16_t VirtAddress, uint16_t Data);


#if 0
typedef	struct {
	float		Vref;
	float		Boffset;
	float		Coffset;
	int		sign;
} OFFSET_TYPE;

#define	OFFSET_SIGN	0x12345678
#define	OFFSET_ERASE	0xffffffff
#define	OFFSET_MAX_INDEX	( PAGE_SIZE / sizeof(OFFSET_TYPE))

OFFSET_TYPE	offset_data = {0.0, 0.0, 0.0, OFFSET_SIGN};


typedef	struct {
	int	page;
	int	index;
} EE_OFFSET_TYPE;

EE_OFFSET_TYPE	eprom;

#endif

uint8_t	fb_buffer[100];
FLASH_POOL_TYPE	fp;

int	FB_find_page_active(FLASH_BLOCK_TYPE *fb)
{

	int i, j, index;
        uint16_t  sign;

	for ( i = 0; i < fb->page_size; i++) {
		if ( FB_read_record(i, 0, fb, (uint16_t *) fb_buffer) == 0 )  break;	
	}

	if ( i < fb->page_size )  {
		j = i;
 		j--;
 		if ( j < 0 )  {
 			j = fb->page_size -1;
 		}
		index = PAGE_SIZE / fb->record_size -1;
		if (FB_read_record(j, index, fb, (uint16_t *) fb_buffer) == 1) {
			#if 0
			FLASH_Unlock();
			FLASH_ErasePage( fb->start_addr + i * PAGE_SIZE);
			FLASH_Lock();
			#endif
			return i;
		}
		else if ( i == 0 )	return -1;	// case 1, empty log
		return j;						// case 2 
	}

	index = PAGE_SIZE / fb->record_size -1;
	for ( i = 0; i < fb->page_size; i++) {
		if ( FB_read_record(i, index, fb, (uint16_t *) fb_buffer) == 0 )  break;	
	}
	if ( i < fb->page_size )  {
		return i;						// case 3
	}
	else  return -1;

}


int FB_find_index(FLASH_BLOCK_TYPE *fb, int page)
{
	int last_index;
        uint16_t sign;


	last_index = PAGE_SIZE / fb->record_size -1;

	while ( last_index >= 0 ) {
		if ( FB_read_record(page, last_index, fb, (uint16_t *)fb_buffer) == 1 )  break;
		last_index -= 1;
	}

	//if ( last_index < 0 ) last_index = 0;

	return last_index+1;
}



void	FP_init()
{
	fp.start_page = FP_START_PAGE;
	fp.end_page = FP_END_PAGE;
	fp.page_size = FP_PAGE_SIZE;
}

void	 FB_format(FLASH_BLOCK_TYPE * fb)
{
	int i;

	FLASH_Unlock();
	printf("FB format\n");
	for ( i= 0; i < fb->page_size; i++) {
		FLASH_ErasePage(fb->start_addr + PAGE_SIZE * i );
	}

	FLASH_Lock();
	printf("FB format.. Done\n");
}

int	FB_init( FLASH_BLOCK_TYPE *fb, int page_size, int record_size,   uint16_t sign)
{
	int	ret = 1;
	
	fb->start_addr = FP_BASE_ADDR + (PAGE_SIZE * fp.start_page);
	printf("FB start addr=%x, record size=%d\n", fb->start_addr, fp.start_page);
	fp.start_page += page_size;
	fp.page_size -= page_size;
	if ( fp.page_size < 1 ) {

		printf("FB pool overflow : %d.\n", fp.page_size);
	}

	fb->page_size = page_size;
	record_size = record_size+ record_size % 2;	// make size even no
	fb->record_size = record_size + 2;
	fb->sign = sign;


	
	fb->active_page = FB_find_page_active(fb);
	if ( fb->active_page == -1) {
		FB_format(fb);
		fb->active_page = 0;
		fb->index = 0;		
		ret = 0;
	}
	else fb->index = FB_find_index(fb, fb->active_page);	

	printf("FB sign : %x, page=%d, index=%d\n", fb->sign, fb->active_page, fb->index);

	return ret;

}



void	FB_write_record(int page, int index, FLASH_BLOCK_TYPE *fb, uint16_t  *buffer)
{
	int	addr;
	int	i;

#if 0
	if ( index >= OFFSET_MAX_INDEX ) {
		printf("EE_write : INDEX > MAX error\n");
		return;
	}
#endif

	addr = fb->start_addr + PAGE_SIZE * page + index * fb->record_size;

	for ( i = 0; i < (fb->record_size -2)/2; i++) {
		FLASH_ProgramHalfWord(addr, *buffer);
		buffer++;
		addr += 2;	// sizeof(uint16_t)
	}
	FLASH_ProgramHalfWord(addr, fb->sign);
}

int	FB_read_record(int page, int index, FLASH_BLOCK_TYPE *fb, uint16_t  *buffer)
{
	uint16_t   sign;
	int	i, addr;
	void *buf;

	buf = (void *) buffer;

#if 0
	if ( index >= OFFSET_MAX_INDEX ) {
		printf("EE_read : INDEX > MAX error\n");
		return;
	}
#endif

	
	addr = fb->start_addr + PAGE_SIZE * page + index * fb->record_size;
 	printf("FB read record : ADDR = %x\n", addr);
	for ( i = 0; i < (fb->record_size-2) / sizeof(uint16_t ); i++) {
		*buffer = *(uint16_t *)addr;
		buffer++;
		addr+= sizeof(uint16_t);
	}

	dump_memory(buf,  fb->record_size);
	sign = *(uint16_t *) addr;
	 printf("FB read record : sign = %x\n", sign);
	 if ( sign == fb->sign ) return 1;
	 else return 0;
}


int	FB_read_block(FLASH_BLOCK_TYPE *fb, uint16_t *buf)
{
	return FB_read_record(fb->active_page, fb->index, fb, buf);;
}

void	FB_write_block(FLASH_BLOCK_TYPE *fb, uint16_t *buf)
{
	int ri, rp;

	ri = fb->index;
	rp = fb->active_page;
	printf("FB_write block\n");
	FLASH_Unlock();	
	FB_write_record(fb->active_page, fb->index, fb, buf);

	fb->index++;
	if ( fb->index >= PAGE_SIZE / fb->record_size ) {
		fb->active_page ++;		
		if ( fb->active_page >= fb->page_size ) fb->active_page = 0;
		fb->index = 0;
		FLASH_ErasePage(fb->start_addr + PAGE_SIZE * fb->active_page );
	}
	FLASH_Lock();

	dump_memory((byte*)buf, fb->record_size);
	
	FB_read_record(rp, ri, fb, buf);
	
}

#if 0
void	EE_init_offset()
{
	OFFSET_TYPE	record;

	EE_read_record(0, 0, &record);

	if ( record.sign == OFFSET_SIGN ) {
		EE_read_record(1, 0, &record);
		if ( record.sign ==  OFFSET_ERASE ) {
			eprom.page = 0;			
		}
		else if ( record.sign ==  OFFSET_SIGN ) {
			eprom.page = EE_valid_page_offset();		
		}

	}
	else {
		EE_read_record(1, 0, &record);
		if ( record.sign ==  OFFSET_ERASE ) {
			eprom.page = 0;			
		}
		else if ( record.sign == OFFSET_SIGN ) {
			eprom.page = 1;
		}
		else EE_format_offset();
	}

}

void	EE_format_offset()
{
	eprom.page = 0;
	eprom.index = 0;
	FLASH_ErasePage(EEPROM_START_ADDRESS 
	FLASH_ErasePage(EEPROM_START_ADDRESS + PAGE_SIZE );

	offset_data.Vref = V_REF;
	offset_data.Boffset = 0.0;
	offset_data.Coffset = 0.0;
	offset_data.sign = OFFSET_SIGN;

}

void	EE_valid_page_offset()
{
	OFFSET_TYPE	offset;

	EE_read_record(0, OFFSET_MAX_INDEX-1, &offset);
	if ( offset.sign == OFFSET_SIGN ) return 0;
	
	EE_read_record(1, OFFSET_MAX_INDEX-1, &offset);
	if ( offset.sign == OFFSET_SIGN ) return 1;
}

void	EE_read_record(int page, int index, OFFSET_TYPE *offset)
{
	word	*addr;
	int	i;

	if ( index >= OFFSET_MAX_INDEX ) {
		printf("EE_read : INDEX > MAX error\n");
		return;
	}
	
	addr = EEPROM_START_ADDRESS + PAGE_SIZE * page + index * sizeof(OFFSET_TYPE);

	for ( i = 0; i < sizeof(OFFSET_TYPE) / sizeof(word); i++) {
		*(word *) OFFSET_TYPE = * addr;
		addr++;
	}
}


void	EE_write_record(int page, int index, OFFSET_TYPE *offset)
{
	int	addr;
	int	i;
	word	*ptr;

	if ( index >= OFFSET_MAX_INDEX ) {
		printf("EE_write : INDEX > MAX error\n");
		return;
	}

	ptr = (word *) offset;
	addr = EEPROM_START_ADDRESS + PAGE_SIZE * page + index * sizeof(OFFSET_TYPE);

	for ( i = 0; i < sizeof(OFFSET_TYPE) / sizeof(word); i++) {
		FLASH_ProgramHalfWord(addr, *word);
		word++;
		addr += sizeof (word);
	}
}

void	EE_read_offset(OFFSET_TYPE *offset)
{
	EE_read_record(eprom.page, eprom.index, offset);
}

void	EE_write_offset(OFFSET_TYPE *offset)
{

	eprom.index++;
	if ( eprom.index >= OFFSET_MAX_INDEX ) {
		if ( eprom.page == 1 ) eprom.page = 0;
		else eprom.page = 1;		
		eprom.index = 0;
		FLASH_ErasePage(EEPROM_START_ADDRESS + PAGE_SIZE * eprom.page );
	}
	
	EE_write_record(eprom.page, eprom.index, offset);
}

#endif
  

