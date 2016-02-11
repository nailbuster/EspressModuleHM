/*
AVRFlasher for esp8266

based on: https://github.com/mirobot/mirobot-wifi/blob/master/user/arduino.c

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

#include "esp8266AVRFlash.h"
#include <SoftwareSerial.h>
#include "FS.h"



bool Esp8266AVRFlashClass::Hex2Bin(String hfile, String bfile) {   //converts from spiffs a hexfile to binfile.  NOT implemented yet...need to find good example code...
	/*File Hx = SPIFFS.open(hfile, "r");
	if (!Hx) {
		Serial.println("file open failed " + fname);
		return false;
	}
	File Bn = SPIFFS.open(bfile, "w");

	String line;
	int fsize = Hx.size();
	
	line = Hx.readString();
	while (line != "")
	{
		char start_code = line.charAt(0)
			byte_count = int(line[1:3], 16)
			record_type = line[7:9]

			if start_code != ':' or ((byte_count * 2 + 11) != len(line)) :
				ErrorAndExit('broken ihex file')

				if record_type == '00' :
					for i in range(byte_count) :
						rawdata = rawdata + chr(int(line[9 + i * 2:11 + i * 2], 16))
						elif record_type == '01' :
						break
						elif record_type == '04' :
						continue
				else:
		//error
		line = Hx.readString();
	}
	hx.close();
	Bn.close();
	*/   
}


bool ICACHE_FLASH_ATTR Esp8266AVRFlashClass::WaitForAVROK()
{
	unsigned long startTime = millis();
	while (!_AvrSerial->available()) {
		if (millis() - startTime > 2000) {
			Serial.print("AVRtimeOUT");
			return false; //error, wait 3 seconds for response
		}
	//	yield();
	}; 
	String res = _AvrSerial->readStringUntil('\x10');
	if (!res == NULL) {
		Serial.print("got avr ok "); Serial.println((uint8_t)res[0]);
//		yield();
		return true;
	}
	else {		
		return false;
	}

}


void ICACHE_FLASH_ATTR Esp8266AVRFlashClass::AVRReset()
{
	pinMode(AVR_RESETPIN, OUTPUT);
	
	
	digitalWrite(AVR_RESETPIN, LOW);
	delay(200);     //hold off for 500ms
	
	digitalWrite(AVR_RESETPIN, HIGH);
	delay(300);     //wait 200ms before connect;
}




void ICACHE_FLASH_ATTR Esp8266AVRFlashClass::AVRLoadAddress(int addr) {
	// Send the "load address" command
	// U
	// addr >> 1 & 0xFF
	// (addr >> 9) & 0xFF
	// ' '
	uint8_t temp[] = "U\0\0 ";
	temp[1] = (addr >> 1) & 0xFF;
	temp[2] = (addr >> 9) & 0xFF;
	_AvrSerial->write(temp, 4);
	WaitForAVROK();
}




void ICACHE_FLASH_ATTR Esp8266AVRFlashClass::AVRProgramPage(uint8_t * data, int len) {
	// Send the "program page" command
	// d
	// len & 0xFF
	// (len >> 8) & 0xFF
	// 'F'
	// data
	// ' '
	uint8_t temp[] = "d\0\0F";
	temp[1] = (len >> 8) & 0xFF;
	temp[2] = len & 0xFF;
	_AvrSerial->write(temp, 4);
	_AvrSerial->write(data, len);
	_AvrSerial->write(" ");
	WaitForAVROK();
	Serial.println("wrote page");

}

void ICACHE_FLASH_ATTR Esp8266AVRFlashClass::AVRReadPage(int len) {
	// Send the "read page" command
	// t
	// len & 0xFF
	// (len >> 8) & 0xFF
	// 'F '
	uint8_t temp[] = "t\0\0F ";
	temp[1] = (len >> 8) & 0xFF;
	temp[2] = len & 0xFF;
	_AvrSerial->write(temp, 5);
	WaitForAVROK();
}




void ICACHE_FLASH_ATTR Esp8266AVRFlashClass::AVRDisconnect( ) {
	// Send the "connect" command
	// '0 '

	_AvrSerial->print("Q ");
	Serial.println("done programming");
	WaitForAVROK();

}




bool ICACHE_FLASH_ATTR Esp8266AVRFlashClass::AVRConnect() {
	//connect to AVR and enter programming mode (optiboot bootloader);

	_AvrSerial->print("1 ");
	if (!WaitForAVROK()) return false;

	_AvrSerial->print("0 ");
	if (!WaitForAVROK()) return false;

	_AvrSerial->print("0 ");
	if (!WaitForAVROK()) return false;

	_AvrSerial->print("P ");
	if (!WaitForAVROK()) return false;

	return true;
}




bool ICACHE_FLASH_ATTR Esp8266AVRFlashClass::FlashAVR(Stream *pSerial, String fname)
{
	//connect to serial
	_AvrSerial = pSerial;
	isFlashing= true;
	File f = SPIFFS.open(fname, "r");
	if (!f) {
		Serial.println("file open failed " + fname);
		isFlashing = false;
		return false;
	}

	
	_AvrSerial->flush();
	delay(10);

//	_AvrSerial->end();
//	_AvrSerial->begin(115200);		
    while (!_AvrSerial) { ; }


	//reset AVR
	AVRReset();	
	//start programming mode	
	AVRConnect();


	///this is where we flash AVR via serial connection (not spi)

	
	int curpage = 0;
	int curpos = 0;
	int binfilepos = 0;
	int numbytes = f.size();
	int numpages = numbytes / AVR_PAGESIZE;
	Serial.print("num pages"); Serial.println(numpages);
	Serial.print("num bytes"); Serial.println(numbytes);

	for (int pg = 0; pg < (numpages + 1); pg++)
	{
		uint8_t fbuf[AVR_PAGESIZE];
		int bufpos = 0;
		char binhex[3];
		bufpos=f.readBytes((char *) fbuf, AVR_PAGESIZE);				
		//Serial.print("read bytes "); Serial.println(bufpos);
		for (int j = 0; j < bufpos; j++)
		{
		//	Serial.print(String(fbuf[j]) + "-");
		}
		if (bufpos>0)  //stuffs to print
		{
			Serial.print("Page"); Serial.print(pg);
			Serial.print(" bytes"); Serial.println(bufpos);
			//delay(20);
			AVRLoadAddress(pg*AVR_PAGESIZE);
			AVRProgramPage(fbuf, bufpos);

		}
	} 

	AVRDisconnect();
	//reset AVR
	AVRReset();

	f.close(); //close file from spiffs
	_AvrSerial->flush();
	isFlashing = false;
}


Esp8266AVRFlashClass Esp8266AVRFlash;

