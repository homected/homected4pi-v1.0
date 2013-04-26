/**************************************************************************

	X10RF.h

	Header file for Rfxcom X10RF functions

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
//#define X10RF_ALL_UNITS_OFF	0	// Not exists for X10RF protocol
#define X10RF_ALL_LIGHTS_ON		1	
#define X10RF_ON				2
#define X10RF_OFF				3
#define X10RF_DIM				4
#define X10RF_BRIGHT			5
#define X10RF_ALL_LIGHTS_OFF	6	
//#define X10RF_EXTENDED_CODE	7	// Not exists for X10RF protocol
//#define X10RF_HAIL_REQUEST	8	// Not exists for X10RF protocol
//#define X10RF_HAIL_ACK		9	// Not exists for X10RF protocol
//#define X10RF_PRESET_DIM_1	10	// Not exists for X10RF protocol
//#define X10RF_PRESET_DIM_2	11	// Not exists for X10RF protocol
//#define X10RF_EXTENDED_DATA	12	// Not exists for X10RF protocol
//#define X10RF_STATUS_ON		13	// Not exists for X10RF protocol
//#define X10RF_STATUS_OFF		14	// Not exists for X10RF protocol
//#define X10RF_STATUS_REQUEST	15	// Not exists for X10RF protocol


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

int isX10RF(BYTE *msg);
int isDM10(BYTE *msg);
int decode_X10RF(BYTE *msg, BYTE *hcode, BYTE *ucode, BYTE *func);
