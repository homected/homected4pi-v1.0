/**************************************************************************

	cosm.c

	Cosm connector for xAP protocol

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

	25/10/12 by Jose Luis Galindo : Create cosm-uploader as a new process
	24/07/12 by Jose Luis Galindo : Added return codes to libcosm
	12/05/12 by Jose Luis Galindo : Changed pachube references to Cosm
	07/05/12 by Jose Luis Galindo : Add xAP library and libxml2
	01/05/12 by Jose Luis Galindo : Library updated to Pachube API v2
	
	Based on HAH livebox pachube connector from www.DBzoo.com
	Copyright (c) Brett England, 2009
	 

***************************************************************************/

//#define DEBUG_APP						// Set this to debug to console
#include "shared.h"
#include "cosm.h"
#include <unistd.h>
#include "libprocess/libprocess.h"

//*************************************************************************
//								APPLICATION FUNCTIONS 
//*************************************************************************

// Initialize connector settings
void init() {
	int i;
	
	// Load default xAP values
	strcpy(cosmConfig->interfacename, "eth0");				// Default interface name
	strcpy(cosmConfig->xap_addr.vendor, XAP_VENDOR);		// Vendor name is a constant string
	strcpy(cosmConfig->xap_addr.device, XAP_DEVICE);		// Device name is a constant string
	strcpy(cosmConfig->xap_addr.instance, XAP_INSTANCE);	// Default instance name
	cosmConfig->xap_ver = XAP_VER_12;						// Default xAP version
	lxap_xap_version = cosmConfig->xap_ver;
	cosmConfig->xap_uid = XAP_UID;							// Default UID
	cosmConfig->xap_port = XAP_PORT;						// Default Port (Broadcast)
	cosmConfig->xap_hbeat = XAP_HBEAT_FREQ;					// Default Heartbeat interval
	cosmConfig->xap_wserv = XAP_WSERV_FREQ;					// Default Web service interval
	
	// Load default device values
	cosmConfig->enabled = FALSE;							// Default enabled value
	strcpy(cosmConfig->apikey, "place-your-api-here");		// Default API Key
	cosmConfig->updatef = 60;								// Default upload frecuency
	
	// Init datastream data
	cosmConfig->num_dstreams = 0;
	for (i=0; i < MAX_DATASTREAMS; i++)	{					// Default values for datastreams
	
		cosmConfig->dstreams[i].enabled = FALSE;
		cosmConfig->dstreams[i].feed = 0;
		strcpy(cosmConfig->dstreams[i].id, "");
		strcpy(cosmConfig->dstreams[i].xapsource, "");
		strcpy(cosmConfig->dstreams[i].xapclass, "");
		strcpy(cosmConfig->dstreams[i].xapsection, "");
		strcpy(cosmConfig->dstreams[i].xapkey, "");
	}
	
	// Reset flags
	cosmConfig->saveFlag = FALSE;
	cosmConfig->restartFlag = FALSE;
}

// Send web service message
short int sendxAPWebServiceMsg(char *strMsg) {

	char strUID[XAP_UID_LEN];
	char strIcon[XAP_WEB_FIELD_LEN];	// Web service: Icon
	char strUrl[XAP_WEB_FIELD_LEN];		// Web service: Url
	
	// Build the UID
	xapBuildUid(strUID, cosmConfig->xap_uid, 0);	
	
	// Set xAP web service params
	sprintf(strIcon, "http://%s%s", lxap_local_IP_addr, WSERV_ICON);
	sprintf(strUrl, "http://%s", lxap_local_IP_addr);
	
	return xapSendWebService(cosmConfig->xap_addr, strUID, WSERV_NAME, WSERV_DESC, cosmConfig->xap_addr.instance, strIcon, strUrl, strMsg);
}

// Search for a defined body block
short int xapSearchDatastream(char *pBody, char *blockname, char *key, char *keyvalue) {
	
	char strBlockName[XAP_KEY_LEN];		// Block name string
	char strKey[XAP_KEY_LEN];			// Key string
	char strValue[XAP_EPVALUE_LEN];		// Value string
	char *pStr1, *pStr2;				// Pointers to strings
	BYTE lastBody = FALSE;				// Last body flag

	// Form the block name string to found
	sprintf(strBlockName, "%s\n{\n", blockname);

	// Inspect the message body
	pStr1 = pBody;						// Place the pointer at the beginning of the xAP body

	// Search up to the end of the body section
	while (!lastBody) {
		// Search for the block name
		if (!strncasecmp(pStr1, strBlockName, strlen(strBlockName))) {
			pStr1 += strlen(strBlockName);							// Set pointer after block name
			while (pStr1[0] != '}') {
		
				// Capture the key string
				if ((pStr2 = strchr(pStr1, '=')) != NULL) {

					pStr2[0] = 0;									// Key found
					if (strlen(pStr1) < sizeof(strKey)) {			// Only within the limits of strKey
						strcpy(strKey, pStr1);						// Copy the key string into strKey
						pStr1 = pStr2 + 1;							// Move the pointer to the position of the value
						
						// Capture the value string
						if ((pStr2 = strchr(pStr1, '\n')) != NULL) {
							pStr2[0] = 0;							// Value found
							if (strlen(pStr1) < sizeof(strValue)) {	// Only within the limits of strValue
								strcpy(strValue, pStr1);			// Copy the key string into strValue
								pStr1 = pStr2 + 1;					// Move the pointer to the next line

								// Compare the key searched with the key found
								if (!strcasecmp(strKey, key)) {
									strcpy(keyvalue, strValue);		// Key match, copy value and exit
									return 0;						
								}
							}
						}
					}
				}
				else
					pStr1++;
			}
		}
			
		// If block name not found go to the end of the block
		while ((pStr1[0] != '}') && (strlen(pStr1) > 0)) {
			pStr1++;
		}
		
		// If end of message, set last body flag
		if (strlen(pStr1) == 0)
			lastBody = TRUE;
		else
			pStr1 += 2;	// advance '}\n'
	}
	
	return 1;
}

// Process XAP received messages
short int processxAPMessage(char *message) {
	
	xAP_HEADER header;					// Header struct to collect values from xapReadHead
	char *pXapStr;						// Pointer to the xAP message string
	char strValue[XAP_EPVALUE_LEN];		// Value string
	char fulladdr[XAP_ADDRESS_LEN];		// Temporary buffer
	BYTE msgType;						// Message type
	t_UPDATER_MSG updater_msg;			// Messsage for updater
	int i;

	// Extract data from the header
	if ((pXapStr = xapReadHead(message, &header)) != NULL)
	{
		// In case "xapRead", in the xap library, doesn't identify the IP address of the message source,
		// filter by xAP source address, if this message has been sent by our device, ignore it
		xapBuildAddress(fulladdr, cosmConfig->xap_addr, NULL);
		if (!strncmp(header.source, fulladdr, strlen(fulladdr)))
			return 1;

		// Search for the configured datastreams
		for(i = 0; i < cosmConfig->num_dstreams; i++) {
			if (cosmConfig->dstreams[i].enabled) {
				// Check for xAP source
				if (!strcasecmp(header.source, cosmConfig->dstreams[i].xapsource)) {
					// Check for xAP class
					if (!strcasecmp(header.class, cosmConfig->dstreams[i].xapclass)) {
						// Check for xAP section and Value
						if (!xapSearchDatastream(pXapStr, cosmConfig->dstreams[i].xapsection, cosmConfig->dstreams[i].xapkey, strValue)) {
							// Send data to Cosm updater
							updater_msg.ds = i;
							updater_msg.timestamp = time((time_t*)0);
							if (!strcasecmp(strValue, "ON")) 
								strcpy(updater_msg.curr_value, "1");
							else if (!strcasecmp(strValue, "OFF")) 
								strcpy(updater_msg.curr_value, "0");
							else
								strcpy(updater_msg.curr_value, strValue);
							#ifdef DEBUG_APP
							printf("ds:%d sending value %s to Cosm updater\n", i, updater_msg.curr_value);
							#endif
							logEvent(TRUE, "processxAPMessage: (ds:%d) sending value %s to Cosm updater", i, updater_msg.curr_value);
							updater_msg.msg_id = COSM_MSGQUEUE_MSG_ID;
							msgsnd(g_msg_queue, (struct msgbuf *)&updater_msg, sizeof(updater_msg.ds)+sizeof(updater_msg.timestamp)+sizeof(updater_msg.curr_value), IPC_NOWAIT);
						}
					}
				}
			}
		}
	
		// Process web.service messages
		if (!strcasecmp(header.class, "web.service")) {
						
			// Is the message addressed to our device?
			xapBuildAddress(fulladdr, cosmConfig->xap_addr, ">");
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
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", cosmConfig->xap_addr.instance);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "enabled", "%d", cosmConfig->enabled);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "apikey", "%s", cosmConfig->apikey);
    if (rc < 0) return 4;
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "updatef", "%d", cosmConfig->updatef);
    if (rc < 0) return 4;

    // Close node "device"
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) return 5;

	// Create child node "xap"
    rc = xmlTextWriterStartElement(writer, BAD_CAST "xap");
    if (rc < 0) return 2;
    
	// Write <xap> elements
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "iface", "%s", cosmConfig->interfacename);
	if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "version", "%d", cosmConfig->xap_ver);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "hbeatf", "%d", cosmConfig->xap_hbeat);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "wservf", "%d", cosmConfig->xap_wserv);
    if (rc < 0) return 4;
    if (cosmConfig->xap_ver == XAP_VER_12)
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "uid", "%04X", cosmConfig->xap_uid);
	else 
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "uid", "%08X", cosmConfig->xap_uid);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "port", "%d", cosmConfig->xap_port);
    if (rc < 0) return 4;        
    
    // Close node "xap"
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) return 5;
    
    // Create child node "datastreams"
    rc = xmlTextWriterStartElement(writer, BAD_CAST "datastreams");
    if (rc < 0) return 2;
    
	// Write <datastreams> elements
	int i;
	for (i=0; i < cosmConfig->num_dstreams; i++)
	{
		// Create child node "datastream"
		rc = xmlTextWriterStartElement(writer, BAD_CAST "datastream");
		if (rc < 0) return 2;
		
		// Write <datastream> elements
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "enabled", "%d", cosmConfig->dstreams[i].enabled);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "feed", "%d", cosmConfig->dstreams[i].feed);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "id", "%s", cosmConfig->dstreams[i].id);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "xapsource", "%s", cosmConfig->dstreams[i].xapsource);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "xapclass", "%s", cosmConfig->dstreams[i].xapclass);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "xapsection", "%s", cosmConfig->dstreams[i].xapsection);
		if (rc < 0) return 4;
		rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "xapkey", "%s", cosmConfig->dstreams[i].xapkey);
		if (rc < 0) return 4;
		
		// Close node "datastream"
		rc = xmlTextWriterEndElement(writer);
		if (rc < 0) return 5;
	}
    
    // Close node "datastreams"
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
	else if((strcmp("apikey", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_APIKEY;
		return;
	}
	else if((strcmp("updatef", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DEVICE)) {
		g_settings_tag = SETTINGS_TAG_DEV_UPDATEF;
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
	// DATASTREAMS Tags
	else if(strcmp("datastreams", (char*) name) == 0) {
		g_settings_tag = SETTINGS_TAG_DATASTREAMS;
        return;
	}
	else if((strcmp("datastream", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DATASTREAMS)) {
		g_settings_tag = SETTINGS_TAG_DATASTREAM;
        return;
	}
	else if((strcmp("enabled", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DATASTREAM)) {
		g_settings_tag = SETTINGS_TAG_DS_ENABLED;
		return;
	}
	else if((strcmp("feed", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DATASTREAM)) {
		g_settings_tag = SETTINGS_TAG_DS_FEED;
		return;
	}
	else if((strcmp("id", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DATASTREAM)) {
		g_settings_tag = SETTINGS_TAG_DS_ID;
		return;
	}
	else if((strcmp("xapsource", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DATASTREAM)) {
		g_settings_tag = SETTINGS_TAG_DS_XAPSOURCE;
		return;
	}
	else if((strcmp("xapclass", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DATASTREAM)) {
		g_settings_tag = SETTINGS_TAG_DS_XAPCLASS;
		return;
	}
	else if((strcmp("xapsection", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DATASTREAM)) {
		g_settings_tag = SETTINGS_TAG_DS_XAPSECTION;
		return;
	}
	else if((strcmp("xapkey", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_DATASTREAM)) {
		g_settings_tag = SETTINGS_TAG_DS_XAPKEY;
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
	else if((strcmp("apikey", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_APIKEY) {
		g_settings_tag = SETTINGS_TAG_DEVICE;
		return;
	}
	else if((strcmp("updatef", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DEV_UPDATEF) {
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
	// DATASTREAMS TAGS
	else if((strcmp("datastreams", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DATASTREAMS) {
		cosmConfig->num_dstreams = g_curr_datastream;
		g_settings_tag = SETTINGS_TAG_NONE;
		return;
	}
	else if((strcmp("datastream", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DATASTREAM) {
		g_curr_datastream++;
		g_settings_tag = SETTINGS_TAG_DATASTREAMS;
		return;
	}
	else if((strcmp("enabled", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DS_ENABLED) {
		g_settings_tag = SETTINGS_TAG_DATASTREAM;
		return;
	}
	else if((strcmp("feed", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DS_FEED) {
		g_settings_tag = SETTINGS_TAG_DATASTREAM;
		return;
	}
	else if((strcmp("id", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DS_ID) {
		g_settings_tag = SETTINGS_TAG_DATASTREAM;
		return;
	}
	else if((strcmp("xapsource", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DS_XAPSOURCE) {
		g_settings_tag = SETTINGS_TAG_DATASTREAM;
		return;
	}
	else if((strcmp("xapclass", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DS_XAPCLASS) {
		g_settings_tag = SETTINGS_TAG_DATASTREAM;
		return;
	}
	else if((strcmp("xapsection", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DS_XAPSECTION) {
		g_settings_tag = SETTINGS_TAG_DATASTREAM;
		return;
	}
	else if((strcmp("xapkey", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_DS_XAPKEY) {
		g_settings_tag = SETTINGS_TAG_DATASTREAM;
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
			if (strlen(elemvalue) <= sizeof(cosmConfig->xap_addr.instance))
				strcpy(cosmConfig->xap_addr.instance, elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_ENABLED:
			cosmConfig->enabled = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_APIKEY:
			if (strlen(elemvalue) <= sizeof(cosmConfig->apikey))
				strcpy(cosmConfig->apikey, elemvalue);
			break;
			
		case SETTINGS_TAG_DEV_UPDATEF:
			cosmConfig->updatef = atoi(elemvalue);
			break;

		// XAP element values
		case SETTINGS_TAG_XAP_IFACE:
			if (strlen(elemvalue) <= sizeof(cosmConfig->interfacename))
				strcpy(cosmConfig->interfacename, elemvalue);
			break;
			
		case SETTINGS_TAG_XAP_VERSION:
			cosmConfig->xap_ver = atoi(elemvalue);
			lxap_xap_version = cosmConfig->xap_ver;
			break;
			
		case SETTINGS_TAG_XAP_HBEATF:
			cosmConfig->xap_hbeat = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_XAP_WSERVF:
			cosmConfig->xap_wserv = atoi(elemvalue);
			break;
		
		case SETTINGS_TAG_XAP_UID:
			cosmConfig->xap_uid = strtoul(elemvalue, 0, 16);
			break;
			
		case SETTINGS_TAG_XAP_PORT:
			cosmConfig->xap_port = atoi(elemvalue);
			break;
			
		// DATASTREAMS element values
		case SETTINGS_TAG_DS_ENABLED:
			cosmConfig->dstreams[g_curr_datastream].enabled = atoi(elemvalue);
			break;
			
        case SETTINGS_TAG_DS_FEED:
			cosmConfig->dstreams[g_curr_datastream].feed = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_DS_ID:
			if (strlen(elemvalue) <= sizeof(cosmConfig->dstreams[g_curr_datastream].id))
				strcpy(cosmConfig->dstreams[g_curr_datastream].id, elemvalue);
			break;
			
		case SETTINGS_TAG_DS_XAPSOURCE:
			if (strlen(elemvalue) <= sizeof(cosmConfig->dstreams[g_curr_datastream].xapsource))
				strcpy(cosmConfig->dstreams[g_curr_datastream].xapsource, elemvalue);
			break;
			
		case SETTINGS_TAG_DS_XAPCLASS:
			if (strlen(elemvalue) <= sizeof(cosmConfig->dstreams[g_curr_datastream].xapclass))
				strcpy(cosmConfig->dstreams[g_curr_datastream].xapclass, elemvalue);
			break;
			
		case SETTINGS_TAG_DS_XAPSECTION:
			if (strlen(elemvalue) <= sizeof(cosmConfig->dstreams[g_curr_datastream].xapsection))
				strcpy(cosmConfig->dstreams[g_curr_datastream].xapsection, elemvalue);
			break;
			
		case SETTINGS_TAG_DS_XAPKEY:
			if (strlen(elemvalue) <= sizeof(cosmConfig->dstreams[g_curr_datastream].xapkey))
				strcpy(cosmConfig->dstreams[g_curr_datastream].xapkey, elemvalue);
			break;
	}
}

// Parse the settings XML file.
int parseXmlSettings(const char *filename) {

	int retVal = 0;
	
	xmlSAXHandler saxHandler;

	g_curr_datastream = 0;
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

//*************************************************************************
//								MAIN PROGRAM 
//*************************************************************************

int main(void) {
	
	BYTE flgRestart = FALSE;					// if TRUE, restart main loop
	
	time_t timenow;								// Current time
	time_t heartbeattick;						// Time for next hearbeat message
	time_t webservicetick;						// Time for next web service message
	
	char xapuid[XAP_UID_LEN];					// Var to store xAP uid's
	char xapmessage[XAP_MSG_SIZE];				// Var to store xAP messages
	
	fd_set rdfs;								// Vars for attent to clients
	struct timeval tv;
	
	// Header verbage
	#ifdef DEBUG_APP
	printf("\nHomected xAP-Cosm Connector\n");
	printf("Copyright (C) Jose Luis Galindo, 2012\n");
	#endif
		
	// Create shared memory areas
	if (!cosmSharedMemSetup()) {
		#ifdef DEBUG_APP
		printf("Error allocating shared resources\n");
		#endif
		logError("main: Error allocating shared resources");
		unlink(pid_filepath);
		exit(EXIT_FAILURE);
	}
	
	// Initialize application
	init();
	logInit(LOG_EVENTS_FILE, LOG_ERRORS_FILE);
	LIBXML_TEST_VERSION
	
	// Create the message queue to communicate with cosm-updater
	if ((g_msg_queue = msgget(COSM_MSGQUEUE, IPC_CREAT | 0644)) == -1) {
		#ifdef DEBUG_APP
		printf("Error creating message queue\n");
		#endif
		logError("main: Error creating message queue");
		unlink(pid_filepath);
		exit(EXIT_FAILURE);
	}
	
	// Create the process
	#ifndef DEBUG_APP
	process_init("xap-cosm", pid_filepath);
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
		g_xap_rxport = xapSetup(cosmConfig->interfacename, cosmConfig->xap_port);
		#ifdef DEBUG_APP
		printf("main: xAP Port: %d\n", g_xap_rxport);
		#endif
		logEvent(TRUE, "main: xAP Port: %d", g_xap_rxport);
		
		// force heartbeat on startup
		heartbeattick = time((time_t*)0); 
		
		#ifdef DEBUG_APP
		printf("Running...\n");
		#endif
		logEvent(TRUE, "main: Running...");
	
		while (!flgRestart && (process_state == PROC_RUNNING)) {
			
			// Wait for application enabled
			if (!cosmConfig->enabled) {
				#ifdef DEBUG_APP
				printf("Device is not enabled, wait\n");
				#endif
				logEvent(TRUE, "main: Device is not enabled, wait");
				while(!cosmConfig->enabled) 
					sleep(1);
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
				xapBuildUid(xapuid, cosmConfig->xap_uid, 0);
				xapSendHbeat(cosmConfig->xap_addr, xapuid, cosmConfig->xap_hbeat, g_xap_rxport);

				// Set next tick
				heartbeattick = timenow + cosmConfig->xap_hbeat;
			}

			// Web service tick
			if (cosmConfig->xap_wserv > 0) {
				if (timenow >= webservicetick) {
					#ifdef DEBUG_APP
					printf("Outgoing web service tick %d\n",(int)timenow);
					#endif
					logEvent(TRUE, "main: Outgoing web service tick %d",(int)timenow);
					
					// Send a server.start message
					if (!sendxAPWebServiceMsg("server.start"))
						logError("main: Impossible to send xAP server.start message");
					
					// Set next tick
					webservicetick = timenow + cosmConfig->xap_wserv;
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
			if (cosmConfig->saveFlag) {
				cosmConfig->saveFlag = FALSE;				// Reset flag
				lxap_xap_version = cosmConfig->xap_ver;		// Update library version var
				if (saveXmlSettings(SETTINGS_FILE) > 0) {
					syslog(LOG_ERR, "main: Error saving settings file");
					logError("main: Error saving settings file");
					unlink(pid_filepath);
					exit(EXIT_FAILURE);
				}
			}
			
			// Check if has to restart
			if (cosmConfig->restartFlag) {
				cosmConfig->restartFlag = FALSE;			// Reset flag
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
		xapBuildUid(xapuid, cosmConfig->xap_uid, 0);
		if (!xapSendHbShutdown(cosmConfig->xap_addr, xapuid, cosmConfig->xap_hbeat, g_xap_rxport)) {
			syslog(LOG_ERR, "main: Impossible to send xAP shutdown message");
			logError("main: Impossible to send xAP shutdown message");
		}

		// Close xAP communications
		close(lxap_tx_sockfd);					// Close Transmitter socket
		close(lxap_rx_sockfd);					// Close Receiver socket
	}
	
	// Close message queue
	msgctl(g_msg_queue, IPC_RMID, (struct msqid_ds *)NULL);
	
	// Close shared memory areas
	cosmSharedMemClose();
	
	// Destroy the process
	#ifndef DEBUG_APP
	process_finish();
	#endif
	
	return 0;
}
