/**************************************************************************

	X10RF.c

	Rfxcom X10RF functions

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

	12/01/13 by Jose Luis Galindo : Added support for DM10
	11/12/12 by Jose Luis Galindo : First version
	
***************************************************************************/

//*************************************************************************
//*************************************************************************
// 								INCLUDE SECTION
//*************************************************************************
//*************************************************************************

#include "X10RF.h"


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
//	isX10RF(BYTE *msg)
//
//	Return TRUE if the message passed is a X10 message, if not return FALSE
//
//	'msg'		Message passed to check
//
//	Returns:
//		TRUE 	if the message passed is a X10 message
//		FALSE 	if the message is not a X10 message
//*************************************************************************
int isX10RF(BYTE *msg) {
	
	// Check 32 bit message length
	if ((msg[0] & 0x7F) == 0x20) {
		// Check bytes
		if ((msg[1] + msg[2] == 0xFF) && (msg[3] + msg[4] == 0xFF))
			return TRUE;
		
	}
	return FALSE;
}


//*************************************************************************
//	isDM10(BYTE *msg)
//
//	Return TRUE if the message passed is a DM10 message, if not return FALSE
//
//	'msg'		Message passed to check
//
//	Returns:
//		TRUE 	if the message passed is a DM10 message
//		FALSE 	if the message is not a DM10 message
//*************************************************************************
int isDM10(BYTE *msg) {
	
	// Check 32 bit message length
	if ((msg[0] & 0x7F) == 0x20) {
		// Check bytes
		if (((msg[1] ^ msg[2]) == 0xFE) && ((msg[3] ^ msg[4]) == 0xFF))
			return TRUE;
	}
	return FALSE;
}

//*************************************************************************
//	decode_X10RF(BYTE *msg, BYTE *hcode, BYTE *ucode, BYTE *func)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'hcode'		House code extracted from the message
//	'ucode'		Unit code extracted from the message
//	'func'		Function extracted from the message
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_X10RF(BYTE *msg, BYTE *hcode, BYTE *ucode, BYTE *func) {

	// Extract HouseCode
	switch(msg[1] & 0xF0) {
		case 0x60:
			*hcode = 'A'; break;
		case 0x70:
			*hcode = 'B'; break;
		case 0x40:
			*hcode = 'C'; break;
		case 0x50:
			*hcode = 'D'; break;
		case 0x80:
			*hcode = 'E'; break;
		case 0x90:
			*hcode = 'F'; break;
		case 0xA0:
			*hcode = 'G'; break;
		case 0xB0:
			*hcode = 'H'; break;
		case 0xE0:
			*hcode = 'I'; break;
		case 0xF0:
			*hcode = 'J'; break;
		case 0xC0:
			*hcode = 'K'; break;
		case 0xD0:
			*hcode = 'L'; break;
		case 0x00:
			*hcode = 'M'; break;
		case 0x10:
			*hcode = 'N'; break;
		case 0x20:
			*hcode = 'O'; break;
		case 0x30:
			*hcode = 'P'; break;
		default:
			return FALSE;
	}

	// Extract UnitCode & Function
	switch(msg[3]) {
		case 0x80:
			*func = X10RF_ALL_LIGHTS_OFF; break;
		case 0x90:
			*func = X10RF_ALL_LIGHTS_ON; break;
		case 0x88:
			*func = X10RF_BRIGHT; break;
		case 0x98:
			*func = X10RF_DIM; break;
		default:
			// Create unit code (format 0..15)
			if ((msg[3] & 0x10) == 0)
				*ucode = 0x00;
			else
				*ucode = 0x01;
			if ((msg[3] & 0x08) != 0)
				*ucode += 0x02;
			if ((msg[3] & 0x40) != 0)
				*ucode += 0x04;
			if ((msg[1] & 0x04) != 0)
				*ucode += 0x08;
			// Convert unit code to format 1..16
			(*ucode)++;
			// Extract function for addressable commands
			if ((msg[3] & 0x20) == 0)
				*func = X10RF_ON;
			else
				*func = X10RF_OFF;
	}
	return TRUE;
}


//*************************************************************************
//	decode_DM10(BYTE *msg, BYTE *hcode, BYTE *ucode, BYTE *func)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'hcode'		House code extracted from the message
//	'ucode'		Unit code extracted from the message
//	'func'		Function extracted from the message
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_DM10(BYTE *msg, BYTE *hcode, BYTE *ucode, BYTE *func) {
	*hcode = ((msg[1] & 0xF0) >> 4) + 65;
	*ucode = (msg[1] & 0x0F) + 1;
	switch (msg[3]) {
		case 0xE0:	// Motion detected
			*func = X10RF_ON;
			break;
		
		case 0xF0:	// Dark detected
			*func = X10RF_OFF;
			break;
			
		case 0xF8:	// Light detected
			*func = X10RF_ON;
			break;
		
		default:
			return FALSE;
	}
	return TRUE;
}
