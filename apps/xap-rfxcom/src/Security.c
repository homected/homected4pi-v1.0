/**************************************************************************

	Security.c

	Rfxcom Visonic and X10 security sensors

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

	28/12/12 by Jose Luis Galindo : First version
	
***************************************************************************/
/*
	Message code format:
  Bit	7 6 5 4 3 2 1 0
		| | | | | | | |______ 0: Battery OK		1: Battery low
	  	| | | | | | |________ 0: Sensor			1: Keyfob
		| | | | | |__________ 0: No Visonic		1: Visonic
		| | | | |____________ 0: Door/Window	1: Motion
		| | | |______________ 0: 				1: 
		| | |________________ 0: 				1: 
		| |__________________ 0: Tamper close	1: Tamper open
		|____________________ 0: Alert			1: Normal
 */


//*************************************************************************
//*************************************************************************
// 								INCLUDE SECTION
//*************************************************************************
//*************************************************************************

#include "Security.h"


//*************************************************************************
//*************************************************************************
// 							PRIVATE FUNCTIONS SECTION
//*************************************************************************
//*************************************************************************

BYTE even_parity(BYTE byte_to_check) {
	
	BYTE sum_of_bits;

	sum_of_bits =  (byte_to_check & 0x80) >> 7;
	sum_of_bits += (byte_to_check & 0x40) >> 6;
	sum_of_bits += (byte_to_check & 0x20) >> 5;
	sum_of_bits += (byte_to_check & 0x10) >> 4;
	sum_of_bits += (byte_to_check & 0x08) >> 3;
	sum_of_bits += (byte_to_check & 0x04) >> 2;
	sum_of_bits += (byte_to_check & 0x02) >> 1;
	sum_of_bits += (byte_to_check & 0x01);

	return (sum_of_bits & 0x01);
}


//*************************************************************************
//*************************************************************************
// 							PUBLIC FUNCTIONS SECTION
//*************************************************************************
//*************************************************************************

//*************************************************************************
//	isSecurity(BYTE *msg, BYTE *msgtype)
//
//	Return TRUE if the message is a SECURITY message, if not return FALSE
//
//	'msg'		Message passed to check
//	'msgtype'	Return type of security message
//
//	Returns:
//		TRUE 	if the message passed is a security message
//		FALSE 	if the message is not a security message
//*************************************************************************
int isSecurity(BYTE *msg, BYTE *msgtype) {
	
	// Check 41 bit message length for a Visonic PowerCode message
	if ((msg[0] & 0x7F) == 0x29) {
		// Check bytes
		if ((msg[3] + msg[4] == 0xFF) && (even_parity(msg[5]) == ((msg[6] & 0x80) >> 7))) {
			// Check for jamming
			if (secIsJammingDetect(msg))
				*msgtype = RFXCOM_SECURITY_JAMMING_DETECT;
			else if (secIsJammingEnd(msg))
				*msgtype = RFXCOM_SECURITY_JAMMING_END;
			else
				*msgtype = RFXCOM_SECURITY_MSG_POWERCODE;
			return TRUE;
		}
	}
	// Check XX bit message length for a Visonic CodeSecure message
	//else if ((msg[0] & 0x7F) == 0x??) {
	//	*msgtype = RFXCOM_SECURITY_MSG_CODESECURE;
	//}
	// Check if is a X10Security message
	else if ((msg[1] == ((msg[2] & 0xF0) + (0xF - (msg[2] & 0xF)))) && ((msg[3] ^ msg[4]) == 0xFF)) {
		// Check for jamming
		if (secIsJammingDetect(msg))
			*msgtype = RFXCOM_SECURITY_JAMMING_DETECT;
		else if (secIsJammingEnd(msg))
			*msgtype = RFXCOM_SECURITY_JAMMING_END;
		else
			*msgtype = RFXCOM_SECURITY_MSG_X10SEC;
		return TRUE;
	}
	*msgtype = RFXCOM_SECURITY_MSG_UNKNOWN;
	return FALSE;
}


//*************************************************************************
//	decode_powercode(BYTE *msg, char *addr, BYTE *msgcode)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'addr'		Address in ASCII format extracted from the message
//	'msgcode'	Message code extracted from the message
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_powercode(BYTE *msg, char *addr, BYTE *msgcode) {
	
	// Extract Address
	addr[0] = ((msg[1] & 0xF0) >> 4) < 10 ? ((msg[1] & 0xF0) >> 4) + 48 : ((msg[1] & 0xF0) >> 4) + 55;
	addr[1] = (msg[1] & 0x0F) < 10 ? (msg[1] & 0x0F) + 48 : (msg[1] & 0x0F) + 55;
	addr[2] = ((msg[2] & 0xF0) >> 4) < 10 ? ((msg[2] & 0xF0) >> 4) + 48 : ((msg[2] & 0xF0) >> 4) + 55;
	addr[3] = (msg[2] & 0x0F) < 10 ? (msg[2] & 0x0F) + 48 : (msg[2] & 0x0F) + 55;
	addr[4] = ((msg[5] & 0xF0) >> 4) < 10 ? ((msg[5] & 0xF0) >> 4) + 48 : ((msg[5] & 0xF0) >> 4) + 55;
	addr[5] = (msg[5] & 0x0F) < 10 ? (msg[5] & 0x0F) + 48 : (msg[5] & 0x0F) + 55;
	addr[6] = '\0';
	
	// Extract msgcode
	*msgcode = msg[3];
	
	return TRUE;
}


//*************************************************************************
//	decode_X10Sec(BYTE *msg, char *addr, BYTE *msgcode)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'addr'		Address in ASCII format extracted from the message
//	'msgcode'	Message code extracted from the message
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_X10Sec(BYTE *msg, char *addr, BYTE *msgcode) {
	return decode_powercode(msg, addr, msgcode);
}


//*************************************************************************
//	secGetSensorNumEndpoints(int devcaps)
//
//	Returns the number of endpoints for the sensor devcaps given
//
//	'devcaps'	Device capabilities of the sensor
//
//	Returns:
//		The number of endpoints for the sensor given
//
//*************************************************************************
int secGetSensorNumEndpoints(int devcaps) {
	int result;
	
	result =  (devcaps & 0x001);		// Has Alert contact
	result += (devcaps & 0x002) >> 1;	// Detect Tamper open/close
	result += (devcaps & 0x004) >> 2;	// Detect Battery low
	result += (devcaps & 0x008) >> 3;	// Has Disarm
	result += (devcaps & 0x010) >> 4;	// Has Arm away
	result += (devcaps & 0x020) >> 5;	// Has Arm home
	result += (devcaps & 0x040) >> 6;	// Has Panic
	result += (devcaps & 0x080) >> 7;	// Has Lights on
	result += (devcaps & 0x100) >> 8;	// Has Lights off
	
	return result;
}


//*************************************************************************
//	secGetSensorEndpointSubUID(BYTE endpoint, int devcaps)
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
int secGetSensorEndpointSubUID(BYTE endpoint, int devcaps) {
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
//	secSupportAlertEvents(int devcaps)
//
//	Returns true if the sensor devcaps given support Alert/Normal events
//
//	'devcaps'	Device capabilities of the sensor
//
//	Returns:
//		TRUE 	The sensor support Alert/Normal events
//		FALSE	The sensor not support Alert/Normal events
//*************************************************************************
int secSupportAlertEvents(int devcaps) {

	if (devcaps & 0x01)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secSupportTamperEvents(int devcaps)
//
//	Returns true if the sensor devcaps given support Tamper events
//
//	'devcaps'	Device capabilities of the sensor
//
//	Returns:
//		TRUE 	The sensor support Tamper events
//		FALSE	The sensor not support Tamper events
//*************************************************************************
int secSupportTamperEvents(int devcaps) {

	if (devcaps & 0x02)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secSupportBatteryLowEvents(int devcaps)
//
//	Returns true if the sensor devcaps given support low battery events
//
//	'devcaps'	Device capabilities of the sensor
//
//	Returns:
//		TRUE 	The sensor support low battery events
//		FALSE	The sensor not support low battery events
//*************************************************************************
int secSupportBatteryLowEvents(int devcaps) {

	if (devcaps & 0x04)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secSupportDisarm(int devcaps)
//
//	Returns true if the sensor devcaps given support Disarm
//
//	'devcaps'	Device capabilities of the sensor
//
//	Returns:
//		TRUE 	The sensor support Disarm
//		FALSE	The sensor not support Disarm
//*************************************************************************
int secSupportDisarm(int devcaps) {

	if (devcaps & 0x08)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secSupportArmAway(int devcaps)
//
//	Returns true if the sensor devcaps given support Arm away
//
//	'devcaps'	Device capabilities of the sensor
//
//	Returns:
//		TRUE 	The sensor support Arm away
//		FALSE	The sensor not support Arm away
//*************************************************************************
int secSupportArmAway(int devcaps) {

	if (devcaps & 0x10)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secSupportArmHome(int devcaps)
//
//	Returns true if the sensor devcaps given support Arm home
//
//	'devcaps'	Device capabilities of the sensor
//
//	Returns:
//		TRUE 	The sensor support Arm home
//		FALSE	The sensor not support Arm home
//*************************************************************************
int secSupportArmHome(int devcaps) {

	if (devcaps & 0x20)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secSupportPanic(int devcaps)
//
//	Returns true if the sensor devcaps given support Panic
//
//	'devcaps'	Device capabilities of the sensor
//
//	Returns:
//		TRUE 	The sensor support Panic
//		FALSE	The sensor not support Panic
//*************************************************************************
int secSupportPanic(int devcaps) {

	if (devcaps & 0x40)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secSupportLightsOn(int devcaps)
//
//	Returns true if the sensor devcaps given support Lights on
//
//	'devcaps'	Device capabilities of the sensor
//
//	Returns:
//		TRUE 	The sensor support Lights on
//		FALSE	The sensor not support Lights on
//*************************************************************************
int secSupportLightsOn(int devcaps) {

	if (devcaps & 0x80)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secSupportLightsOff(int devcaps)
//
//	Returns true if the sensor devcaps given support Lights off
//
//	'devcaps'	Device capabilities of the sensor
//
//	Returns:
//		TRUE 	The sensor support Lights off
//		FALSE	The sensor not support Lights off
//*************************************************************************
int secSupportLightsOff(int devcaps) {

	if (devcaps & 0x100)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsJammingDetect(BYTE *msg)
//
//	Returns true if the message has a jamming detect event
//
//	'msg'		Message passed to check
//
//	Returns:
//		TRUE 	if is an jamming detect event
//		FALSE	in case of no jamming detect event
//*************************************************************************
int secIsJammingDetect(BYTE *msg) {
	if ((msg[3] == 0xE0) && ((msg[1] == 0xFF) || (msg[1] == 0x00)))
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsJammingEnd(BYTE *msg)
//
//	Returns true if the message has a jamming end event
//
//	'msg'		Message passed to check
//
//	Returns:
//		TRUE 	if is an jamming end event
//		FALSE	in case of no jamming end event
//*************************************************************************
int secIsJammingEnd(BYTE *msg) {
	if ((msg[3] == 0xF8) && ((msg[1] == 0xFF) || (msg[1] == 0x00)))
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsAlert(BYTE msgcode)
//
//	Returns true if the message has a alert event
//
//	'msgcode'	Message code of the message
//
//	Returns:
//		TRUE 	if is an alert event
//		FALSE	in case of no alert event
//*************************************************************************
int secIsAlert(BYTE msgcode) {
	if (((msgcode & 0x82) == 0x00) || (msgcode == 0x20))
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsNormal(BYTE msgcode)
//
//	Returns true if the message has a normal event
//
//	'msgcode'	Message code of the message
//
//	Returns:
//		TRUE 	if is an normal event
//		FALSE	in case of no normal event
//*************************************************************************
int secIsNormal(BYTE msgcode) {
	if ((msgcode & 0x82) == 0x80)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsTamperOpen(BYTE msgcode)
//
//	Returns true if the message has a tamper open event
//
//	'msgcode'	Message code of the message
//
//	Returns:
//		TRUE 	if is a tamper open event
//		FALSE	in case of no tamper open event
//*************************************************************************
int secIsTamperOpen(BYTE msgcode) {
	if ((msgcode & 0x42) == 0x40)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsTamperClose(BYTE msgcode)
//
//	Returns true if the message has a tamper close event
//
//	'msgcode'	Message code of the message
//
//	Returns:
//		TRUE 	if is a tamper close event
//		FALSE	in case of no tamper close event
//*************************************************************************
int secIsTamperClose(BYTE msgcode) {
	if ((msgcode & 0x42) == 0x00)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsBatteryLow(BYTE msgcode)
//
//	Returns true if the message has a battery low event
//
//	'msgcode'	Message code of the message
//
//	Returns:
//		TRUE 	if is a battery low event
//		FALSE	in case of no battery low event
//*************************************************************************
int secIsBatteryLow(BYTE msgcode) {
	if ((msgcode & 0x03) == 0x01)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsBatteryGood(BYTE msgcode)
//
//	Returns true if the message has a battery good event
//
//	'msgcode'	Message code of the message
//
//	Returns:
//		TRUE 	if is a battery good event
//		FALSE	in case of no battery good event
//*************************************************************************
int secIsBatteryGood(BYTE msgcode) {
	if ((msgcode & 0x03) == 0x00)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsNoKeyfob(BYTE msgcode)
//
//	Returns true if the message hasn't any keyfob event
//
//	'msgcode'	Message code of the message
//
//	Returns:
//		TRUE 	if is not a keyfob event
//		FALSE	in case of a keyfob event
//*************************************************************************
int secIsNoKeyfob(BYTE msgcode) {
	if ((msgcode & 0x12) == 0x02)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsDisarm(BYTE msgcode)
//
//	Returns true if the message has a Disarm event
//
//	'msgcode'	Message code of the message
//
//	Returns:
//		TRUE 	if is a Disarm event
//		FALSE	in case of no Disarm event
//*************************************************************************
int secIsDisarm(BYTE msgcode) {
	if ((msgcode & 0xFB) == 0x82)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsArmAway(BYTE msgcode)
//
//	Returns true if the message has a Arm Away event
//
//	'msgcode'	Message code of the message
//
//	Returns:
//		TRUE 	if is a Arm Away event
//		FALSE	in case of no Arm Away event
//*************************************************************************
int secIsArmAway(BYTE msgcode) {
	if ((msgcode & 0xFB) == 0x02)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsArmHome(BYTE msgcode)
//
//	Returns true if the message has a Arm Home event
//
//	'msgcode'	Message code of the message
//
//	Returns:
//		TRUE 	if is a Arm Home event
//		FALSE	in case of no Arm Home event
//*************************************************************************
int secIsArmHome(BYTE msgcode) {
	if ((msgcode & 0xFB) == 0x0A)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsPanic(BYTE msgcode)
//
//	Returns true if the message has a Panic event
//
//	'msgcode'	Message code of the message
//
//	Returns:
//		TRUE 	if is a Panic event
//		FALSE	in case of no Panic event
//*************************************************************************
int secIsPanic(BYTE msgcode) {
	if (((msgcode & 0xFB) == 0x22) || (msgcode == 0x03))
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsLightsOn(BYTE msgcode)
//
//	Returns true if the message has a Lights On event
//
//	'msgcode'	Message code of the message
//
//	Returns:
//		TRUE 	if is a Lights On event
//		FALSE	in case of no Lights On event
//*************************************************************************
int secIsLightsOn(BYTE msgcode) {
	if ((msgcode & 0xFB) == 0x42)
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	secIsLightsOff(BYTE msgcode)
//
//	Returns true if the message has a Lights Off event
//
//	'msgcode'	Message code of the message
//
//	Returns:
//		TRUE 	if is a Lights Off event
//		FALSE	in case of no Lights Off event
//*************************************************************************
int secIsLightsOff(BYTE msgcode) {
	if ((msgcode & 0xFB) == 0xC2)
		return TRUE;
	return FALSE;
}

