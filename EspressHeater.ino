/*
Espress Module for HeaterMeter for esp8266

Copyright (c) 2016 David Paiva (david@nailbuster.com). All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "MQTTLink.h"
#include <SoftwareSerial.h>
#include <TimeLib.h>
#include "ThingSpeak.h"
#include "globals.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <FS.h>
#include <ArduinoJson.h> 
#include <myWebServer.h>
#include <PubSubClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <esp8266AVRFlash.h>



void setup() {
	
	Serial.begin(HM_COM_BAUDRATE);
	Serial.println("starting ESP");

	MyWebServer.begin();
	ThingSpeak.begin();
	MQTTLink.begin();
	HMGlobal.begin();

}

void loop() {
	
	MyWebServer.handle();
	if (MyWebServer.OTAisflashing==false  && MyWebServer.isDownloading==false)  //don't do anything during flashing OTA/downloading
	{
		ThingSpeak.handle();
		MQTTLink.handle();
		HMGlobal.handle();  //checks for serial msgs from HM
	}

  	//Delay to allow ESP8266 WIFI functions to run
  	delay(10);
}


/*
TODO
FLASH AVR adding/testing
json get values for offline graphing....
error checking/reconnection when cloud/internet services down...
*/
