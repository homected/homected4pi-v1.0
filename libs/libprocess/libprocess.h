/**************************************************************************

	process.h

	Header file for process.c

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

	03/04/12 by Jose Luis Galindo : first version.

***************************************************************************/

//*************************************************************************
//*************************************************************************
// 								DEFINE SECTION
//************************************************************************* 
//*************************************************************************

#ifndef _LIBPROCESS_H
#define _LIBPROCESS_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>

//*************************************************************************
//*************************************************************************
// 								DEFINE SECTION
//************************************************************************* 
//*************************************************************************

#define PROC_RUNNING	0
#define PROC_STOPPED 	1

//*************************************************************************
//*************************************************************************
// 								GLOBAL VARIABLES
//				Accessible from anywhere within the application
//************************************************************************* 
//*************************************************************************

volatile char process_state;
char pid_filepath[100];

//************************************************************************* 
//*************************************************************************
// 								FUNCTIONS SECTION
//************************************************************************* 
//*************************************************************************

void process_stop(int);
void process_init(const char*, char*);
void process_finish(void);

#endif
