#include <SoftwareSerial.h>

SoftwareSerial XBee(0, 1); // RX, TX

Sensor sensor1, sensor2;

void setup(){
	// Communication
	Serial.begin(9600); // Remove once finalized
	XBee.begin(9600);
	XBee.println("XBee connected");
	
	connect_sensor(sensor1);
	connect_sensor(sensor2);
}

void loop(){

}
