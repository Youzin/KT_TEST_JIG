#include "stm32f10x.h"
#include "types.h"
#include "w5500\w5500.h"
#include "w5500\socket.h"
#include "w5500\wiz_config.h"
#include "W5500\telnet.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "rabm.h"
void	set_management_id(u32 id);
extern uint32	management_id;


//#define	TELNET_DEBUG

//#define	FALSE 0
enum tel_cmd {
//  HELP_CMD,
  IP_SET_CMD,
  IP_RESET_CMD,
  IP_IP_CMD,
  STATUS_CMD,
  ALARM_CMD,
  REBOOT_CMD,
  EXIT_CMD,
  IP_NAME_CMD,
  MID_SET_CMD,
  MID_CMD,
};

// Command table
char *commands[] = {
 // "help",
  "ipset",
  "ipreset",
  "ip",
  "status",
  "alarm",
  "reboot",
  "exit",
  "name",
  "midset",
  "mid",

  NULL
};

// Telnet options
char *tel_options[] = {
        "Transmit Binary",
        "Echo",
	"",
	"Suppress Go Ahead",
	"",
	"Status",
	"Timing Mark"
};

//buf in send() : const uint8 * buf
char telnet_buf[LINELEN] = {0,};
uint8 remote[NOPTIONS] = {0,};

uint8 telnet_ID[] = {"root"};
uint8 telnet_PW[] = {"123qwe"};

uint8 user_name[DATA_BUF_SIZE];
uint8 user_password[DATA_BUF_SIZE];
char data_buf[2][256];

uint16 buf_index =0;

/** 0:close, 1:ready, 2:connected */
extern uint8 ch_status[MAX_SOCK_NUM];

#define MAX_TELNET_SESSIONS 2
uint8 user_state[MAX_TELNET_SESSIONS];
uint16	user_no = 0;
uint32	new_ip[MAX_TELNET_SESSIONS];
uint32	confirm_state[MAX_TELNET_SESSIONS];
uint32	mid[MAX_TELNET_SESSIONS];

#define	IDLE_CF	0
#define WAIT_CF 1
#define REBOOT_CF  2
#define	NAME_CF    3
#define MID_CF	4

uint8 sock[2];
uint8 psock[2];

char name[100];

extern char user_psu_name[];

extern uint8 dipsw_ip[], psu_ip[],saved_ip[];
extern RECT_STATUS rect_sts;
extern ALARM_STATUS alarm_sts;

void TELNETS(SOCKET s, uint16 port)
{  

 if ( s == 1 ) user_no = 0;
 else user_no =1;
  switch (sock[user_no] = getSn_SR(s))	{  /* Get the state of socket s */
    case SOCK_ESTABLISHED :   /* If the socket is established */	    
      if(ch_status[s] == ready_state) {  
       // printf("\r\nTELNET server established via SOCKET %d\r\n", s);
        init_telopt(s);     /* Initialize and negotiate the options */
        ch_status[s] = connected_state;  
      }

	  //printf("\r\nSOCKET %d : EST", s);
      if(getSn_RX_RSR(s) > 0) {             
        tel_input(s);     /* If there is any received data, process it */          
      }
      break;    
  
    case SOCK_CLOSE_WAIT :      
      //printf("\r\nSOCKET %d : CLOSE_WAIT", s);
      disconnect(s);     /* Disconnect the socket s */          
      break;      
  
    case SOCK_CLOSED :
      if(!ch_status[s]) {      
       // printf("\r\n----------------------------------- \r\n");
        //printf("\r\nTELNET server start!");
        ch_status[s] = ready_state;           
      } 
      if(socket(s,Sn_MR_TCP,port,0x00) == 0) {     /* reinitialize the socket */     
        //printf("\r\n%d : Fail to create socket.", s);
        ch_status[s] = close_state;
		
      } else {
		
        listen(s);     /* Listen sockets */
       // printf("\r\nSOCKET %d : LISTEN", s);
        user_state[user_no] = USERNAME;
		ch_status[s] = ready_state;	// ksyoo 
      }
	  confirm_state[user_no] = IDLE_CF;
	  user_state[user_no] = USERNAME;	// ksyoo
      break;       
  }     /* End switch */

#if 0
	if ( sock[user_no] != psock[user_no] ) {
		printf("\r\nSOCKET[%d] %d, status = %x ",s, sock[user_no], ch_status[s] );
		psock[user_no] = sock[user_no];
	}
#endif
  
}     /* End TELNETS function */

void init_telopt(SOCKET s)
{
  sendIAC(s, DO, TN_ENVIRONMENT);
  sendIAC(s, WILL, TN_ECHO);      /* Negotiate ECHO option */
  //sendIAC(s, WILL, 34);
}

void sendIAC(SOCKET s, uint8 r1, uint8 r2) 
{
#ifdef TELNET_DEBUG
  switch(r1) {
    case WILL :
     // printf("sent : will");      /* WILL command */
      break;
      
    case WONT :
     // printf("sent : wont");      /* WONT command */
      break;
      
    case DO :
     // printf("sent : do");      /*DO command */
      break;
      
    case DONT :
     // printf("sent : dont");      /* DONT command */
      break;  
      
    case IAC :
      break;
  }
#if 0  
  if(r2 <= NOPTIONS) {
    printf("%s\r\n", tel_options[r2]);
  } else {
    printf("%u\r\n", r2);
  }
#endif

#endif

  sprintf(telnet_buf, "%c%c%c", IAC, r1, r2);  
  send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);      /* Send IAC, command and option to the client */    
}     /* End init_telopt function */


void tel_input(SOCKET s)
{
  uint8 c;
	uint8 input_command[10];

  sprintf(input_command, "TS-%1d>", user_no);


  while(1)
  {
    if(getSn_RX_RSR(s) == 0 ) break;      /* If there is no received data, break */
    if(recv(s, &c, 1) == 0) break;      /* If there the received data is 0, break */
    if(user_state[user_no] == LOGOUT) break;     /* If the user's state is LOGOUT, break */     


	if ( c== 0x03 ) {	// ctrl-c
	        sprintf(telnet_buf, "Exit by user..\r\n");
	        send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
			close(s);
			user_state[user_no] = LOGOUT;
			break;
	}


	
    if(c != IAC) {      /* If the received data is not a control character */
		if(c == '\n' ) {
			//putchar ('N');
			buf_index = 0;
			//sprintf(telnet_buf, "%c", c);
	        //send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
			continue;
		}
		
		   
      
      if(user_state[user_no] != PASSWORD) {  
        sprintf(telnet_buf, "%c", c);
        send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		if ( c == 0x08 ) { // backspace		
			//data_buf[user_no][buf_index] = 0;
			if ( buf_index > 0 ) buf_index--;
			continue;
	  	}
      }
	  if (c == 0 ) break;
	  data_buf[user_no][buf_index] = c;      /* Save the received data to data_buf */
	  if ( buf_index >= 255 ) buf_index = 255;
	  else buf_index++;
      
      
	  if ( c == '\r' ) {	  	
		  	sprintf(telnet_buf, "%c", '\n');
	        send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
	  }
	  #ifdef TELNET_DEBUG
	  else {
	  	putchar (c);		
	  }
	  #endif

	  if ( c == '\r') { // || c == '\r' ) {     /* If receive an '\n' ASCII code */
        if(buf_index > 1) {

		  data_buf[user_no][buf_index-1] = '\0';
          
          if(user_state[user_no] != LOGIN) login(s); /* Call the login() to process login */
          else proc_command(s);     /* Process the received data */
          
          if(user_state[user_no] == LOGIN && confirm_state[user_no] == IDLE_CF) {           
            send(s, input_command, 6, FALSE);
          }
        } else if (confirm_state[user_no] == IDLE_CF ) {      
          if ( user_state[user_no] == LOGIN ) send(s, input_command, 6, FALSE); 
        }
        buf_index = 0;
      }
	  
      continue; 
    }    
      
    if(recv(s, &c, 1) == 0) break;     
    switch(c) {      /* If received an IAC character */
      case WILL :
        if(recv(s, &c, 1) == 0) break;
        willopt(s, c);      /* Call the willopt() to process WILL command */
        break;
      case WONT :
        if(recv(s, &c, 1) == 0) break;
        wontopt(s, c);      /* Call the wontopt() to process WONT command */
        break;
      case DO :
        if(recv(s, &c, 1) == 0) break;
        doopt(s, c);      /* Call the doopt() to process DO command */
        break;
      case DONT :
        if(recv(s, &c, 1) == 0) break;
        dontopt(c);     /* Call the dontopt() to process DONT command */
        break;
      case IAC :
        break;      
    }
    break;
  }
}     /* End tel_input function */

void willopt(SOCKET s, uint16 opt)
{
  int ack;
  //printf("recv: will");
  #if 0
  if(opt <= NOPTIONS) {
    printf("%s\r\n", tel_options[opt]);
  } else {
    printf("%u\r\n", opt);
  }
  #endif
  
  switch(opt) {
  case TN_TRANSMIT_BINARY :
  case TN_ECHO :
  case TN_SUPPRESS_GA :
    ack = DO;     /* If receive 'WILL' and it has TN_SUPPRESS_GA option, transmit 'DO' */
    break;
  default :
    ack = DONT;     /* Refuse other commands which not defined */	
  }
  sendIAC(s, ack, opt);
}     /* End willopt function */

void wontopt(SOCKET s, uint16 opt)
{

#ifdef TELNET_DEBUG
 // printf("recv: wont");
  if(opt <= NOPTIONS) {
    printf("%s\r\n", tel_options[opt]);
  } else {
    printf("%u\r\n", opt);
  }
#endif

  switch(opt) {
  case TN_TRANSMIT_BINARY :
  case TN_ECHO :
  case TN_SUPPRESS_GA :     /* If receive WONT command with TN_SUPPRESS_GA option */
    if(remote[opt] == 0) {
      remote[opt] = 1;      /* Set the TN_SUPPRESS_GA option */
      sendIAC(s, DONT, opt);      /* Send DONT command with TN_SUPPRESS_GA */
    }
    break;
  }
}     /* End wontopt function */

void doopt(SOCKET s, uint16 opt)
{

#ifdef TELNET_DEBUG
  //printf("recv: do ");
  if(opt <= NOPTIONS) {
    printf("%s\r\n", tel_options[opt]);
  } else {
    printf("%u\r\n", opt);
  }
#endif

  switch(opt) {
  case TN_SUPPRESS_GA :     /* If receive DO command with TN_SUPPRESS_GA option */
    sendIAC(s, WILL, opt);      /* Send WILL command with TN_SUPPRESS_GA */
    break;
  case TN_ECHO :      /* If receive DO command with TN_ECHO option */
    sprintf(telnet_buf, "WELCOME!  \r\nID : ");
    send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
    break;
  default :
    sendIAC(s, WONT, opt);
  }  
}     /* End doopt function */

void dontopt(uint16 opt)
{

#ifdef TELNET_DEBUG
 // printf("recv: dont ");
  if(opt <= NOPTIONS) {
    printf("%s\r\n", tel_options[opt]);
  } else {
    printf("%u\r\n", opt);
  }
#endif

  switch(opt) {
  case TN_TRANSMIT_BINARY :
  case TN_ECHO :
  case TN_SUPPRESS_GA :     /* If receive DONT command with TN_SUPPRESS_GA option */
    if(remote[opt] == 0) {      /* Set the TN_SUPPRESS_GA option */
      remote[opt] = 1;     
    }
    break;
  }  
}     /* End dontopt function */


static uint32_t get_mid()
{
	uint32_t mid;
	int len;
	char *str, *num;	
	//char buf[4];

	len = strlen( commands[MID_SET_CMD]);
	str = &data_buf[user_no][len];

	while ( *str == 0x20 ) {
		str++;
	}
	*(str + 99 ) = 0;
	printf("\nMID = %s ", str);
	
	num = str;
	
	while ( *num >= '0' && *num <= '9') {
		num++;	
	}
	if ( *num != 0 ) return 0xffffffff;

	strcpy(name, str);
	if ( strlen(name) < 1 ) return 0xffffffff;

	if ( strlen(name) == 1 && *name == '0' ) return 0;
	mid = atoi(name);
	printf(" %u\n", mid);
	if ( strlen(name) > 9 &&  mid == 0) {
		return 0xffffffff;
	}

	return mid;	

}

static int get_psu_name()
{
	int len;
	char *str;	

	len = strlen( commands[IP_NAME_CMD]);
	str = &data_buf[user_no][len];

	while ( *str == 0x20 ) str++;
	*(str + 99 ) = 0;
	strcpy(name, str);
	//printf("\nPSU_NAME = %s %d", name, strlen(name));
	if ( strlen(name) > 0 ) {
		return 1;
	}
	return 0;	

}
static unsigned int get_ip_addr()
{
	int len, i;

	len = strlen( commands[IP_SET_CMD]);

	char numstr[4], *str, ch;;		
	int	 index =0;
	unsigned int ip = 0;

	str = &data_buf[user_no][len];

	while ( *str == 0x20 ) str++;

	//printf("%s \n", str);

	for (i= 0; i < 3; i++) {
		for (index = 0; index < 4; index++) {
			ch = *str++;
			//printf("%c, ", ch);
	        if ( ch == '.' )  {				
				break;
	        }
			if ( ch < '0' || ch > '9' ) {
		        //printf("number only\n");
				return 0xff;
			}			
			numstr[index] = ch;
		}
		if ( index > 3 ) return 0xff;
		numstr[index] = 0;		
		ip += atoi(numstr);			
		ip <<= 8;
	}

	for (index = 0; index < 3; index++) {
		ch = *str++;
		//printf("%c, ", ch);
		if ( ch < '0' || ch > '9' ) {
	        break;
		}			
		numstr[index] = ch;
	}
	numstr[index] = 0;
			
	while ( *str  == 0x20) str++;
	if (*str != 0  && *str != 0x0a && *str != 0x0d ) {
		//printf("Bad end %x \n", *str);
		//return 0;
	}
			
	ip += atoi(numstr);
	//printf("IP addrss : 0x%x \n", ip);
	ip = set_int(ip);
	//printf("--> 0x%x \n", ip);
	return ip;		

}

#if 0

char *help = {
	   ">HELP : Show all available commands\
    \r\n>IPSET  ipaddr : SET new address\
    \r\n>IPRESET : Reset IP addr and use DIP SW IP address\
    \r\n>IPDIP : show DIP SW IP address\
    \r\n>EXIT \r\n"}; /* command HELP : Message */ 
#endif

void proc_command(SOCKET s)
{  
  char **cmdp;
  char *cp; 
  uint32	temp;
  float fno;

	
  for(cp = data_buf[user_no]; *cp != '\0';  cp++){
    *cp = tolower(*cp);     /* Translate big letter to small letter */       
  }  
    
  if(*data_buf[user_no] != '\0') {

	

	if ( confirm_state[user_no] == WAIT_CF ) {
		if ( *data_buf[user_no] == 'y') {
			sprintf(telnet_buf, "IP address changed to -> %d.%d.%d.%d\r\n", 
				new_ip[user_no] & 0xff ,(new_ip[user_no] >> 8 ) & 0xff,(new_ip[user_no]>> 16) & 0xff , (new_ip[user_no]>> 24) & 0xff );
			send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
			ip_set_ip_addr(new_ip[user_no]);			
		}
		else {
			sprintf(telnet_buf, "IPSET cancelled\r\n");
			send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		}
		confirm_state[user_no] = IDLE_CF;		
		
		return;
	}
	else if ( confirm_state[user_no] == REBOOT_CF ){
		if ( *data_buf[user_no] == 'y') {
			sprintf(telnet_buf, "System Reboot!!\r\n");
			send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
			sprintf(telnet_buf, "Logout from TELNET\r\n");
        	send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
       	 	close(s); 
        	user_state[user_no]= LOGOUT;
			
			set_system_restart(200);			
		}
		else {
			sprintf(telnet_buf, "Reboot cancelled\r\n");
			send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		}
		confirm_state[user_no] = IDLE_CF;		
		
		return;
	}
	else if ( confirm_state[user_no] == NAME_CF ){
		if ( *data_buf[user_no] == 'y') {
			sprintf(telnet_buf, "PSU NAME Changed!!\r\n");
			send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
			ip_write_psu_name(name);		
		}
		else {
			sprintf(telnet_buf, "PSU NAME Cancelled\r\n");
			send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		}
		confirm_state[user_no] = IDLE_CF;		
		
		return;
	}
	else if ( confirm_state[user_no] == MID_CF ){
		if ( *data_buf[user_no] == 'y') {
			sprintf(telnet_buf, "MID Changed!!\r\n");
			send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
			temp = (mid[user_no] >> 24 ) & 0xff;
			temp  |= (mid[user_no]>> 8 ) & 0xff00;
			temp  |= (mid[user_no]<< 8 ) & 0xff0000;
			temp  |= (mid[user_no] << 24 ) & 0xff000000;
			set_management_id(temp);		
		}
		else {
			sprintf(telnet_buf, "MID Cancelled\r\n");
			send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		}
		confirm_state[user_no] = IDLE_CF;		
		
		return;
	}
	
    /* Find the input command in table; if it isn't there, return Syntax Error */
    for(cmdp = commands; *cmdp != NULL; cmdp++) {      
      if(strncmp(*cmdp, data_buf[user_no], strlen(*cmdp)) == 0) break;      
    }
    
    if(*cmdp == NULL) {
      //printf("NULL command\r\n");
      sprintf(telnet_buf, "%s : BAD command\r\n", data_buf[user_no]);
      send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
      return;
    }
    
    switch(cmdp - commands) {
	  #if 0
      case HELP_CMD :     /* Process HELP command */
        //printf("HELP_CMD\r\n");
        sprintf(telnet_buf, help);
        send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
        break;
      #endif
	  case MID_SET_CMD :      
              
		mid[user_no] = get_mid();
		printf("MID_SET_CMD : %u\r\n", mid[user_no]);
       	if ( mid[user_no] != 0xffffffff ) {
			sprintf(telnet_buf, "Current MID: %u\r\n", get_management_id() );
			send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
			sprintf(telnet_buf, "New MID: %u\r\n", mid[user_no] );
			send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
			sprintf(telnet_buf, "Are you sure to change the MID?(y/n)");
        	send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
			confirm_state[user_no] = MID_CF;			
       	}
		else  {
			sprintf(telnet_buf, "INVALID MID!\r\n");
	        send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		}
		break;
	  case MID_CMD :      
        printf("MID_CMD\r\n");      
		sprintf(telnet_buf, "Current MID: %u\r\n", get_management_id() );
		send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		break;
	  case IP_NAME_CMD:
	  	if ( get_psu_name()) {
			sprintf(telnet_buf, "NEW PSU NAME : \r\n[%s]\r\nChange PSU NAME(y/n)?", name );
			send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
			confirm_state[user_no] = NAME_CF;
	  	}
		else {
			sprintf(telnet_buf, "PSU_NAME :\r\n[%s]\r\n", user_psu_name);
			send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		}
		break;
      case IP_SET_CMD :      
        //printf("IP_SET_CMD\r\n");      
		new_ip[user_no] = get_ip_addr();
       	if ( new_ip[user_no]!= 0xff ) {
			sprintf(telnet_buf, "SAVED IP : %d.%d.%d.%d\r\n", saved_ip[0], saved_ip[1], saved_ip[2], saved_ip[3] );
			send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
			sprintf(telnet_buf, "New SAVED IP : %d.%d.%d.%d\r\n", 
				new_ip[user_no] & 0xff ,(new_ip[user_no] >> 8 ) & 0xff,(new_ip[user_no]>> 16) & 0xff , (new_ip[user_no]>> 24) & 0xff );
			send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
			sprintf(telnet_buf, "Are you sure to change the IP address?(y/n)");
        	send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
			confirm_state[user_no] = WAIT_CF;			
       	}
		else  {
			sprintf(telnet_buf, "INVALID IP address!\r\n");
	        send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		}

        break;

	  case IP_IP_CMD :    
        //printf("IP_DIP_CMD\r\n");      
		sprintf(telnet_buf, "Running IP : %d.%d.%d.%d\r\n", psu_ip[0], psu_ip[1], psu_ip[2], psu_ip[3]  );
		send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		sprintf(telnet_buf, "SAVED   IP : %d.%d.%d.%d\r\n", saved_ip[0], saved_ip[1], saved_ip[2], saved_ip[3] ); 
		send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		sprintf(telnet_buf, "DEFAULT IP : %d.%d.%d.%d\r\n", dipsw_ip[0], dipsw_ip[1], dipsw_ip[2], dipsw_ip[3] ); 
		send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
        break;
	case IP_RESET_CMD :      
        //printf("IP_RESET_CMD\r\n");      
		new_ip[user_no] = 0;
		sprintf(telnet_buf, "SAVED IP : %d.%d.%d.%d\r\n", saved_ip[0], saved_ip[1], saved_ip[2], saved_ip[3] );
		send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		sprintf(telnet_buf, "SAVED IP address cleared : 0.0.0.0\r\n"); 
		send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		
		sprintf(telnet_buf, "Are you sure to change the saved IP address?(y/n)");
    	send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		
		confirm_state[user_no] = WAIT_CF;

        break;

	  case STATUS_CMD:
	  	if ( rect_sts.output_A < 0.1 ) fno = 0.0;
		else fno = rect_sts.output_A;
	    sprintf(telnet_buf, "ACV = %3.2fV, ACA = %2.2fA \r\n", rect_sts.input_V, rect_sts.input_A );
		send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
	  	sprintf(telnet_buf, "DCV = %2.2fV, DCA = %2.2fA \r\n", rect_sts.output_V, fno );
		send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		//sprintf(telnet_buf, "BAT = %2.2fA, BATT = %3.2fC, RecT = %3.2fC\r\n", rect_sts.bat_A, rect_sts.bat_T, rect_sts.rec_T );
		sprintf(telnet_buf, "BAT = %2.2fA, RecT = %3.2fC\r\n", rect_sts.bat_A, rect_sts.rec_T );
		send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
  		break;
	case ALARM_CMD:
		sprintf(telnet_buf, "ALARM = 0x%08x \r\n", ip_get_alarm(&alarm_sts));
		send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		break;
	case REBOOT_CMD:
		sprintf(telnet_buf, "Are you sure to REBOOT!(y/n)");
		send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
		confirm_state[user_no] = REBOOT_CF;
		break;
      case EXIT_CMD :     /* Process EXIT command */
        //printf("EXIT command\r\n");
        sprintf(telnet_buf, "Logout from TELNET\r\n");
        send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
        close(s); 
        user_state[user_no]= LOGOUT;
        break;
      
      default :
        break;
    }
  }
}     /* End proc_command function */

void login(SOCKET s)
{  
  if(user_state[user_no] == USERNAME) {       /* input the client ID and Password */
    strcpy((char *)user_name, data_buf[user_no]);    
    sprintf(telnet_buf, "Password : ");
    send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
    user_state[user_no] = PASSWORD;

	//printf("USER:%s\n", data_buf[user_no]);
    return;
  } else if(user_state[user_no] == PASSWORD) {
   // printf ("PW:%s\n", data_buf[user_no]);
    strcpy((char *)user_password, data_buf[user_no]);   
    
    /*Check the client ID and Password*/    
    if(!(strcmp((char const *)user_name, (char const *)telnet_ID)) && !(strcmp((char const *)user_password, (char const *)telnet_PW))) {
      sprintf(telnet_buf, "\r\n===================================================================== ");
      send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
      sprintf(telnet_buf, "\r\nCommands: IP, IPSET <addr>, IPRESET, ALARM, STATUS, REBOOT, NAME, EXIT");                      
      send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
      sprintf(telnet_buf, "\r\nCommands: MID, MIDSET <no>");                      
      send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
      sprintf(telnet_buf, "\r\n=====================================================================\r\n");
      send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
      user_state[user_no] = LOGIN;

      return;
    } else {
      sprintf(telnet_buf, "\r\nID or Password Error!\r\n");  /* If the ID or Password incorrect, print error msg */
      send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
      sprintf(telnet_buf, "ID : ");
      send(s, (uint8 const *)telnet_buf, strlen(telnet_buf), FALSE);
      user_state[user_no] = USERNAME;

	 
      return;      
    }
  }
}     /* End login function */