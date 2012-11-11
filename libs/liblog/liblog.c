/**************************************************************************

	liblog.c

	Functions to manage a log

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

// Include section:

#include "liblog.h"

// Define section:

// Global functions section:

// Global variables section:

//*************************************************************************
// logInit
//
//	Set up the error and event log files
//
//	'strEvnFile'	Event log file path
//	'strErrFile'	Error log file path
//
//*************************************************************************

void logInit(const char *strEvnFile, const char *strErrFile) {
	strcpy(logfile_event, strEvnFile);
	strcpy(logfile_error, strErrFile);
}

//*************************************************************************
// logError
//
//	Writes an error line in the error file
//
//	'strFormat'		format log string
//
//*************************************************************************

void logError(const char *strFormat, ...) {
	
	FILE *fpErr;							// Pointer to the error file
	va_list argp;							// Variable list of arguments
	time_t sysTimer;						// Stores the content of the system timer
	struct tm *tmCurrDate;					// Stores date and time (current date/time)
	struct stat statFile;					// File data

	sysTimer = time(NULL);					// Get the current chronometer
	tmCurrDate = localtime(&sysTimer);		// Get the current date/time

	// Current sizeo of the log file
	stat(logfile_error, &statFile);

	if (statFile.st_size > LOG_FILE_SIZE)	// If the log file exceeds the maximum size
		fpErr = fopen(logfile_error, "w+");	// Create a new log file
	else									// else
		fpErr = fopen(logfile_error, "a");	// Open the xml file in "append" mode

	if (fpErr != NULL) {
		// Log date-time
		fprintf(fpErr, "%.2d/%.2d/%.2d %.2d:%.2d : ", tmCurrDate->tm_mon+1, tmCurrDate->tm_mday,
			tmCurrDate->tm_year-100, tmCurrDate->tm_hour, tmCurrDate->tm_min);
		
		// Add log
		va_start(argp, strFormat);
		vfprintf(fpErr, strFormat, argp);
		va_end(argp);
		fprintf(fpErr, " (errno = %d)\n", errno);
		
		// Close log file
		fclose(fpErr);
	}
}

//*************************************************************************
// logEvent
//
//	Writes a new line in the event log file
//
//	'flgDate'		if TRUE, add date/time to the log
//	'strFormat'		format log string
//
//*************************************************************************

void logEvent(BYTE flgDate, const char *strFormat, ...) {
	
	FILE *fpLog;							// Pointer to the log file
	va_list argp;							// Variable list of arguments
	time_t sysTimer;						// Stores the content of the system timer
	struct tm *tmCurrDate;					// Stores date and time (current date/time)
	struct stat statFile;					// File data

	sysTimer = time(NULL);					// Get the current chronometer
	tmCurrDate = localtime(&sysTimer);		// Get the current date/time

	// Current sizeo of the log file
	stat(logfile_event, &statFile);

	if (statFile.st_size > LOG_FILE_SIZE)	// If the log file exceeds the maximum size
		fpLog = fopen(logfile_event, "w+");	// Create a new log file
	else									// else
		fpLog = fopen(logfile_event, "a");	// Open the xml file in "append" mode

	if (fpLog != NULL) {
		// If date/time must be added
		if (flgDate) {
			// Log date-time
			fprintf(fpLog, "%.2d/%.2d/%.2d %.2d:%.2d : ", tmCurrDate->tm_mon+1, tmCurrDate->tm_mday,
				tmCurrDate->tm_year-100, tmCurrDate->tm_hour, tmCurrDate->tm_min);
		}
		
		// Add log
		va_start(argp, strFormat);
		vfprintf(fpLog, strFormat, argp);
		va_end(argp);
		fprintf(fpLog, "\n");
		
		// Close log file
		fclose(fpLog);
	}
}
