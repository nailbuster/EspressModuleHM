// espEmail.h

#ifndef _ESPEMAIL_h
#define _ESPEMAIL_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

/*
esp8266 Email & Text Messge
by Ben Lipsey www.varind.com 2016. This code is public domain, enjoy!
github: http://github.com/varind/esp8266-SMTP
Convert your user/pass to base 64:
https://www.base64decode.org

Based on sketch by Erni
http://www.esp8266.com/viewtopic.php?f=32&t=6139#p32186
original sketch:
http://playground.arduino.cc/Code/Email?action=sourceblock&num=2
Email client sketch for IDE v1.0.5 and w5100/w5200
Posted 7 May 2015 by SurferTim
*/




#include <ESP8266WiFi.h>



class espSendMailClass {

protected:		
		byte eRcv(WiFiClient client);
		boolean debugSerial = true;
public:
		espSendMailClass();  //constructor

		// Email Settings
	String server = "mail.yourserver.com";		// mail.yourMailServer.com
	int port = 25;								// your outgoing port; my server uses 25
	String user = "user";						// base64, ASCII encoded user
	String pass = "pass";						// base64, ASCII encoded password
	String from = "your@email.com";				// your@email.com
	String to = "their@email.com";				// their@email.com
	String myName = "Your Name";				// Your name to display
	String subject = "esp8266 Test";			// email subject
	String message = " ";						//"This message is from your esp8266.\nIt works!";
	//-------------------------------------
	String Status = "";
	String ReturnMsg = "";

	byte sendEmail();

};


//-See more at : http ://www.esp8266.com/viewtopic.php?f=32&t=6139&start=12#sthash.Zqs86CHI.dpuf



#endif

