/**************************************************************************

	X10RFMsg.h

	Header file for Rfxcom X10RF Message queue functions

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

	27/12/12 by Jose Luis Galindo : First version
	
***************************************************************************/

//*************************************************************************
//*************************************************************************
// 									INCLUDE SECTION
//*************************************************************************
//*************************************************************************

#include <sys/msg.h>


//*************************************************************************
//*************************************************************************
// 									DEFINES SECTION
//*************************************************************************
//*************************************************************************

#define MSGQUEUE_X10RF	8008	// Message queue ID for X10RF commands to send


//*************************************************************************
//*************************************************************************
// 									CUSTOM TYPES
//*************************************************************************
//*************************************************************************

// X10 Message
typedef struct {
	long msg_id;
	char device[4];				// X10 Address (A1 to P16)
	BYTE funct;					// X10 Function
} t_X10RF_MSG;


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

