/**************************************************************************

	libcosm.c

	Functions to interface to Cosm

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

	Library based on PACHULIB (https://sourceforge.net/projects/pachulib/)
	Original source code of pachulib included with this library.
	
	Last changes:

	24/07/12 by Jose Luis Galindo: Added return codes for functions
	05/05/12 by Jose Luis Galindo: Improved and recoded
	18/12/09 by pachulib : version 0.4.

***************************************************************************/

// Include section:

#include "libcosm.h"

// Define section:

// Global variables section:
char *data_format_tp_str[] = {"csv","json","xml","png","rss","atom"};
//char *t_trigger_str[] ={"gt","gte","lt","lte","eq","change", NULL};
//char *http_method_tp_str[] = {"POST","GET","PUT","DELETE",NULL};

// Private functions section:

// socket related
int resolve_host(char *host,char *ip);
int connect_server(int *fd);
void disconnect_server(int fd);
int read_data(int fd, char *buffer);
int send_data(int fd, char *buffer, unsigned int long_buffer);

// http status codes:
int recover_status(char *msg);
int recover_env_id_status(char *msg, unsigned int *env_id);
int recover_environment(char *msg, char *environment);
int recover_datastream(char *msg, char *datastream);
int recover_trigger_id_status(char *msg, unsigned int *tr_id);
int find_in_http(char *msg, char *tag, int *position);

// Other
int data_format_str(t_DATA_FORMAT format, char *format_str);
int save_file(char *filename, char *datastream);

// Get the IP address of the host given
int resolve_host(char *host, char *ip) {
	
	struct hostent *he;

	if ((he=gethostbyname(host)) == NULL) 
		return FALSE;
    
	if(inet_ntop(AF_INET, (void *)he->h_addr_list[0], ip, COSM_IP_LEN) == NULL) 
		return FALSE;
	
	#ifdef DEBUG_LIBCOSM
	printf("Host %s have the IP: %s\n", host, ip);
	#endif

  	return TRUE;
}

// Connect and get the file descriptor of the connection socket
int connect_server(int *fd) {
    
    struct sockaddr_in cosm_socket;
    char ip_addr[COSM_IP_LEN];

	if (!resolve_host(COSM_HOST, ip_addr))
		return FALSE;

	if ((*fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) 
		return FALSE;
    	
	cosm_socket.sin_family = AF_INET;    
	cosm_socket.sin_port = htons(COSM_PORT); 
	cosm_socket.sin_addr.s_addr = inet_addr(ip_addr);

    memset(&(cosm_socket.sin_zero), 8,0);  
    if (connect(*fd, (struct sockaddr *)&cosm_socket, sizeof(struct sockaddr)) == -1) 
		return FALSE;
	
	return TRUE;
}

// Disconnect the socket of the file descriptor given
void disconnect_server(int fd) {
	close(fd);
}

// Send data to server
int send_data(int fd, char *buffer, unsigned int long_buffer) {
	
	int ret = 0;

	#ifdef DEBUG_LIBCOSM
	printf("\nSEND_DATA:\n%s\n", buffer);
	#endif
	
	while (long_buffer)	{
		ret = send(fd, buffer, long_buffer, MSG_NOSIGNAL);

		if (ret == -1) {
			close(fd);
			return FALSE;
		}
		long_buffer -= ret;
		buffer += ret;
	}
	return TRUE;
}

// Read data
int read_data(int fd, char *buffer) {

	int long_buffer = COSM_MSG_LEN;
	int ret;
	
	char *ptoB = buffer;

	while (long_buffer > 0)	{
		ret = recv(fd, ptoB, long_buffer, MSG_NOSIGNAL);
		if (ret < 0) {
			close(fd);
			return FALSE;
		}
		if (ret == 0) {
			close(fd);
			if (long_buffer == COSM_MSG_LEN)	//no-data
				return FALSE;
			*ptoB='\0';
			return TRUE;
		}
		long_buffer -= ret;
		ptoB += ret;
	}
	return TRUE;
}

// Check returned code
int recover_status(char *msg) {
	
	int return_code;

	#ifdef DEBUG_LIBCOSM
	printf("\n__READ_DATA:\n%s\nEND_DATA__\n",msg);
	#endif

	if (sscanf(msg, "HTTP/1.1 %d*", &return_code) != 1) 
		return FALSE;

	#ifdef DEBUG_LIBCOSM
	printf("HTTP return code : %d\n", return_code);
	#endif

	return return_code;
}

// Get environment id
int recover_env_id_status(char *msg, unsigned int *env_id) {

	if (msg == NULL) 
		return FALSE; //strtok
	
	int nLength = strlen(msg);
	if (msg[nLength-1] == '\n') 
		msg[nLength-1] = '\0';

	char *tok = strtok(msg, "\n");
	while (tok != NULL) {
		if (sscanf(tok,"Location: http://api.cosm.com/v2/feeds/%d", env_id) == 1) {
			return TRUE;
		}
		tok = strtok(NULL,"\n");
	}

	return FALSE;
}

// Get environment
int recover_environment(char *msg, char *environment) {

	if (msg == NULL) 
		return FALSE;

	int offset1 = 0;
	if (!find_in_http(msg, "<title>", &offset1)) 
		return FALSE;
		
	int offset2 = 0;
	if (!find_in_http(msg, "</title>", &offset2)) 
		return FALSE;

	offset1 += strlen("<title>");
	sprintf(environment, "%s", msg + offset1);
	environment[offset2 - offset1] = '\0';
	return TRUE;
}

// Get datastream
int recover_datastream(char *msg, char *datastream) {

	if (msg == NULL) 
		return FALSE;

	int offset1 = 0;
	if (!find_in_http(msg, "<?xml", &offset1)) 
		return FALSE;
		
	int offset2 = 0;
	if (!find_in_http(msg, "</eeml>", &offset2)) 
		return FALSE;

	offset2 += strlen("</eeml>");
	sprintf(datastream, "%s", msg + offset1);
	datastream[offset2 - offset1] = '\0';
	return TRUE;
}

// Get trigger id status (NOT TESTED)
int recover_trigger_id_status(char *msg, unsigned int *tr_id) {
	
	if (msg == NULL) 
		return FALSE;

	int nLength = strlen(msg);
	if (msg[nLength-1] == '\n') 
		msg[nLength-1] = '\0';

	char * tok = strtok(msg, "\n");
	while (tok != NULL) {
		if (sscanf(tok, "Location: http://www.cosm.com/api/triggers/%d",tr_id) == 1) 
			return TRUE;
		tok = strtok(NULL,"\n");
	}

	return FALSE;
}

// Search tag in msg
int find_in_http(char *msg, char *tag, int *position) {
	
	int i = 0;
	int j = 0;
	int tagL = strlen(tag);

	if ((msg == NULL) || (tag ==NULL)) 
		return FALSE;

	while (msg[i]!='\0') {
		if (msg[i] != tag [j])
			j = 0;
		else {
			j++;
			if (j==tagL) { //found
				*position = i - tagL + 1;
				return TRUE;
			}
		}
		i++;
	}
	return FALSE;
}

// Convert data to string
int data_format_str(t_DATA_FORMAT format, char *format_str) {

	if ((format < CSV) || (format > ATOM)) {
		format_str = NULL;
		return FALSE;
	}
	else {
		sprintf(format_str, data_format_tp_str[format]);
		return TRUE;
	}
}

// Save datastream to a file
int save_file(char *filename, char *datastream) {
	
	FILE *ofp;
	ofp = fopen(filename, "w+");	//create if not exists

	if (ofp == NULL) {
		#ifdef DEBUG_LIBCOSM
		printf("Can't open output file %s!\n", filename);
		#endif
		return FALSE;
	}

	fprintf(ofp, "Data:\n\n%s\n\nEnd.", datastream);
	fflush(ofp);
	fclose(ofp);

	return TRUE;
}

// Global functions section:

//*************************************************************************
// create_environment_xml
//
//	Create an environment with the xml data passed (POST)
//
//	'api_key'		API Key
//	'xmldata'		Xml data
//	'env_id'		Returned environment id
//
//*************************************************************************
int create_environment_xml(char *api_key, char *xmldata, unsigned int *env_id) {

	char msg[COSM_MSG_LEN];
	int fd;

	sprintf(msg, "POST /v2/feeds/ HTTP/1.1\r\nHOST: %s\r\nX-ApiKey: %s\r\nAccept: application/xml\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", COSM_HOST, api_key, strlen(xmldata));
	sprintf(msg, "%s%s",msg, xmldata);

	if (!connect_server(&fd)) return COSM_SERVER_CONN_ERROR;

	if (!send_data(fd, msg, strlen(msg)+1)) return COSM_SEND_DATA_ERROR;

	if (!read_data(fd, msg)) return COSM_READ_DATA_ERROR;

	if (!recover_env_id_status(msg, env_id)) return COSM_GET_ENV_ID_ERROR;
	
	disconnect_server(fd);

	return COSM_NO_ERROR;
}

//*************************************************************************
// create_environment
//
//	Create an environment (POST)
//
//	'api_key'		API Key
//	'environment'	Environment name
//	'env_id'		Returned environment id
//
//*************************************************************************
int create_environment(char *api_key, char *environment, unsigned int *env_id) {

	char body[COSM_MSG_LEN];

	if (environment == NULL)
		//default environment: default_title
		sprintf(environment, "%s", "default_title");
	sprintf(body, "<eeml xmlns=\"http://www.eeml.org/xsd/0.5.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version=\"0.5.1\" xsi:schemaLocation=\"http://www.eeml.org/xsd/0.5.1 http://www.eeml.org/xsd/0.5.1/0.5.1.xsd\"><environment><title>%s</title></environment></eeml>", environment);

	return create_environment_xml(api_key, body, env_id);
}

//*************************************************************************
// delete_environment
//
//	Delete an environment (DELETE)
//
//	'api_key'		API Key
//	'env_id'		Environment id to delete
//
//*************************************************************************
int delete_environment(char *api_key, unsigned int env_id) {

	char msg[COSM_MSG_LEN];
	int fd = 0;
	int ret_code;

	//add header to msg
	sprintf(msg, "DELETE /v2/feeds/%d HTTP/1.1\r\nHOST: %s\r\nX-ApiKey: %s\r\n\r\n",env_id, COSM_HOST, api_key);

	if (!connect_server(&fd)) return COSM_SERVER_CONN_ERROR;

	if (!send_data(fd,msg,strlen(msg)+1)) return COSM_SEND_DATA_ERROR;

	if (!read_data(fd, msg)) return COSM_READ_DATA_ERROR;
	
	ret_code = recover_status(msg);
	if((ret_code != COSM_RET_OK) && (ret_code != COSM_RET_CREATED)) return ret_code;

	disconnect_server(fd);
	
	return COSM_NO_ERROR;
}

//*************************************************************************
// get_environment_xml
//
//	Get the environment (GET)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'xmldata'		Returned environment in xml format
//
//*************************************************************************
int get_environment_xml(char * api_key, unsigned int env_id, char *xmldata) {

	char msg[COSM_MSG_LEN];
	int fd = 0;
	int offset = 0;
	int ret_code;
	
	sprintf(msg, "GET /v2/feeds/%d.xml HTTP/1.1\r\nHOST: %s\r\nX-ApiKey: %s\r\nAccept: application/xml\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", env_id, COSM_HOST, api_key, 0);

	if (!connect_server(&fd)) return COSM_SERVER_CONN_ERROR;

	if (!send_data(fd, msg, strlen(msg)+1)) return COSM_SEND_DATA_ERROR;

	if (!read_data(fd, msg)) return COSM_READ_DATA_ERROR;

	ret_code = recover_status(msg);
	if((ret_code != COSM_RET_OK) && (ret_code != COSM_RET_CREATED)) return ret_code;
	
	if (msg == NULL) return COSM_GET_ENV_NAME_ERROR;

	if (!find_in_http(msg, "<?xml", &offset)) return COSM_GET_ENV_NAME_ERROR;

	disconnect_server(fd);
	
	sprintf(xmldata, "%s", msg + offset);
	
	#ifdef DEBUG_LIBCOSM
	printf("\nXML_DATA:\n%s\n", xmldata);
	#endif		
	
	return COSM_NO_ERROR;
}

//*************************************************************************
// get_environment
//
//	Get the environment name (GET)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'environment'	Returned environment name
//
//*************************************************************************
int get_environment(char *api_key, unsigned int env_id, char *environment) {

	char msg[COSM_MSG_LEN];
	int fd = 0;
	int ret_code;
	
	sprintf(msg, "GET /v2/feeds/%d.xml HTTP/1.1\r\nHOST: %s\r\nX-ApiKey: %s\r\nAccept: application/xml\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", env_id, COSM_HOST, api_key, 0);

	if (!connect_server(&fd)) return COSM_SERVER_CONN_ERROR;

	if (!send_data(fd, msg, strlen(msg)+1)) return COSM_SEND_DATA_ERROR;

	if (!read_data(fd, msg)) return COSM_READ_DATA_ERROR;

	ret_code = recover_status(msg);
	if((ret_code != COSM_RET_OK) && (ret_code != COSM_RET_CREATED)) return ret_code;

	if (!recover_environment(msg, environment)) return COSM_GET_ENV_NAME_ERROR;
	
	disconnect_server(fd);
	
	#ifdef DEBUG_LIBCOSM
	printf("\nEnvironment: %s\n", environment);
	#endif
	
	return COSM_NO_ERROR;
}

//*************************************************************************
// update_environment_xml
//
//	Update the environment with the xml data passed (PUT)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'xmldata'		Xml data
//
//*************************************************************************
int update_environment_xml(char *api_key, unsigned int env_id, char *xmldata) {
	
	char msg[COSM_MSG_LEN];
	int fd = 0;
	int ret_code;

	sprintf(msg, "PUT /v2/feeds/%d HTTP/1.1\r\nHOST: %s\r\nX-ApiKey: %s\r\nAccept: application/xml\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", env_id, COSM_HOST, api_key, strlen(xmldata));
	sprintf(msg, "%s%s", msg, xmldata);

	if (!connect_server(&fd)) return COSM_SERVER_CONN_ERROR;

	if (!send_data(fd, msg, strlen(msg)+1)) return COSM_SEND_DATA_ERROR;

	if (!read_data(fd, msg)) return COSM_READ_DATA_ERROR;

	ret_code = recover_status(msg);
	if((ret_code != COSM_RET_OK) && (ret_code != COSM_RET_CREATED)) return ret_code;

	disconnect_server(fd);
	
	return COSM_NO_ERROR;
}

//*************************************************************************
// update_environment
//
//	Update the environment name (PUT)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'environment'	environment name
//
//*************************************************************************
int update_environment(char *api_key, unsigned int env_id, char *environment) {
	
	char body[COSM_MSG_LEN];

	sprintf(body, "<eeml xmlns=\"http://www.eeml.org/xsd/0.5.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version=\"0.5.1\" xsi:schemaLocation=\"http://www.eeml.org/xsd/0.5.1 http://www.eeml.org/xsd/0.5.1/0.5.1.xsd\"><environment><title>%s</title></environment></eeml>", environment);
	
	return update_environment_xml(api_key, env_id, body);
}

//*************************************************************************
// create_datastream_xml
//
//	Create a datastream in the environment given with the passed xmldata (POST)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'ds_id'			Datastream id
//	'xmldata'		Raw xml data
//
//*************************************************************************
int create_datastream_xml(char *api_key, unsigned int env_id, char *ds_id, char *xmldata) {

	char msg[COSM_MSG_LEN];
	int fd;
	int ret_code;

	sprintf(msg, "POST /v2/feeds/%d/datastreams/ HTTP/1.1\r\nHOST: %s\r\nX-ApiKey: %s\r\nAccept: application/xml\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",env_id, COSM_HOST, api_key, strlen(xmldata));
	sprintf(msg, "%s%s", msg, xmldata);

	if (!connect_server(&fd)) return COSM_SERVER_CONN_ERROR;

	if (!send_data(fd, msg, strlen(msg)+1)) return COSM_SEND_DATA_ERROR;

	if (!read_data(fd, msg)) return COSM_READ_DATA_ERROR;

	ret_code = recover_status(msg);
	if((ret_code != COSM_RET_OK) && (ret_code != COSM_RET_CREATED)) return ret_code;
	
	disconnect_server(fd);
	
	return COSM_NO_ERROR;
}

//*************************************************************************
// create_datastream
//
//	Create a datastream in the environment given (POST)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'ds_id'			Datastream id
//	'curr_value'	Current value
//
//*************************************************************************
int create_datastream(char *api_key, unsigned int env_id, char *ds_id, char *curr_value) {

	char body[COSM_MSG_LEN];

	sprintf(body, "<eeml xmlns=\"http://www.eeml.org/xsd/0.5.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version=\"0.5.1\" xsi:schemaLocation=\"http://www.eeml.org/xsd/0.5.1 http://www.eeml.org/xsd/0.5.1/0.5.1.xsd\"><environment><data id=\"%s\"><current_value>%s</current_value></data></environment></eeml>", ds_id, curr_value);

	return create_datastream_xml(api_key, env_id, ds_id, body);
}

//*************************************************************************
// delete_datastream
//
//	Delete a datastream in the environment given (DELETE)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'ds_id'			Datastream id
//
//*************************************************************************
int delete_datastream(char *api_key, unsigned int env_id, char *ds_id) {

	char msg[COSM_MSG_LEN];
	int fd;
	int ret_code;

	sprintf(msg,"DELETE /v2/feeds/%d/datastreams/%s HTTP/1.1\r\nHOST: %s\r\nX-ApiKey: %s\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", env_id, ds_id, COSM_HOST, api_key, 0);

	if (!connect_server(&fd)) return COSM_SERVER_CONN_ERROR;

	if (!send_data(fd, msg, strlen(msg)+1)) return COSM_SEND_DATA_ERROR;

	if (!read_data(fd, msg)) return COSM_READ_DATA_ERROR;

	ret_code = recover_status(msg);
	if((ret_code != COSM_RET_OK) && (ret_code != COSM_RET_CREATED)) return ret_code;
	
	disconnect_server(fd);
	
	return COSM_NO_ERROR;
}

//*************************************************************************
// get_datastream_xml
//
//	Get the xml of the datastream given (GET)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'ds_id'			Datastream id
//	'xmldata'		Returned Xml data
//
//*************************************************************************
int get_datastream_xml(char *api_key, unsigned int env_id, char *ds_id, char *xmldata) {

	char msg[COSM_MSG_LEN];
	int fd;
	int ret_code;

	sprintf(msg,"GET /v2/feeds/%d/datastreams/%s HTTP/1.1\r\nHOST: %s\r\nX-ApiKey: %s\r\nAccept: application/xml\r\n\r\n", env_id, ds_id, COSM_HOST, api_key);

	if (!connect_server(&fd)) return COSM_SERVER_CONN_ERROR;

	if (!send_data(fd, msg, strlen(msg)+1)) return COSM_SEND_DATA_ERROR;

	if (!read_data(fd, msg)) return COSM_READ_DATA_ERROR;

	ret_code = recover_status(msg);
	if((ret_code != COSM_RET_OK) && (ret_code != COSM_RET_CREATED)) return ret_code;
	
	if (!recover_datastream(msg, xmldata)) return COSM_GET_DATASTREAM_ERROR;
	
	disconnect_server(fd);
	
	#ifdef DEBUG_LIBCOSM
	printf("\nDatastream: %s\n", xmldata);
	#endif
	
	return COSM_NO_ERROR;
}

//*************************************************************************
// update_datastream_xml
//
//	update the datastream given with the xmldata passed (PUT)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'ds_id'			Datastream id
//	'xmldata'		Raw xml data
//
//*************************************************************************
int update_datastream_xml(char *api_key, unsigned int env_id, char *ds_id, char *xmldata) {

	char msg[COSM_MSG_LEN];
	int fd;
	int ret_code;
	
	sprintf(msg, "PUT /v2/feeds/%d/datastreams/%s HTTP/1.1\r\nHOST: %s\r\nX-ApiKey: %s\r\nAccept: application/xml\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", env_id, ds_id, COSM_HOST, api_key, strlen(xmldata));
	sprintf(msg, "%s%s", msg, xmldata);
	
	if (!connect_server(&fd)) return COSM_SERVER_CONN_ERROR;

	if (!send_data(fd, msg, strlen(msg)+1)) return COSM_SEND_DATA_ERROR;

	if (!read_data(fd, msg)) return COSM_READ_DATA_ERROR;

	ret_code = recover_status(msg);
	if((ret_code != COSM_RET_OK) && (ret_code != COSM_RET_CREATED)) return ret_code;

	disconnect_server(fd);
	
	return COSM_NO_ERROR;
}

//*************************************************************************
// update_datastream
//
//	update the datastream given (PUT)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'ds_id'			Datastream id
//	'curr_value'	Current value
//
//*************************************************************************
int update_datastream(char *api_key, unsigned int env_id, char *ds_id, char *curr_value) {

	char body[COSM_MSG_LEN];
	
	sprintf(body, "<eeml xmlns=\"http://www.eeml.org/xsd/0.5.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version=\"0.5.1\" xsi:schemaLocation=\"http://www.eeml.org/xsd/0.5.1 http://www.eeml.org/xsd/0.5.1/0.5.1.xsd\"><environment><data id=\"%s\"><current_value>%s</current_value></data></environment></eeml>", ds_id, curr_value);

	return update_datastream_xml(api_key, env_id, ds_id, body);
}

//*************************************************************************
// create_datapoint_xml
//
//	Create a datapoint in the datastream given with the passed xmldata (POST)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'ds_id'			Datastream id
//	'xmldata'		Raw xml data
//
//*************************************************************************
int create_datapoint_xml(char *api_key, unsigned int env_id, char *ds_id, char *xmldata) {

	char msg[COSM_MSG_LEN];
	int fd;
	int ret_code;

	sprintf(msg, "POST /v2/feeds/%d/datastreams/%s/datapoints/ HTTP/1.1\r\nHOST: %s\r\nX-ApiKey: %s\r\nAccept: application/xml\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",env_id, ds_id, COSM_HOST, api_key, strlen(xmldata));
	sprintf(msg, "%s%s", msg, xmldata);

	if (!connect_server(&fd)) return COSM_SERVER_CONN_ERROR;

	if (!send_data(fd, msg, strlen(msg)+1)) return COSM_SEND_DATA_ERROR;

	if (!read_data(fd, msg)) return COSM_READ_DATA_ERROR;

	ret_code = recover_status(msg);
	if((ret_code != COSM_RET_OK) && (ret_code != COSM_RET_CREATED)) return ret_code;
		
	disconnect_server(fd);
	
	return COSM_NO_ERROR;
}

//*************************************************************************
// create_datapoint
//
//	Create a datapoint in the datastream given (POST)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'ds_id'			Datastream id
//	'timestamp'		Timestamp for the value
//	'curr_value'	Current value
//
//*************************************************************************
int create_datapoint(char *api_key, unsigned int env_id, char *ds_id, char *timestamp, char *curr_value) {

	char body[COSM_MSG_LEN];

	sprintf(body, "<eeml xmlns=\"http://www.eeml.org/xsd/0.5.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version=\"0.5.1\" xsi:schemaLocation=\"http://www.eeml.org/xsd/0.5.1 http://www.eeml.org/xsd/0.5.1/0.5.1.xsd\"><environment><data><datapoints><value at=\"%s\">%s</value></datapoints></data></environment></eeml>", timestamp, curr_value);

	return create_datapoint_xml(api_key, env_id, ds_id, body);
}

//*************************************************************************
// delete_datapoint
//
//	Delete the datapoint given (DELETE)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'ds_id'			Datastream id
//	'timestamp'		Timestamp for the value
//
//*************************************************************************
int delete_datapoint(char *api_key, unsigned int env_id, char *ds_id, char *timestamp) {

	char msg[COSM_MSG_LEN];
	int fd;
	int ret_code;

	sprintf(msg,"DELETE /v2/feeds/%d/datastreams/%s/datapoints/%s HTTP/1.1\r\nHOST: %s\r\nX-ApiKey: %s\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", env_id, ds_id, timestamp, COSM_HOST, api_key, 0);

	if (!connect_server(&fd)) return COSM_SERVER_CONN_ERROR;

	if (!send_data(fd, msg, strlen(msg)+1)) return COSM_SEND_DATA_ERROR;

	if (!read_data(fd, msg)) return COSM_READ_DATA_ERROR;

	ret_code = recover_status(msg);
	if((ret_code != COSM_RET_OK) && (ret_code != COSM_RET_CREATED)) return ret_code;
	
	disconnect_server(fd);
	
	return COSM_NO_ERROR;
}

//*************************************************************************
// get_datastream_xml
//
//	Get the xml of the datastream given (GET)
//
//	'env_id'		Environment id
//	'api_key'		API Key
//	'ds_id'			Datastream id
//	'xmldata'		Returned Xml data
//
//*************************************************************************
int get_datapoint_xml(char *api_key, unsigned int env_id, char *ds_id, char *timestamp, char *xmldata) {

	char msg[COSM_MSG_LEN];
	int fd;
	int ret_code;

	sprintf(msg,"GET /v2/feeds/%d/datastreams/%s/datapoints/%s HTTP/1.1\r\nHOST: %s\r\nX-ApiKey: %s\r\nAccept: application/xml\r\n\r\n", env_id, ds_id, timestamp, COSM_HOST, api_key);

	if (!connect_server(&fd)) return COSM_SERVER_CONN_ERROR;

	if (!send_data(fd, msg, strlen(msg)+1)) return COSM_SEND_DATA_ERROR;

	if (!read_data(fd, msg)) return COSM_READ_DATA_ERROR;

	ret_code = recover_status(msg);
	if((ret_code != COSM_RET_OK) && (ret_code != COSM_RET_CREATED)) return ret_code;
	
	if (!recover_datastream(msg, xmldata)) return COSM_GET_DATAPOINT_ERROR;
	
	disconnect_server(fd);
	
	#ifdef DEBUG_LIBCOSM
	printf("\nDatapoint: %s\n", xmldata);
	#endif
	
	return COSM_NO_ERROR;
}

//*************************************************************************
// update_datapoint_xml
//
//	update the datapoint with the xmldata passed (PUT)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'ds_id'			Datastream id
//	'timestamp'		Timestamp for the value
//	'xmldata'		Raw xml data
//
//*************************************************************************
int update_datapoint_xml(char *api_key, unsigned int env_id, char *ds_id, char *timestamp, char *xmldata) {

	char msg[COSM_MSG_LEN];
	int fd;
	int ret_code;
	
	sprintf(msg, "PUT /v2/feeds/%d/datastreams/%s/datapoints/%s HTTP/1.1\r\nHOST: %s\r\nX-ApiKey: %s\r\nAccept: application/xml\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", env_id, ds_id, timestamp, COSM_HOST, api_key, strlen(xmldata));
	sprintf(msg, "%s%s", msg, xmldata);
	
	if (!connect_server(&fd)) return COSM_SERVER_CONN_ERROR;

	if (!send_data(fd, msg, strlen(msg)+1)) return COSM_SEND_DATA_ERROR;

	if (!read_data(fd, msg)) return COSM_READ_DATA_ERROR;

	ret_code = recover_status(msg);
	if((ret_code != COSM_RET_OK) && (ret_code != COSM_RET_CREATED)) return ret_code;

	disconnect_server(fd);
	
	return COSM_NO_ERROR;
}

//*************************************************************************
// update_datapoint
//
//	update the datapoint given (PUT)
//
//	'api_key'		API Key
//	'env_id'		Environment id
//	'ds_id'			Datastream id
//	'timestamp'		Timestamp for the value
//	'curr_value'	Current value
//
//*************************************************************************
int update_datapoint(char *api_key, unsigned int env_id, char *ds_id, char *timestamp, char *curr_value) {

	char body[COSM_MSG_LEN];
	
	sprintf(body, "<eeml xmlns=\"http://www.eeml.org/xsd/0.5.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" version=\"0.5.1\" xsi:schemaLocation=\"http://www.eeml.org/xsd/0.5.1 http://www.eeml.org/xsd/0.5.1/0.5.1.xsd\"><environment><data><datapoints><value>%s</value></datapoints></data></environment></eeml>", curr_value);

	return update_datapoint_xml(api_key, env_id, ds_id, timestamp, body);
}
