#include "stm32f10x.h"
#include "wiz_config.h"
#include "w5500.h"
#include "socket.h"
#include "util.h"
#include "loopback.h"
#include <stdio.h>
#include <string.h>
#include	"alarmComm.h"

#define tick_second 1

extern uint8 ch_status[MAX_SOCK_NUM];
//extern CHCONFIG_TYPE_DEF Chconfig_Type_Def;
extern uint32_t presentTime;
uint16 any_port = 1000;

void loopback_tcps(SOCKET s, uint16 port)
{
   uint16 RSR_len;
   uint16 received_len;
        uint8 * data_buf = TX_BUF;

   switch (getSn_SR(s))
   {
   case SOCK_ESTABLISHED:              /* if connection is established */
      if(ch_status[s]==1)
      {
         printf("\r\n%d : Connected",s);
         printf("\r\n Peer IP : %d.%d.%d.%d", IINCHIP_READ(Sn_DIPR0(s)),  IINCHIP_READ(Sn_DIPR1(s)), IINCHIP_READ(Sn_DIPR2(s)), IINCHIP_READ(Sn_DIPR3(s)) );
         printf("\r\n Peer Port : %d", ( (uint16)(IINCHIP_READ(Sn_DPORT0(s)))<<8) +(uint16)IINCHIP_READ( Sn_DPORT1(s)) ) ;
         ch_status[s] = 2;
      }

      if ((RSR_len = getSn_RX_RSR(s)) > 0)        /* check Rx data */
      {
         if (RSR_len > TX_RX_MAX_BUF_SIZE) RSR_len = TX_RX_MAX_BUF_SIZE;   /* if Rx data size is lager than TX_RX_MAX_BUF_SIZE */                                                                           /* the data size to read is MAX_BUF_SIZE. */
         received_len = recv(s, data_buf, RSR_len);      /* read the received data */
         send(s, data_buf, received_len, (bool)0);         /* sent the received data */
      }

      break;
   case SOCK_CLOSE_WAIT:                              /* If the client request to close */
      printf("\r\n%d : CLOSE_WAIT", s);
        if ((RSR_len = getSn_RX_RSR(s)) > 0)     /* check Rx data */
      {
                   if (RSR_len > TX_RX_MAX_BUF_SIZE) RSR_len = TX_RX_MAX_BUF_SIZE;  /* if Rx data size is lager than TX_RX_MAX_BUF_SIZE */                                                                     /* the data size to read is MAX_BUF_SIZE. */
                   received_len = recv(s, data_buf, RSR_len);     /* read the received data */
      }
      disconnect(s);
      ch_status[s] = 0;
      break;
   case SOCK_CLOSED:                                       /* if a socket is closed */
      if(!ch_status[s])
      {
                   printf("\r\n%d : Loop-Back TCP Server Started. port : %d", s, port);
                   ch_status[s] = 1;
      }
      if(socket(s,(Sn_MR_ND|Sn_MR_TCP),port,0x00) == 0)    /* reinitialize the socket */
      {
                   printf("\r\n%d : Fail to create socket.",s);
                   ch_status[s] = 0;
      }
      break;
        case SOCK_INIT:   /* if a socket is initiated */

                  listen(s);
                  printf("\r\n%x :LISTEN socket %d ",getSn_SR(s), s);
                break;
        default:
                break;
   }
}

void loopback_tcpc(SOCKET s, uint16 port)
{
        uint16 RSR_len;
        uint16 received_len;
	uint8 * data_buf = TX_BUF;

   switch (getSn_SR(s))
   {
   case SOCK_ESTABLISHED:                 /* if connection is established */
      if(ch_status[s]==1)
      {
         printf("\r\n%d : Connected",s);
         ch_status[s] = 2;
      }

      if ((RSR_len = getSn_RX_RSR(s)) > 0)         /* check Rx data */
      {
         if (RSR_len > TX_RX_MAX_BUF_SIZE) RSR_len = TX_RX_MAX_BUF_SIZE;   /* if Rx data size is lager than TX_RX_MAX_BUF_SIZE */
                                                                            /* the data size to read is MAX_BUF_SIZE. */
         received_len = recv(s, data_buf, RSR_len);         /* read the received data */
         send(s, data_buf, received_len, (bool)0);         /* sent the received data */
      }

      break;
   case SOCK_CLOSE_WAIT:                                 /* If the client request to close */
      printf("\r\n%d : CLOSE_WAIT", s);
      if ((RSR_len = getSn_RX_RSR(s)) > 0)         /* check Rx data */
      {
         if (RSR_len > TX_RX_MAX_BUF_SIZE) RSR_len = TX_RX_MAX_BUF_SIZE;   /* if Rx data size is lager than TX_RX_MAX_BUF_SIZE */
                                                                                    /* the data size to read is MAX_BUF_SIZE. */
         received_len = recv(s, data_buf, RSR_len);         /* read the received data */
      }
      disconnect(s);
      ch_status[s] = 0;
      break;
   case SOCK_CLOSED:                                               /* if a socket is closed */
      if(!ch_status[s])
      {
         printf("\r\n%d : Loop-Back TCP Client Started. port: %d", s, port);
         ch_status[s] = 1;
      }
      if(socket(s, Sn_MR_TCP, any_port++, 0x00) == 0)    /* reinitialize the socket */
      {
         printf("\a%d : Fail to create socket.",s);
         ch_status[s] = 0;
      }
      break;
        case SOCK_INIT:     /* if a socket is initiated */
			#if 0	// ksyoo 
                if(time_return() - presentTime >= (tick_second * 3)) {  /* For TCP client's connection request delay : 3 sec */
                        connect(s, Chconfig_Type_Def.destip, Chconfig_Type_Def.port); /* Try to connect to TCP server(Socket, DestIP, DestPort) */
                        presentTime = time_return();
                }
			#endif
                break;
        default:
                break;
        }
}

void loopback_udp(SOCKET s, uint16 port)
{
   uint16 RSR_len;
        uint16 received_len;
	uint8 * data_buf = TX_BUF;
   uint32 destip = 0;
   uint16 destport;

   switch (getSn_SR(s))
   {
   case SOCK_UDP:
      if ((RSR_len = getSn_RX_RSR(s)) > 0)         /* check Rx data */
      {
         if (RSR_len > TX_RX_MAX_BUF_SIZE) RSR_len = TX_RX_MAX_BUF_SIZE;   /* if Rx data size is lager than TX_RX_MAX_BUF_SIZE */

         /* the data size to read is MAX_BUF_SIZE. */
         received_len = recvfrom(s, data_buf, RSR_len, (uint8*)&destip, &destport);       /* read the received data */
         if(sendto(s, data_buf, received_len,(uint8*)&destip, destport) == 0) /* send the received data */
         {
            printf("\a\a\a%d : System Fatal Error.", s);
         }
      }
      break;
   case SOCK_CLOSED:                                               /* if a socket is closed */
      printf("\r\n%d : Loop-Back UDP Started. port :%d", s, port);
      if(socket(s,Sn_MR_UDP,port,0x00)== 0)    /* reinitialize the socket */
         printf("\a%d : Fail to create socket.",s);
      break;
   }
}


void tx_udp(SOCKET s, uint16 port, UDP_PACKET *tx)
{
	switch (getSn_SR(s))
	{
	case SOCK_UDP:

			if(sendto(s, tx->data, tx->length,(uint8*)&tx->dest_ip, tx->dest_port) == 0)	/* send the data */
			{
				printf("\n** %d : System Fatal Error.", s);
			}
			//dump_memory(tx->data, tx->length);
			//dprintf("TX size = %d %x %x \n", tx->length, tx->dest_ip, tx->dest_port);
		break;
	case SOCK_CLOSED:                                               /* if a socket is closed */
		//printf("\r\n%d : Tx UDP Started. port :%d", s, port);
		if(socket(s,Sn_MR_UDP,port,0x00)== 0)    /* reinitialize the socket */
			printf("\a%d :TUDP SERR",s);
		break;
	}
}

int rx_udp(SOCKET s, uint16 port, UDP_PACKET *rx)
{
	uint16 RSR_len;
    uint16 received_len;
	uint8 *data_buf;
	uint32 destip = 0;
	uint16 destport;

	switch (getSn_SR(s))
	{
	case SOCK_UDP:
		data_buf = rx->data;
		if ((RSR_len = getSn_RX_RSR(s)) > 0) 			/* check Rx data */
		{
			if (RSR_len > TX_RX_MAX_BUF_SIZE) RSR_len = TX_RX_MAX_BUF_SIZE;	/* if Rx data size is lager than TX_RX_MAX_BUF_SIZE */
																												/* the data size to read is MAX_BUF_SIZE. */
			received_len = recvfrom(s, data_buf, RSR_len, (uint8*)&destip, &destport);			/* read the received data */
			rx->dest_ip = destip;
			rx->dest_port = destport;
			rx->length = received_len;
#if 0
			if(sendto(s, data_buf, received_len,(uint8*)&destip, destport) == 0)	/* send the received data */
			{
				printf("\a\a\a%d : System Fatal Error.", s);
			}
#endif
			//dprintf("RX size = %d %x %x \n", rx->length, destip, destport);
			//dump_memory((byte *)rx, rx->length+8);
			return received_len;
		}
		break;
	case SOCK_CLOSED:                                               /* if a socket is closed */
		//printf("\r\n%d : Rx UDP Started. port :%d", s, port);
		if(socket(s,Sn_MR_UDP,port,0x00)== 0)    /* reinitialize the socket */
			printf("\a%d :RUDP SERR.",s);
		break;
	}

	return 0;
}
