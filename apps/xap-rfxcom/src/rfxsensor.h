/**************************************************************************

	rfxsensor.h

	Header file for Rfxsensor and Rfxmeter devices

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

// Message Types
#define RFXCOM_RFXSENSOR_MSG_UNKNOWN	0
#define RFXCOM_RFXSENSOR_MSG_RFXSENSOR	1
#define RFXCOM_RFXSENSOR_MSG_RFXPOWER	2
#define RFXCOM_RFXSENSOR_MSG_SRFXPOWER	3

// Function Types
#define RFXCOM_RFXSENSOR_FUNC_UNKNOWN	0
#define RFXCOM_RFXSENSOR_FUNC_INFO		1
#define RFXCOM_RFXSENSOR_FUNC_ERROR		2
#define RFXCOM_RFXSENSOR_FUNC_TEMP		3
#define RFXCOM_RFXSENSOR_FUNC_ADC		4
#define RFXCOM_RFXSENSOR_FUNC_SVOLTAGE	5
#define RFXCOM_RFXSENSOR_FUNC_MULTI		6
#define RFXCOM_RFXSENSOR_FUNC_MULTI_AD	7

// Devcaps
#define RFXCOM_RFXSENSOR_DCAPS_TEMP		1
#define RFXCOM_RFXSENSOR_DCAPS_ADC		2
#define RFXCOM_RFXSENSOR_DCAPS_SVOLTAGE	4
#define RFXCOM_RFXSENSOR_DCAPS_MULTI	8
#define RFXCOM_RFXSENSOR_DCAPS_MULTI_AD	16

// Device config
#define RFXCOM_RFXSENSOR_CFG_TEMPF		1


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

int isRfxSensor(BYTE *msg, BYTE *msgtype);
int rfxGetSensorNumEndpoints(int devcaps);
int decode_rfxsensor(BYTE *msg, char *addr, BYTE *func, float *value1, float *value2, float *value3);

// TO DO:
//int decode_rfxmeter(BYTE *msg, BYTE *addrlist, BYTE *addrcount, BYTE *func, BYTE *cnt);
//int decode_srfxmeter(BYTE *msg, BYTE *addrlist, BYTE *addrcount, BYTE *func, BYTE *cnt);
