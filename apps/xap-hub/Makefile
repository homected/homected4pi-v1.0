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

.PHONY:  src

all: src copy_files chg_permissions set_inittab

src:
	$(MAKE) -C $@

clean:
	$(MAKE) -C src $@
	rm -f /usr/sbin/xap-hub
	rm -f /etc/init.d/xap-hub
	update-rc.d -f xap-hub remove

copy_files:
	#
	# You need to be root to run this makefile correctly!!!
	# =====================================================
	#
	mv /home/homected/apps/xap-hub/bin/xap-hub /usr/sbin/
	cp xap-hub /etc/init.d/
	
chg_permissions:
	chmod 775 /usr/sbin/xap-hub
	chown root:root /usr/sbin/xap-hub
	chmod 755 /etc/init.d/xap-hub
	chown root:root /etc/init.d/xap-hub

set_inittab:
	update-rc.d xap-hub defaults 81
