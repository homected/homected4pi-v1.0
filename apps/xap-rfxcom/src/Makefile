#**************************************************************************
#
#	Makefile
#
#	xap-rfxcom makefile
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
#	27/11/12 by Jose Luis Galindo: first version.
#
#***************************************************************************/
XAP=rfxcom

CC=gcc 
CFLAGS=-g -Wall -O2
LFLAGS=-lxml2 -lxap -llog -lprocess
INCLUDES=-I/usr/include/libxml2 -I../../../libs
LIBS=-L../../../libs/libxap -L../../../libs/liblog -L../../../libs/libprocess
OBJECTS= 

xap-$(XAP): shared.o X10RF.o Security.o Oregon.o homeeasy.o koppla.o rfxsensor.o
	$(CC) -o xap-$(XAP) $(XAP).c $(INCLUDES) $(LIBS) $(LFLAGS) shared.o X10RF.o Security.o Oregon.o homeeasy.o koppla.o rfxsensor.o
	test -e ../bin || mkdir -p  ../bin
	sudo mv xap-$(XAP) ../bin/xap-$(XAP)

shared.o: shared.c shared.h
	$(CC) -c shared.c $(INCLUDES) $(LIBS) $(CFLAGS)
	
X10RF.o: X10RF.c X10RF.h
	$(CC) -c X10RF.c $(INCLUDES) $(LIBS) $(CFLAGS)
	
Security.o: Security.c Security.h
	$(CC) -c Security.c $(INCLUDES) $(LIBS) $(CFLAGS)
	
Oregon.o: Oregon.c Oregon.h
	$(CC) -c Oregon.c $(INCLUDES) $(LIBS) $(CFLAGS)
	
homeeasy.o: homeeasy.c homeeasy.h
	$(CC) -c homeeasy.c $(INCLUDES) $(LIBS) $(CFLAGS)
	
koppla.o: koppla.c koppla.h
	$(CC) -c koppla.c $(INCLUDES) $(LIBS) $(CFLAGS)
	
rfxsensor.o: rfxsensor.c rfxsensor.h
	$(CC) -c rfxsensor.c $(INCLUDES) $(LIBS) $(CFLAGS)
	
clean:
	sudo rm -f xap-$(XAP) *.o ../bin/xap-$(XAP)
