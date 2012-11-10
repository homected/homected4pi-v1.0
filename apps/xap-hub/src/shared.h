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

#include <stdio.h>
#include <sys/shm.h>

//*************************************************************************
//*************************************************************************
// 									DEFINES SECTION
//*************************************************************************
//*************************************************************************

#define XAP_VENDOR_LEN		16			// Length for xAP address fields
#define XAP_DEVICE_LEN		16
#define XAP_INSTANCE_LEN	48

// Shared memory keys
#define SHMKEY_HUBCFG		8001		// Key for share resources

//*************************************************************************
//*************************************************************************
// 									CUSTOM TYPES
//*************************************************************************
//*************************************************************************

#define FALSE				0
#define TRUE				1
#define BYTE				unsigned char
#define USHORT				unsigned short int

typedef struct {						// xAP address: Vendor.Device.Instance
	char vendor[XAP_VENDOR_LEN];		// Vendor name
	char device[XAP_DEVICE_LEN];		// Device
	char instance[XAP_INSTANCE_LEN];	// Instance
} xAP_ADDRESS;

// Config structure
typedef struct {
	char interfacename[20];				// Hardware interface name (eth0, ...)
	xAP_ADDRESS xap_addr;				// xAP address structure
	char xap_uid[9];					// xAP UID
	USHORT xap_port;					// xAP Port (Broadcast)
	USHORT xap_hbeat;					// xAP Heartbeat interval
	BYTE saveFlag;						// Flag to save settings
	BYTE restartFlag;					// Flag to restart application
} xAP_HUB_CONFIG;

//*************************************************************************
//*************************************************************************
// 									GLOBAL VARIABLES
//*************************************************************************
//*************************************************************************

int shmHubConfig;					// Shared memory area for config
xAP_HUB_CONFIG *hubConfig;			// Shared application config structure

//*************************************************************************
//*************************************************************************
// 									GLOBAL FUNCTIONS
//*************************************************************************
//*************************************************************************

short int hubSharedMemSetup(void);
void hubSharedMemClose(void);
