#include "libxap/libxap.h"
#include "liblog/liblog.h"
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <sqlite3.h>
#include <time.h>

//*************************************************************************
// 								DEFINE SECTION
//*************************************************************************

//#define DEBUG_LIBXAP									// Set this to debug xAP library

#define XAP_VENDOR					"Homected"			// Default VENDOR
#define XAP_DEVICE					"xAP-SQLite"		// Default DEVICE
#define XAP_INSTANCE				"Sqlite"			// Default INSTANCE
#define XAP_UID						0x8005				// Default UID

#define XML_ENCODING 				"ISO-8859-1"		// Xml encoding
#ifdef DEBUG_APP
#define SETTINGS_FILE				"xap-sqlite.xml"		// Xml settings file
#define LOG_EVENTS_FILE				"xap-sqlite_logevn.txt"	// Events log file
#define LOG_ERRORS_FILE				"xap-sqlite_logerr.txt"	// Errors log file
#else
#define SETTINGS_FILE				"/home/homected/config/xap-sqlite.xml"		// Xml settings file
#define LOG_EVENTS_FILE				"/home/homected/logs/xap-sqlite_logevn.txt"	// Events log file
#define LOG_ERRORS_FILE				"/home/homected/logs/xap-sqlite_logerr.txt"	// Errors log file
#endif

// xAP Web service
#define WSERV_NAME					"xAP-SQLite"								// Web Service name
#define WSERV_DESC					"Homected xAP SQLite connector"				// Web Service description
#define WSERV_ICON					"/connectors/xap-sqlite/images/wserv.png"	// Web service icon

// Settings tag's
#define SETTINGS_TAG_NONE			0					// Type of tag read from Settings xml file
#define SETTINGS_TAG_DEVICE			1					// 	Device Tags
#define SETTINGS_TAG_DEV_NAME		2
#define SETTINGS_TAG_DEV_ENABLED	3
#define SETTINGS_TAG_XAP			4					// 	Xap Tags
#define SETTINGS_TAG_XAP_IFACE		5
#define SETTINGS_TAG_XAP_VERSION	6
#define SETTINGS_TAG_XAP_HBEATF		7
#define SETTINGS_TAG_XAP_WSERVF		8
#define SETTINGS_TAG_XAP_UID		9
#define SETTINGS_TAG_XAP_PORT		10
#define SETTINGS_TAG_DATASTREAMS	11					// 	Datastreams Tags
#define SETTINGS_TAG_DATASTREAM		12
#define SETTINGS_TAG_DS_ENABLED		13
#define SETTINGS_TAG_DS_ID			14
#define SETTINGS_TAG_DS_DB_FILE		15
#define SETTINGS_TAG_DS_DB_TABLE	16
#define SETTINGS_TAG_DS_DB_FIELD	17
#define SETTINGS_TAG_DS_DB_FTYPE	18
#define SETTINGS_TAG_DS_XAPSOURCE	19
#define SETTINGS_TAG_DS_XAPCLASS	20
#define SETTINGS_TAG_DS_XAPSECTION	21
#define SETTINGS_TAG_DS_XAPKEY		22

//*************************************************************************
// 								TYPEDEFS SECTION
//*************************************************************************


//*************************************************************************
// 								GLOBAL VARIABLES
//				Accessible from anywhere within the application
//*************************************************************************

unsigned char g_settings_tag;				// Settings xml tag
unsigned int g_curr_datastream;				// Current datastream index

USHORT g_xap_rxport = XAP_PORT;				// xAP Port (Receive)

