#include "process.h"

int main(void)
{
	process_init("xaphub", pid_filepath);
	
	/* Bucle principal que se ejecuta hasta que se reciba un SIGTERM (ejecución del comando kill) */
	while(process_state == PROC_RUNNING)
	{
		sleep( 1 );
	}
	
	process_finish();
}
