#include <SoftwareSerial.h>
#include <string>
#define TeamID "SNAP" #define BuzzerPin -1
#define LEDPin -1

SoftwareSerial XBee(0, 1); // RX, TX
SoftwareSerial SD(12, 11); // RX, TX

#define ServoPin1 -1
#define ServoPin2 -1
#define ServoPin3 -1
Sensor sensor1, sensor2;

unsigned long programTime;
unsigned baseTime;

unsigned long packetct = 0;
char mode = 'f'; char state = 0;
float altitude; float airspeed;
char hs = 0; char pc = 0;
float temp; float pressure; float voltage;
std::string gpsTime;
float gpsAltitude;
float gpsLatitude; float gpsLongitude;
float tiltX; float tiltY; float rotX;
char cmdEcho;

std::string packet;

void setup(){
	baseTime = get_baseTime();
	
	// Communication
	Serial.begin(9600); // Remove once finalized
	XBee.begin(9600);
	XBee.println("XBee connected");
	
	connect_sensor(sensor1);
	connect_sensor(sensor2);
	
	connect_sd(SD);
	
	connect_servo(servo1);
	connect_servo(servo2);
	connect_servo(servo3);
}

void loop(){
	programTime = millis();
	
	if(mode == 'f'){
		
	}else if(mode == 's'){
		packet = "";
		while(XBee.available()){ packet += (char)XBee.read(); }
		// Parse packet
	}
}

