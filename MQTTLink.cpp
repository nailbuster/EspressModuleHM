/*
MQTTLink.cpp Espress Connection MQTT for esp8266

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
#include "globals.h"
#include <myWebServer.h>
#include <ArduinoJson.h> 
#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

WiFiClient wclient;
PubSubClient mqttclient(wclient);






MQTTLinkClass MQTTLink;


//mqTT stuffs

void callbackMQTT(const MQTT::Publish& pub) {     //callback when we get a msg from subtopics....
												  // handle message arrived
	DebugPrint(pub.topic());
	DebugPrint(" => ");
	/*if (pub.has_stream()) {
	uint8_t buf[100];
	int read;
	while (read = pub.payload_stream()->read(buf, 100)) {
	Serial.write(buf, read);
	}
	pub.payload_stream()->stop();
	DebugPrintln("");
	}
	else*/
	DebugPrintln(pub.payload_string());

	if (pub.topic() == MQTTLink.mqSubTopic + "/SetTemp")  //setTemp
	{
		int tbSetTemp;
		tbSetTemp = pub.payload_string().toInt();  //set RemoteTemp;
		if (tbSetTemp >= 1) HMGlobal.SetTemp(tbSetTemp);
	}
	if (pub.topic() == MQTTLink.mqSubTopic + "/SetAlarm")  //setTemp
	{
		HMGlobal.ConfigAlarms(pub.payload_string()); 
	}
}


void MQTTLinkClass::StartMqtt()
{
	if (mqEnabled == false) exit;
	mqttclient.set_server(mqServer, mqPort);
	mqttclient.set_callback(callbackMQTT);
	DebugPrintln("MQTT Started");

}


void MQTTLinkClass::SubscribeMqtt()    ///called in loop and enabled   example to test :  MyEspSub/SetTemp   and message is just the int value
{
	if (mqEnabled == false) exit;
	if (mqttclient.connected()) mqttclient.loop();

	if (WiFi.status() == WL_CONNECTED) {
		if (!mqttclient.connected()) {
			if (mqttclient.connect(MQTT::Connect("espHeater").set_auth(mqUser, mqPassword))) {
				mqttclient.subscribe(mqSubTopic + "/#");
				DebugPrintln("connected to MQTT");
			} 
		}
	}


}

void MQTTLinkClass::PublishMQTT()
{
	if (mqEnabled == false) exit;
	if (WiFi.status() == WL_CONNECTED) {
		if (mqttclient.connected()) {
			mqttclient.publish(mqPubTopic + "/SetTemp", HMGlobal.hmSetPoint);
			mqttclient.publish(mqPubTopic + "/PitTemp", HMGlobal.hmProbeTemp[0]);
			mqttclient.publish(mqPubTopic + "/Food1", HMGlobal.hmProbeTemp[1]);
			mqttclient.publish(mqPubTopic + "/Food2", HMGlobal.hmProbeTemp[2]);
			mqttclient.publish(mqPubTopic + "/Food3", HMGlobal.hmProbeTemp[3]);
			mqttclient.publish(mqPubTopic + "/Fan", HMGlobal.hmFan);
			mqttclient.publish(mqPubTopic + "/FanAvg", HMGlobal.hmFanMovAvg);
			mqttclient.publish(mqPubTopic + "/LidOpenCount", HMGlobal.hmLidOpenCountdown);			
		} //if mqtt connected
	}  //if wifi connected		
}

void MQTTLinkClass::SendAlarm(String AlarmMsg)
{
	if (mqEnabled == false) exit;
	if (WiFi.status() == WL_CONNECTED) {
		if (mqttclient.connected()) {
			mqttclient.publish(mqPubTopic + "/Alarm", AlarmMsg);
		}
	}
}





void MQTTLinkClass::begin() //loads settings from json file....
{
	String values = "";

	File f = SPIFFS.open("/cloudgen.json", "r");
	if (!f) {
		DebugPrintln("mqtt config not found");
	}
	else {  //file exists;
		values = f.readStringUntil('\n');  //read json         
		f.close();

		DynamicJsonBuffer jsonBuffer;

		JsonObject& root = jsonBuffer.parseObject(values);  //parse weburl
		if (!root.success())
		{
			DebugPrintln("parseObject() mqtt failed");
			return;
		}
		if (root["mqserver"].asString() != "") { //verify good json info                                                
			if (String(root["mqstatus"].asString()).toInt() == 1) mqEnabled = true; else mqEnabled = false;
			mqServer = root["mqserver"].asString();
			mqUser = root["mquser"].asString();
			mqPassword = root["mqpass"].asString();
			mqPubTopic = root["mqpub"].asString();
			mqSubTopic = root["mqsub"].asString();
			mqInterval = String(root["mqint"].asString()).toInt();
			mqPort = String(root["mqport"].asString()).toInt();
		}
	} //file exists;        

	if (mqEnabled) StartMqtt();
}

void MQTTLinkClass::handle()
{
	if (mqEnabled == false) return;
	
	SubscribeMqtt();

	unsigned long curTime = millis();

	//manage timers manually....more stable this way!
	if (curTime - lastMqttChk >= mqInterval * 1000) {   //mqtt stuffs
		PublishMQTT();
		lastMqttChk = millis();
	}

}



