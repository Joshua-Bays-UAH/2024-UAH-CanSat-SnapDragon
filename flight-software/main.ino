/*
Author: Joshua Bays
Project: 2023 UAH CanSat Team Snapdragon
*/

/* 
TODO
	1. Create flight mode 
	2. Create simulation mode
	3. Create way to transfer from flight to simulation
	4. Create command inputs
		1. CX
		2. ST
		3. SIM
		4. SIMP
		5. CAL
		6. BCN
	5. Create audio beacon
	6. Copy this code and make sensor data real
		1. Sensors
		2. Pins
*/

#include <SoftwareSerial.h>
#include <string>

#define TeamID "SNAP"
#define BuzzerPin -1 // UPDATE
#define LEDPin -1 // UPDATE

SoftwareSerial XBee(0, 1); // RX, TX
SoftwareSerial SD(12, 11); // RX, TX

#define ServoPin1 -1 // UPDATE
#define ServoPin2 -1 // UPDATE
#define ServoPin3 -1 // UPDATE
Sensor sensor1, sensor2; // MAKE REAL

#define ReleaseHeight 760
#define ParaHeight 100

unsigned long programTime;
unsigned baseTime;

unsigned long packetct = 0; // how many packets have been sent
char mode = 'f'; // flight mode | F: FLIGHT, S:SIMULATION
char state = 'W'; // software state | W: LAUNCH_WAIT, A: ASCENT, R: ROCKET_SEPARATION, D: DESCENT, H: HS_RELEASE, L: LANDED
float altitude; // current altitude
float airspeed; // current airspeed
char hs = 'N'; // heat shield | P: DEPLOYED, N: NOT_DEPLOYED
char pc = 'N'; // parachute | C: DEPLOYED, N: NOT_DEPLOYED
float temp; // current temperature
float voltage; // current voltage
float pressure; // current air pressure
std::string gpsTime; // current GPS time
float gpsAltitude; // current GPS altitude
float gpsCoords[2]; // current GPS latitude and longitude (in that order)
unsigned gpsStats; // current count of GPS connections
float gyro[3]; // current X and Y tilt, and Z rotation (in that order)
char cmdEcho; // last command received

_Bool cx = 1; // flag to send telemetry or not
_Bool bcn = 0; // flag to turn on and off the audio beacon

std::string packet;

void setup(){
	baseTime = get_baseTime();
	
	// Communication
	Serial.begin(9600); // Remove once finalized
	XBee.begin(9600);
	XBee.println("XBee connected");
	
	connect_sensor(sensor1); // MAKE REAL
	connect_sensor(sensor2); // MAKE REAL
	
	connect_sd(SD); // MAKE REAL
	
	connect_servo(servo1); // MAKE REAL
	connect_servo(servo2); // MAKE REAL
	connect_servo(servo3); // MAKE REAL
}

void loop(){
	// Update how long the program has been running
	programTime = millis();
	
	if(mode == 'F'){
		// Make decisions
		if(altitude >= AscentHeight && state == 'W'){ // Check to move into ascent
		AscentCmd:
			state = 'A';
		}
		if(altitude >= ReleaseHeight && state == 'A'){ // Check to move into release (and release)
		ReleaseCmd:
			spin_servo(servo1);
			state = 'R';
		}
	}else if(mode == 'S'){
		// Read packet
		packet = "";
		while(XBee.available()){ packet += (char)XBee.read(); }
		// Parse packet
	}
}

