/*
*
@file		socket.h
@brief	define function of socket API 
*
*/

#ifndef	_SOCKET_H_
#define	_SOCKET_H_

#include "Types.h"

#define TX_RX_MAX_BUF_SIZE	2048

extern uint8 TX_BUF[TX_RX_MAX_BUF_SIZE];
extern uint8 RX_BUF[TX_RX_MAX_BUF_SIZE];


extern uint8 socket(SOCKET s, uint8 protocol, uint16 port, uint8 flag); // Opens a socket(TCP or UDP or IP_RAW mode)
extern void close(SOCKET s); // Close socket
extern uint8 connect(SOCKET s, uint8 * addr, uint16 port); // Establish TCP connection (Active connection)
extern void disconnect(SOCKET s); // disconnect the connection
extern uint8 listen(SOCKET s);	// Establish TCP connection (Passive connection)
extern uint16 send(SOCKET s, const uint8 * buf, uint16 len, bool retry); // Send data (TCP)
extern uint16 recv(SOCKET s, uint8 * buf, uint16 len);	// Receive data (TCP)
extern uint16 sendto(SOCKET s, const uint8 * buf, uint16 len, uint8 * addr, uint16 port); // Send data (UDP/IP RAW)
extern uint16 recvfrom(SOCKET s, uint8 * buf, uint16 len, uint8 * addr, uint16  *port); // Receive data (UDP/IP RAW)
extern uint16 igmpsend(SOCKET s, const uint8 * buf, uint16 len);
#ifdef __MACRAW__
void macraw_open(void);
uint16 macraw_send( const uint8 * buf, uint16 len ); //Send data (MACRAW)
uint16 macraw_recv( uint8 * buf, uint16 len ); //Recv data (MACRAW)
#endif

#endif
/* _SOCKET_H_ */
