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

	26/04/12 by Jose Luis Galindo : First version
	
***************************************************************************/

//*************************************************************************
//*************************************************************************
// 									INCLUDE SECTION
//*************************************************************************
//*************************************************************************

#include "libxap/libxap.h"
#include <stdio.h>
#include <sys/shm.h>

//*************************************************************************
//*************************************************************************
// 									DEFINES SECTION
//*************************************************************************
//*************************************************************************

#define CC_SENSORS					10		// Num of sensors managed from Currentcost
#define CC_LOCATION_LEN				41		// Sensor location text field length + 1
#define CC_NAME_LEN					41		// Sensor name text field length + 1
#define CC_UNITS_LEN				11		// Sensor units text field length + 1
#define CC_DESC_LEN					51		// Sensor description text field length + 1

// Shared memory keys
#define SHMKEY_CCCFG				8003	// Key for share resources

//*************************************************************************
//*************************************************************************
// 									CUSTOM TYPES
//*************************************************************************
//*************************************************************************

typedef struct {							// CurrentCost sensor data
	int enabled;							// Enable broadcast data of this sensor
	int stype;								// Sensor type (1=elec, 2=impulse, 3=gas, 4=water)
	char digital;							// Set to TRUE for digital sensors
	int hyst;								// Amount of data that must change before a value is published to the xAP bus
	int ch1;								// Channel 1 data (for type=1,3?,4?)
	int ch1old;								// Old Channel 1 data (for type=1,3?,4?)
	int ch2;								// Channel 2 data (for type=1,3?,4?)
	int ch2old;								// Old Channel 2 data (for type=1,3?,4?)
	int ch3;								// Channel 3 data (for type=1,3?,4?)
	int ch3old;								// Old Channel 3 data (for type=1,3?,4?)
	long imp;								// Meter impulse count (for type=2)
	long impold;							// Old Meter impulse count (for type=2)
	int ipu;								// Meter impulses Per Unit (for type=2)
	char location[CC_LOCATION_LEN];			// Sensor location text field
	char name[CC_NAME_LEN];					// Sensor name text field
	char units[CC_UNITS_LEN];				// Sensor units text field
	char desc[CC_DESC_LEN];					// Sensor description text field
	time_t lastupdate;						// Time of the last update
	int uidsub;								// Subaddress ID for xAP
} t_CC_SENSOR;

typedef struct {							// CurrentCost monitor temp sensor
	float hyst;								// Amount of data that must change before a value is published to the xAP bus
	float temp;								// Sensor temperature value
	float tempold;							// Old Sensor temperature value
	char location[CC_LOCATION_LEN];			// location text field
	char name[CC_NAME_LEN];					// name text field
	char units[CC_UNITS_LEN];				// units text field
	char desc[CC_DESC_LEN];					// description text field
	time_t lastupdate;						// Time of the last update
	int uidsub;								// Subaddress ID for xAP
} t_CC_MONTEMP;

// Config structure
typedef struct {
	char interfacename[20];					// Hardware interface name (eth0, ...)
	USHORT xap_ver;							// xAP version used
	xAP_ADDRESS xap_addr;					// xAP address structure
	USHORT xap_uid;							// xAP UID
	USHORT xap_port;						// xAP Port (Broadcast)
	USHORT xap_hbeat;						// xAP Heartbeat interval
	USHORT xap_infof;						// xAP Info messages interval
	USHORT xap_wserv;						// xAP Web service interval
	BYTE cc_enabled;						// Enable xAP broadcasting of CurrentCost events
	enum {CC128,CLASSIC,ORIGINAL} cc_model;	// CurrentCost device model
	char cc_serial[20];						// CurrentCost serial port (/dev/ttyUSB0, ...)
	t_CC_SENSOR cc_sensor[CC_SENSORS];		// CurrentCost sensor config and data
	t_CC_MONTEMP cc_montemp;				// CurrentCost monitor temp sensor
	BYTE saveFlag;							// Flag to save settings
	BYTE restartFlag;						// Flag to restart application
} t_CC_CONFIG;

//*************************************************************************
//*************************************************************************
// 									GLOBAL VARIABLES
//*************************************************************************
//*************************************************************************

int shmCcConfig;						// Shared memory area for config
t_CC_CONFIG *ccConfig;					// Shared application config structure

//*************************************************************************
//*************************************************************************
// 									GLOBAL FUNCTIONS
//*************************************************************************
//*************************************************************************

short int ccSharedMemSetup(void);
void ccSharedMemClose(void);
