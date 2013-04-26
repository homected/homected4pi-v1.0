/**************************************************************************

	Oregon.h

	Header file for Oregon Scientific sensors

	Copyright (c) 2013 Jose Luis Galindo <support@homected.com>

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

	12/01/13 by Jose Luis Galindo : First version
	
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
#define RFXCOM_OREGON_MSG_UNKNOWN	0
#define RFXCOM_OREGON_MSG_TEMP1		1
#define RFXCOM_OREGON_MSG_TEMP2		2
#define RFXCOM_OREGON_MSG_TEMP3		3
#define RFXCOM_OREGON_MSG_TEMP4		4
#define RFXCOM_OREGON_MSG_TH1		5
#define RFXCOM_OREGON_MSG_TH2		6
#define RFXCOM_OREGON_MSG_TH3		7
#define RFXCOM_OREGON_MSG_TH4		8
#define RFXCOM_OREGON_MSG_TH5		9
#define RFXCOM_OREGON_MSG_TH6		10
#define RFXCOM_OREGON_MSG_THB1		11
#define RFXCOM_OREGON_MSG_THB2		12
#define RFXCOM_OREGON_MSG_RAIN1		13
#define RFXCOM_OREGON_MSG_RAIN2		14
#define RFXCOM_OREGON_MSG_RAIN3		15
#define RFXCOM_OREGON_MSG_WIND1		16
#define RFXCOM_OREGON_MSG_WIND2		17
#define RFXCOM_OREGON_MSG_WIND3		18
#define RFXCOM_OREGON_MSG_UV1		19
#define RFXCOM_OREGON_MSG_UV2		20
#define RFXCOM_OREGON_MSG_DT1		21
#define RFXCOM_OREGON_MSG_WEIGHT1	22
#define RFXCOM_OREGON_MSG_WEIGHT2	23
#define RFXCOM_OREGON_MSG_ELEC1		24
#define RFXCOM_OREGON_MSG_ELEC2		25

// Endpoints
#define RFXCOM_OREGON_ENDP_BATLOW	0
#define RFXCOM_OREGON_ENDP_TEMP		1
#define RFXCOM_OREGON_ENDP_HUM		2
#define RFXCOM_OREGON_ENDP_BARO		3
#define RFXCOM_OREGON_ENDP_RAINRATE	4
#define RFXCOM_OREGON_ENDP_RAINALL	5
#define RFXCOM_OREGON_ENDP_WINDDIR	6
#define RFXCOM_OREGON_ENDP_WINDSPD	7
#define RFXCOM_OREGON_ENDP_WINDAVG	8
#define RFXCOM_OREGON_ENDP_UV		9
#define RFXCOM_OREGON_ENDP_WEIGHT	10
#define RFXCOM_OREGON_ENDP_PHASE1	11
#define RFXCOM_OREGON_ENDP_PHASE2	12
#define RFXCOM_OREGON_ENDP_PHASE3	13

// Baterry status
#define RFXCOM_OREGON_BAT_UNKNOWN	255
#define RFXCOM_OREGON_BAT_OK		254
#define RFXCOM_OREGON_BAT_LOW		253

// Device config
#define RFXCOM_OREGON_CFG_TEMPF		0x01
#define RFXCOM_OREGON_CFG_PRESSBAR	0x02
#define RFXCOM_OREGON_CFG_RAININCH	0x04
#define RFXCOM_OREGON_CFG_WINDMILLE	0x08
#define RFXCOM_OREGON_CFG_WINDHR	0x10
#define RFXCOM_OREGON_CFG_WEIGHTLB	0x20


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

int isOregon(BYTE *msg, BYTE *msgtype);
int oreSupport(BYTE cap, int devcaps);
int oreGetSensorNumEndpoints(int devcaps);
int oreGetSensorEndpointSubUID(BYTE endpoint, int devcaps);
int decode_oregon_temp(BYTE *msg, BYTE msgType, char *addr, float *temp, BYTE *bat);
int decode_oregon_th(BYTE *msg, BYTE msgType, char *addr, float *temp, float *hum, BYTE *bat);
int decode_oregon_thb(BYTE *msg, BYTE msgType, char *addr, float *temp, float *hum, float *baro, BYTE *bat);
int decode_oregon_rain(BYTE *msg, BYTE msgType, char *addr, float *rrate, float *rtotal, BYTE *bat);
int decode_oregon_wind(BYTE *msg, BYTE msgType, char *addr, float *wdir, float *wspeed, float *wavgspeed, BYTE *bat);
int decode_oregon_uv(BYTE *msg, BYTE msgType, char *addr, float *uvfactor, BYTE *bat);
int decode_oregon_weight(BYTE *msg, BYTE msgType, char *addr, float *weight);
int decode_oregon_elec(BYTE *msg, BYTE msgType, char *addr, float *ct1, float *ct2, float *ct3, BYTE *bat);
int decode_oregon_dt(BYTE *msg, BYTE msgType, char *addr, float *date, float *time, float *day);
