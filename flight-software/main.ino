#include <Wire.h>
#include <SoftwareSerial.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <BMP388_DEV.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <107-Arduino-Servo-RP2040.h>

#include <string.h>
#include <utility/imumaths.h>
#include "defs.h"

int o = 0;

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

char packet[PacketLength];
char cmd[CmdLength]; char c;

float GroundAltitude = 0;
float pa = 0;

long packetTimer;
long veloTimer;
long landedTimer;

bool cx = 1;
bool bcn = 1;
bool simE = 0;
bool simA = 0;

BMP388_DEV bmp; /* BMP 388 sensor */
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28); /* BNO055 sensor*/
SFE_UBLOX_GNSS m8q; /* GPS sensor*/

static _107_::Servo releaseServo; /* Aerobrake release servo */
static _107_::Servo camServo; /* Aerobrake release servo */

SoftwareSerial XBee(0, 1); // RX, TX
SoftwareSerial SD(5, 4); /* OpenLog SD Card */

void setup(void){
	/* Attempt to open Serial line (testing purposes) */
	Serial.begin(9600);
	//while(!Serial){ delay(500); Serial.begin(9600); }

	/* If Gabe's PCB is used, the following 3 lines are needed */
	Wire.setSDA(8);
	Wire.setSCL(9);
	//Wire.begin();

	XBee.begin(9600);
	for(int i = 0; i < 10 && !XBee; i++){
		delay(150);
	}
	Serial.println("XBee connected.");

	SD.begin(9600);
	for(int i = 0; i < 10 && !SD; i++){
		SD.begin(9600);
		delay(150);
	}
	Serial.println("OpenLog connected.");
	for(int i = 0; i < 1 && !bno.begin(); i++){
		Serial.println("BNO not connected!");
		delay(150);
	}
	Serial.println("BNO connected.");
	bno.setExtCrystalUse(true);

	for(int i = 0; i < 10 && !bmp.begin(); i++){
		Serial.println("BMP not connected!");
		delay(150);
	}
	Serial.println("BMP connected.");
	bmp.startForcedConversion();

	for(int i = 0; i < 10 && !m8q.begin(); i++){
		Serial.println("M8Q not connected!");
		delay(150);
	}
	Serial.println("M8Q connected.");
	m8q.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
	
	releaseServo.attach(releaseServoPin);
	camServo.attach(camServoPin);
	
	pinMode(LEDPin, OUTPUT);
	pinMode(BuzzerPin, OUTPUT);
	pinMode(CamPin, OUTPUT);
	digitalWrite(CamPin, HIGH);
	delay(500);
	digitalWrite(CamPin, LOW);
	Serial.println("camera on");

	packetTimer = millis();
	veloTimer = millis();
}

void loop(void){
	sensors_event_t event;
	bno.getEvent(&event);
	uint8_t sys, gyro, accel, mag = 0;
	bno.getCalibration(&sys, &gyro, &accel, &mag);
	/* delay(BNO055_SAMPLERATE_DELAY_MS); */

	bmp.startForcedConversion();
	if(!simA){
		bmp.getMeasurements(temperature, pressure, altitude);
	}else{ pressure = -1; altitude = -1; }

	if(GroundAltitude == 0 && altitude > 0){ GroundAltitude = altitude; }

	tilt_x = event.orientation.x;
	tilt_y = event.orientation.y;
	rot_z = event.orientation.z;

	gps_latitude = m8q.getLatitude()/10000000;
	gps_longitude = m8q.getLongitude()/10000000;
	gps_altitude = m8q.getAltitude();
	sprintf(gps_time, "%i:%i:%i", m8q.getHour(), m8q.getMinute(), m8q.getSecond());

	if(state == 0 && altitude-GroundAltitude >= AscentAlt){
	ChangeAscent:
		state++;
	}else if(state == 1 && altitude-GroundAltitude >= SeparateAltitude){
	ChangeSeparate:
		state++;
	}else if(state == 2 && velocity <= -.5){
	ChangeDescent:
		state++;
	}else if(state == 3 && altitude-GroundAltitude <= HRAltitutde){
	ChangeHRelease:
		state++;
		releaseServo.writeMicroseconds(2000);
	}else if(state == 4 && altitude-GroundAltitude <= LandAlt){
	ChangeLanded:
		state++;
		cx = 0;
		landedTimer = millis();
		o = 1;
		digitalWrite(LEDPin, HIGH);
		digitalWrite(BuzzerPin, HIGH);
		digitalWrite(CamPin, HIGH);
		delay(5000);
		digitalWrite(CamPin, LOW);
	}
	camServo.writeMicroseconds((event.gyro.pitch/180)+1500);

	

	/* Command handling */
	if(XBee.available()){
		XBee.readBytesUntil('\0', cmd, sizeof(cmd));
		XBee.println(cmd);
		if(strncmp(cmd, CmdPrefix, CmdPreLen) == 0){
			if(strncmp(cmd+CmdPreLen, "CX,ON", 5) == 0){ cx = 1; }
			else if(strncmp(cmd+CmdPreLen, "CX,OFF", 6) == 0){ cx = 0; }
			else if(strncmp(cmd+CmdPreLen, "CAL", 3) == 0){ GroundAltitude = altitude; }
			else if(strncmp(cmd+CmdPreLen, "PCKT,", 5) == 0){ sscanf(cmd+14, "%i", &packet_count); }
			else if(strncmp(cmd+CmdPreLen, "ST,", 3) == 0){
				if(strncmp(cmd+CmdPreLen, "GPS", 3) == 0){ /* Set to GPS */ }
				else{ /* Set to manual time */ }}
			else if(strncmp(cmd+CmdPreLen, "BCN,ON", 6) == 0){ bcn = 1; }
			else if(strncmp(cmd+CmdPreLen, "BCN,OFF", 7) == 0){ bcn = 0; }
			else if(strncmp(cmd+CmdPreLen, "SIM,ENABLE", 10) == 0){ simE = 1; }
			else if(strncmp(cmd+CmdPreLen, "SIM,ACTIVATE", 12) == 0){ if(simE){ simA = 1; } }
			else if(strncmp(cmd+CmdPreLen, "SIM,DISABLE", 11) == 0){ simE = 0; simA = 0; }
			else if(strncmp(cmd+CmdPreLen, "STATE,HS_RELEASE", 16) == 0){ state = 3; goto ChangeHRelease; }
		}
		for(int i = 0; i < CmdLength; i++){ cmd[i] = '\0'; }
	}

	if(state == 5 && millis() - landedTimer >= 1000){
		digitalWrite(LEDPin, o ? LOW : HIGH);
		digitalWrite(BuzzerPin, o ? LOW : HIGH);
		o = !o;
	}

	/* Send telemetry */
	if(cx && millis() - packetTimer >= PacketSpeed){
		velocity = (altitude - pa); pa = altitude;
		//sprintf(packet, "%u,%i,%f,%f,%f,%f,%f,%f,%f, %s", TEAM_ID, packet_count, tilt_x, tilt_y, rot_z, temperature, pressure, altitude-GroundAltitude, velocity, States[state]);
		sprintf(packet, "%u,%u,%s,%f,%f,%f,%s,%f,%f,%f,%f,%f,%f,%s,,%f", TEAM_ID, packet_count, States[state], altitude-GroundAltitude, temperature,pressure, gps_time, gps_altitude, gps_latitude, gps_longitude, tilt_x, tilt_y, rot_z, cmd_echo, velocity);
		//sprintf(packet, "%u,%s,%u,%s,%s,%f,%f,%c,%c,%f,%f,%f,%s,%f,%f,%f,%u,%f,%f,%f,%s,,%f", TEAM_ID, mission_time, packet_count, modes[mode], states[state], altitude, air_speed, hs_deployed, pc_deployed, temperature, voltage, pressure, gps_time, gps_altitude, gps_latitude, gps_longitude, gps_sats, tilt_x, tilt_y, rot_z, cmd_echo, velocity);
		packet_count++;
		
		//Serial.println(packet);
		XBee.println(packet);
		SD.println(packet);
		packetTimer = millis();
	}
}



