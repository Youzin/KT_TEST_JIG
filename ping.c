
#include "ping.h"

PINGMSGR PingRequest;	 // Variable for Ping Request
PINGMSGR PingReply;	     // Variable for Ping Reply
static uint16_t RandomID = 0x1234; 
static uint16_t RandomSeqNum = 0x4321;
uint8_t ping_reply_received = 0, ping_request_received = 0; 
uint8_t req=0;
uint8_t rep=0;

uint8_t ping_buf[128];
extern uint8_t TX_BUF[TX_RX_MAX_BUF_SIZE]; // TX Buffer for applications
extern uint8_t RX_BUF[TX_RX_MAX_BUF_SIZE]; // RX Buffer for applications


uint8_t ping_start = 0, ping_ok = 0;
int ping_repeat = 0, ping_reply_count = 0;
uint8_t ping_state = 0;

unsigned long long ping_wait_time;
extern unsigned long long time10ms;
extern uint8_t ping_socket, ping_addr[4];
extern uint8_t du_ip[];


#define PING_IDLE	0
#define PING_WAIT_REPLY	1
#define PING_REPLYED	2
#define PING_WAIT_NEXT	3
#define PING_SUCCESS	4
#define PING_START		5
#define PING_TX_REQ		6
#define PING_END		7


#define PING_REPLY_TIME	300	// 3sec
#define PING_NEXT_TIME	100	// 1sec

#define PING_MAX_REPEAT	(30000/PING_REPLY_TIME)


void start_ping()
{
	ping_start = 1;
	ping_repeat = 20;
}

uint8_t due_ping_result()
{
	if ( ping_ok == 1 ) return 1;
	else return 0;
}

void due_ping()
{
	int ip;
	ip = set_int(*(int *) du_ip);
	ping_to( ip);
}

void ping_to(int addr)
{

	ping_addr[3] =  addr  & 0xff;;
	ping_addr[2] = ( addr >> 8 ) & 0xff;;
	ping_addr[1] = ( addr >> 16 ) & 0xff;
	ping_addr[0] = ( addr >> 24 ) & 0xff;

	printf("PING ADDR = %x \n", addr );	


	ping_start = 1;
	ping_repeat = 2;
	ping_ok = 0;
}


uint8_t ping_tx()
{

	switch(ping_state) {

		case PING_IDLE:
			if ( ping_start != 0  ) {
				ping_state = PING_START;
			}
			break;
		case PING_TX_REQ:
			ping_tx_req(ping_socket, ping_addr);
			ping_state = PING_WAIT_REPLY;
			ping_wait_time = time10ms + PING_REPLY_TIME;
			ping_repeat--;
			break;
		case PING_WAIT_REPLY:
			if ( ping_reply_received == 1) {
				ping_reply_received = 0;
				ping_reply_count++;
				if ( ping_reply_count >= 1  ) {
					ping_ok = 1;
					ping_state = PING_END;
					printf("PING OK\n");
				}
				else {
					ping_wait_time = time10ms + PING_NEXT_TIME;
					ping_state = PING_WAIT_NEXT;
				}
			}
			else if ( ping_wait_time < time10ms ) {
				if ( ping_repeat < 1 ) {
					ping_state = PING_END;
					ping_ok = 2;
					printf("PING FAIL\n");
				}
				else ping_state = PING_TX_REQ;
			}
			break;
		case PING_WAIT_NEXT:
			if ( ping_repeat < 1 ) {
					ping_state = PING_END;
					ping_ok = 2;
			}
			else {
				if ( ping_wait_time < time10ms ) {
					ping_state = PING_TX_REQ;
				}
			}
			break;
		case PING_START:
			ping_reply_count = 0;
			if ( ping_start != 0 && ping_repeat > 0 ) {
				ping_state = PING_TX_REQ;
				ping_ok = 0;
				ping_reply_received = 0;
			}
			else ping_state = PING_IDLE;
			break;
		case PING_END:
			ping_start = 0;
			ping_repeat = 0;
			ping_state = PING_IDLE;
			break;
		default:
			printf("PING STATE ERR!\n");
			break;
	}

	return ping_ok;
}

uint8_t ping_tx_req(uint8_t s, uint8_t *addr)
{
	int32_t len = 0;
	uint8_t cnt=0;


	switch(getSn_SR(s))
	{
		case SOCK_CLOSED:
			close(s);
			IINCHIP_WRITE(Sn_PROTO(s), IPPROTO_ICMP);              // set ICMP Protocol
			if(socket(s,Sn_MR_IPRAW,3000,0x02)!=0){       // open the SOCKET with IPRAW mode, if fail then Error
				//printf( "Ping Socket %d fail\n",   (0)) ;
			}
			/* Check socket register */
			while(getSn_SR(s)!=SOCK_IPRAW);
			break;
		case SOCK_IPRAW:
			ping_request(s, addr);
			break;
		default:
			break;

	}

}


void ping_init(uint8_t s)
{
	switch(getSn_SR(s))
	{
		case SOCK_CLOSED:
			close(s);  				                                                  // close the SOCKET
			/* Create Socket */  
			IINCHIP_WRITE(Sn_PROTO(s), IPPROTO_ICMP);              // set ICMP Protocol
			if (socket(s, Sn_MR_IPRAW, 3000, 0x02)!=s){       // open the SOCKET with IPRAW mode, if fail then Error
				//printf( "Ping init socket %d fail\n",   (s)) ;
			}	
			{
				/* Check socket register */
				while(getSn_SR(s)!=SOCK_IPRAW);
				printf("Ping Socket Open OK\n");
			}
			break;

		case SOCK_IPRAW:
			// printf("Ping Socket Already OPEN\n");			 
			break;

		default:		
			break;
	
   }	
	
}

uint8_t ping_rx(uint8_t s, uint8_t *addr)
{
	int rlen;

  	switch(getSn_SR(s))
		{
			case SOCK_CLOSED:
				close(s);  				                                                  // close the SOCKET
				/* Create Socket */  
				IINCHIP_WRITE(Sn_PROTO(s), IPPROTO_ICMP);              // set ICMP Protocol
				if (socket(s, Sn_MR_IPRAW, 3000,0x02)!=s){       // open the SOCKET with IPRAW mode, if fail then Error
					//printf( "Ping socket %d fail\n",   (s)) ;
				}	
				else {
					/* Check socket register */
					while(getSn_SR(s)!=SOCK_IPRAW);
					printf("Ping Socket Open OK\n");
				}
				break;
			case SOCK_IPRAW:
				if ( (rlen = getSn_RX_RSR(s) ) > 0){
					ping_reply(s, addr, rlen);
					rep++;
					if (ping_reply_received)  break;
				   
				}
				break;

			default:		
				break;
		
       }
}


uint8_t ping_request(uint8_t s, uint8_t *addr){
  uint16_t i;

	/* make header of the ping-request  */
	PingRequest.Type = PING_REQUEST;                   // Ping-Request
	PingRequest.Code = CODE_ZERO;	                   // Always '0'
	PingRequest.ID = htons(RandomID++);	       // set ping-request's ID to random integer value
	PingRequest.SeqNum =htons(RandomSeqNum++);// set ping-request's sequence number to ramdom integer value
	//size = 32;                                 // set Data size

	/* Fill in Data[]  as size of BIF_LEN (Default = 32)*/
  	for(i = 0 ; i < 32; i++){	                                
		PingRequest.Data[i] = (i) % 8;		  //'0'~'8' number into ping-request's data 	
	}
	 /* Do checksum of Ping Request */
	PingRequest.CheckSum = 0;		               // value of checksum before calucating checksum of ping-request packet
	PingRequest.CheckSum = checksum((uint8_t*)&PingRequest, 40);  // Calculate checksum
	
     /* sendto ping_request to destination */
	if(sendto(s,(uint8_t *)&PingRequest, 40, addr,3000)==0){  // Send Ping-Request to the specified peer.
	  	 printf( "Fail to send ping\n") ;					
	}else{
	 	  printf( "Ping to %d.%d.%d.%d\n",   (addr[0]),  (addr[1]),  (addr[2]),  (addr[3])) ;
		  //printf( " ID:%x  SeqNum:%x CheckSum:%x\r\n",   htons(PingRequest.ID),  htons(PingRequest.SeqNum),  htons(PingRequest.CheckSum)) ;
	}
	return 0;
} // ping request

uint8_t ping_reply(uint8_t s, uint8_t *addr,  uint16_t rlen){	 
	 uint16_t tmp_checksum;	
	 uint16_t len;
	 uint16_t i;
	 uint8_t *data_buf;
	 uint16_t port = 3000;
	 //PINGMSGR PingReply;

	 data_buf = RX_BUF;
	 //printf("PING RX LEN:%d\n", rlen);
		/* receive data from a destination */
	  	len = recvfrom(s, data_buf, rlen, addr,&port);
		//printf("PING LEN:%d\n", len);
		if ( len > 136 ) len = 136;
			if(data_buf[0] == PING_REPLY) {
				PingReply.Type 		 = data_buf[0];
				PingReply.Code 		 = data_buf[1];
				PingReply.CheckSum   = (data_buf[3]<<8) + data_buf[2];
				PingReply.ID 		 = (data_buf[5]<<8) + data_buf[4];
				PingReply.SeqNum 	 = (data_buf[7]<<8) + data_buf[6];


				/* check Checksum of Ping Reply */
				tmp_checksum = ~ping_checksum(data_buf,len);
				if(tmp_checksum != 0xffff)
					printf("CS ERROR = %x\n",tmp_checksum);
				else{
					/*  Output the Destination IP and the size of the Ping Reply Message*/
				    	printf("PING Reply from %d.%d.%d.%d\n",(addr[0]),  (addr[1]),  (addr[2]),  (addr[3]) );
					    //printf("PING Reply from %d.%d.%d.%d  ID:%x SeqNum:%x \n",
						//  (addr[0]),  (addr[1]),  (addr[2]),  (addr[3]),  htons(PingReply.ID),  htons(PingReply.SeqNum) );

				    	/*  SET ping_reply_receiver to '1' and go out the while_loop (waitting for ping reply)*/
					ping_reply_received =1;
				}
			}
			else if(data_buf[0] == PING_REQUEST){
				PingReply.Code 	 = 0;
				PingReply.Type 	 = PING_REPLY;
				//PingReply.CheckSum  = (data_buf[3]<<8) + data_buf[2];
				PingReply.ID 		 = (data_buf[5]<<8) + data_buf[4];
				PingReply.SeqNum 	 = (data_buf[7]<<8) + data_buf[6];

				for(i=0; i<len-8 ; i++)
				{
					PingReply.Data[i] = data_buf[8+i];
				}
				
				/* check Checksum of Ping Reply */
				//dump_memory(data_buf, len);
				data_buf[2] = data_buf[3] = 0;
				tmp_checksum = htons(checksum(data_buf, len));
				
				PingReply.CheckSum = 0;
				PingReply.CheckSum =checksum(&PingReply, len);
#if 0
				if(tmp_checksum != PingReply.CheckSum){
					printf( "CheckSum Error %x,%x \n",   (tmp_checksum),  htons(PingReply.CheckSum)) ;
				}else{
					//printf( "\r\n Checksum is correct  \r\n") ;					
				}
#endif		
				/*  Output the Destination IP and the size of the Ping Reply Message*/
			    	//printf("Request from %d.%d.%d.%d  ID:%x SeqNum:%x  :data size %d bytes\r\n",
					//  (addr[0]),  (addr[1]),  (addr[2]),  (addr[3]),  (PingReply.ID),  (PingReply.SeqNum),  (rlen+6) );
				/*  SET ping_reply_receiver to '1' and go out the while_loop (waitting for ping reply)*/		   
				ping_request_received =1;
				//ping_tx(s, addr, PING_REPLY, len);
				if(sendto(s,(uint8_t *)&PingReply, len, addr, 3000)==0){  // Send Ping-Request to the specified peer.
				  	 //printf( "Fail to send ping\n") ;					
				} else {
				 	  //printf( "Send Ping REPLY to") ;					
			          printf( "PING ECHO TO %d.%d.%d.%d\n",   (addr[0]),  (addr[1]),  (addr[2]),  (addr[3])) ;
					  // printf( " ID:%x  SeqNum:%x CheckSum:%x\r\n",   htons(PingRequest.ID),  htons(PingRequest.SeqNum),  htons(PingRequest.CheckSum)) ;
				}

			}
			else{      
 					 printf(" Unkonwn msg. \n");
			}


			return 0;
}// ping_reply

uint16_t ping_checksum(uint8_t *addr, uint16_t len)
{
	    int nleft = len;
		uint16_t *w = (uint16_t *) addr;
		int sum = 0;
		uint16_t answer = 0;
	
		/*
		 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
		 * sequential 16 bit words to it, and at the end, fold back all the
		 * carry bits from the top 16 bits into the lower 16 bits.
		 */
		while (nleft > 1)  {
			sum += *w++;
			nleft -= 2;
		}
	
		/* mop up an odd byte, if necessary */
		if (nleft == 1) {
			*(u_char *)(&answer) = *(u_char *)w ;
			sum += answer;
		}
	
		/* add back carry outs from top 16 bits to low 16 bits */
		sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
		sum += (sum >> 16); 		/* add carry */
		answer = ~sum;				/* truncate to 16 bits */

		
		return(answer);

}

uint16_t checksum(void *b, int len) 
{   
	uint16_t *buf = b; 
    uint32_t sum=0; 
    uint16_t result; 
  
    for ( sum = 0; len > 1; len -= 2 ) 
        sum += *buf++; 
    if ( len == 1 ) 
        sum += *(unsigned char*)buf; 
    sum = (sum >> 16) + (sum & 0xFFFF); 
    sum += (sum >> 16); 
    result = ~sum; 
    return result; 
} 


uint16_t _checksum(uint8_t * data_buf, uint16_t len)
{
  uint16_t sum, tsum, i, j;
  uint32_t lsum;

  j = len >> 1;
  lsum = 0;
  tsum = 0;
  for (i = 0; i < j; i++)
    {
      tsum = data_buf[i * 2];
      tsum = tsum << 8;
      tsum += data_buf[i * 2 + 1];
      lsum += tsum;
    }
   if (len % 2)
    {
      tsum = data_buf[i * 2];
      lsum += (tsum << 8);
    }
    sum = (uint16_t)lsum;
    sum = ~(sum + (lsum >> 16));
	printf("CS = %x\n", sum);
  return sum;

}

#if 0
uint16_t htons( uint16_t hostshort)
{
#if 1
  //#ifdef LITTLE_ENDIAN
	uint16_t netshort=0;
	netshort = (hostshort & 0xFF) << 8;

	netshort |= ((hostshort >> 8)& 0xFF);
	return netshort;
#else
	return hostshort;
#endif
}
#endif

/*****************************************************************************************
	Function name: wait_1us
	Input		:	cnt; Delay duration = cnt * 1u seconds
	Output	:	non
	Description
	: A delay function for waiting cnt*1u second.
*****************************************************************************************/
void wait_1us(unsigned int cnt)
{
	unsigned int i;

	for(i = 0; i<cnt; i++) {

		}
}


/*****************************************************************************************
	Function name: wait_1ms
	Input		:	cnt; Delay duration = cnt * 1m seconds
	Output	:	non
	Description
	: A delay function for waiting cnt*1m second. This function use wait_1us but the wait_1us
		has some error (not accurate). So if you want exact time delay, please use the Timer.
*****************************************************************************************/
void wait_1ms(unsigned int cnt)
{
	unsigned int i;
	for (i = 0; i < cnt; i++) wait_1us(1000);
}

/*****************************************************************************************
	Function name: wait_10ms
	Input		:	cnt; Delay duration = cnt * 10m seconds
	Output	:	non
	Description
	: A delay function for waiting cnt*10m second. This function use wait_1ms but the wait_1ms
		has some error (not accurate more than wait_1us). So if you want exact time delay,
		please use the Timer.
*****************************************************************************************/
void wait_10ms(unsigned int cnt)
{
	unsigned int i;
	for (i = 0; i < cnt; i++) wait_1ms(10);
}

