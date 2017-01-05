/*
globals.h for esp8266 to communicate with HeaterMeter

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

#ifndef _GLOBALS_h
#define _GLOBALS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#define comdelay 200  //serial comdelay
#define HM_COM_BAUDRATE 38400

class GlobalsClass
{
 protected:

 public:
 String hmSetPoint;
 String hmFan;
 String hmFanMovAvg;
 String hmLidOpenCountdown;
 String hmProbeTemp[4];
 String hmProbeName[4];
 String hmAlarmRinging[4];
 String hmAlarmLo[4];
 String hmAlarmHi[4];


 int ResetAlarmSeconds = 10;  //number of seconds before we reset alarm....0 = means never....
 unsigned long ResetTimeCheck = 0; 
 GlobalsClass();
 void SetTemp(int sndTemp);
 void begin();
 void SendHeatGeneralToHM(String fname);
 void SendProbesToHM(String fname);
 void ReadProbesJSON(String fname);
 void handle();
 void checkSerialMsg();
 void ResetAlarms();
 void ConfigAlarms(String msgStr);
 String getValue(String data, int index);
};

extern GlobalsClass HMGlobal;

#endif

