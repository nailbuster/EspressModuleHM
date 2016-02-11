/*
ThingSpeak.h Espress Connection for esp8266

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


#ifndef _THINGSPEAK_h
#define _THINGSPEAK_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class ThingSpeakClass
{
 protected:
	 unsigned long lastThingSpeak;
	 unsigned long lastTalkBack;

 public:
	 ThingSpeakClass();

	 String thingSpeakURL;
	 String thingWriteKey;
	 int    thingInterval;  //in secs
	 String TalkBackID;
	 String TalkBackKey;
	 int    talkBackInterval;   //in secs
	 bool   ThingEnabled;
	 bool   TalkBackEnabled;
	 String ThingStatus;
	 bool inAlarm = false;
	 String AlarmInfo;

	 void begin();  //loads from spiffs
	 void handle(); 
	 void ProcessTalkBacks();
	 String getThingSpeak(String talkBackID, String talkApiKey);
	 void SendThingSpeakValues();
	 void SendAlarm(String AlarmMsg);
};

extern ThingSpeakClass ThingSpeak;

#endif

