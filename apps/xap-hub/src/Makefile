#**************************************************************************
#
#	Makefile
#
#	xaphub makefile
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
#	along with HOMECTED; if not, write to the Free Software Foundation,
#	Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#	
#	
#	Last changes:
#
#	10/04/12 by Jose Luis Galindo: first version.
#
#***************************************************************************/

XAP=hub

CC=gcc 
CFLAGS=-g -O2
LFLAGS=-lxml2 -llog -lprocess
INCLUDES=-I/usr/include/libxml2 -I../../../libs
LIBS=-L../../../libs/liblog -L../../../libs/libprocess

xap-$(XAP): shared.o
	$(CC) -o xap-$(XAP) $(XAP).c $(INCLUDES) $(LIBS) $(LFLAGS) shared.o
	test -e ../bin || mkdir -p  ../bin
	mv xap-$(XAP) ../bin/xap-$(XAP)
	
shared.o: shared.c shared.h
	$(CC) -c shared.c $(INCLUDES) $(LIBS) $(CFLAGS)

clean:
	rm -f xap-$(XAP) *.o ../bin/xap-$(XAP)
