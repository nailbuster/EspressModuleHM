/*
AVRFlasher Optiboot for esp8266

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

THIS IS A WIP,  currently you must send the funcition a software servial already configured to correct speed of optiboot flashing...
AVR_RESETPIN 14 is the gpio pin used to reset AVR.
IT ONLY supports pure BIN files (no hex files yet).
doesn't verify pages yet,  upload all data and restarts....


*/

#ifndef _ESP8266AVRFLASH_h
#define _ESP8266AVRFLASH_h


#include <SoftwareSerial.h>


#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

//SETUP AVR ITEMS,  THIS CAN BE USED FOR NANO/MINI PRO
//#define AVR_PAGESIZE 128
//#define AVR_BAUDRATE 57600

//RESET ON D0/ RX D1 / TX D2 for nodemcu
//#define AVR_RESETPIN 2    //14 is D6,  2 is D4
//#define AVR_RXPIN  5       //softwareserial connection to AVR
//#define AVR_TXPIN  4

//SoftwareSerial AvrSerial(AVR_RXPIN, AVR_TXPIN, false, 128); // RX, TX

class Esp8266AVRFlashClass
{
 protected:
	 Stream * _AvrSerial;
	 bool WaitForAVROK();
	 void AVRReset();
	 void AVRLoadAddress(int addr);
	 void AVRProgramPage(uint8_t * data, int len);
	 void AVRReadPage(int len);
	 void AVRDisconnect();
	 bool AVRConnect();

 public:
	 int AVR_PAGESIZE = 128;
	 long AVR_BAUDRATE = 57600;
	 byte AVR_RESETPIN = 14;
	// byte AVR_RXPIN = 5;
	// byte AVR_TXPIN = 4;	

	bool isFlashing = false;         //simple method to see if we can flash (maybe flash in progress alreayd)
	bool FlashAVR(Stream *pSerial, String fname);  //send file that is be flashed from spiffs  (spiffs must be open already)....
	bool Hex2Bin(String hfile, String bfile);   //converts from spiffs a hexfile to binfile. not yet implemented
};

extern Esp8266AVRFlashClass Esp8266AVRFlash;

#endif

