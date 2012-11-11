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

	01/05/12 by Jose Luis Galindo : First version
	
***************************************************************************/

//*************************************************************************
//*************************************************************************
// 									INCLUDE SECTION
//*************************************************************************
//*************************************************************************

#include "libxap/libxap.h"
#include "libcosm/libcosm.h"
#include <stdio.h>
#include <sys/shm.h>

//*************************************************************************
//*************************************************************************
// 									DEFINES SECTION
//*************************************************************************
//*************************************************************************

#define APIKEY_LEN					COSM_APIKEY_LEN	// Max length of API key
#define MAX_DATASTREAMS				255				// Max number of datastreams
#define DSTREAM_ID_LEN				25				// Datastream ID length
#define DSTREAM_XAPSOURCE_LEN		XAP_ADDRESS_LEN	// Datastream xAP source length
#define DSTREAM_XAPCLASS_LEN		XAP_EPVALUE_LEN	// Datastream xAP class max length
#define DSTREAM_XAPSECTION_LEN		XAP_EPVALUE_LEN	// Datastream xAP section max length
#define DSTREAM_XAPKEY_LEN			XAP_KEY_LEN		// Datastream xAP key max length

// Shared memory keys
#define SHMKEY_COSMCFG				8004			// Key for share resources

//*************************************************************************
//*************************************************************************
// 									CUSTOM TYPES
//*************************************************************************
//*************************************************************************

// Datastream config data
typedef struct {								
	BYTE enabled;							// Enable upload data of this datastream
	unsigned int feed;						// Datastream Feed
	char id[DSTREAM_ID_LEN];				// Datastream Id
	char xapsource[DSTREAM_XAPSOURCE_LEN];	// Datastream xAP source
	char xapclass[DSTREAM_XAPCLASS_LEN];	// Datastream xAP class
	char xapsection[DSTREAM_XAPSECTION_LEN];// Datastream xAP section
	char xapkey[DSTREAM_XAPKEY_LEN];		// Datastream xAP key
} t_DATASTREAM;

// Config structure
typedef struct {
	BYTE enabled;							// Enable connector
	char apikey[APIKEY_LEN];				// Cosm API key
	USHORT updatef;							// Update frecuency to Cosm (seconds)
	USHORT num_dstreams;					// Num of datastreams
	t_DATASTREAM dstreams[MAX_DATASTREAMS];	// Datastream triggers
	char interfacename[20];					// Hardware interface name (eth0, ...)
	USHORT xap_ver;							// xAP version used
	xAP_ADDRESS xap_addr;					// xAP address structure
	USHORT xap_uid;							// xAP UID
	USHORT xap_port;						// xAP Port (Broadcast)
	USHORT xap_hbeat;						// xAP Heartbeat interval
	USHORT xap_wserv;						// xAP Web service interval
	BYTE saveFlag;							// Flag to save settings
	BYTE restartFlag;						// Flag to restart application
} t_COSM_CONFIG;

//*************************************************************************
//*************************************************************************
// 									GLOBAL VARIABLES
//*************************************************************************
//*************************************************************************

int shmCosmConfig;						// Shared memory area for config
t_COSM_CONFIG *cosmConfig;				// Shared application config structure

//*************************************************************************
//*************************************************************************
// 									GLOBAL FUNCTIONS
//*************************************************************************
//*************************************************************************

short int cosmSharedMemSetup(void);
void cosmSharedMemClose(void);
