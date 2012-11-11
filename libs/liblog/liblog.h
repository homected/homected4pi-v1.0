/**************************************************************************

	liblog.h

	Header file for the Log library

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

	Library based on OPNODE project from Daniel Berenguer (www.opnode.org)
	Copyright (c) Daniel Berenguer <dberenguer@usapiens.com>, 2007

	Last changes:

	20/04/12 by Jose Luis Galindo: Recoded
	02/27/07 by Daniel Berenguer : first version.

***************************************************************************/

//*************************************************************************
//*************************************************************************
// 									INCLUDE SECTION
//*************************************************************************
//*************************************************************************

#ifndef _LIBLOG_H
#define _LIBLOG_H

#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

//*************************************************************************
//*************************************************************************
// 									DEFINES SECTION
//*************************************************************************
//*************************************************************************

#define BYTE				unsigned char
#define LOG_FILE_PATH_LEN	255
#define LOG_FILE_SIZE		50000			// Maximum allowed size for log files

//*************************************************************************
//*************************************************************************
// 									CUSTOM TYPES
//*************************************************************************
//*************************************************************************


//*************************************************************************
//*************************************************************************
// 									GLOBAL VARIABLES
//*************************************************************************
//*************************************************************************

char logfile_event[LOG_FILE_PATH_LEN];		// Event log file
char logfile_error[LOG_FILE_PATH_LEN];		// Error log file

//*************************************************************************
//*************************************************************************
// 									GLOBAL FUNCTIONS
//*************************************************************************
//*************************************************************************

// liblog.c functions:
//-------------------------
void logInit(const char *strEvnFile, const char *strErrFile);
void logError(const char *strFormat, ...);
void logEvent(BYTE flgDate, const char *strFormat, ...);

#endif
