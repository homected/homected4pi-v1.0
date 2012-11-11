xAP SQLite connector from Homected
========================================

This application grabs the desired data to a SQLite database.

This application is done for the Homected project with the Raspberry Pi hardware, but it can run standalone.


How it works:
=============

At startup the application first load the xAP settings from the xap-sqlite.xml file. After that, it setup the xAP interface and listen for incoming xAP messages, if the message received is from one of the xAP sources from the settings file and match the class and body section defined in the settings file, it copy the value from the defined key and add to a table in a sqlite database with the timestamp.


Setup:
======

It's possible to run this code as an application or as a process. It depends of the next line at the start of the code in sqlite.c file

//#define DEBUG_APP

For compile as an application uncomment this define, otherwise keep it uncommented.

To run as a process, compile with the makefile of the same folder of this readme file. Remember to make as a root user. 

To compile this code as an application use the make file from the /src folder. The compiled binary file is moved to the /bin folder, use this file to run as an application.


Settings:
=========

The xAP settings can be loaded from the xap-cosm.xml file. In cosm.h you can set the path to this file.

To setup the Xml settings file, run for the first time the application and it will create a xml file with default settings. You can add up to 255 datastreams. The xml file is like this:

<settings>
	<device>
		<name>RaspberryPi</name>
		<enabled>1</enabled>
	</device>
	<xap>
		<iface>eth0</iface>
		<version>12</version>
		<hbeatf>60</hbeatf>
		<wservf>300</wservf>
		<uid>8005</uid>
		<port>3639</port>
	</xap>
	<datastreams>
		<datastream>
			<enabled>1</enabled>
			<id>Temperature</id>
			<dbfile>dbtest.sl3</dbfile>
			<dbtable>logTemp</dbtable>
			<dbfieldname>TempValue</dbfieldname>
			<dbfieldtype>REAL</dbfieldtype>
			<xapsource>Homected.xAP-CurrentCost.RaspberryPi:Temp.Monitor.temp</xapsource>
			<xapclass>xAPBSC.event</xapclass>
			<xapsection>input.state</xapsection>
			<xapkey>text</xapkey>
		</datastream>
		<datastream>
			<enabled>1</enabled>
			<id>TotalEnergy</id>
			<dbfile>dbtest.sl3</dbfile>
			<dbtable>logEnergy</dbtable>
			<dbfieldname>TotalEnergyValue</dbfieldname>
			<dbfieldtype>INTEGER</dbfieldtype>
			<xapsource>Homected.xAP-CurrentCost.RaspberryPi:Hall.TotalEnergy.ch.1</xapsource>
			<xapclass>xAPBSC.event</xapclass>
			<xapsection>input.state</xapsection>
			<xapkey>text</xapkey>
		</datastream>
		.
		.
	</datastreams>
</settings>

Settings description:
* Device - name: Instance name for xAP address
* Device - enabled: Enable xAP listening and upload data to Cosm
* xap - iface: Network interface name
* xap - version: xAP version to use (12 = v1.2 or 13 = v1.3)
* xap - hbeatf: Interval for xAP heartbeat messages
* xap - wservf: Interval for xAP web service messages (0=disable)
* xap - uid: xAP Unique IDentifier
* xap - port: Port to use in xAP communications
* datastreams - datastream - enabled: Enable upload data for this datastream
* datastreams - datastream - id: ID of the datastream
* datastreams - datastream - dbfile: database file path
* datastreams - datastream - dbtable: Table of the database to use
* datastreams - datastream - dbfieldname: Field of the table where insert the values
* datastreams - datastream - dbfieldtype: Field type (INTEGER, REAL, TEXT or BLOB)
* datastreams - datastream - xapsource: xAP source to match to upload datapoints
* datastreams - datastream - xapclass: xAP class to match to upload datapoints
* datastreams - datastream - xapsection: xAP section to match to upload datapoints
* datastreams - datastream - xapkey: Key for capture the value and upload as a datapoint


Usage:
======

You can use this commands to start/stop if you compile as a process:

/etc/init.d/xap-sqlite start

or 

/etc/init.d/xap-sqlite stop

But if you compile as an application run this command from the /bin folder

./xap-sqlite


License:
========

Copyright (c) Jose Luis Galindo, 2012.
   
No commercial use.
No redistribution at profit.
All derivative work must retain this message and
acknowledge the work of the original author.  

Please report defects to support@homected.com


Last revision: 
==============

17 May 2012 by Jose Luis Galindo (support@homected.com)
