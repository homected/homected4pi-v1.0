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
#	along with HOMECTED; if not, write to the Free Software Foundation,
#	Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#	
#	
#	Last changes:
#
#	25/10/12 by Jose Luis Galindo: Create cosm-uploader as a new process
#	07/05/12 by Jose Luis Galindo: first version.
#
#***************************************************************************/

.PHONY:  src

all: src copy_files chg_permissions 

src:
	$(MAKE) -C $@

clean:
	$(MAKE) -C src $@
	sudo rm -f /usr/sbin/xap-cosm
	sudo rm -f /usr/sbin/xap-cosm-updater
	sudo rm -f /etc/init.d/xap-cosm

copy_files:
	sudo mv bin/xap-cosm /usr/sbin/
	sudo mv bin/xap-cosm-updater /usr/sbin/
	sudo cp xap-cosm /etc/init.d/
	test -e /var/www/xap/connectors/xap-cosm/images || mkdir -p /var/www/xap/connectors/xap-cosm/images
	cp src/wserv.png /var/www/xap/connectors/xap-cosm/images/
	
chg_permissions:
	sudo chmod 775 /usr/sbin/xap-cosm
	sudo chmod 775 /usr/sbin/xap-cosm-updater
	sudo chown root:root /usr/sbin/xap-cosm
	sudo chown root:root /usr/sbin/xap-cosm-updater
	sudo chmod 755 /etc/init.d/xap-cosm
	sudo chown root:root /etc/init.d/xap-cosm
