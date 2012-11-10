xAP Hub connector from Homected
===============================

This application was based on the "xAP Compliant Hub" from Patrick Lidstone.

With this versión some improvements were introduced and to work for the Homected project on Raspberry Pi hardware.
The code has been rewritten and adapt to run as a process. The old code from Patrick Lidstone is also included.


How it works:
=============

The application first load the xAP settings from the xap-hub.xml file. After that discover the network settings (IP address, netmast, gateway) and setup several a socket to listen for incoming messages from any address at port 3639 by default, another socket to broadcast heartbeats to the network at broadcast    address (for example 192.168.1.255) and a third socket to send the incoming    messages from the network to the connected clients at address 127.0.0.1 with    the associated port for each client.

The received messages are filtered and relayed to the clients, also the messages from the clients are relayed to the other clients.


Setup:
======

Compile with supplied makefile (enter command make) from a root terminal. The makefile adds the links to allow xap-hub is started at boot time.

The xAP settings can be loaded from the xap-hub.xml file in /home/homected/config directory. 

To uninstall type "make clean" from a root terminal.


Usage:
======

Makefile is configured to add the links to allow to start the process at boot time, but if you need to start o stop the process type:

/etc/init.d/xap-hub start

or

/etc/init.d/xap-hub stop

If the process is running the file /var/run/xap-hub.pid exists otherwise the file don't exists.


License:
========

Copyright (c) Jose Luis Galindo, 2012.
Copyright (c) Patrick Lidstone, 2002.
   
No commercial use.
No redistribution at profit.
All derivative work must retain this message and
acknowledge the work of the original author.  

Please report defects to support@homected.com
Original code from patrick@lidstone.net attached in zip file.


Last revision: 
==============

10 March 2012 by Jose Luis Galindo (support@homected.com)
