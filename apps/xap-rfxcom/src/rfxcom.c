/**************************************************************************

	rfxcom.c

	Rfxcom connector for xAP protocol

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

	18/02/13 by Jose Luis Galindo : Added RFXCOM sensors
	31/01/13 by Jose Luis Galindo : Added IKEA Koppla
	30/01/13 by Jose Luis Galindo : Added Home Easy
	28/01/13 by Jose Luis Galindo : Added Oregon Scientific
	12/01/13 by Jose Luis Galindo : Added Security (Visonic + X10SEC)
	27/12/12 by Jose Luis Galindo : Added X10RF
	27/11/12 by Jose Luis Galindo : First realease
			 

***************************************************************************/

//#define DEBUG_APP						// Set this to debug to console
#include "shared.h"
#include "rfxcom.h"
#include "X10RF.h"
#include "X10RFMsg.h"
#include "Security.h"
#include "Oregon.h"
#include "homeeasy.h"
#include "koppla.h"
#include "rfxsensor.h"
#include "libprocess/libprocess.h"


//*************************************************************************
//								APPLICATION FUNCTIONS 
//*************************************************************************

// Initialize connector settings
void init() {
	int i;
	
	// Load default xAP values
	strcpy(rfxcomConfig->interfacename, "eth0");			// Default interface name
	strcpy(rfxcomConfig->xap_addr.vendor, XAP_VENDOR);		// Vendor name is a constant string
	strcpy(rfxcomConfig->xap_addr.device, XAP_DEVICE);		// Device name is a constant string
	strcpy(rfxcomConfig->xap_addr.instance, XAP_INSTANCE);	// Default instance name
	rfxcomConfig->xap_ver = XAP_VER_12;						// Default xAP version
	lxap_xap_version = rfxcomConfig->xap_ver;
	g_max_subuid = RFXCOM_XAP_V12_MAX_SENSORS;				// Default max subuid's
	rfxcomConfig->xap_uid = XAP_UID;						// Default UID
	rfxcomConfig->xap_port = XAP_PORT;						// Default Port (Broadcast)
	rfxcomConfig->xap_hbeat = XAP_HBEAT_FREQ;				// Default Heartbeat interval
	rfxcomConfig->xap_infof = XAP_INFO_FREQ;				// Default Info interval
	rfxcomConfig->xap_wserv = XAP_WSERV_FREQ;				// Default Web service interval
	
	// Load default device values
	rfxcomConfig->enabled = FALSE;							// Default enabled value
	strcpy(rfxcomConfig->commport, "/dev/ttyUSB0");			// Default serial port name
	strcpy(rfxcomConfig->x10_location, "main");				// Default location for X10RF xAP messages
	strcpy(rfxcomConfig->x10_name, "X10RF");				// Default name for X10RF xAP messages
	rfxcomConfig->disable_x10 = FALSE;						// Default disable X10 value
	rfxcomConfig->disable_oregon = FALSE;					// Default disable Oregon value
	rfxcomConfig->disable_arc = FALSE;						// Default disable ARC value
	rfxcomConfig->disable_visonic = FALSE;					// Default disable Visonic value
	rfxcomConfig->disable_ati = FALSE;						// Default disable ATI Wonder value
	rfxcomConfig->disable_koppla = FALSE;					// Default disable Koppla value
	rfxcomConfig->disable_heasyUK = FALSE;					// Default disable Home Easy UK value
	rfxcomConfig->disable_heasyEU = FALSE;					// Default disable Home Easy EU value
	rfxcomConfig->disable_x10_xapbsc_sch = FALSE;			// Default disable xAP-BSC for X10RF
	rfxcomConfig->disable_x10_xapx10_sch = FALSE;			// Default disable xAP-X10 for X10RF
	rfxcomConfig->disable_x10_msgqueue = FALSE;				// Default disable the message queue for X10RF
		
	// Init sensor data
	g_curr_sensor_type = RFXCOM_SENSOR_UNKNOWN;
	g_subuid_counter = 1;									// Subaddress 0 is reserved
	g_xapx10_subuid = 1;
	
	// Init X10 devices
	for (i=0; i < RFXCOM_X10_DEVICES; i++)	{				// Default values for X10RF devices
		rfxcomConfig->x10_device[i].enabled = FALSE;
		strcpy(rfxcomConfig->x10_device[i].address, "");
		rfxcomConfig->x10_device[i].devcaps = 0;
		strcpy(rfxcomConfig->x10_device[i].location, "X10");
		sprintf(rfxcomConfig->x10_device[i].name, "dev%d", i);
		g_x10_devices_xapbsc[i].uidsub = 0;
		g_x10_devices_xapbsc[i].state = RFXCOM_XAPBSC_UNKNOWN;
		g_x10_devices_xapbsc[i].value = RFXCOM_XAPBSC_UNKNOWN;
		g_x10_devices_xapbsc[i].allowOnOff = FALSE;
		g_x10_devices_xapbsc[i].allowDimBright = FALSE;
		g_x10_devices_xapbsc[i].allowAllLightsOff = FALSE;
		g_x10_devices_xapbsc[i].allowAllLightsOn = FALSE;
		g_x10_devices_xapbsc[i].hasMemory = FALSE;
		g_x10_devices_xapbsc[i].canOffFromDim = FALSE;
		g_x10_devices_xapbsc[i].actionDimFromOFF = RFXCOM_XAPBSC_ACTION_NONE;
		g_x10_devices_xapbsc[i].actionBrightFromOFF = RFXCOM_XAPBSC_ACTION_NONE;
	}
	g_next_x10_device = 0;
	
	// Init SECURITY devices
	for (i=0; i < RFXCOM_SEC_DEVICES; i++)	{				// Default values for SECURITY devices
		rfxcomConfig->sec_device[i].enabled = FALSE;
		strcpy(rfxcomConfig->sec_device[i].address, "");
		rfxcomConfig->sec_device[i].devcaps = 0;
		strcpy(rfxcomConfig->sec_device[i].location, "security");
		sprintf(rfxcomConfig->sec_device[i].name, "%d", i);
		g_sec_devices_xapbsc[i].uidsub = 0;
		g_sec_devices_xapbsc[i].state = 0xFF;
		g_sec_devices_xapbsc[i].lastState = 0xFF;
	}
	g_next_sec_device = 0;
	
	// Init OREGON devices
	for (i=0; i < RFXCOM_ORE_DEVICES; i++)	{				// Default values for OREGON devices
		rfxcomConfig->ore_device[i].enabled = FALSE;
		strcpy(rfxcomConfig->ore_device[i].address, "");
		rfxcomConfig->ore_device[i].devcaps = 0;
		rfxcomConfig->ore_device[i].devconfig = 0;
		strcpy(rfxcomConfig->ore_device[i].location, "oregon");
		sprintf(rfxcomConfig->ore_device[i].name, "%d", i);
		g_ore_devices_xapbsc[i].uidsub = 0;
		g_ore_devices_xapbsc[i].status_unknown = TRUE;
		g_ore_devices_xapbsc[i].value1 = 0;
		g_ore_devices_xapbsc[i].lastvalue1 = 0;
		g_ore_devices_xapbsc[i].value2 = 0;
		g_ore_devices_xapbsc[i].lastvalue2 = 0;
		g_ore_devices_xapbsc[i].value3 = 0;
		g_ore_devices_xapbsc[i].lastvalue3 = 0;
		g_ore_devices_xapbsc[i].battery = RFXCOM_OREGON_BAT_UNKNOWN;
		g_ore_devices_xapbsc[i].lastbattery = RFXCOM_OREGON_BAT_UNKNOWN;
	}
	g_next_ore_device = 0;
	
	// Init HOMEEASY devices
	for (i=0; i < RFXCOM_HE_DEVICES; i++)	{				// Default values for HOMEEASY devices
		rfxcomConfig->he_device[i].enabled = FALSE;
		strcpy(rfxcomConfig->he_device[i].address, "");
		strcpy(rfxcomConfig->he_device[i].location, "homeeasy");
		sprintf(rfxcomConfig->he_device[i].name, "%d", i);
		g_he_devices_xapbsc[i].uidsub = 0;
		g_he_devices_xapbsc[i].status_unknown = TRUE;
		g_he_devices_xapbsc[i].state = RFXCOM_HOMEEASY_FUNC_NONE;
		g_he_devices_xapbsc[i].laststate = RFXCOM_HOMEEASY_FUNC_NONE;
		g_he_devices_xapbsc[i].level = 0;
		g_he_devices_xapbsc[i].lastlevel = 0;
	}
	g_next_he_device = 0;
	
	// Init KOPPLA devices
	for (i=0; i < RFXCOM_KOP_DEVICES; i++)	{				// Default values for KOPPLA devices
		rfxcomConfig->kop_device[i].enabled = FALSE;
		strcpy(rfxcomConfig->kop_device[i].address, "");
		strcpy(rfxcomConfig->kop_device[i].location, "koppla");
		sprintf(rfxcomConfig->kop_device[i].name, "%d", i);
		g_kop_devices_xapbsc[i].uidsub = 0;
		g_kop_devices_xapbsc[i].state = RFXCOM_KOPPLA_FUNC_UNKNOWN;
		g_kop_devices_xapbsc[i].laststate = RFXCOM_KOPPLA_FUNC_UNKNOWN;
	}
	g_next_kop_device = 0;
	
	// Init RFXCOM devices
	for (i=0; i < RFXCOM_RFX_DEVICES; i++)	{				// Default values for RFXCOM devices
		rfxcomConfig->rfx_device[i].enabled = FALSE;
		strcpy(rfxcomConfig->rfx_device[i].address, "");
		rfxcomConfig->rfx_device[i].devcaps = 0;
		rfxcomConfig->rfx_device[i].devconfig = 0;
		strcpy(rfxcomConfig->rfx_device[i].location, "rfxcom");
		sprintf(rfxcomConfig->rfx_device[i].name, "%d", i);
		g_rfx_devices_xapbsc[i].uidsub = 0;
		g_rfx_devices_xapbsc[i].status_unknown = TRUE;
		g_rfx_devices_xapbsc[i].lastfunc = RFXCOM_RFXSENSOR_FUNC_UNKNOWN;
		g_rfx_devices_xapbsc[i].value1 = 0;
		g_rfx_devices_xapbsc[i].lastvalue1 = 0;
		g_rfx_devices_xapbsc[i].value2 = 0;
		g_rfx_devices_xapbsc[i].lastvalue2 = 0;
		g_rfx_devices_xapbsc[i].value3 = 0;
		g_rfx_devices_xapbsc[i].lastvalue3 = 0;
	}
	g_next_rfx_device = 0;
	
	// Reset flags
	rfxcomConfig->saveFlag = FALSE;
	rfxcomConfig->restartFlag = FALSE;
}

// Get the index of the x10_device list for the housecode and unitcode given
int get_x10_device_index(BYTE hcode, BYTE ucode) {
	char x10_address[4];
	BYTE found = FALSE;
	int i = 0;
	
	// Search for X10 sensor in sensor list
	sprintf(x10_address, "%c%d", hcode, ucode);
	while ((!found) && (i < g_next_x10_device)) {
		if (!strcasecmp(rfxcomConfig->x10_device[i].address, x10_address))
			found = TRUE;
		else
			i++;
	}
	if (found) 
		return(i);
	else
		return(-1);
}

// Update g_x10_devices_xapbsc with information from devcaps
void update_x10_devices_xapbsc_with_devcaps(unsigned int sensor) {
	
	int devcaps = rfxcomConfig->x10_device[sensor].devcaps;
	
	g_x10_devices_xapbsc[sensor].allowOnOff = (devcaps & 0x0001);
	g_x10_devices_xapbsc[sensor].allowDimBright = (devcaps & 0x0002) >> 1;
	g_x10_devices_xapbsc[sensor].allowAllLightsOff = (devcaps & 0x0004) >> 2;
	g_x10_devices_xapbsc[sensor].allowAllLightsOn = (devcaps & 0x0008) >> 3;
	g_x10_devices_xapbsc[sensor].hasMemory = (devcaps & 0x0100) >> 8;
	g_x10_devices_xapbsc[sensor].canOffFromDim = (devcaps & 0x0200) >> 9;
	g_x10_devices_xapbsc[sensor].actionDimFromOFF = (devcaps & 0x0C00) >> 10;
	g_x10_devices_xapbsc[sensor].actionBrightFromOFF = (devcaps & 0x1000) >> 12;
}

// Update xAP-BSC endpoint state for X10RF
short int update_x10_xapbsc_endpoint(BYTE hcode, BYTE ucode, BYTE func, unsigned int sensor) {
	
	// Set endpoint state and level
	switch(func) {
		case X10RF_OFF:
			if (g_x10_devices_xapbsc[sensor].allowOnOff)
				g_x10_devices_xapbsc[sensor].state = RFXCOM_XAPBSC_OFF;
			if (g_x10_devices_xapbsc[sensor].allowDimBright) {
				if (g_x10_devices_xapbsc[sensor].value == RFXCOM_XAPBSC_UNKNOWN)
					g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MIN_VALUE;
				if (!g_x10_devices_xapbsc[sensor].hasMemory)
					g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MIN_VALUE;
			}
			break;
			
		case X10RF_ON:
			if (g_x10_devices_xapbsc[sensor].allowOnOff)
				g_x10_devices_xapbsc[sensor].state = RFXCOM_XAPBSC_ON;
			if (g_x10_devices_xapbsc[sensor].allowDimBright) {
				if (g_x10_devices_xapbsc[sensor].value == RFXCOM_XAPBSC_UNKNOWN)
					g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MAX_VALUE;
				if (!g_x10_devices_xapbsc[sensor].hasMemory)
					g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MAX_VALUE;
			}
			break;
			
		case X10RF_DIM:
			if (g_x10_devices_xapbsc[sensor].allowDimBright) {
				if (g_x10_devices_xapbsc[sensor].state == RFXCOM_XAPBSC_OFF) {
					// Determine action from device capabilities
					switch (g_x10_devices_xapbsc[sensor].actionDimFromOFF) {
						case RFXCOM_XAPBSC_ACTION_VALUE:
							g_x10_devices_xapbsc[sensor].state = RFXCOM_XAPBSC_ON;
							g_x10_devices_xapbsc[sensor].value -= (RFXCOM_XAPBSC_MAX_VALUE / RFXCOM_XAPBSC_NUM_DIMS);
							if ((g_x10_devices_xapbsc[sensor].value < RFXCOM_XAPBSC_MIN_VALUE)||(g_x10_devices_xapbsc[sensor].value > RFXCOM_XAPBSC_MAX_VALUE))
								g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MIN_VALUE;
							break;
						
						case RFXCOM_XAPBSC_ACTION_LOW:
							g_x10_devices_xapbsc[sensor].state = RFXCOM_XAPBSC_ON;
							g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MIN_VALUE + (RFXCOM_XAPBSC_MAX_VALUE / RFXCOM_XAPBSC_NUM_DIMS);
							break;
							
						case RFXCOM_XAPBSC_ACTION_FULL:
							g_x10_devices_xapbsc[sensor].state = RFXCOM_XAPBSC_ON;
							g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MAX_VALUE;
							break;
					}
				}
				else {
					// Calculate new value
					g_x10_devices_xapbsc[sensor].value -= (RFXCOM_XAPBSC_MAX_VALUE / RFXCOM_XAPBSC_NUM_DIMS);
					if ((g_x10_devices_xapbsc[sensor].value < RFXCOM_XAPBSC_MIN_VALUE)||(g_x10_devices_xapbsc[sensor].value > RFXCOM_XAPBSC_MAX_VALUE))
						g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MIN_VALUE;
					if ((g_x10_devices_xapbsc[sensor].value == RFXCOM_XAPBSC_MIN_VALUE) && (!g_x10_devices_xapbsc[sensor].canOffFromDim))
						g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MIN_VALUE + (RFXCOM_XAPBSC_MAX_VALUE / RFXCOM_XAPBSC_NUM_DIMS);
				}
			}
			break;

		case X10RF_BRIGHT:
			if (g_x10_devices_xapbsc[sensor].allowDimBright) {
				if (g_x10_devices_xapbsc[sensor].state == RFXCOM_XAPBSC_OFF) {
					// Determine action from device capabilities
					switch (g_x10_devices_xapbsc[sensor].actionBrightFromOFF) {
						case RFXCOM_XAPBSC_ACTION_VALUE:
							g_x10_devices_xapbsc[sensor].state = RFXCOM_XAPBSC_ON;
							g_x10_devices_xapbsc[sensor].value += (RFXCOM_XAPBSC_MAX_VALUE / RFXCOM_XAPBSC_NUM_DIMS);
							if (g_x10_devices_xapbsc[sensor].value > RFXCOM_XAPBSC_MAX_VALUE)
								g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MAX_VALUE;
							break;
						
						case RFXCOM_XAPBSC_ACTION_LOW:
							g_x10_devices_xapbsc[sensor].state = RFXCOM_XAPBSC_ON;
							g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MIN_VALUE + (RFXCOM_XAPBSC_MAX_VALUE / RFXCOM_XAPBSC_NUM_DIMS);
							break;
							
						case RFXCOM_XAPBSC_ACTION_FULL:
							g_x10_devices_xapbsc[sensor].state = RFXCOM_XAPBSC_ON;
							g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MAX_VALUE;
							break;
					}
				}
				else {
					// Calculate new value
					g_x10_devices_xapbsc[sensor].value += (RFXCOM_XAPBSC_MAX_VALUE / RFXCOM_XAPBSC_NUM_DIMS);
					if (g_x10_devices_xapbsc[sensor].value > RFXCOM_XAPBSC_MAX_VALUE)
						g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MAX_VALUE;
				}
			}
			break;
			
		case X10RF_ALL_LIGHTS_OFF:
			if (g_x10_devices_xapbsc[sensor].allowAllLightsOff)
				g_x10_devices_xapbsc[sensor].state = RFXCOM_XAPBSC_OFF;
			if (g_x10_devices_xapbsc[sensor].allowDimBright) {
				if (g_x10_devices_xapbsc[sensor].value == RFXCOM_XAPBSC_UNKNOWN)
					g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MIN_VALUE;
				if (!g_x10_devices_xapbsc[sensor].hasMemory)
					g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MIN_VALUE;
			}
			break;
			
		case X10RF_ALL_LIGHTS_ON:
			if (g_x10_devices_xapbsc[sensor].allowAllLightsOn)
				g_x10_devices_xapbsc[sensor].state = RFXCOM_XAPBSC_ON;
			if (g_x10_devices_xapbsc[sensor].allowDimBright) {
				if (g_x10_devices_xapbsc[sensor].value == RFXCOM_XAPBSC_UNKNOWN)
					g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MAX_VALUE;
				if (!g_x10_devices_xapbsc[sensor].hasMemory)
					g_x10_devices_xapbsc[sensor].value = RFXCOM_XAPBSC_MAX_VALUE;
			}
			break;
	}
}

// Update xAP-BSC endpoint state for SECURITY
short int update_sec_xapbsc_endpoint(char *addr, BYTE msgcode, unsigned int *sensor) {
	BYTE found = FALSE;
	unsigned int i = 0;
	
	// Search for SECURITY sensor in sensor list
	while ((!found) && (i < g_next_sec_device)) {
		if (!strcasecmp(rfxcomConfig->sec_device[i].address, addr))
			found = TRUE;
		else
			i++;
	}
	if (found) 
		*sensor = i;
	else
		return 0;
		
	// Set endpoint state
	g_sec_devices_xapbsc[i].lastState = g_sec_devices_xapbsc[i].state;
	g_sec_devices_xapbsc[i].state = msgcode;
	return 1;
}

// Update xAP-BSC endpoint state for OREGON
short int update_ore_xapbsc_endpoint(char *addr, float val1, float val2, float val3, BYTE bat, unsigned int *sensor) {
	BYTE found = FALSE;
	unsigned int i = 0;
	
	// Search for ORE sensor in sensor list
	while ((!found) && (i < g_next_ore_device)) {
		if (!strcasecmp(rfxcomConfig->ore_device[i].address, addr))
			found = TRUE;
		else
			i++;
	}
	if (found) 
		*sensor = i;
	else
		return 0;
		
	// Update endpoint values
	g_ore_devices_xapbsc[i].lastvalue1 = g_ore_devices_xapbsc[i].value1;
	g_ore_devices_xapbsc[i].lastvalue2 = g_ore_devices_xapbsc[i].value2;
	g_ore_devices_xapbsc[i].lastvalue3 = g_ore_devices_xapbsc[i].value3;
	g_ore_devices_xapbsc[i].lastbattery = g_ore_devices_xapbsc[i].battery;
	g_ore_devices_xapbsc[i].value1 = val1;
	g_ore_devices_xapbsc[i].value2 = val2;
	g_ore_devices_xapbsc[i].value3 = val3;
	g_ore_devices_xapbsc[i].battery = bat;

	// Remove unknown state flag
	g_ore_devices_xapbsc[i].status_unknown = FALSE;
	
	return 1;
}

// Update xAP-BSC endpoint state for HOMEEASY
short int update_he_xapbsc_endpoint(char *addr, BYTE func, BYTE level, unsigned int *sensor) {
	BYTE found = FALSE;
	unsigned int i = 0;
	
	// Search for HOMEEASY sensor in sensor list
	while ((!found) && (i < g_next_he_device)) {
		if (!strcasecmp(rfxcomConfig->he_device[i].address, addr))
			found = TRUE;
		else
			i++;
	}
	if (found) 
		*sensor = i;
	else
		return 0;
		
	// Update endpoint values
	g_he_devices_xapbsc[i].laststate = g_he_devices_xapbsc[i].state;
	g_he_devices_xapbsc[i].lastlevel = g_he_devices_xapbsc[i].level;
	switch(func) {
		case RFXCOM_HOMEEASY_FUNC_OFF:
		case RFXCOM_HOMEEASY_FUNC_ON:
			g_he_devices_xapbsc[i].state = func;
			break;
			
		case RFXCOM_HOMEEASY_FUNC_PRCMD:
			g_he_devices_xapbsc[i].state = func;
			g_he_devices_xapbsc[i].level = level;
			break;
			
		case RFXCOM_HOMEEASY_FUNC_GRPOFF:
		case RFXCOM_HOMEEASY_FUNC_GRPON:
		case RFXCOM_HOMEEASY_FUNC_PRGRP:
			break;
	}

	// Remove unknown state flag
	g_he_devices_xapbsc[i].status_unknown = FALSE;
	
	return 1;
}

// Update xAP-BSC endpoint state for KOPPLA
short int update_kop_xapbsc_endpoint(BYTE addr, BYTE func, BYTE count, unsigned int *devid) {
	char address[3];
	BYTE found = FALSE;
	unsigned int i = 0;
	
	// Convert address to ASCII
	if (addr < 10) {
		address[0] = addr + 48;
		address[1] = '\0';
	}
	else {
		address[0] = '1';
		address[1] = '0';
		address[2] = '\0';
	}
		
	// Search for KOPPLA device in device list
	while ((!found) && (i < g_next_kop_device)) {
		if (!strcasecmp(rfxcomConfig->he_device[i].address, address))
			found = TRUE;
		else
			i++;
	}
	if (found) 
		*devid = i;
	else
		return 0;
		
	// Update endpoint values
	g_kop_devices_xapbsc[i].laststate = g_kop_devices_xapbsc[i].state;
	switch(func) {
		case RFXCOM_KOPPLA_FUNC_UP:
			if ((g_he_devices_xapbsc[i].state >= 0) && (g_he_devices_xapbsc[i].state < 10)) {
				if (g_he_devices_xapbsc[i].state + count > 10)
					g_he_devices_xapbsc[i].state = 10;
				else
					g_he_devices_xapbsc[i].state = g_he_devices_xapbsc[i].state + count;
			}
			break;
			
		case RFXCOM_KOPPLA_FUNC_DOWN:
			if ((g_he_devices_xapbsc[i].state > 0) && (g_he_devices_xapbsc[i].state <= 10)) {
				if (g_he_devices_xapbsc[i].state - count < 0)
					g_he_devices_xapbsc[i].state = 0;
				else
					g_he_devices_xapbsc[i].state = g_he_devices_xapbsc[i].state - count;
			}
			break;
			
		default:
			g_kop_devices_xapbsc[i].state = func;
	}
	
	return 1;
}

// Update xAP-BSC endpoint state for RFXCOM
short int update_rfx_xapbsc_endpoint(char *addr, BYTE func, float val1, float val2, float val3, unsigned int *sensor) {
	BYTE found = FALSE;
	unsigned int i = 0;
	
	// Search for RFXCOM sensor in sensor list
	while ((!found) && (i < g_next_rfx_device)) {
		if (!strcasecmp(rfxcomConfig->rfx_device[i].address, addr))
			found = TRUE;
		else
			i++;
	}
	if (found) 
		*sensor = i;
	else
		return 0;

	// Update function
	g_rfx_devices_xapbsc[i].lastfunc = func;
	
	// Update endpoint values
	g_rfx_devices_xapbsc[i].lastvalue1 = g_rfx_devices_xapbsc[i].value1;
	g_rfx_devices_xapbsc[i].lastvalue2 = g_rfx_devices_xapbsc[i].value2;
	g_rfx_devices_xapbsc[i].lastvalue3 = g_rfx_devices_xapbsc[i].value3;
	switch (func) {
		case RFXCOM_RFXSENSOR_FUNC_UNKNOWN:
		case RFXCOM_RFXSENSOR_FUNC_INFO:
		case RFXCOM_RFXSENSOR_FUNC_ERROR:
			return 1;
			break;
			
		case RFXCOM_RFXSENSOR_FUNC_TEMP:
		case RFXCOM_RFXSENSOR_FUNC_ADC:
		case RFXCOM_RFXSENSOR_FUNC_SVOLTAGE:
			g_rfx_devices_xapbsc[i].value1 = val1;
			break;
			
		case RFXCOM_RFXSENSOR_FUNC_MULTI:
		case RFXCOM_RFXSENSOR_FUNC_MULTI_AD:
			g_rfx_devices_xapbsc[i].value1 = val1;
			g_rfx_devices_xapbsc[i].value2 = val2;
			g_rfx_devices_xapbsc[i].value3 = val3;
			break;
	}

	// Remove unknown state flag
	g_rfx_devices_xapbsc[i].status_unknown = FALSE;
	
	return 1;
}

// Send web service message
short int sendxAPWebServiceMsg(char *strMsg) {

	char strUID[XAP_UID_LEN];
	char strIcon[XAP_WEB_FIELD_LEN];	// Web service: Icon
	char strUrl[XAP_WEB_FIELD_LEN];		// Web service: Url
	
	// Build the UID
	xapBuildUid(strUID, rfxcomConfig->xap_uid, 0);	
	
	// Set xAP web service params
	sprintf(strIcon, "http://%s%s", lxap_local_IP_addr, WSERV_ICON);
	sprintf(strUrl, "http://%s", lxap_local_IP_addr);
	
	return xapSendWebService(rfxcomConfig->xap_addr, strUID, WSERV_NAME, WSERV_DESC, rfxcomConfig->xap_addr.instance, strIcon, strUrl, strMsg);
}

// Send a xAP BSC Info message for X10RF
short int sendxAPBscInfoMsgX10(unsigned int sensor) {
	
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	char uid[XAP_UID_LEN];
	char state[XAP_EPVALUE_LEN];
	char level[XAP_EPVALUE_LEN];
	char display[XAP_EPVALUE_LEN];
	
	// Build sub-address
	sprintf(subaddr, "%s.%s", rfxcomConfig->x10_device[sensor].location, rfxcomConfig->x10_device[sensor].name);
	xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
	
	// Build UID
	xapBuildUid(uid, rfxcomConfig->xap_uid, g_x10_devices_xapbsc[sensor].uidsub);
	
	// Build STATE key
	switch (g_x10_devices_xapbsc[sensor].state) {
		case RFXCOM_XAPBSC_OFF:
			strcpy(state, XAP_STATE_OFF);
			break;
		
		case RFXCOM_XAPBSC_ON:
			strcpy(state, XAP_STATE_ON);
			break;
			
		case RFXCOM_XAPBSC_UNKNOWN:
			strcpy(state, XAP_STATE_UNKNOWN);
			break;
			
		default:
			return 0;
			break;
	}

	// Build LEVEL key
	if (g_x10_devices_xapbsc[sensor].value == RFXCOM_XAPBSC_UNKNOWN)
		sprintf(level, "?/%d", RFXCOM_XAPBSC_MAX_VALUE);
	else
		sprintf(level, "%d/%d", g_x10_devices_xapbsc[sensor].value, RFXCOM_XAPBSC_MAX_VALUE);
	
	// Build DISPLAY key
	if (g_x10_devices_xapbsc[sensor].value == RFXCOM_XAPBSC_UNKNOWN)
		sprintf(display, "%s Level at ?%%", rfxcomConfig->x10_device[sensor].address);
	else {
		double level_percent = (g_x10_devices_xapbsc[sensor].value / (double)RFXCOM_XAPBSC_MAX_VALUE) * 100;
		sprintf(display, "%s Level at %.0f%%", rfxcomConfig->x10_device[sensor].address, level_percent);
	}
	
	// Send message
	return (xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT));
}

// Send a xAP BSC Event message for X10RF
short int sendxAPBscEventMsgX10(BYTE hcode, BYTE ucode, BYTE func, unsigned int sensor) {
	
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	char uid[XAP_UID_LEN];
	char state[XAP_EPVALUE_LEN];
	char level[XAP_EPVALUE_LEN];
	char display[XAP_EPVALUE_LEN];
	
	// Build sub-address
	sprintf(subaddr, "%s.%s", rfxcomConfig->x10_device[sensor].location, rfxcomConfig->x10_device[sensor].name);
	xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
	
	// Build UID
	xapBuildUid(uid, rfxcomConfig->xap_uid, g_x10_devices_xapbsc[sensor].uidsub);
		
	// Build STATE key
	switch (g_x10_devices_xapbsc[sensor].state) {
		case RFXCOM_XAPBSC_OFF:
			strcpy(state, XAP_STATE_OFF);
			break;
		
		case RFXCOM_XAPBSC_ON:
			strcpy(state, XAP_STATE_ON);
			break;
			
		case RFXCOM_XAPBSC_UNKNOWN:
			strcpy(state, XAP_STATE_UNKNOWN);
			break;
			
		default:
			return 0;
			break;
	}

	// Build LEVEL key
	sprintf(level, "%d/%d", g_x10_devices_xapbsc[sensor].value, RFXCOM_XAPBSC_MAX_VALUE);
	
	// Build DISPLAY key
	if ((func == X10RF_OFF) || (func == X10RF_ON)) 
		sprintf(display, "%c%d %s", hcode, ucode, state);
	else if ((func == X10RF_DIM) || (func == X10RF_BRIGHT)) {
		double level_percent = (g_x10_devices_xapbsc[sensor].value / (double)RFXCOM_XAPBSC_MAX_VALUE) * 100;
		if (func == X10RF_DIM) 
			sprintf(display, "%c%d DIM to %.0f%%", hcode, ucode, level_percent);
		else
			sprintf(display, "%c%d BRIGHT to %.0f%%", hcode, ucode, level_percent);
	}
	else if ((func == X10RF_ALL_LIGHTS_OFF) || (func == X10RF_ALL_LIGHTS_ON)) {
		 if (func == X10RF_ALL_LIGHTS_OFF) 
			sprintf(display, "%c ALL LIGHTS OFF", hcode);
		else
			sprintf(display, "%c ALL LIGHTS ON", hcode);
	}
	else
		return 0;
	
	// Send message
	return (xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT));
}

// Send a xAP X10 Event message for X10RF
short int sendxAPX10EventMsgX10(BYTE hcode, BYTE ucode, BYTE func) {
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	char uid[XAP_UID_LEN];
	char device[XAP_EPVALUE_LEN];
	char command[XAP_EPVALUE_LEN];
	double count;
	
	// Build sub-address
	sprintf(subaddr, "%s.%s", rfxcomConfig->x10_location, rfxcomConfig->x10_name);
	xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
	
	// Build UID
	xapBuildUid(uid, rfxcomConfig->xap_uid, g_xapx10_subuid);
	
	// Build DEVICE
	if ((func == X10RF_ALL_LIGHTS_OFF) || (func == X10RF_ALL_LIGHTS_OFF))
		sprintf(device, "%c", hcode);
	else
		sprintf(device, "%c%d", hcode, ucode);
	
	// Build COMMAND
	switch(func) {
		case X10RF_OFF:
			strcpy(command, XAP_X10_CMD_OFF);
			break;
		case X10RF_ON:
			strcpy(command, XAP_X10_CMD_ON);
			break;
		case X10RF_DIM:
			strcpy(command, XAP_X10_CMD_DIM);
			break;
		case X10RF_BRIGHT:
			strcpy(command, XAP_X10_CMD_BRIGHT);
			break;
		case X10RF_ALL_LIGHTS_OFF:
			strcpy(command, XAP_X10_CMD_ALL_LIGHTS_OFF);
			break;
		case X10RF_ALL_LIGHTS_ON:
			strcpy(command, XAP_X10_CMD_ALL_LIGHTS_ON);
			break;
	}
	
	// Build COUNT
	if ((func == X10RF_DIM) || (func == X10RF_BRIGHT))
		count = (RFXCOM_XAPBSC_MAX_VALUE/(double)RFXCOM_XAPBSC_NUM_DIMS);
	else
		count = 0; 
	
	// Send message
	return (xapSendX10evn(fulladdr, uid, device, command, (BYTE)count, 0));
}

// Send a xAP BSC Info message for SECURITY
short int sendxAPBscInfoMsgSEC(unsigned int sensor) {
	BYTE currState;
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	int uid_base;
	char uid[XAP_UID_LEN];
	char state[XAP_EPVALUE_LEN];
	char display[XAP_EPVALUE_LEN];

	// Get current state of the sensor
	currState = g_sec_devices_xapbsc[sensor].state;
	
	// Check for status unknown
	if (currState == 0XFF) {
		// Define common status and display text
		strcpy(state, XAP_STATE_UNKNOWN);
		strcpy(display, "Unknown state");
		
		// Create sensor message
		if (secSupportAlertEvents(rfxcomConfig->sec_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.sensor", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_ALERT, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create tamper message
		if (secSupportTamperEvents(rfxcomConfig->sec_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.tamper", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_TAMPER, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create battery low message
		if (secSupportBatteryLowEvents(rfxcomConfig->sec_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.lowbat", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_BATLOW, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create Disarm message
		if (secSupportDisarm(rfxcomConfig->sec_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.disarm", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_DISARM, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create Arm Away message
		if (secSupportArmAway(rfxcomConfig->sec_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.armAway", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_ARMAWAY, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create Arm Home message
		if (secSupportArmHome(rfxcomConfig->sec_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.armHome", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_ARMHOME, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create Panic message
		if (secSupportPanic(rfxcomConfig->sec_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.panic", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_PANIC, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create Lights On message
		if (secSupportLightsOn(rfxcomConfig->sec_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.lightsOn", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_LIGHTSON, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create Lights Off message
		if (secSupportLightsOff(rfxcomConfig->sec_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.lightsOff", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_LIGHTSOFF, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))	return 0;
		}
	}
	else {
		// Check if the message has an alert/normal event
		if (secSupportAlertEvents(rfxcomConfig->sec_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.sensor", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);

			// Build UID
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_ALERT, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);			
			
			// Build STATE and DISPLAY key
			if (secIsAlert(currState)) {
				strcpy(state, XAP_STATE_ON);
				strcpy(display, "Alert event");
			}
			else {
				strcpy(state, XAP_STATE_OFF);
				strcpy(display, "Normal event");
			}

			// Send message
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Check if the message has a tamper open/close event
		if (secSupportTamperEvents(rfxcomConfig->sec_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.tamper", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);

			// Build UID
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_TAMPER, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE and DISPLAY key
			if (secIsTamperOpen(currState)) {
				strcpy(state, XAP_STATE_ON);
				strcpy(display, "Tamper open");
			}
			else {
				strcpy(state, XAP_STATE_OFF);
				strcpy(display, "Tamper close");
			}

			// Send message
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Check if is the message has a battery low event
		if (secSupportBatteryLowEvents(rfxcomConfig->sec_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.lowbat", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);

			// Build UID
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_BATLOW, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
							
			// Build STATE and DISPLAY key
			if (secIsBatteryLow(currState)) {
				strcpy(state, XAP_STATE_ON);
				strcpy(display, "Battery low!");
			}
			else {
				strcpy(state, XAP_STATE_OFF);
				strcpy(display, "Battery good!");
			}

			// Send message
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Check if is the message has a disarm event
		if (secSupportDisarm(rfxcomConfig->sec_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.disarm", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_DISARM, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
				
			// Build STATE and DISPLAY key
			if (secIsDisarm(currState)) {
				strcpy(state, XAP_STATE_ON);
				strcpy(display, "Disarm!");
			}
			else {
				strcpy(state, XAP_STATE_OFF);
				strcpy(display, "");
			}

			// Send message
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Check if is the message has a arm away event
		if (secSupportArmAway(rfxcomConfig->sec_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.armAway", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);

			// Build UID
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_ARMAWAY, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);			
				
			// Build STATE and DISPLAY key
			if (secIsArmAway(currState)) {
				strcpy(state, XAP_STATE_ON);
				strcpy(display, "Arm Away!");
			}
			else {
				strcpy(state, XAP_STATE_OFF);
				strcpy(display, "");
			}

			// Send message
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Check if is the message has a arm home event
		if (secSupportArmHome(rfxcomConfig->sec_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.armHome", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_ARMHOME, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE and DISPLAY key
			if (secIsArmHome(currState)) {
				strcpy(state, XAP_STATE_ON);
				strcpy(display, "Arm Home!");
			}
			else {
				strcpy(state, XAP_STATE_OFF);
				strcpy(display, "");
			}

			// Send message
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Check if is the message has a panic event
		if (secSupportPanic(rfxcomConfig->sec_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.panic", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);

			// Build UID
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_PANIC, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE and DISPLAY key
			if (secIsPanic(currState)) {
				strcpy(state, XAP_STATE_ON);
				strcpy(display, "Panic!");
			}
			else {
				strcpy(state, XAP_STATE_OFF);
				strcpy(display, "");
			}

			// Send message
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Check if is the message has a lights on event
		if (secSupportLightsOn(rfxcomConfig->sec_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.lightsOn", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);

			// Build UID
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_LIGHTSON, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
				
			// Build STATE and DISPLAY key
			if (secIsLightsOn(currState)) {
				strcpy(state, XAP_STATE_ON);
				strcpy(display, "Lights On!");
			}
			else {
				strcpy(state, XAP_STATE_OFF);
				strcpy(display, "");
			}

			// Send message
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Check if is the message has a lights off event
		if (secSupportLightsOff(rfxcomConfig->sec_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.lightsOff", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);

			// Build UID
			uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_LIGHTSOFF, rfxcomConfig->sec_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
				
			// Build STATE and DISPLAY key
			if (secIsLightsOff(currState)) {
				strcpy(state, XAP_STATE_ON);
				strcpy(display, "Lights Off!");
			}
			else {
				strcpy(state, XAP_STATE_OFF);
				strcpy(display, "");
			}

			// Send message
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
		}
	}
	return 1;
}

// Send a xAP BSC Event message for SECURITY
short int sendxAPBscEventMsgSEC(unsigned int sensor) {
	BYTE currState;
	BYTE lastState;
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	int uid_base;
	char uid[XAP_UID_LEN];
	char state[XAP_EPVALUE_LEN];
	char display[XAP_EPVALUE_LEN];
	
	// Get state of the sensor
	currState = g_sec_devices_xapbsc[sensor].state;
	lastState = g_sec_devices_xapbsc[sensor].lastState;
	
	// Check if the message has an alert/normal event
	if (secSupportAlertEvents(rfxcomConfig->sec_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.sensor", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_ALERT, rfxcomConfig->sec_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
	
		// Build STATE and DISPLAY key
		if (secIsAlert(currState)) {
			strcpy(state, XAP_STATE_ON);
			strcpy(display, "Alert event");
		}
		else {
			strcpy(state, XAP_STATE_OFF);
			strcpy(display, "Normal event");
		}
			
		// Send message when endpoint state change
		if ((secIsAlert(currState) != secIsAlert(lastState)) || (secIsNormal(currState) != secIsNormal(lastState)))
			if (!xapSendBSCevn(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if the message has a tamper open/close event
	if (secSupportTamperEvents(rfxcomConfig->sec_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.tamper", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_TAMPER, rfxcomConfig->sec_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
	
		// Build STATE and DISPLAY key
		if (secIsTamperOpen(currState)) {
			strcpy(state, XAP_STATE_ON);
			strcpy(display, "Tamper open");
		}
		else {
			strcpy(state, XAP_STATE_OFF);
			strcpy(display, "Tamper close");
		}
			
		// Send message when endpoint state change
		if ((secIsTamperOpen(currState) != secIsTamperOpen(lastState)) || (secIsTamperClose(currState) != secIsTamperClose(lastState)))
			if (!xapSendBSCevn(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if is the message has a battery low event
	if (secSupportBatteryLowEvents(rfxcomConfig->sec_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.lowbat", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_BATLOW, rfxcomConfig->sec_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
	
		// Build STATE and DISPLAY key
		if (secIsBatteryLow(currState)) {
			strcpy(state, XAP_STATE_ON);
			strcpy(display, "Battery low!");
		}
		else {
			strcpy(state, XAP_STATE_OFF);
			strcpy(display, "Battery good!");
		}
			
		// Send message when endpoint state change
		if ((secIsBatteryLow(currState) != secIsBatteryLow(lastState)) || (secIsBatteryGood(currState) != secIsBatteryGood(lastState)))
			if (!xapSendBSCevn(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if is the message has a disarm event
	if (secSupportDisarm(rfxcomConfig->sec_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.disarm", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_DISARM, rfxcomConfig->sec_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);
	
		// Build STATE and DISPLAY key
		if (secIsDisarm(currState)) {
			strcpy(state, XAP_STATE_ON);
			strcpy(display, "Disarm!");
		}
		else {
			strcpy(state, XAP_STATE_OFF);
			strcpy(display, "");
		}
			
		// Send message when endpoint state change
		if ((secIsDisarm(currState) != secIsDisarm(lastState)) || (secIsNoKeyfob(currState) != secIsNoKeyfob(lastState)))
			if (!xapSendBSCevn(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if is the message has a arm away event
	if (secSupportArmAway(rfxcomConfig->sec_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.armAway", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);

		// Build UID
		uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_ARMAWAY, rfxcomConfig->sec_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);			
				
		// Build STATE and DISPLAY key
		if (secIsArmAway(currState)) {
			strcpy(state, XAP_STATE_ON);
			strcpy(display, "Arm Away!");
		}
		else {
			strcpy(state, XAP_STATE_OFF);
			strcpy(display, "");
		}
			
		// Send message when endpoint state change
		if ((secIsArmAway(currState) != secIsArmAway(lastState)) || (secIsNoKeyfob(currState) != secIsNoKeyfob(lastState)))
			if (!xapSendBSCevn(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if is the message has a arm home event
	if (secSupportArmHome(rfxcomConfig->sec_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.armHome", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);

		// Build UID
		uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_ARMHOME, rfxcomConfig->sec_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);			
				
		// Build STATE and DISPLAY key
		if (secIsArmHome(currState)) {
			strcpy(state, XAP_STATE_ON);
			strcpy(display, "Arm Home!");
		}
		else {
			strcpy(state, XAP_STATE_OFF);
			strcpy(display, "");
		}
			
		// Send message when endpoint state change
		if ((secIsArmHome(currState) != secIsArmHome(lastState)) || (secIsNoKeyfob(currState) != secIsNoKeyfob(lastState)))
			if (!xapSendBSCevn(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if is the message has a panic event
	if (secSupportPanic(rfxcomConfig->sec_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.panic", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);

		// Build UID
		uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_PANIC, rfxcomConfig->sec_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);			
				
		// Build STATE and DISPLAY key
		if (secIsPanic(currState)) {
			strcpy(state, XAP_STATE_ON);
			strcpy(display, "Panic!");
		}
		else {
			strcpy(state, XAP_STATE_OFF);
			strcpy(display, "");
		}
			
		// Send message when endpoint state change
		if ((secIsPanic(currState) != secIsPanic(lastState)) || (secIsNoKeyfob(currState) != secIsNoKeyfob(lastState)))
			if (!xapSendBSCevn(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if is the message has a lights on event
	if (secSupportLightsOn(rfxcomConfig->sec_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.lightsOn", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);

		// Build UID
		uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_LIGHTSON, rfxcomConfig->sec_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);			
				
		// Build STATE and DISPLAY key
		if (secIsLightsOn(currState)) {
			strcpy(state, XAP_STATE_ON);
			strcpy(display, "Lights On!");
		}
		else {
			strcpy(state, XAP_STATE_OFF);
			strcpy(display, "");
		}
			
		// Send message when endpoint state change
		if ((secIsLightsOn(currState) != secIsLightsOn(lastState)) || (secIsNoKeyfob(currState) != secIsNoKeyfob(lastState)))
			if (!xapSendBSCevn(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if is the message has a lights off event
	if (secSupportLightsOff(rfxcomConfig->sec_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.lightsOff", rfxcomConfig->sec_device[sensor].location, rfxcomConfig->sec_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);

		// Build UID
		uid_base = secGetSensorEndpointSubUID(RFXCOM_SECURITY_ENDP_LIGHTSOFF, rfxcomConfig->sec_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_sec_devices_xapbsc[sensor].uidsub + uid_base);			
				
		// Build STATE and DISPLAY key
		if (secIsLightsOff(currState)) {
			strcpy(state, XAP_STATE_ON);
			strcpy(display, "Lights Off!");
		}
		else {
			strcpy(state, XAP_STATE_OFF);
			strcpy(display, "");
		}
			
		// Send message when endpoint state change
		if ((secIsLightsOff(currState) != secIsLightsOff(lastState)) || (secIsNoKeyfob(currState) != secIsNoKeyfob(lastState)))
			if (!xapSendBSCevn(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	return 1;
}

// Send a xAP BSC Info message for OREGON
short int sendxAPBscInfoMsgORE(unsigned int sensor) {
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	int uid_base;
	char uid[XAP_UID_LEN];
	char state[XAP_EPVALUE_LEN];
	char level[XAP_EPVALUE_LEN];
	char display[XAP_EPVALUE_LEN];
	
	// Check for status unknown
	if (g_ore_devices_xapbsc[sensor].status_unknown == TRUE) {
		// Define common status, level and display text
		strcpy(state, XAP_STATE_UNKNOWN);
		strcpy(level, "?");
		strcpy(display, "Unknown state");
		
		// Create low battery message
		if (oreSupport(RFXCOM_OREGON_ENDP_BATLOW, rfxcomConfig->ore_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.lowbat", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_BATLOW, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create temp message
		if (oreSupport(RFXCOM_OREGON_ENDP_TEMP, rfxcomConfig->ore_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.temp", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_TEMP, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create humidity message
		if (oreSupport(RFXCOM_OREGON_ENDP_HUM, rfxcomConfig->ore_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.hum", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_HUM, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create pressure message
		if (oreSupport(RFXCOM_OREGON_ENDP_BARO, rfxcomConfig->ore_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.baro", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_BARO, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create rain fall rate message
		if (oreSupport(RFXCOM_OREGON_ENDP_RAINRATE, rfxcomConfig->ore_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.rain.fallrate", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_RAINRATE, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create rain total message
		if (oreSupport(RFXCOM_OREGON_ENDP_RAINALL, rfxcomConfig->ore_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.rain.total", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_RAINALL, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create wind direction message
		if (oreSupport(RFXCOM_OREGON_ENDP_WINDDIR, rfxcomConfig->ore_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.wind.direction", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_WINDDIR, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create wind speed message
		if (oreSupport(RFXCOM_OREGON_ENDP_WINDSPD, rfxcomConfig->ore_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.wind.speed", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_WINDSPD, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
			
		// Create wind average speed message
		if (oreSupport(RFXCOM_OREGON_ENDP_WINDAVG, rfxcomConfig->ore_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.wind.speedavg", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_WINDAVG, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
	
		// Create UV factor message
		if (oreSupport(RFXCOM_OREGON_ENDP_UV, rfxcomConfig->ore_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.uv", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_UV, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
		
		// Create Weight message
		if (oreSupport(RFXCOM_OREGON_ENDP_WEIGHT, rfxcomConfig->ore_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.weight", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_WEIGHT, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
	
		// Create phase1 message
		if (oreSupport(RFXCOM_OREGON_ENDP_PHASE1, rfxcomConfig->ore_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.elec.phase1", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_PHASE1, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
	
		// Create phase2 message
		if (oreSupport(RFXCOM_OREGON_ENDP_PHASE2, rfxcomConfig->ore_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.elec.phase2", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_PHASE2, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
	
		// Create phase3 message
		if (oreSupport(RFXCOM_OREGON_ENDP_PHASE3, rfxcomConfig->ore_device[sensor].devcaps)) {
			sprintf(subaddr, "%s.%s.elec.phase3", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_PHASE3, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
	}
	else {
		// Create low battery message
		if (oreSupport(RFXCOM_OREGON_ENDP_BATLOW, rfxcomConfig->ore_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.lowbat", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_BATLOW, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);

			// Build STATE, LEVEL and DISPLAY key
			if (g_ore_devices_xapbsc[sensor].battery == RFXCOM_OREGON_BAT_OK) {
				strcpy(state, XAP_STATE_OFF);
				level[0] = '\0';
				strcpy(display, "Battery good");
			}
			else if (g_ore_devices_xapbsc[sensor].battery == RFXCOM_OREGON_BAT_LOW) {
				strcpy(state, XAP_STATE_ON);
				level[0] = '\0';
				strcpy(display, "Battery low!");
			}
			else if (g_ore_devices_xapbsc[sensor].battery == RFXCOM_OREGON_BAT_UNKNOWN) {
				strcpy(state, XAP_STATE_UNKNOWN);
				level[0] = '\0';
				strcpy(display, "Battery status unknown");
			}
			else {
				if (g_ore_devices_xapbsc[sensor].battery > 10) {
					strcpy(state, XAP_STATE_OFF);
					sprintf(level, "%d", g_ore_devices_xapbsc[sensor].battery);
					sprintf(display, "Battery charge at %d%%", g_ore_devices_xapbsc[sensor].battery);
				}
				else {
					strcpy(state, XAP_STATE_ON);
					sprintf(level, "%d", g_ore_devices_xapbsc[sensor].battery);
					sprintf(display, "Battery charge at %d%%", g_ore_devices_xapbsc[sensor].battery);
				}
			}
			
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Create temp message
		if (oreSupport(RFXCOM_OREGON_ENDP_TEMP, rfxcomConfig->ore_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.temp", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_TEMP, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE, LEVEL and DISPLAY key
			strcpy(state, XAP_STATE_ON);
			if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_TEMPF) {
				sprintf(level, "%.1f", (g_ore_devices_xapbsc[sensor].value1 * 1.8 + 32));
				sprintf(display, "Temp is %.1f°F", (g_ore_devices_xapbsc[sensor].value1 * 1.8 + 32));
			}
			else {
				sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value1);
				sprintf(display, "Temp is %.1f°C", g_ore_devices_xapbsc[sensor].value1);
			}
			
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Create humidity message
		if (oreSupport(RFXCOM_OREGON_ENDP_HUM, rfxcomConfig->ore_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.hum", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_HUM, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE, LEVEL and DISPLAY key
			strcpy(state, XAP_STATE_ON);
			sprintf(level, "%.0f", g_ore_devices_xapbsc[sensor].value2);
			sprintf(display, "Humidity is %.0f%%", g_ore_devices_xapbsc[sensor].value2);
		
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Create pressure message
		if (oreSupport(RFXCOM_OREGON_ENDP_BARO, rfxcomConfig->ore_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.baro", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_BARO, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE, LEVEL and DISPLAY key
			strcpy(state, XAP_STATE_ON);
			if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_PRESSBAR) {
				sprintf(level, "%.3f", g_ore_devices_xapbsc[sensor].value3 / 0.001);
				sprintf(display, "Barometric pressure is %.3f Bar", g_ore_devices_xapbsc[sensor].value3 / 0.001);
			}
			else {
				sprintf(level, "%.0f", g_ore_devices_xapbsc[sensor].value3);
				sprintf(display, "Barometric pressure is %.0f hPA", g_ore_devices_xapbsc[sensor].value3);
			}
			
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Create rain fall rate message
		if (oreSupport(RFXCOM_OREGON_ENDP_RAINRATE, rfxcomConfig->ore_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.rain.fallrate", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_RAINRATE, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE, LEVEL and DISPLAY key
			strcpy(state, XAP_STATE_ON);
			if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_RAININCH) {
				sprintf(level, "%.3f", g_ore_devices_xapbsc[sensor].value1 / 25.4);
				sprintf(display, "Rain fall rate is %.3f inch//hr", g_ore_devices_xapbsc[sensor].value1 / 25.4);
			}
			else {
				sprintf(level, "%.3f", g_ore_devices_xapbsc[sensor].value1);
				sprintf(display, "Rain fall rate is %.3f mm//hr", g_ore_devices_xapbsc[sensor].value1);
			}
			
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Create rain total message
		if (oreSupport(RFXCOM_OREGON_ENDP_RAINALL, rfxcomConfig->ore_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.rain.total", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_RAINALL, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE, LEVEL and DISPLAY key
			strcpy(state, XAP_STATE_ON);
			if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_RAININCH) {
				sprintf(level, "%.3f", g_ore_devices_xapbsc[sensor].value2 / 25.4);
				sprintf(display, "Rain total is %.3f inch", g_ore_devices_xapbsc[sensor].value2 / 25.4);
			}
			else {
				sprintf(level, "%.3f", g_ore_devices_xapbsc[sensor].value2);
				sprintf(display, "Rain total is %.3f mm", g_ore_devices_xapbsc[sensor].value2);
			}
			
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Create wind direction message
		if (oreSupport(RFXCOM_OREGON_ENDP_WINDDIR, rfxcomConfig->ore_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.wind.direction", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_WINDDIR, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE, LEVEL and DISPLAY key
			strcpy(state, XAP_STATE_ON);
			sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value1);
			sprintf(display, "Wind direction is %.1f°", g_ore_devices_xapbsc[sensor].value1);
		
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Create wind speed message
		if (oreSupport(RFXCOM_OREGON_ENDP_WINDSPD, rfxcomConfig->ore_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.wind.speed", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_WINDSPD, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE, LEVEL and DISPLAY key
			strcpy(state, XAP_STATE_ON);
			if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_WINDMILLE) {
				if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_WINDHR) {
					sprintf(level, "%.2f", g_ore_devices_xapbsc[sensor].value2 * 2.24);
					sprintf(display, "Wind speed is %.2f milles//hr", g_ore_devices_xapbsc[sensor].value2 * 2.24);
				}
				else {
					sprintf(level, "%.2f", g_ore_devices_xapbsc[sensor].value2 * 0.00621);
					sprintf(display, "Wind speed is %.2f milles//s", g_ore_devices_xapbsc[sensor].value2 * 0.00621);
				}
			}
			else {
				if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_WINDHR) {
					sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value2 * 3.6);
					sprintf(display, "Wind speed is %.1f km//hr", g_ore_devices_xapbsc[sensor].value2 * 3.6);
				}
				else {
					sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value2);
					sprintf(display, "Wind speed is %.1f m//s", g_ore_devices_xapbsc[sensor].value2);
				}
			}
			
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
			
		// Create wind average speed message
		if (oreSupport(RFXCOM_OREGON_ENDP_WINDAVG, rfxcomConfig->ore_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.wind.speedavg", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_WINDAVG, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE, LEVEL and DISPLAY key
			strcpy(state, XAP_STATE_ON);
			if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_WINDMILLE) {
				if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_WINDHR) {
					sprintf(level, "%.2f", g_ore_devices_xapbsc[sensor].value3 * 2.24);
					sprintf(display, "Wind average speed is %.2f milles//hr", g_ore_devices_xapbsc[sensor].value3 * 2.24);
				}
				else {
					sprintf(level, "%.2f", g_ore_devices_xapbsc[sensor].value3 * 0.00621);
					sprintf(display, "Wind average speed is %.2f milles//s", g_ore_devices_xapbsc[sensor].value3 * 0.00621);
				}
			}
			else {
				if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_WINDHR) {
					sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value3 * 3.6);
					sprintf(display, "Wind average speed is %.1f km//hr", g_ore_devices_xapbsc[sensor].value3 * 3.6);
				}
				else {
					sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value3);
					sprintf(display, "Wind average speed is %.1f m//s", g_ore_devices_xapbsc[sensor].value3);
				}
			}
			
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
	
		// Create UV factor message
		if (oreSupport(RFXCOM_OREGON_ENDP_UV, rfxcomConfig->ore_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.uv", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_UV, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE, LEVEL and DISPLAY key
			strcpy(state, XAP_STATE_ON);
			sprintf(level, "%.0f", g_ore_devices_xapbsc[sensor].value1);
			sprintf(display, "UV factor is %.0f", g_ore_devices_xapbsc[sensor].value1);
		
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
		
		// Create Weight message
		if (oreSupport(RFXCOM_OREGON_ENDP_WEIGHT, rfxcomConfig->ore_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.weight", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_WEIGHT, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE, LEVEL and DISPLAY key
			strcpy(state, XAP_STATE_ON);
			if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_WEIGHTLB) {
				sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value1 * 2.2);
				sprintf(display, "Weight is %.1f Lb", g_ore_devices_xapbsc[sensor].value1 * 2.2);
			}
			else {
				sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value1);
				sprintf(display, "Weight is %.1f Kg", g_ore_devices_xapbsc[sensor].value1);
			}
			
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
	
		// Create phase1 message
		if (oreSupport(RFXCOM_OREGON_ENDP_PHASE1, rfxcomConfig->ore_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.elec.phase1", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_PHASE1, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE, LEVEL and DISPLAY key
			strcpy(state, XAP_STATE_ON);
			sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value1);
			sprintf(display, "Energy monitor phase1 is %.1f", g_ore_devices_xapbsc[sensor].value1);
		
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
	
		// Create phase2 message
		if (oreSupport(RFXCOM_OREGON_ENDP_PHASE2, rfxcomConfig->ore_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.elec.phase2", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_PHASE2, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE, LEVEL and DISPLAY key
			strcpy(state, XAP_STATE_ON);
			sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value2);
			sprintf(display, "Energy monitor phase2 is %.1f", g_ore_devices_xapbsc[sensor].value2);
		
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
	
		// Create phase3 message
		if (oreSupport(RFXCOM_OREGON_ENDP_PHASE3, rfxcomConfig->ore_device[sensor].devcaps)) {
			// Build sub-address
			sprintf(subaddr, "%s.%s.elec.phase3", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
			// Build UID
			uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_PHASE3, rfxcomConfig->ore_device[sensor].devcaps);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
			
			// Build STATE, LEVEL and DISPLAY key
			strcpy(state, XAP_STATE_ON);
			sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value3);
			sprintf(display, "Energy monitor phase3 is %.1f", g_ore_devices_xapbsc[sensor].value3);
			
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
	}
	return 1;
}

// Send a xAP BSC Event message for OREGON
short int sendxAPBscEventMsgORE(unsigned int sensor) {
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	int uid_base;
	char uid[XAP_UID_LEN];
	char state[XAP_EPVALUE_LEN];
	char level[XAP_EPVALUE_LEN];
	char display[XAP_EPVALUE_LEN];

	// Check if the message has a battery low event
	if (oreSupport(RFXCOM_OREGON_ENDP_BATLOW, rfxcomConfig->ore_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.lowbat", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_BATLOW, rfxcomConfig->ore_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
		
		// Build STATE, LEVEL and DISPLAY key
		if (g_ore_devices_xapbsc[sensor].battery == RFXCOM_OREGON_BAT_OK) {
			strcpy(state, XAP_STATE_OFF);
			level[0] = '\0';
			strcpy(display, "Battery good");
		}
		else if (g_ore_devices_xapbsc[sensor].battery == RFXCOM_OREGON_BAT_LOW) {
			strcpy(state, XAP_STATE_ON);
			level[0] = '\0';
			strcpy(display, "Battery low!");
		}
		else if (g_ore_devices_xapbsc[sensor].battery == RFXCOM_OREGON_BAT_UNKNOWN) {
			strcpy(state, XAP_STATE_UNKNOWN);
			level[0] = '\0';
			strcpy(display, "Battery status unknown");
		}
		else {
			if (g_ore_devices_xapbsc[sensor].battery > 10) {
				strcpy(state, XAP_STATE_OFF);
				sprintf(level, "%d", g_ore_devices_xapbsc[sensor].battery);
				sprintf(display, "Battery charge at %d%%", g_ore_devices_xapbsc[sensor].battery);
			}
			else {
				strcpy(state, XAP_STATE_ON);
				sprintf(level, "%d", g_ore_devices_xapbsc[sensor].battery);
				sprintf(display, "Battery charge at %d%%", g_ore_devices_xapbsc[sensor].battery);
			}
		}
		
		// Send message when endpoint state change
		if (g_ore_devices_xapbsc[sensor].battery != g_ore_devices_xapbsc[sensor].lastbattery)
			if (level[0] == '\0') {
				if (!xapSendBSCevn(fulladdr, uid, NULL, state, display, XAP_BSC_INPUT))
					return 0;
			}
			else
				if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
					return 0;
	}
	
	// Check if the message has a temp event
	if (oreSupport(RFXCOM_OREGON_ENDP_TEMP, rfxcomConfig->ore_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.temp", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_TEMP, rfxcomConfig->ore_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
		
		// Build STATE, LEVEL and DISPLAY key
		strcpy(state, XAP_STATE_ON);
		if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_TEMPF) {
			sprintf(level, "%.1f", (g_ore_devices_xapbsc[sensor].value1 * 1.8 + 32));
			sprintf(display, "Temp is %.1f°F", (g_ore_devices_xapbsc[sensor].value1 * 1.8 + 32));
		}
		else {
			sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value1);
			sprintf(display, "Temp is %.1f°C", g_ore_devices_xapbsc[sensor].value1);
		}
		
		// Send message when endpoint state change
		if (g_ore_devices_xapbsc[sensor].value1 != g_ore_devices_xapbsc[sensor].lastvalue1)
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if the message has a humidity event
	if (oreSupport(RFXCOM_OREGON_ENDP_HUM, rfxcomConfig->ore_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.hum", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_HUM, rfxcomConfig->ore_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
		
		// Build STATE, LEVEL and DISPLAY key
		strcpy(state, XAP_STATE_ON);
		sprintf(level, "%.0f", g_ore_devices_xapbsc[sensor].value2);
		sprintf(display, "Humidity is %.0f%%", g_ore_devices_xapbsc[sensor].value2);
		
		// Send message when endpoint state change
		if (g_ore_devices_xapbsc[sensor].value2 != g_ore_devices_xapbsc[sensor].lastvalue2)
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if the message has a pressure event
	if (oreSupport(RFXCOM_OREGON_ENDP_BARO, rfxcomConfig->ore_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.baro", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_BARO, rfxcomConfig->ore_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
		
		// Build STATE, LEVEL and DISPLAY key
		strcpy(state, XAP_STATE_ON);
		if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_PRESSBAR) {
			sprintf(level, "%.3f", g_ore_devices_xapbsc[sensor].value3 / 0.001);
			sprintf(display, "Barometric pressure is %.3f Bar", g_ore_devices_xapbsc[sensor].value3 / 0.001);
		}
		else {
			sprintf(level, "%.0f", g_ore_devices_xapbsc[sensor].value3);
			sprintf(display, "Barometric pressure is %.0f hPA", g_ore_devices_xapbsc[sensor].value3);
		}
		
		// Send message when endpoint state change
		if (g_ore_devices_xapbsc[sensor].value3 != g_ore_devices_xapbsc[sensor].lastvalue3)
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if the message has a rain fall rate event
	if (oreSupport(RFXCOM_OREGON_ENDP_RAINRATE, rfxcomConfig->ore_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.rain.fallrate", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_RAINRATE, rfxcomConfig->ore_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
		
		// Build STATE, LEVEL and DISPLAY key
		strcpy(state, XAP_STATE_ON);
		if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_RAININCH) {
			sprintf(level, "%.3f", g_ore_devices_xapbsc[sensor].value1 / 25.4);
			sprintf(display, "Rain fall rate is %.3f inch//hr", g_ore_devices_xapbsc[sensor].value1 / 25.4);
		}
		else {
			sprintf(level, "%.3f", g_ore_devices_xapbsc[sensor].value1);
			sprintf(display, "Rain fall rate is %.3f mm//hr", g_ore_devices_xapbsc[sensor].value1);
		}
		
		// Send message when endpoint state change
		if (g_ore_devices_xapbsc[sensor].value1 != g_ore_devices_xapbsc[sensor].lastvalue1)
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
	}

	// Check if the message has a rain total event
	if (oreSupport(RFXCOM_OREGON_ENDP_RAINALL, rfxcomConfig->ore_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.rain.total", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_RAINALL, rfxcomConfig->ore_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
		
		// Build STATE, LEVEL and DISPLAY key
		strcpy(state, XAP_STATE_ON);
		if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_RAININCH) {
			sprintf(level, "%.3f", g_ore_devices_xapbsc[sensor].value2 / 25.4);
			sprintf(display, "Rain total is %.3f inch", g_ore_devices_xapbsc[sensor].value2 / 25.4);
		}
		else {
			sprintf(level, "%.3f", g_ore_devices_xapbsc[sensor].value2);
			sprintf(display, "Rain total is %.3f mm", g_ore_devices_xapbsc[sensor].value2);
		}
		
		// Send message when endpoint state change
		if (g_ore_devices_xapbsc[sensor].value2 != g_ore_devices_xapbsc[sensor].lastvalue2)
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if the message has a wind direction event
	if (oreSupport(RFXCOM_OREGON_ENDP_WINDDIR, rfxcomConfig->ore_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.wind.direction", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_WINDDIR, rfxcomConfig->ore_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
		
		// Build STATE, LEVEL and DISPLAY key
		strcpy(state, XAP_STATE_ON);
		sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value1);
		sprintf(display, "Wind direction is %.1f°", g_ore_devices_xapbsc[sensor].value1);
		
		// Send message when endpoint state change
		if (g_ore_devices_xapbsc[sensor].value1 != g_ore_devices_xapbsc[sensor].lastvalue1)
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if the message has a wind speed event
	if (oreSupport(RFXCOM_OREGON_ENDP_WINDSPD, rfxcomConfig->ore_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.wind.speed", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_WINDSPD, rfxcomConfig->ore_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
		
		// Build STATE, LEVEL and DISPLAY key
		strcpy(state, XAP_STATE_ON);
		if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_WINDMILLE) {
			if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_WINDHR) {
				sprintf(level, "%.2f", g_ore_devices_xapbsc[sensor].value2 * 2.24);
				sprintf(display, "Wind speed is %.2f milles//hr", g_ore_devices_xapbsc[sensor].value2 * 2.24);
			}
			else {
				sprintf(level, "%.2f", g_ore_devices_xapbsc[sensor].value2 * 0.00621);
				sprintf(display, "Wind speed is %.2f milles//s", g_ore_devices_xapbsc[sensor].value2 * 0.00621);
			}
		}
		else {
			if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_WINDHR) {
				sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value2 * 3.6);
				sprintf(display, "Wind speed is %.1f km//hr", g_ore_devices_xapbsc[sensor].value2 * 3.6);
			}
			else {
				sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value2);
				sprintf(display, "Wind speed is %.1f m//s", g_ore_devices_xapbsc[sensor].value2);
			}
		}
		
		// Send message when endpoint state change
		if (g_ore_devices_xapbsc[sensor].value2 != g_ore_devices_xapbsc[sensor].lastvalue2)
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if the message has a wind average speed event
	if (oreSupport(RFXCOM_OREGON_ENDP_WINDAVG, rfxcomConfig->ore_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.wind.speedavg", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_WINDAVG, rfxcomConfig->ore_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
		
		// Build STATE, LEVEL and DISPLAY key
		strcpy(state, XAP_STATE_ON);
		if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_WINDMILLE) {
			if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_WINDHR) {
				sprintf(level, "%.2f", g_ore_devices_xapbsc[sensor].value3 * 2.24);
				sprintf(display, "Wind average speed is %.2f milles//hr", g_ore_devices_xapbsc[sensor].value3 * 2.24);
			}
			else {
				sprintf(level, "%.2f", g_ore_devices_xapbsc[sensor].value3 * 0.00621);
				sprintf(display, "Wind average speed is %.2f milles//s", g_ore_devices_xapbsc[sensor].value3 * 0.00621);
			}
		}
		else {
			if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_WINDHR) {
				sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value3 * 3.6);
				sprintf(display, "Wind average speed is %.1f km//hr", g_ore_devices_xapbsc[sensor].value3 * 3.6);
			}
			else {
				sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value3);
				sprintf(display, "Wind average speed is %.1f m//s", g_ore_devices_xapbsc[sensor].value3);
			}
		}
		
		// Send message when endpoint state change
		if (g_ore_devices_xapbsc[sensor].value3 != g_ore_devices_xapbsc[sensor].lastvalue3)
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if the message has a UV factor event
	if (oreSupport(RFXCOM_OREGON_ENDP_UV, rfxcomConfig->ore_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.uv", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_UV, rfxcomConfig->ore_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
		
		// Build STATE, LEVEL and DISPLAY key
		strcpy(state, XAP_STATE_ON);
		sprintf(level, "%.0f", g_ore_devices_xapbsc[sensor].value1);
		sprintf(display, "UV factor is %.0f", g_ore_devices_xapbsc[sensor].value1);
		
		// Send message when endpoint state change
		if (g_ore_devices_xapbsc[sensor].value1 != g_ore_devices_xapbsc[sensor].lastvalue1)
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if the message has a Weight event
	if (oreSupport(RFXCOM_OREGON_ENDP_WEIGHT, rfxcomConfig->ore_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.weight", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_WEIGHT, rfxcomConfig->ore_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
		
		// Build STATE, LEVEL and DISPLAY key
		strcpy(state, XAP_STATE_ON);
		if (rfxcomConfig->ore_device[sensor].devconfig & RFXCOM_OREGON_CFG_WEIGHTLB) {
			sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value1 * 2.2);
			sprintf(display, "Weight is %.1f Lb", g_ore_devices_xapbsc[sensor].value1 * 2.2);
		}
		else {
			sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value1);
			sprintf(display, "Weight is %.1f Kg", g_ore_devices_xapbsc[sensor].value1);
		}
				
		// Send message when endpoint state change
		if (g_ore_devices_xapbsc[sensor].value1 != g_ore_devices_xapbsc[sensor].lastvalue1)
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if the message has a phase1 event
	if (oreSupport(RFXCOM_OREGON_ENDP_PHASE1, rfxcomConfig->ore_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.elec.phase1", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_PHASE1, rfxcomConfig->ore_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
		
		// Build STATE, LEVEL and DISPLAY key
		strcpy(state, XAP_STATE_ON);
		sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value1);
		sprintf(display, "Energy monitor phase1 is %.1f", g_ore_devices_xapbsc[sensor].value1);
		
		// Send message when endpoint state change
		if (g_ore_devices_xapbsc[sensor].value1 != g_ore_devices_xapbsc[sensor].lastvalue1)
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if the message has a phase2 event
	if (oreSupport(RFXCOM_OREGON_ENDP_PHASE2, rfxcomConfig->ore_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.elec.phase2", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_PHASE2, rfxcomConfig->ore_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
		
		// Build STATE, LEVEL and DISPLAY key
		strcpy(state, XAP_STATE_ON);
		sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value2);
		sprintf(display, "Energy monitor phase2 is %.1f", g_ore_devices_xapbsc[sensor].value2);
		
		// Send message when endpoint state change
		if (g_ore_devices_xapbsc[sensor].value2 != g_ore_devices_xapbsc[sensor].lastvalue2)
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	// Check if the message has a phase3 event
	if (oreSupport(RFXCOM_OREGON_ENDP_PHASE3, rfxcomConfig->ore_device[sensor].devcaps)) {
		// Build sub-address
		sprintf(subaddr, "%s.%s.elec.phase3", rfxcomConfig->ore_device[sensor].location, rfxcomConfig->ore_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
		// Build UID
		uid_base = oreGetSensorEndpointSubUID(RFXCOM_OREGON_ENDP_PHASE3, rfxcomConfig->ore_device[sensor].devcaps);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_ore_devices_xapbsc[sensor].uidsub + uid_base);
		
		// Build STATE, LEVEL and DISPLAY key
		strcpy(state, XAP_STATE_ON);
		sprintf(level, "%.1f", g_ore_devices_xapbsc[sensor].value3);
		sprintf(display, "Energy monitor phase3 is %.1f", g_ore_devices_xapbsc[sensor].value3);
		
		// Send message when endpoint state change
		if (g_ore_devices_xapbsc[sensor].value3 != g_ore_devices_xapbsc[sensor].lastvalue3)
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
	}
	
	return 1;
}

// Send a xAP BSC Info message for HOMEEASY
short int sendxAPBscInfoMsgHE(unsigned int sensor) {
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	char uid[XAP_UID_LEN];
	char state[XAP_EPVALUE_LEN];
	char level[XAP_EPVALUE_LEN];
	char display[XAP_EPVALUE_LEN];
	BYTE showlevel = FALSE;

	// Check for status unknown
	if (g_he_devices_xapbsc[sensor].status_unknown == TRUE) {
		sprintf(subaddr, "%s.%s", rfxcomConfig->he_device[sensor].location, rfxcomConfig->he_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_he_devices_xapbsc[sensor].uidsub);
		strcpy(state, XAP_STATE_UNKNOWN);
		strcpy(level, "?");
		strcpy(display, "Unknown state");
		if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	
			return 0;
	}
	else {
		// Build sub-address
		sprintf(subaddr, "%s.%s", rfxcomConfig->he_device[sensor].location, rfxcomConfig->he_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
		// Build UID
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_he_devices_xapbsc[sensor].uidsub);
			
		// Build STATE, LEVEL and DISPLAY key
		switch(g_he_devices_xapbsc[sensor].state) {
			case RFXCOM_HOMEEASY_FUNC_OFF:
				strcpy(state, XAP_STATE_OFF);
				break;
				
			case RFXCOM_HOMEEASY_FUNC_ON:
				strcpy(state, XAP_STATE_ON);
				break;
			
			case RFXCOM_HOMEEASY_FUNC_PRCMD:
				strcpy(state, XAP_STATE_ON);
				sprintf(level, "%d", g_he_devices_xapbsc[sensor].level);
				sprintf(display, "Preset to level = %d", g_he_devices_xapbsc[sensor].level);
				showlevel = TRUE;
				break;
			
			case RFXCOM_HOMEEASY_FUNC_GRPOFF:
			case RFXCOM_HOMEEASY_FUNC_GRPON:
			case RFXCOM_HOMEEASY_FUNC_PRGRP:
				return 1;
				break;
		}
			
		// Send message
		if (showlevel) {
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
		else {
			if (!xapSendBSCinf(fulladdr, uid, NULL, state, NULL, XAP_BSC_INPUT))
				return 0;
		}
	}
	return 1;
}

// Send a xAP BSC Event message for HOMEEASY
short int sendxAPBscEventMsgHE(unsigned int sensor) {
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	char uid[XAP_UID_LEN];
	char state[XAP_EPVALUE_LEN];
	char level[XAP_EPVALUE_LEN];
	char display[XAP_EPVALUE_LEN];
	BYTE showlevel = FALSE;
	
	// Build sub-address
	sprintf(subaddr, "%s.%s", rfxcomConfig->he_device[sensor].location, rfxcomConfig->he_device[sensor].name);
	xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
	// Build UID
	xapBuildUid(uid, rfxcomConfig->xap_uid, g_he_devices_xapbsc[sensor].uidsub);
	
	// Build STATE, LEVEL and DISPLAY key
	switch(g_he_devices_xapbsc[sensor].state) {
		case RFXCOM_HOMEEASY_FUNC_OFF:
			strcpy(state, XAP_STATE_OFF);
			break;
				
		case RFXCOM_HOMEEASY_FUNC_ON:
			strcpy(state, XAP_STATE_ON);
			break;
			
		case RFXCOM_HOMEEASY_FUNC_PRCMD:
			strcpy(state, XAP_STATE_ON);
			sprintf(level, "%d", g_he_devices_xapbsc[sensor].level);
			sprintf(display, "Preset to level = %d", g_he_devices_xapbsc[sensor].level);
			showlevel = TRUE;
			break;
			
		case RFXCOM_HOMEEASY_FUNC_GRPOFF:
		case RFXCOM_HOMEEASY_FUNC_GRPON:
		case RFXCOM_HOMEEASY_FUNC_PRGRP:
			return 1;
			break;
	}
			
	// Send message when endpoint state change
	if ((g_he_devices_xapbsc[sensor].state != g_he_devices_xapbsc[sensor].laststate) || (g_he_devices_xapbsc[sensor].level != g_he_devices_xapbsc[sensor].lastlevel)) {
		if (showlevel) {
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
		else {
			if (!xapSendBSCevn(fulladdr, uid, NULL, state, NULL, XAP_BSC_INPUT))
				return 0;
		}
	}
	return 1;
}

// Send a xAP BSC Info message for KOPPLA
short int sendxAPBscInfoMsgKOP(unsigned int devid) {
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	char uid[XAP_UID_LEN];
	char state[XAP_EPVALUE_LEN];
	char level[XAP_EPVALUE_LEN];
	char display[XAP_EPVALUE_LEN];

	// Check for status unknown
	if (g_kop_devices_xapbsc[devid].state == RFXCOM_KOPPLA_FUNC_UNKNOWN) {
		sprintf(subaddr, "%s.%s", rfxcomConfig->kop_device[devid].location, rfxcomConfig->kop_device[devid].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_kop_devices_xapbsc[devid].uidsub);
		strcpy(state, XAP_STATE_UNKNOWN);
		strcpy(level, "?");
		strcpy(display, "Unknown state");
		if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	
			return 0;
	}
	else {
		// Build sub-address
		sprintf(subaddr, "%s.%s", rfxcomConfig->kop_device[devid].location, rfxcomConfig->kop_device[devid].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			
		// Build UID
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_kop_devices_xapbsc[devid].uidsub);
			
		// Build STATE, LEVEL and DISPLAY key
		switch(g_kop_devices_xapbsc[devid].state) {
			case RFXCOM_KOPPLA_FUNC_OFF:
				strcpy(state, XAP_STATE_OFF);
				sprintf(level, "%d/10", g_kop_devices_xapbsc[devid].state);
				sprintf(display, "Command OFF");
				break;
				
			case RFXCOM_KOPPLA_FUNC_LEVEL1:
			case RFXCOM_KOPPLA_FUNC_LEVEL2:
			case RFXCOM_KOPPLA_FUNC_LEVEL3:
			case RFXCOM_KOPPLA_FUNC_LEVEL4:
			case RFXCOM_KOPPLA_FUNC_LEVEL5:
			case RFXCOM_KOPPLA_FUNC_LEVEL6:
			case RFXCOM_KOPPLA_FUNC_LEVEL7:
			case RFXCOM_KOPPLA_FUNC_LEVEL8:
			case RFXCOM_KOPPLA_FUNC_LEVEL9:
				strcpy(state, XAP_STATE_ON);
				sprintf(level, "%d/10", g_kop_devices_xapbsc[devid].state);
				sprintf(display, "Preset to level = %d", g_kop_devices_xapbsc[devid].state);
				break;
			
			case RFXCOM_KOPPLA_FUNC_ON:
				strcpy(state, XAP_STATE_ON);
				sprintf(level, "%d/10", g_kop_devices_xapbsc[devid].state);
				sprintf(display, "Command ON");
				break;
			
			case RFXCOM_KOPPLA_FUNC_UP:
			case RFXCOM_KOPPLA_FUNC_DOWN:
			case RFXCOM_KOPPLA_FUNC_PROG:
			case RFXCOM_KOPPLA_FUNC_UNKNOWN:
				return 1;
				break;
				
			default:
				return 0;
		}
			
		// Send message
		if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
			return 0;
	}
	return 1;
}

// Send a xAP BSC Event message for KOPPLA
short int sendxAPBscEventMsgKOP(unsigned int devid) {
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	char uid[XAP_UID_LEN];
	char state[XAP_EPVALUE_LEN];
	char level[XAP_EPVALUE_LEN];
	char display[XAP_EPVALUE_LEN];
	
	// Build sub-address
	sprintf(subaddr, "%s.%s", rfxcomConfig->kop_device[devid].location, rfxcomConfig->kop_device[devid].name);
	xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		
	// Build UID
	xapBuildUid(uid, rfxcomConfig->xap_uid, g_kop_devices_xapbsc[devid].uidsub);
	
	// Build STATE, LEVEL and DISPLAY key
	switch(g_kop_devices_xapbsc[devid].state) {
		case RFXCOM_KOPPLA_FUNC_OFF:
			strcpy(state, XAP_STATE_OFF);
			sprintf(level, "%d/10", g_kop_devices_xapbsc[devid].state);
			sprintf(display, "Command OFF");
			break;
				
		case RFXCOM_KOPPLA_FUNC_LEVEL1:
		case RFXCOM_KOPPLA_FUNC_LEVEL2:
		case RFXCOM_KOPPLA_FUNC_LEVEL3:
		case RFXCOM_KOPPLA_FUNC_LEVEL4:
		case RFXCOM_KOPPLA_FUNC_LEVEL5:
		case RFXCOM_KOPPLA_FUNC_LEVEL6:
		case RFXCOM_KOPPLA_FUNC_LEVEL7:
		case RFXCOM_KOPPLA_FUNC_LEVEL8:
		case RFXCOM_KOPPLA_FUNC_LEVEL9:
			strcpy(state, XAP_STATE_ON);
			sprintf(level, "%d/10", g_kop_devices_xapbsc[devid].state);
			sprintf(display, "Preset to level = %d", g_kop_devices_xapbsc[devid].state);
			break;
			
		case RFXCOM_KOPPLA_FUNC_ON:
			strcpy(state, XAP_STATE_ON);
			sprintf(level, "%d/10", g_kop_devices_xapbsc[devid].state);
			sprintf(display, "Command ON");
			break;
			
		case RFXCOM_KOPPLA_FUNC_UP:
		case RFXCOM_KOPPLA_FUNC_DOWN:
		case RFXCOM_KOPPLA_FUNC_PROG:
		case RFXCOM_KOPPLA_FUNC_UNKNOWN:
			return 1;
			break;
				
		default:
			return 0;
	}
			
	// Send message when endpoint state change
	if (g_kop_devices_xapbsc[devid].state != g_kop_devices_xapbsc[devid].laststate) {
		if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
			return 0;
	}
	return 1;
}

// Send a xAP BSC Info message for RFXCOM
short int sendxAPBscInfoMsgRFX(unsigned int sensor) {
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	int uid_base;
	char uid[XAP_UID_LEN];
	char state[XAP_EPVALUE_LEN];
	char level[XAP_EPVALUE_LEN];
	char display[XAP_EPVALUE_LEN];
	
	// Check for status unknown
	if (g_rfx_devices_xapbsc[sensor].status_unknown == TRUE) {
		
		// Define common status, level and display text
		strcpy(state, XAP_STATE_UNKNOWN);
		strcpy(level, "?");
		strcpy(display, "Unknown state");
		
		// For 1-measure devices
		if ((rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_TEMP) || 
			(rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_ADC) ||
			(rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_SVOLTAGE)) {
			
			// Compose and send message
			sprintf(subaddr, "%s.%s", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_rfx_devices_xapbsc[sensor].uidsub);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
		// For 3-measure devices
		else if ((rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_MULTI) ||
				(rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_MULTI_AD)) {
					
			// Compose and send temp message
			sprintf(subaddr, "%s.%s.temp", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_rfx_devices_xapbsc[sensor].uidsub + 0);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
			
			// Compose and send temp/ADC message
			if (rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_MULTI)
				sprintf(subaddr, "%s.%s.temp", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
			else
				sprintf(subaddr, "%s.%s.adc", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_rfx_devices_xapbsc[sensor].uidsub + 1);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
			
			// Compose and send supply voltage message
			sprintf(subaddr, "%s.%s.supplyvoltage", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_rfx_devices_xapbsc[sensor].uidsub + 2);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
	}
	else {
		// For 1-measure devices
		if ((rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_TEMP) || 
			(rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_ADC) ||
			(rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_SVOLTAGE)) {
			
			// Compose and send temp message
			sprintf(subaddr, "%s.%s", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_rfx_devices_xapbsc[sensor].uidsub);
			
			// Build STATE, LEVEL and DISPLAY key
			strcpy(state, XAP_STATE_ON);
			if (rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_TEMP) {
				if (rfxcomConfig->rfx_device[sensor].devconfig & RFXCOM_RFXSENSOR_CFG_TEMPF) {
					sprintf(level, "%.1f", (g_rfx_devices_xapbsc[sensor].value1 * 1.8 + 32));
					sprintf(display, "Temp is %.1fºF", (g_rfx_devices_xapbsc[sensor].value1 * 1.8 + 32));
				}
				else {
					sprintf(level, "%.1f", g_rfx_devices_xapbsc[sensor].value1);
					sprintf(display, "Temp is %.1fºC", g_rfx_devices_xapbsc[sensor].value1);
				}
			}
			else if (rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_ADC) {
				sprintf(level, "%.1f", g_rfx_devices_xapbsc[sensor].value1);
				sprintf(display, "ADC voltage is %.1fV", g_rfx_devices_xapbsc[sensor].value1);
			}
			else if (rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_SVOLTAGE) {
				sprintf(level, "%.1f", g_rfx_devices_xapbsc[sensor].value1);
				sprintf(display, "Supply voltage is %.1fV", g_rfx_devices_xapbsc[sensor].value1);
			}
			else
				return 0;
			
			// Send message
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
		// For 3-measure devices
		else if ((rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_MULTI) ||
				(rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_MULTI_AD)) {
			
			// Compose and send temp message
			sprintf(subaddr, "%s.%s.temp", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_rfx_devices_xapbsc[sensor].uidsub + 0);
			strcpy(state, XAP_STATE_ON);
			if (rfxcomConfig->rfx_device[sensor].devconfig & RFXCOM_RFXSENSOR_CFG_TEMPF) {
				sprintf(level, "%.1f", (g_rfx_devices_xapbsc[sensor].value1 * 1.8 + 32));
				sprintf(display, "Temp is %.1f°F", (g_rfx_devices_xapbsc[sensor].value1 * 1.8 + 32));
			}
			else {
				sprintf(level, "%.1f", g_rfx_devices_xapbsc[sensor].value1);
				sprintf(display, "Temp is %.1f°C", g_rfx_devices_xapbsc[sensor].value1);
			}
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
			
			// Compose and send temp/ADC message
			if (rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_MULTI)
				sprintf(subaddr, "%s.%s.temp", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
			else
				sprintf(subaddr, "%s.%s.adc", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_rfx_devices_xapbsc[sensor].uidsub + 1);
			strcpy(state, XAP_STATE_ON);
			if (rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_MULTI) {
				if (rfxcomConfig->rfx_device[sensor].devconfig & RFXCOM_RFXSENSOR_CFG_TEMPF) {
					sprintf(level, "%.1f", (g_rfx_devices_xapbsc[sensor].value2 * 1.8 + 32));
					sprintf(display, "Temp is %.1f°F", (g_rfx_devices_xapbsc[sensor].value2 * 1.8 + 32));
				}
				else {
					sprintf(level, "%.1f", g_rfx_devices_xapbsc[sensor].value2);
					sprintf(display, "Temp is %.1f°C", g_rfx_devices_xapbsc[sensor].value2);
				}
			}
			else {
				sprintf(level, "%.1f", g_rfx_devices_xapbsc[sensor].value2);
				sprintf(display, "ADC voltage is %.1fV", g_rfx_devices_xapbsc[sensor].value2);
			}
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
			
			// Compose and send supply voltage message
			sprintf(subaddr, "%s.%s.supplyvoltage", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
			xapBuildUid(uid, rfxcomConfig->xap_uid, g_rfx_devices_xapbsc[sensor].uidsub + 2);
			strcpy(state, XAP_STATE_ON);
			sprintf(level, "%.1f", g_rfx_devices_xapbsc[sensor].value3);
			sprintf(display, "Supply voltage is %.1fV", g_rfx_devices_xapbsc[sensor].value3);
			if (!xapSendBSCinf(fulladdr, uid, level, state, display, XAP_BSC_INPUT))	return 0;
		}
	}
	return 1;
}

// Send a xAP BSC Event message for RFXCOM
short int sendxAPBscEventMsgRFX(unsigned int sensor) {
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	int uid_base;
	char uid[XAP_UID_LEN];
	char state[XAP_EPVALUE_LEN];
	char level[XAP_EPVALUE_LEN];
	char display[XAP_EPVALUE_LEN];

	// For 1-measure devices
	if ((rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_TEMP) || 
		(rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_ADC) ||
		(rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_SVOLTAGE)) {
			
		// Compose and send temp message
		sprintf(subaddr, "%s.%s", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_rfx_devices_xapbsc[sensor].uidsub);
			
		// Build STATE, LEVEL and DISPLAY key
		strcpy(state, XAP_STATE_ON);
		if (rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_TEMP) {
			if (rfxcomConfig->rfx_device[sensor].devconfig & RFXCOM_RFXSENSOR_CFG_TEMPF) {
				sprintf(level, "%.1f", (g_rfx_devices_xapbsc[sensor].value1 * 1.8 + 32));
				sprintf(display, "Temp is %.1fºF", (g_rfx_devices_xapbsc[sensor].value1 * 1.8 + 32));
			}
			else {
				sprintf(level, "%.1f", g_rfx_devices_xapbsc[sensor].value1);
				sprintf(display, "Temp is %.1fºC", g_rfx_devices_xapbsc[sensor].value1);
			}
		}
		else if (rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_ADC) {
			sprintf(level, "%.1f", g_rfx_devices_xapbsc[sensor].value1);
			sprintf(display, "ADC voltage is %.1fV", g_rfx_devices_xapbsc[sensor].value1);
		}
		else if (rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_SVOLTAGE) {
			sprintf(level, "%.1f", g_rfx_devices_xapbsc[sensor].value1);
			sprintf(display, "Supply voltage is %.1fV", g_rfx_devices_xapbsc[sensor].value1);
		}
		else
			return 0;
			
		// Send message when endpoint state change
		if (g_rfx_devices_xapbsc[sensor].value1 != g_rfx_devices_xapbsc[sensor].lastvalue1) {
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
	}
	// For 3-measure devices
	else if ((rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_MULTI) ||
			(rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_MULTI_AD)) {
			
		// Compose temp message
		sprintf(subaddr, "%s.%s.temp", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_rfx_devices_xapbsc[sensor].uidsub + 0);
		strcpy(state, XAP_STATE_ON);
		if (rfxcomConfig->rfx_device[sensor].devconfig & RFXCOM_RFXSENSOR_CFG_TEMPF) {
			sprintf(level, "%.1f", (g_rfx_devices_xapbsc[sensor].value1 * 1.8 + 32));
			sprintf(display, "Temp is %.1f°F", (g_rfx_devices_xapbsc[sensor].value1 * 1.8 + 32));
		}
		else {
			sprintf(level, "%.1f", g_rfx_devices_xapbsc[sensor].value1);
			sprintf(display, "Temp is %.1f°C", g_rfx_devices_xapbsc[sensor].value1);
		}
		
		// Send message when endpoint state change
		if (g_rfx_devices_xapbsc[sensor].value1 != g_rfx_devices_xapbsc[sensor].lastvalue1) {
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
			
		// Compose temp/ADC message
		if (rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_MULTI)
			sprintf(subaddr, "%s.%s.temp", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
		else
			sprintf(subaddr, "%s.%s.adc", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_rfx_devices_xapbsc[sensor].uidsub + 1);
		strcpy(state, XAP_STATE_ON);
		if (rfxcomConfig->rfx_device[sensor].devcaps & RFXCOM_RFXSENSOR_DCAPS_MULTI) {
			if (rfxcomConfig->rfx_device[sensor].devconfig & RFXCOM_RFXSENSOR_CFG_TEMPF) {
				sprintf(level, "%.1f", (g_rfx_devices_xapbsc[sensor].value2 * 1.8 + 32));
				sprintf(display, "Temp is %.1f°F", (g_rfx_devices_xapbsc[sensor].value2 * 1.8 + 32));
			}
			else {
				sprintf(level, "%.1f", g_rfx_devices_xapbsc[sensor].value2);
				sprintf(display, "Temp is %.1f°C", g_rfx_devices_xapbsc[sensor].value2);
			}
		}
		else {
			sprintf(level, "%.1f", g_rfx_devices_xapbsc[sensor].value2);
			sprintf(display, "ADC voltage is %.1fV", g_rfx_devices_xapbsc[sensor].value2);
		}
		
		// Send message when endpoint state change
		if (g_rfx_devices_xapbsc[sensor].value2 != g_rfx_devices_xapbsc[sensor].lastvalue2) {
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
			
		// Compose supply voltage message
		sprintf(subaddr, "%s.%s.supplyvoltage", rfxcomConfig->rfx_device[sensor].location, rfxcomConfig->rfx_device[sensor].name);
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
		xapBuildUid(uid, rfxcomConfig->xap_uid, g_rfx_devices_xapbsc[sensor].uidsub + 2);
		strcpy(state, XAP_STATE_ON);
		sprintf(level, "%.1f", g_rfx_devices_xapbsc[sensor].value3);
		sprintf(display, "Supply voltage is %.1fV", g_rfx_devices_xapbsc[sensor].value3);

		// Send message when endpoint state change
		if (g_rfx_devices_xapbsc[sensor].value3 != g_rfx_devices_xapbsc[sensor].lastvalue3) {
			if (!xapSendBSCevn(fulladdr, uid, level, state, display, XAP_BSC_INPUT))
				return 0;
		}
	}
	
	return 1;
}

// Process XAP received messages
short int processxAPMessage(char *message) {
	
	xAP_HEADER header;					// Header struct to collect values from xapReadHead
	char *pXapStr;						// Pointer to the xAP message string
	char fulladdr[XAP_ADDRESS_LEN];		// Temporary buffer
	char subaddr[XAP_ADDRESS_LEN];		// Temporary buffer
	BYTE msgType;						// Message type
	int i;

	// Extract data from the header
	if ((pXapStr = xapReadHead(message, &header)) != NULL)
	{
		// In case "xapRead", in the xap library, doesn't identify the IP address of the message source,
		// filter by xAP source address, if this message has been sent by our device, ignore it
		xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, NULL);
		if (!strncmp(header.source, fulladdr, strlen(fulladdr)))
			return 1;
			
		// Process xAPBSC.query messages
		if (!strcasecmp(header.class, "xAPBSC.query")) {

			// Is the message addressed to our device?
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, ">");
			if (xapEvalTarget(header.target, fulladdr)) {

				// Search if query is for a X10RF device
				for (i = 0; i < g_next_x10_device; i++) {
					if (rfxcomConfig->x10_device[i].enabled) {
						// Check if query is for this device
						sprintf(subaddr, "%s.%s", rfxcomConfig->x10_device[i].location, rfxcomConfig->x10_device[i].name);
						xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
						if (xapEvalTarget(header.target, fulladdr)) {
							if (!sendxAPBscInfoMsgX10(i)) {
								#ifdef DEBUG_APP
								printf("Impossible to send xAP BSC Info message\n");
								#endif
								logEvent(TRUE, "processxAPMessage: Impossible to send xAP BSC Info message");
							}
						}
					}
				}
				
				// Search if query is for a SECURITY device
				for (i = 0; i < g_next_sec_device; i++) {
					if (rfxcomConfig->sec_device[i].enabled) {
						// Check if query is for this device
						sprintf(subaddr, "%s.%s.>", rfxcomConfig->sec_device[i].location, rfxcomConfig->sec_device[i].name);
						xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
						if (xapEvalTarget(header.target, fulladdr)) {
							if (!sendxAPBscInfoMsgSEC(i)) {
								#ifdef DEBUG_APP
								printf("Impossible to send xAP BSC Info message\n");
								#endif
								logEvent(TRUE, "processxAPMessage: Impossible to send xAP BSC Info message");
							}
						}
					}
				}
				
				// Search if query is for a OREGON device
				for (i = 0; i < g_next_ore_device; i++) {
					if (rfxcomConfig->ore_device[i].enabled) {
						// Check if query is for this device
						sprintf(subaddr, "%s.%s.>", rfxcomConfig->ore_device[i].location, rfxcomConfig->ore_device[i].name);
						xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
						if (xapEvalTarget(header.target, fulladdr)) {
							if (!sendxAPBscInfoMsgORE(i)) {
								#ifdef DEBUG_APP
								printf("Impossible to send xAP BSC Info message\n");
								#endif
								logEvent(TRUE, "processxAPMessage: Impossible to send xAP BSC Info message");
							}
						}
					}
				}
				
				// Search if query is for a HOMEEASY device
				for (i = 0; i < g_next_he_device; i++) {
					if (rfxcomConfig->he_device[i].enabled) {
						// Check if query is for this device
						sprintf(subaddr, "%s.%s.>", rfxcomConfig->he_device[i].location, rfxcomConfig->he_device[i].name);
						xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
						if (xapEvalTarget(header.target, fulladdr)) {
							if (!sendxAPBscInfoMsgHE(i)) {
								#ifdef DEBUG_APP
								printf("Impossible to send xAP BSC Info message\n");
								#endif
								logEvent(TRUE, "processxAPMessage: Impossible to send xAP BSC Info message");
							}
						}
					}
				}
				
				// Search if query is for a KOPPLA device
				for (i = 0; i < g_next_kop_device; i++) {
					if (rfxcomConfig->kop_device[i].enabled) {
						// Check if query is for this device
						sprintf(subaddr, "%s.%s.>", rfxcomConfig->kop_device[i].location, rfxcomConfig->kop_device[i].name);
						xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
						if (xapEvalTarget(header.target, fulladdr)) {
							if (!sendxAPBscInfoMsgKOP(i)) {
								#ifdef DEBUG_APP
								printf("Impossible to send xAP BSC Info message\n");
								#endif
								logEvent(TRUE, "processxAPMessage: Impossible to send xAP BSC Info message");
							}
						}
					}
				}
				
				// Search if query is for a RFXCOM device
				for (i = 0; i < g_next_rfx_device; i++) {
					if (rfxcomConfig->rfx_device[i].enabled) {
						// Check if query is for this device
						sprintf(subaddr, "%s.%s.>", rfxcomConfig->rfx_device[i].location, rfxcomConfig->rfx_device[i].name);
						xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, subaddr);
						if (xapEvalTarget(header.target, fulladdr)) {
							if (!sendxAPBscInfoMsgRFX(i)) {
								#ifdef DEBUG_APP
								printf("Impossible to send xAP BSC Info message\n");
								#endif
								logEvent(TRUE, "processxAPMessage: Impossible to send xAP BSC Info message");
							}
						}
					}
				}
			}
		}
			
		// Process web.service messages
		if (!strcasecmp(header.class, "web.service")) {
						
			// Is the message addressed to our device?
			xapBuildAddress(fulladdr, rfxcomConfig->xap_addr, ">");
			if (xapEvalTarget(header.target, fulladdr)) {
			
				// Read body of the message
				if (!xapReadWebBody(pXapStr, NULL, NULL, NULL, NULL, NULL, &msgType))
					return 0;
					
				// Check if it is a web request message
				if (msgType == WEB_REQUEST) {
					// Send a server.start message
					if (!sendxAPWebServiceMsg("server.start"))
						logError("processxAPMessage: Impossible to send xAP server.start message");
				}
			}
		}
	}

	return 1;
}

// SAX save settings
int settingsXmlSave(const char *filename) {

	int i;
    int rc;
    xmlTextWriterPtr writer;

    // Create a new XmlWriter with no compression
    writer = xmlNewTextWriterFilename(filename, 0);
    if (writer == NULL) return 1;

    // Start document with xml default encoding ISO 8859-1
    rc = xmlTextWriterStartDocument(writer, NULL, XML_ENCODING, NULL);
    if (rc < 0) return 2;

    // Start root node "settings"
    rc = xmlTextWriterStartElement(writer, BAD_CAST "settings");
    if (rc < 0) return 2;

    // Create child node "device"
    rc = xmlTextWriterStartElement(writer, BAD_CAST "device");
    if (rc < 0) return 2;

    // Write <device> elements
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", rfxcomConfig->xap_addr.instance);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "enabled", "%d", rfxcomConfig->enabled);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "commport", "%s", rfxcomConfig->commport);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "x10_location", "%s", rfxcomConfig->x10_location);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "x10_name", "%s", rfxcomConfig->x10_name);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "disable_x10", "%d", rfxcomConfig->disable_x10);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "disable_oregon", "%d", rfxcomConfig->disable_oregon);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "disable_arc", "%d", rfxcomConfig->disable_arc);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "disable_visonic", "%d", rfxcomConfig->disable_visonic);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "disable_ati", "%d", rfxcomConfig->disable_ati);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "disable_koppla", "%d", rfxcomConfig->disable_koppla);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "disable_heasyUK", "%d", rfxcomConfig->disable_heasyUK);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "disable_heasyEU", "%d", rfxcomConfig->disable_heasyEU);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "disable_x10_xapbsc_sch", "%d", rfxcomConfig->disable_x10_xapbsc_sch);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "disable_x10_xapx10_sch", "%d", rfxcomConfig->disable_x10_xapx10_sch);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "disable_x10_msgqueue", "%d", rfxcomConfig->disable_x10_msgqueue);
    if (rc < 0) return 4;

    // Close node "device"
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) return 5;

	// Create child node "xap"
    rc = xmlTextWriterStartElement(writer, BAD_CAST "xap");
    if (rc < 0) return 2;
    
	// Write <xap> elements
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "iface", "%s", rfxcomConfig->interfacename);
	if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "version", "%d", rfxcomConfig->xap_ver);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "hbeatf", "%d", rfxcomConfig->xap_hbeat);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "infof", "%d", rfxcomConfig->xap_infof);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "wservf", "%d", rfxcomConfig->xap_wserv);
    if (rc < 0) return 4;
    if (rfxcomConfig->xap_ver == XAP_VER_12)
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "uid", "%04X", rfxcomConfig->xap_uid);
	else 
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "uid", "%08X", rfxcomConfig->xap_uid);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "port", "%d", rfxcomConfig->xap_port);
    if (rc < 0) return 4;        
    
    // Close node "xap"
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) return 5;
    
    // Create child node "sensors"
    rc = xmlTextWriterStartElement(writer, BAD_CAST "sensors");
    if (rc < 0) return 2;
    
	// Write each <sensor-x10>
	for (i=0; i < g_next_x10_device; i++)
	{
		// Create child node
		rc = xmlTextWriterStartElement(writer, BAD_CAST "sensor-x10");
		if (rc < 0) return 2;
		
	    // Add an attribute with the address of sensor
		rc = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "address", "%s", rfxcomConfig->x10_device[i].address);
		if (rc < 0) return 3;
    
		// Write node elements
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "enabled", "%d", rfxcomConfig->x10_device[i].enabled);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "devcaps", "%d", rfxcomConfig->x10_device[i].devcaps);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "location", "%s", rfxcomConfig->x10_device[i].location);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", rfxcomConfig->x10_device[i].name);
		if (rc < 0) return 4;
		
		// Close node
		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) return 5;
	}
	
	// Write each <sensor-security>
	for (i=0; i < g_next_sec_device; i++)
	{
		// Create child node
		rc = xmlTextWriterStartElement(writer, BAD_CAST "sensor-security");
		if (rc < 0) return 2;
		
	    // Add an attribute with the address of sensor
		rc = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "address", "%s", rfxcomConfig->sec_device[i].address);
		if (rc < 0) return 3;
    
		// Write node elements
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "enabled", "%d", rfxcomConfig->sec_device[i].enabled);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "devcaps", "%d", rfxcomConfig->sec_device[i].devcaps);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "location", "%s", rfxcomConfig->sec_device[i].location);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", rfxcomConfig->sec_device[i].name);
		if (rc < 0) return 4;
		
		// Close node
		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) return 5;
	}
    
    // Write each <sensor-oregon>
	for (i=0; i < g_next_ore_device; i++)
	{
		// Create child node
		rc = xmlTextWriterStartElement(writer, BAD_CAST "sensor-oregon");
		if (rc < 0) return 2;
		
	    // Add an attribute with the address of sensor
		rc = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "address", "%s", rfxcomConfig->ore_device[i].address);
		if (rc < 0) return 3;
    
		// Write node elements
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "enabled", "%d", rfxcomConfig->ore_device[i].enabled);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "devcaps", "%d", rfxcomConfig->ore_device[i].devcaps);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "devconfig", "%d", rfxcomConfig->ore_device[i].devconfig);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "location", "%s", rfxcomConfig->ore_device[i].location);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", rfxcomConfig->ore_device[i].name);
		if (rc < 0) return 4;
		
		// Close node
		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) return 5;
	}
	
	// Write each <sensor-homeeasy>
	for (i=0; i < g_next_he_device; i++)
	{
		// Create child node
		rc = xmlTextWriterStartElement(writer, BAD_CAST "sensor-homeeasy");
		if (rc < 0) return 2;
		
	    // Add an attribute with the address of sensor
		rc = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "address", "%s", rfxcomConfig->he_device[i].address);
		if (rc < 0) return 3;
    
		// Write node elements
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "enabled", "%d", rfxcomConfig->he_device[i].enabled);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "location", "%s", rfxcomConfig->he_device[i].location);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", rfxcomConfig->he_device[i].name);
		if (rc < 0) return 4;
		
		// Close node
		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) return 5;
	}
	
	// Write each <sensor-koppla>
	for (i=0; i < g_next_kop_device; i++)
	{
		// Create child node
		rc = xmlTextWriterStartElement(writer, BAD_CAST "sensor-koppla");
		if (rc < 0) return 2;
		
	    // Add an attribute with the address of sensor
		rc = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "address", "%s", rfxcomConfig->kop_device[i].address);
		if (rc < 0) return 3;
    
		// Write node elements
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "enabled", "%d", rfxcomConfig->kop_device[i].enabled);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "location", "%s", rfxcomConfig->kop_device[i].location);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", rfxcomConfig->kop_device[i].name);
		if (rc < 0) return 4;
		
		// Close node
		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) return 5;
	}
	
    // Write each <sensor-rfxcom>
	for (i=0; i < g_next_rfx_device; i++)
	{
		// Create child node
		rc = xmlTextWriterStartElement(writer, BAD_CAST "sensor-rfxcom");
		if (rc < 0) return 2;
		
	    // Add an attribute with the address of sensor
		rc = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "address", "%s", rfxcomConfig->rfx_device[i].address);
		if (rc < 0) return 3;
    
		// Write node elements
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "enabled", "%d", rfxcomConfig->rfx_device[i].enabled);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "devcaps", "%d", rfxcomConfig->rfx_device[i].devcaps);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "devconfig", "%d", rfxcomConfig->rfx_device[i].devconfig);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "location", "%s", rfxcomConfig->rfx_device[i].location);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", rfxcomConfig->rfx_device[i].name);
		if (rc < 0) return 4;
		
		// Close node
		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) return 5;
	}
	
    // Close node "sensors"
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) return 5;
    
    // Close the rest nodes pending to close
    rc = xmlTextWriterEndDocument(writer);
	if (rc < 0) return 5;

	// Free the writer
    xmlFreeTextWriter(writer);
    
    // Cleanup function for the XML library.
    xmlCleanupParser();
    
    // this is to debug memory for regression tests
    xmlMemoryDump();
    
    return 0;
}

// SAX start element (TAG) callback for Xml settings
static void settingsXmlStartElementCallback(void *ctx, const xmlChar *name, const xmlChar **atts) {

	//printf("<%s>\n", name);	// Print start tag

	// DEVICE Tags
	if(strcmp("device", (char*) name) == 0) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
        return;
	}
	else if((strcmp("name", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_NAME;
		return;
	}
	else if((strcmp("enabled", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_ENABLED;
		return;
	}
	else if((strcmp("commport", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_COMMPORT;
		return;
	}
	else if((strcmp("x10_location", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_X10LOC;
		return;
	}
	else if((strcmp("x10_name", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_X10NAME;
		return;
	}
	else if((strcmp("disable_x10", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_DISX10;
		return;
	}
	else if((strcmp("disable_oregon", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_DISOREGON;
		return;
	}
	else if((strcmp("disable_arc", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_DISARC;
		return;
	}
	else if((strcmp("disable_visonic", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_DISVISONIC;
		return;
	}
	else if((strcmp("disable_ati", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_DISATI;
		return;
	}
	else if((strcmp("disable_koppla", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_DISKOPPLA;
		return;
	}
	else if((strcmp("disable_heasyUK", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_DISHEASYUK;
		return;
	}
	else if((strcmp("disable_heasyEU", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_DISHEASYEU;
		return;
	}
	else if((strcmp("disable_x10_xapbsc_sch", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_DISX10SBSC;
		return;
	}
	else if((strcmp("disable_x10_xapx10_sch", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_DISX10SX10;
		return;
	}
	else if((strcmp("disable_x10_msgqueue", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_DISX10MSGQ;
		return;
	}
	// XAP Tags
	else if(strcmp("xap", (char*) name) == 0) {
		g_settings_tag = SETTINGS_TAG_XAP;
        return;
	}
	else if((strcmp("iface", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_XAP)) {
		g_settings_tag = SETTINGS_TAG_XAP_IFACE;
		return;
	}
	else if((strcmp("version", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_XAP)) {
		g_settings_tag = SETTINGS_TAG_XAP_VERSION;
		return;
	}
	else if((strcmp("hbeatf", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_XAP)) {
		g_settings_tag = SETTINGS_TAG_XAP_HBEATF;
		return;
	}
	else if((strcmp("infof", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_XAP)) {
		g_settings_tag = SETTINGS_TAG_XAP_INFOF;
		return;
	}
	else if((strcmp("wservf", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_XAP)) {
		g_settings_tag = SETTINGS_TAG_XAP_WSERVF;
		return;
	}
	else if((strcmp("uid", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_XAP)) {
		g_settings_tag = SETTINGS_TAG_XAP_UID;
		return;
	}
	else if((strcmp("port", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_XAP)) {
		g_settings_tag = SETTINGS_TAG_XAP_PORT;
		return;
	}
	// SENSORS Tags
	else if(strcmp("sensors", (char*) name) == 0) {
		g_settings_tag = SETTINGS_TAG_SENSORS;
        return;
	}
	else if((strcmp("sensor-x10", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSORS)) {
		if (atts != NULL) {
			if (atts[1] != NULL) {
				if(strcmp("address", (char*) atts[0]) == 0) {
					if (atts[1] != NULL) {
						g_settings_tag = SETTINGS_TAG_SENSOR;
						strcpy(rfxcomConfig->x10_device[g_next_x10_device].address, (char*) atts[1]);
						g_curr_sensor_type = RFXCOM_SENSOR_X10;
					}
				}
			}
		}
        return;
	}
	else if((strcmp("sensor-security", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSORS)) {
		if (atts != NULL) {
			if (atts[1] != NULL) {
				if(strcmp("address", (char*) atts[0]) == 0) {
					if (atts[1] != NULL) {
						g_settings_tag = SETTINGS_TAG_SENSOR;
						strcpy(rfxcomConfig->sec_device[g_next_sec_device].address, (char*) atts[1]);
						g_curr_sensor_type = RFXCOM_SENSOR_SECURITY;
					}
				}
			}
		}
        return;
	}
	else if((strcmp("sensor-oregon", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSORS)) {
		if (atts != NULL) {
			if (atts[1] != NULL) {
				if(strcmp("address", (char*) atts[0]) == 0) {
					if (atts[1] != NULL) {
						g_settings_tag = SETTINGS_TAG_SENSOR;
						strcpy(rfxcomConfig->ore_device[g_next_ore_device].address, (char*) atts[1]);
						g_curr_sensor_type = RFXCOM_SENSOR_OREGON;
					}
				}
			}
		}
        return;
	}
	else if((strcmp("sensor-homeeasy", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSORS)) {
		if (atts != NULL) {
			if (atts[1] != NULL) {
				if(strcmp("address", (char*) atts[0]) == 0) {
					if (atts[1] != NULL) {
						g_settings_tag = SETTINGS_TAG_SENSOR;
						strcpy(rfxcomConfig->he_device[g_next_he_device].address, (char*) atts[1]);
						g_curr_sensor_type = RFXCOM_SENSOR_HOMEEASY;
					}
				}
			}
		}
        return;
	}
	else if((strcmp("sensor-koppla", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSORS)) {
		if (atts != NULL) {
			if (atts[1] != NULL) {
				if(strcmp("address", (char*) atts[0]) == 0) {
					if (atts[1] != NULL) {
						g_settings_tag = SETTINGS_TAG_SENSOR;
						strcpy(rfxcomConfig->kop_device[g_next_kop_device].address, (char*) atts[1]);
						g_curr_sensor_type = RFXCOM_SENSOR_KOPPLA;
					}
				}
			}
		}
        return;
	}
	else if((strcmp("sensor-rfxcom", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSORS)) {
		if (atts != NULL) {
			if (atts[1] != NULL) {
				if(strcmp("address", (char*) atts[0]) == 0) {
					if (atts[1] != NULL) {
						g_settings_tag = SETTINGS_TAG_SENSOR;
						strcpy(rfxcomConfig->rfx_device[g_next_rfx_device].address, (char*) atts[1]);
						g_curr_sensor_type = RFXCOM_SENSOR_RFXCOM;
					}
				}
			}
		}
        return;
	}
	else if((strcmp("enabled", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSOR)) {
		g_settings_tag = SETTINGS_TAG_SENSOR_ENABLED;
		return;
	}
	else if((strcmp("location", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSOR)) {
		g_settings_tag = SETTINGS_TAG_SENSOR_LOC;
		return;
	}
	else if((strcmp("name", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSOR)) {
		g_settings_tag = SETTINGS_TAG_SENSOR_NAME;
		return;
	}
	else if((strcmp("devcaps", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSOR)) {
		if (g_curr_sensor_type == RFXCOM_SENSOR_X10) {
			g_settings_tag = SETTINGS_TAG_SENSOR_X10CAPS;
			return;
		}
		else if (g_curr_sensor_type == RFXCOM_SENSOR_SECURITY) {
			g_settings_tag = SETTINGS_TAG_SENSOR_SECCAPS;
			return;
		}
		else if (g_curr_sensor_type == RFXCOM_SENSOR_OREGON) {
			g_settings_tag = SETTINGS_TAG_SENSOR_ORECAPS;
			return;
		}
		else if (g_curr_sensor_type == RFXCOM_SENSOR_RFXCOM) {
			g_settings_tag = SETTINGS_TAG_SENSOR_RFXCAPS;
			return;
		}
	}
	else if((strcmp("devconfig", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSOR)) {
		if (g_curr_sensor_type == RFXCOM_SENSOR_OREGON) {
			g_settings_tag = SETTINGS_TAG_SENSOR_ORECFG;
			return;
		}
		else if (g_curr_sensor_type == RFXCOM_SENSOR_RFXCOM) {
			g_settings_tag = SETTINGS_TAG_SENSOR_RFXCFG;
			return;
		}
	}
}

// SAX end element (TAG) callback for Xml settings
static void settingsXmlEndElementCallback(void *ctx, const xmlChar *name) {
	
	//printf("</%s>", name);	// Print end tag

	// DEVICE Tags
	if((strcmp("name", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_NAME) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("enabled", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_ENABLED) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("commport", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_COMMPORT) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("x10_location", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_X10LOC) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("x10_name", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_X10NAME) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("disable_x10", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_DISX10) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("disable_oregon", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_DISOREGON) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("disable_arc", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_DISARC) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("disable_visonic", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_DISVISONIC) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("disable_ati", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_DISATI) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("disable_koppla", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_DISKOPPLA) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("disable_heasyUK", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_DISHEASYUK) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("disable_heasyEU", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_DISHEASYEU) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("disable_x10_xapbsc_sch", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_DISX10SBSC) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("disable_x10_xapx10_sch", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_DISX10SX10) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("disable_x10_msgqueue", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_DISX10MSGQ) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	// XAP Tags
	else if((strcmp("iface", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_XAP_IFACE) {
		g_settings_tag = SETTINGS_TAG_XAP;
		return;
	}
	else if((strcmp("version", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_XAP_VERSION) {
		g_settings_tag = SETTINGS_TAG_XAP;
		return;
	}
	else if((strcmp("hbeatf", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_XAP_HBEATF) {
		g_settings_tag = SETTINGS_TAG_XAP;
		return;
	}
	else if((strcmp("infof", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_XAP_INFOF) {
		g_settings_tag = SETTINGS_TAG_XAP;
		return;
	}
	else if((strcmp("wservf", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_XAP_WSERVF) {
		g_settings_tag = SETTINGS_TAG_XAP;
		return;
	}
	else if((strcmp("uid", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_XAP_UID) {
		g_settings_tag = SETTINGS_TAG_XAP;
		return;
	}
	else if((strcmp("port", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_XAP_PORT) {
		g_settings_tag = SETTINGS_TAG_XAP;
		return;
	}
	// SENSOR TAGS
	else if((strcmp("sensor-x10", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR) {
		g_settings_tag = SETTINGS_TAG_SENSORS;
		if ((g_subuid_counter < g_max_subuid) && (g_next_x10_device < RFXCOM_X10_DEVICES)) {
			g_x10_devices_xapbsc[g_next_x10_device].uidsub = g_subuid_counter;
			g_subuid_counter++;
			g_next_x10_device++;
		}
		g_curr_sensor_type = RFXCOM_SENSOR_UNKNOWN;
		return;
	}
	else if((strcmp("sensor-security", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR) {
		g_settings_tag = SETTINGS_TAG_SENSORS;
		if ((g_subuid_counter + secGetSensorNumEndpoints(rfxcomConfig->sec_device[g_next_sec_device].devcaps) < g_max_subuid) && 
		(g_next_sec_device < RFXCOM_SEC_DEVICES)) {
			g_sec_devices_xapbsc[g_next_sec_device].uidsub = g_subuid_counter;
			g_subuid_counter += secGetSensorNumEndpoints(rfxcomConfig->sec_device[g_next_sec_device].devcaps);
			g_next_sec_device++;
		}
		g_curr_sensor_type = RFXCOM_SENSOR_UNKNOWN;
		return;
	}
	else if((strcmp("sensor-oregon", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR) {
		g_settings_tag = SETTINGS_TAG_SENSORS;
		if ((g_subuid_counter + oreGetSensorNumEndpoints(rfxcomConfig->ore_device[g_next_ore_device].devcaps) < g_max_subuid) && 
		(g_next_ore_device < RFXCOM_ORE_DEVICES)) {
			g_ore_devices_xapbsc[g_next_ore_device].uidsub = g_subuid_counter;
			g_subuid_counter += oreGetSensorNumEndpoints(rfxcomConfig->ore_device[g_next_ore_device].devcaps);
			g_next_ore_device++;
		}
		g_curr_sensor_type = RFXCOM_SENSOR_UNKNOWN;
		return;
	}
	else if((strcmp("sensor-homeeasy", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR) {
		g_settings_tag = SETTINGS_TAG_SENSORS;
		if ((g_subuid_counter < g_max_subuid) && (g_next_he_device < RFXCOM_HE_DEVICES)) {
			g_he_devices_xapbsc[g_next_he_device].uidsub = g_subuid_counter;
			g_subuid_counter++;
			g_next_he_device++;
		}
		g_curr_sensor_type = RFXCOM_SENSOR_UNKNOWN;
		return;
	}
	else if((strcmp("sensor-koppla", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR) {
		g_settings_tag = SETTINGS_TAG_SENSORS;
		if ((g_subuid_counter < g_max_subuid) && (g_next_kop_device < RFXCOM_KOP_DEVICES)) {
			g_kop_devices_xapbsc[g_next_kop_device].uidsub = g_subuid_counter;
			g_subuid_counter++;
			g_next_kop_device++;
		}
		g_curr_sensor_type = RFXCOM_SENSOR_UNKNOWN;
		return;
	}
	else if((strcmp("sensor-rfxcom", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR) {
		g_settings_tag = SETTINGS_TAG_SENSORS;
		if ((g_subuid_counter + rfxGetSensorNumEndpoints(rfxcomConfig->rfx_device[g_next_rfx_device].devcaps) < g_max_subuid) && 
		(g_next_rfx_device < RFXCOM_RFX_DEVICES)) {
			g_rfx_devices_xapbsc[g_next_rfx_device].uidsub = g_subuid_counter;
			g_subuid_counter += rfxGetSensorNumEndpoints(rfxcomConfig->rfx_device[g_next_rfx_device].devcaps);
			g_next_rfx_device++;
		}
		g_curr_sensor_type = RFXCOM_SENSOR_UNKNOWN;
		return;
	}
	else if((strcmp("enabled", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR_ENABLED) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else if((strcmp("location", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR_LOC) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else if((strcmp("name", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR_NAME) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else if((strcmp("devcaps", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR_X10CAPS) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else if((strcmp("devcaps", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR_SECCAPS) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else if((strcmp("devcaps", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR_ORECAPS) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else if((strcmp("devcaps", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR_RFXCAPS) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else if((strcmp("devconfig", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR_ORECFG) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else if((strcmp("devconfig", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR_RFXCFG) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else
		g_settings_tag = SETTINGS_TAG_NONE;
}

// SAX value element callback for Xml settings
static void settingsXmlValueElementCallback(void *ctx, const xmlChar *ch, int len) {
	
	char elemvalue[XAP_ADDRESS_LEN];		// XAP_ADDRESS_LEN is the biggest length for tags

	// Value to STRZ
	int i;
	for (i = 0; (i<len) && (i < sizeof(elemvalue)-1); i++)
		elemvalue[i] = ch[i];
	elemvalue[i] = 0;
	
	//printf("Value: %s", elemvalue);	// Print element value
	
	switch(g_settings_tag) {

		// DEVICE element values
		case SETTINGS_TAG_DEV_NAME:
			if (strlen(elemvalue) <= sizeof(rfxcomConfig->xap_addr.instance))
				strcpy(rfxcomConfig->xap_addr.instance, elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_ENABLED:
			rfxcomConfig->enabled = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_COMMPORT:
			if (strlen(elemvalue) <= sizeof(rfxcomConfig->commport))
				strcpy(rfxcomConfig->commport, elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_X10LOC:
			if (strlen(elemvalue) <= sizeof(rfxcomConfig->x10_location))
				strcpy(rfxcomConfig->x10_location, elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_X10NAME:
			if (strlen(elemvalue) <= sizeof(rfxcomConfig->x10_name))
				strcpy(rfxcomConfig->x10_name, elemvalue);
			break;
		
		case SETTINGS_TAG_DEV_DISX10:
			rfxcomConfig->disable_x10 = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_DISOREGON:
			rfxcomConfig->disable_oregon = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_DISARC:
			rfxcomConfig->disable_arc = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_DISVISONIC:
			rfxcomConfig->disable_visonic = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_DISATI:
			rfxcomConfig->disable_ati = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_DISKOPPLA:
			rfxcomConfig->disable_koppla = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_DISHEASYUK:
			rfxcomConfig->disable_heasyUK = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_DISHEASYEU:
			rfxcomConfig->disable_heasyEU = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_DISX10SBSC:
			rfxcomConfig->disable_x10_xapbsc_sch = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_DISX10SX10:
			rfxcomConfig->disable_x10_xapx10_sch = atoi(elemvalue);
			if (!rfxcomConfig->disable_x10_xapx10_sch) {
				// Update subUID counter
				g_xapx10_subuid = g_subuid_counter;
				g_subuid_counter++;
			}
			break;
			
		case SETTINGS_TAG_DEV_DISX10MSGQ:
			rfxcomConfig->disable_x10_msgqueue = atoi(elemvalue);
			break;
			
		// XAP element values
		case SETTINGS_TAG_XAP_IFACE:
			if (strlen(elemvalue) <= sizeof(rfxcomConfig->interfacename))
				strcpy(rfxcomConfig->interfacename, elemvalue);
			break;
			
		case SETTINGS_TAG_XAP_VERSION:
			rfxcomConfig->xap_ver = atoi(elemvalue);
			lxap_xap_version = rfxcomConfig->xap_ver;
			if (rfxcomConfig->xap_ver == XAP_VER_12)
				g_max_subuid = RFXCOM_XAP_V12_MAX_SENSORS;
			else
				g_max_subuid = RFXCOM_XAP_V13_MAX_SENSORS;
			break;
			
		case SETTINGS_TAG_XAP_HBEATF:
			rfxcomConfig->xap_hbeat = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_XAP_INFOF:
			rfxcomConfig->xap_infof = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_XAP_WSERVF:
			rfxcomConfig->xap_wserv = atoi(elemvalue);
			break;
		
		case SETTINGS_TAG_XAP_UID:
			rfxcomConfig->xap_uid = strtoul(elemvalue, 0, 16);
			break;
			
		case SETTINGS_TAG_XAP_PORT:
			rfxcomConfig->xap_port = atoi(elemvalue);
			break;
			
		// SENSORS element values
		case SETTINGS_TAG_SENSOR_ENABLED:
			if (g_subuid_counter < g_max_subuid) {
				switch (g_curr_sensor_type) {
					case RFXCOM_SENSOR_X10:
						if (g_next_x10_device < RFXCOM_X10_DEVICES)
							rfxcomConfig->x10_device[g_next_x10_device].enabled = atoi(elemvalue);
						break;
						
					case RFXCOM_SENSOR_SECURITY:
						if (g_next_sec_device < RFXCOM_SEC_DEVICES)
							rfxcomConfig->sec_device[g_next_sec_device].enabled = atoi(elemvalue);
						break;
						
					case RFXCOM_SENSOR_OREGON:
						if (g_next_ore_device < RFXCOM_ORE_DEVICES)
							rfxcomConfig->ore_device[g_next_ore_device].enabled = atoi(elemvalue);
						break;
						
					case RFXCOM_SENSOR_HOMEEASY:
						if (g_next_he_device < RFXCOM_HE_DEVICES)
							rfxcomConfig->he_device[g_next_he_device].enabled = atoi(elemvalue);
						break;
						
					case RFXCOM_SENSOR_KOPPLA:
						if (g_next_kop_device < RFXCOM_KOP_DEVICES)
							rfxcomConfig->kop_device[g_next_kop_device].enabled = atoi(elemvalue);
						break;
						
					case RFXCOM_SENSOR_RFXCOM:
						if (g_next_rfx_device < RFXCOM_RFX_DEVICES)
							rfxcomConfig->rfx_device[g_next_rfx_device].enabled = atoi(elemvalue);
						break;
				}
			}
			break;
			
		case SETTINGS_TAG_SENSOR_LOC:
			if (g_subuid_counter < g_max_subuid) {
				switch (g_curr_sensor_type) {
					case RFXCOM_SENSOR_X10:
						if (g_next_x10_device < RFXCOM_X10_DEVICES) {
							if (strlen(elemvalue) <= sizeof(rfxcomConfig->x10_device[g_next_x10_device].location))
								strcpy(rfxcomConfig->x10_device[g_next_x10_device].location, elemvalue);
						}
						break;
						
					case RFXCOM_SENSOR_SECURITY:
						if (g_next_sec_device < RFXCOM_SEC_DEVICES) {
							if (strlen(elemvalue) <= sizeof(rfxcomConfig->sec_device[g_next_sec_device].location))
								strcpy(rfxcomConfig->sec_device[g_next_sec_device].location, elemvalue);
						}
						break;
						
					case RFXCOM_SENSOR_OREGON:
						if (g_next_ore_device < RFXCOM_ORE_DEVICES) {
							if (strlen(elemvalue) <= sizeof(rfxcomConfig->ore_device[g_next_ore_device].location))
								strcpy(rfxcomConfig->ore_device[g_next_ore_device].location, elemvalue);
						}
						break;
						
					case RFXCOM_SENSOR_HOMEEASY:
						if (g_next_he_device < RFXCOM_HE_DEVICES) {
							if (strlen(elemvalue) <= sizeof(rfxcomConfig->he_device[g_next_he_device].location))
								strcpy(rfxcomConfig->he_device[g_next_he_device].location, elemvalue);
						}
						break;
						
					case RFXCOM_SENSOR_KOPPLA:
						if (g_next_kop_device < RFXCOM_KOP_DEVICES) {
							if (strlen(elemvalue) <= sizeof(rfxcomConfig->kop_device[g_next_kop_device].location))
								strcpy(rfxcomConfig->kop_device[g_next_kop_device].location, elemvalue);
						}
						break;
						
					case RFXCOM_SENSOR_RFXCOM:
						if (g_next_rfx_device < RFXCOM_RFX_DEVICES) {
							if (strlen(elemvalue) <= sizeof(rfxcomConfig->rfx_device[g_next_rfx_device].location))
								strcpy(rfxcomConfig->rfx_device[g_next_rfx_device].location, elemvalue);
						}
						break;
				}
			}
			break;
			
		case SETTINGS_TAG_SENSOR_NAME:
			if (g_subuid_counter < g_max_subuid) {
				switch (g_curr_sensor_type) {
					case RFXCOM_SENSOR_X10:
						if (g_next_x10_device < RFXCOM_X10_DEVICES) {
							if (strlen(elemvalue) <= sizeof(rfxcomConfig->x10_device[g_next_x10_device].name))
								strcpy(rfxcomConfig->x10_device[g_next_x10_device].name, elemvalue);
						}
						break;
						
					case RFXCOM_SENSOR_SECURITY:
						if (g_next_sec_device < RFXCOM_SEC_DEVICES) {
							if (strlen(elemvalue) <= sizeof(rfxcomConfig->sec_device[g_next_sec_device].name))
								strcpy(rfxcomConfig->sec_device[g_next_sec_device].name, elemvalue);
						}
						break;
						
					case RFXCOM_SENSOR_OREGON:
						if (g_next_ore_device < RFXCOM_ORE_DEVICES) {
							if (strlen(elemvalue) <= sizeof(rfxcomConfig->ore_device[g_next_ore_device].name))
								strcpy(rfxcomConfig->ore_device[g_next_ore_device].name, elemvalue);
						}
						break;
						
					case RFXCOM_SENSOR_HOMEEASY:
						if (g_next_he_device < RFXCOM_HE_DEVICES) {
							if (strlen(elemvalue) <= sizeof(rfxcomConfig->he_device[g_next_he_device].name))
								strcpy(rfxcomConfig->he_device[g_next_he_device].name, elemvalue);
						}
						break;
						
					case RFXCOM_SENSOR_KOPPLA:
						if (g_next_kop_device < RFXCOM_KOP_DEVICES) {
							if (strlen(elemvalue) <= sizeof(rfxcomConfig->kop_device[g_next_kop_device].name))
								strcpy(rfxcomConfig->kop_device[g_next_kop_device].name, elemvalue);
						}
						break;
						
					case RFXCOM_SENSOR_RFXCOM:
						if (g_next_rfx_device < RFXCOM_RFX_DEVICES) {
							if (strlen(elemvalue) <= sizeof(rfxcomConfig->rfx_device[g_next_rfx_device].name))
								strcpy(rfxcomConfig->rfx_device[g_next_rfx_device].name, elemvalue);
						}
						break;
				}
			}
			break;
		
		// X10 SENSORS specific element values
		case SETTINGS_TAG_SENSOR_X10CAPS:
			if (g_subuid_counter < g_max_subuid) {
				if (g_next_x10_device < RFXCOM_X10_DEVICES) {
					rfxcomConfig->x10_device[g_next_x10_device].devcaps = atoi(elemvalue);
					update_x10_devices_xapbsc_with_devcaps(g_next_x10_device);
				}
			}
			break;
			
		// SECURITY SENSORS specific element values
		case SETTINGS_TAG_SENSOR_SECCAPS:
			if (g_subuid_counter < g_max_subuid) {
				if (g_next_sec_device < RFXCOM_SEC_DEVICES) {
					rfxcomConfig->sec_device[g_next_sec_device].devcaps = atoi(elemvalue);
				}
			}
			break;
			
		// OREGON SENSORS specific element values
		case SETTINGS_TAG_SENSOR_ORECAPS:
			if (g_subuid_counter < g_max_subuid) {
				if (g_next_ore_device < RFXCOM_ORE_DEVICES) {
					rfxcomConfig->ore_device[g_next_ore_device].devcaps = atoi(elemvalue);
				}
			}
			break;
			
		case SETTINGS_TAG_SENSOR_ORECFG:
			if (g_subuid_counter < g_max_subuid) {
				if (g_next_ore_device < RFXCOM_ORE_DEVICES) {
					rfxcomConfig->ore_device[g_next_ore_device].devconfig = atoi(elemvalue);
				}
			}
			break;
			
		// RFXCOM SENSORS specific element values
		case SETTINGS_TAG_SENSOR_RFXCAPS:
			if (g_subuid_counter < g_max_subuid) {
				if (g_next_rfx_device < RFXCOM_RFX_DEVICES) {
					rfxcomConfig->rfx_device[g_next_rfx_device].devcaps = atoi(elemvalue);
				}
			}
			break;
			
		case SETTINGS_TAG_SENSOR_RFXCFG:
			if (g_subuid_counter < g_max_subuid) {
				if (g_next_rfx_device < RFXCOM_RFX_DEVICES) {
					rfxcomConfig->rfx_device[g_next_rfx_device].devconfig = atoi(elemvalue);
				}
			}
			break;
	}
}

// Parse the settings XML file.
int parseXmlSettings(const char *filename) {

	int retVal = 0;
	
	xmlSAXHandler saxHandler;

	memset(&saxHandler, 0, sizeof(saxHandler));
	saxHandler.initialized = XML_SAX2_MAGIC;
	saxHandler.startElement = settingsXmlStartElementCallback;	
	saxHandler.characters = settingsXmlValueElementCallback;
	saxHandler.endElement = settingsXmlEndElementCallback;
	
	if(xmlSAXUserParseFile(&saxHandler, NULL, filename) < 0) 		
		retVal = 1;
		
	// Cleanup function for the XML library.
	xmlCleanupParser();

    // This is to debug memory for regression tests
	xmlMemoryDump();
	
	return retVal;
}

// Save Xml settings
short int saveXmlSettings(const char *filename) {
	int retVal;

	retVal = settingsXmlSave(filename);
	switch(retVal) {
		case 1:
			syslog(LOG_ERR, "settingsXmlSave: Error creating the xml writer");
			logError("settingsXmlSave: Error creating the xml writer");
			break;
			
		case 2:
			syslog(LOG_ERR, "settingsXmlSave: Error at xmlTextWriterStartDocument");
			logError("settingsXmlSave: Error at xmlTextWriterStartDocument");
			break;
			
		case 3:
			syslog(LOG_ERR, "settingsXmlSave: Error at xmlTextWriterWriteAttribute");
			logError("settingsXmlSave: Error at xmlTextWriterWriteAttribute");
			break;
			
		case 4:
			syslog(LOG_ERR, "settingsXmlSave: Error at xmlTextWriterWriteFormatElement");
			logError("settingsXmlSave: Error at xmlTextWriterWriteFormatElement");
			break;
			
		case 5:
			syslog(LOG_ERR, "settingsXmlSave: Error at xmlTextWriterEndElement");
			logError("settingsXmlSave: Error at xmlTextWriterEndElement");
			break;
	}
	
	return retVal;
}

// Get version of Rfxcom
int rfxcomGetVersion(int fd, int retries, BYTE *master, BYTE *slave) {
	
	unsigned char txBuf[2];
	unsigned char rxBuf[4];
	int rxBufLen;
	int numBytes;
	int retry = retries;
	
	while(retry > 0) { 
		// Init vars
		numBytes = 0;
		*master = 0;
		*slave = 0;
		memset(rxBuf, 0x00, sizeof(rxBuf));
					
		// Set command to get version
		txBuf[0] = 0xF0;
		txBuf[1] = 0x20;
		if ((write(fd, txBuf, 2)) >= 0) {
			// Read response
			while(numBytes < 4) {
				rxBufLen = read(fd, rxBuf + numBytes, sizeof(rxBuf)-numBytes);
				numBytes += rxBufLen;
				if (numBytes == 4) {
					if (rxBuf[0] == 0x4D) {
						*master = rxBuf[1];
						if (rxBuf[2] == 0x53)
							*slave = rxBuf[3];
						return TRUE;
					}
					else if (rxBuf[2] == 0x4D) {
						*master = rxBuf[3];
						if (rxBuf[0] == 0x53)
							*slave = rxBuf[1];
						return TRUE;
					}
				}
			}
		}
		retry--;
	}
	return FALSE;
}

// Send command to Rfxcom
int rfxcomSendCommand(int fd, unsigned char command, int mode, int retries) {

	unsigned char txBuf[2];
	unsigned char rxBuf[2];
	int retry = retries;
	
	while(retry > 0) { 
		// Clear buffer
		memset(rxBuf, 0x00, sizeof(rxBuf));
		
		// Send command
		txBuf[0] = 0xF0;
		txBuf[1] = command & 0xFF;
		if ((write(fd, txBuf, 2)) >= 0) {
			if (read(fd, rxBuf, sizeof(rxBuf)) > 0) {
				//printf("Response: %X\n", rxBuf[0]);
				if (rxBuf[0] == mode)
					return TRUE;
			}
		}
		retry--;
	}
	return FALSE;
}

// Open Rfxcom serial port
int rfxcomOpenSerialPort() {
	int fd;
	struct termios tty;
	BYTE ver_master, ver_slave;
	
	if(strncmp(rfxcomConfig->commport, "/dev/", 5) != 0)
		return -1;

	if ((fd = open(rfxcomConfig->commport, O_RDWR | O_NOCTTY | O_SYNC)) < 0)
		return fd;
		
	memset (&tty, 0, sizeof tty);
	if (tcgetattr(fd, &tty) != 0)
		return -1;

	cfsetospeed(&tty, B4800);
	cfsetispeed(&tty, B4800);
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break as \000 chars
	tty.c_iflag &= ~IGNBRK;         				// ignore break signal
	tty.c_lflag = 0;                				// no signaling chars, no echo, no canonical processing
	tty.c_oflag = 0;                				// no remapping, no delays
	tty.c_cc[VMIN]  = 0;            				// read doesn't block
	tty.c_cc[VTIME] = 5;            				// 0.5 seconds read timeout
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); 		// shut off xon/xoff ctrl
	tty.c_cflag |= (CLOCAL | CREAD);				// ignore modem controls, enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      		// shut off parity
	tty.c_cflag |= 0;								// parity none
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (fd, TCSANOW, &tty) != 0) 
		return -1;

	// Get software version
	if (!rfxcomGetVersion(fd, 2, &ver_master, &ver_slave)) return RFXCOM_ERROR_INIT;
	#ifdef DEBUG_APP
	printf("RFXCOM version master:%X slave:%X\n", ver_master, ver_slave);
	#endif
	logEvent(TRUE, "RFXCOM version master:%X slave:%X", ver_master, ver_slave);
		
	// Set variable length mode
	if (!rfxcomSendCommand(fd, RFXCOM_MODE_VAR_LEN, RFXCOM_MODE_VAR_LEN, 2)) return RFXCOM_ERROR_INIT;
	#ifdef DEBUG_APP
	printf("RFXCOM variable length mode\n");
	#endif
	logEvent(TRUE, "RFXCOM variable length mode");
	
	// Enable all possible receiving modes
	if (!rfxcomSendCommand(fd, RFXCOM_MODE_ENABLE_ALL, RFXCOM_MODE_VAR_LEN, 2)) return RFXCOM_ERROR_INIT;

	// If set, disable X10
	if (rfxcomConfig->disable_x10) {
		if (!rfxcomSendCommand(fd, RFXCOM_MODE_DISABLE_X10, RFXCOM_MODE_VAR_LEN, 2)) return RFXCOM_ERROR_DISX10;
	}
	
	// If set, disable Oregon
	if (rfxcomConfig->disable_oregon) {
		if (!rfxcomSendCommand(fd, RFXCOM_MODE_DISABLE_OREGON, RFXCOM_MODE_VAR_LEN, 2)) return RFXCOM_ERROR_DISOREGON;
	}
	
	// If set, disable ARC
	if (rfxcomConfig->disable_arc) {
		if (!rfxcomSendCommand(fd, RFXCOM_MODE_DISABLE_ARC, RFXCOM_MODE_VAR_LEN, 2)) return RFXCOM_ERROR_DISARC;
	}
	
	// If set, disable Visonic
	if (rfxcomConfig->disable_visonic) {
		if (!rfxcomSendCommand(fd, RFXCOM_MODE_DISABLE_VIS, RFXCOM_MODE_VAR_LEN, 2)) return RFXCOM_ERROR_DISVISONIC;
	}

	// If set, disable ATI
	if (rfxcomConfig->disable_ati) {
		if (!rfxcomSendCommand(fd, RFXCOM_MODE_DISABLE_ATI, RFXCOM_MODE_VAR_LEN, 2)) return RFXCOM_ERROR_DISATI;
	}
	
	// If set, disable Koppla
	if (rfxcomConfig->disable_koppla) {
		if (!rfxcomSendCommand(fd, RFXCOM_MODE_DISABLE_KOPPLA, RFXCOM_MODE_VAR_LEN, 2)) return RFXCOM_ERROR_DISKOPPLA;
	}

	// If set, disable HomeEasy UK
	if (rfxcomConfig->disable_heasyUK) {
		if (!rfxcomSendCommand(fd, RFXCOM_MODE_DISABLE_HEASYUK, RFXCOM_MODE_VAR_LEN, 2)) return RFXCOM_ERROR_DISHEASYUK;
	}
	
	// If set, disable HomeEasy EU
	if (rfxcomConfig->disable_heasyEU) {
		if (!rfxcomSendCommand(fd, RFXCOM_MODE_DISABLE_HEASYEU, RFXCOM_MODE_VAR_LEN, 2)) return RFXCOM_ERROR_DISHEASYEU;
	}

	return fd;
}

// Process X10RF messages
void processX10RfMsg(BYTE *buffer) {
	BYTE hcode, ucode, func;
	
	// Decode the message
	decode_X10RF(buffer, &hcode, &ucode, &func);
	
	// Log the received message
	logEvent(TRUE, "X10RF received, Addr:%c%d Func:%d", hcode, ucode, func);
			
	// Save last housecode and unitcode for ON/OFF messages
	if ((func == X10RF_OFF) || (func == X10RF_ON)) {
		last_hcode = hcode;
		last_ucode = ucode;
	}
	else {
		// Else, set unit code to last unitcode received if housecode is the same
		if (last_hcode == hcode)
			ucode = last_ucode;
	}
			
	// If enabled xAP-BSC schema for X10 messages
	if (!rfxcomConfig->disable_x10_xapbsc_sch) {
		int sensor;
		if ((func == X10RF_ALL_LIGHTS_OFF) || (func == X10RF_ALL_LIGHTS_ON)) {
			// Send command to all units in the same housecode
			BYTE i;
			for(i=0; i<16; i++) {
				// Search for X10 sensor in sensor list
				if ((sensor = get_x10_device_index(hcode, i+1)) >= 0) {
					// Update endpoint state
					update_x10_xapbsc_endpoint(hcode, i+1, func, sensor);
					
					// Send the xAP-BSC message event if sensor is enabled
					if (rfxcomConfig->x10_device[sensor].enabled) {
						if (!sendxAPBscEventMsgX10(hcode, i+1, func, sensor)) {
							#ifdef DEBUG_APP
							printf("processX10RfMsg: Failed to send X10RF xAP-BSC event message\n");
							#endif
							syslog(LOG_ERR, "processX10RfMsg: Failed to send X10RF xAP-BSC event message");
							logError("processX10RfMsg: Failed to send X10RF xAP-BSC event message");
						}
					}
				}
			}
		}
		else {
			// Search for X10 sensor in sensor list
			if ((sensor = get_x10_device_index(hcode, ucode)) >= 0) {
				// Update endpoint state
				update_x10_xapbsc_endpoint(hcode, ucode, func, sensor);

				// Send the xAP-BSC message event if sensor is enabled
				if (rfxcomConfig->x10_device[sensor].enabled) {
					if (!sendxAPBscEventMsgX10(hcode, ucode, func, sensor)) {
						#ifdef DEBUG_APP
						printf("processX10RfMsg: Failed to send X10RF xAP-BSC event message\n");
						#endif
						syslog(LOG_ERR, "processX10RfMsg: Failed to send X10RF xAP-BSC event message");
						logError("processX10RfMsg: Failed to send X10RF xAP-BSC event message");
					}
				}
			}
			else {
				// Sensor not found in x10_device list
				#ifdef DEBUG_APP
				printf("processX10RfMsg: X10RF xAP-BSC endpoint not found\n");
				#endif
				syslog(LOG_ERR, "processX10RfMsg: X10RF xAP-BSC endpoint not found");
				logError("processX10RfMsg: X10RF xAP-BSC endpoint not found");
			}
		}
	}
			
	// If enabled xAP-X10 schema for X10 messages
	if (!rfxcomConfig->disable_x10_xapx10_sch) {
		// Send the xAP-X10 message event
		if (!sendxAPX10EventMsgX10(hcode, ucode, func)) {
			#ifdef DEBUG_APP
			printf("processX10RfMsg: Failed to send X10RF xAP-X10 event message\n");
			#endif
			syslog(LOG_ERR, "processX10RfMsg: Failed to send X10RF xAP-X10 event message");
			logError("processX10RfMsg: Failed to send X10RF xAP-X10 event message");
		}
	}
		
	// If enabled X10RF message queue
	if (!rfxcomConfig->disable_x10_msgqueue) {
		t_X10RF_MSG x10RfMsg;
		x10RfMsg.msg_id = 2;
		sprintf(x10RfMsg.device, "%c%d", hcode, ucode);
		x10RfMsg.funct = func;
		msgsnd(x10RfMsgQueue, (struct msgbuf *)&x10RfMsg, sizeof(x10RfMsg.device)+sizeof(x10RfMsg.funct), IPC_NOWAIT);
	}
}

// Process DM10 messages
void processDM10Msg(BYTE *buffer) {
	BYTE hcode, ucode, func;
	
	// Decode the message
	decode_DM10(buffer, &hcode, &ucode, &func);
	
	// Log the received message
	logEvent(TRUE, "DM10 received, Addr:%c%d Func:%d", hcode, ucode, func);
			
	// Save last housecode and unitcode
	last_hcode = hcode;
	last_ucode = ucode;
			
	// If enabled xAP-BSC schema for X10 messages
	if (!rfxcomConfig->disable_x10_xapbsc_sch) {
		int sensor;

		// Search for X10 sensor in sensor list
		if ((sensor = get_x10_device_index(hcode, ucode)) >= 0) {
			// Update endpoint state
			update_x10_xapbsc_endpoint(hcode, ucode, func, sensor);

			// Send the xAP-BSC message event if sensor is enabled
			if (rfxcomConfig->x10_device[sensor].enabled) {
				if (!sendxAPBscEventMsgX10(hcode, ucode, func, sensor)) {
					#ifdef DEBUG_APP
					printf("processX10RfMsg: Failed to send X10RF xAP-BSC event message\n");
					#endif
					syslog(LOG_ERR, "processX10RfMsg: Failed to send X10RF xAP-BSC event message");
					logError("processX10RfMsg: Failed to send X10RF xAP-BSC event message");
				}
			}
		}
		else {
			// Sensor not found in x10_device list
			#ifdef DEBUG_APP
			printf("processX10RfMsg: X10RF xAP-BSC endpoint not found\n");
			#endif
			syslog(LOG_ERR, "processX10RfMsg: X10RF xAP-BSC endpoint not found");
			logError("processX10RfMsg: X10RF xAP-BSC endpoint not found");
		}
	}
			
	// If enabled xAP-X10 schema for X10 messages
	if (!rfxcomConfig->disable_x10_xapx10_sch) {
		// Send the xAP-X10 message event
		if (!sendxAPX10EventMsgX10(hcode, ucode, func)) {
			#ifdef DEBUG_APP
			printf("processX10RfMsg: Failed to send X10RF xAP-X10 event message\n");
			#endif
			syslog(LOG_ERR, "processX10RfMsg: Failed to send X10RF xAP-X10 event message");
			logError("processX10RfMsg: Failed to send X10RF xAP-X10 event message");
		}
	}
		
	// If enabled X10RF message queue
	if (!rfxcomConfig->disable_x10_msgqueue) {
		t_X10RF_MSG x10RfMsg;
		x10RfMsg.msg_id = 2;
		sprintf(x10RfMsg.device, "%c%d", hcode, ucode);
		x10RfMsg.funct = func;
		msgsnd(x10RfMsgQueue, (struct msgbuf *)&x10RfMsg, sizeof(x10RfMsg.device)+sizeof(x10RfMsg.funct), IPC_NOWAIT);
	}
}

// Process SECURITY messages
void processSecurityMsg(BYTE *buffer, BYTE msgType) {
	char addr[7];			// Store the address of device
	BYTE msgcode;			// Store the message code
	unsigned int sensor;	// Store the index of the sensor list
		
	switch(msgType) {
		case RFXCOM_SECURITY_MSG_POWERCODE:
			// Decode the PowerCode message
			decode_powercode(buffer, addr, &msgcode);
			
			// Log the received message
			logEvent(TRUE, "Visonic PowerCode received, Addr:%s MsgCode:%d", addr, msgcode);
			
			// Update endpoint state
			if (update_sec_xapbsc_endpoint(addr, msgcode, &sensor)) {
				if (rfxcomConfig->sec_device[sensor].enabled) {
					// Send the xAP-BSC message event
					if (!sendxAPBscEventMsgSEC(sensor)) {
						#ifdef DEBUG_APP
						printf("processSecurityMsg: Failed to send xAP-BSC event message\n");
						#endif
						syslog(LOG_ERR, "processSecurityMsg: Failed to send xAP-BSC event message");
						logError("processSecurityMsg: Failed to send xAP-BSC event message");
					}
				}
			}
			else {
				#ifdef DEBUG_APP
				printf("processSecurityMsg: xAP-BSC endpoint not found\n");
				#endif
				syslog(LOG_ERR, "processSecurityMsg: xAP-BSC endpoint not found");
				logError("processSecurityMsg: xAP-BSC endpoint not found");
			}
			break;

		case RFXCOM_SECURITY_MSG_CODESECURE:
			break;
			
		case RFXCOM_SECURITY_MSG_X10SEC:
			// Decode the X10 Security message
			decode_X10Sec(buffer, addr, &msgcode);
			
			// Log the received message
			logEvent(TRUE, "X10 Security received, Addr:%s MsgCode:%d", addr, msgcode);
			
			// Update endpoint state
			if (update_sec_xapbsc_endpoint(addr, msgcode, &sensor)) {
				if (rfxcomConfig->sec_device[sensor].enabled) {
					// Send the xAP-BSC message event
					if (!sendxAPBscEventMsgSEC(sensor)) {
						#ifdef DEBUG_APP
						printf("processSecurityMsg: Failed to send xAP-BSC event message\n");
						#endif
						syslog(LOG_ERR, "processSecurityMsg: Failed to send xAP-BSC event message");
						logError("processSecurityMsg: Failed to send xAP-BSC event message");
					}
				}
			}
			else {
				#ifdef DEBUG_APP
				printf("processSecurityMsg: xAP-BSC endpoint not found\n");
				#endif
				syslog(LOG_ERR, "processSecurityMsg: xAP-BSC endpoint not found");
				logError("processSecurityMsg: xAP-BSC endpoint not found");
			}
			break;
			
		case RFXCOM_SECURITY_JAMMING_DETECT:
			#ifdef DEBUG_APP
			printf("processSecurityMsg: jamming detected\n");
			#endif
			syslog(LOG_ERR, "processSecurityMsg: jamming detected");
			logError("processSecurityMsg: jamming detected");
			break;
		
		case RFXCOM_SECURITY_JAMMING_END:
			#ifdef DEBUG_APP
			printf("processSecurityMsg: end jamming detected\n");
			#endif
			syslog(LOG_ERR, "processSecurityMsg: end jamming detected");
			logError("processSecurityMsg: end jamming detected");
			break;
	}
}

// Process OREGON messages
void processOregonMsg(BYTE *buffer, BYTE msgType) {
	char addr[4];					// Store the address of device
	float value1, value2, value3;	// Store sensor values
	BYTE bat;						// Store battery status
	unsigned int sensor;			// Store the index of the sensor list
	
	// Initialize vars
	value1 = 0;
	value2 = 0;
	value3 = 0;
	bat = RFXCOM_OREGON_BAT_UNKNOWN;
	
	// Decode the message received
	switch(msgType) {
		case RFXCOM_OREGON_MSG_TEMP1:
		case RFXCOM_OREGON_MSG_TEMP2:
		case RFXCOM_OREGON_MSG_TEMP3:
		case RFXCOM_OREGON_MSG_TEMP4:
			if (decode_oregon_temp(buffer, msgType, addr, &value1, &bat))
				logEvent(TRUE, "Oregon temp sensor Addr:%s Temp:%.1fºC", addr, value1);
			else {
				logEvent(TRUE, "Error decoding Oregon temp sensor msg:%s", buffer);
				return;
			}
			break;
			
		case RFXCOM_OREGON_MSG_TH1:
		case RFXCOM_OREGON_MSG_TH2:
		case RFXCOM_OREGON_MSG_TH3:
		case RFXCOM_OREGON_MSG_TH4:
		case RFXCOM_OREGON_MSG_TH5:
		case RFXCOM_OREGON_MSG_TH6:
			if (decode_oregon_th(buffer, msgType, addr, &value1, &value2, &bat)) {
				logEvent(TRUE, "Oregon temp-hygro sensor Addr:%s Temp:%.1fºC Hum:%.0f%%", addr, value1, value2);
			}
			else {
				logEvent(TRUE, "Error decoding Oregon temp-hygro sensor msg:%s", buffer);
				return;
			}
			break;
		
		case RFXCOM_OREGON_MSG_THB1:
		case RFXCOM_OREGON_MSG_THB2:
			if (decode_oregon_thb(buffer, msgType, addr, &value1, &value2, &value3, &bat))
				logEvent(TRUE, "Oregon temp-hygro-baro sensor Addr:%s Temp:%fºC Hum:%.0f%% Baro:%.0fhPA", addr, value1, value2, value3);
			else {
				logEvent(TRUE, "Error decoding Oregon temp-hygro-baro sensor msg:%s", buffer);
				return;
			}
			break;
		
		case RFXCOM_OREGON_MSG_RAIN1:
		case RFXCOM_OREGON_MSG_RAIN2:
		case RFXCOM_OREGON_MSG_RAIN3:
			if (decode_oregon_rain(buffer, msgType, addr, &value1, &value2, &bat)) {
				logEvent(TRUE, "Oregon rain sensor Addr:%s Rain rate:%.3fmm//hr Rain total:%.3fmm", addr, value1, value2);
			}
			break;
		
		case RFXCOM_OREGON_MSG_WIND1:
		case RFXCOM_OREGON_MSG_WIND2:
		case RFXCOM_OREGON_MSG_WIND3:
			if (decode_oregon_wind(buffer, msgType, addr, &value1, &value2, &value3, &bat))
				logEvent(TRUE, "Oregon wind sensor Addr:%s Dir:%.1fº Speed:%.1fm//s Avg:%.1fm//s", addr, value1, value2, value3);
			else {
				logEvent(TRUE, "Error decoding Oregon wind sensor msg:%s", buffer);
				return;
			}
			break;
		
		case RFXCOM_OREGON_MSG_UV1:
		case RFXCOM_OREGON_MSG_UV2:
			if (decode_oregon_uv(buffer, msgType, addr, &value1, &bat))
				logEvent(TRUE, "Oregon UV sensor Addr:%s UV factor:%.0f", addr, value1);
			else {
				logEvent(TRUE, "Error decoding Oregon UV sensor msg:%s", buffer);
				return;
			}
			break;
		
		case RFXCOM_OREGON_MSG_DT1:
			if (decode_oregon_dt(buffer, msgType, addr, &value1, &value2, &value3))
				logEvent(TRUE, "Oregon datetime sensor Addr:%s date:%.0f time:%.0f day:%.0f", addr, value1, value2, value3);
			else {
				logEvent(TRUE, "Error decoding Oregon datetime sensor msg:%s", buffer);
				return;
			}
			break;
		
		case RFXCOM_OREGON_MSG_WEIGHT1:
		case RFXCOM_OREGON_MSG_WEIGHT2:
			if (decode_oregon_weight(buffer, msgType, addr, &value1))
				logEvent(TRUE, "Oregon weight sensor Addr:%s Weight:%.1fKg", addr, value1);
			else {
				logEvent(TRUE, "Error decoding Oregon weight sensor msg:%s", buffer);
				return;
			}
			break;
		
		case RFXCOM_OREGON_MSG_ELEC1:
		case RFXCOM_OREGON_MSG_ELEC2:
			if (decode_oregon_elec(buffer, msgType, addr, &value1, &value2, &value3, &bat))
				logEvent(TRUE, "Oregon elec sensor Addr:%s CT1:%.1f CT2:%.1f CT3:%.1f", addr, value1, value2, value3);
			else {
				logEvent(TRUE, "Error decoding Oregon elec sensor msg:%s", buffer);
				return;
			}
			break;
			
		default:
			logEvent(TRUE, "Oregon device type not found in allowed device type list. msg:%s", buffer);
			return;
	}
	
	// Update endpoint state
	if (update_ore_xapbsc_endpoint(addr, value1, value2, value3, bat, &sensor)) {
		if (rfxcomConfig->ore_device[sensor].enabled) {
			// Send the xAP-BSC message event
			if (!sendxAPBscEventMsgORE(sensor)) {
				#ifdef DEBUG_APP
				printf("processOregonMsg: Failed to send xAP-BSC event message\n");
				#endif
				syslog(LOG_ERR, "processOregonMsg: Failed to send xAP-BSC event message");
				logError("processOregonMsg: Failed to send xAP-BSC event message");
			}
		}
	}
	else {
		#ifdef DEBUG_APP
		printf("processOregonMsg: xAP-BSC endpoint not found\n");
		#endif
		syslog(LOG_ERR, "processOregonMsg: xAP-BSC endpoint not found");
		logError("processOregonMsg: xAP-BSC endpoint not found");
	}
}

// Process HOMEEASY messages
void processHomeEasyMsg(BYTE *buffer) {
	char addr[9];					// Store the address of the device
	char id[9];						// Store the ID of the device
	BYTE func;						// Store the function
	BYTE level;						// Store the level
	unsigned int sensor;			// Store the index of the sensor list
	
	// Decode the message
	decode_HomeEasy(buffer, addr, id, &func, &level);
	
	// Log the received message
	logEvent(TRUE, "HomeEasy received, Addr:%s ID:%s Func:%d Level:%d", addr, id, func, level);
	
	// Update endpoint state
	if (update_he_xapbsc_endpoint(addr, func, level, &sensor)) {
		if (rfxcomConfig->he_device[sensor].enabled) {
			// Send the xAP-BSC message event
			if (!sendxAPBscEventMsgHE(sensor)) {
				#ifdef DEBUG_APP
				printf("processHomeEasyMsg: Failed to send xAP-BSC event message\n");
				#endif
				syslog(LOG_ERR, "processHomeEasyMsg: Failed to send xAP-BSC event message");
				logError("processHomeEasyMsg: Failed to send xAP-BSC event message");
			}
		}
	}
	else {
		#ifdef DEBUG_APP
		printf("processHomeEasyMsg: xAP-BSC endpoint not found\n");
		#endif
		syslog(LOG_ERR, "processHomeEasyMsg: xAP-BSC endpoint not found");
		logError("processHomeEasyMsg: xAP-BSC endpoint not found");
	}
}

// Process KOPPLA messages
void processKopplaMsg(BYTE *buffer) {
	BYTE addrlist[10];				// Store the list of addresses
	BYTE addrcount;					// Number of entries in list of addresses
	BYTE func;						// Store the function
	BYTE count;						// Store the count for UP & DOWN functions
	unsigned int devid;				// Store the index of the device in the list
	int i;
	
	// Decode the message
	decode_Koppla(buffer, addrlist, &addrcount, &func, &count);
	
	// Log the received message
	for (i=0; i<addrcount; i++) {
		logEvent(TRUE, "Koppla received, Addr:%d ID:%s Func:%d Count:%d", addrlist[i], func, count);
	}
	
	// Update endpoint state
	for (i=0; i<addrcount; i++) {
		if (update_kop_xapbsc_endpoint(addrlist[i], func, count, &devid)) {
			if (rfxcomConfig->kop_device[devid].enabled) {
				// Send the xAP-BSC message event
				if (!sendxAPBscEventMsgKOP(devid)) {
					#ifdef DEBUG_APP
					printf("processKopplaMsg: Failed to send xAP-BSC event message\n");
					#endif
					syslog(LOG_ERR, "processKopplaMsg: Failed to send xAP-BSC event message");
					logError("processKopplaMsg: Failed to send xAP-BSC event message");
				}
			}
		}
		else {
			#ifdef DEBUG_APP
			printf("processKopplaMsg: xAP-BSC endpoint not found\n");
			#endif
			syslog(LOG_ERR, "processKopplaMsg: xAP-BSC endpoint not found");
			logError("processKopplaMsg: xAP-BSC endpoint not found");
		}
	}
}

// Process RFXSENSOR messages
void processRfxsensorMsg(BYTE *buffer, BYTE msgType) {
	char addr[5];			// Store the address of device
	BYTE msgfunc;			// Store the message function
	float val1, val2, val3;	// Store the value returned for the function
	unsigned int sensor;	// Store the index of the sensor list
		
	switch(msgType) {
		case RFXCOM_RFXSENSOR_MSG_RFXSENSOR:
			// Decode the Rfxsensor message
			decode_rfxsensor(buffer, addr, &msgfunc, &val1, &val2, &val3);
			
			// Log the received message
			logEvent(TRUE, "Rfxsensor received, Addr:%s Func:%d, Values:%f,%f,%f", addr, msgfunc, val1, val2, val3);
			
			// Update endpoint state
			if (update_rfx_xapbsc_endpoint(addr, msgfunc, val1, val2, val3, &sensor)) {
				if (rfxcomConfig->rfx_device[sensor].enabled) {
					// Send the xAP-BSC message event
					if (!sendxAPBscEventMsgRFX(sensor)) {
						#ifdef DEBUG_APP
						printf("processRfxsensorMsg: Failed to send xAP-BSC event message\n");
						#endif
						syslog(LOG_ERR, "processRfxsensorMsg: Failed to send xAP-BSC event message");
						logError("processRfxsensorMsg: Failed to send xAP-BSC event message");
					}
				}
			}
			else {
				#ifdef DEBUG_APP
				printf("processRfxsensorMsg: xAP-BSC endpoint not found\n");
				#endif
				syslog(LOG_ERR, "processRfxsensorMsg: xAP-BSC endpoint not found");
				logError("processRfxsensorMsg: xAP-BSC endpoint not found");
			}
			break;

		default:
			break;
	}
}

// Read the serial data from the Rfxcom unit.
void serialHandler(int fd, void *data) {
	
	char buffer[16];		// Input buffer
	int bytes_to_read;		// Bytes to read
	int bytes_readed;		// Bytes readed
	BYTE msgType;			// message type

	// Check for a message in the  buffer
	if (read(fd, buffer, 1) > 0) {
		// Set message length at first byte
		buffer[0] = buffer[0] & 0x7F;
		
		// Calculate num of bytes to read
		bytes_to_read = (buffer[0] % 8) ? (buffer[0]/8) + 1 : (buffer[0]/8);

		// Read message
		bytes_readed = 0;
		while (bytes_readed < bytes_to_read) {
			bytes_readed += read(fd, buffer + bytes_readed + 1, bytes_to_read - bytes_readed);
		}
		
		#ifdef DEBUG_APP
		int i;
		for(i = 0; i < (bytes_to_read+1); i++)
			printf("%02X", buffer[i] & 0xFF);
		printf("\n");
		#endif
				
		// Check if is a X10RF or a DM10 message
		if (isX10RF(buffer))
			processX10RfMsg(buffer);
		else if (isDM10(buffer))
			processDM10Msg(buffer);
		
		// Check if is a SECURITY message
		else if (isSecurity(buffer, &msgType)) 
			processSecurityMsg(buffer, msgType);
			
		// Check if is a OREGON message
		else if (isOregon(buffer, &msgType)) 
			processOregonMsg(buffer, msgType);
			
		// Check if is a HOMEEASY message
		else if (isHomeEasy(buffer)) 
			processHomeEasyMsg(buffer);
			
		// Check if is a KOPPLA message
		else if (isKoppla(buffer)) 
			processKopplaMsg(buffer);
			
		// Check if is a RFXSENSOR message
		else if (isRfxSensor(buffer, &msgType)) 
			processRfxsensorMsg(buffer, msgType);
	}
}


//*************************************************************************
//								MAIN PROGRAM 
//*************************************************************************

int main(void) {
	
	BYTE flgRestart = FALSE;					// if TRUE, restart main loop
	
	int commport_fd = -1;						// File descriptor for serial port
	
	time_t timenow;								// Current time
	time_t heartbeattick;						// Time for next hearbeat message
	time_t infotick;							// Time for next info message
	time_t webservicetick;						// Time for next web service message
	
	char xapuid[XAP_UID_LEN];					// Var to store xAP uid's
	char xapmessage[XAP_MSG_SIZE];				// Var to store xAP messages
	
	int i;										// Auxiliary variables, multiple use
	
	fd_set rdfs;								// Vars for attent to clients
	struct timeval tv;
	
	// Header verbage
	#ifdef DEBUG_APP
	printf("\nHomected xAP-Rfxcom Connector\n");
	printf("Copyright (C) Jose Luis Galindo, 2012\n");
	#endif
	
	// Create shared memory areas
	if (!rfxcomSharedMemSetup()) {
		#ifdef DEBUG_APP
		printf("Error allocating shared resources\n");
		#endif
		logError("main: Error allocating shared resources");
		rfxcomSharedMemClose();
		unlink(pid_filepath);
		exit(EXIT_FAILURE);
	}
	
	// Initialize application
	init();
	commport_fd = -1;
	logInit(LOG_EVENTS_FILE, LOG_ERRORS_FILE);
	LIBXML_TEST_VERSION
	
	// Create the message queue for X10RF messages with access only for the owner
	if ((x10RfMsgQueue = msgget(MSGQUEUE_X10RF, IPC_CREAT | 0644)) == -1) {
		#ifdef DEBUG_APP
		printf("Error creating X10RF message queue\n");
		#endif
		logError("main: Error creating X10RF message queue");
		unlink(pid_filepath);
		exit(EXIT_FAILURE);
	}
	
	// Create the process
	#ifndef DEBUG_APP
	process_init("xap-rfxcom", pid_filepath);
	#endif
	
	while(process_state == PROC_RUNNING) {
	
		// Load xml file with general settings
		if (parseXmlSettings(SETTINGS_FILE) > 0) {
			syslog(LOG_ERR, "main: Failed to parse xml settings document, default values loaded");
			logError("main: Failed to parse xml settings document, default values loaded");
			if (saveXmlSettings(SETTINGS_FILE) > 0) {
				syslog(LOG_ERR, "main: Error saving settings file");
				logError("main: Error saving settings file");
				unlink(pid_filepath);
				exit(EXIT_FAILURE);
			}
		}
		
		// Init xAP interface
		g_xap_rxport = xapSetup(rfxcomConfig->interfacename, rfxcomConfig->xap_port);
		#ifdef DEBUG_APP
		printf("main: xAP Port: %d\n", g_xap_rxport);
		#endif
		logEvent(TRUE, "main: xAP Port: %d", g_xap_rxport);
		
		// force heartbeat and info messages on startup
		heartbeattick = time((time_t*)0); 
		infotick = time((time_t*)0);  
		
		#ifdef DEBUG_APP
		printf("Running...\n");
		#endif
		logEvent(TRUE, "main: Running...");
	
		while (!flgRestart && (process_state == PROC_RUNNING)) {
			
			// Wait for application enabled
			if (!rfxcomConfig->enabled) {
				#ifdef DEBUG_APP
				printf("Device is not enabled, wait\n");
				#endif
				logEvent(TRUE, "main: Device is not enabled, wait");
				if (commport_fd != -1) {
					// Send stop hearbeats
					xapBuildUid(xapuid, rfxcomConfig->xap_uid, 0);
					xapSendHbShutdown(rfxcomConfig->xap_addr, xapuid, rfxcomConfig->xap_hbeat, g_xap_rxport);
				}
				while(!rfxcomConfig->enabled) 
					sleep(1);
			}
			
			// Open serial port if not already open
			if (commport_fd == -1) {
				if ((commport_fd = rfxcomOpenSerialPort()) < 0) {
					#ifdef DEBUG_APP
					printf("Failed to open serial port %s\n", rfxcomConfig->commport);
					#endif
					syslog(LOG_ERR, "main: Failed to open serial port %s", rfxcomConfig->commport);
					logError("main: Failed to open serial port %s", rfxcomConfig->commport);
					#ifndef DEBUG_APP
					unlink(pid_filepath);
					exit(EXIT_FAILURE);
					#endif
				}
			}
			else {
				serialHandler(commport_fd, NULL);
			}
			
			// Get current time
			timenow = time((time_t*)0);
		
			// Heartbeat tick
			if (timenow >= heartbeattick) {
				#ifdef DEBUG_APP
				printf("Outgoing heartbeat tick %d\n",(int)timenow);
				#endif
				logEvent(TRUE, "main: Outgoing heartbeat tick %d",(int)timenow);
				
				// Create the heartbeat message
				xapBuildUid(xapuid, rfxcomConfig->xap_uid, 0);
				xapSendHbeat(rfxcomConfig->xap_addr, xapuid, rfxcomConfig->xap_hbeat, g_xap_rxport);

				// Set next tick
				heartbeattick = timenow + rfxcomConfig->xap_hbeat;
			}
			
			// Info tick
			if (timenow >= infotick) {
				#ifdef DEBUG_APP
				printf("Outgoing Info tick %d\n",(int)timenow);
				#endif
				logEvent(TRUE, "main: Outgoing info tick %d",(int)timenow);

				// Send the info message for X10 devices
				if (!rfxcomConfig->disable_x10 && !rfxcomConfig->disable_x10_xapbsc_sch) {
					for (i=0; i < g_next_x10_device; i++) {
						if (rfxcomConfig->x10_device[i].enabled) {
							if (!sendxAPBscInfoMsgX10(i)) {
								#ifdef DEBUG_APP
								printf("Impossible to send xAP BSC Info message\n");
								#endif
								logEvent(TRUE, "main: Impossible to send xAP BSC Info message");
							}
						}
					}
				}

				// Send the info message for SECURITY devices
				for (i=0; i < g_next_sec_device; i++) {
					if (rfxcomConfig->sec_device[i].enabled) {
						if (!sendxAPBscInfoMsgSEC(i)) {
							#ifdef DEBUG_APP
							printf("Impossible to send xAP BSC Info message\n");
							#endif
							logEvent(TRUE, "main: Impossible to send xAP BSC Info message");
						}
					}
				}
				
				// Send the info message for OREGON devices
				for (i=0; i < g_next_ore_device; i++) {
					if (rfxcomConfig->ore_device[i].enabled) {
						if (!sendxAPBscInfoMsgORE(i)) {
							#ifdef DEBUG_APP
							printf("Impossible to send xAP BSC Info message\n");
							#endif
							logEvent(TRUE, "main: Impossible to send xAP BSC Info message");
						}
					}
				}
				
				// Send the info message for HOMEEASY devices
				for (i=0; i < g_next_he_device; i++) {
					if (rfxcomConfig->he_device[i].enabled) {
						if (!sendxAPBscInfoMsgHE(i)) {
							#ifdef DEBUG_APP
							printf("Impossible to send xAP BSC Info message\n");
							#endif
							logEvent(TRUE, "main: Impossible to send xAP BSC Info message");
						}
					}
				}
				
				// Send the info message for KOPPLA devices
				for (i=0; i < g_next_kop_device; i++) {
					if (rfxcomConfig->kop_device[i].enabled) {
						if (!sendxAPBscInfoMsgKOP(i)) {
							#ifdef DEBUG_APP
							printf("Impossible to send xAP BSC Info message\n");
							#endif
							logEvent(TRUE, "main: Impossible to send xAP BSC Info message");
						}
					}
				}
				
				// Send the info message for RFXCOM devices
				for (i=0; i < g_next_rfx_device; i++) {
					if (rfxcomConfig->rfx_device[i].enabled) {
						if (!sendxAPBscInfoMsgRFX(i)) {
							#ifdef DEBUG_APP
							printf("Impossible to send xAP BSC Info message\n");
							#endif
							logEvent(TRUE, "main: Impossible to send xAP BSC Info message");
						}
					}
				}
				
				// Set next tick
				infotick = timenow + rfxcomConfig->xap_infof;
			}

			// Web service tick
			if (rfxcomConfig->xap_wserv > 0) {
				if (timenow >= webservicetick) {
					#ifdef DEBUG_APP
					printf("Outgoing web service tick %d\n",(int)timenow);
					#endif
					logEvent(TRUE, "main: Outgoing web service tick %d",(int)timenow);
					
					// Send a server.start message
					if (!sendxAPWebServiceMsg("server.start"))
						logError("main: Impossible to send xAP server.start message");
					
					// Set next tick
					webservicetick = timenow + rfxcomConfig->xap_wserv;
				}
			}
			
			// xAP incomming messages proccessing
			FD_ZERO(&rdfs);									// Reset the file descriptor set
			FD_SET(lxap_rx_sockfd, &rdfs);
			tv.tv_sec = 0;									// Timeout = 100ms.
			tv.tv_usec = 100000;
			select(lxap_rx_sockfd + 1, &rdfs, NULL, NULL, &tv);
			if (xapRead(xapmessage) > -1) {
				processxAPMessage(xapmessage);				// Process incoming xAP messages
				memset(xapmessage, 0, sizeof(xapmessage));	// Clean message
			}
			
			// Check if has to save settings
			if (rfxcomConfig->saveFlag) {
				rfxcomConfig->saveFlag = FALSE;				// Reset flag
				lxap_xap_version = rfxcomConfig->xap_ver;	// Update library version var
				if (saveXmlSettings(SETTINGS_FILE) > 0) {
					syslog(LOG_ERR, "main: Error saving settings file");
					logError("main: Error saving settings file");
					unlink(pid_filepath);
					exit(EXIT_FAILURE);
				}
			}
			
			// Check if has to restart
			if (rfxcomConfig->restartFlag) {
				rfxcomConfig->restartFlag = FALSE;			// Reset flag
				flgRestart = TRUE;
			}
		}
		
		// Restore flgRestart
		flgRestart = FALSE;
		
		// Save xml settings
		if (saveXmlSettings(SETTINGS_FILE) > 0) {
			syslog(LOG_ERR, "main: Error saving settings file");
			logError("main: Error saving settings file");
			unlink(pid_filepath);
			exit(EXIT_FAILURE);
		}
			
		// Send a xAP shutdown message
		xapBuildUid(xapuid, rfxcomConfig->xap_uid, 0);
		if (!xapSendHbShutdown(rfxcomConfig->xap_addr, xapuid, rfxcomConfig->xap_hbeat, g_xap_rxport)) {
			syslog(LOG_ERR, "main: Impossible to send xAP shutdown message");
			logError("main: Impossible to send xAP shutdown message");
		}

		// Close xAP communications
		close(lxap_tx_sockfd);					// Close Transmitter socket
		close(lxap_rx_sockfd);					// Close Receiver socket
	}
	
	// Close X10RF message queue
	msgctl(x10RfMsgQueue, IPC_RMID, (struct msqid_ds *)NULL);
	
	// Close shared memory areas
	rfxcomSharedMemClose();
	
	// Destroy the process
	#ifndef DEBUG_APP
	process_finish();
	#endif
	
	return 0;
}
