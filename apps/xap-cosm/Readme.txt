xAP Cosm connector from Homected
================================

This application is based on the "HAH livebox Pachube connector" from Brett England (www.DBzoo.com) and use the base of PACHULIB (http://sourceforge.net/projects/pachulib/) to build an improved library updated to Cosm (Old Pachube) API v2.

This application is done for the Homected project with the Raspberry Pi hardware, but it can run standalone.

The old source code of Pachulib and from the HAH livebox Pachube connector is also included.


How it works:
=============

At startup the application first load the xAP settings from the xap-cosm.xml file. After that, it setup the xAP interface and listen for incoming xAP messages, if the message received is from one of the xAP sources from the settings file and match the class and body section defined in the settings file, it copy the value from the defined key and put in to a queue with the timestamp. The connector send regulary to Cosm the queue with the data from the endpoints.


Setup:
======

It's possible to run this code as an application or as a process. It depends of the next line at the start of the code in cosm.c and cosm-uploader.c files

//#define DEBUG_APP

For compile as an application uncomment this define, otherwise keep it uncommented.

To run as a process, compile with the makefile of the same folder of this readme file. Remember to make as a root user. 

To compile this code as an application use the make file from the /src folder. The compiled binary file is moved to the /bin folder, use this file to run as an application.


Settings:
=========

The xAP settings can be loaded from the xap-cosm.xml file. In cosm.h you can set the path to this file.

To setup the Xml settings file, run for the first time the application and it will create a xml file with default settings. You can add up to 255 datastreams. The xml file is like this:

<?xml version="1.0" encoding="ISO-8859-1"?>
<settings>
	<device>
		<name>RaspberryPi</name>
		<enabled>1</enabled>
		<apikey>put-your-api-key-here</apikey>
		<updatef>60</updatef>
	</device>
	<xap>
		<iface>eth0</iface>
		<version>12</version>
		<hbeatf>60</hbeatf>
		<wservf>300</wservf>
		<uid>8004</uid>
		<port>3639</port>
	</xap>
	<datastreams>
		<datastream>
			<enabled>1</enabled>
			<feed>put-your-cosm-feed-here</feed>
			<id>put-your-datastream-here</id>
			<xapsource>Homected.xAP-CurrentCost.RaspberryPi:sensor.0.ch.1</xapsource>
			<xapclass>xAPBSC.event</xapclass>
			<xapsection>input.state</xapsection>
			<xapkey>text</xapkey>
		</datastream>
		<datastream>
		.
		.
		.
		</datastream>
	</datastreams>
</settings>

Settings description:
* Device - name: Instance name for xAP address
* Device - enabled: Enable xAP listening and upload data to Cosm
* Device - apikey: Put here your Cosm API key
* Device - updatef: Interval to update data to Cosm
* xap - iface: Network interface name
* xap - version: xAP version to use (12 = v1.2 or 13 = v1.3)
* xap - hbeatf: Interval for xAP heartbeat messages
* xap - wservf: Interval for xAP web service messages (0=disable)
* xap - uid: xAP Unique IDentifier
* xap - port: Port to use in xAP communications
* datastreams - datastream - enabled: Enable upload data for this datastream
* datastreams - datastream - feed: Feed of the datastream
* datastreams - datastream - id: ID of the datastream to upload data
* datastreams - datastream - xapsource: xAP source to match to upload datapoints
* datastreams - datastream - xapclass: xAP class to match to upload datapoints
* datastreams - datastream - xapsection: xAP section to match to upload datapoints
* datastreams - datastream - xapkey: Key for capture the value and upload as a datapoint


Usage:
======

You can use this commands to start/stop if you compile as a process:

/etc/init.d/xap-cosm start

or 

/etc/init.d/xap-cosm stop

But if you compile as an application run this command from the /bin folder

./xap-cosm
./xap-cosm-uploader


License:
========

Copyright (c) Jose Luis Galindo, 2012.
Copyright (c) Brett England, 2009.
   
No commercial use.
No redistribution at profit.
All derivative work must retain this message and
acknowledge the work of the original author.  

Please report defects to support@homected.com
Original code from Brett England attached in zip file.
Original code from Pachulib attached in zip file.


Last revision: 
==============

25 Oct 2012 by Jose Luis Galindo (support@homected.com)
