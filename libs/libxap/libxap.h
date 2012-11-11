/**************************************************************************

	libxap.h

	Header file for the xAP library

	Copyright (c) 2012 Jose Luis Galindo <support@homected.com>

	This file is part of the HOMECTED project.

	HOMECTED is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	HOMECTED is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with HOMECTED; if not, write to the Free Software	Foundation,
	Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

	Library based on OPNODE project from Daniel Berenguer (www.opnode.org)
	and HAH livebox from Brett England (www.DBzoo.com)
	Copyright (c) Brett England, 2009
	Copyright (c) Daniel Berenguer <dberenguer@usapiens.com>, 2007

	Last changes:

	24/04/12 by Jose Luis Galindo: Recode and debug support
	06/04/08 by Daniel Berenguer : web.service schema
	22/04/07 by Daniel Berenguer : SMS functions added
	15/04/07 by Daniel Berenguer : Type of endpoint added
	09/03/07 by Daniel Berenguer : first version.

***************************************************************************/

//*************************************************************************
//*************************************************************************
// 									INCLUDE SECTION
//*************************************************************************
//*************************************************************************

#ifndef _LIBXAP_H
#define _LIBXAP_H

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>

//*************************************************************************
//*************************************************************************
// 									DEFINES SECTION
//*************************************************************************
//*************************************************************************

//#define DEBUG_LIBXAP									// Set this to debug library
#define BYTE						unsigned char
#define USHORT						unsigned short int
#define FALSE						0
#define TRUE						1

// xAP default settings
#define XAP_PORT					3639				// Default UDP Port
#define XAP_HBEAT_FREQ 				60					// Default Heartbeat interval in secs
#define XAP_INFO_FREQ 				120					// Default Info messages interval in secs
#define XAP_WSERV_FREQ				300					// Default seconds between each periodic server.start message

// xAP discover port range
//#define XAP_LOWEST_PORT 			3640				// If XAP_PORT is not available try to use
//#define XAP_HIGHEST_PORT 			3740				// one from XAP_LOWEST_PORT to XAP_HIGHEST_PORT
//#define XAP_LOWEST_PORT 			49152				// If XAP_PORT is not available try to use
//#define XAP_HIGHEST_PORT 			50152				// one from XAP_LOWEST_PORT to XAP_HIGHEST_PORT

#define MAX_QUEUE_BACKLOG 			5					// Number of connections for listen function

// xAP other settings
#define XAP_MSG_SIZE				1500				// Maximum length of xAP messages
#define XAP_VER_12					12					// Version 1.2 of the xAP specification
#define XAP_VER_13					13					// Version 1.3 of the xAP specification
#define XAP_ADDRESS_LEN				128					// Maximum length of xAP addresses
#define XAP_VENDOR_LEN				9					// Maximum length of VENDOR address field (logical part)
#define XAP_DEVICE_LEN				26					// Maximum length of DEVICE address field (logical part)
#define XAP_INSTANCE_LEN			26					// Maximum length of INSTANCE address field (logical part)
#define XAP_SUBADDR_LEN				70					// Maximum length for names and locations (subaddress part)
#define XAP_KEY_LEN					36					// Maximum length of xAP keywords
#define XAP_UID_LEN					17					// Maximum length of xAP UID's (16 for V 1.3)
#define XAP_EPVALUE_LEN				92					// Maximum length for value strings

// xAP BSC 
#define XAP_BSC_OUTPUT				0					// BSC device type for output
#define XAP_BSC_INPUT				1					// BSC device type for input
#define XAP_STATE_ON				"ON"				// BSC state ON
#define XAP_STATE_OFF				"OFF"				// BSC state OFF
#define XAP_STATE_TOGGLE			"TOGGLE"			// BSC state TOGGLE

// xAP Intranet
#define XAP_WEB_FIELD_LEN			255					// Maximum length of Web service fields

// Supported xAP classes
#define XAP_HBEAT					0					// xAP heartbeat
#define BSC_COMMAND					1					// xAP BSC command
#define BSC_QUERY					2					// xAP BSC query
#define BSC_EVENT					3					// xAP BSC event
#define BSC_INFO					4					// xAP BSC info
//#define X10_REQUEST				5					// xAP X10 command
//#define X10_EVENT					6					// xAP X10 event
#define XAP_SHUTDWN					7					// xAP shutdown
//#define SMS_MESSAGE				8					// xAP SMS message
#define WEB_REQUEST					9					// Web service request
#define WEB_START					10					// Web service start
#define WEB_STOP					11					// Web service stop


//*************************************************************************
//*************************************************************************
// 									CUSTOM TYPES
//*************************************************************************
//*************************************************************************

// xAP address structure
typedef struct {							// xAP address: Vendor.Device.Instance
	char vendor[XAP_VENDOR_LEN];			// Vendor name
	char device[XAP_DEVICE_LEN];			// Device
	char instance[XAP_INSTANCE_LEN];		// Instance
} xAP_ADDRESS;

// xAP header structure
typedef struct {
	int v;									// xAP version
	int hop;								// hop-count
	char uid[XAP_UID_LEN];					// Unique identifier (UID)
	char class[40];							// Message class
	char source[XAP_ADDRESS_LEN];			// Message source
	char target[XAP_ADDRESS_LEN];			// Message target
	int interval;							// Heartbeat interval
} xAP_HEADER;

// xAP endpoint structure
typedef struct {
	int UIDsub;								// UID subaddress number
	char subaddr[XAP_SUBADDR_LEN];			// name
	char state[7];							// on, off, toggle
	char value[XAP_EPVALUE_LEN];			// Value
	BYTE type;								// Type of endpoint: 0=binary input; 1=binary output; 2=level/text input; 3=level/text output
} xAP_BSCENDP;

//*************************************************************************
//*************************************************************************
// 									GLOBAL VARIABLES
//*************************************************************************
//*************************************************************************

int lxap_rx_sockfd;							// Receiver socket
struct sockaddr_in lxap_rx_address;			// Receiver socket address and port
int lxap_tx_sockfd;							// Transmitter socket
struct sockaddr_in lxap_tx_address;			// Transmitter socket address and port
char lxap_local_IP_addr[16];				// Local IP address of the device

USHORT lxap_xap_version;					// xAP version used

USHORT lxap_callBodyTimes;					// Number of studied bodies after inspecting 
											// the message header (Number of times xapReadBscBody
											// is called after calling xapReadHead)

//*************************************************************************
//*************************************************************************
// 									GLOBAL FUNCTIONS
//*************************************************************************
//*************************************************************************

// xapcommon.c functions:
//-------------------------
USHORT xapSetup(char *ifacename, USHORT udpport);
short int xapBuildAddress(char *xapaddr, xAP_ADDRESS xaplogical, char *subaddr);
short int xapBuildUid(char *xapuid, USHORT uidbase, USHORT uidsub);
short int xapRead(char *xapMessage);
char *xapReadHead(char *xapMessage, xAP_HEADER *header);
short int xapEvalTarget(char *strTarget, char *strGiven);
short int xapSendHbeat(xAP_ADDRESS xapaddr, char *xapuid, USHORT hbeatFreq, USHORT udpport);
short int xapSendHbShutdown(xAP_ADDRESS xapaddr, char *xapuid, USHORT hbeatFreq, USHORT udpport);

// xapbsc.c functions:
//-------------------------
char *xapReadBscBody(char *pBody, xAP_HEADER header, xAP_BSCENDP *endp);
short int xapSendBSCevn(char *strSource, char *strUID, char *strValue, char *strState, char *strDisplay, BYTE flgType);
short int xapSendBSCinf(char *strSource, char *strUID, char *strValue, char *strState, char *strDisplay, BYTE flgType);
short int xapSendBSCcmd(char *strSource, char *strUID, char *strTarget, char *strValue, char *strState);
short int xapSendBSCqry(char *strSource, char *strUID, char *strTarget);
/*
// xapx10.c functions:
//-------------------------
short int xapReadX10Body(int fdSocket, char *pBody, xaphead header, char *devlist, char *command, BYTE *level);
short int xapSendX10evn(int fdSocket, char *strDevice, char *strUID, char *device, char *command, BYTE level);
short int xapSendX10req(int fdSocket, char *strDevice, char *strUID, char *strTarget, char *device, char *command, BYTE level);
*/
// xapsms.c functions:
//-------------------------
short int xapReadSMSBody(char *pBody, char *phoneNum, char *SMStext, BYTE *flgIncoming);
short int xapSendSMSreceipt(char *strSource, char *strUID, char *phoneNum, char *SMStext, BYTE flgSent, char *strError);
short int xapSendSMSinbound(char *strSource, char *strUID, char *phoneNum, char *SMStext);
short int xapSendSMSoutbound(char *strSource, char *strUID, char *strTarget, char *phoneNum, char *SMStext);

// xapintranet.c functions:
//-------------------------
short int xapReadWebBody(char *pBody, char *name, char *desc, char *pc, char *icon, char *url, BYTE *type);
short int xapSendWebService(xAP_ADDRESS xapaddr, char *strUID, char *strName, char *strDesc, char *strPC, char *strIcon, char *strURL, char* type);
short int xapSendWebRequest(char *strSource, char *strUID);

#endif
