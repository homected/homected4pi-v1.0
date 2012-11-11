#**************************************************************************
#
#	Makefile
#
#	xap-cosm makefile
#	
#	Copyright (c) 2012 Jose Luis Galindo <support@homected.com>
#	
#	This file is part of the HOMECTED project.
#	
#	HOMECTED is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#	
#	HOMECTED is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#	GNU General Public License for more details.
#	
#	You should have received a copy of the GNU General Public License
#	along with OPNODE; if not, write to the Free Software
#	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#	
#	
#	Last changes:
#
#	25/10/12 by Jose Luis Galindo: Create cosm-uploader as a new process
#	01/05/12 by Jose Luis Galindo: first version.
#
#***************************************************************************/
XAP=cosm

CC=gcc 
CFLAGS=-g -Wall -O2
LFLAGS=-lxml2 -lxap -lcosm -llog -lprocess
INCLUDES=-I/usr/include/libxml2 -I../../../libs
LIBS=-L../../../libs/libxap -L../../../libs/libcosm -L../../../libs/liblog -L../../../libs/libprocess
OBJECTS= 

xap-$(XAP): shared.o
	$(CC) -o xap-$(XAP)-updater $(XAP)-updater.c $(INCLUDES) $(LIBS) $(LFLAGS) shared.o
	$(CC) -o xap-$(XAP) $(XAP).c $(INCLUDES) $(LIBS) $(LFLAGS) shared.o
	mv xap-$(XAP) ../bin/xap-$(XAP)
	mv xap-$(XAP)-updater ../bin/xap-$(XAP)-updater

shared.o: shared.c shared.h
	$(CC) -c shared.c $(INCLUDES) $(LIBS) $(CFLAGS)
	
clean:
	rm -f xap-$(XAP) *.o ../bin/xap-$(XAP)
	rm -f xap-$(XAP)-updater ../bin/xap-$(XAP)-updater
