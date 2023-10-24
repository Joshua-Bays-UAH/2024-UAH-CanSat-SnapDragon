#include <SoftwareSerial.h>

SoftwareSerial XBee(0, 1); // RX, TX

void setup(){
	Serial.begin(9600); // Remove once finalized
	XBee.begin(9600);
	XBee.println("XBee connected");
}
