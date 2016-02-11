/*
MQTTLink.h Espress Connection MQTT for esp8266

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

#ifndef _MQTTLINK_h
#define _MQTTLINK_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class MQTTLinkClass
{
 protected:


 public:
	 bool mqEnabled = false;
	 String mqServer;
	 int mqPort = 1883;
	 String mqUser;
	 String mqPassword;
	 String mqPubTopic;
	 String mqSubTopic;
	 int mqInterval;  //seconds
	 unsigned long lastMqttChk = millis();

	 void begin();  //loads from spiffs
	 void handle();

	 void StartMqtt();
	 void SubscribeMqtt();
	 void PublishMQTT();
	 void SendAlarm(String AlarmMsg);

};

extern MQTTLinkClass MQTTLink;

#endif

