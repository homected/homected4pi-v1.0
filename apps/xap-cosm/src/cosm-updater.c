/**************************************************************************

	cosm-updater.c

	Cosm data updater

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

	25/10/12 by Jose Luis Galindo : Initial release
	
	Based on HAH livebox pachube connector from www.DBzoo.com
	Copyright (c) Brett England, 2009
	 

***************************************************************************/

//#define DEBUG_APP						// Set this to debug to console
#include "shared.h"
#include "cosm.h"
#include "cosm-updater.h"
#include "libprocess/libprocess.h"


//*************************************************************************
//								APPLICATION FUNCTIONS 
//*************************************************************************

// Purge update list
void purge_update_list() {
	
	int i;
	for (i = 0; i < UPDATE_LIST_SIZE; i++) {
		g_update_list[i].sent = FALSE;
		g_update_list[i].ds = 0;
		g_update_list[i].timestamp = 0;
		strcpy(g_update_list[i].curr_value, "");
	}
	g_update_list_count = 0;
}

// Send update list to cosm
void send_list_to_cosm() {
	
	int i, j;
	char xmldata[COSM_MSG_LEN];
	char timestamp[21];					// Format: YYYY-MM-DDTHH:MM:SSZ
	char last_value[COSM_MSGQUEUE_VALUE_LEN];
	struct tm *ptm;
	int ret_code;
	int retry = 1;

	for(i = 0; i < g_update_list_count; i++) {
		if(g_update_list[i].sent == FALSE) {
			
			// Create xml data header
			ptm = gmtime(&g_update_list[i].timestamp);
			sprintf(timestamp, "%4d-%02d-%02dT%02d:%02d:%02dZ", 1900 + (ptm->tm_year), (ptm->tm_mon) + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
			sprintf(xmldata, "<eeml xmlns=\"http://www.eeml.org/xsd/0.5.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version=\"0.5.1\" xsi:schemaLocation=\"http://www.eeml.org/xsd/0.5.1 http://www.eeml.org/xsd/0.5.1/0.5.1.xsd\"><environment><data><datapoints><value at=\"%s\">%s</value>", timestamp, g_update_list[i].curr_value);
			strcpy(last_value, g_update_list[i].curr_value);

			// Search for more updates for this datastream
			for(j = i + 1; j < g_update_list_count; j++) {
				// If same datastream and not sent -> add datapoint
				if ((g_update_list[j].ds == g_update_list[i].ds) && (g_update_list[j].sent == FALSE)) {
					// add datapoint only if value changes (to reduce uploads)
					//if (strcmp(last_value, g_update_list[j].curr_value) != 0) {
						ptm = gmtime(&g_update_list[j].timestamp);
						sprintf(timestamp, "%4d-%02d-%02dT%02d:%02d:%02dZ", 1900 + (ptm->tm_year), (ptm->tm_mon) + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
						sprintf(xmldata, "%s<value at=\"%s\">%s</value>", xmldata, timestamp, g_update_list[j].curr_value);
					//}
					strcpy(last_value, g_update_list[j].curr_value);
					g_update_list[j].sent = TRUE;
				}
			}
			
			// End the xml data
			sprintf(xmldata, "%s</datapoints></data></environment></eeml>", xmldata);
			
			// Sent data
			retry = 1;
			while(retry <= 3) {
				ret_code = create_datapoint_xml(cosmConfig->apikey, cosmConfig->dstreams[g_update_list[i].ds].feed, cosmConfig->dstreams[g_update_list[i].ds].id, xmldata);
				if (ret_code != COSM_NO_ERROR) {
					#ifdef DEBUG_APP
					printf("(Trying %d) Fail %d at creating datapoint in Cosm.\nXml data:\n%s\n", retry, ret_code, xmldata);
					#endif
					logError("send_list_to_cosm: (Trying %d) Fail %d at creating datapoint in Cosm.\nXml data:\n%s\n", retry, ret_code, xmldata);
					retry++;
				}
				else
					retry = 4;
			}
		}
	}
}


//*************************************************************************
//								MAIN PROGRAM 
//*************************************************************************

int main(void) {
	
	BYTE flgRestart = FALSE;					// if TRUE, restart main loop
	
	time_t timenow;								// Current time
	time_t cosmtick;							// Time for next upload
	time_t newvalue;
	
	t_UPDATER_MSG updater_msg;					// Messsage for updater
	
	// Header verbage
	#ifdef DEBUG_APP
	printf("\nHomected xAP-Cosm Connector (Updater)\n");
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
	purge_update_list();
	logInit(LOG_EVENTS_FILE, LOG_ERRORS_FILE);
	LIBXML_TEST_VERSION
	
	// Connect to the message queue to communicate with xap-cosm
	if ((g_msg_queue = msgget(COSM_MSGQUEUE, 0644)) == -1) {
		#ifdef DEBUG_APP
		printf("Error connecting to message queue\n");
		#endif
		logError("main: Error connecting to message queue");
		unlink(pid_filepath);
		exit(EXIT_FAILURE);
	}

	// Create the process
	#ifndef DEBUG_APP
	process_init("xap-cosm-updater", pid_filepath);
	#endif
	
	while(process_state == PROC_RUNNING) {
	
		#ifdef DEBUG_APP
		printf("Running...\n");
		#endif
		logEvent(TRUE, "updater: Running...");
	
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

			// Wait for an incomining message
			if (msgrcv(g_msg_queue, (struct msgbuf *)&updater_msg, sizeof(updater_msg.ds)+sizeof(updater_msg.timestamp)+sizeof(updater_msg.curr_value), COSM_MSGQUEUE_MSG_ID,  IPC_NOWAIT) != -1) {
				// Add message data to update list
				g_update_list[g_update_list_count].sent = FALSE;
				g_update_list[g_update_list_count].ds = updater_msg.ds;
				g_update_list[g_update_list_count].timestamp = updater_msg.timestamp;
				strcpy(g_update_list[g_update_list_count].curr_value, updater_msg.curr_value);
				#ifdef DEBUG_APP
				printf("ds:%d added value %s to Cosm update list queue (pos=%d)\n", g_update_list[g_update_list_count].ds, g_update_list[g_update_list_count].curr_value, g_update_list_count);
				#endif
				logEvent(TRUE, "updater: (ds:%d) added value %s to Cosm update list queue (pos=%d)", g_update_list[g_update_list_count].ds, g_update_list[g_update_list_count].curr_value, g_update_list_count);
				g_update_list_count++;
			}
			usleep(10000); //convert to microseconds
		
			// Cosm tick
			if (timenow >= cosmtick) {
				#ifdef DEBUG_APP
				printf("Outgoing cosm tick %d\n",(int)timenow);
				#endif
				logEvent(TRUE, "updater: Outgoing cosm tick %d",(int)timenow);
				
				// Upload update list to cosm
				send_list_to_cosm();
				
				// Clear update list
				purge_update_list();
				
				// Set next tick
				cosmtick = timenow + cosmConfig->updatef;
			}
			
			// Check if has to restart
			if (cosmConfig->restartFlag) {
				cosmConfig->restartFlag = FALSE;			// Reset flag
				flgRestart = TRUE;
			}
		}
		
		// Restore flgRestart
		flgRestart = FALSE;
		
	}
	
	// Close shared memory areas
	cosmSharedMemClose();
	
	// Destroy the process
	#ifndef DEBUG_APP
	process_finish();
	#endif
	
	return 0;
}
