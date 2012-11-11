xAP Current Cost connector from Homected
========================================

This application was based on the "HAH livebox current cost connector" from Brett England (www.DBzoo.com).

With this versión some improvements were introduced to work for the Homected project on Raspberry Pi hardware. 

The old code from Brett England is also included.


How it works:
=============

The application first load the xAP settings from the xap-cc.xml file. After that setup the xAP interface and listen for incoming xAP BSC Queries or wait for messages from the CurrentCost monitor through the serial connection.

The messages from the CurrentCost will be broadcasting to the xAP network if the sensor is enabled and when the hysteresis condition is given.


Setup:
======

It's possible to run this code as an application or as a process. It depends of the next line at the start of the code in cc.c file

//#define DEBUG_APP

For compile as an application uncomment this define, otherwise keep it uncommented.

To run as a process, compile with the makefile of the same folder of this readme file. Remember to make as a root user. 

To compile this code as an application use the make file from the /src folder. The compiled binary file is moved to the /bin folder, use this file to run as an application.


Settings:
=========

The xAP settings can be loaded from the xap-cc.xml file. In cc.h you can set the path to this file.

To setup the Xml settings file, run for the first time the application and it will create a xml file with default settings. The xml file is like this:

<?xml version="1.0" encoding="ISO-8859-1"?>
<settings>
	<device>
		<name>RaspberryPi</name>
		<enabled>1</enabled>
		<ccmodel>0</ccmodel>
		<ccserial>/dev/ttyUSB0</ccserial>
		<temphyst>0.1</temphyst>
		<temploc>Temp</temploc>
		<tempname>Monitor</tempname>
		<tempunits>C</tempunits>
		<tempdesc>CC Temp monitor</tempdesc>
	</device>
	<xap>
		<iface>eth0</iface>
		<version>12</version>
		<hbeatf>60</hbeatf>
		<infof>120</infof>
		<wservf>300</wservf>
		<uid>8003</uid>
		<port>3639</port>
	</xap>
	<sensors>
		<sensor name="0">
			<enabled>1</enabled>
			<digital>0</digital>
			<hyst>0</hyst>
			<location>Hall</location>
			<name>TotalEnergy</name>
			<units>Watts</units>
			<description>Consumo de energia total</description>
		</sensor>
		.
		.
		.
		<sensor name="9">
		..
		</sensor>
	</sensors>
</settings>

Settings description:
* Device - name: Instance name for xAP address
* Device - enabled: Enable xAP listening and upload data to Cosm
* Device - ccmodel: 0=CC128, 1=CLASSIC, 2=ORIGINAL
* Device - ccserial: Serial port where CurrentCost is connected
* Device - temphyst: Hysteresis value for temperature monitoring
* Device - temploc: Location name for temperature monitoring
* Device - tempname: Instance name for temperature monitoring
* Device - tempunits: Units for temperature monitoring (C/F)
* Device - tempdesc: Description for temparature monitoring endpoint
* xap - iface: Network interface name
* xap - version: xAP version to use (12 = v1.2 or 13 = v1.3)
* xap - hbeatf: Interval for xAP heartbeat messages
* xap - wservf: Interval for xAP web service messages (0=disable)
* xap - uid: xAP Unique IDentifier
* xap - port: Port to use in xAP communications
* sensors - sensor id - enabled: Enable upload data for this sensor
* sensors - sensor id - digital: Set this sensor as a digital type
* sensors - sensor id - hyst: Hysteresis value that trigger a xap message
* sensors - sensor id - location: Location name for the sensor
* sensors - sensor id - name: Instance name for the sensor
* sensors - sensor id - units: Units of the measurement of this sensor
* sensors - sensor id - description: Description for the sensor


Usage:
======

You can use this commands to start/stop if you compile as a process:

/etc/init.d/xap-cc start

or 

/etc/init.d/xap-cc stop

But if you compile as an application run this command from the /bin folder

./xap-cc


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


Last revision: 
==============

17 MAy 2012 by Jose Luis Galindo (support@homected.com)
