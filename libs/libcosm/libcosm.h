/**************************************************************************

	libcosm.h

	Header file for libcosm

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
	12/05/12 by Jose Luis Galindo: Updated references from Pachube to Cosm
	05/05/12 by Jose Luis Galindo: Improved and recoded
	18/12/09 by pachulib : version 0.4.

***************************************************************************/


//*************************************************************************
//*************************************************************************
// 									INCLUDE SECTION
//*************************************************************************
//*************************************************************************

#ifndef _LIBCOSM_H
#define _LIBCOSM_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>     // signal
#include <sys/types.h>  // socket, connect,bind, sccept, send, recv, setsockopt    
#include <sys/socket.h> // socket, inet_addr, inet_ntoa, connect, bind, listen, accept, send, recv, setsockopt     
#include <netinet/in.h> // htons, inet_addr, inet_ntoa 
#include <arpa/inet.h>  // inet_addr, inet_ntoa
#include <unistd.h>     // close    
#include <string.h>     // memset, strerror,      
#include <errno.h>
#include <netdb.h>      // gethostbyname, gethostbyaddr,  

//*************************************************************************
//*************************************************************************
// 									DEFINES SECTION
//*************************************************************************
//*************************************************************************

//#define	DEBUG_LIBCOSM				// Remove this comment to debug

#define TRUE 							1
#define FALSE 							0

#define COSM_IP_LEN						16		// xxx.xxx.xxx.xxx + '\0'
#define COSM_PORT			 			80
#define COSM_HOST						"api.cosm.com"
#define COSM_APIKEY_LEN					200
#define COSM_MSG_LEN					1500
#define COSM_URL_LEN					255
#define COSM_DATAFORMAT_LEN				5

//Server return codes:
#define COSM_NO_ERROR					000
#define COSM_SERVER_CONN_ERROR			001
#define COSM_SEND_DATA_ERROR			002
#define COSM_READ_DATA_ERROR			003
#define COSM_GET_ENV_ID_ERROR			004
#define COSM_GET_ENV_NAME_ERROR			005
#define COSM_GET_DATASTREAM_ERROR		006
#define COSM_GET_DATAPOINT_ERROR		007
#define COSM_RET_OK						200
#define COSM_RET_CREATED				201
#define COSM_RET_NOT_AUTH 				401
#define COSM_RET_FORBIDDEN 				403
#define COSM_RET_NOT_FOUND 				404
#define COSM_RET_UNPROCESSABLE_ENTITY 	422
#define COSM_RET_INTERNAL_SERVER_ERROR 	500
#define COSM_RET_NO_SERVER_ERROR 		503

//*************************************************************************
//*************************************************************************
// 									CUSTOM TYPES
//*************************************************************************
//*************************************************************************

//http method types
typedef enum {
    POST,
    GET,
    PUT,
    DELETE
} t_HTTP_METHOD;

//data format types
typedef enum {
    CSV,
    JSON,
    XML,
    PNG,
    RSS,
    ATOM
} t_DATA_FORMAT;
/*
//trigger operation types
typedef enum {
	GT,
	GTE,
	LT,
	LTE,
	EQ,
	CHANGE
} t_TRIGGER_OP;

//trigger
typedef struct {
	unsigned int env_id;
	unsigned int ds_id;
	unsigned int threshold;
	t_TRIGGER_OP type;
	char url[COSM_URL_LEN];
	unsigned int tr_id;
} t_TRIGGER;

//graph configuration:
typedef struct {
    unsigned int weigth;
    unsigned int height;
    char *color;
    char *title;
    char *legend;
    unsigned int stroke_size;
    unsigned int axix_labels;
    unsigned int detailed_grid;
} t_GRAPH;
*/

//*************************************************************************
//*************************************************************************
// 									GLOBAL VARIABLES
//*************************************************************************
//*************************************************************************

//*************************************************************************
//*************************************************************************
// 									GLOBAL FUNCTIONS
//*************************************************************************
//*************************************************************************

// environment functions
int create_environment_xml(char *api_key, char *xmldata, unsigned int *env_id); //POST
int create_environment(char *api_key, char *environment, unsigned int *env_id); //POST
int delete_environment(char *api_key, unsigned int env_id); //DELETE
int get_environment_xml(char *api_key, unsigned int env_id, char *xmldata); //GET
int get_environment(char *api_key, unsigned int env_id, char *environment); //GET
int update_environment_xml(char *api_key, unsigned int env_id, char *xmldata); //PUT
int update_environment(char *api_key, unsigned int env_id, char *environment); //PUT

//datastream functions
int create_datastream_xml(char *api_key, unsigned int env_id, char *ds_id, char *xmldata); //POST
int create_datastream(char *api_key, unsigned int env_id, char *ds_id, char *curr_value); //POST
int delete_datastream(char *api_key, unsigned int env_id, char *ds_id); //DELETE
int get_datastream_xml(char *api_key, unsigned int env_id, char *ds_id, char *xmldata); //GET
int update_datastream_xml(char *api_key, unsigned int env_id, char *ds_id, char *xmldata); //PUT
int update_datastream(char *api_key, unsigned int env_id, char *ds_id, char *curr_value); //PUT

//datapoints functions
int create_datapoint_xml(char *api_key, unsigned int env_id, char *ds_id, char *xmldata); //POST
int create_datapoint(char *api_key, unsigned int env_id, char *ds_id, char *timestamp, char *curr_value); //POST
int delete_datapoint(char *api_key, unsigned int env_id, char *ds_id, char *timestamp); //DELETE
int get_datapoint_xml(char *api_key, unsigned int env_id, char *ds_id, char *timestamp, char *xmldata); //GET
int update_datapoint_xml(char *api_key, unsigned int env_id, char *ds_id, char *timestamp, char *xmldata); //PUT
int update_datapoint(char *api_key, unsigned int env_id, char *ds_id, char *timestamp, char *curr_value); //PUT

//triggers functions
//int create_trigger(t_TRIGGER *trigger, char *api_key); //POST
//int create_trigger(unsigned int env_id, unsigned int ds_id, unsigned int threshold, trigger_tp type, char * url,char *api_key,unsigned int *tr_id); //POST
//int get_all_triggers(data_format_tp format, char *api_key, char *trigger_list); //GET
//int update_trigger(trigger_tp * trigger, char *api_key);//PUT
//int get_trigger(int trigger_id, data_format_tp format, char *api_key, char *trigger); //GET
//int delete_trigger(int trigger_id, char *api_key); //DELETE

#endif
