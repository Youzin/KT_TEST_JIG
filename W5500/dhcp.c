/**
 * @file		dhcp.c
 * @brief 		functions relative to dhcp
 */

#include <string.h>

#include "types.h"

#include "socket.h"
#include "sockutil.h"
#include "myprintf.h"

#include "w5500.h"
//#include "wiz_config.h"

#include "dhcp.h"
#include "util_dhcp.h"

extern uint8 txsize[];
extern uint8 rxsize[];
extern uint32 management_id;

extern u_long dhcp_time;
extern unsigned char dhcp_timer_flag;
extern unsigned char psu_mac[];
extern SOCKET	dhcp_socket;

extern void save_psu_ip(uint8 *ip);
extern void save_gw_ip(uint8 *ip);
extern void save_subnet_mask(uint8 *ip);



//#define DHCP_DEBUG
#define HOST_NAME	"YOOHA_PCU"		/**< Host Name */


u_char GET_SN_MASK[4];				/**< Subnet mask received from the DHCP server */
u_char GET_GW_IP[4];				/**< Gateway ip address received from the DHCP server */
u_char GET_DNS_IP[4] = "\x00\x00\x00\x00";	/**< DNS server ip address received from the DHCP server */
u_char GET_SIP[4] = {0,};				/**< Local ip address received from the DHCP server */


static u_char DHCP_SIP[4] = {0,};				/**< DNS server ip address is discovered */
static u_char DHCP_REAL_SIP[4] = {0,};			/**< For extract my DHCP server in a few DHCP servers */
static u_char OLD_SIP[4];				/**< Previous local ip address received from DHCP server */

char dhcp_state;				/**< DHCP client status */
static char retry_count;				/**< retry count */

static u_char DHCP_timeout;				/**< DHCP Timeout flag */
static un_l2cval lease_time;				/**< Leased time */
static u_long  next_dhcp_time;			/**< DHCP Timer tick count */

static u_long DHCP_XID;				
SOCKET DHCPC_SOCK;				/**< Socket for the DHCP client */
static RIP_MSG* pRIPMSG;					/**< Pointer for the DHCP message */

void (*dhcp_ip_update)(void) = 0;		/**< handler to be called when the IP address from DHCP server is updated */
void (*dhcp_ip_conflict)(void) = 0;		/**< handler to be called when the IP address from DHCP server is conflict */


char 	client_id[20];

uint32	dhcp_T1, dhcp_T2;

static void send_DHCP_DISCOVER(SOCKET s);		/* Send the discovery message to the DHCP server */
static void send_DHCP_REQUEST(SOCKET s);		/* Send the request message to the DHCP server */
static void send_DHCP_RELEASE_DECLINE(SOCKET s,char msgtype);		/**< send the release message to the DHCP server */
static char parseDHCPMSG(SOCKET s, u_int length);	/* Receive the message from DHCP server and parse it. */
static void reset_DHCP_time(void);			/* Initialize DHCP Timer */
static void DHCP_timer_handler(void);			/* DHCP Timer handler */
static char check_leasedIP(void);			/* Check the leased IP address	*/
static void check_DHCP_Timeout(void);			/* Check DHCP Timeout  */ 
static void set_DHCP_network(void);			/* Apply the leased IP address to LP-NetCAM II */
static void proc_ip_conflict(void);			/* called when the leased IP address is conflict */



extern void set_wiz_network();
/**
 * @brief		reset timeout value and retry count
 */ 
static void reset_DHCP_time(void)
{
	dhcp_time = 0;
	next_dhcp_time = dhcp_time + DHCP_WAIT_TIME;
	retry_count = 0;
}


/**
 * @brief		This function sends DHCP DISCOVER message to DHCP server.
 */
static void send_DHCP_DISCOVER(
	SOCKET s	/**< a socket number. */
	)
{
	u_char ip[4];
	u_int i=0;

	pRIPMSG = (RIP_MSG*)TX_BUF;

	*((u_long*)DHCP_SIP)=0;
	*((u_long*)DHCP_REAL_SIP)=0;
	
	memset((void*)pRIPMSG,0,sizeof(RIP_MSG));

	pRIPMSG->op = DHCP_BOOTREQUEST;
	pRIPMSG->htype = DHCP_HTYPE10MB;
	pRIPMSG->hlen = DHCP_HLENETHERNET;
	pRIPMSG->hops = DHCP_HOPS;
	pRIPMSG->xid = htonl(DHCP_XID);
	pRIPMSG->secs = htons(DHCP_SECS);
	pRIPMSG->flags = htons(DHCP_FLAGSBROADCAST);
	pRIPMSG->chaddr[0] = psu_mac[0];
	pRIPMSG->chaddr[1] = psu_mac[1];
	pRIPMSG->chaddr[2] = psu_mac[2];
	pRIPMSG->chaddr[3] = psu_mac[3];
	pRIPMSG->chaddr[4] = psu_mac[4];
	pRIPMSG->chaddr[5] = psu_mac[5];

	/* MAGIC_COOKIE */
	pRIPMSG->OPT[i++] = (char)((MAGIC_COOKIE >> 24)& 0xFF);
	pRIPMSG->OPT[i++] = (char)((MAGIC_COOKIE >> 16)& 0xFF);
	pRIPMSG->OPT[i++] = (char)((MAGIC_COOKIE >> 8)& 0xFF);
	pRIPMSG->OPT[i++] = (char)(MAGIC_COOKIE& 0xFF);

	/* Option Request Param. */
	pRIPMSG->OPT[i++] = dhcpMessageType;
	pRIPMSG->OPT[i++] = 0x01;
	pRIPMSG->OPT[i++] = DHCP_DISCOVER;

	// Client identifier
	pRIPMSG->OPT[i++] = dhcpClientIdentifier;

	get_client_id(management_id);
	int len = strlen(client_id)+ 1;
	pRIPMSG->OPT[i++] = len;
	memcpy(&pRIPMSG->OPT[i], client_id, len);
	i += len;

	
	// host name
	pRIPMSG->OPT[i++] = hostName;
	pRIPMSG->OPT[i++] = strlen(HOST_NAME); // length of hostname + 3
	strcpy((char*)&(pRIPMSG->OPT[i]),HOST_NAME);
	
	i+=strlen(HOST_NAME);

	pRIPMSG->OPT[i++] = dhcpIPaddrLeaseTime;
	pRIPMSG->OPT[i++] = 4;

	pRIPMSG->OPT[i++] = 0;
	pRIPMSG->OPT[i++] = 0;
	pRIPMSG->OPT[i++] = 0x7;
	pRIPMSG->OPT[i++] = 0x08;		// for test lease time = 1800sec  (30 min)
	
	
	pRIPMSG->OPT[i++] = dhcpParamRequest;
	pRIPMSG->OPT[i++] = 0x06;
	pRIPMSG->OPT[i++] = subnetMask;
	pRIPMSG->OPT[i++] = routersOnSubnet;
	pRIPMSG->OPT[i++] = dns;
	pRIPMSG->OPT[i++] = domainName;
	pRIPMSG->OPT[i++] = dhcpT1value;
	pRIPMSG->OPT[i++] = dhcpT2value;
	pRIPMSG->OPT[i++] = endOption;

	/* send broadcasting packet */
	ip[0] = 255;
	ip[1] = 255;
	ip[2] = 255;
	ip[3] = 255;
	if(0 == sendto(s, (u_char *)pRIPMSG, sizeof(RIP_MSG), ip, DHCP_SERVER_PORT))
	{
		//printf("DHCP : Fatal Error(0).");
		if ( dhcp_ip_conflict != 0 )
			(*dhcp_ip_conflict)();
	}
	
	//printf("sent DHCP_DISCOVER");
}


/**
 * @brief		This function sends DHCP REQUEST message to DHCP server.
 */
static void send_DHCP_REQUEST(
	SOCKET s	/**<  socket number */
	)
{
	u_char ip[4];
	u_int i = 0;

	pRIPMSG = (RIP_MSG*)TX_BUF;
	
	memset((void*)pRIPMSG,0,sizeof(RIP_MSG));

	pRIPMSG->op = DHCP_BOOTREQUEST;
	pRIPMSG->htype = DHCP_HTYPE10MB;
	pRIPMSG->hlen = DHCP_HLENETHERNET;
	pRIPMSG->hops = DHCP_HOPS;
	pRIPMSG->xid = htonl(DHCP_XID);
	pRIPMSG->secs = htons(DHCP_SECS);

	if(dhcp_state < STATE_DHCP_LEASED)
		pRIPMSG->flags = htons(DHCP_FLAGSBROADCAST);
	else
	{
		pRIPMSG->flags = 0;		// For Unicast
		pRIPMSG->ciaddr[0] = GET_SIP[0];
		pRIPMSG->ciaddr[1] = GET_SIP[1];
		pRIPMSG->ciaddr[2] = GET_SIP[2];
		pRIPMSG->ciaddr[3] = GET_SIP[3];
	}		

	pRIPMSG->chaddr[0] = psu_mac[0];
	pRIPMSG->chaddr[1] = psu_mac[1];
	pRIPMSG->chaddr[2] = psu_mac[2];
	pRIPMSG->chaddr[3] = psu_mac[3];
	pRIPMSG->chaddr[4] = psu_mac[4];
	pRIPMSG->chaddr[5] = psu_mac[5];

	/* MAGIC_COOKIE */
	pRIPMSG->OPT[i++] = (u_char)((MAGIC_COOKIE >> 24) & 0xFF);
	pRIPMSG->OPT[i++] = (u_char)((MAGIC_COOKIE >> 16) & 0xFF);
	pRIPMSG->OPT[i++] = (u_char)((MAGIC_COOKIE >> 8) & 0xFF);
	pRIPMSG->OPT[i++] = (u_char)(MAGIC_COOKIE & 0xFF);

	/* Option Request Param. */
	pRIPMSG->OPT[i++] = dhcpMessageType;
	pRIPMSG->OPT[i++] = 0x01;
	pRIPMSG->OPT[i++] = DHCP_REQUEST;

	pRIPMSG->OPT[i++] = dhcpClientIdentifier;
	get_client_id(management_id);
	int len = strlen(client_id)+1;
	pRIPMSG->OPT[i++] = len;
	memcpy(&pRIPMSG->OPT[i], client_id, len);
	i += len;	

	if(dhcp_state < STATE_DHCP_LEASED)
	{
		pRIPMSG->OPT[i++] = dhcpRequestedIPaddr;
		pRIPMSG->OPT[i++] = 0x04;
		pRIPMSG->OPT[i++] = GET_SIP[0];
		pRIPMSG->OPT[i++] = GET_SIP[1];
		pRIPMSG->OPT[i++] = GET_SIP[2];
		pRIPMSG->OPT[i++] = GET_SIP[3];
	
		pRIPMSG->OPT[i++] = dhcpServerIdentifier;
		pRIPMSG->OPT[i++] = 0x04;
		pRIPMSG->OPT[i++] = DHCP_SIP[0];
		pRIPMSG->OPT[i++] = DHCP_SIP[1];
		pRIPMSG->OPT[i++] = DHCP_SIP[2];
		pRIPMSG->OPT[i++] = DHCP_SIP[3];
	}
	
	// host name
	pRIPMSG->OPT[i++] = hostName;
	pRIPMSG->OPT[i++] = strlen(HOST_NAME); // length of hostname + 3
	strcpy((char*)&(pRIPMSG->OPT[i]),HOST_NAME);
	i+=strlen(HOST_NAME);

//
	pRIPMSG->OPT[i++] = dhcpIPaddrLeaseTime;
	pRIPMSG->OPT[i++] = 4;

	pRIPMSG->OPT[i++] = 0;
	pRIPMSG->OPT[i++] = 0;
	pRIPMSG->OPT[i++] = 0x07;
	pRIPMSG->OPT[i++] = 0x08;		// for test lease time = 30min
//
	pRIPMSG->OPT[i++] = dhcpParamRequest;
	pRIPMSG->OPT[i++] = 0x08;
	pRIPMSG->OPT[i++] = subnetMask;
	pRIPMSG->OPT[i++] = routersOnSubnet;
	pRIPMSG->OPT[i++] = dns;
	pRIPMSG->OPT[i++] = domainName;
	pRIPMSG->OPT[i++] = dhcpT1value;
	pRIPMSG->OPT[i++] = dhcpT2value;
	pRIPMSG->OPT[i++] = performRouterDiscovery;
	pRIPMSG->OPT[i++] = staticRoute;
	pRIPMSG->OPT[i++] = endOption;

	/* send broadcasting packet */
	if(dhcp_state < STATE_DHCP_LEASED)
	{
		ip[0] = 255;
		ip[1] = 255;
		ip[2] = 255;
		ip[3] = 255;
	}
	else
	{
		ip[0] = DHCP_SIP[0];
		ip[1] = DHCP_SIP[1];
		ip[2] = DHCP_SIP[2];
		ip[3] = DHCP_SIP[3];
	}

	if(0 == sendto(s, (u_char*)pRIPMSG, sizeof(RIP_MSG), ip, DHCP_SERVER_PORT))
	{
		//printf("DHCP : Fatal Error(1).");
		if ( dhcp_ip_conflict != 0 )
			(*dhcp_ip_conflict)();
	}

	//printf("sent DHCP_REQUEST");

}


/**
 * @brief		This function sends DHCP RELEASE message to DHCP server.
 */
static void send_DHCP_RELEASE_DECLINE(
	SOCKET s,		/**< socket number */
	char msgtype	/**< 0 : RELEASE, Not Zero : DECLINE */
	)
{
	u_int i =0;
	u_char ip[4];
	
	pRIPMSG = (RIP_MSG*)TX_BUF;
	memset((void*)pRIPMSG,0,sizeof(RIP_MSG));

	pRIPMSG->op = DHCP_BOOTREQUEST;
	pRIPMSG->htype = DHCP_HTYPE10MB;
	pRIPMSG->hlen = DHCP_HLENETHERNET;
	pRIPMSG->hops = DHCP_HOPS;
	pRIPMSG->xid = htonl(DHCP_XID);
	pRIPMSG->secs = htons(DHCP_SECS);
	pRIPMSG->flags = 0;	//DHCP_FLAGSBROADCAST;

	pRIPMSG->chaddr[0] = psu_mac[0];
	pRIPMSG->chaddr[1] = psu_mac[1];
	pRIPMSG->chaddr[2] = psu_mac[2];
	pRIPMSG->chaddr[3] = psu_mac[3];
	pRIPMSG->chaddr[4] = psu_mac[4];
	pRIPMSG->chaddr[5] = psu_mac[5];


	/* MAGIC_COOKIE */
	pRIPMSG->OPT[i++] = (u_char)((MAGIC_COOKIE >> 24) & 0xFF);
	pRIPMSG->OPT[i++] = (u_char)((MAGIC_COOKIE >> 16) & 0xFF);
	pRIPMSG->OPT[i++] = (u_char)((MAGIC_COOKIE >> 8) & 0xFF);
	pRIPMSG->OPT[i++] = (u_char)(MAGIC_COOKIE & 0xFF);

	/* Option Request Param. */
	pRIPMSG->OPT[i++] = dhcpMessageType;
	pRIPMSG->OPT[i++] = 0x01;
	pRIPMSG->OPT[i++] = ((!msgtype) ? DHCP_RELEASE : DHCP_DECLINE);

	pRIPMSG->OPT[i++] = dhcpClientIdentifier;
	get_client_id(management_id);
	int len = strlen(client_id) + 1;
	pRIPMSG->OPT[i++] = len;
	memcpy(&pRIPMSG->OPT[i], client_id, len);
	i += len;


	pRIPMSG->OPT[i++] = dhcpServerIdentifier;
	pRIPMSG->OPT[i++] = 0x04;
	pRIPMSG->OPT[i++] = DHCP_SIP[0];
	pRIPMSG->OPT[i++] = DHCP_SIP[1];
	pRIPMSG->OPT[i++] = DHCP_SIP[2];
	pRIPMSG->OPT[i++] = DHCP_SIP[3];

	if(msgtype)
	{
		pRIPMSG->OPT[i++] = dhcpRequestedIPaddr;
		pRIPMSG->OPT[i++] = 0x04;
		pRIPMSG->OPT[i++] = GET_SIP[0];
		pRIPMSG->OPT[i++] = GET_SIP[1];
		pRIPMSG->OPT[i++] = GET_SIP[2];
		pRIPMSG->OPT[i++] = GET_SIP[3];		
		pRIPMSG->OPT[i++] = endOption;
		//DPRINTLN("sent DHCP_DECLINE");
	}
	else
	{
		pRIPMSG->OPT[i++] = endOption;
		//DPRINTLN("sent DHCP_RELEASE");
	}
	
	if(!msgtype)
	{
		ip[0] = DHCP_SIP[0];
		ip[1] = DHCP_SIP[1];
		ip[2] = DHCP_SIP[2];
		ip[3] = DHCP_SIP[3];
	}
	else
	{
		ip[0] = 255;
		ip[1] = 255;
		ip[2] = 255;
		ip[3] = 255;
	}

	if(0 == sendto(s, (u_char *)pRIPMSG, sizeof(RIP_MSG), ip, DHCP_SERVER_PORT))
	{
		printf("DHCP : Fatal Error(2).");
		if ( dhcp_ip_conflict != 0 )
			(*dhcp_ip_conflict)();
	}	
}


/**
 * @brief		This function parses the reply message from DHCP server.
 * @return	success - return type, fail - 0
 */
static char parseDHCPMSG(
	SOCKET s, 	/**< socket number */
	u_int length	/**< a size data to receive. */
	)
{
	u_char svr_addr[6];
	u_int  svr_port;

	pRIPMSG = (RIP_MSG*)RX_BUF;

	u_int len;
	u_char * p;
	u_char * e;
	u_char type, opt_len=0, option;

	len = recvfrom(s, (u_char *)pRIPMSG, length, svr_addr, &svr_port);

	if ( len == 0 ) return 0;
#ifdef DHCP_DEBUG
	printf("DHCP_SIP:%d.%d.%d.%d",DHCP_SIP[0],DHCP_SIP[1],DHCP_SIP[2],DHCP_SIP[3]);
	printf("DHCP_RIP:%d.%d.%d.%d",DHCP_REAL_SIP[0],DHCP_REAL_SIP[1],DHCP_REAL_SIP[2],DHCP_REAL_SIP[3]);
	printf("svr_addr:%d.%d.%d.%d",svr_addr[0],svr_addr[1],svr_addr[2],svr_addr[3]);
#endif	
	
	if(pRIPMSG->op != DHCP_BOOTREPLY)
	{
		printf("DHCP:NO MSG\n");
	}
	else
	{
		if (svr_port == DHCP_SERVER_PORT)
		{
			if(memcmp(pRIPMSG->chaddr,psu_mac,6) != 0 || pRIPMSG->xid != htonl(DHCP_XID))
			{
#ifdef DHCP_DEBUG
				printf("No My DHCP Message. This message is ignored.");
				printf("\tSRC_MAC_ADDR(%02X.%02X.%02X.",psu_mac[0],psu_mac[1],psu_mac[2]);
				printf("%02X.%02X.%02X)\n",psu_mac[3],psu_mac[4],psu_mac[5]);
				printf(", pRIPMSG->chaddr(%02X.%02X.%02X.",pRIPMSG->chaddr[0],pRIPMSG->chaddr[1],pRIPMSG->chaddr[2]);
				printf("%02X.%02X.%02X)\n",pRIPMSG->chaddr[3],pRIPMSG->chaddr[4],pRIPMSG->chaddr[5]);
				printf("\tpRIPMSG->xid(%08lX), DHCP_XID(%08lX)\n",pRIPMSG->xid,htonl(DHCP_XID));
				printf("\tpRIMPMSG->yiaddr:%d.%d.%d.%d\n",pRIPMSG->yiaddr[0],pRIPMSG->yiaddr[1],pRIPMSG->yiaddr[2],pRIPMSG->yiaddr[3]);
#endif				
				return 0;
			}

			if( *((u_long*)DHCP_SIP) != 0x00000000 )
			{
				if( *((u_long*)DHCP_REAL_SIP) != *((u_long*)svr_addr) && 
					*((u_long*)DHCP_SIP) != *((u_long*)svr_addr) ) 
				{
#ifdef DHCP_DEBUG		
					dprintf("Another DHCP sever send a response message. This is ignored.");
					dprintf("\tIP:%d.%d.%d.%d\n",svr_addr[0],svr_addr[1],svr_addr[2],svr_addr[3]);
#endif				
					return 0;								
				}
			}

			
			memcpy(GET_SIP,pRIPMSG->yiaddr,4);
	
			//printf("\nDHCP RX:%d", len);
			//printf("\nyiaddr : %d.%d.%d.%d\n",GET_SIP[0],GET_SIP[1],GET_SIP[2],GET_SIP[3]);
	
			type = 0;
			p = (u_char *)(&pRIPMSG->op);
			p = p + 240;
			e = p + (len - 240);
	
			//printf("p : 0x%08X  e : 0x%08X  len : %d\n", (u_int)p, (u_int)e, len);
	
			while ( p < e ) 
			{
				option = *p++;
				switch ( option ) 
				{
				case endOption :
				 	return type;
					break;	
	       			case padOption :
					break;
				case dhcpMessageType :
					opt_len = *p++;
					type = *p;
					//printf("dhcpMessageType : %x\n", type);
	
					break;
				case subnetMask :
					opt_len =* p++;
					memcpy(GET_SN_MASK,p,4);
	
					//printf("subnetMask : ");
					//printf("%d.%d.%d.%d\n",GET_SN_MASK[0],GET_SN_MASK[1],GET_SN_MASK[2],GET_SN_MASK[3]);
	
					break;
				case routersOnSubnet :
					opt_len = *p++;
					memcpy(GET_GW_IP,p,4);

					//printf("routersOnSubnet : ");
					//printf("%d.%d.%d.%d\n",GET_GW_IP[0],GET_GW_IP[1],GET_GW_IP[2],GET_GW_IP[3]);
	
					break;
				case dns :
					opt_len = *p++;
					memcpy(GET_DNS_IP,p,4);
					break;
				case dhcpIPaddrLeaseTime :
					opt_len = *p++;
					lease_time.lVal = ntohl(*((u_long*)p));
					dhcp_T1 = lease_time.lVal / 2;
					dhcp_T2 = (int) (lease_time.lVal * 0.875);
					//printf("LeaseTime :%d\n", lease_time.lVal);
					//printf("DHCP t1 = %d, t2 = %d\n", dhcp_T1, dhcp_T2);
					break;
	
				case dhcpServerIdentifier :
					opt_len = *p++;
					//printf("DHCP_SIP : %d.%d.%d.%d\n", DHCP_SIP[0], DHCP_SIP[1], DHCP_SIP[2], DHCP_SIP[3]);
					
					if( *((u_long*)DHCP_SIP) == 0 || 
					    *((u_long*)DHCP_REAL_SIP) == *((u_long*)svr_addr) || 
					    *((u_long*)DHCP_SIP) == *((u_long*)svr_addr) )
					{
						memcpy(DHCP_SIP,p,4);
						memcpy(DHCP_REAL_SIP,svr_addr,4);	// Copy the real ip address of my DHCP server
#ifdef DHCP_DEBUG						
						printf("My dhcpServerIdentifier : ");
						printf("%d.%d.%d.%d\n", DHCP_SIP[0], DHCP_SIP[1], DHCP_SIP[2], DHCP_SIP[3]);
						printf("My DHCP server real IP address : ");
						printf("%d.%d.%d.%d\n", DHCP_REAL_SIP[0], DHCP_REAL_SIP[1], DHCP_REAL_SIP[2], DHCP_REAL_SIP[3]);
#endif						
					}
					else
					{
						//printf("Another dhcpServerIdentifier : ");
						//printf("\tMY(%d.%d.%d.%d) ", DHCP_SIP[0], DHCP_SIP[1], DHCP_SIP[2], DHCP_SIP[3]);
						//printf("Another(%d.%d.%d.%d) : ", svr_addr[0], svr_addr[1], svr_addr[2], svr_addr[3]);
					}

					break;
				case dhcpT1value:
					opt_len = *p++;
					dhcp_T1 = *( (uint32* )p);
					dhcp_T1 = htonl(dhcp_T1);
					//printf("T1=%d\n", dhcp_T1);
					break;

				case dhcpT2value:
					opt_len = *p++;
					dhcp_T2 = *( (uint32* )p);
					dhcp_T2 = htonl(dhcp_T2);
					//printf("T2=%d\n", dhcp_T2);
					break;
					break;
				default :
					opt_len = *p++;
					//printf("option : %d, opt_len : %d\n", option, opt_len);
	
					break;
				} // switch
				p+=opt_len;
			} // while
		} // if
	}
	return 0;
}


/**
 * @brief		This function checks the state of DHCP.
 */
void check_DHCP_state(
	SOCKET s	/**< socket number */
	) 
{
	u_int len;
	u_char type;
	
	type = 0;

#if 0
	unsigned long time = get_time_counter();
	static unsigned long time_p;
	if ( time_p != time && time % 5 == 0) {
		//printf("SEC = %d\n", time);
		time_p = time;
	}
#endif

	if( s < MAX_SOCK_NUM && getSn_SR(s)!=SOCK_CLOSED)
	{
		if ((len = getSn_RX_RSR(s)) > 0)
		{
			 type = parseDHCPMSG(s, len);
		}
	}
	else if(!socket(s, Sn_MR_UDP, DHCP_CLIENT_PORT, 0x00))
	{
		printf("ERR DHCP_S %d\n", s);
	}
	
		
	switch ( dhcp_state )
	{
	case STATE_DHCP_DISCOVER :
		if (type == DHCP_OFFER) 
		{
			reset_DHCP_time();
			send_DHCP_REQUEST(s);
			dhcp_state = STATE_DHCP_REQUEST;
			//printf("state : STATE_DHCP_REQUEST\n");
		}
		else check_DHCP_Timeout();
		break;

	case STATE_DHCP_REQUEST :
		if (type == DHCP_ACK) 
		{
			reset_DHCP_time();
			if (check_leasedIP()) 
			{
				set_DHCP_network();
				dhcp_state = STATE_DHCP_LEASED;
				//printf("state : STATE_DHCP_LEASED\n");
			} 
			else 
			{
				dhcp_state = STATE_DHCP_DISCOVER;
				//printf("state : STATE_DHCP_DISCOVER\n");
			}
		}
		else if (type == DHCP_NAK) 
		{
			reset_DHCP_time();
			dhcp_state = STATE_DHCP_DISCOVER;
			//printf("state : STATE_DHCP_DISCOVER\n");
		}
		else check_DHCP_Timeout();
		break;

	case STATE_DHCP_LEASED :
		if ((lease_time.lVal != 0xffffffff) && ((dhcp_T1) < dhcp_time)) 
		{
			type = 0;
			memcpy(OLD_SIP,GET_SIP,4);
			DHCP_XID++;
			send_DHCP_REQUEST(s);
			dhcp_state = STATE_DHCP_REREQUEST;
			//printf("state : STATE_DHCP_REREQUEST\n");
			reset_DHCP_time();
		}
		break;

	case STATE_DHCP_REREQUEST :
		if (type == DHCP_ACK) 
		{
			if(memcmp(OLD_SIP,GET_SIP,4)!=0)	
			{
				//printf("OLD_SIP=%s,GET_SIP=%s\n",inet_ntoa(ntohl(*((u_long*)OLD_SIP))), inet_ntoa(ntohl(*((u_long*)GET_SIP))));
				if ( dhcp_ip_update != 0 )
					(*dhcp_ip_update)();
				//printf("The IP address from the DHCP server is updated.\n");				
			}
			else
			{
				//printf("state : STATE_DHCP_LEASED : same IP\n");
			}
			reset_DHCP_time();
			dhcp_state = STATE_DHCP_LEASED;
		} 
		else if (type == DHCP_NAK) 
		{
			reset_DHCP_time();
			dhcp_state = STATE_DHCP_DISCOVER;
			//printf("state : STATE_DHCP_DISCOVER\n");
		} 
		else check_DHCP_Timeout();
		break;

	case STATE_DHCP_RELEASE :
		break;
	default :
		break;
	}
}


#define MAX_RETRY	5

/**
 * @brief		This function checks the timeout of DHCP in each state.
 */
static void check_DHCP_Timeout(void)
{
	static uint32 retry_time = 2;
	
	if (retry_count < MAX_DHCP_RETRY) 
	{
		if ( next_dhcp_time < dhcp_time) 
		{
			dhcp_time = 0;

			next_dhcp_time = retry_time + DHCP_WAIT_TIME;
			retry_count++;
			if ( retry_count > MAX_RETRY) {
				if ( dhcp_state == STATE_DHCP_DISCOVER ) {
					retry_count = MAX_RETRY;
				}
				else {
					retry_count = 0;
					retry_time = 2;
					dhcp_state = STATE_DHCP_DISCOVER;
				}
			}
			else retry_time = retry_time *2;
			
			switch ( dhcp_state ) 
			{
			case STATE_DHCP_DISCOVER :
				//printf("<<%d %d:STATE_DHCP_DISCOVER\n", retry_count, retry_time);
				send_DHCP_DISCOVER(DHCPC_SOCK);
				break;
	
			case STATE_DHCP_REQUEST :
				//printf("<<timeout : %d>> : STATE_DHCP_REQUEST\n", retry_count);
				send_DHCP_REQUEST(DHCPC_SOCK);
				break;

			case STATE_DHCP_REREQUEST :
				//printf("<<%d:%d DHCP_REREQUEST\n", retry_count, retry_time);
				send_DHCP_REQUEST(DHCPC_SOCK);
				break;	
			default :
				break;
			}
			
		}
	} 
	else 
	{
		reset_DHCP_time();
		DHCP_timeout = 1;
		
		send_DHCP_DISCOVER(DHCPC_SOCK);
		dhcp_state = STATE_DHCP_DISCOVER;
		//printf("timeout\r\nstate : STATE_DHCP_DISCOVER");
	}
}


/**
 * @brief		This function loads network info. to iinChip
 */
static void set_DHCP_network(void)
{
	iinchip_init();
	save_psu_ip(GET_SIP);
	save_gw_ip(GET_GW_IP);
	save_subnet_mask(GET_SN_MASK);
	set_wiz_network();
	start_udp_port();
}


/**
 * @brief		check if a leased IP is valid
 * @return	0 : conflict, 1 : no conflict
 */
static char check_leasedIP(void)
{

	u_int a;

	//printf("<Check the IP Conflict : ");
	// sendto is complete. that means there is a node which has a same IP.

	/* +200801 (hwkim) */
	//a = sendto(DHCPC_SOCK, (const u_char*)"CHECK_IP_CONFLICT", 17, GET_SIP, 5000); 
	a=0; // Skip checking IP Confilct ; W5100 reply itself to ARP requset with self-IP in non-switching network environment.
	/* ---------------*/
	if ( a> 0)
	{
		//printf(" Conflict>");
		send_DHCP_RELEASE_DECLINE(DHCPC_SOCK,1);
		if ( dhcp_ip_conflict != 0 )
			(*dhcp_ip_conflict)();
		return 0;
	}
	//printf(" No Conflict>\n");
	return 1;

}	

#if 0
/**
 * @brief		Get an IP from the DHCP server.
 * @return	0 : timeout, 1 : get dhcp ip
 */
u_int getIP_DHCPS() 
{
	//printf("DHCP SetIP..\n");

	send_DHCP_DISCOVER(DHCPC_SOCK);
	dhcp_state = STATE_DHCP_DISCOVER;

	reset_DHCP_time();
	dhcp_timer_flag = 1;	//set_timer();
	DHCP_timeout = 0;

	while (dhcp_state != STATE_DHCP_LEASED)
	{
		if (DHCP_timeout == 1)
		{
			dhcp_timer_flag = 0;	// kill_timer()
			return 0;
		}
		check_DHCP_state(DHCPC_SOCK);
	}
	
	return 1;
}
#endif

/**
 * @brief		DHCP timer interrupt handler(For checking dhcp lease time).
 *
 * Increase 'my_time' each one second.
 */
static void DHCP_timer_handler(void)
{
	dhcp_time++;
}


/**
 * @brief		Get an IP from the DHCP server.
 */
void init_dhcp_client(
	SOCKET s,				/**< Socket number for the DHCP client */
	void(*ip_update)(void),	/**< handler called when the leased IP address is updated */
	void(*ip_conflict)(void)	/**< handler called when the leased IP address is conflict */
	)
{
	if(!ip_update)	dhcp_ip_update = set_DHCP_network;
	else		dhcp_ip_update = ip_update;

	if(!ip_conflict) dhcp_ip_conflict = proc_ip_conflict;
	else		 dhcp_ip_conflict = ip_conflict;

	init_dhcpc_ch(s);
}


/**
 * @brief		Get an IP from the DHCP server.
 */
static void proc_ip_conflict(void)
{
	//printf(	"IP Address CONFLICT\n");

}


void	get_client_id(uint32 management_id)
{
	sprintf( client_id, "REC_00_%u", management_id);
	//printf("Client ID : %s\n", client_id);
}


/**
 * @brief		Initialize the socket for DHCP client
 */
u_int init_dhcpc_ch(SOCKET s)
{
	u_int ret;
	char gip[4] = { 0xc0, 0 , 0, 0};

	DHCP_XID = 0x1234ABCD;
	memset(GET_SIP,0,4);
	memset(GET_GW_IP,0,4);
	memset(GET_SN_MASK,0,4);
	
	iinchip_init();
	setGAR(gip);
	setSIPR(GET_SIP);

	set_system_MAC();
	setSHAR(psu_mac);

#ifdef __DEF_IINCHIP_INT__
       setIMR(0xEF);
#endif 
	sysinit(txsize, rxsize);

#ifdef DHCP_DEBUG
{
	u_int i;
	//printf("MAC : ");
	//for (i = 0; i < 5; i++) printf("0x%02X.", psu_mac[i]);
	dprintf("0x%02X\n",psu_mac[5]);
}	
#endif

	//dprintf("DHCP socket %d ",s);
	if(!socket(s, Sn_MR_UDP, DHCP_CLIENT_PORT, 0x00))
	{
		dprintf("fail..\n");
		ret = 0;
	}
	else
	{
		dprintf("ok..\n");
		ret = 1;
	}
	DHCPC_SOCK = s;
	return ret;
}

void	start_dhcp()
{
	//printf("DHCP Init..\n");

	send_DHCP_DISCOVER(DHCPC_SOCK);
	dhcp_state = STATE_DHCP_DISCOVER;

	reset_DHCP_time();
	dhcp_timer_flag = 1;	//set_timer();
	DHCP_timeout = 0;
}

