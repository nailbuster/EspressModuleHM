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

#include "globals.h"
#include <myWebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "ThingSpeak.h"
#include "MQTTLink.h"
#include <FS.h>
#include <ArduinoJson.h> 
#include <esp8266AVRFlash.h>
#include <htmlEmbed.h>
#include <TimeLib.h>
#include "espEmail.h"


GlobalsClass HMGlobal;


#define SoftSerial   //define if using Software serial on 10,11; 
//#define HardSerial   //define if using Software serial on 10,11;  
//#define HardSerialSwap    //swap tx/rx pins...only used if hardserial....
#ifdef SoftSerial
#include <SoftwareSerial.h>
SoftwareSerial qCon(4, 5, false, 128);
#endif

#ifdef HardSerial
HardwareSerial &qCon = Serial;   //we use espSerial to communicate to blynk through cmdMessenger;
#endif


void  FlashHM() {  //server request to flash avr file to HM...file exists on spiffs	
	if (!MyWebServer.isAuthorized()) return;

	String fname = "";	
	if (server.hasArg("fname")) { fname=server.arg("fname"); }

	if (fname == "") return server.send(200, "text/html", "Flashing File NOT FOUND");;
	DebugPrintln("FLashing :" + server.arg("fname"));
	MyWebServer.OTAisflashing = true;	
	//delay(200);
#ifdef SoftSerial
	qCon.enableRx(true);
#endif
	qCon.flush();	
//	delay(10);
	qCon.begin(115200);  //HM speed for flashing with optiboot	
	//qCon.begin(Esp8266AVRFlash.AVR_BAUDRATE);
//	Esp8266AVRFlash.AVR_PAGESIZE = 256;
	Esp8266AVRFlash.FlashAVR(&qCon, "/"+fname);  //flashAVR HM
	qCon.flush();  	
	server.send(200, "text/html", "Flashing AVR....please wait...will auto-reboot...do NOT touch system!!!");
	delay(2000);
	ESP.restart(); //restart ESP after reboot.....
}

//NOTE: Authorization is not necessary to access the HM status
void sendHMJsonweb() {
	
	String postStr = "{";
	postStr += "\"time\":" + String(now()) + ",";
	postStr += "\"set\":" + HMGlobal.hmSetPoint + ",";
	postStr += "\"lid\":" + HMGlobal.hmLidOpenCountdown + ",";
  
	postStr += "\"fan\":{";
	postStr += "\"c\":" + HMGlobal.hmFan + ",";
	postStr += "\"a\":" + HMGlobal.hmFanMovAvg; //+ ",";
	//postStr += "\"f\":0"; //TODO: Add "f" and uncomment comma in line above
	postStr += "},";
	

	//TODO: Add "adc"
	//...
 
	postStr += "\"temps\":[";
	for (int i=0; i<4; i++)
	{
		postStr += "{";
		postStr += "\"n\":\"" + HMGlobal.hmProbeName[i] + "\",";
		
		String t = (HMGlobal.hmProbeTemp[i] == "U") ? "null" : HMGlobal.hmProbeTemp[i];
		postStr += "\"c\":" + t + ",";
    
		//postStr += "\"dph\":0,"; //TODO: Calculate and update dph (degrees per hour)
		postStr += "\"a\":{\"l\":" + HMGlobal.hmAlarmLo[i] + ",\"h\":" + HMGlobal.hmAlarmHi[i] + ",\"r\":" + HMGlobal.hmAlarmRinging[i] + "}";

		if (i <= 2) postStr += "},";
	} 
	postStr += "}]";
  
	postStr += "}";

	server.send(200, "application/json", postStr);	
}


void  setHMweb() {	    ///hm/set?do=settemp&setpointf=225
	bool isOK = false;
	if (!MyWebServer.isAuthorized()) return;
	if (server.arg("do") == "settemp") {
		if (server.arg("setpointf") != "") {
			int nt=String(server.arg("setpointf")).toInt(); 
			if (nt > 0) {
				HMGlobal.SetTemp(nt);
				server.send(200, "text/html", "Temp Set to : " + String(nt));
				isOK = true;
			}
		}
	}
	if (server.arg("do") == "setalarm") {    //hm/set?do=setalarm&alarms=10,10,20,20,30,30,40,40
		if (server.arg("alarms") != "") {
			String al = server.arg("alarms");
			if (al.length() > 0) {
				HMGlobal.ConfigAlarms(al);
				server.send(200, "text/html", "Alarms Setup Sent");
				isOK = true;
			}
		}
	}
	if (!isOK) { server.send(200, "text/html", "invalid request"); }
}

void JsonSaveCallback(String fname)  ///this is the callback funtion when the webserver saves a json file....we check which one and do things....
{
	if (fname == "/heatgeneral.json") HMGlobal.SendHeatGeneralToHM(fname);   //send HM general to serial port.
	else if (fname == "/heatprobes.json") HMGlobal.SendProbesToHM(fname);   //send HM probe info to serial port...
	else if (fname == "/cloudgen.json") ThingSpeak.begin();        //reload cloud settings
}



GlobalsClass::GlobalsClass()
{  //init vars
	hmSetPoint = "11";
	hmProbeTemp[0] = "U";
	hmProbeTemp[1] = "U";
	hmProbeTemp[2] = "U";
	hmProbeTemp[3] = "U";
	hmFan = "0";
	hmFanMovAvg = "0";
	hmLidOpenCountdown = "0";

	for (int i = 0; i<4; i++)
	{
		hmAlarmRinging[i] = "null";
		hmAlarmLo[i] = "-40";
		hmAlarmHi[i] = "200";
		hmProbeName[i] = "Probe" + String(i);
	}

}

void  GlobalsClass::SetTemp(int sndTemp)   //send temperature to HM via serial....
{	
	if (sndTemp>0)
		{
			qCon.println(String("/set?sp=") + String(sndTemp));
			qCon.println(String("/set?tt=Remote Temp,Set to ") + String(sndTemp));
			DebugPrintln(String("Setting Remote Temp ") + String(sndTemp));
			hmSetPoint = String(sndTemp);
		}
	
}


void sendPitDroidOK() {
	//if (!MyWebServer.isAuthorized()) return;
	//TODO  figure out how to get pitdroids user/password request and approve decline here...
	//DebugPrintln(server.args());
	//server.sendHeader("Set-Cookie", "sysauth=9b735437932ce9486dccc6345ec18a0b; path=/luci/;stok=fc9434a4f4c06b0ecd73486cc1eb1e29");
	//server.sendHeader("Set-Cookie", "sysauth=9b735437932ce9486dccc6345ec18a0b; path=/luci/");
	//server.sendHeader("Set-Cookie", "sysauth=9b735437932ce9486dccc6345ec18a0b;");
	//server.send(200, "text/html", "Set-Cookie:sysauth=9b735437932ce9486dccc6345ec18a0b; path=/luci/;stok=fc9434a4f4c06b0ecd73486cc1eb1e29");
	server.send(301);
	DebugPrintln("pitdroid authing");
}
void sendPitHistOK() {
	//if (!MyWebServer.isAuthorized()) return;
	 server.send(200, "text/html", "1405344600, 65, 94.043333333333, nan, 44.475555555556, 82.325555555556, 0\n");  //dummy data?
}


void getPitDroidSP() {
	//TODO to support linkmeter type request to change temp ex. PitDroid.
//	DebugPrintln("pitdroid temp set");
//	DebugPrintln(server.arg(0));
//	DebugPrintln(server.arg(1));
	
}


void testgz() {
//	server.sendHeader("Content-Encoding", "gzip");
//	server.send_P(200, "text/html", wifisetup_html_gz, sizeof(wifisetup_html_gz));
//	FileSaveContent_P("/testconfig.html.gz", wifisetup_html_gz, sizeof(wifisetup_html_gz),false);
}

void  GlobalsClass::begin()
{
	//serveron("/probesave", handleProbeSave);	
	//MyWebServer.ServerON("/test", &TestCallback);
    //MyWebServer.CurServer->on("/test", TestCallback);

	//Update globals from json file
	ReadProbesJSON("/heatprobes.json");


	server.on("/flashavr", FlashHM);
	server.on("/luci/lm/hmstatus", sendHMJsonweb);
	server.on("/luci/admin/lm", sendPitDroidOK); //approve login	
	server.on("/luci/admin/lm/hist", sendPitHistOK); //approve history	
	server.on("/luci/admin/lm/set", getPitDroidSP); //PitDroid SetTemp
	server.on("/hm/set", setHMweb);
	server.on("/testgz", testgz);	


	MyWebServer.jsonSaveHandle = &JsonSaveCallback;  //server on jsonsave file we hook into it to see which one and process....

#ifdef SoftSerial
#include <SoftwareSerial.h>
	qCon.begin(HM_COM_BAUDRATE);
	delay(20);
//	qCon.enableRx(true);
	delay(20);
#endif

#ifdef HardSerial
		#ifdef HardSerialSwap
			Serial.swap();  //toggle between use of GPIO13/GPIO15 or GPIO3/GPIO(1/2) as RX and TX
		#endif
#endif

	if (WiFi.status() == WL_CONNECTED)
	{
		// ... print IP Address to HM
		qCon.println("/set?tt=WiFi Connected,"+ (String)WiFi.localIP()[0] + "." + (String)WiFi.localIP()[1] + "." + (String)WiFi.localIP()[2] + "." + (String)WiFi.localIP()[3]);
	} else qCon.println("/set?tt=WiFi NOT, Connected!!"); 
	
}



void  GlobalsClass::SendHeatGeneralToHM(String fname) {   //sends general info to HM

	String values = "";
	String hmsg;
	File f = SPIFFS.open(fname, "r");	
	if (f) { // we could open the file 
		values = f.readStringUntil('\n');  //read json         
		f.close();
		
		//WRITE CONFIG TO HeaterMeter
		//fBuf(sbuf,"/set?sp=%iF",299);  //format command;
		DynamicJsonBuffer jsonBuffer;

		JsonObject& root = jsonBuffer.parseObject(values);  //parse weburl
		if (!root.success())
		{
			DebugPrintln("parseObject() failed");
			return;
		}
		//const char* sensor    = root["sensor"];
		//long        time      = root["time"];
		//double      latitude  = root["data"][0];
		//double      longitude = root["data"][1];           }

		//set PID                 

		qCon.println(String("/set?pidb=") + root["pidb"].asString()); delay(comdelay);
		qCon.println(String("/set?pidp=") + root["pidp"].asString()); delay(comdelay);
		qCon.println(String("/set?pidi=") + root["pidi"].asString()); delay(comdelay);
		qCon.println(String("/set?pidd=") + root["pidd"].asString()); delay(comdelay);

		//Set Fan info /set?fn=FL,FH,SL,SH,Flags,MSS,FAF,SAC 
		hmsg = String("/set?fn=") + root["minfan"].asString() + "," + root["maxfan"].asString() + "," + root["srvlow"].asString() + "," + root["srvhi"].asString() + "," + root["fanflg"].asString() + "," +
					     			root["maxstr"].asString() + "," + root["fanflr"].asString() + "," + root["srvcl"].asString();

		qCon.println(hmsg); delay(comdelay);
		DebugPrintln(hmsg);
		//Set Display props       
		hmsg = String("/set?lb=") + root["blrange"].asString() + "," + root["hsmode"].asString() + "," + root["ledcfg"].asString();
		qCon.println(hmsg); delay(comdelay);
		DebugPrintln(hmsg);
		//Set Lid props    
		hmsg = String("/set?ld=") + root["lidoff"].asString() + "," + root["liddur"].asString();
		qCon.println(hmsg); delay(comdelay);
		DebugPrintln(hmsg);
		qCon.println("/set?tt=Web Settings,Updated!!"); delay(comdelay);
		qCon.println("/save?"); delay(comdelay);

	}  //open file success

}



void GlobalsClass::SendProbesToHM(String fname) {   //sends Probes info to HM
	String values = "";
	String hmsg;
	File f = SPIFFS.open(fname, "r");
	if (f) { // we could open the file 
		values = f.readStringUntil('\n');  //read json         
		f.close();

		//WRITE CONFIG TO HeaterMeter

		DynamicJsonBuffer jsonBuffer;

		JsonObject& root = jsonBuffer.parseObject(values);  //parse json data
		if (!root.success())
		{
			DebugPrintln("parseObject() failed");
			return;
		}
		//const char* sensor    = root["sensor"];
		//long        time      = root["time"];
		//double      latitude  = root["data"][0];
		//double      longitude = root["data"][1];           }

		//NOTE: Need to update global probe names here (ReadProbesJSON function only called on start-up)
		hmProbeName[0] = root["p0name"].asString();
		hmProbeName[1] = root["p1name"].asString();
		hmProbeName[2] = root["p2name"].asString();
		hmProbeName[3] = root["p3name"].asString();

		qCon.println(String("/set?pn0=") + hmProbeName[0]); delay(comdelay);
		qCon.println(String("/set?pn1=") + hmProbeName[1]); delay(comdelay);
		qCon.println(String("/set?pn2=") + hmProbeName[2]); delay(comdelay);
		qCon.println(String("/set?pn3=") + hmProbeName[3]); delay(comdelay);

		//Set offsets
		hmsg = String("/set?po=") + root["p0off"].asString() + "," + root["p1off"].asString() + "," + root["p2off"].asString() + "," + root["p3off"].asString();
		qCon.println(hmsg); delay(comdelay);
		DebugPrintln(hmsg);

		//Set Probe coeff.
		hmsg = String("/set?pc0=") + root["p0a"].asString() + "," + root["p0b"].asString() + "," + root["p0c"].asString() + "," + root["p0r"].asString() + "," + root["p0trm"].asString();
		qCon.println(hmsg); delay(comdelay);
		DebugPrintln(hmsg);
		hmsg = String("/set?pc1=") + root["p1a"].asString() + "," + root["p1b"].asString() + "," + root["p1c"].asString() + "," + root["p1r"].asString() + "," + root["p1trm"].asString();
		qCon.println(hmsg); delay(comdelay);
		DebugPrintln(hmsg);
		hmsg = String("/set?pc2=") + root["p2a"].asString() + "," + root["p2b"].asString() + "," + root["p2c"].asString() + "," + root["p2r"].asString() + "," + root["p2trm"].asString();
		qCon.println(hmsg); delay(comdelay);
		DebugPrintln(hmsg);
		hmsg = String("/set?pc3=") + root["p3a"].asString() + "," + root["p3b"].asString() + "," + root["p3c"].asString() + "," + root["p3r"].asString() + "," + root["p3trm"].asString();
		qCon.println(hmsg); delay(comdelay);
		DebugPrintln(hmsg);

		//Set Alarm limits
		hmAlarmLo[0] = root["p0all"].asString();
		hmAlarmLo[1] = root["p1all"].asString();
		hmAlarmLo[2] = root["p2all"].asString();
		hmAlarmLo[3] = root["p3all"].asString();
		hmAlarmHi[0] = root["p0alh"].asString();
		hmAlarmHi[1] = root["p1alh"].asString();
		hmAlarmHi[2] = root["p2alh"].asString();
		hmAlarmHi[3] = root["p3alh"].asString();



		hmsg = String("/set?al=") + HMGlobal.hmAlarmLo[0] + "," + HMGlobal.hmAlarmHi[0] + "," + HMGlobal.hmAlarmLo[1] + "," + HMGlobal.hmAlarmHi[1] + "," + HMGlobal.hmAlarmLo[2] + "," + HMGlobal.hmAlarmHi[2] + "," + HMGlobal.hmAlarmLo[3] + "," + HMGlobal.hmAlarmHi[3];
		qCon.println(hmsg); delay(comdelay);
		DebugPrintln(hmsg);


		qCon.println("/set?tt=Web Settings,Updated!!"); delay(comdelay);
		qCon.println("/save?"); delay(comdelay);

	}  //open file success

}


void GlobalsClass::ReadProbesJSON(String fname) {
	String values = "";
	String hmsg;
	File f = SPIFFS.open(fname, "r");
	if (f) { // we could open the file 
		values = f.readStringUntil('\n');  //read json         
		f.close();

		DynamicJsonBuffer jsonBuffer;

		JsonObject& root = jsonBuffer.parseObject(values);  //parse json data
		if (!root.success())
		{
			DebugPrintln("parseObject() failed");
			return;
		}

		hmProbeName[0] = root["p0name"].asString();
		hmProbeName[1] = root["p1name"].asString();
		hmProbeName[2] = root["p2name"].asString();
		hmProbeName[3] = root["p3name"].asString();

		hmAlarmLo[0] = root["p0all"].asString();
		hmAlarmLo[1] = root["p1all"].asString();
		hmAlarmLo[2] = root["p2all"].asString();
		hmAlarmLo[3] = root["p3all"].asString();
		hmAlarmHi[0] = root["p0alh"].asString();
		hmAlarmHi[1] = root["p1alh"].asString();
		hmAlarmHi[2] = root["p2alh"].asString();
		hmAlarmHi[3] = root["p3alh"].asString();

	}  //open file success

}


void GlobalsClass::handle()
{
	if (qCon.available() > 0) checkSerialMsg();

	//alarm time checking
	if (ResetTimeCheck > 0) {
		if (millis() - ResetTimeCheck > (ResetAlarmSeconds * 1000)) { ResetAlarms(); }   //check to see if we've past resetalarm;
	}
}


String GlobalsClass::getValue(String data, int index)
{
	char separator = ',';
	int stringData = 0; //variable to count data part nr 
	String dataPart = ""; //variable to hole the return text

	for (int i = 0; i <= data.length() - 1; i++) { //Walk through the text one letter at a time
		if (data[i] == separator) {
			//Count the number of times separator character appears in the text
			stringData++;
		}
		else if (stringData == index) {
			//get the text when separator is the rignt one
			dataPart.concat(data[i]);
		}
		else if (stringData>index) {
			//return text and stop if the next separator appears - to save CPU-time
			return dataPart;
			break;
		}
	}
	//return text if this is the last part
	return dataPart;
}


boolean validatechksum(String msg)
{  //NMEA 0183 format
	
	String tstmsg = msg.substring(1, msg.length() - 3);
	String inCHK = msg.substring(msg.length() - 2, msg.length());
	char cs='\0'; //chksum

	if (msg.charAt(msg.length() - 3) != '*') { return false; }
	
	
	for (int fx = 0; fx < tstmsg.length(); fx++)
	{
		cs ^= tstmsg.charAt(fx);
	}	
	if (cs == (int)strtol(inCHK.c_str(), NULL, 16)) {
		return true;
	}
	else { 
	DebugPrintln("*** Serial MSG CHKSUM FAILED ***");
	MyWebServer.ServerLog("CF:CHKSUM Failed" );
	return false;
	}
}




void  GlobalsClass::checkSerialMsg()
{

	String msgStr = qCon.readStringUntil('\n');
	DebugPrintln("received :" + msgStr);		

	if ((getValue(msgStr, 0) == "$HMSU")) //msg is good updatemsg
	{
		if (validatechksum(msgStr) == false) {
			return;
		}
		
		hmSetPoint = getValue(msgStr, 1); if (hmSetPoint == "U") hmSetPoint = "0";
		hmProbeTemp[0] = getValue(msgStr, 2);  //if (hmProbeTemp[0] == "U") hmProbeTemp[0] = "0";
		hmProbeTemp[1] = getValue(msgStr, 3);  //if (hmProbeTemp[1] == "U") hmProbeTemp[1] = "0";
		hmProbeTemp[2] = getValue(msgStr, 4);  //if (hmProbeTemp[2] == "U") hmProbeTemp[2] = "0";
		hmProbeTemp[3] = getValue(msgStr, 5);  //if (hmProbeTemp[3] == "U") hmProbeTemp[3] = "0";
		hmFan = getValue(msgStr, 6);     // if (hmFan == "U") hmFan = "0";
		hmFanMovAvg = getValue(msgStr, 7); //if (hmFanMovAvg == "U") hmFanMovAvg = "0";
		hmLidOpenCountdown = getValue(msgStr, 8);//	if (hmLidOpenCountdown == "U") hmLidOpenCountdown = "0";
	}
	else if ((getValue(msgStr, 0) == "$HMAL")) //Alarm is firing....
	{
		if (validatechksum(msgStr) == false) return;
		String AlarmInfo;
		bool HasAlarm;

		AlarmInfo = "Pit Alarm! : ";
		HasAlarm = false;
		int msgpos = 1;
		for (int i = 0; i < 4; i++) {
			bool ringing = false;
			String AlarmLo;
			String AlarmHi;
			AlarmLo = getValue(msgStr, msgpos);
			if (AlarmLo.charAt(AlarmLo.length() - 1) == 'L')
			{
				AlarmLo.remove(AlarmLo.length() - 1, 1);
				AlarmInfo += "Probe " + String(i + 1) + " Low:  " + AlarmLo + " ! ";
				HasAlarm = true;
				hmAlarmRinging[i] = "\"L\"";
				ringing = true;
			}
			msgpos += 1;
			AlarmHi = getValue(msgStr, msgpos);
			if (AlarmHi.charAt(AlarmHi.length() - 1) == 'H')
			{
				AlarmHi.remove(AlarmHi.length() - 1, 1);
				AlarmInfo += "Probe " + String(i + 1) + " Hi:  " + AlarmHi + " ! ";
				HasAlarm = true;
				hmAlarmRinging[i] = "\"H\"";
				ringing = true;
			}

			if (ringing == false)
			{
				hmAlarmRinging[i] = "null";
			}

			msgpos += 1;
		}  //for each probe, check alarms
		//reset alarms
		if (ResetTimeCheck > 0) { HasAlarm = false; }  //if we're already in alarm countdown, ignore alarm....
		if (HasAlarm)	{
			if (ResetAlarmSeconds > 0) { ResetTimeCheck = millis(); }
			else { ResetTimeCheck = 0; }   //reset alarm in x Seconds.
		MQTTLink.SendAlarm(AlarmInfo);
		ThingSpeak.SendAlarm(AlarmInfo);		
		}		
	} else  if ((getValue(msgStr, 0) == "$QCAL")) //Alarm was fired (one time)....QControl  $QCAL,1,Low,CheckTemp,CurTemp     ($QCAL, probe number, Low or High, alarmtemp, curtemp)
	{
		if (validatechksum(msgStr) == false) return;
		String AlarmInfo;
		
		AlarmInfo = "Pit Alarm! : ";
		AlarmInfo += "Probe " + getValue(msgStr, 1) + " : " + getValue(msgStr, 2) + " ! Temp:" + getValue(msgStr, 4) + "(" + getValue(msgStr, 3) + ")";
		MQTTLink.SendAlarm(AlarmInfo);
		ThingSpeak.SendAlarm(AlarmInfo);
	}
	
	
}

void  GlobalsClass::ResetAlarms()
{
	ResetTimeCheck = 0;
	qCon.println("/set?al=0,0,0,0,0,0,0,0"); delay(comdelay);
	
}


void  GlobalsClass::ConfigAlarms(String msgStr)
{   //format is $ALARM,10,20,30,40,50,60,70,80   (lo/hi pairs);  send to comport;
	msgStr.replace("$ALARM,", "");  //remove the alarm command and send rest to HM
	qCon.println("/set?al="+msgStr); delay(comdelay);
	DebugPrintln("setting new alarms " + msgStr);
	qCon.println("/set?tt=Web Alarms,Updated.."); delay(comdelay);
}

