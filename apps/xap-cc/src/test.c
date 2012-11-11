#include "shared.h"

int main()
{
	// Create shared memory areas
	if (!ccSharedMemSetup()) 
		printf("Error allocating shared resources\n");
		
	strcpy(ccConfig->xap_addr.instance, "cambiada");
	ccConfig->xap_hbeat = 15;
	ccConfig->saveFlag = TRUE;
	
	printf("Parameters changed\n");
	
	// Close shared memory areas
	ccSharedMemClose();
	
	return 1;
}
