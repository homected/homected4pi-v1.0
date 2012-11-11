/**************************************************************************

	xapcommon.c

	Bottom level functions of the xAP interface. Common functions to any
	xAP schema

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

	17/04/12 by Jose Luis Galindo: Recode xapSetup
	07/10/07 by Daniel Berenguer : Allow extended UID's
	09/03/07 by Daniel Berenguer : first version.

***************************************************************************/

// Include section:

#include "libxap.h"

// Define section:

// Global functions section:

// Global variables section:

//*************************************************************************
// xapSetup
//
//	Initializes the UDP communication parameters, opens the UDP socket to
//	receive and send messages and heartbeats
//
//	'ifacename'		Interface name
//	'udpport'		Port number for xAP communications
//
// Returns:
//		Port opened for the receiver socket
//		0 if any error
//*************************************************************************
USHORT xapSetup(char *ifacename, USHORT udpport) {

	struct ifreq interface;						// ioctls to configure network interface
	
	struct sockaddr_in myinterface;				// Interface address
	struct sockaddr_in mynetmask;				// Interface netmask
	struct sockaddr_in mybroadcast;				// Interface broadcast address
	
	int optval, optlen;							// Vars for socket options
	USHORT rx_port;								// Store rx port, it will be different than 
												//  tx port if a hub is active
	
	// Initialize the address structures
	memset((void*)&myinterface,	0,	sizeof(myinterface));
	memset((void*)&mynetmask,	0,	sizeof(mynetmask));
	memset((void*)&mybroadcast,	0,	sizeof(mybroadcast));
	memset((void*)&lxap_rx_address,	0,	sizeof(lxap_rx_address));
	memset((void*)&lxap_tx_address,	0,	sizeof(lxap_tx_address));
	
	// Set up the xAP receiver socket
	lxap_rx_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	// Set options for the receiver socket
	optval=1;
	optlen=sizeof(int);
	if (setsockopt(lxap_rx_sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&optval, optlen)) {
		#ifdef DEBUG_LIBXAP
		printf("xapSetup: Cannot set options on receiver socket\n");
		#endif
		return 0;
	}
	
	optval=1;
	optlen=sizeof(int);
	if (setsockopt(lxap_rx_sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, optlen)) {
		#ifdef DEBUG_LIBXAP
		printf("xapSetup: Cannot set reuseaddr options on receiver socket\n");
		#endif
		return 0;
	}
	
	// Query the low-level capabilities of the network interface to get address and netmask
	memset((char*)&interface, sizeof(interface),0);
	strcpy(interface.ifr_name, ifacename);
	
	// Get the interface address
	interface.ifr_addr.sa_family = AF_INET; 
	if (ioctl(lxap_rx_sockfd, SIOCGIFADDR, &interface) != 0) {
		#ifdef DEBUG_LIBXAP
		printf("xapSetup: Could not determine IP address for interface %s\n", ifacename);
		#endif
		return 0;
	}
	myinterface.sin_addr.s_addr = ((struct sockaddr_in*)&interface.ifr_broadaddr)->sin_addr.s_addr;
	strcpy(lxap_local_IP_addr, inet_ntoa(myinterface.sin_addr));
	#ifdef DEBUG_LIBXAP
	printf("xapSetup: (%s) address %s\n", interface.ifr_name, inet_ntoa(((struct sockaddr_in*)&interface.ifr_addr)->sin_addr));
	#endif

	// Get the interface netmask
	interface.ifr_broadaddr.sa_family = AF_INET; 
	if (ioctl(lxap_rx_sockfd, SIOCGIFNETMASK, &interface) != 0) {
		#ifdef DEBUG_LIBXAP
		printf("xapSetup: Unable to determine netmask for interface %s\n", ifacename);
		#endif
		return 0;
	}
	mynetmask.sin_addr.s_addr = ((struct sockaddr_in*)&interface.ifr_broadaddr)->sin_addr.s_addr;
	#ifdef DEBUG_LIBXAP
	printf("xapSetup: (%s) netmask %s\n", interface.ifr_name, inet_ntoa(((struct sockaddr_in*)&interface.ifr_netmask)->sin_addr));
	#endif
		
	// Determine the interface broadcast address 
	long int inverted_netmask;
	inverted_netmask=~mynetmask.sin_addr.s_addr;
	mybroadcast.sin_addr.s_addr = inverted_netmask | myinterface.sin_addr.s_addr;

	// Set receiver address and port
	rx_port = udpport;										// Rx port can change if a hub is active
	lxap_rx_address.sin_family = AF_INET; 					// Same family than the socket
	lxap_rx_address.sin_addr.s_addr = htonl(INADDR_ANY);	// Any packet addressed to us
	lxap_rx_address.sin_port = htons(rx_port);				// xAP UDP port (Rx)
	memset(&(lxap_rx_address.sin_zero), '\0', 8); 			// Clean the rest of the structure
	
	// Bind the receiver socket with the address and port
	fcntl(lxap_rx_sockfd, F_SETFL, O_NONBLOCK);
	if (bind(lxap_rx_sockfd, (struct sockaddr*)&lxap_rx_address, sizeof(lxap_rx_address)) != 0) {
		// if fails then we can assume that a hub is active on this host
		#ifdef DEBUG_LIBXAP
		printf("xapSetup: Receiver socket port %d in use, assuming local hub is active\n", rx_port);
		#endif
		
		// Try to bind with another port at local host
		lxap_rx_address.sin_addr.s_addr = inet_addr("127.0.0.1");	// Set IP address to localhost
		lxap_rx_address.sin_port = 0;
		if (bind(lxap_rx_sockfd, (struct sockaddr*)&lxap_rx_address, sizeof(lxap_rx_address)) == 0) {
			socklen_t slen = sizeof(lxap_rx_address); 
			getsockname(lxap_rx_sockfd, (struct sockaddr *)&lxap_rx_address, &slen); 
			rx_port = ntohs(lxap_rx_address.sin_port); 
			#ifdef DEBUG_LIBXAP
			printf("xapSetup: Acquired receiver socket, port %d\n", rx_port);
			#endif
		}
		else {
			#ifdef DEBUG_LIBXAP
			printf("xapSetup: Cannot find any port free for receiver socket\n");
			#endif
			return 0;
		}
		
/*		
		for (rx_port = XAP_LOWEST_PORT; rx_port <= XAP_HIGHEST_PORT; rx_port++) {
			memset((void*)&lxap_rx_address,	0,	sizeof(lxap_rx_address));
			lxap_rx_address.sin_family = AF_INET; 						// Same family than the socket
			lxap_rx_address.sin_addr.s_addr = inet_addr("127.0.0.1");	// Set IP address to localhost
			lxap_rx_address.sin_port = htons(rx_port);					// xAP UDP port (Rx)
			memset(&(lxap_rx_address.sin_zero), '\0', 8); 				// Clean the rest of the structure
			if (bind(lxap_rx_sockfd, (struct sockaddr*)&lxap_rx_address, sizeof(lxap_rx_address)) == 0) {
				#ifdef DEBUG_LIBXAP
				printf("xapSetup: Acquired receiver socket, port %d\n", rx_port);
				#endif
				break;
			}
		}
		if (rx_port > XAP_HIGHEST_PORT) {
			#ifdef DEBUG_LIBXAP
			printf("xapSetup: All socket ports from %d to %d are in use\n", XAP_LOWEST_PORT, XAP_HIGHEST_PORT);
			#endif
			return 0;
		}
*/
	}
	else {
		#ifdef DEBUG_LIBXAP
		printf("xapSetup: Acquired receiver socket, port %d, assuming no local hub is active\n", rx_port);
		#endif
	}
	
	// Set the receiver socket to listen
	listen(lxap_rx_sockfd, MAX_QUEUE_BACKLOG);
	
	// Set up the broadcast socket
	lxap_tx_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (lxap_tx_sockfd == -1) {
		#ifdef DEBUG_LIBXAP
		printf("xapSetup: Unable to establish broadcast socket\n");
		#endif
		return 0;
	}
	
	// Set options for the broadcast socket
	optval = 1;
    optlen = sizeof(int);
    if (setsockopt(lxap_tx_sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&optval, optlen)) {
		#ifdef DEBUG_LIBXAP
		printf("xapSetup: Unable to set broadcast socket options\n");
		#endif
		return 0;
	}
	
	// Set broadcast address and port
	lxap_tx_address.sin_family = AF_INET;							// Same family than the socket
	lxap_tx_address.sin_port = htons(udpport);						// xAP UDP port (Tx)
	lxap_tx_address.sin_addr.s_addr = mybroadcast.sin_addr.s_addr;	// Set address to broadcast IP
	memset(&(lxap_tx_address.sin_zero), '\0', 8); 					// Clean the rest of the structure
	
	return rx_port;
}

//*************************************************************************
// xapBuildAddress
//
// This functions builds the full xAP address (logical + subaddress)
//
//	'xapaddr'		xAP full address (Vendor.Device.Instance:Subaddress)
//	'xaplogical'	xAP logical address (Vendor, Device, Instance)
//	'subaddr'		xAP Subaddress (Name, location,...)
//
//	Returns:
//		1 if the functions builds the xap address successfully
//		0 otherwise
//*************************************************************************

short int xapBuildAddress(char *xapaddr, xAP_ADDRESS xaplogical, char *subaddr) {

	if (subaddr == NULL) {
		if ((strlen(xaplogical.vendor) + strlen(xaplogical.device) + 
			strlen(xaplogical.instance)) >= XAP_ADDRESS_LEN)
		return 0;
		sprintf(xapaddr, "%s.%s.%s", xaplogical.vendor, xaplogical.device, xaplogical.instance);
	}
	else {
		if ((strlen(xaplogical.vendor) + strlen(xaplogical.device) + 
			strlen(xaplogical.instance) + strlen(subaddr)) >= XAP_ADDRESS_LEN)
		return 0;
		sprintf(xapaddr, "%s.%s.%s:%s", xaplogical.vendor, xaplogical.device, xaplogical.instance, subaddr);
	}
	return 1;
}

//*************************************************************************
// xapBuildUid
//
// This functions builds the xAP Uid for an endpoint
//
//	'xapuid'		Full UID (Network ID + base + subaddress)
//	'uidbase'		Base uid
//	'uidsub'		Subaddress
//
//	Returns:
//		1 if the functions builds the xap uid successfully
//		0 otherwise
//*************************************************************************

short int xapBuildUid(char *xapuid, USHORT uidbase, USHORT uidsub) {

	switch(lxap_xap_version) {
		case XAP_VER_12:
			if (uidsub > 0xFF) 
				return 0;
			sprintf(xapuid, "FF%04X%02X", uidbase, uidsub);
			break;
			
		case XAP_VER_13:
			if (uidsub > 0xFFFF) 
				return 0;
			sprintf(xapuid, "FF.%08X:%04X", uidbase, uidsub);
			break;
	}
	return 1;
}

//*************************************************************************
// xapRead
//
//	Reads incoming xAP messages
//
//	'xapMessage'	xAP message string to be returned
//
// Returns:
//		1 if the function executes succesfully and the message doesn't
//		come from our device
//		0 if the function executes succesfully and the message comes
//		from our device
//		-1 if any error
//*************************************************************************

short int xapRead(char *xapMessage)
{
	char strMessage[XAP_MSG_SIZE];		// Will store the incoming UDP message
	socklen_t intAddressLen;			// Size of the origin address
	struct sockaddr_in sckInAddress;	// Address structure
	int intRes = 1;

	// Clean the buffer contents
	memset(strMessage, 0, sizeof(strMessage));
	
	// Read the incoming message
	intAddressLen = sizeof(sckInAddress);
	intRes = recvfrom(lxap_rx_sockfd, strMessage, sizeof(strMessage), 0, (struct sockaddr*) &sckInAddress, &intAddressLen);
	if (intRes == -1)
		return -1;

	// Copy the received message into the target buffer
	strMessage[intRes] = '\0';	// Add NULL to the end of message
	strcpy(xapMessage, strMessage);

	// If the received message doesn't come from our device
	if(!strcmp(lxap_local_IP_addr, inet_ntoa(sckInAddress.sin_addr)) || (sckInAddress.sin_addr.s_addr == inet_addr("127.0.0.1")))
		intRes = 0;
	else
		intRes = 1;
	
	return intRes;
}

//*************************************************************************
// xapReadHead
//
//	Evaluates the header section of the given message 
//
//	'xapMessage'	xAP message string to be returned
//	'header'		Pointer to the header struct to return the results
//
// Returns:
//		Pointer to the message body if the function executes succesfully
//		and the message doesn't come from our device
//		NULL otherwise
//*************************************************************************

char * xapReadHead(char *xapMessage, xAP_HEADER *header)
{
	char *pStr1, *pStr2;				// Pointers to strings
	char strKey[XAP_KEY_LEN];			// Keyword string
	char strValue[XAP_EPVALUE_LEN];		// Value string
	int intLen;
	char chr;
	BYTE flgDataCheck = 0;				// Checks the consistency of the message header
	BYTE flgHbeat = FALSE;				// TRUE if the received message is a xAP heartbeat

	// Initialize the target structure (header)
	header->v = 0;
	header->hop = 0;
	memset(header->uid, 0, sizeof(header->uid));
	memset(header->class, 0, sizeof(header->class));
	memset(header->source, 0, sizeof(header->source));
	memset(header->target, 0, sizeof(header->target));

	// Reset the number of studied bodies (this is a new message)
	lxap_callBodyTimes = 0;	

	// Inspect the message header
	pStr1 = xapMessage;				// Place the pointer at the beginning of the xAP message

	// Skip possible leading <LF>
	if (pStr1[0] == '\n')
		pStr1++;

	// Verify the correct beginning of the message header
	if (!strncasecmp(pStr1, "xap-header\n{\n", 13))
		pStr1 += 13;				// Place the pointer inside the header section
	// Is it a xAP heartbeat
	else if (!strncasecmp(pStr1, "xap-hbeat\n{\n", 12)) {
		flgHbeat = TRUE;			// it's a xAP heartbeat
		pStr1 += 12;				// Place the pointer inside the header section
	}
	else
		return NULL;

	// Until the end of the header section
	while (pStr1[0] != '}') {
		// Capture the key string
		if ((pStr2 = strchr(pStr1, '=')) == NULL)
			return NULL;

		if ((intLen = pStr2 - pStr1) >= sizeof(strKey))		// Only within the limits of strKey
			return NULL;

		strncpy(strKey, pStr1, intLen);						// Copy the key string into strKey
		strKey[intLen] = 0;									// Terminate the string
		pStr1 = pStr2 + 1;									// Move the pointer to the position of the value

		// Capture the value string
		if ((pStr2 = strchr(pStr1, '\n')) == NULL)
			return NULL;

		if ((intLen = pStr2 - pStr1) >= sizeof(strValue))	// Only within the limits of strValue
			return NULL;

		strncpy(strValue, pStr1, intLen);					// Copy the value string into strValue
		strValue[intLen] = 0;								// Terminate the string
		pStr1 = pStr2 + 1;									// Move the pointer to the next line

		// Evaluate the key/value pair
		chr = strKey[0];									// Take strKey first character
		if (isupper((int)chr))								// Convert strKey first character to lower case
			chr = tolower((int)chr);						// if necessary

		switch (chr) {										// Consider strKey first character
			case 'v':
				// xAP specification version?
				if (!strcasecmp(strKey, "v")) {
					header->v = atoi(strValue);				// Store the version in the struct
					flgDataCheck |= 0x01;					// First bit on
				}
				break;
			case 'h':
				// hop count?
				if (!strcasecmp(strKey, "hop")) {
					header->hop = atoi(strValue);			// Store the hop count in the struct
					flgDataCheck |= 0x02;					// Second bit on
				}
				break;
			case 'u':
				// UID?
				if (!strcasecmp(strKey, "uid")) {
					if (strlen(strValue) >= sizeof(header->uid))
						return NULL;
					strcpy(header->uid, strValue);			// Store the UID in the struct
					flgDataCheck |= 0x04;					// Third bit on
				}
				break;
			case 'c':
				// Message class?
				if (!strcasecmp(strKey, "class")) {
					if (strlen(strValue) > sizeof(header->class))
						return NULL;
					strcpy(header->class, strValue);		// Store the class in the struct
					flgDataCheck |= 0x08;					// Fourth bit on
				}
				break;
			case 's':
				// xAP source?
				if (!strcasecmp(strKey, "source")) {
					if (strlen(strValue) > sizeof(header->source))
						return NULL;
					strcpy(header->source, strValue);		// Store the source in the struct
					flgDataCheck |= 0x10;					// Fifth bit on
				}
				break;
			case 't':
				// xAP target?
				if (!strcasecmp(strKey, "target")) {
					if (strlen(strValue) > sizeof(header->target))
						return NULL;
					strcpy(header->target, strValue);		// Store the target in the struct
					flgDataCheck |= 0x20;					// Sixth bit on
				}
				break;
			case 'i':
				// Heartbeat interval?
				if (!strcasecmp(strKey, "interval")) {
					header->interval = atoi(strValue);		// Store the heartbeat interval in the struct
					flgDataCheck |= 0x40;					// Seventh bit on
				}
				break;
			default:
				break;
		}
	}

	// Verify the consistency of the message header
	// If the message is a xAP heartbeat
	if (flgHbeat) {
		// Is the message really not a xAP heartbeat? Consistency error?
		if (strcasecmp(header->class, "xap-hbeat.alive") || flgDataCheck != 0x5F)	// 0x5F = 01011111b
			return NULL;
	}
	// If conventional message
	else {
		// Suported schemas

		// If BSC command or query, there should be a target field. Same for xap-x10.request
		if (!strcasecmp(header->class, "xAPBSC.cmd") || !strcasecmp(header->class, "xAPBSC.query") ||
			 !strcasecmp(header->class, "xap-x10.request"))	{
			if (flgDataCheck != 0x3F)												// 0x3F = 00111111b
				return NULL;
		}
	}

	return pStr1+2;		// Return pointer to the body
}

//*************************************************************************
//	xapEvalTarget
//
//	Evaluates the target field and decides whether the message is addressed
//	to the device or not. This function considers wildcarding.
//	Address format: Vendor.Device.Instance:subaddress
//
//	'strTarget'		string containing the target of the xAP message
//	'strGiven'		string containing the target of the xAP message
//
// Returns:
//		1 if the target address matches the given address
//		0 otherwise
//*************************************************************************

short int xapEvalTarget(char *strTarget, char *strGiven)
{
	char *pStr1, *pStr2, *pSubAddrTarget;	// Pointers to strings
	char *pStr3, *pStr4, *pSubAddrGiven;	// Pointers to strings
	BYTE flgGoOn = FALSE;					// If true, continue parsing
	BYTE flgWildCard = FALSE;				// If true, ">" wildcard found in the target base address
	BYTE flgTargetSubaddrEnd = FALSE;		// If true, no more subaddress part found
	BYTE flgGivenSubaddrEnd = FALSE;		// If true, no more subaddress part found
	char strTargetBuf[XAP_ADDRESS_LEN];
	char strGivenBuf[XAP_ADDRESS_LEN];

	strcpy(strTargetBuf, strTarget);		// Copy addresses to buffers
	strcpy(strGivenBuf, strGiven);
	
	pStr1 = strTargetBuf;					// Pointer at the beginning of the target string
	pStr3 = strGivenBuf;					// Pointer at the beginning of the given string
	
	// Search target subaddress 
	pSubAddrTarget = NULL;
	if ((pStr2 = strchr(pStr1, ':')) != NULL) {
		pStr2[0] = 0;						// Terminate base address string
		pSubAddrTarget = pStr2 + 1;			// Pointer to the target subaddress string
	}
		
	// Search given subaddress 
	pSubAddrGiven = NULL;
	if ((pStr4 = strchr(pStr3, ':')) != NULL) {
		pStr4[0] = 0;						// Terminate base address string
		pSubAddrGiven = pStr4 + 1;			// Pointer to the given subaddress string
	}

	// Compare vendor name
	if ((pStr2 = strchr(pStr1, '.')) != NULL) {
		pStr2[0] = 0;
		if ((pStr4 = strchr(pStr3, '.')) != NULL) {
			pStr4[0] = 0;
			if (!strcasecmp(pStr1, pStr3) || !strcmp(pStr1, "*") || !strcmp(pStr3, "*")) {
				pStr1 = pStr2 + 1;
				pStr3 = pStr4 + 1;
				// Compare device name
				if ((pStr2 = strchr(pStr1, '.')) != NULL) {
					pStr2[0] = 0;
					if ((pStr4 = strchr(pStr3, '.')) != NULL) {
						pStr4[0] = 0;
						if (!strcasecmp(pStr1, pStr3) || !strcmp(pStr1, "*") || !strcmp(pStr3, "*")) {
							pStr1 = pStr2 + 1;
							pStr3 = pStr4 + 1;
							// Compare instance name
							if (!strcasecmp(pStr1, pStr3) || !strcmp(pStr1, "*") || !strcmp(pStr3, "*"))
								flgGoOn = TRUE;
							// ">" wildcard? in instance part of target address
							else if (!strcmp(pStr1, ">"))
								flgWildCard = TRUE;
							// ">" wildcard? in instance part of given address
							else if (!strcmp(pStr3, ">"))
								flgWildCard = TRUE;
						}
					}
					// ">" wildcard? in device part of given address
					else if (!strcmp(pStr3, ">"))
						flgWildCard = TRUE;
				}
				// ">" wildcard? in device part of target address
				else if (!strcmp(pStr1, ">"))
					flgWildCard = TRUE;
			}
		}
		// ">" wildcard? in vendor part of given address
		else if (!strcmp(pStr3, ">"))
			flgWildCard = TRUE;
	}
	// ">" wildcard? in vendor part of target address
	else if (!strcmp(pStr1, ">"))
		flgWildCard = TRUE;

	// If logical address is wildcarding, the target matches
	if (flgWildCard)
		return 1;							// ">" wildcard found, the target matches
		
	// If logical address different to target logical part, the target doesn't match
	if (!flgGoOn)
		return 0;							// logical part doesn't match

	// Check if the addresses has subaddress part
	if ((pSubAddrTarget == NULL) && (pSubAddrGiven == NULL)) 
		return flgGoOn;						// Both addresses hasn't subaddress parts, logical part matches
	if ((pSubAddrTarget == NULL) || (pSubAddrGiven == NULL)) 
		return 0;							// If one of address hasn't subaddress part, addresses doesn't match
	
	// Parse Subaddress	
	pStr1 = pSubAddrTarget;					// Pointer at the beginning of the target subaddress string
	pStr3 = pSubAddrGiven;					// Pointer at the beginning of the given subaddress string

	while (!flgTargetSubaddrEnd && !flgGivenSubaddrEnd) {
		// Search for the next part of the target subaddress
		if ((pStr2 = strchr(pStr1, '.')) != NULL)
			pStr2[0] = 0;
		else
			flgTargetSubaddrEnd = TRUE;		// End of target subaddress reached
			
		// Search for the next part of the given subaddress
		if ((pStr4 = strchr(pStr3, '.')) != NULL)
			pStr4[0] = 0;
		else
			flgGivenSubaddrEnd = TRUE;		// End of given subaddress reached

		// Compare subaddress parts
		if (!strcasecmp(pStr1, pStr3) || !strcmp(pStr1, "*") || !strcmp(pStr3, "*")) {
			if (!flgTargetSubaddrEnd)
				pStr1 = pStr2 + 1;			// Set the string pointer for next target subaddress part
			if (!flgGivenSubaddrEnd)
				pStr3 = pStr4 + 1;			// Set the string pointer for next given subaddress part
		}
		else if ((!strcmp(pStr1, ">")) || (!strcmp(pStr3, ">")))
			return 1;						// subaddress is wildcarding, the target matches
		else
			return 0;						// Given and Target subaddresses doesn't match
	}
	
	// If both subaddresses ends, the target matches
	if (flgTargetSubaddrEnd && flgGivenSubaddrEnd)
		return 1;							// subaddress is wildcarding, the target matches
	else
		return 0;							// Given and Target subaddresses doesn't match
}

//*************************************************************************
// xapSendHbeat
//
// This functions sends a xAP heartbeat
//
//	'xapaddr'		xAP Address (Vendor, Device, Instance)
//	'xapuid'		xAP UID
//	'hbeatFreq'		xAP heartbeat frequence (in seconds)
//	'udpport'		UDP port to broadcast xAP messages
//
//	Returns:
//		1 if the functions sends the message successfully
//		0 otherwise
//*************************************************************************

short int xapSendHbeat(xAP_ADDRESS xapaddr, char *xapuid, USHORT hbeatFreq, USHORT udpport) {
	
	char strMessage[XAP_MSG_SIZE];				// Will store the outgoing UDP message

	// Assemble the "xap-hbeat.alive" message
	sprintf(strMessage, "xap-hbeat\n{\nv=%d\nhop=1\nuid=%s\nclass=xap-hbeat.alive\nsource=%s.%s.%s\ninterval=%d\nport=%d\n}\n",
		 lxap_xap_version, xapuid, xapaddr.vendor, xapaddr.device, xapaddr.instance, hbeatFreq, udpport);

	#ifdef DEBUG_LIBXAP
	printf("Heartbeat source=%s.%s.%s, interval=%d, port=%d\n", xapaddr.vendor, xapaddr.device, xapaddr.instance, hbeatFreq, udpport);
	#endif

	// Send the message
	if (sendto(lxap_tx_sockfd, strMessage, strlen(strMessage), 0, (struct sockaddr*)&lxap_tx_address, sizeof(lxap_tx_address)) < 0)
		return 0;

	return 1;
}

//*************************************************************************
// xapSendHbShutdown
//
// This functions sends a xAP heartbeat STOP message (xap-hbeat.stopped)
//
//	'xapaddr'		xAP Address (Vendor, Device, Instance)
//	'xapuid'		xAP UID
//	'hbeatFreq'		xAP heartbeat frequence (in seconds)
//	'udpport'		UDP port to broadcast xAP messages
//
//	Returns:
//		1 if the functions sends the message successfully
//		0 otherwise
//*************************************************************************

short int xapSendHbShutdown(xAP_ADDRESS xapaddr, char *xapuid, USHORT hbeatFreq, USHORT udpport) {
	
	char strMessage[XAP_MSG_SIZE];				// Will store the outgoing UDP message

	// Assemble the "xap-hbeat.stopped" message
	sprintf(strMessage, "xap-hbeat\n{\nv=%d\nhop=1\nuid=%s\nclass=xap-hbeat.stopped\nsource=%s.%s.%s\ninterval=%d\nport=%d\n}\n",
		 lxap_xap_version, xapuid, xapaddr.vendor, xapaddr.device, xapaddr.instance, hbeatFreq, udpport);

	#ifdef DEBUG_LIBXAP
	printf("Heartbeat stop for source=%s.%s.%s, interval=%d, port=%d\n", xapaddr.vendor, xapaddr.device, xapaddr.instance, hbeatFreq, udpport);
	#endif

	// Send the message
	if (sendto(lxap_tx_sockfd, strMessage, strlen(strMessage), 0, (struct sockaddr*)&lxap_tx_address, sizeof(lxap_tx_address)) < 0)
		return 0;

	return 1;
}
