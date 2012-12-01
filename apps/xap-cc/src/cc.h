#include "libxap/libxap.h"
#include "liblog/liblog.h"
#include <termios.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <time.h>

//*************************************************************************
// 								DEFINE SECTION
//*************************************************************************

//#define DEBUG_LIBXAP									// Set this to debug xAP library

#define XAP_VENDOR					"Homected"			// Default VENDOR
#define XAP_DEVICE					"xAP-CurrentCost"	// Default DEVICE
#define XAP_INSTANCE				"EnergyMonitor"		// Default INSTANCE
#define XAP_UID						0x8003				// Default UID

#define XML_ENCODING 				"ISO-8859-1"		// Xml encoding
#ifdef DEBUG_APP
#define SETTINGS_FILE				"xap-cc.xml"		// Xml settings file
#define LOG_EVENTS_FILE				"xap-cc_logevn.txt"	// Events log file
#define LOG_ERRORS_FILE				"xap-cc_logerr.txt"	// Errors log file
#else
#define SETTINGS_FILE				"/etc/homected4pi/xap-cc.xml"			// Xml settings file
#define LOG_EVENTS_FILE				"/var/homected4pi/xap-cc_logevn.txt"	// Events log file
#define LOG_ERRORS_FILE				"/var/homected4pi/xap-cc_logerr.txt"	// Errors log file
#endif

// xAP Web service
#define WSERV_NAME					"xAP-CurrentCost"						// Web Service name
#define WSERV_DESC					"Homected xAP Current Cost connector"	// Web Service description
#define WSERV_ICON					"/connectors/xap-cc/images/wserv.png"	// Web service icon

#define SENSOR_TIMEOUT				600					// Sensor timeout to treat as active (in seconds)

#define SETTINGS_TAG_NONE			0					// Type of tag read from Settings xml file
#define SETTINGS_TAG_DEVICE			1					// 	Device Tags
#define SETTINGS_TAG_DEV_NAME		2
#define SETTINGS_TAG_DEV_ENABLED	3
#define SETTINGS_TAG_DEV_CCMODEL	4
#define SETTINGS_TAG_DEV_CCSERIAL	5
#define SETTINGS_TAG_DEV_TEMPHYST	6
#define SETTINGS_TAG_DEV_TEMPLOC	7
#define SETTINGS_TAG_DEV_TEMPNAME	8
#define SETTINGS_TAG_DEV_TEMPUNITS	9
#define SETTINGS_TAG_DEV_TEMPDESC	10
#define SETTINGS_TAG_XAP			11					// 	Xap Tags
#define SETTINGS_TAG_XAP_IFACE		12
#define SETTINGS_TAG_XAP_VERSION	13
#define SETTINGS_TAG_XAP_HBEATF		14
#define SETTINGS_TAG_XAP_WSERVF		15
#define SETTINGS_TAG_XAP_INFOF		16
#define SETTINGS_TAG_XAP_UID		17
#define SETTINGS_TAG_XAP_PORT		18
#define SETTINGS_TAG_SENSORS		19					// 	Sensors Tags
#define SETTINGS_TAG_SENSOR			20
#define SETTINGS_TAG_S_ENABLED		21
#define SETTINGS_TAG_S_DIGITAL		22
#define SETTINGS_TAG_S_HYST			23
#define SETTINGS_TAG_S_LOC			24
#define SETTINGS_TAG_S_NAME			25
#define SETTINGS_TAG_S_UNITS		26
#define SETTINGS_TAG_S_DESC			27

#define CC_TAG_NONE					0					// Type of tag read from CurrentCost xml
#define CC_TAG_MSG					1
#define CC_TAG_SRC					2
#define CC_TAG_DSB					3
#define CC_TAG_TIME					4
#define CC_TAG_CURR_TMPR			5
#define CC_TAG_CURR_SENSOR			6
#define CC_TAG_CURR_ID				7
#define CC_TAG_CURR_TYPE			8
#define CC_TAG_CURR_CH				9
#define CC_TAG_CURR_CH_WATTS		10
#define CC_TAG_CURR_IMP				11
#define CC_TAG_CURR_IPU				12
#define CC_TAG_HIST					13

#define CC_TEMPMON					-1					// Index of sensor for temp monitor
#define CC_STYPE_DISABLED			0					// Type of sensor: Disabled (No broadcast data)
#define CC_STYPE_ELECTRIC			1					// Type of sensor: Electric
#define CC_STYPE_IMPULSE			2					// Type of sensor: Impulse
#define CC_STYPE_GAS				3					// Type of sensor: Gas
#define CC_STYPE_WATER				4					// Type of sensor: Water
#define CC_CHANNEL_NONE				0					// Const for channels
#define CC_CHANNEL_1				1
#define CC_CHANNEL_2				2
#define CC_CHANNEL_3				3
#define CC_CHANNEL_TOTAL			4

//*************************************************************************
// 								TYPEDEFS SECTION
//*************************************************************************

//*************************************************************************
// 								GLOBAL VARIABLES
//				Accessible from anywhere within the application
//*************************************************************************

unsigned char g_cc_tag;						// CurrentCost xml tag
unsigned char g_cc_tag_hist_msg;			// CurrentCost xml tag from hist message
int g_curr_sensor;							// Current sensor (0..CC_SENSORS-1)
int g_curr_channel;							// Current sensor channel (ch1, ch2 or ch3)

unsigned char g_settings_tag;				// Settings xml tag

USHORT g_xap_rxport = XAP_PORT;				// xAP Port (Receive)
