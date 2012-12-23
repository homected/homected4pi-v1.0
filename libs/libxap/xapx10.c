/**************************************************************************

	xapx10.c

	Functions needed to communicate under the xAP-X10 schema

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

	20/12/12 by Jose Luis Galindo: Recoded for Homected project
	03/09/07 by Daniel Berenguer : first version.

***************************************************************************/

// Include section:

#include "libxap.h"

// Define section:

// Global functions section:

// Global variables section:

//*************************************************************************
//	getX10presetDim1Level
//
//	Return the preset dim absolute level for the code given
//
//	'value'	preset dim code to be converted to absolute value
//
//	Returns:
//		Absolute Preset Dim level
//
//*************************************************************************
BYTE getX10presetDim1Level(BYTE code)
{
	if (code == 0) return 0;
	else if (code == 8) return 3;
	else if (code == 4) return 6;
	else if (code == 12) return 10;
	else if (code == 2) return 13;
	else if (code == 10) return 16;
	else if (code == 6) return 19;
	else if (code == 14) return 23;
	else if (code == 1) return 26;
	else if (code == 9) return 29;
	else if (code == 5) return 32;
	else if (code == 13) return 35;
	else if (code == 3) return 39;
	else if (code == 11) return 42;
	else if (code == 7) return 45;
	else if (code == 15) return 48;
	else return 255;
}

//*************************************************************************
//	getX10presetDim2Level
//
//	Return the preset dim absolute level for the code given
//
//	'value'	preset dim code to be converted to absolute value
//
//	Returns:
//		Absolute Preset Dim level
//
//*************************************************************************
BYTE getX10presetDim2Level(BYTE code)
{
	if (code == 0) return 52;
	else if (code == 8) return 55;
	else if (code == 4) return 58;
	else if (code == 12) return 61;
	else if (code == 2) return 65;
	else if (code == 10) return 68;
	else if (code == 6) return 71;
	else if (code == 14) return 74;
	else if (code == 1) return 77;
	else if (code == 9) return 81;
	else if (code == 5) return 84;
	else if (code == 13) return 87;
	else if (code == 3) return 90;
	else if (code == 11) return 94;
	else if (code == 7) return 97;
	else if (code == 15) return 100;
	else return 255;
}

//*************************************************************************
// xapReadX10Body
//
//	Inspects the xAP X10 message body pointed by pBody
//
//	'pBody'			Pointer to the beginning of the body section within the
//						xAP message. This pointer should be recovered from a
//						previous call to 'xapReadHead'
//	'header'		Header struct obtained from a previous call to xapReadHead
//	'devlist'		List of devices pointed by the message
//	'value1'		Dim / Bright value or extended code
//	'value2'		Extended data
//
// Returns:
//		1 if the function completes successfully
//		0 in case of error
//*************************************************************************
short int xapReadX10Body(char *pBody, xAP_HEADER header, char *devlist, char *command, BYTE *value1, BYTE *value2)
{
	char strDevList[50];				// List of devices pointed by the message
	char strCommand[XAP_EPVALUE_LEN];	// Command string
	char *pStr1, *pStr2;				// Pointers to strings
	char strKey[XAP_KEY_LEN];			// Keyword string
	char strValue[XAP_ADDRESS_LEN];		// Value string
	int intLen;							// Data length of strValue
	int intValue1 = 0;					// Store dim/bright level or extended code
	int intValue2 = 0;					// Store extended data
	char chr;
	BYTE flgDataCheck = 0;				// Checks the consistency of the message body

	// Inspect the message body
	pStr1 = pBody;		// Place the pointer at the beginning of the xAP body

	// Verify the correct beginning of the body depending of the type of xAP BSC message
	// If xap-x10.request
	if (!strcasecmp(header.class, "xap-x10.request"))
	{
		// Verify the body tittle
		if (!strncasecmp(pStr1, "xap-x10.request\n{\n", 18))
			pStr1 += 18;			// Place the pointer inside the body section
		else
			return 0;
	}
	// If xap-x10.event
	else if (!strcasecmp(header.class, "xap-x10.event"))
	{
		//Verify the body tittle
		if (!strncasecmp(pStr1, "xap-x10.event\n{\n", 16))
			pStr1 += 16;			// Place the pointer inside the body section
		else
			return 0;
	}
	else
		return 0;

	// Up to the end of the body section
	while (pStr1[0] != '}')
	{
		// Capture the key string
		if ((pStr2 = strchr(pStr1, '=')) == NULL)
			return 0;
		if ((intLen = pStr2 - pStr1) >= sizeof(strKey))	// Only within the limits of strKey
			return 0;
		strncpy(strKey, pStr1, intLen);					// Copy the key string into strKey
		strKey[intLen] = 0;								// Terminate the string
		pStr1 = pStr2 + 1;								// Move the pointer to the position of the value

		// Capture the value string
		if ((pStr2 = strchr(pStr1, '\n')) == NULL)
			return 0;
		if ((intLen = pStr2 - pStr1) >= sizeof(strValue))	// Only within the limits od strValue
			return 0;
		strncpy(strValue, pStr1, intLen);					// Copy the key string into strValue
		strValue[intLen] = 0;								// Terminate the string
		pStr1 = pStr2 + 1;									// Move the pointer to the next line

		// Evaluate the key/value pair
		chr = strKey[0];								// Take strKey first character
		if (isupper((int)chr))							// Convert strKey first character to lower case
			chr = tolower((int)chr);					// if necessary

		switch (chr)									// Consider strKey first character
		{
			case 'c':
				// command to be executed?
				if (!strcasecmp(strKey, "command"))
				{
					if (strlen(strValue) >= sizeof(strCommand))
						return 0;
					//Only allowed commands
					if (strcasecmp(strValue, XAP_X10_CMD_ON) 
						&& strcasecmp(strValue, XAP_X10_CMD_OFF) 
						&& strcasecmp(strValue, XAP_X10_CMD_DIM) 
						&& strcasecmp(strValue, XAP_X10_CMD_BRIGHT) 
						&& strcasecmp(strValue, XAP_X10_CMD_ALL_LIGHTS_ON) 
						&& strcasecmp(strValue, XAP_X10_CMD_ALL_LIGHTS_OFF) 
						&& strcasecmp(strValue, XAP_X10_CMD_ALL_UNITS_OFF) 
						&& strcasecmp(strValue, XAP_X10_CMD_HAIL_REQ) 
						//&& strcasecmp(strValue, XAP_X10_EVN_HAIL_ACK)		// is an event not a command
						&& strcasecmp(strValue, XAP_X10_CMD_PRESET_DIM_1) 
						&& strcasecmp(strValue, XAP_X10_CMD_PRESET_DIM_2) 
						//&& strcasecmp(strValue, XAP_X10_EVN_STATUS_ON)	// is an event not a command
						//&& strcasecmp(strValue, XAP_X10_EVN_STATUS_OFF)	// is an event not a command
						&& strcasecmp(strValue, XAP_X10_CMD_STATUS_REQ) 
						&& strcasecmp(strValue, XAP_X10_CMD_EXT_DATA) 
						&& strcasecmp(strValue, XAP_X10_CMD_EXT_CODE) 		// Not included in xAP-X10 schema
						&& strcasecmp(strValue, XAP_X10_CMD_EXTENDED1) 		// Not included in xAP-X10 schema
						&& strcasecmp(strValue, XAP_X10_CMD_EXTENDED2) 		// Not included in xAP-X10 schema
						&& strcasecmp(strValue, XAP_X10_CMD_EXTENDED3))		// Not included in xAP-X10 schema
						return 0;
					strcpy(strCommand, strValue);			// Store the command string
					flgDataCheck |= 0x01;					// First bit on
				}
				// Count (only for dim, bright and preset dim commands)?
				else if (!strcasecmp(strKey, "count"))
				{
					if (strlen(strValue) > 3)
						return 0;
					intValue1 = atoi(strValue);				// Absolute Dim/Bright level
					flgDataCheck |= 0x08;					// Fourth bit on
				}
				break;
			case 'd':
				// Device?
				if (!strcasecmp(strKey, "device"))
				{
					if (strlen(strValue) >= sizeof(strDevList))
						return 0;
					strcpy(strDevList, strValue);			// Store the list of devices
					flgDataCheck |= 0x02;					// Second bit on
				}
				break;
			case 'e':
				// Event?
				if (!strcasecmp(strKey, "event"))
				{
					if (strlen(strValue) >= sizeof(strCommand))
						return 0;
					//Only allowed commands
					if (strcasecmp(strValue, XAP_X10_CMD_ON) 
						&& strcasecmp(strValue, XAP_X10_CMD_OFF) 
						&& strcasecmp(strValue, XAP_X10_CMD_DIM) 
						&& strcasecmp(strValue, XAP_X10_CMD_BRIGHT) 
						&& strcasecmp(strValue, XAP_X10_CMD_ALL_LIGHTS_ON) 
						&& strcasecmp(strValue, XAP_X10_CMD_ALL_LIGHTS_OFF) 
						&& strcasecmp(strValue, XAP_X10_CMD_ALL_UNITS_OFF) 
						&& strcasecmp(strValue, XAP_X10_CMD_HAIL_REQ)
						&& strcasecmp(strValue, XAP_X10_EVN_HAIL_ACK)
						&& strcasecmp(strValue, XAP_X10_CMD_PRESET_DIM_1) 
						&& strcasecmp(strValue, XAP_X10_CMD_PRESET_DIM_2) 
						&& strcasecmp(strValue, XAP_X10_EVN_STATUS_ON)
						&& strcasecmp(strValue, XAP_X10_EVN_STATUS_OFF)
						&& strcasecmp(strValue, XAP_X10_CMD_STATUS_REQ) 
						&& strcasecmp(strValue, XAP_X10_CMD_EXT_DATA) 
						&& strcasecmp(strValue, XAP_X10_CMD_EXT_CODE) 	// Not included in xAP-X10 schema
						&& strcasecmp(strValue, XAP_X10_CMD_EXTENDED1)	// Not included in xAP-X10 schema
						&& strcasecmp(strValue, XAP_X10_CMD_EXTENDED2)	// Not included in xAP-X10 schema
						&& strcasecmp(strValue, XAP_X10_CMD_EXTENDED3))	// Not included in xAP-X10 schema
						return 0;
					strcpy(strCommand, strValue);			// Store the command string
					flgDataCheck |= 0x04;					// Third bit on
				}
				// extCmd (only for extended commands)	Not included in xAP-X10 schema
				else if (!strcasecmp(strKey, "extCmd"))
				{
					if (strlen(strValue) > 3)
						return 0;
					intValue1 = atoi(strValue);				// Extended command value
					flgDataCheck |= 0x10;					// Five bit on
				}
				// extData (only for extended commands)	Not included in xAP-X10 schema
				else if (!strcasecmp(strKey, "extData"))
				{
					if (strlen(strValue) > 3)
						return 0;
					intValue2 = atoi(strValue);				// Extended data value
					flgDataCheck |= 0x20;					// Six bit on
				}
				break;
			default:
				break;
		}
	}

	// Verify the consistency of the message body
	switch(flgDataCheck)
	{
//		case 0x01:	// Only "device" fields
			// Check allowed commands without parameters
//			if (!strcasecmp(strCommand, "HAIL_REQ"))		// NOT SUPPORTED IN THIS VERSION
//				return 0;
//			break;

		case 0x06:	// "device" and "event" fields
			// Check extended event without extCmd and extData fields
			if (!strcasecmp(strCommand, XAP_X10_CMD_EXT_DATA) 		// EXT_DATA=EXTENDED2, EXT_CODE=EXTENDED1
				|| !strcasecmp(strCommand, XAP_X10_CMD_EXT_CODE)	// Not included in xAP-X10 schema
				|| !strcasecmp(strCommand, XAP_X10_CMD_EXTENDED1) 	// Not included in xAP-X10 schema
				|| !strcasecmp(strCommand, XAP_X10_CMD_EXTENDED2))	// Not included in xAP-X10 schema
				return 0;
			// The "event" field is only allowed for xap-x10.event
			if (strcasecmp(header.class, "xap-x10.event"))
				return 0;
			break;

		case 0x03:	// "device" and "command" fields
			// Check extended command without extCmd and extData fields
			if (!strcasecmp(strCommand, XAP_X10_CMD_EXT_DATA) 		// EXT_DATA=EXTENDED2, EXT_CODE=EXTENDED1
				|| !strcasecmp(strCommand, XAP_X10_CMD_EXT_CODE)	// Not included in xAP-X10 schema
				|| !strcasecmp(strCommand, XAP_X10_CMD_EXTENDED1) 	// Not included in xAP-X10 schema
				|| !strcasecmp(strCommand, XAP_X10_CMD_EXTENDED2))	// Not included in xAP-X10 schema
				return 0;
			break;

		case 0x0E:	// "device", "event" and "count" fields
			// The "event" field is only allowed for xap-x10.event
			if (strcasecmp(header.class, "xap-x10.event"))
				return 0;

		case 0x0B:	// "device", "command" and "count" fields
			// The "count" field is only allowed for dim/bright/preset_dim commands
			if (strcasecmp(strCommand, XAP_X10_CMD_DIM) && 
				strcasecmp(strCommand, XAP_X10_CMD_BRIGHT) && 
				strcasecmp(strCommand, XAP_X10_CMD_PRESET_DIM_1) && 
				strcasecmp(strCommand, XAP_X10_CMD_PRESET_DIM_2) && 
				strcasecmp(strCommand, XAP_X10_CMD_EXTENDED3))		// Not included in xAP-X10 schema
				return 0;
			break;

		case 0x36:	// "device" and "event" fields with extended data
			// Check command is extended
			if (!strcasecmp(strCommand, XAP_X10_CMD_EXT_DATA) 		
				|| !strcasecmp(strCommand, XAP_X10_CMD_EXT_CODE)	// Not included in xAP-X10 schema
				|| !strcasecmp(strCommand, XAP_X10_CMD_EXTENDED1) 	// Not included in xAP-X10 schema
				|| !strcasecmp(strCommand, XAP_X10_CMD_EXTENDED2))	// Not included in xAP-X10 schema
				return 0;
			// The "event" field is only allowed for xap-x10.event
			if (strcasecmp(header.class, "xap-x10.event"))
				return 0;
			break;

		case 0x33:	// "device" and "command" fields with extended data
			// Check command is extended
			if (!strcasecmp(strCommand, XAP_X10_CMD_EXT_DATA) 		
				|| !strcasecmp(strCommand, XAP_X10_CMD_EXT_CODE)	// Not included in xAP-X10 schema
				|| !strcasecmp(strCommand, XAP_X10_CMD_EXTENDED1) 	// Not included in xAP-X10 schema
				|| !strcasecmp(strCommand, XAP_X10_CMD_EXTENDED2))	// Not included in xAP-X10 schema
				return 0;
			break;

		default:	// Bad format
			return 0;
			break;
	}

	// Verify the correct end of the body
	if (pStr1[1] != '\n')
		return 0;

	// Return values
	strcpy(command, strCommand);
	strcpy(devlist, strDevList);
	*value1 = (BYTE)intValue1;
	*value2 = (BYTE)intValue2;

	return 1;
}

//*************************************************************************
// xapSendX10evn
//
// This functions sends a xap-x10.event message notifying a value change
//
//	'strSource'		Source address (Vendor.Device.Instance)
//	'strUID'		Device UID
//	'device'		Device X10 code or list of devices
//	'command'		Executed X10 command
//	'value1'		Dim/Bright level or extended type command
//	'value2'		extended data
//
//	Returns:
//		1 if the functions sends the message successfully
//		0 in case of error
//*************************************************************************
short int xapSendX10evn(char *strSource, char *strUID, char *device, char *command, BYTE value1, BYTE value2)
{
	char strMessage[XAP_MSG_SIZE];		// Outgoing UDP message
	char strBody[XAP_MSG_SIZE];			// Body string

	// Build Body
	if (!strcasecmp(command, XAP_X10_CMD_ON) || 
		!strcasecmp(command, XAP_X10_CMD_OFF) || 
		!strcasecmp(command, XAP_X10_CMD_ALL_LIGHTS_OFF) ||
		!strcasecmp(command, XAP_X10_CMD_ALL_LIGHTS_ON) || 
		!strcasecmp(command, XAP_X10_CMD_ALL_UNITS_OFF) ||
		!strcasecmp(command, XAP_X10_CMD_HAIL_REQ) ||
		!strcasecmp(command, XAP_X10_EVN_HAIL_ACK) || 
		!strcasecmp(command, XAP_X10_EVN_STATUS_ON) ||
		!strcasecmp(command, XAP_X10_EVN_STATUS_OFF) || 
		!strcasecmp(command, XAP_X10_CMD_STATUS_REQ) ||
		!strcasecmp(command, XAP_X10_CMD_EXTENDED3))
		sprintf(strBody, "{\nevent=%s\ndevice=%s\n}\n", command, device);
	
	// If Dim/Bright event add "count" field
	else if (!strcasecmp(command, XAP_X10_CMD_DIM) || !strcasecmp(command, XAP_X10_CMD_BRIGHT))
		sprintf(strBody, "{\nevent=%s\ndevice=%s\ncount=%d\n}\n", command, device, value1);

	// If preset_dim_1 event add "count" field
	else if (!strcasecmp(command, XAP_X10_CMD_PRESET_DIM_1) || !strcasecmp(command, XAP_X10_CMD_EXTENDED3))
	{	
		BYTE absLevel;
		if((absLevel = getX10presetDim1Level(value1)) <= 100)
			sprintf(strBody, "{\nevent=%s\ndevice=%s\ncount=%d\n}\n", command, device, absLevel);
		else
			return 0;
	}

	// If preset_dim_2 event add "count" field
	else if (!strcasecmp(command, XAP_X10_CMD_PRESET_DIM_2))
	{	
		BYTE absLevel;
		if((absLevel = getX10presetDim2Level(value1)) <= 100)
			sprintf(strBody, "{\nevent=%s\ndevice=%s\ncount=%d\n}\n", command, device, absLevel);
		else
			return 0;
	}

	// If extendend code/data add "extCmd" and "extData" fields
	else if (!strcasecmp(command, XAP_X10_CMD_EXT_DATA) || !strcasecmp(command, XAP_X10_CMD_EXT_CODE) || 
			!strcasecmp(command, XAP_X10_CMD_EXTENDED1) || !strcasecmp(command, XAP_X10_CMD_EXTENDED2))
		sprintf(strBody, "{\nevent=%s\ndevice=%s\nextCmd=%d\nextData=%d\n}\n", command, device, value1, value2);
	else
		return 0;

	// Assemble the "xap-x10.event" message
	sprintf(strMessage, "xap-header\n{\nv=%d\nhop=1\nuid=%s\nclass=xap-x10.event\nsource=%s\n}\nxap-x10.event\n%s",
		lxap_xap_version, strUID, strSource, strBody);
		
	#ifdef DEBUG_LIBXAP
	printf("xapSendX10evn:\n%s", strMessage);
	#endif

	// Send the message
	if (sendto(lxap_tx_sockfd, strMessage, strlen(strMessage), 0, (struct sockaddr*)&lxap_tx_address, sizeof(lxap_tx_address)) < 0)
		return 0;
	
	return 1;
}

//*************************************************************************
// xapSendX10req
//
// This functions sends a xap-x10.request message
//
//	'strSource'		Source address (Vendor.Device.Instance)
//	'strUID'		Device UID
//	'strTarget'		Target address (Vendor.Device.Instance)
//	'device'		Device X10 code, list of devices or house code
//	'command'		Executed X10 command
//	'value1'		Dim/Bright level or extended type command
//	'value2'		extended data
//
//	Returns:
//		1 if the functions sends the message successfully
//		0 in case of error
//*************************************************************************
short int xapSendX10req(char *strSource, char *strUID, char *strTarget, char *device, char *command, BYTE value1, BYTE value2)
{
	char strMessage[XAP_MSG_SIZE];		// Outgoing UDP message
	char strBody[XAP_MSG_SIZE];			// Body string

	// Build Body
	if (!strcasecmp(command, XAP_X10_CMD_ON) || !strcasecmp(command, XAP_X10_CMD_OFF) || 
		!strcasecmp(command, XAP_X10_CMD_ALL_LIGHTS_ON) || 
		!strcasecmp(command, XAP_X10_CMD_ALL_LIGHTS_OFF) || 
		!strcasecmp(command, XAP_X10_CMD_ALL_UNITS_OFF))
		sprintf(strBody, "{\ncommand=%s\ndevice=%s\n}\n", command, device);
	
	// If Dim/Bright request add "count" field
	else if (!strcasecmp(command, XAP_X10_CMD_DIM) || !strcasecmp(command, XAP_X10_CMD_BRIGHT))
		sprintf(strBody, "{\ncommand=%s\ndevice=%s\ncount=%d\n}\n", command, device, value1);

	// Assemble the "xap-x10.request" message
	sprintf(strMessage, "xap-header\n{\nv=%d\nhop=1\nuid=%s\nclass=xap-x10.request\nsource=%s\ntarget=%s\n}\nxap-x10.request\n%s",
		lxap_xap_version, strUID, strSource, strTarget, strBody);

	#ifdef DEBUG_LIBXAP
	printf("xapSendX10req:\n%s", strMessage);
	#endif

	// Send the message
	if (sendto(lxap_tx_sockfd, strMessage, strlen(strMessage), 0, (struct sockaddr*)&lxap_tx_address, sizeof(lxap_tx_address)) < 0)
		return 0;
		
	return 1;
}
