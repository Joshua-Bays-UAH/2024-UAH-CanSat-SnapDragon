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
*/

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_LSM6DSOX.h>
#include <107-Arduino-Servo-RP2040.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <string>
#include <string.h>
#include <time.h>
#include "defs.hpp"

#include "setup.ino"
#include "loop.ino"

Adafruit_LSM6DSOX sox;
Adafruit_BMP3XX bmp;

SoftwareSerial XBee(0, 1); // RX, TX
SoftwareSerial SD(12, 11); // RX, TX

static _107_::Servo servo_r, servo_p;

unsigned long programTime;
unsigned baseTime;

unsigned long packetct = 0; // how many packets have been sent
char mode = 'f'; // flight mode | F: FLIGHT, S:SIMULATION
char state = 'W'; // software state | W: LAUNCH_WAIT, A: ASCENT, R: ROCKET_SEPARATION, D: DESCENT, H: HS_RELEASE, L: LANDED
float altitude; // current altitude
float oldAlt; // previous altitude, will be used for velocity
float velocity; //
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

_Bool bcnOn = 0;

std::string packet;
char packetBuff[128];

unsigned releaseTimer;
