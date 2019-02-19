#include	"W5500\w5500.h"
#include	"w5500\types.h"
#include	"w5500\wiz_config.h"
#include	"w5500\socket.h"





#define BUF_LEN 128
#define PING_REQUEST 8
#define PING_REPLY 0
#define CODE_ZERO 0

#define SOCKET_ERROR 1
#define TIMEOUT_ERROR 2
#define SUCCESS 3
#define REPLY_ERROR 4
#define PING_DEBUG


typedef struct pingmsg
{
  uint8_t  Type; 		// 0 - Ping Reply, 8 - Ping Request
  uint8_t  Code;		// Always 0
  int16_t  CheckSum;	// Check sum
  int16_t  ID;	            // Identification
  int16_t  SeqNum; 	// Sequence Number
  int8_t	 Data[BUF_LEN];// Ping Data  : 1452 = IP RAW MTU - sizeof(Type+Code+CheckSum+ID+SeqNum)
} PINGMSGR;

void start_ping();
uint8_t due_ping_result();
void due_ping();
void ping_to(int addr);


uint8_t ping_tx();
uint8_t ping_rx(uint8_t s, uint8_t *addr);
//uint8_t ping_tx(uint8_t s, uint8_t *addr, uint8_t type, uint16_t len);
uint8_t ping_tx_req(uint8_t s, uint8_t *addr);


void wait_1ms(unsigned int cnt);
uint16_t checksum(void *b, int len) ;

uint8_t ping_auto(uint8_t s, uint8_t *addr);
uint8_t ping_count(uint8_t s, uint16_t pCount, uint8_t *addr);
uint8_t ping_request(uint8_t s, uint8_t *addr);
uint8_t ping_reply(uint8_t s,  uint8_t *addr, uint16_t rlen);
uint16_t ping_checksum(uint8_t * data_buf, uint16_t len);
uint16_t htons( uint16_t  hostshort);	/* htons function converts a unsigned short from host to TCP/IP network byte order (which is big-endian).*/
