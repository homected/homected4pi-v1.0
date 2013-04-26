/**************************************************************************

	rfxsensor.c

	Functions for Rfxsensor and Rfxmeter devices

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


	Last changes:

	03/02/13 by Jose Luis Galindo : First version
	
***************************************************************************/

//*************************************************************************
//*************************************************************************
// 								INCLUDE SECTION
//*************************************************************************
//*************************************************************************

#include "rfxsensor.h"


//*************************************************************************
//*************************************************************************
// 							PRIVATE FUNCTIONS SECTION
//*************************************************************************
//*************************************************************************

BYTE crc8(BYTE *msg) {
	BYTE result, temp2, temp4;
	int intBytes, intBits;
		
	result = 0;
	for (intBytes = 1; intBytes < 6; intBytes++) {
		temp2 = msg[intBytes];
		for (intBits = 0; intBits < 8; intBits++) {
			temp4 = (temp2 ^ result) & 0x01;
			result = result >> 1;
			if (temp4 == 0x01)
				result = result ^ 0x8C;
			temp2 = temp2 >> 1;
		}
	}
	return result;
}


//*************************************************************************
//*************************************************************************
// 							PUBLIC FUNCTIONS SECTION
//*************************************************************************
//*************************************************************************

//*************************************************************************
//	isRfxSensor(BYTE *msg, BYTE *msgtype)
//
//	Return TRUE if the message passed is a Rfxsensor message, if not return FALSE
//
//	'msg'		Message passed to check
//	'msgtype'	Return type of message
//
//	Returns:
//		TRUE 	if the message passed is a Rfxsensor message
//		FALSE 	if the message is not a Rfxsensor message
//*************************************************************************
int isRfxSensor(BYTE *msg, BYTE *msgtype) {
	int parity;
	
	// Check 32 bit for a Rfxsensor message
	if ((msg[0] & 0x7F) == 32) {
		parity = ~(((msg[1] >> 4) & 0x0F) + (msg[1] & 0x0F) + ((msg[2] >> 4) & 0x0F) + (msg[2] & 0x0F) +
			((msg[3] >> 4) & 0x0F) + (msg[3] & 0x0F) + ((msg[4] >> 4) & 0x0F)) & 0x0F;
		if ((parity = (msg[4] & 0x0F)) && ((msg[1] + (msg[2] ^ 0x0F)) == 0xFF)) {
			*msgtype = RFXCOM_RFXSENSOR_MSG_RFXSENSOR;
			return TRUE;
		}
	}
	// Check 32 bit for a RFXMeter or RFXPower or RFXSensorL message
	else if ((msg[0] & 0x7F) == 48) {
		if (crc8(msg) == msg[6]) {
			*msgtype = RFXCOM_RFXSENSOR_MSG_RFXSENSOR;
			return TRUE;
		}
		else if (msg[1] == (msg[2] ^ 0xF0)) {
			parity = ~(((msg[1] >> 4) & 0x0F) + (msg[1] & 0x0F) + ((msg[2] >> 4) & 0x0F) + (msg[2] & 0x0F) +
			((msg[3] >> 4) & 0x0F) + (msg[3] & 0x0F) + ((msg[4] >> 4) & 0x0F) + (msg[4] & 0x0F) +
			((msg[5] >> 4) & 0x0F) + (msg[5] & 0x0F) + ((msg[6] >> 4) & 0x0F)) & 0x0F;
			if ((parity = (msg[6] & 0x0F)) && ((msg[1] + (msg[2] ^ 0x0F)) == 0xFF)) {
				*msgtype = RFXCOM_RFXSENSOR_MSG_RFXPOWER;
				return TRUE;
			}
			else if (parity == (msg[6] & 0x0F)) {
				*msgtype = RFXCOM_RFXSENSOR_MSG_SRFXPOWER;
				return TRUE;
			}
		}
	}
	return FALSE;
}


//*************************************************************************
//	rfxGetSensorNumEndpoints(int devcaps)
//
//	Returns the number of endpoints for the sensor devcaps given
//
//	'devcaps'	Device capabilities of the sensor
//
//	Returns:
//		The number of endpoints for the sensor given
//
//*************************************************************************
int rfxGetSensorNumEndpoints(int devcaps) {
	
	if (devcaps & RFXCOM_RFXSENSOR_DCAPS_MULTI)
		return 3;
	if (devcaps & RFXCOM_RFXSENSOR_DCAPS_MULTI_AD)
		return 3;
	else
		return 1;
}


//*************************************************************************
//	decode_rfxsensor(BYTE *msg, char *addr, char *addrcount, BYTE *func, float *value1, float *value2, float *value3)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'addr'		Address of the sensor in ASCII format extracted from the message
//	'addrcount'	Number of entries in address list
//	'func'		Function extracted from the message
//	'value1'	Value for the function
//	'value2'	Value for the function
//	'value3'	Value for the function
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_rfxsensor(BYTE *msg, char *addr, BYTE *func, float *value1, float *value2, float *value3) {
	float measured_value;
	
	// Init vars
	*value1 = 0;
	*value2 = 0;
	*value3 = 0;
	
	// For 32 bit messages
	if ((msg[0] & 0x7F) == 32) {
		// Extract Address 
		addr[0] = ((msg[1] >> 4) & 0x0F) < 10 ? ((msg[1] >> 4) & 0x0F) + 48 : ((msg[1] >> 4) & 0x0F) + 55;
		addr[1] = (msg[1] & 0x0F) < 10 ? (msg[1] & 0x0F) + 48 : (msg[1] & 0x0F) + 55;
		addr[2] = ((msg[2] >> 4) & 0x0F) < 10 ? ((msg[2] >> 4) & 0x0F) + 48 : ((msg[2] >> 4) & 0x0F) + 55;
		addr[3] = (msg[2] & 0x0F) < 10 ? (msg[2] & 0x0F) + 48 : (msg[2] & 0x0F) + 55;
		addr[4] = '\0';
		
		// Check for errors
		if ((msg[4] & 0x10) != 0) {
			switch(msg[3]) {
				case 0x01:	// Info: address incremented
					*func = RFXCOM_RFXSENSOR_FUNC_INFO;
					return TRUE;
					break;
					
				case 0x02:	// Info: battery low
					*func = RFXCOM_RFXSENSOR_FUNC_INFO;
					return TRUE;
					break;
					
				case 0x03:	// Info: conversion not ready, 1 retry is done
					*func = RFXCOM_RFXSENSOR_FUNC_INFO;
					return TRUE;
					break;
					
				case 0x81:	// Error: No 1-Wire device connected
					*func = RFXCOM_RFXSENSOR_FUNC_ERROR;
					return TRUE;
					break;
					
				case 0x82:	// Error: 1-Wire ROM CRC error
					*func = RFXCOM_RFXSENSOR_FUNC_ERROR;
					return TRUE;
					break;
					
				case 0x83:	// Error: 1-Wire device connected is not a DS1820
					*func = RFXCOM_RFXSENSOR_FUNC_ERROR;
					return TRUE;
					break;
					
				case 0x84:	// Error: No end of read signal received from 1-Wire device
					*func = RFXCOM_RFXSENSOR_FUNC_ERROR;
					return TRUE;
					break;
					
				case 0x85:	// Error: 1-Wire device Scratchpad CRC error
					*func = RFXCOM_RFXSENSOR_FUNC_ERROR;
					return TRUE;
					break;
					
				case 0x86:	// Error: temperature conversion not ready in time
					*func = RFXCOM_RFXSENSOR_FUNC_ERROR;
					return TRUE;
					break;
					
				case 0x87:	// Error: A/D conversion not ready in time
					*func = RFXCOM_RFXSENSOR_FUNC_ERROR;
					return TRUE;
					break;
					
				default:	// Unknown Info/Error code!
					*func = RFXCOM_RFXSENSOR_FUNC_UNKNOWN;
					return TRUE;
			}
		}
		// If no errors, get sensor data
		else {
			// Temp sensor
			if ((msg[1] & 0x03) == 0) {
				measured_value = msg[3] + ((msg[4] >> 5) * 0.125);
				if (measured_value > 200)
					measured_value = 0 - (256 - measured_value);
				*value1 = measured_value;
				*func = RFXCOM_RFXSENSOR_FUNC_TEMP;
				return TRUE;
			}
			// A/D converter
			else if ((msg[1] & 0x03) == 1) {
				measured_value = ((msg[3] * 256) + msg[4]) >> 5;
				*value1 = measured_value / 100.0;
				*func = RFXCOM_RFXSENSOR_FUNC_ADC;
				return TRUE;
			}
			// Supply voltage
			else if ((msg[1] & 0x03) == 2) {
				measured_value = ((msg[3] * 256) + msg[4]) >> 5;
				*value1 = measured_value / 100.0;
				*func = RFXCOM_RFXSENSOR_FUNC_SVOLTAGE;
				return TRUE;
			}
			// Other sensors
			else if ((msg[1] & 0x03) == 3) {
				/*
				if ((msg[4] & 0x20) == 0) {
					WriteMessage("ZAP25:" & Convert.ToString(Math.Round((5 / 1024) * (recbuf(2) * 2 + (recbuf(3) >> 7)) / 0.033, 2) & "A"), False)
                    WriteMessage(" ZAP50:" & Convert.ToString(Math.Round((5 / 1024) * (recbuf(2) * 2 + (recbuf(3) >> 7)) / 0.023, 2) & "A"), False)
                    WriteMessage(" ZAP100:" & Convert.ToString(Math.Round((5 / 1024) * (recbuf(2) * 2 + (recbuf(3) >> 7)) / 0.019, 2) & "A"), False)
				}
				else {
					WriteMessage("Voltage=" & Convert.ToString(recbuf(2) * 2), False)
				}
				*/
				*value1 = 0;
				*func = RFXCOM_RFXSENSOR_FUNC_UNKNOWN;
				return TRUE;
			}
		}
	}
	// For bigger messages
	else {
		// Extract Address 
		addr[0] = ((msg[1] >> 4) & 0x0F) < 10 ? ((msg[1] >> 4) & 0x0F) + 48 : ((msg[1] >> 4) & 0x0F) + 55;
		addr[1] = (msg[1] & 0x0F) < 10 ? (msg[1] & 0x0F) + 48 : (msg[1] & 0x0F) + 55;
		addr[2] = '\0';

		// Extract sensor data
		measured_value = msg[2] + ((msg[3] >> 6) * 0.25);
		if (measured_value > 200)
			measured_value = 0 - (256 - measured_value);
		*value1 = measured_value;
		*func = RFXCOM_RFXSENSOR_FUNC_MULTI;
		
		if ((msg[3] & 0x20) == 0) {
			// no DS18B20 connected so A/D value
			measured_value = ((msg[3] & 0x1F) * 32) + (msg[4] >> 3);
			*value2 = measured_value / 100.0;
			*func = RFXCOM_RFXSENSOR_FUNC_MULTI_AD;
		}
		else {
			// DS18B20 temperature sensor connected
			measured_value = ((msg[3] & 0x1F) << 3) + ((msg[4] >> 3) * 0.25);
			if (measured_value > 200)
				measured_value = 0 - (256 - measured_value);
			*value2 = measured_value;
		}
		
		// Supply voltage
		measured_value = ((msg[4] & 0x03) * 256) + msg[5];
		*value3 = measured_value / 100.0;

		return TRUE;
	}
	return FALSE;
}

