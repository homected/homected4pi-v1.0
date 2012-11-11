
//*************************************************************************
// 								DEFINE SECTION
//*************************************************************************

#define UPDATE_LIST_SIZE			1024				// Number of items in update list


//*************************************************************************
// 								TYPEDEFS SECTION
//*************************************************************************

// Datastream update item
typedef struct {								
	BYTE sent;									// TRUE if already sent
	unsigned int ds;							// Datastream id (from config)
	time_t timestamp;							// Time of the event
	char curr_value[COSM_MSGQUEUE_VALUE_LEN];	// Current value
} t_DS_UPDATE_ITEM;


//*************************************************************************
// 								GLOBAL VARIABLES
//				Accessible from anywhere within the application
//*************************************************************************

t_DS_UPDATE_ITEM g_update_list[UPDATE_LIST_SIZE];	// Update list queue
unsigned int g_update_list_count;					// Number of entries in update list
