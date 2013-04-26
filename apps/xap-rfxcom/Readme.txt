xAP RFXCOM connector from Homected
==================================

This application retrieves information from RFXCOM receivers and broadcast on xAP messages and to an interprocess message list with xap-X10.

This application is done for the Homected project with the Raspberry Pi hardware, but it can run standalone.


How it works:
=============

At startup the application first load the xAP settings from the xap-rfxcom.xml file and listen the RFXCOM device in variable length mode.

When a signal is receive from the RFXCOM interface, the connector decode it and build a xAP message to broadcast over the network.

For X10RF commands, the connector can send a message directly to the xap-X10 process through IPC.


Device and sensors supported:
=============================

At this moment these are the devices and sensor supported in this version of xAP-RFXCOM:
- Library X10RF: X10RF devices, BMB HOME DM10 Outdoor motion sensor.
- Library SECURITY: Visonic (PowerCode sensors and SecureCode keyfobs), Regent MS20 Motion sensor, Marmitek DS10 and DS90 Window/door sensors, Marmitek MS90 motion sensor.
- Library OREGON: Oregon Scientific sensors, for a complete list see \docs\Oregon supported devices.htm file.
- Library HOMEEASY: Home Easy devices.
- Library KOPPLA: IKEA Koppla remote.

Setup:
======

It's possible to run this code as an application or as a process. It depends of the next line at the start of the code in rfxcom.c file

//#define DEBUG_APP

For compile as an application uncomment this define, otherwise keep it uncommented.

To run as a process, compile with the makefile of the same folder of this readme file. Remember to make as a root user. 

To compile this code as an application use the make file from the /src folder. The compiled binary file is moved to the /bin folder, use this file to run as an application.


Settings:
=========

The xAP settings can be loaded from the xap-rfxcom.xml file. In rfxcom.h you can set the path to this file.

To setup the Xml settings file, run for the first time the application and it will create a xml file with default settings. The xml file is like this:

<?xml version="1.0" encoding="ISO-8859-1"?>
<settings>
	<device>
		<name>RaspberryPi</name>
		<enabled>1</enabled>
		<commport>/dev/ttyUSB0</commport>
		<x10_location>Hall</x10_location>
		<x10_name>Rfxcom</x10_name>
		<disable_x10>0</disable_x10>
		<disable_oregon>0</disable_oregon>
		<disable_arc>0</disable_arc>
		<disable_visonic>0</disable_visonic>
		<disable_ati>0</disable_ati>
		<disable_koppla>0</disable_koppla>
		<disable_heasyUK>0</disable_heasyUK>
		<disable_heasyEU>0</disable_heasyEU>
		<disable_x10_xapbsc_sch>0</disable_x10_xapbsc_sch>
		<disable_x10_xapx10_sch>0</disable_x10_xapx10_sch>
		<disable_x10_msgqueue>0</disable_x10_msgqueue>
	</device>
	<xap>
		<iface>eth0</iface>
		<version>12</version>
		<hbeatf>60</hbeatf>
		<infof>120</infof>
		<wservf>300</wservf>
		<uid>8007</uid>
		<port>3639</port>
	</xap>
	<sensors>
		<sensor-x10 address="B13">
			<enabled>1</enabled>
			<devcaps>3</devcaps>
			<location>Bedroom</location>
			<name>Lamp</name>
		</sensor-x10>
		.
		.
		<sensor-security address="4D4061">
			<enabled>1</enabled>
			<devcaps>3</devcaps>
			<location>Bedroom</location>
			<name>Window</name>
		</sensor-security>
		.
		.
		<sensor-oregon address="0500A61">
			<enabled>1</enabled>
			<devcaps>3</devcaps>
			<devconfig>0</devconfig>
			<location>Bedroom</location>
			<name>Window</name>
		</sensor-oregon>
		.
		.
		<sensor-homeeasy address="ABCDEF01">
			<enabled>1</enabled>
			<location>LivingRoom</location>
			<name>Lamp</name>
		</sensor-homeeasy>
		.
		.
		<sensor-koppla address="10">
			<enabled>1</enabled>
			<location>LivingRoom</location>
			<name>MainLamp</name>
		</sensor-koppla>
		.
		.
	</sensors>
</settings>

Settings description:
* Device - name: Instance name for xAP address
* Device - enabled: Enable xAP listening
* Device - commport: Serial port where RFXCOM is connected
* Device - x10_location: Location for xAP-X10 messages
* Device - x10_name: Name for xAP-X10 messages
* Device - disable_x10: Disable receive X10RF messages
* Device - disable_oregon: Disable receive Oregon Scientific messages
* Device - disable_arc: Disable receive ARC messages
* Device - disable_visonic: Disable receive VISONIC messages
* Device - disable_ati: Disable receive ATI Wonder messages
* Device - disable_koppla: Disable receive KOPPLA messages
* Device - disable_heasyUK: Disable receive HomeEasy UK messages
* Device - disable_heasyEU: Disable receive HomeEasy EU messages
* Device - disable_x10_xapbsc_sch: Disable xAP-BSC schema for X10RF
* Device - disable_x10_xapx10_sch: Disable xAP-X10 schema for X10RF
* Device - disable_x10_msgqueue: Disable transmission of X10RF messages to X10RF message queue
* xap - iface: Network interface name
* xap - version: xAP version to use (12 = v1.2 or 13 = v1.3)
* xap - hbeatf: Interval for xAP heartbeat messages
* xap - infof: Interval for xAP info messages
* xap - wservf: Interval for xAP web service messages (0=disable)
* xap - uid: xAP Unique IDentifier
* xap - port: Port to use in xAP communications
* sensors - sensor-x10 address - enabled: Enable this sensor
* sensors - sensor-x10 address - devcaps: Device capabilites of this sensor (see details below)
* sensors - sensor-x10 address - location: Location for xAP-BSC messages
* sensors - sensor-x10 address - name: Name for xAP-BSC messages
* sensors - sensor-security address - enabled: Enable this sensor
* sensors - sensor-security address - devcaps: Device capabilites of this sensor (see details below)
* sensors - sensor-security address - location: Location for xAP-BSC messages
* sensors - sensor-security address - name: Name for xAP-BSC messages
* sensors - sensor-oregon address - enabled: Enable this sensor
* sensors - sensor-oregon address - devcaps: Device capabilites of this sensor (see details below)
* sensors - sensor-oregon address - devconfig: Device configuration for this sensor (see details below)
* sensors - sensor-oregon address - location: Location for xAP-BSC messages
* sensors - sensor-oregon address - name: Name for xAP-BSC messages
* sensors - sensor-homeeasy address - enabled: Enable this sensor
* sensors - sensor-homeeasy address - location: Location for xAP-BSC messages
* sensors - sensor-homeeasy address - name: Name for xAP-BSC messages
* sensors - sensor-koppla address - enabled: Enable this sensor
* sensors - sensor-koppla address - location: Location for xAP-BSC messages
* sensors - sensor-koppla address - name: Name for xAP-BSC messages
* sensors - sensor-rfxcom address - enabled: Enable this sensor
* sensors - sensor-rfxcom address - devcaps: Device capabilites of this sensor (see details below)
* sensors - sensor-rfxcom address - devconfig: Device configuration for this sensor (see details below)
* sensors - sensor-rfxcom address - location: Location for xAP-BSC messages
* sensors - sensor-rfxcom address - name: Name for xAP-BSC messages


Capabilities for X10RF device (devcaps) used in xAP-BSC squema

  Bit  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
		|  |  |  |  |  | | | | | | | | | | |______ Accept On / Off commands
	  	|  |  |  |  |  | | | | | | | | | |________ Accept Dim / Bright commands
		|  |  |  |  |  | | | | | | | | |__________ Accept All Lights Off
		|  |  |  |  |  | | | | | | | |____________ Accept All Lights On
		|  |  |  |  |  | | | | | | |______________ Accept All Units Off
		|  |  |  |  |  | | | | | |________________ Accept Status request
		|  |  |  |  |  | | | | |__________________ Accept Extended Code
		|  |  |  |  |  | | | |____________________ Accept Preset Dim (not used)
		|  |  |  |  |  | | |______________________ Has memory function for value
        |  |  |  |  |  | |________________________ Can OFF from DIM
		|  |  |  |  |  |__________________________ Function from DIM bit 0
		|  |  |  |  |_____________________________ Function from DIM bit 1
		|  |  |  |________________________________ Function from BRIGHT bit 0
		|  |  |___________________________________ Function from BRIGHT bit 1
		|  |______________________________________ Not set
		|_________________________________________ Not set

	Table for "Function from DIM / BRIGHT":

	Bit: 	1	0	Action
	======================
			0	0	Nothing done from DIM / BRIGHT
			0	1	DIM / BRIGHT from recalled value
			1	0	Set to lowest level from DIM / BRIGHT
			1	1	Set to highest level from DIM / BRIGHT									


Capabilities for SECURITY (Visonic & X10SEC) devices (devcaps) used in xAP-BSC squema

  Bit  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
		|  |  |  |  |  | | | | | | | | | | |______ Has Alert contact
	  	|  |  |  |  |  | | | | | | | | | |________ Detect Tamper open/close
		|  |  |  |  |  | | | | | | | | |__________ Detect Battery low
		|  |  |  |  |  | | | | | | | |____________ Has Disarm
		|  |  |  |  |  | | | | | | |______________ Has Arm away
		|  |  |  |  |  | | | | | |________________ Has Arm home
		|  |  |  |  |  | | | | |__________________ Has Panic
		|  |  |  |  |  | | | |____________________ Has Lights on
		|  |  |  |  |  | | |______________________ Has Lights off		


Capabilities for OREGON Scientific devices (devcaps) used in xAP-BSC squema

  Bit  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
		|  |  |  |  |  | | | | | | | | | | |______ Detect battery low
	  	|  |  |  |  |  | | | | | | | | | |________ Measure temperature
		|  |  |  |  |  | | | | | | | | |__________ Measure humidity
		|  |  |  |  |  | | | | | | | |____________ Measure pressure (barometer)
		|  |  |  |  |  | | | | | | |______________ Measure rain fall rate
		|  |  |  |  |  | | | | | |________________ Measure total rain
		|  |  |  |  |  | | | | |__________________ Measure wind direction
		|  |  |  |  |  | | | |____________________ Measure wind speed
		|  |  |  |  |  | | |______________________ Measure wind average speed
		|  |  |  |  |  | |________________________ Measure UV factor
		|  |  |  |  |  |__________________________ Measure weight
		|  |  |  |  |_____________________________ Measure phase1
		|  |  |  |________________________________ Measure phase2
		|  |  |___________________________________ Measure phase3

Configuration for OREGON Scientific devices (devconfig) used in xAP-BSC squema

  Bit   7 6 5 4 3 2 1 0
		| | | | | | | |______ 0: Celsius		1: Fahrenheit		for temperature
	  	| | | | | | |________ 0: hPA			1: Bar				for pressure
		| | | | | |__________ 0: mm				1: inch				for rain
		| | | | |____________ 0: meters			1: milles			for wind speed
		| | | |______________ 0: m/s, milles/s 	1: km/h, milles/h	for wind speed
		| | |________________ 0: Kg				1: lb				for weight

Capabilities for RFXCOM devices (devcaps) used in xAP-BSC squema

  Bit  	7 6 5 4 3 2 1 0
		| | | | | | | |______ Measure temperature
		| | | | | | |________ Measure from AD Converter
		| | | | | |__________ Measure from supply voltage

Configuration for RFXCOM devices (devconfig) used in xAP-BSC squema

  Bit   7 6 5 4 3 2 1 0
		| | | | | | | |______ 0: Celsius		1: Fahrenheit		for temperature


Usage:
======

You can use this commands to start/stop if you compile as a process:

/etc/init.d/xap-rfxcom start

or 

/etc/init.d/xap-rfxcom stop

But if you compile as an application run this command from the /bin folder

./xap-rfxcom


Devices and sensors tested:
===========================

This devices and sensors are tested with this application:
- X10RF: Remote KR22E
- Visonic: MCT-302N
- Oregon Scientific: BWR102, THGR122N, THGN132N


License:
========

Copyright (c) Jose Luis Galindo, 2013.
   
No commercial use.
No redistribution at profit.
All derivative work must retain this message and
acknowledge the work of the original author.  

Please report defects to support@homected.com


Last revision: 
==============

03 Mar 2013 by Jose Luis Galindo (support@homected.com)
