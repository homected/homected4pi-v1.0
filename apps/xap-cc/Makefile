#**************************************************************************
#
#	Makefile
#
#	xap-cc makefile
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
#	28/04/12 by Jose Luis Galindo: first version.
#
#***************************************************************************/

.PHONY:  src

all: src copy_files chg_permissions

src:
	$(MAKE) -C $@

clean:
	$(MAKE) -C src $@
	sudo rm -f /usr/sbin/xap-cc
	sudo rm -f /etc/init.d/xap-cc

copy_files:
	sudo mv bin/xap-cc /usr/sbin/
	sudo cp xap-cc /etc/init.d/
	test -e /var/www/xap/connectors/xap-cc/images || mkdir -p  /var/www/xap/connectors/xap-cc/images
	cp src/wserv.png /var/www/xap/connectors/xap-cc/images/
	
chg_permissions:
	sudo chmod 775 /usr/sbin/xap-cc
	sudo chown root:root /usr/sbin/xap-cc
	sudo chmod 755 /etc/init.d/xap-cc
	sudo chown root:root /etc/init.d/xap-cc
	# Modify to match your serial port
	sudo chmod o+rw /dev/ttyUSB0
