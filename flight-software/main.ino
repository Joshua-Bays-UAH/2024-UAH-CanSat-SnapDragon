#include <Wire.h>
#include <SoftwareSerial.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <BMP388_DEV.h>

#include <string.h>
#include <utility/imumaths.h>
#define BNO055_SAMPLERATE_DELAY_MS (100)

#define TEAM_ID 2079
#define PacketSpeed 990 /* Send a packet every 1000ms */
const char Modes[2][12] = {"FLIGHT", "SIMULATION"};
const char States[6][12] = {"LAUNCH_WAIT", "ASCENT", "SEPARATE", "DESCENT", "HS_RELEASE", "LANDED"};
char mission_time[16] = "";
unsigned packet_count = 0;
unsigned mode = 0;
unsigned state = 0;
float altitude = 0;
float air_speed = 0;
char hs_deployed = 'N';
char pc_deployed = 'N';
float temperature = 0;
float voltage = 0;
float pressure = 0;
char gps_time[16] = "";
float gps_altitude = 0;
float gps_latitude = 0;
float gps_longitude = 0;
unsigned gps_sats = 0;
float tilt_x = 0;
float tilt_y = 0;
float rot_z = 0;
char cmd_echo[64] = "";
float velocity = 0;

char packet[512];

float GroundAltitude = 0;
float pa = 0;

long packetTimer;
long veloTimer;

#define AscentAlt 4
#define SeparateAltitude 8 /* Altitude to separate from rocket (Should be 700) */
#define HRAltitutde 3 /* Altitude to release heat shield (Should be 100) */
#define LandAlt 1 /* Altitude to determine if landed (Should be 5) */

BMP388_DEV bmp388; /* BMP 388 sensor */
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28); /* BNO055 sensor*/

SoftwareSerial SD(1, 0); /* OpenLog SD Card */


void setup(void){
	/* If Gabe's PCB is used, the following 3 lines are needed */
	/* Wire.begin(); */
	/* Wire.setSDA(8); */
	/* Wire.setSCL(9); */

	/* Attempt to open Serial line */
	Serial.begin(9600);
	for(int i = 0; i < 10 && !Serial; i++){
		delay(150);
	}
	Serial.println("Serial connected.");

	SD.begin(9600);
	for(int i = 0; i < 10 && !SD; i++){
		delay(150);
	}
	SD.println("Serial connected.");

	for(int i = 0; i < 10 && !bno.begin(); i++){
		Serial.println("BNO not connected!");
		delay(150);
	}
	Serial.println("BNO connected.");
	
	/* delay(1000); */

	bno.setExtCrystalUse(true);

	for(int i = 0; i < 10 && !bmp388.begin(); i++){
		Serial.println("BMP not connected!");
		delay(150);
	}
	Serial.println("BMP connected.");
	bmp388.startForcedConversion();

	packetTimer = millis();
	veloTimer = millis();
}

void loop(void){
	sensors_event_t event;
	bno.getEvent(&event);
	uint8_t sys, gyro, accel, mag = 0;
	bno.getCalibration(&sys, &gyro, &accel, &mag);
	/* delay(BNO055_SAMPLERATE_DELAY_MS); */

	bmp388.startForcedConversion();
	bmp388.getMeasurements(temperature, pressure, altitude);

	if(GroundAltitude == 0 && altitude > 0){ GroundAltitude = altitude; }

	tilt_x = event.orientation.x;
	tilt_y = event.orientation.y;
	rot_z = event.orientation.z;

	if(state == 0 && altitude-GroundAltitude >= AscentAlt){
	ChangeAscent:
		state++;
		Serial.println(States[state]);
	}else if(state == 1 && altitude-GroundAltitude >= SeparateAltitude){
	ChangeSeparate:
		state++;
		Serial.println(States[state]);
	}else if(state == 2 && velocity <= -.5){
	ChangeDescent:
		state++;
		Serial.println(States[state]);
	}else if(state == 3 && altitude-GroundAltitude <= HRAltitutde){
	ChangeHRelease:
		state++;
		Serial.println(States[state]);
	}else if(state == 4 && altitude-GroundAltitude <= LandAlt){
	ChangeLanded:
		state++;
		Serial.println(States[state]);
	}

	/* Send telemetry */
	if(millis() - packetTimer >= PacketSpeed){
		velocity = (altitude - pa);
		sprintf(packet, "%u,%f,%f,%f,%f,%f,%f, %f, %s", TEAM_ID, tilt_x, tilt_y, rot_z, temperature, pressure, altitude-GroundAltitude, velocity, States[state]);
		/* sprintf(packet, "%u,%s,%u,%s,%s,%f,%f,%c,%c,%f,%f,%f,%s,%f,%f,%f,%u,%f,%f,%f,%s,,%f", TEAM_ID, mission_time[16], packet_count, Modes[mode], States[state], altitude, air_speed, hs_deployed, pc_deployed, temperature, voltage, pressure, gps_time[16], gps_altitude, gps_latitude, gps_longitude, gps_sats, tilt_x, tilt_y, rot_z, cmd_echo[64], velocity); */
		Serial.println(packet);
		packetTimer = millis();
	}
}

