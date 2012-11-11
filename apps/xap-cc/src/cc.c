/**************************************************************************

	cc.c

	CurrentCost connector for xAP protocol

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

	24/04/12 by Jose Luis Galindo : Use of xAP library
	28/03/12 by Jose Luis Galindo : Code rewritten, added xml settings
	
	Based on HAH livebox current cost connector from www.DBzoo.com
	Copyright (c) Brett England, 2009
	 

***************************************************************************/
//#define DEBUG_APP						// Set this to debug to console
#include "shared.h"
#include "cc.h"
#include <unistd.h>
#include "libprocess/libprocess.h"

//*************************************************************************
//								APPLICATION FUNCTIONS 
//*************************************************************************

// Initialize connector settings
void init() {
	int i;
	
	// Load default xAP values
	strcpy(ccConfig->interfacename, "eth0");			// Default interface name
	strcpy(ccConfig->xap_addr.vendor, XAP_VENDOR);		// Vendor name is a constant string
	strcpy(ccConfig->xap_addr.device, XAP_DEVICE);		// Device name is a constant string
	strcpy(ccConfig->xap_addr.instance, XAP_INSTANCE);	// Default instance name
	ccConfig->xap_ver = XAP_VER_12;						// Default xAP version
	lxap_xap_version = ccConfig->xap_ver;
	ccConfig->xap_uid = XAP_UID;						// Default UID
	ccConfig->xap_port = XAP_PORT;						// Default Port (Broadcast)
	ccConfig->xap_hbeat = XAP_HBEAT_FREQ;				// Default Heartbeat interval
	ccConfig->xap_infof = XAP_INFO_FREQ;				// Default Info interval
	ccConfig->xap_wserv = XAP_WSERV_FREQ;				// Default Web service interval
	
	// Load default device values
	ccConfig->cc_enabled = FALSE;						// Default enabled value
	ccConfig->cc_model = CC128;							// Default Current Cost device model
	strcpy(ccConfig->cc_serial, "/dev/ttyUSB0");		// Default serial port name
	
	// Init monitor temp sensor
	ccConfig->cc_montemp.hyst = 0;						// Default values for monitor temp sensor
	ccConfig->cc_montemp.temp = 0;	
	ccConfig->cc_montemp.tempold = 0;	
	strcpy(ccConfig->cc_montemp.location, "Temp");
	strcpy(ccConfig->cc_montemp.name, "Monitor");
	strcpy(ccConfig->cc_montemp.units, "");
	strcpy(ccConfig->cc_montemp.desc, "");
	ccConfig->cc_montemp.lastupdate = time((time_t*)0) - SENSOR_TIMEOUT;
	ccConfig->cc_montemp.uidsub = 1;

	// Init sensor data
	for (i=0; i < CC_SENSORS; i++)	{					// Default values for sensor data
	
		ccConfig->cc_sensor[i].enabled = FALSE;
		ccConfig->cc_sensor[i].stype = 0;
		ccConfig->cc_sensor[i].digital = 0;
		ccConfig->cc_sensor[i].hyst = 0;
		ccConfig->cc_sensor[i].ch1 = 0;
		ccConfig->cc_sensor[i].ch1old = 0;
		ccConfig->cc_sensor[i].ch2 = 0;
		ccConfig->cc_sensor[i].ch2old = 0;
		ccConfig->cc_sensor[i].ch3 = 0;
		ccConfig->cc_sensor[i].ch3old = 0;
		ccConfig->cc_sensor[i].imp = 0;
		ccConfig->cc_sensor[i].impold = 0;
		ccConfig->cc_sensor[i].ipu = 0;
		strcpy(ccConfig->cc_sensor[i].location, "sensor");
		sprintf(ccConfig->cc_sensor[i].name, "%d", i);
		strcpy(ccConfig->cc_sensor[i].units, "");
		strcpy(ccConfig->cc_sensor[i].desc, "");
		ccConfig->cc_sensor[i].lastupdate = time((time_t*)0) - SENSOR_TIMEOUT;
		ccConfig->cc_sensor[i].uidsub = 2 + (i * 4);	// SubUID begin at 2 and they are 4 uid's for sensor
	}
	
	// Reset flags
	ccConfig->saveFlag = FALSE;
	ccConfig->restartFlag = FALSE;
}

// Send a xAP BSC Info message for the sensor given.
short int sendxAPBscInfoMsg(int sensor, int channel) {
	
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	char uid[XAP_UID_LEN];
	char state[XAP_UID_LEN];
	char value[XAP_EPVALUE_LEN];
	char display[XAP_EPVALUE_LEN];
	
	if (sensor < 0) {
		// Send temp monitor message
		sprintf(subaddr, "%s.%s.temp", ccConfig->cc_montemp.location, ccConfig->cc_montemp.name);
		xapBuildAddress(fulladdr, ccConfig->xap_addr, subaddr);
		xapBuildUid(uid, ccConfig->xap_uid, ccConfig->cc_montemp.uidsub);
		strcpy(state, XAP_STATE_ON);
		sprintf(value, "%.1f", ccConfig->cc_montemp.temp);
		sprintf(display, "temp %.1f %s", ccConfig->cc_montemp.temp, ccConfig->cc_montemp.units);
		return (xapSendBSCinf(fulladdr, uid, value, state, display, XAP_BSC_INPUT));
	}
	else {
		// Send sensor message
		sprintf(subaddr, "%s.%s.ch.%d", ccConfig->cc_sensor[sensor].location, ccConfig->cc_sensor[sensor].name, channel);
		switch(channel) {
			case CC_CHANNEL_1:
				sprintf(value, "%d", ccConfig->cc_sensor[sensor].ch1);
				break;
			case CC_CHANNEL_2:
				sprintf(value, "%d", ccConfig->cc_sensor[sensor].ch2);
				break;
			case CC_CHANNEL_3:
				sprintf(value, "%d", ccConfig->cc_sensor[sensor].ch3);
				break;
			case CC_CHANNEL_TOTAL:
				sprintf(subaddr, "%s.%s.ch.total", ccConfig->cc_sensor[sensor].location, ccConfig->cc_sensor[sensor].name);
				sprintf(value, "%d", ccConfig->cc_sensor[sensor].ch1 + ccConfig->cc_sensor[sensor].ch2 + ccConfig->cc_sensor[sensor].ch3);
				break;
			case CC_CHANNEL_NONE:
				sprintf(subaddr, "%s.%s.impulse", ccConfig->cc_sensor[sensor].location, ccConfig->cc_sensor[sensor].name);
				sprintf(value, "%ld", ccConfig->cc_sensor[sensor].imp);
				break;
			default:
				return 0;
		}
		xapBuildAddress(fulladdr, ccConfig->xap_addr, subaddr);
		xapBuildUid(uid, ccConfig->xap_uid, ccConfig->cc_sensor[sensor].uidsub + channel - 1);	
		if(ccConfig->cc_sensor[sensor].digital) {
			// for digital sensors send state type messages
			if(atoi(value) == 500)
				strcpy(state, XAP_STATE_ON);
			else
				strcpy(state, XAP_STATE_OFF);
			return (xapSendBSCinf(fulladdr, uid, NULL, state, state, XAP_BSC_INPUT));
		}
		else {
			// for analog sensors send level type messages
			strcpy(state, XAP_STATE_ON);
			sprintf(display, "%s %s", value, ccConfig->cc_sensor[sensor].units);
			return (xapSendBSCinf(fulladdr, uid, value, state, display, XAP_BSC_INPUT));
		}
	}
	return 0;
}

// Send a xAP BSC Event message for the sensor given.
short int sendxAPBscEventMsg(int sensor, int channel) {
	
	char fulladdr[XAP_ADDRESS_LEN];
	char subaddr[XAP_SUBADDR_LEN];
	char uid[XAP_UID_LEN];
	char state[XAP_UID_LEN];
	char value[XAP_EPVALUE_LEN];
	char display[XAP_EPVALUE_LEN];
	
	if (sensor < 0) {
		// Send temp monitor message
		sprintf(subaddr, "%s.%s.temp", ccConfig->cc_montemp.location, ccConfig->cc_montemp.name);
		xapBuildAddress(fulladdr, ccConfig->xap_addr, subaddr);
		xapBuildUid(uid, ccConfig->xap_uid, ccConfig->cc_montemp.uidsub);
		strcpy(state, XAP_STATE_ON);
		sprintf(value, "%.1f", ccConfig->cc_montemp.temp);
		sprintf(display, "temp %.1f %s", ccConfig->cc_montemp.temp, ccConfig->cc_montemp.units);
		return (xapSendBSCevn(fulladdr, uid, value, state, display, XAP_BSC_INPUT));
	}
	else {
		// Send sensor message
		sprintf(subaddr, "%s.%s.ch.%d", ccConfig->cc_sensor[sensor].location, ccConfig->cc_sensor[sensor].name, channel);
		switch(channel) {
			case CC_CHANNEL_1:
				sprintf(value, "%d", ccConfig->cc_sensor[sensor].ch1);
				break;
			case CC_CHANNEL_2:
				sprintf(value, "%d", ccConfig->cc_sensor[sensor].ch2);
				break;
			case CC_CHANNEL_3:
				sprintf(value, "%d", ccConfig->cc_sensor[sensor].ch3);
				break;
			case CC_CHANNEL_TOTAL:
				sprintf(subaddr, "%s.%s.ch.total", ccConfig->cc_sensor[sensor].location, ccConfig->cc_sensor[sensor].name);
				sprintf(value, "%d", ccConfig->cc_sensor[sensor].ch1 + ccConfig->cc_sensor[sensor].ch2 + ccConfig->cc_sensor[sensor].ch3);
				break;
			case CC_CHANNEL_NONE:
				sprintf(subaddr, "%s.%s.impulse", ccConfig->cc_sensor[sensor].location, ccConfig->cc_sensor[sensor].name);
				sprintf(value, "%ld", ccConfig->cc_sensor[sensor].imp);
				break;
			default:
				return 0;
		}
		xapBuildAddress(fulladdr, ccConfig->xap_addr, subaddr);
		xapBuildUid(uid, ccConfig->xap_uid, ccConfig->cc_sensor[sensor].uidsub + channel - 1);	
		if(ccConfig->cc_sensor[sensor].digital) {
			// for digital sensors send state type messages
			if(atoi(value) == 500)
				strcpy(state, XAP_STATE_ON);
			else
				strcpy(state, XAP_STATE_OFF);
			return (xapSendBSCevn(fulladdr, uid, NULL, state, state, XAP_BSC_INPUT));
		}
		else {
			// for analog sensors send level type messages
			strcpy(state, XAP_STATE_ON);
			sprintf(display, "%s %s", value, ccConfig->cc_sensor[sensor].units);
			return (xapSendBSCevn(fulladdr, uid, value, state, display, XAP_BSC_INPUT));
		}
	}
	return 0;
}

// Send web service message
short int sendxAPWebServiceMsg(char *strMsg) {

	char strUID[XAP_UID_LEN];
	char strIcon[XAP_WEB_FIELD_LEN];		// Web service: Icon
	char strUrl[XAP_WEB_FIELD_LEN];			// Web service: Url
	
	// Build the UID
	xapBuildUid(strUID, ccConfig->xap_uid, 0);	
	
	// Set xAP web service params
	sprintf(strIcon, "http://%s%s", lxap_local_IP_addr, WSERV_ICON);
	sprintf(strUrl, "http://%s", lxap_local_IP_addr);
	
	return xapSendWebService(ccConfig->xap_addr, strUID, WSERV_NAME, WSERV_DESC, ccConfig->xap_addr.instance, strIcon, strUrl, strMsg);
}

// Process XAP received messages
short int processxAPMessage(char *message) {
	
	xAP_HEADER header;					// Header struct to collect values from xapReadHead
	char *pXapStr;						// Pointer to the xAP message string
	char fulladdr[XAP_ADDRESS_LEN];		// Temporary buffer
	char subaddr[XAP_ADDRESS_LEN];		// Temporary buffer
	BYTE msgType;						// Message type
	int i, j;
			
	// Extract data from the header
	if ((pXapStr = xapReadHead(message, &header)) != NULL)
	{
		// In case "xapRead", in the xap library, doesn't identify the IP address of the message source,
		// filter by xAP source address, if this message has been sent by our device, ignore it
		xapBuildAddress(fulladdr, ccConfig->xap_addr, NULL);
		if (!strncmp(header.source, fulladdr, strlen(fulladdr)))
			return 1;

		// Process xAPBSC.query messages
		if (!strcasecmp(header.class, "xAPBSC.query")) {

			// Is the message addressed to our device?
			xapBuildAddress(fulladdr, ccConfig->xap_addr, ">");
			if (xapEvalTarget(header.target, fulladdr)) {

				// Search if query is for temp monitor
				sprintf(subaddr, "%s.%s.temp", ccConfig->cc_montemp.location, ccConfig->cc_montemp.name);
				xapBuildAddress(fulladdr, ccConfig->xap_addr, subaddr);
				if (xapEvalTarget(header.target, fulladdr)) 
					sendxAPBscInfoMsg(CC_TEMPMON, CC_CHANNEL_NONE);
						
				// Search if query is for a sensor
				for (i = 0; i < CC_SENSORS; i++) {
					if (ccConfig->cc_sensor[i].enabled) {
						if (ccConfig->cc_sensor[i].stype == CC_STYPE_IMPULSE) {
							// Impulse type sensor
							sprintf(subaddr, "%s.%s.impulse", ccConfig->cc_sensor[i].location, ccConfig->cc_sensor[i].name);
							xapBuildAddress(fulladdr, ccConfig->xap_addr, subaddr);
							if (xapEvalTarget(header.target, fulladdr)) 
								sendxAPBscInfoMsg(i, CC_CHANNEL_NONE);
						}
						else {
							// Check for channel 1 to 3
							for (j = 1; j < 4; j++) {
								sprintf(subaddr, "%s.%s.ch.%d", ccConfig->cc_sensor[i].location, ccConfig->cc_sensor[i].name, j);
								xapBuildAddress(fulladdr, ccConfig->xap_addr, subaddr);
								if (xapEvalTarget(header.target, fulladdr))
									sendxAPBscInfoMsg(i, j);
							}
							
							// Check for channel total
							sprintf(subaddr, "%s.%s.ch.total", ccConfig->cc_sensor[i].location, ccConfig->cc_sensor[i].name);
							xapBuildAddress(fulladdr, ccConfig->xap_addr, subaddr);
							if (xapEvalTarget(header.target, fulladdr))
								sendxAPBscInfoMsg(i, CC_CHANNEL_TOTAL);
						}
					}
				}
			}
		}
	
		// Process web.service messages
		if (!strcasecmp(header.class, "web.service")) {
						
			// Is the message addressed to our device?
			xapBuildAddress(fulladdr, ccConfig->xap_addr, ">");
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
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", ccConfig->xap_addr.instance);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "enabled", "%d", ccConfig->cc_enabled);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "ccmodel", "%d", ccConfig->cc_model);
    if (rc < 0) return 4;
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "ccserial", "%s", ccConfig->cc_serial);
    if (rc < 0) return 4;
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "temphyst", "%.1f", ccConfig->cc_montemp.hyst);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "temploc", "%s", ccConfig->cc_montemp.location);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "tempname", "%s", ccConfig->cc_montemp.name);
    if (rc < 0) return 4;
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "tempunits", "%s", ccConfig->cc_montemp.units);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "tempdesc", "%s", ccConfig->cc_montemp.desc);
    if (rc < 0) return 4;

    // Close node "device"
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) return 5;

	// Create child node "xap"
    rc = xmlTextWriterStartElement(writer, BAD_CAST "xap");
    if (rc < 0) return 2;
    
	// Write <xap> elements
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "iface", "%s", ccConfig->interfacename);
	if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "version", "%d", ccConfig->xap_ver);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "hbeatf", "%d", ccConfig->xap_hbeat);
    if (rc < 0) return 4;
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "infof", "%d", ccConfig->xap_infof);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "wservf", "%d", ccConfig->xap_wserv);
    if (rc < 0) return 4;
    if (ccConfig->xap_ver == XAP_VER_12)
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "uid", "%04X", ccConfig->xap_uid);
	else 
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "uid", "%08X", ccConfig->xap_uid);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "port", "%d", ccConfig->xap_port);
    if (rc < 0) return 4;        
    
    // Close node "xap"
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) return 5;
    
    // Create child node "sensors"
    rc = xmlTextWriterStartElement(writer, BAD_CAST "sensors");
    if (rc < 0) return 2;
    
	// Write <sensors> elements
	int i;
	for (i=0; i < CC_SENSORS; i++)
	{
		// Create child node "sensor"
		rc = xmlTextWriterStartElement(writer, BAD_CAST "sensor");
		if (rc < 0) return 2;
		
	    // Add an attribute with the name of sensor
		rc = xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "name", "%d", i);
		if (rc < 0) return 3;
    
		// Write <sensor> elements
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "enabled", "%d", ccConfig->cc_sensor[i].enabled);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "digital", "%d", ccConfig->cc_sensor[i].digital);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "hyst", "%d", ccConfig->cc_sensor[i].hyst);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "location", "%s", ccConfig->cc_sensor[i].location);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", ccConfig->cc_sensor[i].name);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "units", "%s", ccConfig->cc_sensor[i].units);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "description", "%s", ccConfig->cc_sensor[i].desc);
		if (rc < 0) return 4;
		
		// Close node "sensor"
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
	else if((strcmp("ccmodel", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_CCMODEL;
		return;
	}
	else if((strcmp("ccserial", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_CCSERIAL;
		return;
	}
	else if((strcmp("temphyst", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_TEMPHYST;
		return;
	}
	else if((strcmp("temploc", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_TEMPLOC;
		return;
	}
	else if((strcmp("tempname", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_TEMPNAME;
		return;
	}
	else if((strcmp("tempunits", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_TEMPUNITS;
		return;
	}
	else if((strcmp("tempdesc", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_TEMPDESC;
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
	else if((strcmp("sensor", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSORS)) {
		if (atts != NULL) {
			if (atts[1] != NULL) {
				if(strcmp("name", (char*) atts[0]) == 0) {
					if (atts[1] != NULL) {
						g_settings_tag = SETTINGS_TAG_SENSOR;
						g_curr_sensor = atoi((char*) atts[1]);
					}
				}
			}
		}
        return;
	}
	else if((strcmp("enabled", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSOR)) {
		g_settings_tag = SETTINGS_TAG_S_ENABLED;
		return;
	}
	else if((strcmp("digital", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSOR)) {
		g_settings_tag = SETTINGS_TAG_S_DIGITAL;
		return;
	}
	else if((strcmp("hyst", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSOR)) {
		g_settings_tag = SETTINGS_TAG_S_HYST;
		return;
	}
	else if((strcmp("location", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSOR)) {
		g_settings_tag = SETTINGS_TAG_S_LOC;
		return;
	}
	else if((strcmp("name", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSOR)) {
		g_settings_tag = SETTINGS_TAG_S_NAME;
		return;
	}
	else if((strcmp("units", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSOR)) {
		g_settings_tag = SETTINGS_TAG_S_UNITS;
		return;
	}
	else if((strcmp("description", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_SENSOR)) {
		g_settings_tag = SETTINGS_TAG_S_DESC;
		return;
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
	else if((strcmp("ccmodel", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_CCMODEL) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("ccserial", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_CCSERIAL) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("temphyst", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_TEMPHYST) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("temploc", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_TEMPLOC) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("tempname", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_TEMPNAME) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("tempunits", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_TEMPUNITS) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("tempdesc", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_TEMPDESC) {
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
	// SENSORS TAGS
	else if((strcmp("sensor", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_SENSOR) {
		g_settings_tag = SETTINGS_TAG_SENSORS;
		return;
	}
	else if((strcmp("enabled", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_S_ENABLED) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else if((strcmp("digital", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_S_DIGITAL) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else if((strcmp("hyst", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_S_HYST) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else if((strcmp("location", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_S_LOC) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else if((strcmp("name", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_S_NAME) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else if((strcmp("units", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_S_UNITS) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else if((strcmp("description", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_S_DESC) {
		g_settings_tag = SETTINGS_TAG_SENSOR;
		return;
	}
	else
		g_settings_tag = SETTINGS_TAG_NONE;
}

// SAX value element callback for Xml settings
static void settingsXmlValueElementCallback(void *ctx, const xmlChar *ch, int len) {
	
	char elemvalue[CC_DESC_LEN];		// CC_DESC_LEN is the biggest length for tags

	// Value to STRZ
	int i;
	for (i = 0; (i<len) && (i < sizeof(elemvalue)-1); i++)
		elemvalue[i] = ch[i];
	elemvalue[i] = 0;
	
	//printf("Value: %s", elemvalue);	// Print element value
	
	switch(g_settings_tag) {

		// DEVICE element values
		case SETTINGS_TAG_DEV_NAME:
			if (strlen(elemvalue) <= sizeof(ccConfig->xap_addr.instance))
				strcpy(ccConfig->xap_addr.instance, elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_ENABLED:
			ccConfig->cc_enabled = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_CCMODEL:
			ccConfig->cc_model = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_CCSERIAL:
			if (strlen(elemvalue) <= sizeof(ccConfig->cc_serial))
				strcpy(ccConfig->cc_serial, elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_TEMPHYST:
			ccConfig->cc_montemp.hyst = atof(elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_TEMPLOC:
			if (strlen(elemvalue) <= sizeof(ccConfig->cc_montemp.location))
				strcpy(ccConfig->cc_montemp.location, elemvalue);
			break;
		
		case SETTINGS_TAG_DEV_TEMPNAME:
			if (strlen(elemvalue) <= sizeof(ccConfig->cc_montemp.name))
				strcpy(ccConfig->cc_montemp.name, elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_TEMPUNITS:
			if (strlen(elemvalue) <= sizeof(ccConfig->cc_montemp.units))
				strcpy(ccConfig->cc_montemp.units, elemvalue);
			break;
		
		case SETTINGS_TAG_DEV_TEMPDESC:
			if (strlen(elemvalue) <= sizeof(ccConfig->cc_montemp.desc))
				strcpy(ccConfig->cc_montemp.desc, elemvalue);
			break;

		// XAP element values
		case SETTINGS_TAG_XAP_IFACE:
			if (strlen(elemvalue) <= sizeof(ccConfig->interfacename))
				strcpy(ccConfig->interfacename, elemvalue);
			break;
			
		case SETTINGS_TAG_XAP_VERSION:
			ccConfig->xap_ver = atoi(elemvalue);
			lxap_xap_version = ccConfig->xap_ver;
			break;
			
		case SETTINGS_TAG_XAP_HBEATF:
			ccConfig->xap_hbeat = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_XAP_INFOF:
			ccConfig->xap_infof = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_XAP_WSERVF:
			ccConfig->xap_wserv = atoi(elemvalue);
			break;
		
		case SETTINGS_TAG_XAP_UID:
			ccConfig->xap_uid = strtoul(elemvalue, 0, 16);
			break;
			
		case SETTINGS_TAG_XAP_PORT:
			ccConfig->xap_port = atoi(elemvalue);
			break;
			
		// SENSORS element values
		case SETTINGS_TAG_S_ENABLED:
			ccConfig->cc_sensor[g_curr_sensor].enabled = atoi(elemvalue);
			break;
			
        case SETTINGS_TAG_S_DIGITAL:
			ccConfig->cc_sensor[g_curr_sensor].digital = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_S_HYST:
			ccConfig->cc_sensor[g_curr_sensor].hyst = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_S_LOC:
			if (strlen(elemvalue) <= sizeof(ccConfig->cc_sensor[g_curr_sensor].location))
				strcpy(ccConfig->cc_sensor[g_curr_sensor].location, elemvalue);
			break;
			
		case SETTINGS_TAG_S_NAME:
			if (strlen(elemvalue) <= sizeof(ccConfig->cc_sensor[g_curr_sensor].name))
				strcpy(ccConfig->cc_sensor[g_curr_sensor].name, elemvalue);
			break;
			
		case SETTINGS_TAG_S_UNITS:
			if (strlen(elemvalue) <= sizeof(ccConfig->cc_sensor[g_curr_sensor].units))
				strcpy(ccConfig->cc_sensor[g_curr_sensor].units, elemvalue);
			break;
			
		case SETTINGS_TAG_S_DESC:
			if (strlen(elemvalue) <= sizeof(ccConfig->cc_sensor[g_curr_sensor].desc))
				strcpy(ccConfig->cc_sensor[g_curr_sensor].desc, elemvalue);
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

// Open CurrentCost serial port
int ccOpenSerialPort() {
	int fd;
	
	if(strncmp(ccConfig->cc_serial, "/dev/", 5) == 0) {
		struct termios newtio;
		
		fd = open(ccConfig->cc_serial, O_RDONLY | O_NDELAY);
		cfmakeraw(&newtio);
		switch(ccConfig->cc_model) {
			case CC128:
				newtio.c_cflag = B57600 | CS8 | CLOCAL | CREAD ;
				break;
				
			case ORIGINAL:
				newtio.c_cflag = B2400 | CS8 | CLOCAL | CREAD ;
				break;
	  
			default:
                newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD ;
		}
		newtio.c_iflag = IGNPAR;
		newtio.c_lflag = ~ICANON;
		newtio.c_cc[VTIME] = 0; // ignore timer
		newtio.c_cc[VMIN] = 0; // no blocking read
		tcflush(fd, TCIFLUSH);
		tcsetattr(fd, TCSANOW, &newtio);
	}
	else {
		fd = open(ccConfig->cc_serial, O_RDONLY);
	}
	return fd;
}

// Process a CurrentCost message
void processCcMessage() {
	
	// Check if Monitor Temp has changed above hyst
	if((ccConfig->cc_montemp.temp >= (ccConfig->cc_montemp.tempold + ccConfig->cc_montemp.hyst)) ||
		(ccConfig->cc_montemp.temp <= (ccConfig->cc_montemp.tempold - ccConfig->cc_montemp.hyst))) {
	
		// Update old temp to calculate new hyst limits
		ccConfig->cc_montemp.tempold = ccConfig->cc_montemp.temp;
			
		// Send message
		#ifdef DEBUG_APP
		printf("Monitor Temp: %.1f\n", ccConfig->cc_montemp.temp);
		#endif
		logEvent(TRUE, "ccXmlEndElementCallback: Monitor Temp: %.1f", ccConfig->cc_montemp.temp);
		sendxAPBscEventMsg(CC_TEMPMON, CC_CHANNEL_NONE);
	}
		
	// Check if sensor data has to be send
	if (ccConfig->cc_sensor[g_curr_sensor].enabled) {
		if (ccConfig->cc_sensor[g_curr_sensor].stype == CC_STYPE_IMPULSE) {
			// If sensor is impulse type
			long total, totalold;
			total = ccConfig->cc_sensor[g_curr_sensor].imp;
			totalold = ccConfig->cc_sensor[g_curr_sensor].impold;
			if((total >= (totalold + ccConfig->cc_sensor[g_curr_sensor].hyst)) ||
				(total <= (totalold - ccConfig->cc_sensor[g_curr_sensor].hyst)))
			{
				// Update old data to calculate new hyst limits
				ccConfig->cc_sensor[g_curr_sensor].impold = ccConfig->cc_sensor[g_curr_sensor].imp;
					
				// Send message
				#ifdef DEBUG_APP
				printf("Sensor %d: Impulse count(%ld) Impulses per unit(%d)\n", g_curr_sensor, ccConfig->cc_sensor[g_curr_sensor].imp, ccConfig->cc_sensor[g_curr_sensor].ipu);
				#endif
				logEvent(TRUE, "ccXmlEndElementCallback: Sensor%d Impulse count(%ld) Impulses per unit(%d)", g_curr_sensor, ccConfig->cc_sensor[g_curr_sensor].imp, ccConfig->cc_sensor[g_curr_sensor].ipu);
				sendxAPBscEventMsg(g_curr_sensor, CC_CHANNEL_NONE);
			}
		}
		else {
			// If sensor is electric, water or gas type
			int total, totalold;
			total  = ccConfig->cc_sensor[g_curr_sensor].ch1;
			total += ccConfig->cc_sensor[g_curr_sensor].ch2;
			total += ccConfig->cc_sensor[g_curr_sensor].ch3;
			totalold  = ccConfig->cc_sensor[g_curr_sensor].ch1old;
			totalold += ccConfig->cc_sensor[g_curr_sensor].ch2old;
			totalold += ccConfig->cc_sensor[g_curr_sensor].ch3old;
			if((total >= (totalold + ccConfig->cc_sensor[g_curr_sensor].hyst)) ||
				(total <= (totalold - ccConfig->cc_sensor[g_curr_sensor].hyst)))
			{
				// Update old data to calculate new hyst limits
				ccConfig->cc_sensor[g_curr_sensor].ch1old = ccConfig->cc_sensor[g_curr_sensor].ch1;
				ccConfig->cc_sensor[g_curr_sensor].ch2old = ccConfig->cc_sensor[g_curr_sensor].ch2;
				ccConfig->cc_sensor[g_curr_sensor].ch3old = ccConfig->cc_sensor[g_curr_sensor].ch3;
					
				#ifdef DEBUG_APP
				printf("Sensor %d: CH1(%dW) CH2(%dW) CH3(%dW)\n", g_curr_sensor, ccConfig->cc_sensor[g_curr_sensor].ch1, ccConfig->cc_sensor[g_curr_sensor].ch2, ccConfig->cc_sensor[g_curr_sensor].ch3);
				#endif
				logEvent(TRUE, "ccXmlEndElementCallback: Sensor%d CH1(%dW) CH2(%dW) CH3(%dW)", g_curr_sensor, ccConfig->cc_sensor[g_curr_sensor].ch1, ccConfig->cc_sensor[g_curr_sensor].ch2, ccConfig->cc_sensor[g_curr_sensor].ch3);
					
				// Send message for channel 1
				sendxAPBscEventMsg(g_curr_sensor, CC_CHANNEL_1);
					
				// Send message for channel 2 if contain data
				if (ccConfig->cc_sensor[g_curr_sensor].ch2 > 0)	
					sendxAPBscEventMsg(g_curr_sensor, CC_CHANNEL_2);
							
				// Send message for channel 3 if contain data
				if (ccConfig->cc_sensor[g_curr_sensor].ch3 > 0)
					sendxAPBscEventMsg(g_curr_sensor, CC_CHANNEL_3);
							
				// Send message for total if ch2 and ch3 contain data
				if ((ccConfig->cc_sensor[g_curr_sensor].ch2 > 0) && (ccConfig->cc_sensor[g_curr_sensor].ch3 > 0)) 
					sendxAPBscEventMsg(g_curr_sensor, CC_CHANNEL_TOTAL);
			}
		}
	}
	return;
}

// SAX start element (TAG) callback for CurrentCost
static void ccXmlStartElementCallback(void *ctx, const xmlChar *name, const xmlChar **atts) {

	//printf("<%s>", name);	// Print start tag
	
	// COMMON Tags
	if(strcmp("msg", (char*) name) == 0) {
		g_cc_tag = CC_TAG_MSG;
        return;
	}
	else if((strcmp("src", (char*) name) == 0) && (g_cc_tag == CC_TAG_MSG)) {
		g_cc_tag = CC_TAG_SRC;
        return;
	}
	else if((strcmp("dsb", (char*) name) == 0) && (g_cc_tag == CC_TAG_MSG)) {
		g_cc_tag = CC_TAG_DSB;
        return;
	}
	else if((strcmp("time", (char*) name) == 0) && (g_cc_tag == CC_TAG_MSG)) {
		g_cc_tag = CC_TAG_TIME;
        return;
	}
	// CURRENT Tags
	else if((strcmp("tmpr", (char*) name) == 0) && (g_cc_tag == CC_TAG_MSG)) {
		g_cc_tag = CC_TAG_CURR_TMPR;
		return;
	}
	else if((strcmp("tmprF", (char*) name) == 0) && (g_cc_tag == CC_TAG_MSG)) {
		g_cc_tag = CC_TAG_CURR_TMPR;
		return;
	}
	else if((strcmp("sensor", (char*) name) == 0) && (g_cc_tag == CC_TAG_MSG)) {
		g_cc_tag = CC_TAG_CURR_SENSOR;
		return;
	}
	else if((strcmp("id", (char*) name) == 0) && (g_cc_tag == CC_TAG_MSG)) {
		g_cc_tag = CC_TAG_CURR_ID;
		return;
	}
	else if((strcmp("type", (char*) name) == 0) && (g_cc_tag == CC_TAG_MSG)) {
		g_cc_tag = CC_TAG_CURR_TYPE;
		return;
	}
	else if((strcmp("imp", (char*) name) == 0) && (g_cc_tag == CC_TAG_MSG)) {
		g_cc_tag = CC_TAG_CURR_IMP;		// Only for Type 2 sensor
		return;
	}
	else if((strcmp("ipu", (char*) name) == 0) && (g_cc_tag == CC_TAG_MSG)) {
		g_cc_tag = CC_TAG_CURR_IPU;		// Only for Type 2 sensor
		return;
	}
	else if((strcmp("ch1", (char*) name) == 0) && (g_cc_tag == CC_TAG_MSG)) {
		g_cc_tag = CC_TAG_CURR_CH;
		g_curr_channel = CC_CHANNEL_1;
		ccConfig->cc_sensor[g_curr_sensor].ch1 = 0;	// Reset channel 1
		ccConfig->cc_sensor[g_curr_sensor].ch2 = 0;	// Reset channel 2
		ccConfig->cc_sensor[g_curr_sensor].ch3 = 0;	// Reset channel 3
		return;
	}
	else if((strcmp("ch2", (char*) name) == 0) && (g_cc_tag == CC_TAG_MSG)) {
		g_cc_tag = CC_TAG_CURR_CH;
		g_curr_channel = CC_CHANNEL_2;
		return;
	}
	else if((strcmp("ch3", (char*) name) == 0) && (g_cc_tag == CC_TAG_MSG)) {
		g_cc_tag = CC_TAG_CURR_CH;
		g_curr_channel = CC_CHANNEL_3;
		return;
	}
	else if((strcmp("watts", (char*) name) == 0) && (g_cc_tag == CC_TAG_CURR_CH)) {
		g_cc_tag = CC_TAG_CURR_CH_WATTS;
		return;
	}
	// HYST Tags
	else if((strcmp("hist", (char*) name) == 0) && (g_cc_tag == CC_TAG_MSG)) {
		g_cc_tag = CC_TAG_HIST;
		return;
	}
}

// SAX end element (TAG) callback for CurrentCost
static void ccXmlEndElementCallback(void *ctx, const xmlChar *name) {
	
	//printf("</%s>", name);	// Print end tag
	
	// COMMON Tags
	if((strcmp("src", (char*) name) == 0) && g_cc_tag == CC_TAG_SRC) {
		g_cc_tag = CC_TAG_MSG;
		return;
	}
	else if((strcmp("dsb", (char*) name) == 0) && g_cc_tag == CC_TAG_DSB) {
		g_cc_tag = CC_TAG_MSG;
		return;
	}
	else if((strcmp("time", (char*) name) == 0) && g_cc_tag == CC_TAG_TIME) {
		g_cc_tag = CC_TAG_MSG;
		return;
	}
	// CURRENT Tags
	else if((strcmp("tmpr", (char*) name) == 0) && g_cc_tag == CC_TAG_CURR_TMPR) {
		g_cc_tag = CC_TAG_MSG;
		g_cc_tag_hist_msg = FALSE;
		return;
	}
	else if((strcmp("tmprF", (char*) name) == 0) && g_cc_tag == CC_TAG_CURR_TMPR) {
		g_cc_tag = CC_TAG_MSG;
		g_cc_tag_hist_msg = FALSE;
		return;
	}
	else if((strcmp("sensor", (char*) name) == 0) && g_cc_tag == CC_TAG_CURR_SENSOR) {
		g_cc_tag = CC_TAG_MSG;
		g_cc_tag_hist_msg = FALSE;
		// Store time of sensor update
		ccConfig->cc_montemp.lastupdate = time((time_t*)0);
		ccConfig->cc_sensor[g_curr_sensor].lastupdate = time((time_t*)0);
		return;
	}
	else if((strcmp("id", (char*) name) == 0) && g_cc_tag == CC_TAG_CURR_ID) {
		g_cc_tag = CC_TAG_MSG;
		g_cc_tag_hist_msg = FALSE;
		return;
	}
	else if((strcmp("type", (char*) name) == 0) && g_cc_tag == CC_TAG_CURR_TYPE) {
		g_cc_tag = CC_TAG_MSG;
		g_cc_tag_hist_msg = FALSE;
		return;
	}
	else if((strcmp("imp", (char*) name) == 0) && g_cc_tag == CC_TAG_CURR_IMP) {
		g_cc_tag = CC_TAG_MSG;
		g_cc_tag_hist_msg = FALSE;
		return;
	}
	else if((strcmp("ipu", (char*) name) == 0) && g_cc_tag == CC_TAG_CURR_IPU) {
		g_cc_tag = CC_TAG_MSG;
		g_cc_tag_hist_msg = FALSE;
		return;
	}
	else if((strcmp("watts", (char*) name) == 0) && g_cc_tag == CC_TAG_CURR_CH_WATTS) {
		g_cc_tag = CC_TAG_CURR_CH;
		g_cc_tag_hist_msg = FALSE;
		return;
	}
	else if((strcmp("ch1", (char*) name) == 0) && g_cc_tag == CC_TAG_CURR_CH) {
		g_cc_tag = CC_TAG_MSG;
		g_cc_tag_hist_msg = FALSE;
		return;
	}
	else if((strcmp("ch2", (char*) name) == 0) && g_cc_tag == CC_TAG_CURR_CH) {
		g_cc_tag = CC_TAG_MSG;
		g_cc_tag_hist_msg = FALSE;
		return;
	}
	else if((strcmp("ch3", (char*) name) == 0) && g_cc_tag == CC_TAG_CURR_CH) {
		g_cc_tag = CC_TAG_MSG;
		g_cc_tag_hist_msg = FALSE;
		return;
	}
	// HYST Tags
	else if((strcmp("hist", (char*) name) == 0) && g_cc_tag == CC_TAG_HIST) {
		g_cc_tag = CC_TAG_MSG;
		g_cc_tag_hist_msg = TRUE;
		return;
	}
	// MSG Tag
	else if((strcmp("msg", (char*) name) == 0) && g_cc_tag == CC_TAG_MSG) {
		g_cc_tag = CC_TAG_NONE;
		if (!g_cc_tag_hist_msg) {
			// If the message is not a history message...
			processCcMessage();
		}
		return;
	}
	else
		g_settings_tag = SETTINGS_TAG_NONE;
}

// SAX value element callback for CurrentCost
static void ccXmlValueElementCallback(void *ctx, const xmlChar *ch, int len) {

	char elemvalue[CC_DESC_LEN];		// CC_DESC_LEN is the biggest length for tags

	// Value to STRZ
	int i;
	for (i = 0; (i<len) && (i < sizeof(elemvalue)-1); i++)
		elemvalue[i] = ch[i];
	elemvalue[i] = 0;
	
	//printf("Value: %s", elemvalue);	// Print element value
	
	switch(g_cc_tag) {

		// CURRENT element values
		case CC_TAG_CURR_TMPR:
			ccConfig->cc_montemp.temp = atof(elemvalue);
			break;
			
		case CC_TAG_CURR_SENSOR:
			g_curr_sensor = atoi(elemvalue);
			break;
			
		case CC_TAG_CURR_TYPE:
			ccConfig->cc_sensor[g_curr_sensor].stype = atoi(elemvalue);
			break;
		
		case CC_TAG_CURR_CH_WATTS:
			switch(g_curr_channel)
			{
				case 1:
					// Log sporeous or save value
					if ((ccConfig->cc_sensor[g_curr_sensor].ch1 = atoi(elemvalue)) > 10000)
						logError("Error: Value too high CH1=%s", elemvalue);
					break;
				
				case 2:
					// Log sporeous or save value
					if ((ccConfig->cc_sensor[g_curr_sensor].ch2 = atoi(elemvalue)) > 10000) {
						logError("Error: Value too high CH2=%s", elemvalue);
						//exit(0);
					}
					break;
				
				case 3:
					// Log sporeous or save value
					if ((ccConfig->cc_sensor[g_curr_sensor].ch3 = atoi(elemvalue)) > 10000) {
						logError("Error: Value too high CH3=%s", elemvalue);
						//exit(0);
					}
					break;
			}
			break;
		
		case CC_TAG_CURR_IMP:
			ccConfig->cc_sensor[g_curr_sensor].imp = atol(elemvalue);
			break;
			
		case CC_TAG_CURR_IPU:
			ccConfig->cc_sensor[g_curr_sensor].ipu = atoi(elemvalue);
			break;

	}
}

// Parse a CurrentCost XML message.
void parseXmlCc(char *data, int size) {

	xmlSAXHandler saxHandler;

	#ifdef DEBUG_APP
	printf("\nMessage arrived to SAX Handler\n");
	printf("%s\n\n", data);
	#endif

	memset(&saxHandler, 0, sizeof(saxHandler));
	saxHandler.initialized = XML_SAX2_MAGIC;
	saxHandler.startElement = ccXmlStartElementCallback;	
	saxHandler.characters = ccXmlValueElementCallback;
	saxHandler.endElement = ccXmlEndElementCallback;
 
	g_cc_tag = CC_TAG_NONE;
	g_curr_sensor = CC_TEMPMON;
	g_curr_channel = CC_CHANNEL_NONE;

	if(xmlSAXUserParseMemory(&saxHandler, NULL, data, size) < 0) {
		#ifdef DEBUG_APP
		printf("Failed to parse xml document.\n");
		#endif
		logError("parseXmlCc: Failed to parse xml document");
		return;
	}
	
	// Cleanup function for the XML library.
	xmlCleanupParser();

    // This is to debug memory for regression tests
	xmlMemoryDump();
}

// Read the XML stream from the CurrentCost unit.
void serialHandler(int fd, void *data) {
	
	char serial_buff[128];
    static char serial_xml[4096]= {0};
    static int serial_cursor = 0;
    int i;
    int len;
	char *ch;
	static int bstate = 0, estate = 0;

	while((len = read(fd, serial_buff, sizeof(serial_buff))) > 0) {		
		for(ch = &serial_buff[0], i=0; i < len; i++, ch++) {
			if(*ch == '\r' || *ch == '\n')
				continue;

			// Prevent buffer overruns.
			if(serial_cursor == sizeof(serial_xml)-1) {
				serial_cursor = 0;
			}
            serial_xml[serial_cursor++] = *ch;
            serial_xml[serial_cursor] = 0;

			if (bstate == 0 && *ch == '<') bstate = 1;
			else if (bstate == 1 && *ch == 'm') bstate = 2;
			else if (bstate == 2 && *ch == 's') bstate = 3;
			else if (bstate == 3 && *ch == 'g') bstate = 4;
			else if (bstate == 4 && *ch == '>' && serial_cursor > 5) {
				strcpy(serial_xml,"<msg>");
				serial_cursor = 5;
				bstate = 0;
			}
			else 
				bstate = 0;

			if (estate == 0 && *ch == '<') estate = 1;
			else if (estate == 1 && *ch == '/') estate = 2;
			else if (estate == 2 && *ch == 'm') estate = 3;
			else if (estate == 3 && *ch == 's') estate = 4;
			else if (estate == 4 && *ch == 'g') estate = 5;
			else if (estate == 5 && *ch == '>') {
				parseXmlCc(serial_xml, serial_cursor);
				serial_cursor = 0;
				serial_xml[0] = 0;			
				estate = 0;
			} 
			else 
				estate = 0;
		}
	}
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

//*************************************************************************
//								MAIN PROGRAM 
//*************************************************************************

int main(void) {
	
	BYTE flgRestart = FALSE;					// if TRUE, restart main loop
	
	int serial_fd = -1;							// File descriptor for serial port
	
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
	printf("\nHomected xAP-CurrentCost Connector\n");
	printf("Copyright (C) Jose Luis Galindo, 2012\n");
	#endif
		
	// Create shared memory areas
	if (!ccSharedMemSetup()) {
		#ifdef DEBUG_APP
		printf("Error allocating shared resources\n");
		#endif
		logError("main: Error allocating shared resources");
		#ifndef DEBUG_APP
		unlink(pid_filepath);
		exit(EXIT_FAILURE);
		#endif
	}
	
	// Initialize application
	init();
	serial_fd = -1;
	logInit(LOG_EVENTS_FILE, LOG_ERRORS_FILE);
	LIBXML_TEST_VERSION
	
	// Create the process
	#ifndef DEBUG_APP
	process_init("xap-cc", pid_filepath);
	#endif
	
	while(process_state == PROC_RUNNING) {
	
		// Load xml file with general settings
		if (parseXmlSettings(SETTINGS_FILE) > 0) {
			#ifdef DEBUG_APP
			printf("main: Failed to parse xml settings document, default values loaded\n");
			#endif
			syslog(LOG_ERR, "main: Failed to parse xml settings document, default values loaded");
			logError("main: Failed to parse xml settings document, default values loaded");
			if (saveXmlSettings(SETTINGS_FILE) > 0) {
				#ifdef DEBUG_APP
				printf("main: Error saving settings file\n");
				#endif
				syslog(LOG_ERR, "main: Error saving settings file");
				logError("main: Error saving settings file");
				#ifndef DEBUG_APP
				unlink(pid_filepath);
				exit(EXIT_FAILURE);
				#endif
			}
		}

		// Init xAP interface
		g_xap_rxport = xapSetup(ccConfig->interfacename, ccConfig->xap_port);
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
			if (!ccConfig->cc_enabled) {
				#ifdef DEBUG_APP
				printf("Device is not enabled, wait\n");
				#endif
				logEvent(TRUE, "main: Device is not enabled, wait");
				if (serial_fd != -1) {
					// Send stop hearbeats
					xapBuildUid(xapuid, ccConfig->xap_uid, 0);
					xapSendHbShutdown(ccConfig->xap_addr, xapuid, ccConfig->xap_hbeat, g_xap_rxport);
				}
				while(!ccConfig->cc_enabled) 
					sleep(1);
			}
			
			// Open CurrentCost serial port if not already open
			if (serial_fd == -1) {
				if ((serial_fd = ccOpenSerialPort()) < 0) {
					#ifdef DEBUG_APP
					printf("Failed to open serial port %s\n", ccConfig->cc_serial);
					#endif
					syslog(LOG_ERR, "main: Failed to open serial port %s", ccConfig->cc_serial);
					logError("main: Failed to open serial port %s", ccConfig->cc_serial);
					#ifndef DEBUG_APP
					unlink(pid_filepath);
					exit(EXIT_FAILURE);
					#endif
				}
			}
			else {
				serialHandler(serial_fd, NULL);
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
				xapBuildUid(xapuid, ccConfig->xap_uid, 0);
				xapSendHbeat(ccConfig->xap_addr, xapuid, ccConfig->xap_hbeat, g_xap_rxport);

				// Set next tick
				heartbeattick = timenow + ccConfig->xap_hbeat;
			}

			// Info tick
			if (timenow >= infotick) {
				#ifdef DEBUG_APP
				printf("Outgoing Info tick %d\n",(int)timenow);
				#endif
				logEvent(TRUE, "main: Outgoing info tick %d",(int)timenow);
				// Send the info message for monitor temp
				if (ccConfig->cc_montemp.lastupdate > (time((time_t*)0) - SENSOR_TIMEOUT))
					sendxAPBscInfoMsg(CC_TEMPMON, CC_CHANNEL_NONE);
				
				// Send the info message for active sensors
				for (i=0; i < CC_SENSORS; i++) {
					// Send only if sensor is active and enabled
					if ((ccConfig->cc_sensor[i].lastupdate > (time((time_t*)0) - SENSOR_TIMEOUT)) && ccConfig->cc_sensor[i].enabled) {
						if (ccConfig->cc_sensor[i].stype == CC_STYPE_IMPULSE) {
							// Impulse type sensor
							sendxAPBscInfoMsg(i, CC_CHANNEL_NONE);
						}
						else {
							// Send channel 1
							sendxAPBscInfoMsg(i, CC_CHANNEL_1);
					
							// Send channel 2 if contain data
							if (ccConfig->cc_sensor[i].ch2 > 0)	
								sendxAPBscInfoMsg(i, CC_CHANNEL_2);
							
							// Send channel 3 if contain data
							if (ccConfig->cc_sensor[i].ch3 > 0)	
								sendxAPBscInfoMsg(i, CC_CHANNEL_3);
							
							// Send total if ch2 and ch3 contain data
							if ((ccConfig->cc_sensor[i].ch2 > 0) && (ccConfig->cc_sensor[i].ch3 > 0)) 
								sendxAPBscInfoMsg(i, CC_CHANNEL_TOTAL);
						}
					}
				}	
				
				// Set next tick
				infotick = timenow + ccConfig->xap_infof;
			}

			// Web service tick
			if(ccConfig->xap_wserv > 0) {
				if (timenow >= webservicetick) {
					#ifdef DEBUG_APP
					printf("Outgoing web service tick %d\n",(int)timenow);
					#endif
					logEvent(TRUE, "main: Outgoing web service tick %d",(int)timenow);
					
					// Send a server.start message
					if (!sendxAPWebServiceMsg("server.start"))
						logError("main: Impossible to send xAP server.start message");
					
					// Set next tick
					webservicetick = timenow + ccConfig->xap_wserv;
				}
			}
			
			// xAP incomming messages proccessing
			FD_ZERO(&rdfs);									// Reset the file descriptor set
			FD_SET(lxap_rx_sockfd, &rdfs);
			tv.tv_sec = 0;									// Timeout = 100ms.
			tv.tv_usec = 100000;
			select(lxap_rx_sockfd + 1, &rdfs, NULL, NULL, &tv);
			if (xapRead(xapmessage)) {
				processxAPMessage(xapmessage);				// Process incoming xAP messages
				memset(xapmessage, 0, sizeof(xapmessage));	// Clean message
			}
			
			// Check if has to save settings
			if (ccConfig->saveFlag) {
				ccConfig->saveFlag = FALSE;					// Reset flag
				lxap_xap_version = ccConfig->xap_ver;		// Update library version var
				if (saveXmlSettings(SETTINGS_FILE) > 0) {
					#ifdef DEBUG_APP
					printf("main: Error saving settings file\n");
					#endif
					syslog(LOG_ERR, "main: Error saving settings file");
					logError("main: Error saving settings file");
					#ifndef DEBUG_APP
					unlink(pid_filepath);
					exit(EXIT_FAILURE);
					#endif
				}
			}
			
			// Check if has to restart
			if (ccConfig->restartFlag) {
				ccConfig->restartFlag = FALSE;				// Reset flag
				flgRestart = TRUE;
			}
		}
		
		// Restore flgRestart
		flgRestart = FALSE;
		
		// Save xml settings
		if (saveXmlSettings(SETTINGS_FILE) > 0) {
			#ifdef DEBUG_APP
			printf("main: Error saving settings file\n");
			#endif
			syslog(LOG_ERR, "main: Error saving settings file");
			logError("main: Error saving settings file");
			#ifndef DEBUG_APP
			unlink(pid_filepath);
			exit(EXIT_FAILURE);
			#endif
		}
			
		// Send a xAP shutdown message
		xapBuildUid(xapuid, ccConfig->xap_uid, 0);
		if (!xapSendHbShutdown(ccConfig->xap_addr, xapuid, ccConfig->xap_hbeat, g_xap_rxport)) {
			syslog(LOG_ERR, "main: Impossible to send xAP shutdown message");
			logError("main: Impossible to send xAP shutdown message");
		}

		// Close xAP communications
		close(lxap_tx_sockfd);					// Close Transmitter socket
		close(lxap_rx_sockfd);					// Close Receiver socket
	}
	
	// Close shared memory areas
	ccSharedMemClose();
	
	// Destroy the process
	#ifndef DEBUG_APP
	process_finish();
	#endif
		
	return 0;
}
