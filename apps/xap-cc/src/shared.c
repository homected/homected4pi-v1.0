/**************************************************************************

	shared.c

	Shared functions

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

	26/04/12 by Jose Luis Galindo : First version
	
***************************************************************************/

#include "shared.h"

// Create shared memory region for the shared variables
short int ccSharedMemSetup(void)
{
	int memsize;
	
	// Calculate nums of pagesize needed
	memsize = (sizeof(t_CC_CONFIG) % getpagesize()) ? (sizeof(t_CC_CONFIG)/getpagesize()) + 1 : (sizeof(t_CC_CONFIG)/getpagesize());
		
	// Create shared memory region for the shared variable 
	if((shmCcConfig = shmget(SHMKEY_CCCFG, memsize, IPC_CREAT | 0666)) < 0)
		return 0;

	// Attach the shared variable to the shared memory area
	if((ccConfig = (t_CC_CONFIG*)shmat(shmCcConfig, NULL, 0)) < 0)
		return 0;

	return 1;
}

// Destroy shared memory areas
void ccSharedMemClose(void)
{
	shmdt(ccConfig);							// Detach the variable from the shared memory area
	shmctl(shmCcConfig, IPC_RMID, NULL);		// Destroy the shared memory area
}
