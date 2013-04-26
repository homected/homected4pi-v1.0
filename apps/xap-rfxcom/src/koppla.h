/**************************************************************************

	koppla.h

	Header file for Koppla devices

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

	31/01/13 by Jose Luis Galindo : First version
	
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
#define RFXCOM_KOPPLA_FUNC_OFF		0	// OFF
#define RFXCOM_KOPPLA_FUNC_LEVEL1	1
#define RFXCOM_KOPPLA_FUNC_LEVEL2	2
#define RFXCOM_KOPPLA_FUNC_LEVEL3	3
#define RFXCOM_KOPPLA_FUNC_LEVEL4	4
#define RFXCOM_KOPPLA_FUNC_LEVEL5	5
#define RFXCOM_KOPPLA_FUNC_LEVEL6	6
#define RFXCOM_KOPPLA_FUNC_LEVEL7	7
#define RFXCOM_KOPPLA_FUNC_LEVEL8	8
#define RFXCOM_KOPPLA_FUNC_LEVEL9	9
#define RFXCOM_KOPPLA_FUNC_ON		10	// ON
#define RFXCOM_KOPPLA_FUNC_UP		11	// UP
#define RFXCOM_KOPPLA_FUNC_DOWN		12	// DOWN
#define RFXCOM_KOPPLA_FUNC_PROG		13	// PROG
#define RFXCOM_KOPPLA_FUNC_UNKNOWN	14	// UNKNOWN


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

int isKoppla(BYTE *msg);
int decode_Koppla(BYTE *msg, BYTE *addrlist, BYTE *addrcount, BYTE *func, BYTE *cnt);
