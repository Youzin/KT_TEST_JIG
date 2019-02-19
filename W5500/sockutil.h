/*
*
@file		sockutil.h
@brief	Implementation of useful function of iinChip
*
*/

#ifndef __SOCKUTIL_H
#define __SOCKUTIL_H


#define NO_USE_SOCKUTIL_FUNC


extern char* inet_ntoa(unsigned long addr);			/* Convert 32bit Address into Dotted Decimal Format */

extern unsigned short htons( unsigned short hostshort);	/* htons function converts a unsigned short from host to TCP/IP network byte order (which is big-endian).*/

extern unsigned long htonl(unsigned long hostlong);		/* htonl function converts a unsigned long from host to TCP/IP network byte order (which is big-endian). */

extern unsigned long ntohs(unsigned short netshort);		/* ntohs function converts a unsigned short from TCP/IP network byte order to host byte order (which is little-endian on Intel processors). */

extern unsigned long ntohl(unsigned long netlong);		/* ntohl function converts a u_long from TCP/IP network order to host byte order (which is little-endian on Intel processors). */


#endif
