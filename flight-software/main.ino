/*
Author: Joshua Bays
Project: 2023 UAH CanSat Team Snapdragon
*/

/* 
TODO
	1. Create flight mode
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

#define TeamID "2079"

#define XBeeRX 0
#define XBeeTX 1

#define SDRX 9
#define SDTX 8

#define BuzzerPin 10
#define LEDPin 11

#define ReleasePin 12
#define ParaPin 13

#define BHigh digitalWrite(BuzzerPin, HIGH)
#define BLow digitalWrite(BuzzerPin, HIGH)

#define AscentHeight 5
#define ReleaseHeight 760
#define ParaHeight 100

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10
#define LSM_CS 10
#define LSM_SCK 13
#define LSM_MISO 12
#define LSM_MOSI 11
#define SEALEVELPRESSURE_HPA (1013.25)


Adafruit_LSM6DSOX sox;
Adafruit_BMP3XX bmp;

SoftwareSerial XBee(0, 1); // RX, TX
SoftwareSerial SD(12, 11); // RX, TX

static _107_::Servo servo_r, servo_p;

unsigned long programTime;
unsigned baseTime;

unsigned long packetct = 0; // how many packets have been sent
char mode = 'F'; // flight mode | F: FLIGHT, S:SIMULATION
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
char cmdEcho[128]; // last command received

_Bool cx = 1; // flag to send telemetry or not
_Bool bcn = 0; // flag to turn on and off the audio beacon

_Bool bcnOn = 0;
_Bool sEnable = 0;

std::string packet;
char packetBuff[128];

unsigned releaseTimer;

void setup(){
	baseTime = get_baseTime();
	
	// Communication
	Serial.begin(9600); // Remove once finalized
	XBee.begin(9600);
	XBee.println("XBee connected");

	// SD card
	SD.begin(9600);
	
	while(!sox.begin_I2C()){ delay(10); XBee.println("LSM!"); }
	XBee.println("LSM6DSOX connected");
	
	XBee.print("Accelerometer range set to: ");
	switch(sox.getAccelRange()){
		case LSM6DS_ACCEL_RANGE_2_G: XBee.println("+-2G"); break;
		case LSM6DS_ACCEL_RANGE_4_G: XBee.println("+-4G"); break;
		case LSM6DS_ACCEL_RANGE_8_G: XBee.println("+-8G"); break;
		case LSM6DS_ACCEL_RANGE_16_G: XBee.println("+-16G"); break;
	}

	XBee.print("Gyro range set to: ");
	switch(sox.getGyroRange()){
		case LSM6DS_GYRO_RANGE_125_DPS: XBee.println("125 degrees/s"); break;
		case LSM6DS_GYRO_RANGE_250_DPS: XBee.println("250 degrees/s"); break;
		case LSM6DS_GYRO_RANGE_500_DPS: XBee.println("500 degrees/s"); break;
		case LSM6DS_GYRO_RANGE_1000_DPS: XBee.println("1000 degrees/s"); break;
		case LSM6DS_GYRO_RANGE_2000_DPS: XBee.println("2000 degrees/s"); break;
		case ISM330DHCX_GYRO_RANGE_4000_DPS: break; //unsupported range for the DSOX
	}

	XBee.print("Accelerometer data rate set to: ");
	switch(sox.getAccelDataRate()){
		case LSM6DS_RATE_SHUTDOWN: XBee.println("0 Hz"); break;
		case LSM6DS_RATE_12_5_HZ: XBee.println("12.5 Hz"); break;
		case LSM6DS_RATE_26_HZ: XBee.println("26 Hz"); break;
		case LSM6DS_RATE_52_HZ: XBee.println("52 Hz"); break;
		case LSM6DS_RATE_104_HZ: XBee.println("104 Hz"); break;
		case LSM6DS_RATE_208_HZ: XBee.println("208 Hz"); break;
		case LSM6DS_RATE_416_HZ: XBee.println("416 Hz"); break;
		case LSM6DS_RATE_833_HZ: XBee.println("833 Hz"); break;
		case LSM6DS_RATE_1_66K_HZ: XBee.println("1.66 KHz"); break;
		case LSM6DS_RATE_3_33K_HZ: XBee.println("3.33 KHz"); break;
		case LSM6DS_RATE_6_66K_HZ: XBee.println("6.66 KHz"); break;
	}

	XBee.print("Gyro data rate set to: ");
	switch(sox.getGyroDataRate()){
		case LSM6DS_RATE_SHUTDOWN: XBee.println("0 Hz"); break;
		case LSM6DS_RATE_12_5_HZ: XBee.println("12.5 Hz"); break;
		case LSM6DS_RATE_26_HZ: XBee.println("26 Hz"); break;
		case LSM6DS_RATE_52_HZ: XBee.println("52 Hz"); break;
		case LSM6DS_RATE_104_HZ: XBee.println("104 Hz"); break;
		case LSM6DS_RATE_208_HZ: XBee.println("208 Hz"); break;
		case LSM6DS_RATE_416_HZ: XBee.println("416 Hz"); break;
		case LSM6DS_RATE_833_HZ: XBee.println("833 Hz"); break;
		case LSM6DS_RATE_1_66K_HZ: XBee.println("1.66 KHz"); break;
		case LSM6DS_RATE_3_33K_HZ: XBee.println("3.33 KHz"); break;
		case LSM6DS_RATE_6_66K_HZ: XBee.println("6.66 KHz"); break;
	}

	XBee.println("Adafruit BMP388 / BMP390 test");
	if(!bmp.begin_I2C()){ XBee.println("Could not find a valid BMP3 sensor, check wiring!"); }

	// Set up oversampling, filter initialization, and ground elevation
	bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
	bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
	bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
	bmp.setOutputDataRate(BMP3_ODR_50_HZ);
	while(!bmp.performReading()){ XBee.println("Failed to perform reading"); }
	for(unsigned i = 0; i < 100; i++){ GroundElevation = bmp.readAltitude(SEALEVELPRESSURE_HPA); } XBee.println("Ground Elevation Set");

	// Print out packet format
	packet = "TEAM_ID,MISSION_TIME,PACKET_COUNT,SW_STATE, PL_STATE,ALTITUDE, TEMP, VOLTAGE, GPS_LATITUDE, GPS_LONGITUDE, GYRO_R,GYRO_P,GYRO_Y";
	for(unsigned i = 0; i < packet.size(); i++){ XBee.print(packet[i]);} XBee.println();

	pinMode(BuzzerPin, OUTPUT); pinMode(LEDPin, OUTPUT);
	packetTimer = time(0);

	oldAlt = 0; velocity = 0;
}

void loop(){
	// Update how long the program has been running
	programTime = millis();

	if(mode == 'F'){
		if(bcn){BcnCmd:
			digitalWrite(BuzzerPin, bcnOn ? HIGH : LOW);
			digitalWrite(LEDPin, bcnOn ? HIGH : LOW);
			bcnOn = !bcnOn;
		}
	}else if(mode == 'S'){ReadPacketCmd:
		// Read packet
		XBee.readBytesUntil('\0', packetBuff, sizeof(packetBuff));
		// Parse packet
	}
	// Make decisions
	if(altitude >= AscentHeight && state == 'W'){AscentCmd: // Check to move into ascent
		state = 'A';
	}
	if(altitude >= ReleaseHeight && state == 'A'){ReleaseCmd: // Check to move into release (and release)
		//spin_servo(servo1);
		state = 'R';
	}
	if(state == 'R'){DescentCmd: // 
		
	}
	if(state == 'D'){HSCmd:
		
	}
	if(state == 'H'){LandCmd:
		
	}

	if(XBee.available){
		XBee.readBytesUntil('\0', packetBuff, sizeof(packetBuff));
		if(strncmp(packetBuff, "CMD,2079,", 9) == 0){
			if(strncmp(packetBuff+9, "CX,", 3) == 0){
				if(strncmp(packetBuff+12, "ON", 2) == 0){
					cx = 1;
				}else if(strncmp(packetBuff+12, "OFF", 3) == 0){
					cx = 0;
				}
			}else if(strncmp(packetBuff+9, "ST", 2) == 0){
				gpsTime = "00:00:00";
			}else if(strncmp(packetBuff+9, "SIM", 3) == 0){
				if(strncmp(packetBuff+13, "ENABLE", 6) == 0){
					sEnable = 1;
				}else if(strncmp(packetBuff+13, "ACTIVATE", 8) == 0 && sEnable){
					state = 'S';
				}else if(strncmp(packetBuff+13, "DISABLE", 7)){
					state = 'F';
					sEnable = 0;
				}
			}else if(strncmp(packetBuff+9, "SIMP", 4) == 0 && state == 'S'){
				pressure = std::stof(packetBuff+10);
			}else if(strncmp(packetBuff+9, "CAL", 3) == 0){
				GroundElevation = altitude;
			}else if(strncmp(packetBuff+9, "BCN", 3) == 0){
				if(strncmp(packetBuff+12, "ON", 2){
					bcn = 1;
				}else if(strncmp(packetBuff+12, "OFF", 3){
					bcn = 0;
				}
			}
		}
	}

	if(cx){SendPacketCmd:
		
	}
}


