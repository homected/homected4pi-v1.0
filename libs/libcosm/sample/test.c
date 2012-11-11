#include <stdio.h>
#include <unistd.h>
#include "../libcosm.h"

int main(void)
{
	char api_key[COSM_APIKEY_LEN];
	unsigned int environment_id;
	char data[1500];
	char datastream[10];

	strcpy(data, "");

	/* Edit it*/
	environment_id = 59210; 												// your-feed
	sprintf(api_key,"%s","JB25QT5LCDfFSed6gV1L-PomCwLb4_phFiCaZnxZziQ");	// your API key
	sprintf(datastream,"%s","1");											// your datastream

/*	
	// Create an environment
	if (create_environment(api_key, "test", &environment_id) == COSM_NO_ERROR)
		printf("OK Created feed: %d\n", environment_id);
	else
		printf("FAIL\n");
*/

/*
	// Get the environment xml data
	if (get_environment_xml(api_key, environment_id, data) == COSM_NO_ERROR)
		printf("OK environment data:\n%s\n", data);
	else
		printf("FAIL\n");
*/

/*	
	// Get the environment name
	if (get_environment(api_key, environment_id, data) == COSM_NO_ERROR)
		printf("OK environment: %s\n", data);
	else
		printf("FAIL\n");
*/

/*	
	// Update the environment name
	if (update_environment(api_key, environment_id, "test2") == COSM_NO_ERROR)
		printf("OK\n");
	else
		printf("FAIL\n");
*/

/*	
	// Delete an environment
	if (delete_environment(api_key, environment_id) == COSM_NO_ERROR)
		printf("OK\n");
	else
		printf("FAIL\n");
*/

/*	
	// Create a datastream
	if (create_datastream(api_key, environment_id, datastream, "16") == COSM_NO_ERROR)
		printf("OK\n");
	else
		printf("FAIL\n");
*/

/*
	// Get the datastream xml data
	if (get_datastream_xml(api_key, environment_id, datastream, data) == COSM_NO_ERROR)
		printf("OK datastream data:\n%s\n", data);
	else
		printf("FAIL\n");
*/

/*
	// Update the datastream
	if (update_datastream(api_key, environment_id, datastream, "20") == COSM_NO_ERROR)
		printf("OK\n");
	else
		printf("FAIL\n");
*/

/*	
	// Delete a datastream
	if (delete_datastream(api_key, environment_id, datastream) == COSM_NO_ERROR)
		printf("OK\n");
	else
		printf("FAIL\n");
*/

/*
	if (create_datapoint(api_key, environment_id, datastream, "2012-05-12T11:03:43Z", "32") == COSM_NO_ERROR)
		printf("OK\n");
	else
		printf("FAIL\n");
*/

/*
	if (get_datapoint_xml(api_key, environment_id, datastream, "2012-05-12T11:03:43Z", data) == COSM_NO_ERROR)
		printf("OK datapoint data:\n%s\n", data);
	else
		printf("FAIL\n");
*/

/*
	if (update_datapoint(api_key, environment_id, datastream, "2012-05-12T11:03:43Z", "10") == COSM_NO_ERROR)
		printf("OK\n");
	else
		printf("FAIL\n");
*/

/*
	if (delete_datapoint(api_key, environment_id, datastream, "2012-05-12T11:03:43Z") == COSM_NO_ERROR)
		printf("OK\n");
	else
		printf("FAIL\n");
*/

	return 1;
}
