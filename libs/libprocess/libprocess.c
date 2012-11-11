/**************************************************************************

	process.c

	functions to init, stop and finish a process

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

#include "libprocess.h"

//*************************************************************************
// process_stop
//
//	Function called when received a SIGTERM signal
//
//	'signum'	
//
//	Returns:
//		Nothing
//*************************************************************************
void process_stop(int signum)
{
	process_state = PROC_STOPPED;
}

//*************************************************************************
// process_init
//
//	Initialize and build a process
//
//	'app_name'		Name of the process, a file is created on /var/run/
//					with this name and extension .pid
//
//	'pid_file_path'	Path to the .pid file (/var/run/app_name.pid)
//
//	Returns:
//		Nothing
//*************************************************************************
void process_init(const char* app_name, char* pid_file_path)
{
	struct stat st;
	FILE *pid_file;
	pid_t pid;
	pid_t sid;
 
	process_state = PROC_RUNNING;
	
	/* Determinamos la ruta del fichero PID */
	sprintf(pid_file_path, "/var/run/%s.pid", app_name);
	
	/* Activamos el manejo de mensajes al demonio de syslogd */
	openlog(app_name, LOG_CONS | LOG_PID, LOG_DAEMON);
 
	/* Comprobamos si existe le fichero PID */
	if(stat(pid_file_path, &st) == 0)
	{
		syslog(LOG_ERR, "A similar process is already loaded");
		exit(EXIT_FAILURE);
	}
 
	/* Creamos el fichero PID */
	pid_file = fopen(pid_file_path, "w");
	if(pid_file == NULL)
	{
		syslog(LOG_ERR, "Error creating the PID file");
        exit(EXIT_FAILURE);
	}
 
	/* Creamos un hilo de ejecucion */
	pid = fork();
 
	/* Si no se pudo crear el hilo */
	if(pid < 0)
	{
		syslog(LOG_ERR, "The thread can't be created");
		fclose(pid_file);
		exit(EXIT_FAILURE);
	}
	/* Si se pudo crear el hilo */
	else if(pid > 0)
	{
		/* Escribimos en el fichero PID el identificador del proceso */
	    fprintf(pid_file, "%d\n", pid);
		fclose(pid_file);
		exit(EXIT_SUCCESS);
	}
 
	/* Se evita el heredar la máscara de ficheros */
	umask(0);
 
	/* Convertimos el proceso hijo a demonio */
	sid = setsid();
 
	/* Si no se pudo convertir en demonio */
	if(sid < 0)
	{
		syslog(LOG_ERR, "The daemon can't be created");
		unlink(pid_file_path);
		exit(EXIT_FAILURE);
	}
 
	/* Cambiamos al directorio raiz */
	if(chdir("/") < 0 )
	{
		syslog(LOG_ERR, "Can't chage to root directory");
		unlink(pid_file_path);
		exit(EXIT_FAILURE);
	}
 
	/* Cerramos los descriptores de archivo que no usaremos */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
 
	/* Registramos la función que recibe la señal SIGTERM */
	signal(SIGTERM, process_stop);
}

//*************************************************************************
// process_finish
//
//	Destroy the process
//
//	Returns:
//		Nothing
//*************************************************************************
void process_finish(void)
{
	/* Borramos el fichero PID */
	unlink(pid_filepath);
	
	/* Cerramos el log */
	closelog();
}
