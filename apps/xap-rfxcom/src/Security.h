/**************************************************************************

	Security.h

	Header file for Rfxcom Visonic and X10 security sensors

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
#define RFXCOM_SECURITY_MSG_UNKNOWN		0
#define RFXCOM_SECURITY_MSG_POWERCODE	1
#define RFXCOM_SECURITY_MSG_CODESECURE	2
#define RFXCOM_SECURITY_MSG_X10SEC		3
#define RFXCOM_SECURITY_JAMMING_DETECT	4
#define RFXCOM_SECURITY_JAMMING_END		5

// Endpoints
#define RFXCOM_SECURITY_ENDP_ALERT		0
#define RFXCOM_SECURITY_ENDP_TAMPER		1
#define RFXCOM_SECURITY_ENDP_BATLOW		2
#define RFXCOM_SECURITY_ENDP_DISARM		3
#define RFXCOM_SECURITY_ENDP_ARMAWAY	4
#define RFXCOM_SECURITY_ENDP_ARMHOME	5
#define RFXCOM_SECURITY_ENDP_PANIC		6
#define RFXCOM_SECURITY_ENDP_LIGHTSON	7
#define RFXCOM_SECURITY_ENDP_LIGHTSOFF	8


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

int isSecurity(BYTE *msg, BYTE *msgtype);
int decode_powercode(BYTE *msg, char *addr, BYTE *msgcode);
int decode_X10Sec(BYTE *msg, char *addr, BYTE *msgcode);
int secGetSensorNumEndpoints(int devcaps);
int secGetSensorEndpointSubUID(BYTE endpoint, int devcaps);
int secSupportAlertEvents(int devcaps);
int secSupportTamperEvents(int devcaps);
int secSupportBatteryLowEvents(int devcaps);
int secSupportDisarm(int devcaps);
int secSupportArmAway(int devcaps);
int secSupportArmHome(int devcaps);
int secSupportPanic(int devcaps);
int secSupportLightsOn(int devcaps);
int secSupportLightsOff(int devcaps);
int secIsJammingDetect(BYTE *msg);
int secIsJammingEnd(BYTE *msg);
int secIsAlert(BYTE msgcode);
int secIsNormal(BYTE msgcode);
int secIsTamperOpen(BYTE msgcode);
int secIsTamperClose(BYTE msgcode);
int secIsBatteryLow(BYTE msgcode);
int secIsBatteryGood(BYTE msgcode);
int secIsNoKeyfob(BYTE msgcode);
int secIsDisarm(BYTE msgcode);
int secIsArmAway(BYTE msgcode);
int secIsArmHome(BYTE msgcode);
int secIsPanic(BYTE msgcode);
int secIsLightsOn(BYTE msgcode);
int secIsLightsOff(BYTE msgcode);
