#include "libxap/libxap.h"
#include "liblog/liblog.h"
#include <termios.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <time.h>

//*************************************************************************
// 								DEFINE SECTION
//*************************************************************************

//#define DEBUG_LIBXAP									// Set this to debug xAP library

#define XAP_VENDOR					"Homected"			// Default VENDOR
#define XAP_DEVICE					"xAP-Rfxcom"		// Default DEVICE
#define XAP_INSTANCE				"Rfxcom"			// Default INSTANCE
#define XAP_UID						0x8008				// Default UID

#define XML_ENCODING 				"ISO-8859-1"		// Xml encoding
#ifdef DEBUG_APP
#define SETTINGS_FILE				"xap-rfxcom.xml"		// Xml settings file
#define LOG_EVENTS_FILE				"xap-rfxcom_logevn.txt"	// Events log file
#define LOG_ERRORS_FILE				"xap-rfxcom_logerr.txt"	// Errors log file
#else
#define SETTINGS_FILE				"/etc/homected4pi/xap-rfxcom.xml"			// Xml settings file
#define LOG_EVENTS_FILE				"/var/homected4pi/xap-rfxcom_logevn.txt"	// Events log file
#define LOG_ERRORS_FILE				"/var/homected4pi/xap-rfxcom_logerr.txt"	// Errors log file
#endif

// xAP Web service
#define WSERV_NAME					"xAP-Rfxcom"								// Web Service name
#define WSERV_DESC					"Homected xAP Rfxcom connector"				// Web Service description
#define WSERV_ICON					"/connectors/xap-rfxcom/images/wserv.png"	// Web service icon

// Settings tag's
#define SETTINGS_TAG_NONE			0					// Type of tag read from Settings xml file
#define SETTINGS_TAG_DEVICE			1					// 	Device Tags
#define SETTINGS_TAG_DEV_NAME		2
#define SETTINGS_TAG_DEV_ENABLED	3
#define SETTINGS_TAG_DEV_COMMPORT	4
#define SETTINGS_TAG_DEV_X10LOC		5
#define SETTINGS_TAG_DEV_X10NAME	6
#define SETTINGS_TAG_DEV_DISX10		7
#define SETTINGS_TAG_DEV_DISOREGON	8
#define SETTINGS_TAG_DEV_DISARC		9
#define SETTINGS_TAG_DEV_DISVISONIC	10
#define SETTINGS_TAG_DEV_DISATI		11
#define SETTINGS_TAG_DEV_DISKOPPLA	12
#define SETTINGS_TAG_DEV_DISHEASYUK	13
#define SETTINGS_TAG_DEV_DISHEASYEU	14
#define SETTINGS_TAG_DEV_DISX10SBSC	15
#define SETTINGS_TAG_DEV_DISX10SX10	16
#define SETTINGS_TAG_DEV_DISX10MSGQ	17
#define SETTINGS_TAG_XAP			18					// 	Xap Tags
#define SETTINGS_TAG_XAP_IFACE		19
#define SETTINGS_TAG_XAP_VERSION	20
#define SETTINGS_TAG_XAP_HBEATF		21
#define SETTINGS_TAG_XAP_INFOF		22
#define SETTINGS_TAG_XAP_WSERVF		23
#define SETTINGS_TAG_XAP_UID		24
#define SETTINGS_TAG_XAP_PORT		25
#define SETTINGS_TAG_SENSORS		26					// 	Sensors Tags
#define SETTINGS_TAG_SENSOR			27					//  Generic Sensor Tags
#define SETTINGS_TAG_SENSOR_ENABLED	28
#define SETTINGS_TAG_SENSOR_LOC		29
#define SETTINGS_TAG_SENSOR_NAME	30
#define SETTINGS_TAG_SENSOR_X10CAPS	31					// devcaps tag for X10RF
#define SETTINGS_TAG_SENSOR_SECCAPS	32					// devcaps tag for SECURITY
#define SETTINGS_TAG_SENSOR_ORECAPS	33					// devcaps tag for OREGON
#define SETTINGS_TAG_SENSOR_ORECFG	34					// devconfig tag for OREGON Sensor
#define SETTINGS_TAG_SENSOR_RFXCAPS	35					// devcaps tag for RFXCOM
#define SETTINGS_TAG_SENSOR_RFXCFG	36					// devconfig tag for RFXCOM Sensor

// RFXCOM modes
#define RFXCOM_MODE_HS				0x21
#define RFXCOM_MODE_KOPPLA			0x23
#define RFXCOM_MODE_ARC				0x24
#define RFXCOM_MODE_DISABLE_HEASYUK	0x28
#define RFXCOM_MODE_32BIT_LEN		0x29
#define RFXCOM_MODE_ENABLE_ALL		0x2A
#define RFXCOM_MODE_VAR_LEN			0x2C
#define RFXCOM_MODE_DISABLE_ARC		0x2D
#define RFXCOM_MODE_DISABLE_KOPPLA	0x2E
#define RFXCOM_MODE_DISABLE_X10		0x2F
#define RFXCOM_MODE_VISONIC			0x40
#define RFXCOM_MODE_VIS_AND_VAR_LEN	0x41
#define RFXCOM_MODE_VIS_CLEAR_AUX	0x42
#define RFXCOM_MODE_DISABLE_OREGON	0x43
#define RFXCOM_MODE_DISABLE_ATI		0x44
#define RFXCOM_MODE_DISABLE_VIS		0x45
#define RFXCOM_MODE_DISABLE_SOMFY	0x46
#define RFXCOM_MODE_DISABLE_HEASYEU	0x47


// RFXCOM Initialization errors
#define RFXCOM_ERROR_INIT			-1000
#define RFXCOM_ERROR_DISX10			-1001
#define RFXCOM_ERROR_DISOREGON		-1002
#define RFXCOM_ERROR_DISARC			-1003
#define RFXCOM_ERROR_DISVISONIC		-1004
#define RFXCOM_ERROR_DISATI			-1005
#define RFXCOM_ERROR_DISKOPPLA		-1006
#define RFXCOM_ERROR_DISHEASYUK		-1007
#define RFXCOM_ERROR_DISHEASYEU		-1008

// SENSOR TYPES
#define RFXCOM_SENSOR_UNKNOWN		0
#define RFXCOM_SENSOR_X10			1
#define RFXCOM_SENSOR_SECURITY		2
#define RFXCOM_SENSOR_OREGON		3
#define RFXCOM_SENSOR_HOMEEASY		4
#define RFXCOM_SENSOR_KOPPLA		5
#define RFXCOM_SENSOR_RFXCOM		6

// XAP-BSC CONSTANTS
#define RFXCOM_XAPBSC_OFF			0x00
#define RFXCOM_XAPBSC_ON			0x01
#define RFXCOM_XAPBSC_UNKNOWN		0xFF
#define RFXCOM_XAPBSC_ACTION_NONE	0x00
#define RFXCOM_XAPBSC_ACTION_VALUE	0x01
#define RFXCOM_XAPBSC_ACTION_LOW	0x02
#define RFXCOM_XAPBSC_ACTION_FULL	0x03
#define RFXCOM_XAPBSC_MIN_VALUE		0
#define RFXCOM_XAPBSC_MAX_VALUE		100
#define RFXCOM_XAPBSC_NUM_DIMS		32

//*************************************************************************
// 								TYPEDEFS SECTION
//*************************************************************************

typedef struct {					// X10RF device data for xAP-BSC schema
	unsigned int uidsub;			// Subaddress UID
	BYTE state;						// Current state
	BYTE value;						// Current value
	BYTE allowOnOff;				// Allow ON and OFF commands
	BYTE allowDimBright;			// Allow DIM and BRIGHT commands
	BYTE allowAllLightsOff;			// Allow ALL LIGHTS OFF command
	BYTE allowAllLightsOn;			// Allow ALL LIGHTS ON command
	BYTE hasMemory;					// Has memory of the value when goes from OFF to ON
	BYTE canOffFromDim;				// Can go to OFF state from DIM commands
	BYTE actionDimFromOFF;			// Action to perform when DIM from OFF
	BYTE actionBrightFromOFF;		// Action to perform when BRIGHT from OFF
} t_x10_device_XAPBSC;

typedef struct {					// SECURITY device data for xAP-BSC schema
	unsigned int uidsub;			// Subaddress UID
	BYTE state;						// Current state
	BYTE lastState;					// Last state
} t_sec_device_XAPBSC;

typedef struct {					// OREGON device data for xAP-BSC schema
	unsigned int uidsub;			// Subaddress UID
	BYTE status_unknown;			// Flag to indicate if the endpoint state is unknown
	float value1;					// Store the value1 (Temp, Rain rate, Wind dir, UV index, Weight)
	float lastvalue1;				// Store the last value1
	float value2;					// Store the value2 (Hum, Total rain, Wind speed)
	float lastvalue2;				// Store the last value2
	float value3;					// Store the value3 (Baro, Wind avg speed)
	float lastvalue3;				// Store the last value3
	BYTE battery;					// Store the battery status
	BYTE lastbattery;				// Store the last battery status
} t_ore_device_XAPBSC;

typedef struct {					// HomeEasy device data for xAP-BSC schema
	unsigned int uidsub;			// Subaddress UID
	BYTE status_unknown;			// Flag to indicate if the endpoint state is unknown
	BYTE state;						// Current state
	BYTE laststate;					// Last state
	BYTE level;						// Current level
	BYTE lastlevel;					// Last level
} t_he_device_XAPBSC;

typedef struct {					// Koppla device data for xAP-BSC schema
	unsigned int uidsub;			// Subaddress UID
	BYTE state;						// Current state
	BYTE laststate;					// Last state
} t_kop_device_XAPBSC;

typedef struct {					// RFXCOM device data for xAP-BSC schema
	unsigned int uidsub;			// Subaddress UID
	BYTE status_unknown;			// Flag to indicate if the endpoint state is unknown
	float lastfunc;					// Store the last function received
	float value1;					// Store the value1 (Temp)
	float lastvalue1;				// Store the last value1
	float value2;					// Store the value2 (Temp, AD voltage)
	float lastvalue2;				// Store the last value2
	float value3;					// Store the value3 (Supply voltage)
	float lastvalue3;				// Store the last value3
} t_rfx_device_XAPBSC;

//*************************************************************************
// 								GLOBAL VARIABLES
//				Accessible from anywhere within the application
//*************************************************************************

unsigned char g_settings_tag;				// Settings xml tag
unsigned char g_curr_sensor_type;			// Sensor type (X10RF, SECURITY, OREGON, ...)
unsigned int g_max_subuid;					// Max number of sub UID's
unsigned int g_subuid_counter;				// Counter for sub UID's
unsigned int g_xapx10_subuid;				// Sub UID for xAP-X10 messages
int x10RfMsgQueue;							// X10RF Message queue
BYTE last_hcode, last_ucode;				// Store the last housecode and unitcode for X10RF devices

t_x10_device_XAPBSC g_x10_devices_xapbsc[RFXCOM_X10_DEVICES];	// xAP-BSC config for X10RF devices
unsigned int g_next_x10_device;									// Counter for X10RF devices
t_sec_device_XAPBSC g_sec_devices_xapbsc[RFXCOM_SEC_DEVICES];	// xAP-BSC config for SECURITY devices
unsigned int g_next_sec_device;									// Counter for SECURITY devices
t_ore_device_XAPBSC g_ore_devices_xapbsc[RFXCOM_ORE_DEVICES];	// xAP-BSC config for OREGON devices
unsigned int g_next_ore_device;									// Counter for OREGON devices
t_he_device_XAPBSC g_he_devices_xapbsc[RFXCOM_HE_DEVICES];		// xAP-BSC config for HOMEEASY devices
unsigned int g_next_he_device;									// Counter for HOMEEASY devices
t_kop_device_XAPBSC g_kop_devices_xapbsc[RFXCOM_KOP_DEVICES];	// xAP-BSC config for KOPPLA devices
unsigned int g_next_kop_device;									// Counter for KOPPLA devices
t_rfx_device_XAPBSC g_rfx_devices_xapbsc[RFXCOM_RFX_DEVICES];	// xAP-BSC config for RFXCOM devices
unsigned int g_next_rfx_device;									// Counter for RFXCOM devices

USHORT g_xap_rxport = XAP_PORT;				// xAP Port (Receive)

