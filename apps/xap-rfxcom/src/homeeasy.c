/**************************************************************************

	homeeasy.c

	Functions for Home Easy devices

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

#include "homeeasy.h"


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
//	isHomeEasy(BYTE *msg)
//
//	Return TRUE if the message passed is a Home Easy message, if not return FALSE
//
//	'msg'		Message passed to check
//
//	Returns:
//		TRUE 	if the message passed is a Home Easy message
//		FALSE 	if the message is not a Home Easy message
//*************************************************************************
int isHomeEasy(BYTE *msg) {
	
	// Check 12, 34 or 38 bit message length for a Home Easy message
	if (((msg[0] & 0x7F) == 12) || ((msg[0] & 0x7F) == 34) || ((msg[0] & 0x7F) == 38))
		return TRUE;
	return FALSE;
}


//*************************************************************************
//	decode_HomeEasy(BYTE *msg, char *addr, char *id, BYTE *func, BYTE *level)
//
//	Returns true if the message is decoded sucessfully
//
//	'msg'		Message to be decoded
//	'addr'		Address of the device in ASCII format extracted from the message
//	'id'		ID of the device in ASCII format extracted from the message
//	'func'		Function extracted from the message
//	'level'		Level extracted from the message
//
//	Returns:
//		TRUE 	if decoded correctly
//		FALSE	in case of error
//*************************************************************************
int decode_HomeEasy(BYTE *msg, char *addr, char *id, BYTE *func, BYTE *level) {

	// Initialize vars
	BYTE a = 0;
	addr[0] = '\0';
	id[0] = '\0';
	*func = RFXCOM_HOMEEASY_FUNC_NONE;
	*level = 0;
	
	// Check for heater message
	if ((msg[0] & 0x7F) == 12) {
		// Extract address
		a = ((msg[1] >> 3) & 0x1F) + 1;
		addr[0] = ((a >> 4) & 0x0F) < 10 ? ((a >> 4) & 0x0F) + 48 : ((a >> 4) & 0x0F) + 55;
		addr[1] = (a & 0x0F) < 10 ? (a & 0x0F) + 48 : (a & 0x0F) + 55;
		addr[2] = '\0';
		
		// Extract command
		switch(msg[2] & 0x60) {
			case 0x20:
				*func = RFXCOM_HOMEEASY_FUNC_OFF;
				break;
				
			case 0x40:
				*func = RFXCOM_HOMEEASY_FUNC_ON;
				break;
				
			case 0x00:
				*func = RFXCOM_HOMEEASY_FUNC_NONE;	// Reserved (unexpected)
				break;
				
			case 0x60:
				*func = RFXCOM_HOMEEASY_FUNC_NONE;	// Reserved (unexpected)
				break;
		}
		return TRUE;
	}
	else {
		// Extract Address
		addr[0] = ((msg[1] >> 4) & 0x0F) < 10 ? ((msg[1] >> 4) & 0x0F) + 48 : ((msg[1] >> 4) & 0x0F) + 55;
		addr[1] = (msg[1] & 0x0F) < 10 ? (msg[1] & 0x0F) + 48 : (msg[1] & 0x0F) + 55;
		addr[2] = ((msg[2] >> 4) & 0x0F) < 10 ? ((msg[2] >> 4) & 0x0F) + 48 : ((msg[2] >> 4) & 0x0F) + 55;
		addr[3] = (msg[2] & 0x0F) < 10 ? (msg[2] & 0x0F) + 48 : (msg[2] & 0x0F) + 55;
		addr[4] = ((msg[3] >> 4) & 0x0F) < 10 ? ((msg[3] >> 4) & 0x0F) + 48 : ((msg[3] >> 4) & 0x0F) + 55;
		addr[5] = (msg[3] & 0x0F) < 10 ? (msg[3] & 0x0F) + 48 : (msg[3] & 0x0F) + 55;
		addr[6] = ((msg[4] >> 4) & 0x0C) < 10 ? ((msg[4] >> 4) & 0x0C) + 48 : ((msg[4] >> 4) & 0x0C) + 55;
		addr[7] = (msg[4] & 0x0F) < 10 ? (msg[4] & 0x0F) + 48 : (msg[4] & 0x0F) + 55;	// Extract unit in ASCII format
		addr[8] = '\0';
		
		// Extract id
		a = msg[1] >> 6;
		id[0] = ((a >> 4) & 0x0F) < 10 ? ((a >> 4) & 0x0F) + 48 : ((a >> 4) & 0x0F) + 55;
		id[1] = (a & 0x0F) < 10 ? (a & 0x0F) + 48 : (a & 0x0F) + 55;
		a = ((msg[1] << 2) + (msg[2] >> 6)) & 0xFF;
		id[2] = ((a >> 4) & 0x0F) < 10 ? ((a >> 4) & 0x0F) + 48 : ((a >> 4) & 0x0F) + 55;
		id[3] = (a & 0x0F) < 10 ? (a & 0x0F) + 48 : (a & 0x0F) + 55;
		a = ((msg[2] << 2) + (msg[3] >> 6)) & 0xFF;
		id[4] = ((a >> 4) & 0x0F) < 10 ? ((a >> 4) & 0x0F) + 48 : ((a >> 4) & 0x0F) + 55;
		id[5] = (a & 0x0F) < 10 ? (a & 0x0F) + 48 : (a & 0x0F) + 55;
		a = ((msg[3] << 2) + (msg[4] >> 6)) & 0xFF;
		id[6] = ((a >> 4) & 0x0F) < 10 ? ((a >> 4) & 0x0F) + 48 : ((a >> 4) & 0x0F) + 55;
		id[7] = (a & 0x0F) < 10 ? (a & 0x0F) + 48 : (a & 0x0F) + 55;
		id[8] = '\0';
		
		// For 34 bit message length
		if ((msg[0] & 0x7F) == 34) {
			switch(msg[4] & 0x30) {
				case 0x00:
					*func = RFXCOM_HOMEEASY_FUNC_OFF;
					break;
					
				case 0x10:
					*func = RFXCOM_HOMEEASY_FUNC_ON;
					break;
					
				case 0x20:
					*func = RFXCOM_HOMEEASY_FUNC_GRPOFF;
					break;
					
				case 0x30:
					*func = RFXCOM_HOMEEASY_FUNC_GRPON;
					break;
			}
			return TRUE;
		}
		// For 38 bit message length
		else if ((msg[0] & 0x7F) == 38) {
			switch(msg[5] & 0x0C) {
				case 0x00:
					*func = RFXCOM_HOMEEASY_FUNC_ERROR;
					break;
					
				case 0x04:
					if ((msg[4] & 0x20) == 0x00)
						*func = RFXCOM_HOMEEASY_FUNC_PRCMD;
					else
						*func = RFXCOM_HOMEEASY_FUNC_PRGRP;
					break;
					
				case 0x08:
					*func = RFXCOM_HOMEEASY_FUNC_NONE;	// Reserved (unexpected)
					break;
					
				case 0x0C:
					*func = RFXCOM_HOMEEASY_FUNC_NONE;	// Reserved (unexpected)
					break;
			}
			*level = (msg[5] >> 4) + 1;
			return TRUE;
		}
	}
	return FALSE;
}
