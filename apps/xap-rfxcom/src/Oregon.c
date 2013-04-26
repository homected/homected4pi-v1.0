/**************************************************************************

	Oregon.c

	Oregon Scientific sensors

	Copyright (c) 2013 Jose Luis Galindo <support@homected.com>

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


	Last changes:

	12/01/13 by Jose Luis Galindo : First version
	
***************************************************************************/


//*************************************************************************
//*************************************************************************
// 								INCLUDE SECTION
//*************************************************************************
//*************************************************************************

#include "Oregon.h"


//*************************************************************************
//*************************************************************************
// 							PRIVATE FUNCTIONS SECTION
//*************************************************************************
//*************************************************************************

BYTE wrchannel(BYTE value) {
	BYTE ch;
	
	switch(value & 0x70) {
		case 0x10:
			ch = 1;
			break;
				
		case 0x20:
			ch = 2;
			break;
				
		case 0x40:
			ch = 3;
			break;
			
		default:
			ch = 0;
	}
	return ch;
}

BYTE wrchannel3(BYTE value) {
	return(value >> 4);
}

BYTE wbattery_indication(BYTE value) {
	if ((value & 0x04) == 0)
		return RFXCOM_OREGON_BAT_OK;
	else
		return RFXCOM_OREGON_BAT_LOW;
}

BYTE wrbattery(BYTE value) {
	return 100 - ((value & 0x0F) * 10);
}

BYTE checksum2(BYTE *msg) {
	BYTE cs = 0;
	
	cs += (msg[1] >> 4) & 0x0F;
	cs += ((msg[2] >> 4) & 0x0F) + (msg[2] & 0x0F);
	cs += ((msg[3] >> 4) & 0x0F) + (msg[3] & 0x0F);
	cs += ((msg[4] >> 4) & 0x0F) + (msg[4] & 0x0F);
	cs += ((msg[5] >> 4) & 0x0F) + (msg[5] & 0x0F);
	cs += ((msg[6] >> 4) & 0x0F) + (msg[6] & 0x0F);
	cs += ((msg[7] >> 4) & 0x0F) + (msg[7] & 0x0F);
	cs += ((msg[8] >> 4) & 0x0F) + (msg[8] & 0x0F);
	cs += (msg[9] & 0x0F);
	cs = (cs - (((msg[9] >> 4) & 0x0F) + ((msg[10] << 4) & 0xF0))) & 0xFF;
	if (cs != 0)
		return FALSE;
	return TRUE;
}

BYTE checksum7(BYTE *msg) {
	BYTE cs = 0;
	
	cs += (msg[1] >> 4) & 0x0F;
	cs += ((msg[2] >> 4) & 0x0F) + (msg[2] & 0x0F);
	cs += ((msg[3] >> 4) & 0x0F) + (msg[3] & 0x0F);
	cs += ((msg[4] >> 4) & 0x0F) + (msg[4] & 0x0F);
	cs += ((msg[5] >> 4) & 0x0F) + (msg[5] & 0x0F);
	cs += ((msg[6] >> 4) & 0x0F) + (msg[6] & 0x0F);
	cs += ((msg[7] >> 4) & 0x0F) + (msg[7] & 0x0F);
	cs = (cs - msg[8]) & 0xFF;
	if (cs != 0)
		return FALSE;
	return TRUE;
}

BYTE checksum8(BYTE *msg) {
	BYTE cs = 0;
	
	cs += (msg[1] >> 4) & 0x0F;
	cs += ((msg[2] >> 4) & 0x0F) + (msg[2] & 0x0F);
	cs += ((msg[3] >> 4) & 0x0F) + (msg[3] & 0x0F);
	cs += ((msg[4] >> 4) & 0x0F) + (msg[4] & 0x0F);
	cs += ((msg[5] >> 4) & 0x0F) + (msg[5] & 0x0F);
	cs += ((msg[6] >> 4) & 0x0F) + (msg[6] & 0x0F);
	cs += ((msg[7] >> 4) & 0x0F) + (msg[7] & 0x0F);
	cs += ((msg[8] >> 4) & 0x0F) + (msg[8] & 0x0F);
	cs = (cs - msg[9]) & 0xFF;
	if (cs != 0)
		return FALSE;
	return TRUE;
}

BYTE checksum9(BYTE *msg) {
	BYTE cs = 0;
	
	cs += (msg[1] >> 4) & 0x0F;
	cs += ((msg[2] >> 4) & 0x0F) + (msg[2] & 0x0F);
	cs += ((msg[3] >> 4) & 0x0F) + (msg[3] & 0x0F);
	cs += ((msg[4] >> 4) & 0x0F) + (msg[4] & 0x0F);
	cs += ((msg[5] >> 4) & 0x0F) + (msg[5] & 0x0F);
	cs += ((msg[6] >> 4) & 0x0F) + (msg[6] & 0x0F);
	cs += ((msg[7] >> 4) & 0x0F) + (msg[7] & 0x0F);
	cs += ((msg[8] >> 4) & 0x0F) + (msg[8] & 0x0F);
	cs += ((msg[9] >> 4) & 0x0F) + (msg[9] & 0x0F);
	cs = (cs - msg[10]) & 0xFF;
	if (cs != 0)
		return FALSE;
	return TRUE;
}

BYTE checksum10(BYTE *msg) {
	BYTE cs = 0;
	
	cs += (msg[1] >> 4) & 0x0F;
	cs += ((msg[2] >> 4) & 0x0F) + (msg[2] & 0x0F);
	cs += ((msg[3] >> 4) & 0x0F) + (msg[3] & 0x0F);
	cs += ((msg[4] >> 4) & 0x0F) + (msg[4] & 0x0F);
	cs += ((msg[5] >> 4) & 0x0F) + (msg[5] & 0x0F);
	cs += ((msg[6] >> 4) & 0x0F) + (msg[6] & 0x0F);
	cs += ((msg[7] >> 4) & 0x0F) + (msg[7] & 0x0F);
	cs += ((msg[8] >> 4) & 0x0F) + (msg[8] & 0x0F);
	cs += ((msg[9] >> 4) & 0x0F) + (msg[9] & 0x0F);
	cs += ((msg[10] >> 4) & 0x0F) + (msg[10] & 0x0F);
	cs = (cs - msg[11]) & 0xFF;
	if (cs != 0)
		return FALSE;
	return TRUE;
}

BYTE checksum11(BYTE *msg) {
	BYTE cs = 0;
	
	cs += (msg[1] >> 4) & 0x0F;
	cs += ((msg[2] >> 4) & 0x0F) + (msg[2] & 0x0F);
	cs += ((msg[3] >> 4) & 0x0F) + (msg[3] & 0x0F);
	cs += ((msg[4] >> 4) & 0x0F) + (msg[4] & 0x0F);
	cs += ((msg[5] >> 4) & 0x0F) + (msg[5] & 0x0F);
	cs += ((msg[6] >> 4) & 0x0F) + (msg[6] & 0x0F);
	cs += ((msg[7] >> 4) & 0x0F) + (msg[7] & 0x0F);
	cs += ((msg[8] >> 4) & 0x0F) + (msg[8] & 0x0F);
	cs += ((msg[9] >> 4) & 0x0F) + (msg[9] & 0x0F);
	cs += ((msg[10] >> 4) & 0x0F) + (msg[10] & 0x0F);
	cs += ((msg[11] >> 4) & 0x0F) + (msg[11] & 0x0F);
	cs = (cs - msg[12]) & 0xFF;
	if (cs != 0)
		return FALSE;
	return TRUE;
}

BYTE checksum12(BYTE *msg) {
	BYTE cs = 0;

	cs += ((msg[2] >> 4) & 0x0F) + (msg[2] & 0x0F);
	cs += ((msg[3] >> 4) & 0x0F) + (msg[3] & 0x0F);
	cs += ((msg[4] >> 4) & 0x0F) + (msg[4] & 0x0F);
	cs += ((msg[5] >> 4) & 0x0F) + (msg[5] & 0x0F);
	cs += ((msg[6] >> 4) & 0x0F) + (msg[6] & 0x0F);
	cs += ((msg[7] >> 4) & 0x0F) + (msg[7] & 0x0F);
	cs += ((msg[8] >> 4) & 0x0F) + (msg[8] & 0x0F);
	cs += ((msg[9] >> 4) & 0x0F) + (msg[9] & 0x0F);
	cs += ((msg[10] >> 4) & 0x0F) + (msg[10] & 0x0F);
	cs += ((msg[11] >> 4) & 0x0F) + (msg[11] & 0x0F);
	cs += (msg[12] & 0x0F);
	cs = (cs - ((msg[13] & 0x0F) << 4) - ((msg[12] >> 4) & 0x0F)) & 0xFF;
	if (cs != 0)
		return FALSE;
	return TRUE;
}

BYTE checksume(BYTE *msg) {
	BYTE cs = 0;
	
	cs += ((msg[1] >> 4) & 0x0F) + (msg[1] & 0x0F);
	cs += ((msg[2] >> 4) & 0x0F) + (msg[2] & 0x0F);
	cs += ((msg[3] >> 4) & 0x0F) + (msg[3] & 0x0F);
	cs += ((msg[4] >> 4) & 0x0F) + (msg[4] & 0x0F);
	cs += ((msg[5] >> 4) & 0x0F) + (msg[5] & 0x0F);
	cs += ((msg[6] >> 4) & 0x0F) + (msg[6] & 0x0F);
	cs += ((msg[7] >> 4) & 0x0F) + (msg[7] & 0x0F);
	cs = (cs - msg[8]) & 0xFF;
	if (cs != 0x18)
		return FALSE;
	return TRUE;
}

BYTE checksumr(BYTE *msg) {
	BYTE cs = 0;
	
	cs += (msg[1] >> 4) & 0x0F;
	cs += ((msg[2] >> 4) & 0x0F) + (msg[2] & 0x0F);
	cs += ((msg[3] >> 4) & 0x0F) + (msg[3] & 0x0F);
	cs += ((msg[4] >> 4) & 0x0F) + (msg[4] & 0x0F);
	cs += ((msg[5] >> 4) & 0x0F) + (msg[5] & 0x0F);
	cs += ((msg[6] >> 4) & 0x0F) + (msg[6] & 0x0F);
	cs += ((msg[7] >> 4) & 0x0F) + (msg[7] & 0x0F);
	cs += ((msg[8] >> 4) & 0x0F) + (msg[8] & 0x0F);
	cs += ((msg[9] >> 4) & 0x0F) + (msg[9] & 0x0F);
	cs += (msg[10] & 0x0F);
	cs = (cs - (((msg[11] << 4) & 0xF0) + ((msg[10] >> 4) & 0x0F))) & 0xFF;
	if (cs != 0)
		return FALSE;
	return TRUE;
}

BYTE checksumw(BYTE *msg) {
	BYTE cs = 0;
	
	cs += ((msg[1] >> 4) & 0x0F);
	cs += ((msg[2] >> 4) & 0x0F) + (msg[2] & 0x0F);
	cs += ((msg[3] >> 4) & 0x0F) + (msg[3] & 0x0F);
	cs += ((msg[4] >> 4) & 0x0F) + (msg[4] & 0x0F);
	cs += ((msg[5] >> 4) & 0x0F) + (msg[5] & 0x0F);
	cs += ((msg[6] >> 4) & 0x0F) + (msg[6] & 0x0F);
	cs += (msg[7] & 0x0F);
	cs = (cs - (((msg[8] << 4) & 0xF0) + ((msg[7] >> 4) & 0x0F))) & 0xFF;
	if (cs != 0)
		return FALSE;
	return TRUE;
}

//*************************************************************************
//*************************************************************************
// 							PUBLIC FUNCTIONS SECTION
//*************************************************************************
//*************************************************************************

//*************************************************************************
//	isOregon(BYTE *msg, BYTE *msgtype)
//
//	Return TRUE if the message is an OREGON message, if not return FALSE
//
//	'msg'		Message passed to check
//	'msgtype'	Return type of message
//
//	Returns:
//		TRUE 	if the message passed is a Oregon message
//		FALSE 	if the message is not a Oregon message
//*************************************************************************
int isOregon(BYTE *msg, BYTE *msgtype) {
	
	// Check 56 bit or >59 bit message length for a Oregon message
	if (((msg[0] & 0x7F) == 56) || ((msg[0] & 0x7F) > 59)) {
		// Check for TEMP1 sensor type
		if ((msg[1] == 0x0A) && (msg[2] == 0x4D) && ((msg[0] & 0x7F) >= 72)) {
			*msgtype = RFXCOM_OREGON_MSG_TEMP1;
			return TRUE;
		}
		// Check for TEMP2 sensor type
		else if ((msg[1] == 0xEA) && (msg[2] == 0x4C) && ((msg[0] & 0x7F) >= 60)) {
			*msgtype = RFXCOM_OREGON_MSG_TEMP2;
			return TRUE;
		}
		// Check for TEMP3 sensor type
		else if ((msg[1] == 0xCA) && (msg[2] == 0x48) && ((msg[0] & 0x7F) >= 60)) {
			*msgtype = RFXCOM_OREGON_MSG_TEMP3;
			return TRUE;
		}
		// Check for TEMP4 sensor type
		else if (((msg[1] & 0x0F) == 0x0A) && (msg[2] == 0xDC) && ((msg[0] & 0x7F) >= 64)) {
			*msgtype = RFXCOM_OREGON_MSG_TEMP4;
			return TRUE;
		}
		// Check for TH1 sensor type
		else if ((msg[1] == 0x1A) && (msg[2] == 0x2D) && ((msg[0] & 0x7F) >= 72)) {
			*msgtype = RFXCOM_OREGON_MSG_TH1;
			return TRUE;
		}
		// Check for TH2 sensor type
		else if ((msg[1] == 0xFA) && (msg[2] == 0x28) && ((msg[0] & 0x7F) >= 72)) {
			*msgtype = RFXCOM_OREGON_MSG_TH2;
			return TRUE;
		}
		// Check for TH3 sensor type
		else if (((msg[1] & 0x0F) == 0x0A) && (msg[2] == 0xCC) && ((msg[0] & 0x7F) >= 72)) {
			*msgtype = RFXCOM_OREGON_MSG_TH3;
			return TRUE;
		}
		// Check for TH4 sensor type
		else if ((msg[1] == 0xCA) && (msg[2] == 0x2C) && ((msg[0] & 0x7F) >= 72)) {
			*msgtype = RFXCOM_OREGON_MSG_TH4;
			return TRUE;
		}
		// Check for TH5 sensor type
		else if ((msg[1] == 0xFA) && (msg[2] == 0xB8) && ((msg[0] & 0x7F) >= 72)) {
			*msgtype = RFXCOM_OREGON_MSG_TH5;
			return TRUE;
		}
		// Check for TH6 sensor type
		else if ((msg[1] == 0x1A) && (msg[2] == 0x3D) && ((msg[0] & 0x7F) >= 72)) {
			*msgtype = RFXCOM_OREGON_MSG_TH6;
			return TRUE;
		}
		// Check for THB1 sensor type
		else if ((msg[1] == 0x5A) && (msg[2] == 0x5D) && ((msg[0] & 0x7F) >= 88)) {
			*msgtype = RFXCOM_OREGON_MSG_THB1;
			return TRUE;
		}
		// Check for THB2 sensor type
		else if ((msg[1] == 0x5A) && (msg[2] == 0x6D) && ((msg[0] & 0x7F) >= 88)) {
			*msgtype = RFXCOM_OREGON_MSG_THB2;
			return TRUE;
		}
		// Check for RAIN1 sensor type
		else if ((msg[1] == 0x2A) && (msg[2] == 0x1D) && ((msg[0] & 0x7F) >= 80)) {
			*msgtype = RFXCOM_OREGON_MSG_RAIN1;
			return TRUE;
		}
		// Check for RAIN2 sensor type
		else if ((msg[1] == 0x2A) && (msg[2] == 0x19) && ((msg[0] & 0x7F) >= 84)) {
			*msgtype = RFXCOM_OREGON_MSG_RAIN2;
			return TRUE;
		}
		// Check for RAIN3 sensor type
		else if ((msg[1] == 0x06) && (msg[2] == 0xE4) && ((msg[0] & 0x7F) >= 84)) {
			*msgtype = RFXCOM_OREGON_MSG_RAIN3;
			return TRUE;
		}
		// Check for WIND1 sensor type
		else if ((msg[1] == 0x1A) && (msg[2] == 0x99) && ((msg[0] & 0x7F) >= 80)) {
			*msgtype = RFXCOM_OREGON_MSG_WIND1;
			return TRUE;
		}
		// Check for WIND2 sensor type
		else if ((msg[1] == 0x1A) && (msg[2] == 0x89) && ((msg[0] & 0x7F) >= 80)) {
			*msgtype = RFXCOM_OREGON_MSG_WIND2;
			return TRUE;
		}
		// Check for WIND3 sensor type
		else if ((msg[1] == 0x3A) && (msg[2] == 0x0D) && ((msg[0] & 0x7F) >= 80)) {
			*msgtype = RFXCOM_OREGON_MSG_WIND3;
			return TRUE;
		}
		// Check for UV1 sensor type
		else if ((msg[1] == 0xEA) && (msg[2] == 0x7C) && ((msg[0] & 0x7F) >= 60)) {
			*msgtype = RFXCOM_OREGON_MSG_UV1;
			return TRUE;
		}
		// Check for UV2 sensor type
		else if ((msg[1] == 0xDA) && (msg[2] == 0x78) && ((msg[0] & 0x7F) >= 64)) {
			*msgtype = RFXCOM_OREGON_MSG_UV2;
			return TRUE;
		}
		// Check for DT1 sensor type
		else if (((msg[1] & 0x0F) == 0x0A) && (msg[2] == 0xEC) && ((msg[0] & 0x7F) >= 96)) {
			*msgtype = RFXCOM_OREGON_MSG_DT1;
			return TRUE;
		}
		// Check for WEIGHT1 sensor type
		else if ((msg[0] & 0x7F) == 56) {
			*msgtype = RFXCOM_OREGON_MSG_WEIGHT1;
			return TRUE;
		}
		// Check for WEIGHT2 sensor type
		else if (((msg[1] & 0x0F) == 0x03) && ((msg[0] & 0x7F) == 64)) {
			*msgtype = RFXCOM_OREGON_MSG_WEIGHT2;
			return TRUE;
		}
		// Check for ELEC1 sensor type
		else if ((msg[1] == 0xEA) && ((msg[2] & 0xC0) == 0x00) && ((msg[0] & 0x7F) >= 64)) {
			*msgtype = RFXCOM_OREGON_MSG_ELEC1;
			return TRUE;
		}
		// Check for ELEC2 sensor type
		else if (((msg[1] == 0x1A) || (msg[1] == 0x2A) || (msg[1] == 0x3A)) && ((msg[0] & 0x7F) == 108)) {
			*msgtype = RFXCOM_OREGON_MSG_ELEC2;
			return TRUE;
		}
	}
	*msgtype = RFXCOM_OREGON_MSG_UNKNOWN;
	return FALSE;
}


//*************************************************************************
//	oreSupport(int cap, int devcaps)
//
//	Returns true if the sensor devcaps given support for capability given
//
//	'cap'		Capability to check support
//	'devcaps'	Device capabilities of the sensor
//
//	Returns:
//		TRUE 	The sensor support the capability
//		FALSE	The sensor not support the capability
//*************************************************************************
int oreSupport(BYTE cap, int devcaps) {
	int j;
	int pow_result = 1;
	
	// Calculate 2^i
	for(j=0; j < cap; j++)
		pow_result = pow_result * 2;
	if (devcaps & pow_result)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	oreGetSensorNumEndpoints(int devcaps)
//
//	Returns the number of endpoints for the sensor devcaps given
//
//	'devcaps'	Device capabilities of the sensor
//
//	Returns:
//		The number of endpoints for the sensor given
//
//*************************************************************************
int oreGetSensorNumEndpoints(int devcaps) {
	int i, j;
	int result = 0;
	
	for (i=0; i<16; i++) {
		// Calculate 2^i
		int pow_result = 1;
		for(j=0; j < i; j++)
			pow_result = pow_result * 2;
		if (devcaps & pow_result)
			result++;
	}
	
	return result;
}


//*************************************************************************
//	oreGetSensorEndpointSubUID(BYTE endpoint, int devcaps)
//
//	Returns the subuid in base 0 for the endpoint and sensor devcaps given
//
//	'endpoint'	Endpoint
//	'devcaps'	Device capabilities of the sensor
//
//	Returns:
//		The subuid in base 0 for the endpoint and sensor devcaps given
//
//*************************************************************************
int oreGetSensorEndpointSubUID(BYTE endpoint, int devcaps) {
	int i, j;
	int result = -1;

	for(i=0; i <= endpoint; i++) {
		// Calculate 2^i
		int pow_result = 1;
		for(j=0; j < i; j++)
			pow_result = pow_result * 2;
		// Check with devcaps if sensor support this endpoint
		if (devcaps & pow_result)
			result++;
	}
	return result;
}


//*************************************************************************
//	decode_oregon_temp(BYTE *msg, BYTE msgType, char *addr, float *temp, BYTE *bat)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'msgType'	Message type (TEMP1..TEMP4)
//	'addr'		Address of the sensor in ASCII format extracted from the message
//	'temp'		Temperature of the sensor extracted from the message
//	'bat'		Battery status of the sensor extracted from the message
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_oregon_temp(BYTE *msg, BYTE msgType, char *addr, float *temp, BYTE *bat) {
	BYTE ch;
	
	// Extract channel
	switch(msgType) {
		case RFXCOM_OREGON_MSG_TEMP1:
		case RFXCOM_OREGON_MSG_TEMP2:
		case RFXCOM_OREGON_MSG_TEMP3:
			ch = wrchannel(msg[3]);
			break;
			
		case RFXCOM_OREGON_MSG_TEMP4:
			ch = wrchannel3(msg[3]);
			break;
			
		default:
			return FALSE;
	}
	
	// Extract Address TTAAAAC (Type + Address + Channel)
	addr[0] = ((msgType >> 4) & 0x0F) < 10 ? ((msgType >> 4) & 0x0F) + 48 : ((msgType >> 4) & 0x0F) + 55;
	addr[1] = (msgType & 0x0F) < 10 ? (msgType & 0x0F) + 48 : (msgType & 0x0F) + 55;
	addr[2] = '0';
	addr[3] = '0';
	addr[4] = ((msg[4] >> 4) & 0x0F) < 10 ? ((msg[4] >> 4) & 0x0F) + 48 : ((msg[4] >> 4) & 0x0F) + 55;
	addr[5] = (msg[4] & 0x0F) < 10 ? (msg[4] & 0x0F) + 48 : (msg[4] & 0x0F) + 55;
	addr[6] = ch + 48;
	addr[7] = '\0';
	
	// Extract temp
	switch(msgType) {
		case RFXCOM_OREGON_MSG_TEMP1:
			if (((msg[6] & 0xF0) < 0xA0) && ((msg[6] & 0x0F) < 0x0A) && ((msg[5] & 0xF0) < 0xA0)) {
				if ((msg[7] & 0x08) == 0)
					*temp = (((msg[6] & 0xF0) >> 4) * 10) + (msg[6] & 0x0F) + (((msg[5] & 0xF0) >> 4) / 10.0);
				else
					*temp = 0 - ((((msg[6] & 0xF0) >> 4) * 10) + (msg[6] & 0x0F) + (((msg[5] & 0xF0) >> 4) / 10.0));
			}
			else
				return FALSE;
			break;
			
		case RFXCOM_OREGON_MSG_TEMP2:
			if ((msg[7] & 0x08) == 0)
				*temp = ((msg[7] & 0x03) * 100) + (((msg[6] & 0xF0) >> 4) * 10) + (msg[6] & 0x0F) + (((msg[5] & 0xF0) >> 4) / 10.0);
			else
				*temp = 0 - (((msg[7] & 0x03) * 100) + (((msg[6] & 0xF0) >> 4) * 10) + (msg[6] & 0x0F) + (((msg[5] & 0xF0) >> 4) / 10.0));
			break;
		
		case RFXCOM_OREGON_MSG_TEMP3:
		case RFXCOM_OREGON_MSG_TEMP4:
			if ((msg[7] & 0x08) == 0)
				*temp = (((msg[6] & 0xF0) >> 4) * 10) + (msg[6] & 0x0F) + (((msg[5] & 0xF0) >> 4) / 10.0);
			else
				*temp = 0 - ((((msg[6] & 0xF0) >> 4) * 10) + (msg[6] & 0x0F) + (((msg[5] & 0xF0) >> 4) / 10.0));
			break;
			
		default:
			return FALSE;
	}
		
	// Extract battery indication
    *bat = wbattery_indication(msg[5]);
    
    // Check for checksum
    if (!checksum8(msg))
		return FALSE;
	
	return TRUE;
}


//*************************************************************************
//	decode_oregon_th(BYTE *msg, BYTE msgType, char *addr, float *temp, float *hum, BYTE *bat)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'msgType'	Message type (TH1..TH6)
//	'addr'		Address of the sensor in ASCII format extracted from the message
//	'temp'		Temperature of the sensor extracted from the message
//	'hum'		Humidity of the sensor extracted from the message
//	'bat'		Battery status of the sensor extracted from the message
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_oregon_th(BYTE *msg, BYTE msgType, char *addr, float *temp, float *hum, BYTE *bat) {
	
	BYTE ch;
	
	// Extract channel
	switch(msgType) {
		case RFXCOM_OREGON_MSG_TH1:
		case RFXCOM_OREGON_MSG_TH4:
		case RFXCOM_OREGON_MSG_TH6:
			ch = wrchannel(msg[3]);
			break;
			
		case RFXCOM_OREGON_MSG_TH2:
		case RFXCOM_OREGON_MSG_TH3:
			ch = wrchannel3(msg[3]);
			break;
			
		case RFXCOM_OREGON_MSG_TH5:
			ch = 0;
			break;
			
		default:
			return FALSE;
	}
	
	// Extract Address TTAAAAC (Type + Address + Channel)
	addr[0] = ((msgType >> 4) & 0x0F) < 10 ? ((msgType >> 4) & 0x0F) + 48 : ((msgType >> 4) & 0x0F) + 55;
	addr[1] = (msgType & 0x0F) < 10 ? (msgType & 0x0F) + 48 : (msgType & 0x0F) + 55;
	addr[2] = '0';
	addr[3] = '0';
	addr[4] = ((msg[4] >> 4) & 0x0F) < 10 ? ((msg[4] >> 4) & 0x0F) + 48 : ((msg[4] >> 4) & 0x0F) + 55;
	addr[5] = (msg[4] & 0x0F) < 10 ? (msg[4] & 0x0F) + 48 : (msg[4] & 0x0F) + 55;
	addr[6] = ch + 48;
	addr[7] = '\0';
	
	// Extract temp
	if ((msg[7] & 0x08) == 0)
		*temp = (((msg[6] & 0xF0) >> 4) * 10) + (msg[6] & 0x0F) + (((msg[5] & 0xF0) >> 4) / 10.0);
	else
		*temp = 0 - ((((msg[6] & 0xF0) >> 4) * 10) + (msg[6] & 0x0F) + (((msg[5] & 0xF0) >> 4) / 10.0));

	// Extract humidity
	 *hum = ((msg[8] & 0x0F) * 10) + ((msg[7] >> 4) & 0x0F);
			
	// Extract battery indication
	switch(msgType) {
		case RFXCOM_OREGON_MSG_TH1:
		case RFXCOM_OREGON_MSG_TH2:
		case RFXCOM_OREGON_MSG_TH3:
		case RFXCOM_OREGON_MSG_TH4:
		case RFXCOM_OREGON_MSG_TH5:
			*bat = wbattery_indication(msg[5]);
			break;
			
		case RFXCOM_OREGON_MSG_TH6:
			*bat = wrbattery(msg[5]);
			break;

		default:
			return FALSE;
	}
    
    // Check for checksum
    if (!checksum8(msg))
		return FALSE;
	
	return TRUE;
}


//*************************************************************************
//	decode_oregon_thb(BYTE *msg, BYTE msgType, char *addr, float *temp, float *hum, float *baro, BYTE *bat)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'msgType'	Message type (THB1, THB2)
//	'addr'		Address of the sensor in ASCII format extracted from the message
//	'temp'		Temperature of the sensor extracted from the message
//	'hum'		Humidity of the sensor extracted from the message
//	'baro'		Pressure of the sensor extracted from the message
//	'bat'		Battery status of the sensor extracted from the message
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_oregon_thb(BYTE *msg, BYTE msgType, char *addr, float *temp, float *hum, float *baro, BYTE *bat) {
	BYTE ch;
	
	// Extract channel
	switch(msgType) {
		case RFXCOM_OREGON_MSG_THB1:
		case RFXCOM_OREGON_MSG_THB2:
			ch = wrchannel(msg[3]);
			break;
			
		default:
			return FALSE;
	}
	
	// Extract Address TTAAAAC (Type + Address + Channel)
	addr[0] = ((msgType >> 4) & 0x0F) < 10 ? ((msgType >> 4) & 0x0F) + 48 : ((msgType >> 4) & 0x0F) + 55;
	addr[1] = (msgType & 0x0F) < 10 ? (msgType & 0x0F) + 48 : (msgType & 0x0F) + 55;
	addr[2] = '0';
	addr[3] = '0';
	addr[4] = ((msg[4] >> 4) & 0x0F) < 10 ? ((msg[4] >> 4) & 0x0F) + 48 : ((msg[4] >> 4) & 0x0F) + 55;
	addr[5] = (msg[4] & 0x0F) < 10 ? (msg[4] & 0x0F) + 48 : (msg[4] & 0x0F) + 55;
	addr[6] = ch + 48;
	addr[7] = '\0';
	
	// Extract temp
	if ((msg[7] & 0x08) == 0)
		*temp = (((msg[6] & 0xF0) >> 4) * 10) + (msg[6] & 0x0F) + (((msg[5] & 0xF0) >> 4) / 10.0);
	else
		*temp = 0 - ((((msg[6] & 0xF0) >> 4) * 10) + (msg[6] & 0x0F) + (((msg[5] & 0xF0) >> 4) / 10.0));

	// Extract humidity
	*hum = ((msg[8] & 0x0F) * 10) + ((msg[7] >> 4) & 0x0F);
	 
	// Extract pressure (hPA)
	switch(msgType) {
		case RFXCOM_OREGON_MSG_THB1:
			*baro = msg[9] + 795;
			break;
			
		case RFXCOM_OREGON_MSG_THB2:
			*baro = msg[9] + 856;
			break;
			
		default:
			return FALSE;
	}
			
	// Extract battery indication
	switch(msgType) {
		case RFXCOM_OREGON_MSG_THB1:
		case RFXCOM_OREGON_MSG_THB2:
			*bat = wbattery_indication(msg[5]);
			break;
			
		default:
			return FALSE;
	}
    
    // Check for checksum
    if (!checksum10(msg))
		return FALSE;
	
	return TRUE;
}


//*************************************************************************
//	decode_oregon_rain(BYTE *msg, BYTE msgType, char *addr, float *rrate, float *rtotal, BYTE *bat)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'msgType'	Message type (TEMP1..TEMP4)
//	'addr'		Address of the sensor in ASCII format extracted from the message
//	'rrate'		Rain fall rate measured by the sensor extracted from the message
//	'rtotal'	Total rain measured by the sensor extracted from the message
//	'bat'		Battery status of the sensor extracted from the message
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_oregon_rain(BYTE *msg, BYTE msgType, char *addr, float *rrate, float *rtotal, BYTE *bat) {

	// Extract Address TTAAAAC (Type + Address + Channel)
	addr[0] = ((msgType >> 4) & 0x0F) < 10 ? ((msgType >> 4) & 0x0F) + 48 : ((msgType >> 4) & 0x0F) + 55;
	addr[1] = (msgType & 0x0F) < 10 ? (msgType & 0x0F) + 48 : (msgType & 0x0F) + 55;
	addr[2] = '0';
	addr[3] = '0';
	addr[4] = ((msg[4] >> 4) & 0x0F) < 10 ? ((msg[4] >> 4) & 0x0F) + 48 : ((msg[4] >> 4) & 0x0F) + 55;
	addr[5] = (msg[4] & 0x0F) < 10 ? (msg[4] & 0x0F) + 48 : (msg[4] & 0x0F) + 55;
	addr[6] = '0';
	addr[7] = '\0';
	
	// Extract rain fall rate (mm/hr) and total rain count (mm)
	switch(msgType) {
		case RFXCOM_OREGON_MSG_RAIN1:
			*rrate = (((msg[6] >> 4) & 0x0F) * 100) + ((msg[6] & 0x0F) * 10) + ((msg[5] >> 4) & 0x0F);
			*rtotal = ((msg[9] & 0x0F) * 1000) + (((msg[8] >> 4) & 0x0F) * 100) + ((msg[8] & 0x0F) * 10) + ((msg[7] >> 4) & 0x0F);
			break;
			
		case RFXCOM_OREGON_MSG_RAIN2:
		case RFXCOM_OREGON_MSG_RAIN3:
			*rrate = (((msg[6] >> 4) & 0x0F) + ((msg[6] & 0x0F) / 10.0) + (((msg[5] >> 4) & 0x0F) / 100.0) + ((msg[7] & 0x0F) / 1000.0)) * 25.4;
			*rtotal = ((msg[10] & 0x0F) * 100) + (((msg[9] >> 4) & 0x0F) * 10) + (msg[9] & 0x0F);
			*rtotal += (((msg[8] >> 4) & 0x0F) / 10.0) + ((msg[8] & 0x0F) / 100.0) + (((msg[7] >> 4) & 0x0F)/ 1000.0);
			*rtotal *= 25.4;
			break;
		
		default:
			return FALSE;
	}
		
	// Extract battery indication
    *bat = wbattery_indication(msg[5]);
    
    // Check for checksum
    switch(msgType) {
		case RFXCOM_OREGON_MSG_RAIN1:
			if (!checksum2(msg))
				return FALSE;
			break;
			
		case RFXCOM_OREGON_MSG_RAIN2:
		case RFXCOM_OREGON_MSG_RAIN3:
			if (!checksumr(msg))
				return FALSE;
			break;
		
		default:
			return FALSE;
	}
	
	return TRUE;
}


//*************************************************************************
//	decode_oregon_wind(BYTE *msg, BYTE msgType, char *addr, float *wdir, float *wspeed, float *wavgspeed, BYTE *bat)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'msgType'	Message type (WIND1..WIND3)
//	'addr'		Address of the sensor in ASCII format extracted from the message
//	'wdir'		Wind direction measured by the sensor extracted from the message
//	'wspeed'	Wind speed measured by the sensor extracted from the message
//	'wavgspeed'	Wind average speed measured by the sensor extracted from the message
//	'bat'		Battery status of the sensor extracted from the message
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_oregon_wind(BYTE *msg, BYTE msgType, char *addr, float *wdir, float *wspeed, float *wavgspeed, BYTE *bat) {
	
	// Extract Address TTAAAAC (Type + Address + Channel)
	addr[0] = ((msgType >> 4) & 0x0F) < 10 ? ((msgType >> 4) & 0x0F) + 48 : ((msgType >> 4) & 0x0F) + 55;
	addr[1] = (msgType & 0x0F) < 10 ? (msgType & 0x0F) + 48 : (msgType & 0x0F) + 55;
	addr[2] = '0';
	addr[3] = '0';
	addr[4] = ((msg[4] >> 4) & 0x0F) < 10 ? ((msg[4] >> 4) & 0x0F) + 48 : ((msg[4] >> 4) & 0x0F) + 55;
	addr[5] = (msg[4] & 0x0F) < 10 ? (msg[4] & 0x0F) + 48 : (msg[4] & 0x0F) + 55;
	addr[6] = '0';
	addr[7] = '\0';
	
	// Extract wind direction (º)
	switch(msgType) {
		case RFXCOM_OREGON_MSG_WIND1:
		case RFXCOM_OREGON_MSG_WIND2:
			*wdir = (msg[5] >> 4) * 22.5;
			break;
			
		case RFXCOM_OREGON_MSG_WIND3:
			*wdir = (((msg[6] >> 4) & 0x0F) * 100) + ((msg[6] & 0x0F) * 10) + ((msg[5] >> 4) & 0x0F);
			break;
			
		default:
			return FALSE;
	}
	
	// Extract wind speed (m/s)
	*wspeed = ((msg[8] & 0x0F) * 10) + ((msg[7] >> 4) & 0x0F) + ((msg[7] & 0x0F) / 10.0);
	
	// Extract wind average speed (m/s)
	*wavgspeed = (((msg[9] >> 4) & 0x0F) * 10.0) + (msg[9] & 0x0F) + (((msg[8] >> 4) & 0x0F) / 10.0);

	// Extract battery indication
	switch(msgType) {
		case RFXCOM_OREGON_MSG_WIND1:
		case RFXCOM_OREGON_MSG_WIND2:
			*bat = wbattery_indication(msg[5]);
			break;
			
		case RFXCOM_OREGON_MSG_WIND3:
			*bat = wrbattery(msg[5]);
			break;
					
		default:
			return FALSE;
	}
        
    // Check for checksum
    if (!checksum9(msg))
		return FALSE;
	
	return TRUE;
}


//*************************************************************************
//	decode_oregon_uv(BYTE *msg, BYTE msgType, char *addr, float *uvfactor, BYTE *bat)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'msgType'	Message type (UV1, UV2)
//	'addr'		Address of the sensor in ASCII format extracted from the message
//	'uvfactor'	UV factor measured by the sensor extracted from the message
//	'bat'		Battery status of the sensor extracted from the message
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_oregon_uv(BYTE *msg, BYTE msgType, char *addr, float *uvfactor, BYTE *bat) {
		
	// Extract Address TTAAAAC (Type + Address + Channel)
	addr[0] = ((msgType >> 4) & 0x0F) < 10 ? ((msgType >> 4) & 0x0F) + 48 : ((msgType >> 4) & 0x0F) + 55;
	addr[1] = (msgType & 0x0F) < 10 ? (msgType & 0x0F) + 48 : (msgType & 0x0F) + 55;
	addr[2] = '0';
	addr[3] = '0';
	addr[4] = ((msg[4] >> 4) & 0x0F) < 10 ? ((msg[4] >> 4) & 0x0F) + 48 : ((msg[4] >> 4) & 0x0F) + 55;
	addr[5] = (msg[4] & 0x0F) < 10 ? (msg[4] & 0x0F) + 48 : (msg[4] & 0x0F) + 55;
	addr[6] = '0';
	addr[7] = '\0';
	
	// Extract UV factor
	switch(msgType) {
		case RFXCOM_OREGON_MSG_UV1:
			*uvfactor = ((msg[6] & 0x0F) * 10) + ((msg[5] >> 4) & 0x0F);
			break;
			
		case RFXCOM_OREGON_MSG_UV2:
			*uvfactor = ((msg[5] >> 4) & 0x0F);
			break;
			
		default:
			return FALSE;
	}
	
	// Extract battery indication
	*bat = wbattery_indication(msg[5]);
        
    // Check for checksum
    switch(msgType) {
		case RFXCOM_OREGON_MSG_UV1:
			if (!checksumw(msg))
				return FALSE;
			break;
			
		case RFXCOM_OREGON_MSG_UV2:
			if (!checksum7(msg))
				return FALSE;
			break;
			
		default:
			return FALSE;
	}
	
	return TRUE;
}


//*************************************************************************
//	decode_oregon_weight(BYTE *msg, BYTE msgType, char *addr, float *weight)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'msgType'	Message type (WEIGHT1, WEIGHT2)
//	'addr'		Address of the sensor in ASCII format extracted from the message
//	'weight'	Weight measured by the sensor extracted from the message
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_oregon_weight(BYTE *msg, BYTE msgType, char *addr, float *weight) {
	
	// Extract Address TTAAAAC (Type + Address + Channel)
	addr[0] = ((msgType >> 4) & 0x0F) < 10 ? ((msgType >> 4) & 0x0F) + 48 : ((msgType >> 4) & 0x0F) + 55;
	addr[1] = (msgType & 0x0F) < 10 ? (msgType & 0x0F) + 48 : (msgType & 0x0F) + 55;
	addr[2] = '0';
	addr[3] = '0';
	addr[4] = '0';
	addr[5] = ((msg[2] >> 4) & 0x0F) < 10 ? ((msg[2] >> 4) & 0x0F) + 48 : ((msg[2] >> 4) & 0x0F) + 55;
	addr[6] = '0';
	addr[7] = '\0';
	
	// Extract Weight (Kg)
	switch(msgType) {
		case RFXCOM_OREGON_MSG_WEIGHT1:
			*weight = ((msg[6] & 0x01) * 100) + (((msg[5] >> 4) & 0x0F) * 10) + (msg[5] & 0x0F) + (((msg[4] >> 4) & 0x0F) / 10.0);
			break;
			
		case RFXCOM_OREGON_MSG_WEIGHT2:
			*weight = (((msg[5] & 0x0F) * 4096) + (msg[4] * 16) + ((msg[3] >> 4) & 0x0F)) / 400.8;
			break;
			
		default:
			return FALSE;
	}
	
    // Check for checksum
    switch(msgType) {
		case RFXCOM_OREGON_MSG_WEIGHT1:
			if (!(((msg[1] & 0xF0) == (msg[6] & 0xF0)) && ((msg[2] & 0x0F) == (msg[7] & 0x0F))))
				return FALSE;
			break;
			
		case RFXCOM_OREGON_MSG_WEIGHT2:
			break;
			
		default:
			return FALSE;
	}
	
	return TRUE;
}


//*************************************************************************
//	decode_oregon_elec(BYTE *msg, BYTE msgType, char *addr, float *ct1, float *ct2, float *ct3, BYTE *bat)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'msgType'	Message type (ELEC1, ELEC2)
//	'addr'		Address of the sensor in ASCII format extracted from the message
//	'ct1'		Current measured on channel 1 of the sensor extracted from the message
//	'ct2'		Current measured on channel 2 of the sensor extracted from the message
//	'ct3'		Current measured on channel 3 of the sensor extracted from the message
//	'bat'		Battery status of the sensor extracted from the message
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_oregon_elec(BYTE *msg, BYTE msgType, char *addr, float *ct1, float *ct2, float *ct3, BYTE *bat) {
	
	// Extract Address TTAAAAC (Type + Address + Counter)
	switch(msgType) {
		case RFXCOM_OREGON_MSG_ELEC1:
			addr[0] = ((msgType >> 4) & 0x0F) < 10 ? ((msgType >> 4) & 0x0F) + 48 : ((msgType >> 4) & 0x0F) + 55;
			addr[1] = (msgType & 0x0F) < 10 ? (msgType & 0x0F) + 48 : (msgType & 0x0F) + 55;
			addr[2] = '0';
			addr[3] = '0';
			addr[4] = ((msg[3] >> 4) & 0x0F) < 10 ? ((msg[3] >> 4) & 0x0F) + 48 : ((msg[3] >> 4) & 0x0F) + 55;
			addr[5] = (msg[3] & 0x0F) < 10 ? (msg[3] & 0x0F) + 48 : (msg[3] & 0x0F) + 55;
			addr[6] = (msg[2] & 0x0F) + 48;
			addr[7] = '\0';
			break;
			
		case RFXCOM_OREGON_MSG_ELEC2:
			addr[0] = ((msgType >> 4) & 0x0F) < 10 ? ((msgType >> 4) & 0x0F) + 48 : ((msgType >> 4) & 0x0F) + 55;
			addr[1] = (msgType & 0x0F) < 10 ? (msgType & 0x0F) + 48 : (msgType & 0x0F) + 55;
			addr[2] = (msg[4] & 0x0F) < 10 ? (msg[4] & 0x0F) + 48 : (msg[4] & 0x0F) + 55;
			addr[3] = ((msg[3] >> 4) & 0x0F) < 10 ? ((msg[3] >> 4) & 0x0F) + 48 : ((msg[3] >> 4) & 0x0F) + 55;
			addr[4] = (msg[3] & 0x0F) < 10 ? (msg[3] & 0x0F) + 48 : (msg[3] & 0x0F) + 55;
			addr[5] = ((msg[1] >> 4) & 0x03) < 10 ? ((msg[1] >> 4) & 0x03) + 48 : ((msg[1] >> 4) & 0x03) + 55;
			addr[6] = (msg[2] & 0x0F) + 48;
			addr[7] = '\0';
			break;
			
		default:
			return FALSE;
	}
		
	// Extract Current measurement (Amps or kW)
	switch(msgType) {
		case RFXCOM_OREGON_MSG_ELEC1: // Amps
			*ct1 = (msg[4] + ((msg[5] & 0x03) * 256)) / 10.0;
			*ct2 = (((msg[5] >> 2) & 0x3F) + ((msg[6] & 0x0F) * 64)) / 10.0;
			*ct3 = (((msg[6] >> 4) & 0x0F) + ((msg[7] & 0x3F) * 16)) / 10.0;
			break;
			
		case RFXCOM_OREGON_MSG_ELEC2: // kW
            *ct1 = (((msg[6] & 0x0F) * 65536) + (msg[5] * 256) + (msg[4] & 0xF0)) / 994.0;
            *ct2 = 0;
            *ct3 = 0;
			break;
			
		default:
			return FALSE;
	}
			
	// Extract battery indication
	if ((msg[2] & 0x10) == 0)
		*bat = RFXCOM_OREGON_BAT_OK;
	else
		*bat = RFXCOM_OREGON_BAT_LOW;
    
    // Check for checksum
    switch(msgType) {
		case RFXCOM_OREGON_MSG_ELEC1:
			if (!checksume(msg))
				return FALSE;
			break;
			
		case RFXCOM_OREGON_MSG_ELEC2:
			if (!checksum12(msg))
				return FALSE;
			break;
			
		default:
			return FALSE;
	}
	
	return TRUE;
}


//*************************************************************************
//	decode_oregon_dt(BYTE *msg, BYTE msgType, char *addr, float *date, float *time, float *day)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'msgType'	Message type (DT1)
//	'addr'		Address of the sensor in ASCII format extracted from the message
//	'date'		Date extracted from the message
//	'time'		Time extracted from the message
//	'day'		Day of week extracted from the message
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_oregon_dt(BYTE *msg, BYTE msgType, char *addr, float *date, float *time, float *day) {
	BYTE ch;
	
	// Extract channel
	switch(msgType) {
		case RFXCOM_OREGON_MSG_DT1:
			ch = wrchannel3(msg[3]);
			break;
			
		default:
			return FALSE;
	}
	
	// Extract Address TTAAAAC (Type + Address + Channel)
	switch(msgType) {
		case RFXCOM_OREGON_MSG_DT1:
			addr[0] = ((msgType >> 4) & 0x0F) < 10 ? ((msgType >> 4) & 0x0F) + 48 : ((msgType >> 4) & 0x0F) + 55;
			addr[1] = (msgType & 0x0F) < 10 ? (msgType & 0x0F) + 48 : (msgType & 0x0F) + 55;
			addr[2] = '0';
			addr[3] = '0';
			addr[4] = ((msg[4] >> 4) & 0x0F) < 10 ? ((msg[4] >> 4) & 0x0F) + 48 : ((msg[4] >> 4) & 0x0F) + 55;
			addr[5] = (msg[4] & 0x0F) < 10 ? (msg[4] & 0x0F) + 48 : (msg[4] & 0x0F) + 55;
			addr[6] = ch + 48;
			addr[7] = '\0';
			break;
			
		default:
			return FALSE;
	}
		
	// Extract Date/Time/Day
	switch(msgType) {
		case RFXCOM_OREGON_MSG_DT1:
			// Date format ddmmyy
			*date = (((msg[9] & 0x0F) * 10) + ((msg[8] >> 4) & 0x0F)) * 10000;	// dd
			*date += ((msg[9] >> 4) & 0x0F) * 100; 								// mm
			*date += ((msg[11] & 0x0F) * 10) + ((msg[10] >> 4) & 0x0F); 		// yy
			
			// Time format hhmmss
			*time = (((msg[8] & 0x0F) * 10) + ((msg[7] >> 4) & 0x0F)) * 10000; 	// hr
			*time += (((msg[7] & 0x0F) * 10) + ((msg[6] >> 4) & 0x0F)) * 100; 	// min
			*time += ((msg[6] & 0x0F) * 10) + ((msg[5] >> 4) & 0x0F); 			// sec
			
			// Day (0=Sunday, 6=Saturday)
			if ((*day = (msg[10] & 0x07)) > 6)
				return FALSE;
			break;
			
		default:
			return FALSE;
	}
			
	// Check for checksum
    switch(msgType) {
		case RFXCOM_OREGON_MSG_DT1:
			if (!checksum11(msg))
				return FALSE;
			break;
			
		default:
			return FALSE;
	}
	
	return TRUE;
}
