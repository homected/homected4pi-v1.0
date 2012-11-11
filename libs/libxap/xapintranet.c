/**************************************************************************

	xapintranet.c

	Functions needed to communicate under the xAP intranet schema

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
	Copyright (c) Daniel Berenguer <dberenguer@usapiens.com>, 2007

	Last changes:

	18/04/12 by Jose Luis Galindo: Recoded
	31/03/08 by Daniel Berenguer : first version.

***************************************************************************/

// Include section:

#include "libxap.h"

// Define section:

// Global functions section:

// Global variables section:


//*************************************************************************
// xapReadWebBody
//
//	Analyzes the xAP SMS message body pointed by pBody
//
//	'pBody'			Pointer to the beginning of the body section within the
//						xAP message. This pointer should be recovered from a
//						previous call to 'xapReadHead'
//	'name'			Name of application (returned value)
//	'desc'			Description of application (returned value)
//	'pc'			Name of PC (returned value)
//	'icon'			URL to the application icon
//	'url'			Application URL
//	'type'			Type of web.service message (returned value)
//
// Returns:
//		1 if the function completes successfully
//		0 in case of error
//*************************************************************************

short int xapReadWebBody(char *pBody, char *name, char *desc, char *pc, char *icon, char *url, BYTE *type)
{
	char strName[XAP_WEB_FIELD_LEN];	// Name of application
	char strDesc[XAP_WEB_FIELD_LEN];	// Description
	char strPC[XAP_WEB_FIELD_LEN];		// Name of PC
	char strIcon[XAP_WEB_FIELD_LEN];	// icon
	char strURL[XAP_WEB_FIELD_LEN];		// URL
	BYTE flgType = WEB_REQUEST;			// Request message by default
	char *pStr1, *pStr2;				// Pointers to strings
	char strKey[XAP_KEY_LEN];			// Keyword string
	char strValue[XAP_EPVALUE_LEN];		// Value string
	char chr;
	BYTE flgDataCheck = 0;				// Checks the consistency of the message body

	// Inspect the message body
	pStr1 = pBody;						// Place the pointer at the beginning of the xAP body

	// Verify the correct beginning of the message body
	if (!strncasecmp(pStr1, "request\n{\n", 10))
		pStr1 += 10;					// Place the pointer inside the body section
	else if (!strncasecmp(pStr1, "server.start\n{\n", 15)) {
		flgType = WEB_START;
		pStr1 += 15;					// Place the pointer inside the body section
	}
	else if (!strncasecmp(pStr1, "server.stop\n{\n", 14)) {
		flgType = WEB_STOP;
		pStr1 += 14;					// Place the pointer inside the body section
	}
	else
		return 0;

	// Up to the end of the body section
	while (pStr1[0] != '}') {
		
		// Capture the key string
		if ((pStr2 = strchr(pStr1, '=')) == NULL)
			return 0;
		pStr2[0] = 0;
		if (strlen(pStr1) >= sizeof(strKey))				// Only within the limits of strKey
			return 0;
		strcpy(strKey, pStr1);								// Copy the key string into strKey
		pStr1 = pStr2 + 1;									// Move the pointer to the position of the value

		// Capture the value string
		if ((pStr2 = strchr(pStr1, '\n')) == NULL)
			return 0;
		pStr2[0] = 0;
		if (strlen(pStr1) >= sizeof(strValue))				// Only within the limits od strValue
			return 0;
		strcpy(strValue, pStr1);							// Copy the key string into strValue
		pStr1 = pStr2 + 1;									// Move the pointer to the next line

		// Evaluate the key/value pair
		chr = strKey[0];									// Take strKey first character
		if (isupper((int)chr))								// Convert strKey first character to lower case
			chr = tolower((int)chr);						// if necessary

		switch (chr)										// Consider strKey first character
		{
			case 'n':
				// name of application?
				if (!strcasecmp(strKey, "name")) {
					if (strlen(strValue) >= sizeof(strName))
						return 0;
					strcpy(strName, strValue);				// Store name
					flgDataCheck |= 0x01;					// First bit on
				}
				break;
			case 'd':
				// description of application?
				if (!strcasecmp(strKey, "desc")) {
					if (strlen(strValue) >= sizeof(strDesc))
						return 0;
					strcpy(strDesc, strValue);				// Store description
					flgDataCheck |= 0x02;					// Second bit on
				}
				break;
			case 'p':
				// name of PC?
				if (!strcasecmp(strKey, "pc")) {
					if (strlen(strValue) >= sizeof(strPC))
						return 0;
					strcpy(strPC, strValue);				// Store PC name
					flgDataCheck |= 0x04;					// Second bit on
				}
				break;
			case 'i':
				// application icon?
				if (!strcasecmp(strKey, "icon")) {
					if (strlen(strValue) >= sizeof(strIcon))
						return 0;
					strcpy(strIcon, strValue);				// Store icon URL
					flgDataCheck |= 0x08;					// Second bit on
				}
				break;
			case 'u':
				// application URL?
				if (!strcasecmp(strKey, "url")) {
					if (strlen(strValue) >= sizeof(strURL))
						return 0;
					strcpy(strURL, strValue);				// Store application URL
					flgDataCheck |= 0x10;					// Second bit on
				}
				break;
			default:
				break;
		}
	}

	*type = flgType;

	// Verify the consistency of the message body
	if (flgType == WEB_REQUEST)	{
		if (flgDataCheck == 0)
			return 1;
	}
	else if (flgDataCheck == 0x1F) {
		// Return values
		if (name != NULL)
			strcpy(name, strName);
		if (desc != NULL)
			strcpy(desc, strDesc);
		if (pc != NULL)
			strcpy(pc, strPC);
		if (icon != NULL)
			strcpy(icon, strIcon);
		if (url != NULL)
			strcpy(url, strURL);

		return 1;
	}
	return 0;
}

//*************************************************************************
// xapSendWebService
//
// This functions sends a Web.Service message
//
//	'xapaddr'		xAP Address (Vendor, Device, Instance)
//	'strUID'		Device UID
//	'strName'		Name of application
//	'strDesc'		Description of application
//	'strPC'			Name of PC
//	'strIcon'		Application icon
//	'strURL'		Application URL
//	'type'			"Server.Start" or "Server.Stop"
//
//	Returns:
//		1 if the functions sends the message successfully
//		0 in case of error
//*************************************************************************

short int xapSendWebService(xAP_ADDRESS xapaddr, char *strUID, char *strName, char *strDesc, char *strPC, char *strIcon, char *strURL, char* type) {

	char strMessage[XAP_MSG_SIZE];		// Outgoing UDP message
	char strBody[XAP_MSG_SIZE];			// Body string

	sprintf(strBody, "{\nname=%s\ndesc=%s\npc=%s\nicon=%s\nurl=%s\n}\n", strName, strDesc, strPC, strIcon, strURL);

	// Assemble the message
	sprintf(strMessage, "xap-header\n{\nv=%d\nhop=1\nuid=%s\nclass=web.service\nsource=%s.%s.%s\n}\n%s\n%s",
		lxap_xap_version, strUID, xapaddr.vendor, xapaddr.device, xapaddr.instance, type, strBody);
		
	#ifdef DEBUG_LIBXAP
	printf("xapSendWebService:\n%s", strMessage);
	#endif

	// Send message
	if (sendto(lxap_tx_sockfd, strMessage, strlen(strMessage), 0, (struct sockaddr*)&lxap_tx_address, sizeof(lxap_tx_address)) < 0)
		return 0;

	return 1;
}

//*************************************************************************
// xapSendWebRequest
//
// This functions sends a Web.Service request
//
//	'strSource'		Source address (Vendor.Device.Instance)
//	'strUID'		Device UID
//
//	Returns:
//		1 if the functions sends the message successfully
//		0 in case of error
//*************************************************************************

short int xapSendWebRequest(char *strSource, char *strUID) {
	
	char strMessage[XAP_MSG_SIZE];		// Outgoing UDP message

	// Assemble the message
	sprintf(strMessage, "xap-header\n{\nv=%d\nhop=1\nuid=%s\nclass=web.service\nsource=%s\n}\nRequest\n{\n}\n",
		lxap_xap_version, strUID, strSource);

	#ifdef DEBUG_LIBXAP
	printf("xapSendWebRequest:\n%s", strMessage);
	#endif
	
	// Send message
	if (sendto(lxap_tx_sockfd, strMessage, strlen(strMessage), 0, (struct sockaddr*)&lxap_tx_address, sizeof(lxap_tx_address)) < 0)
		return 0;

	return 1;
}
