/**************************************************************************

	hub.c

	Hub connector for xAP protocol (process version)

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

	03/04/12 by Jose Luis Galindo : Application converted to a process
	28/03/12 by Jose Luis Galindo : Code rewritten, added xml settings
	14/12/02 by Patrick Lidstone : First version
	 
	Original code from patrick@lidstone.net attached in zip file.

***************************************************************************/

#include "shared.h"
#include "hub.h"

//*************************************************************************
//								APPLICATION FUNCTIONS 
//*************************************************************************

// Initialize connector settings
void init() {

	// Load default xAP values
	strcpy(hubConfig->interfacename, "eth0");			// Default interface name
	strcpy(hubConfig->xap_addr.vendor, XAP_VENDOR);		// Vendor name is a constant string
	strcpy(hubConfig->xap_addr.device, XAP_DEVICE);		// Device name is a constant string
	strcpy(hubConfig->xap_addr.instance, XAP_INSTANCE);	// Default instance name
	strcpy(hubConfig->xap_uid, XAP_UID);				// Default UID
	hubConfig->xap_port = XAP_PORT;						// Default Port (Broadcast)
	hubConfig->xap_hbeat = XAP_HBEAT;					// Default Heartbeat interval
	hubConfig->saveFlag = FALSE;
	hubConfig->restartFlag = FALSE;
	
	// Initialize hub entries
	int i;
	for (i = 0; i < XAP_MAX_HUB_ENTRIES; i++) {
		g_xap_hubentry[i].is_alive=0;
	}
}

void xaphub_build_heartbeat(char* a_buff) {
	sprintf(a_buff, "xap-hbeat\n{\nv=12\nhop=1\nuid=%s00\nclass=xap-hbeat.alive\nsource=%s.%s.%s\ninterval=%d\nport=%d\n}\n", hubConfig->xap_uid, hubConfig->xap_addr.vendor, hubConfig->xap_addr.device, hubConfig->xap_addr.instance, hubConfig->xap_hbeat, hubConfig->xap_port);
}

void xaphub_build_heartbeat_shutdown(char* a_buff) {
	sprintf(a_buff, "xap-hbeat\n{\nv=12\nhop=1\nuid=%s00\nclass=xap-hbeat.stopped\nsource=%s.%s.%s\ninterval=%d\nport=%d\n}\n", hubConfig->xap_uid, hubConfig->xap_addr.vendor, hubConfig->xap_addr.device, hubConfig->xap_addr.instance, hubConfig->xap_hbeat, hubConfig->xap_port);
}

int xaphub_addentry(int a_port, int a_interval) {
	
	// add new or update existing hub entry, resetting countdown timer
	int i;
	int matched;

	matched = XAP_MAX_HUB_ENTRIES;

	for (i = 0; i < XAP_MAX_HUB_ENTRIES; i++) {
		if (g_xap_hubentry[i].port == a_port) {
			// entry exists, update it
			g_xap_hubentry[i].interval = a_interval;
			g_xap_hubentry[i].timer = a_interval * 2;
			g_xap_hubentry[i].is_alive = 1;
			matched = i;
			//printf("Heartbeat for port %d\n", g_xap_hubentry[i].port);
			break;
		}
	}
	
	if (matched == XAP_MAX_HUB_ENTRIES) {
		// no entry exists, create a new entry in first free slot
		for (i = 0; i < XAP_MAX_HUB_ENTRIES; i++) {
			if (g_xap_hubentry[i].is_alive == 0) {
				// free entry exists, use it it
				g_xap_hubentry[i].port = a_port;
				g_xap_hubentry[i].interval = a_interval;
				g_xap_hubentry[i].timer = a_interval * 2;
				g_xap_hubentry[i].is_alive = 1;
				matched = i;
				//printf("Connecting port %d\n", g_xap_hubentry[i].port);
				break;
			}
		}
	}
	return matched; // value of XAP_MAX_HUB_ENTRIES indicates list full
}

void xaphub_tick(time_t a_interval) {
	// Called regularly. a_interval specifies number of whole seconds elapsed since last call.
	int i;

	for (i=0; i < XAP_MAX_HUB_ENTRIES; i++) {
		// update timer entries. If timer is 0 then ignore hearbeat ticks
		if ((g_xap_hubentry[i].is_alive) && (g_xap_hubentry[i].timer != 0)) {
			g_xap_hubentry[i].timer -= a_interval;

			if (g_xap_hubentry[i].timer <= 0) {
				//printf("Disconnecting port %d due to loss of heartbeat\n", g_xap_hubentry[i].port);
				logEvent(TRUE, "xaphub_tick: Disconnecting port %d due to loss of heartbeat\n", g_xap_hubentry[i].port);
				g_xap_hubentry[i].is_alive = 0; // mark as idle
			}
			break;
		}
	}
}

int xaphub_relay(int a_txsock, const char* a_buf) {

	struct sockaddr_in tx_addr;
	int i, j = 0;

	memset((char *) &tx_addr, 0, sizeof(tx_addr));
	tx_addr.sin_family = AF_INET;		
	tx_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	for (i=0; i < XAP_MAX_HUB_ENTRIES; i++) {
		if (g_xap_hubentry[i].is_alive == 1) {
			// entry exists, use it
			//printf("Relayed to %d\n", g_xap_hubentry[i].port);
			j++;
			tx_addr.sin_port = htons(g_xap_hubentry[i].port);
			sendto(a_txsock, a_buf, strlen(a_buf), 0, (struct sockaddr*)&tx_addr, sizeof(struct sockaddr_in));	
		}
	}
	return j; // number of connected hosts we relayed to
}

int xapmsg_parse(const char* a_buffer) {

	// Parse incoming message.

	auto int i_cursor=0;
	auto int i;
	auto int i_state;

	char i_keyname[XAP_MAX_KEYNAME_LEN+1];
	char i_keyvalue[XAP_MAX_KEYVALUE_LEN+1];
	char i_section[XAP_MAX_SECTION_LEN+1];

	#define START_SECTION_NAME 0
	#define IN_SECTION_NAME    1
	#define START_KEYNAME      2
	#define IN_KEYNAME         3
	#define START_KEYVALUE     4
	#define IN_KEYVALUE        5

	i_state = START_SECTION_NAME;
	g_xap_index = 0;

	for (i=0; i < strlen(a_buffer); i++) {
		switch (i_state) {
			case START_SECTION_NAME:
				if ((a_buffer[i]>32) && (a_buffer[i]<128)) {
					i_state ++;
					i_cursor=0;
					i_section[i_cursor++]=a_buffer[i];
				}
				break;

			case IN_SECTION_NAME:
				if (a_buffer[i]!='{') {
					if (i_cursor < XAP_MAX_SECTION_LEN) i_section[i_cursor++] = a_buffer[i];
				}
				else {
					while (i_section[i_cursor-1] <= 32) {
						i_cursor--; // remove possible single trailing space
					}
					i_section[i_cursor] = '\0';
					i_state++;
				}
				break;

			case START_KEYNAME:
				if (a_buffer[i]=='}') {
					i_state=START_SECTION_NAME; // end of this section
					}
				else
				if ((a_buffer[i] > 32) && (a_buffer[i] < 128)) {
					i_state ++;
					i_cursor=0;
					if (i_cursor < XAP_MAX_KEYNAME_LEN) 
						i_keyname[i_cursor++] = a_buffer[i];
				}
				break;

			case IN_KEYNAME:
				// Key name starts with printable character, ends with printable character
				// but may include embedded spaces
				if ((a_buffer[i]>=32) && (a_buffer[i]!='=')) {
					i_keyname[i_cursor++]=a_buffer[i];
				}
				else {
					if (i_keyname[i_cursor-1]==' ') 
						i_cursor--; // remove possible single trailing space
					i_keyname[i_cursor]='\0';
					i_state++;
				}
				break;

			case START_KEYVALUE:
				if ((a_buffer[i]>32) && (a_buffer[i]<128)) {
					i_state ++;
					i_cursor=0;
					if (i_cursor<XAP_MAX_KEYVALUE_LEN) 
						i_keyvalue[i_cursor++]=a_buffer[i];
				}
				break;

			case IN_KEYVALUE:
				if (a_buffer[i]>=32) {
					i_keyvalue[i_cursor++]=a_buffer[i];
				}
				else { // end of key value pair
				
					i_keyvalue[i_cursor]='\0';
					i_state=START_KEYNAME;

					strcpy(g_xap_msg[g_xap_index].section, i_section);
					strcpy(g_xap_msg[g_xap_index].name, i_keyname);
					strcpy(g_xap_msg[g_xap_index].value, i_keyvalue);
					//printf("XAPLIB Msg: Name=<%s:%s>, Value=<%s>\n",g_xap_msg[g_xap_index].section, g_xap_msg[g_xap_index].name, g_xap_msg[g_xap_index].value);
					g_xap_index++;
					
					if (g_xap_index > XAP_MAX_MSG_ELEMENTS) {
						g_xap_index = 0;
						//printf("XAPLIB Warning: data lost (message too big)\n");
						logError("xapmsg_parse: XAPLIB Warning: data lost (message too big)");
					}
				}
				break;
		} // switch
	} // for
	return g_xap_index;
} // parse

int xapmsg_getvalue(const char* a_keyname, char* a_keyvalue) {

	// Retrieve a keyvalue in form <section>:<keyvalue>
	// Return 1 on success, 0 on failure

	auto int i;
	auto int matched;
	char i_composite_name[XAP_MAX_SECTION_LEN+XAP_MAX_KEYVALUE_LEN+1];

	i=0;
	matched=0;

	while ((!matched) && (i++<g_xap_index)) {
		strcpy(i_composite_name, g_xap_msg[i-1].section);
		strcat(i_composite_name,":");
		strcat(i_composite_name, g_xap_msg[i-1].name);
		if (strcasecmp(i_composite_name, a_keyname)==0) {
			matched=1;
			strcpy(a_keyvalue, g_xap_msg[i-1].value);
		} // if
	} // while
	return matched;
} // getvalue

void xap_handler(const char* a_buf) {
	
	char i_interval[16];
	char i_port[16];

	xapmsg_parse(a_buf);

	if (xapmsg_getvalue("xap-hbeat:interval", i_interval) == 0) {
		//printf("Could not find <%s> in message\n","xap-hbeat.interval"); 
		logError("xap_handler: Could not find <%s> in message","xap-hbeat.interval"); 
	} 
	else {
		if (xapmsg_getvalue("xap-hbeat:port", i_port) == 0) {
			//printf("Could not find <%s> in message\n","xap-hbeat:port"); 
			logError("xap_handler: Could not find <%s> in message","xap-hbeat:port"); 
		} 
		else {
			// Add entry if port is not my self
			if(atoi(i_port) != hubConfig->xap_port)
				xaphub_addentry(atoi(i_port), atoi(i_interval));
		}
	}
}

// SAX save settings
short int settingsXmlSave(const char *filename) {

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

	// Create child node "xap"
    rc = xmlTextWriterStartElement(writer, BAD_CAST "xap");
    if (rc < 0) return 2;
    
	// Write <xap> elements
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "iface", "%s", hubConfig->interfacename);
	if (rc < 0) return 4;
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "instance", "%s", hubConfig->xap_addr.instance);
	if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "hbeatf", "%d", hubConfig->xap_hbeat);
    if (rc < 0) return 4;
	rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "uid", "%s", hubConfig->xap_uid);
    if (rc < 0) return 4;
    rc = xmlTextWriterWriteFormatElement(writer, BAD_CAST "port", "%d", hubConfig->xap_port);
    if (rc < 0) return 4;        
    
    // Close node "xap"
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

	// XAP Tags
	if(strcmp("xap", (char*) name) == 0) {
		g_settings_tag = SETTINGS_TAG_XAP;
        return;
	}
	else if((strcmp("iface", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_XAP)) {
		g_settings_tag = SETTINGS_TAG_XAP_IFACE;
		return;
	}
	else if((strcmp("instance", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_XAP)) {
		g_settings_tag = SETTINGS_TAG_XAP_INSTANCE;
		return;
	}
	else if((strcmp("hbeatf", (char*) name) == 0) && (g_settings_tag == SETTINGS_TAG_XAP)) {
		g_settings_tag = SETTINGS_TAG_XAP_HBEATF;
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
}

// SAX end element (TAG) callback for Xml settings
static void settingsXmlEndElementCallback(void *ctx, const xmlChar *name) {
	
	//printf("</%s>", name);	// Print end tag

	// XAP Tags
	if((strcmp("iface", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_XAP_IFACE) {
		g_settings_tag = SETTINGS_TAG_XAP;
		return;
	}
	else if((strcmp("instance", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_XAP_INSTANCE) {
		g_settings_tag = SETTINGS_TAG_XAP;
		return;
	}
	else if((strcmp("hbeatf", (char*) name) == 0) && g_settings_tag == SETTINGS_TAG_XAP_HBEATF) {
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
	else
		g_settings_tag = SETTINGS_TAG_NONE;
}

// SAX value element callback for Xml settings
static void settingsXmlValueElementCallback(void *ctx, const xmlChar *ch, int len) {
	
	char elemvalue[XML_VALUE_LEN];

	// Value to STRZ
	int i;
	for (i = 0; (i<len) && (i < sizeof(elemvalue)-1); i++)
		elemvalue[i] = ch[i];
	elemvalue[i] = 0;
	
	//printf("Value: %s", elemvalue);	// Print element value
	
	switch(g_settings_tag) {

		// XAP element values
		case SETTINGS_TAG_XAP_IFACE:
			if (strlen(elemvalue) <= sizeof(hubConfig->interfacename))
				strcpy(hubConfig->interfacename, elemvalue);
			break;
			
		case SETTINGS_TAG_XAP_INSTANCE:
			if (strlen(elemvalue) <= sizeof(hubConfig->xap_addr.instance))
				strcpy(hubConfig->xap_addr.instance, elemvalue);
			break;
			
		case SETTINGS_TAG_XAP_HBEATF:
			hubConfig->xap_hbeat = atoi(elemvalue);
			break;
			
		case SETTINGS_TAG_XAP_UID:
			if (strlen(elemvalue) <= sizeof(hubConfig->xap_uid))
				strcpy(hubConfig->xap_uid, elemvalue);
			break;
			
		case SETTINGS_TAG_XAP_PORT:
			hubConfig->xap_port = atoi(elemvalue);
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

//*************************************************************************
//								MAIN PROGRAM 
//*************************************************************************

int main(void) {

	BYTE flgRestart = FALSE;					// if TRUE, restart main loop
	
	struct ifreq interface;						// ioctls to configure network interface
	
	struct sockaddr_in myinterface;				// Interface address
	struct sockaddr_in mynetmask;				// Interface netmask
	struct sockaddr_in mybroadcast;				// Interface broadcast address
	
	int server_sockfd;							// Server socket
	struct sockaddr_in server_address;			// Server address and port
	int heartbeat_sockfd;						// Heartbeat socket
	struct sockaddr_in heartbeat_addr;			// Heartbeat address and port
	int tx_sockfd;								// Tx socket
	int optval, optlen;							// Vars for socket options
		
	time_t timenow;								// Current time
	time_t xaptick;								// Last tick
	time_t heartbeattick;						// Time for next hearbeat tick
	
	char heartbeat_msg[1500];					// Buffer for heartbeat messages
	char buff[1500];							// Buffer for messages
	
	fd_set rdfs;								// Vars for attent to clients
	struct timeval tv;
	struct sockaddr_in client_address;			// 	client address and port
	socklen_t client_len;
		
	int i; 										// Auxiliary variable

	// Header verbage
	//printf("\nHomected xAP-Hub Connector\n");
	//printf("Copyright (C) Jose Luis Galindo, 2012\n");
	
	// Create shared memory areas
	if (!hubSharedMemSetup()) {
		syslog(LOG_ERR, "main: Error allocating shared resources");
		logError("main: Error allocating shared resources");
	}
	
	// Initialize application
	init();
	logInit(LOG_EVENTS_FILE, LOG_ERRORS_FILE);
	LIBXML_TEST_VERSION
	
	// Create the process
	process_init("xap-hub", pid_filepath);
	
	while(process_state == PROC_RUNNING) {
	
		// Load xml file with general settings
		if (parseXmlSettings(SETTINGS_FILE) > 0) {
			syslog(LOG_ERR, "main: Failed to parse xml settings document, default values loaded");
			logError("main: Failed to parse xml settings document, default values loaded");
			if (saveXmlSettings(SETTINGS_FILE) > 0) {
				syslog(LOG_ERR, "main: Error saving settings file");
				logError("main: Error saving settings file");
			}
		}

		// Use the server socket to get interface properties
		server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (server_sockfd == -1) {
			syslog(LOG_ERR, "main: Error trying to get interface properties");
			logError("main: Error trying to get interface properties");
			unlink(pid_filepath);
			exit(EXIT_FAILURE);
		}

		// Set options for the socket
		optval=1;
		optlen=sizeof(int);
		if (setsockopt(server_sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&optval, optlen)) {
			syslog(LOG_ERR, "main: Error trying to get interface properties");
			logError("main: Error trying to get interface properties");
			unlink(pid_filepath);
			exit(EXIT_FAILURE);
		}
		
		optval=1;
		optlen=sizeof(int);
		if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, optlen)) {
			syslog(LOG_ERR, "main: Error trying to get interface properties");
			logError("main: Error trying to get interface properties");
			unlink(pid_filepath);
			exit(EXIT_FAILURE);
		}

		// Query the low-level capabilities of the network interface to get address and netmask
		memset((char*)&interface, sizeof(interface),0);
		strcpy(interface.ifr_name, hubConfig->interfacename);
		
		// Get the interface address
		interface.ifr_addr.sa_family = AF_INET; 
		if (ioctl(server_sockfd, SIOCGIFADDR, &interface) != 0) {
			syslog(LOG_ERR, "main: Could not determine IP address for interface %s", hubConfig->interfacename);
			logError("main: Could not determine IP address for interface %s", hubConfig->interfacename);
			unlink(pid_filepath);
			exit(EXIT_FAILURE);
		}
		myinterface.sin_addr.s_addr = ((struct sockaddr_in*)&interface.ifr_broadaddr)->sin_addr.s_addr;
		//printf("%s: address %s\n", interface.ifr_name, inet_ntoa(((struct sockaddr_in*)&interface.ifr_addr)->sin_addr));
		logEvent(TRUE, "main: %s: address %s", interface.ifr_name, inet_ntoa(((struct sockaddr_in*)&interface.ifr_addr)->sin_addr));
		
		// Get the interface netmask
		interface.ifr_broadaddr.sa_family = AF_INET; 
		if (ioctl(server_sockfd, SIOCGIFNETMASK, &interface) != 0) {
			syslog(LOG_ERR, "Unable to determine netmask for interface %s", hubConfig->interfacename);
			logError("main: Unable to determine netmask for interface %s", hubConfig->interfacename);
			unlink(pid_filepath);
			exit(EXIT_FAILURE);
		}
		mynetmask.sin_addr.s_addr = ((struct sockaddr_in*)&interface.ifr_broadaddr)->sin_addr.s_addr;
		//printf("%s: netmask %s\n", interface.ifr_name, inet_ntoa(((struct sockaddr_in*)&interface.ifr_netmask)->sin_addr));
		logEvent(TRUE, "main: %s: netmask %s", interface.ifr_name, inet_ntoa(((struct sockaddr_in*)&interface.ifr_netmask)->sin_addr));
		
		// Determine the interface broadcast address 
		long int inverted_netmask;
		inverted_netmask=~mynetmask.sin_addr.s_addr;
		mybroadcast.sin_addr.s_addr = inverted_netmask | myinterface.sin_addr.s_addr;
		//printf("%s: broadcast %s\n", interface.ifr_name, inet_ntoa(mybroadcast.sin_addr));
		logEvent(TRUE, "main: %s: broadcast %s", interface.ifr_name, inet_ntoa(mybroadcast.sin_addr));

		// Set the server socket
		server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		
		// Set server address and port
		memset((char *) &server_address, 0, sizeof(server_address));
		server_address.sin_family = AF_INET; 	
		server_address.sin_addr.s_addr = htonl(INADDR_ANY);		// Receive from any address
		server_address.sin_port = htons(hubConfig->xap_port);	// on this port (Default 3639)
		
		// Bind the server socket with the server IP address and port
		fcntl(server_sockfd, F_SETFL, O_NONBLOCK);
		if (bind(server_sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) !=0 ) {
			// if fails then we can assume that a hub is active on this host
			syslog(LOG_ERR, "main: Port %d is in use", hubConfig->xap_port);
			syslog(LOG_ERR, "main: Assuming other local hub is active on this host");
			logError("main: Port %d is in use", hubConfig->xap_port);
			logError("main: Assuming other local hub is active on this host");
			unlink(pid_filepath);
			exit(EXIT_FAILURE);
		}
		//printf("Listening for messages on port %d\n", g_xap_port);
		logEvent(TRUE, "main: Listening for messages on port %d", hubConfig->xap_port);

		// Set the server socket to listen
		listen(server_sockfd, MAX_QUEUE_BACKLOG);
		
		// Set up the Tx socket
		tx_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

		// Set up the heartbeat socket, on which we tell the world we are alive and well
		heartbeat_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (heartbeat_sockfd == -1) {
			syslog(LOG_ERR, "main: Heartbeat socket cannot be created");
			logError("main: Heartbeat socket cannot be created");
			unlink(pid_filepath);
			exit(EXIT_FAILURE);
		}
		
		// Set options for the heartbeat socket
		optval = 1;
		optlen = sizeof(int);
		if (setsockopt(heartbeat_sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&optval, optlen)) {
			syslog(LOG_ERR, "main: Unable to set heartbeat socket options");
			logError("main: Unable to set heartbeat socket options");
			unlink(pid_filepath);
			exit(EXIT_FAILURE);
		}
		
		// Set up heartbeat address and port
		memset((char *) &heartbeat_addr, 0, sizeof(heartbeat_addr));
		heartbeat_addr.sin_family = AF_INET;
		heartbeat_addr.sin_port = htons(hubConfig->xap_port);
		heartbeat_addr.sin_addr.s_addr = mybroadcast.sin_addr.s_addr;
		//printf("Set heartbeat broadcast on %s:%d\n", inet_ntoa(heartbeat_addr.sin_addr), g_xap_port);
		logEvent(TRUE, "main: Set heartbeat broadcast on %s:%d", inet_ntoa(heartbeat_addr.sin_addr), hubConfig->xap_port);

		xaptick = time((time_t*)0);
		heartbeattick = time((time_t*)0); // force heartbeat on startup
		//printf("Running...\n");
		logEvent(TRUE, "main: Running...");

		// Parse heartbeat messages received on broadcast interface
		// If they originated from this host, add the port number to the list of known ports
		// Otherwise ignore.
		// If ordinary header then pass to all known listeners
		while (!flgRestart && (process_state == PROC_RUNNING)) {

			// Get current time
			timenow = time((time_t*)0);
			
			// Hub tick, check for alive devices
			if (timenow - xaptick >= 1) {
				xaphub_tick(timenow - xaptick);
				xaptick = timenow;
			}
			
			// Heartbeat tick
			if (timenow >= heartbeattick) {
				//printf("Outgoing heartbeat tick %d\n",(int)timenow);
				logEvent(TRUE, "main: Outgoing heartbeat tick %d",(int)timenow);
				
				// Create the heartbeat message
				xaphub_build_heartbeat(heartbeat_msg);
				//printf("%s", heartbeat_msg);

				// Send heartbeat to all external listeners
				sendto(heartbeat_sockfd, heartbeat_msg, strlen(heartbeat_msg), 0, (struct sockaddr *) &heartbeat_addr, sizeof(heartbeat_addr));

				// Send heartbeat to all locally connected apps
				xaphub_relay(tx_sockfd, heartbeat_msg);
				
				// Set next tick
				heartbeattick = timenow + hubConfig->xap_hbeat;
			}
			
			// Prepare to attent to the clients
			FD_ZERO(&rdfs);
			FD_SET(server_sockfd, &rdfs);
			tv.tv_sec = hubConfig->xap_hbeat;
			tv.tv_usec = 0;
			select(server_sockfd + 1, &rdfs, NULL, NULL, &tv);
			
			// Select either timed out, or there was data - go look for it.
			client_len = sizeof(struct sockaddr);
			i = recvfrom(server_sockfd, buff, sizeof(buff), 0, (struct sockaddr*) &client_address, &client_len);

			// Check if a message was received
			if (i != -1) {
				buff[i]='\0';	// Add NULL to the end of message
			
				//printf("Message from client %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
				logEvent(TRUE, "main: Message from client %s:%d", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
				
				// Message from my interface
				if (client_address.sin_addr.s_addr == myinterface.sin_addr.s_addr) {
					//printf("Message originated from my interface\n");

					// If the message received is a heartbeat message, add the client to the relay list
					xap_handler(buff);
						
					// Relay the message to all local apps, the originator will see his own message
					xaphub_relay(tx_sockfd, buff);
				}
				// Message from local client received
				else if (client_address.sin_addr.s_addr == inet_addr("127.0.0.1")) {
					//printf("Message from local client\n");
				}
				// Remote message
				else {
					//printf("Message originated remotely, relay\n");

					// Relay the message to all local apps
					xaphub_relay(tx_sockfd, buff);
				}
				
				// Clear message
				memset(buff, 0, sizeof(buff));
			}
			
			// Check if has to save settings
			if (hubConfig->saveFlag) {
				hubConfig->saveFlag = FALSE;				// Reset flag
				if (saveXmlSettings(SETTINGS_FILE) > 0) {
					syslog(LOG_ERR, "main: Error saving settings file");
					logError("main: Error saving settings file");
				}
			}
			
			// Check if has to restart
			if (hubConfig->restartFlag) {
				hubConfig->restartFlag = FALSE;				// Reset flag
				flgRestart = TRUE;
			}
		}
		
		// Restore flgRestart
		flgRestart = FALSE;
		
		// Save xml settings
		if (saveXmlSettings(SETTINGS_FILE) > 0) {
			syslog(LOG_ERR, "main: Error saving settings file");
			logError("main: Error saving settings file");
		}
			
		// Build a xAP shutdown message
		xaphub_build_heartbeat_shutdown(heartbeat_msg);
		
		// Send shutdown heartbeat message to all external listeners
		sendto(heartbeat_sockfd, heartbeat_msg, strlen(heartbeat_msg), 0, (struct sockaddr *) &heartbeat_addr, sizeof(heartbeat_addr));
		
		// Send shutdown heartbeat message to all locally connected apps
		xaphub_relay(tx_sockfd, heartbeat_msg);

		// Close xAP communications
		close(server_sockfd);					// Close Server socket
		close(heartbeat_sockfd);				// Close Heartbeat socket
		close(tx_sockfd);						// Close Tx socket
	}
	
	// Close shared memory areas
	hubSharedMemClose();
		
	// Destroy the process
	process_finish();
	
	return 0;
}
