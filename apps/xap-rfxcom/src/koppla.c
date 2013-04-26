/**************************************************************************

	koppla.c

	Functions for Koppla devices

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

	28/01/13 by Jose Luis Galindo : First version
	
***************************************************************************/

//*************************************************************************
//*************************************************************************
// 								INCLUDE SECTION
//*************************************************************************
//*************************************************************************

#include "koppla.h"


//*************************************************************************
//*************************************************************************
// 							PRIVATE FUNCTIONS SECTION
//*************************************************************************
//*************************************************************************


//*************************************************************************
//*************************************************************************
// 							PUBLIC FUNCTIONS SECTION
//*************************************************************************
//*************************************************************************

//*************************************************************************
//	isKoppla(BYTE *msg)
//
//	Return TRUE if the message passed is a Koppla message, if not return FALSE
//
//	'msg'		Message passed to check
//
//	Returns:
//		TRUE 	if the message passed is a Koppla message
//		FALSE 	if the message is not a Koppla message
//*************************************************************************
int isKoppla(BYTE *msg) {
	BYTE temp;
	int parity, i;

	// Check for odd parity error in byte 0
	parity = 0;
	temp = msg[1];
	for (i=0; i<4; i++) {
		parity = parity + (temp & 0x01);
		temp = temp >> 2;
	}
	if ((parity & 0x01) != 1)
		return FALSE;
	
	// Check for even parity error in byte 0
	parity = 0;
	temp = msg[1];
	for (i=0; i<8; i++) {
		parity = parity + (temp & 0x01);
		temp = temp >> 1;
	}
	if ((parity & 0x01) != 0)
		return FALSE;
		
	// Check for odd parity error in byte 1-2
	parity = 0;
	temp = msg[3];
	for (i=0; i<4; i++) {
		parity = parity + (temp & 0x01);
		temp = temp >> 2;
	}
	temp = msg[2];
	for (i=0; i<4; i++) {
		parity = parity + (temp & 0x01);
		temp = temp >> 2;
	}
	if ((parity & 0x01) != 1)
		return FALSE;
	
	// Check for even parity error in byte 1-2
	parity = 0;
	temp = msg[3];
	for (i=0; i<8; i++) {
		parity = parity + (temp & 0x01);
		temp = temp >> 1;
	}
	temp = msg[2];
	for (i=0; i<8; i++) {
		parity = parity + (temp & 0x01);
		temp = temp >> 1;
	}
	if ((parity & 0x01) != 0)
		return FALSE;

	return TRUE;
}


//*************************************************************************
//	decode_Koppla(BYTE *msg, BYTE *addrlist, char *addrcount, BYTE *func, BYTE *cnt)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'addrlist'	List of addresses extracted from the message
//	'addrcount'	Number of entries in address list
//	'func'		Function extracted from the message
//	'cnt'		Count value for UP & DOWN functions
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_Koppla(BYTE *msg, BYTE *addrlist, BYTE *addrcount, BYTE *func, BYTE *cnt) {
	BYTE numchannels = 0;
	*cnt = 0;
	
	// Extract channels (Addresses)
	if ((msg[3] & 0x20) == 0x20) {
		addrlist[numchannels] = 1;
		numchannels++;
	}
	if (msg[3] & 0x40) {
		addrlist[numchannels] = 2;
		numchannels++;
	}
	if (msg[3] & 0x80) {
		addrlist[numchannels] = 3;
		numchannels++;
	}
	if (msg[2] & 0x01) {
		addrlist[numchannels] = 4;
		numchannels++;
	}
	if (msg[2] & 0x02) {
		addrlist[numchannels] = 5;
		numchannels++;
	}
	if (msg[2] & 0x04) {
		addrlist[numchannels] = 6;
		numchannels++;
	}
	if (msg[2] & 0x08) {
		addrlist[numchannels] = 7;
		numchannels++;
	}
	if (msg[2] & 0x10) {
		addrlist[numchannels] = 8;
		numchannels++;
	}
	if (msg[2] & 0x20) {
		addrlist[numchannels] = 9;
		numchannels++;
	}
	if (msg[3] & 0x10) {
		addrlist[numchannels] = 10;
		numchannels++;
	}
	*addrcount = numchannels;
	
	// Extract command (function)
	if ((msg[1] & 0x3F) == 0x10)
		*func = RFXCOM_KOPPLA_FUNC_ON;
	else if ((msg[1] & 0x3F) == 0x11)
		*func = RFXCOM_KOPPLA_FUNC_LEVEL1;
	else if ((msg[1] & 0x3F) == 0x12)
		*func = RFXCOM_KOPPLA_FUNC_LEVEL2;
	else if ((msg[1] & 0x3F) == 0x13)
		*func = RFXCOM_KOPPLA_FUNC_LEVEL3;
	else if ((msg[1] & 0x3F) == 0x14)
		*func = RFXCOM_KOPPLA_FUNC_LEVEL4;
	else if ((msg[1] & 0x3F) == 0x15)
		*func = RFXCOM_KOPPLA_FUNC_LEVEL5;
	else if ((msg[1] & 0x3F) == 0x16)
		*func = RFXCOM_KOPPLA_FUNC_LEVEL6;
	else if ((msg[1] & 0x3F) == 0x17)
		*func = RFXCOM_KOPPLA_FUNC_LEVEL7;
	else if ((msg[1] & 0x3F) == 0x18)
		*func = RFXCOM_KOPPLA_FUNC_LEVEL8;
	else if ((msg[1] & 0x3F) == 0x19)
		*func = RFXCOM_KOPPLA_FUNC_LEVEL9;
	else if ((msg[1] & 0x3F) == 0x1A)
		*func = RFXCOM_KOPPLA_FUNC_OFF;
	else if ((msg[1] & 0x38) == 0x00) {
		*func = RFXCOM_KOPPLA_FUNC_UP;
		*cnt = (msg[1] & 0x07);
	}
	else if ((msg[1] & 0x38) == 0x08) {
		*func = RFXCOM_KOPPLA_FUNC_DOWN;
		*cnt = (msg[1] & 0x07);
	}
	else if ((msg[1] & 0x3F) == 0x1C)
		*func = RFXCOM_KOPPLA_FUNC_PROG;
	else {
		*func = RFXCOM_KOPPLA_FUNC_UNKNOWN;
		return FALSE;
	}
	return TRUE;
}
