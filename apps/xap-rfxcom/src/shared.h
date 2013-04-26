/**************************************************************************

	shared.h

	Header file for shared variables and functions

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

	25/10/12 by Jose Luis Galindo : First version
	
***************************************************************************/

//*************************************************************************
//*************************************************************************
// 									INCLUDE SECTION
//*************************************************************************
//*************************************************************************

#include "libxap/libxap.h"
#include <sys/shm.h>
#include <string.h>
#include <stdio.h>

//*************************************************************************
//*************************************************************************
// 									DEFINES SECTION
//*************************************************************************
//*************************************************************************

#define RFXCOM_X10_DEVICES			255		// Num of X10RF devices managed
#define RFXCOM_SEC_DEVICES			255		// Num of SECURITY sensors managed
#define RFXCOM_ORE_DEVICES			100		// Num of OREGON sensors managed
#define RFXCOM_HE_DEVICES			100		// Num of HOMEEASY devices managed
#define RFXCOM_KOP_DEVICES			100		// Num of KOPPLA devices managed
#define RFXCOM_RFX_DEVICES			100		// Num of RFXCOM devices managed
#define RFXCOM_XAP_LOCATION_LEN		41		// xAP location text field length + 1
#define RFXCOM_XAP_NAME_LEN			41		// xAP name text field length + 1
#define RFXCOM_XAP_V12_MAX_SENSORS	256		// Max number of sensors for xAP-BSC v1.2
#define RFXCOM_XAP_V13_MAX_SENSORS	65535	// Max number of sensors for xAP-BSC > v1.2

// Shared memory keys
#define SHMKEY_RFXCOMCFG			8008	// Key for share resources


//*************************************************************************
//*************************************************************************
// 									CUSTOM TYPES
//*************************************************************************
//*************************************************************************

typedef struct {							// X10RF device data
	BYTE enabled;							// Enable broadcast data of this device
	char address[4];						// X10 address (format A1..P16)
	int devcaps;							// Device capabilities
	char location[RFXCOM_XAP_LOCATION_LEN];	// Device location text field
	char name[RFXCOM_XAP_NAME_LEN];			// Device name text field
} t_X10_DEVICE;

typedef struct {							// SECURITY device data
	BYTE enabled;							// Enable broadcast data of this sensor
	char address[7];						// Address (Hex in ASCII format)
	int devcaps;							// Device capabilities
	char location[RFXCOM_XAP_LOCATION_LEN];	// Device location text field
	char name[RFXCOM_XAP_NAME_LEN];			// Device name text field
} t_SEC_DEVICE;

typedef struct {							// OREGON device data
	BYTE enabled;							// Enable broadcast data of this sensor
	char address[8];						// Address (Hex in ASCII format)
	int devcaps;							// Device capabilities
	BYTE devconfig;							// Device configuration
	char location[RFXCOM_XAP_LOCATION_LEN];	// Device location text field
	char name[RFXCOM_XAP_NAME_LEN];			// Device name text field
} t_ORE_DEVICE;

typedef struct {							// HOMEEASY device data
	BYTE enabled;							// Enable broadcast data of this device
	char address[9];						// Address (Hex in ASCII format)
	char location[RFXCOM_XAP_LOCATION_LEN];	// Device location text field
	char name[RFXCOM_XAP_NAME_LEN];			// Device name text field
} t_HE_DEVICE;

typedef struct {							// KOPPLA device data
	BYTE enabled;							// Enable broadcast data of this device
	char address[3];						// Address (format 1..10)
	char location[RFXCOM_XAP_LOCATION_LEN];	// Device location text field
	char name[RFXCOM_XAP_NAME_LEN];			// Device name text field
} t_KOP_DEVICE;

typedef struct {							// RFXCOM device data
	BYTE enabled;							// Enable broadcast data of this sensor
	char address[5];						// Address (Hex in ASCII format)
	BYTE devcaps;							// Device capabilities
	BYTE devconfig;							// Device configuration
	char location[RFXCOM_XAP_LOCATION_LEN];	// Device location text field
	char name[RFXCOM_XAP_NAME_LEN];			// Device name text field
} t_RFX_DEVICE;

// Config structure
typedef struct {
	BYTE enabled;							// Enable connector
	char commport[20];						// Rfxcom serial port (/dev/ttyUSB0, ...)
	char x10_location[RFXCOM_XAP_LOCATION_LEN];	// X10 location for xAP messages
	char x10_name[RFXCOM_XAP_NAME_LEN];		// X10 name for xAP messages
	BYTE disable_x10;						// Disable X10
	BYTE disable_oregon;					// Disable Oregon
	BYTE disable_arc;						// Disable ARC
	BYTE disable_visonic;					// Disable Visonic
	BYTE disable_ati;						// Disable ATI Wonder
	BYTE disable_koppla;					// Disable Koppla
	BYTE disable_heasyUK;					// Disable Home Easy UK
	BYTE disable_heasyEU;					// Disable Home Easy EU
	BYTE disable_x10_xapbsc_sch;			// Disable xAP-BSC schema for X10RF
	BYTE disable_x10_xapx10_sch;			// Disable xAP-X10 schema for X10RF
	BYTE disable_x10_msgqueue;				// Disable the message queue for X10RF
	char interfacename[20];					// Hardware interface name (eth0, ...)
	USHORT xap_ver;							// xAP version used
	xAP_ADDRESS xap_addr;					// xAP address structure
	USHORT xap_uid;							// xAP UID
	USHORT xap_port;						// xAP Port (Broadcast)
	USHORT xap_hbeat;						// xAP Heartbeat interval
	USHORT xap_infof;						// xAP Info messages interval
	USHORT xap_wserv;						// xAP Web service interval
	t_X10_DEVICE x10_device[RFXCOM_X10_DEVICES];	// X10RF device list
	t_SEC_DEVICE sec_device[RFXCOM_SEC_DEVICES];	// SECURITY device list
	t_ORE_DEVICE ore_device[RFXCOM_ORE_DEVICES];	// OREGON device list
	t_HE_DEVICE he_device[RFXCOM_HE_DEVICES];		// HOMEEASY device list
	t_KOP_DEVICE kop_device[RFXCOM_KOP_DEVICES];	// KOPPLA device list
	t_RFX_DEVICE rfx_device[RFXCOM_RFX_DEVICES];	// RFXCOM device list
	BYTE saveFlag;							// Flag to save settings
	BYTE restartFlag;						// Flag to restart application
} t_RFXCOM_CONFIG;

//*************************************************************************
//*************************************************************************
// 									GLOBAL VARIABLES
//*************************************************************************
//*************************************************************************

int shmRfxcomConfig;						// Shared memory area for config
t_RFXCOM_CONFIG *rfxcomConfig;				// Shared application config structure

//*************************************************************************
//*************************************************************************
// 									GLOBAL FUNCTIONS
//*************************************************************************
//*************************************************************************

short int rfxcomSharedMemSetup(void);
void rfxcomSharedMemClose(void);
