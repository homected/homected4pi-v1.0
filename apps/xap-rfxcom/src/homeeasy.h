/**************************************************************************

	homeeasy.h

	Header file for Home Easy devices

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
// 									INCLUDE SECTION
//*************************************************************************
//*************************************************************************


//*************************************************************************
//*************************************************************************
// 									DEFINES SECTION
//*************************************************************************
//*************************************************************************

#ifndef BYTE
#define BYTE	unsigned char
#endif
#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE	1
#endif

// Function codes
#define RFXCOM_HOMEEASY_FUNC_NONE	0	// UNKNOWN
#define RFXCOM_HOMEEASY_FUNC_OFF	1	// OFF
#define RFXCOM_HOMEEASY_FUNC_ON		2	// ON
#define RFXCOM_HOMEEASY_FUNC_GRPOFF	3	// GROUP OFF
#define RFXCOM_HOMEEASY_FUNC_GRPON	4	// GROUP ON
#define RFXCOM_HOMEEASY_FUNC_PRCMD	5	// PRESET COMMAND
#define RFXCOM_HOMEEASY_FUNC_PRGRP	6	// PRESET GROUP
#define RFXCOM_HOMEEASY_FUNC_ERROR	7


//*************************************************************************
//*************************************************************************
// 									CUSTOM TYPES
//*************************************************************************
//*************************************************************************


//*************************************************************************
//*************************************************************************
// 									GLOBAL VARIABLES
//*************************************************************************
//*************************************************************************


//*************************************************************************
//*************************************************************************
// 									GLOBAL FUNCTIONS
//*************************************************************************
//*************************************************************************

int isHomeEasy(BYTE *msg);
int decode_HomeEasy(BYTE *msg, char *addr, char *id, BYTE *func, BYTE *level);
