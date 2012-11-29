#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include "liblog/liblog.h"
#include "libprocess/libprocess.h"

//*************************************************************************
// 								DEFINE SECTION
//*************************************************************************

#define FALSE					0
#define TRUE					1

#define XML_ENCODING 			"ISO-8859-1"							// Xml encoding
#define XML_VALUE_LEN			30										// Xml value max length
#define SETTINGS_FILE			"/home/homected4pi/config/xap-hub.xml"		// Xml settings file
#define LOG_EVENTS_FILE			"/home/homected4pi/logs/xap-hub_logevn.txt"// Events log file
#define LOG_ERRORS_FILE			"/home/homected4pi/logs/xap-hub_logerr.txt"// Errors log file

#define XAP_VENDOR				"Homected"		// Default VENDOR
#define XAP_DEVICE				"xAP-Hub"		// Default DEVICE
#define XAP_INSTANCE			"myHub"			// Default INSTANCE
#define XAP_UID					"FF8001"		// Default UID
#define XAP_PORT				3639			// Default Port
#define XAP_HBEAT	 			60				// Default Heartbeat interval in secs

#define XAP_MAX_HUB_ENTRIES		50

#define XAP_MAX_MSG_ELEMENTS 	1000			// Consts for message parser
#define XAP_MAX_KEYNAME_LEN 	128
#define XAP_MAX_KEYVALUE_LEN 	1500
#define XAP_MAX_SECTION_LEN 	128

// Xml settings tags
#define SETTINGS_TAG_NONE			0			// Type of tag read from Settings xml file
#define SETTINGS_TAG_XAP			1			// 	Xap Tags
#define SETTINGS_TAG_XAP_IFACE		2
#define SETTINGS_TAG_XAP_INSTANCE	3
#define SETTINGS_TAG_XAP_HBEATF		4
#define SETTINGS_TAG_XAP_UID		5
#define SETTINGS_TAG_XAP_PORT		6

#define MAX_QUEUE_BACKLOG 		5  			// number of connections that can queue (max. 5)

//*************************************************************************
// 								TYPEDEFS SECTION
//*************************************************************************

struct tg_xap_hubentry {
	int port; 								// ip-port to forward to
	int interval;
	int timer;
	int is_alive;
};

struct tg_xap_msg {							// Message elements struct
	char section[XAP_MAX_SECTION_LEN+1];
	char name[XAP_MAX_KEYNAME_LEN+1];
	char value[XAP_MAX_KEYVALUE_LEN+1];
};

//*************************************************************************
// 								GLOBAL VARIABLES
//				Accessible from anywhere within the application
//*************************************************************************

unsigned char g_settings_tag;						// Settings xml tag
struct tg_xap_msg g_xap_msg[XAP_MAX_MSG_ELEMENTS];	// xAP message elements
int g_xap_index;									// Index for xAP message elements array

struct tg_xap_hubentry g_xap_hubentry[XAP_MAX_HUB_ENTRIES];	// xAP-hub entry list
