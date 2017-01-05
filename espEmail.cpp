// 
// 
// 

#include "espEmail.h"
#include "base64.h"


espSendMailClass::espSendMailClass()
{

}


byte espSendMailClass::sendEmail()
{
	byte thisByte = 0;
	byte respCode;
	WiFiClient client;
	base64 bEn;

	if (client.connect(server.c_str(), port) == 1) {
		Serial.println(F("connected"));
	}
	else {
		Serial.println(F("connection failed"));
		return 0;
	}
	if (!eRcv(client)) return 0;

	if (debugSerial) Serial.println(F("Sending EHLO"));
	client.println("EHLO www.example.com");
	if (!eRcv(client)) return 0;
	if (debugSerial) Serial.println(F("Sending auth login"));
	client.println("auth login");
	if (!eRcv(client)) return 0;
	if (debugSerial) Serial.println(F("Sending User"));
	client.println(bEn.encode(user));
	if (!eRcv(client)) return 0;
	if (debugSerial) Serial.println(F("Sending Password"));
	client.println(bEn.encode(pass));
	if (!eRcv(client)) return 0;
	if (debugSerial) Serial.println(F("Sending From"));
	client.println(String("MAIL From: ") + from);
	if (!eRcv(client)) return 0;
	if (debugSerial) Serial.println(F("Sending To"));
	client.println(String("RCPT To: ") + to);
	if (!eRcv(client)) return 0;
	if (debugSerial) Serial.println(F("Sending DATA"));
	client.println(F("DATA"));
	if (!eRcv(client)) return 0;
	if (debugSerial) Serial.println(F("Sending email"));
	client.println(String("To:  ") + to);
	client.println(String("From: ") + myName + " " + from);
	client.println(String("Subject: ") + subject + "\r\n");
	client.println(message);

	client.println(F("."));
	if (!eRcv(client)) return 0;
	if (debugSerial) Serial.println(F("Sending QUIT"));
	client.println(F("QUIT"));
	if (!eRcv(client)) return 0;
	client.stop();
	if (debugSerial) Serial.println(F("disconnected"));
	return 1;
}

byte espSendMailClass::eRcv(WiFiClient client)
{
	byte respCode;
	byte thisByte;
	int loopCount = 0;

	while (!client.available()) {
		delay(1);
		loopCount++;
		// if nothing received for 10 seconds, timeout
		if (loopCount > 10000) {
			client.stop();
			if (debugSerial) Serial.println(F("\r\nTimeout"));
			return 0;
		}
	}

	respCode = client.peek();
	while (client.available())
	{
		thisByte = client.read();
		if (debugSerial) Serial.write(thisByte);
	}

	if (respCode >= '4') return 0;
	return 1;
}
