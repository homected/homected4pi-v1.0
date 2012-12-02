#include "libxap/libxap.h"
#include "liblog/liblog.h"
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>

//*************************************************************************
// 								DEFINE SECTION
//*************************************************************************

//#define DEBUG_LIBXAP									// Set this to debug xAP library

#define COSM_MSGQUEUE				8004				// Message queue ID
#define COSM_MSGQUEUE_MSG_ID		2					// Message ID
#define COSM_MSGQUEUE_VALUE_LEN		11					// Max. length for values

#define XAP_VENDOR					"Homected"			// Default VENDOR
#define XAP_DEVICE					"xAP-Cosm"			// Default DEVICE
#define XAP_INSTANCE				"Cosm"				// Default INSTANCE
#define XAP_UID						0x8004				// Default UID

#define XML_ENCODING 				"ISO-8859-1"		// Xml encoding
#ifdef DEBUG_APP
#define SETTINGS_FILE				"xap-cosm.xml"			// Xml settings file
#define LOG_EVENTS_FILE				"xap-cosm_logevn.txt"	// Events log file
#define LOG_ERRORS_FILE				"xap-cosm_logerr.txt"	// Errors log file
#else
#define SETTINGS_FILE				"/etc/homected4pi/xap-cosm.xml"			// Xml settings file
#define LOG_EVENTS_FILE				"/var/homected4pi/xap-cosm_logevn.txt"	// Events log file
#define LOG_ERRORS_FILE				"/var/homected4pi/xap-cosm_logerr.txt"	// Errors log file
#endif

// xAP Web service
#define WSERV_NAME					"xAP-Cosm"								// Web Service name
#define WSERV_DESC					"Homected xAP Cosm connector"			// Web Service description
#define WSERV_ICON					"/connectors/xap-cosm/images/wserv.png"	// Web service icon

// Settings tag's
#define SETTINGS_TAG_NONE			0					// Type of tag read from Settings xml file
#define SETTINGS_TAG_DEVICE			1					// 	Device Tags
#define SETTINGS_TAG_DEV_NAME		2
#define SETTINGS_TAG_DEV_ENABLED	3
#define SETTINGS_TAG_DEV_APIKEY		4
#define SETTINGS_TAG_DEV_UPDATEF	5
#define SETTINGS_TAG_XAP			6					// 	Xap Tags
#define SETTINGS_TAG_XAP_IFACE		7
#define SETTINGS_TAG_XAP_VERSION	8
#define SETTINGS_TAG_XAP_HBEATF		9
#define SETTINGS_TAG_XAP_WSERVF		10
#define SETTINGS_TAG_XAP_UID		11
#define SETTINGS_TAG_XAP_PORT		12
#define SETTINGS_TAG_DATASTREAMS	13					// 	Datastreams Tags
#define SETTINGS_TAG_DATASTREAM		14
#define SETTINGS_TAG_DS_ENABLED		15
#define SETTINGS_TAG_DS_FEED		16
#define SETTINGS_TAG_DS_ID			17
#define SETTINGS_TAG_DS_XAPSOURCE	18
#define SETTINGS_TAG_DS_XAPCLASS	19
#define SETTINGS_TAG_DS_XAPSECTION	20
#define SETTINGS_TAG_DS_XAPKEY		21


//*************************************************************************
// 								TYPEDEFS SECTION
//*************************************************************************

// Cosm updater message
typedef struct {
	long msg_id;
	unsigned int ds;							// Datastream id (from config)
	time_t timestamp;							// Time of the event
	char curr_value[COSM_MSGQUEUE_VALUE_LEN];	// Current value
} t_UPDATER_MSG;


//*************************************************************************
// 								GLOBAL VARIABLES
//				Accessible from anywhere within the application
//*************************************************************************

unsigned char g_settings_tag;				// Settings xml tag
unsigned int g_curr_datastream;				// Current datastream index

USHORT g_xap_rxport = XAP_PORT;				// xAP Port (Receive)

int g_msg_queue;							// Message queue
