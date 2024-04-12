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

int gpsHour = 0;
int gpsMin = 0;
int gpsSec = 0;

float GroundAltitude = 0;
float pa = 0;
float simPressure = -1;

long packetTimer;
long veloTimer;
long landedTimer;

bool cx = 1;
bool bcn = 1;
bool simE = 0;
bool resetGA = 0;

BMP388_DEV bmp; /* BMP 388 sensor */
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28); /* BNO055 sensor*/
SFE_UBLOX_GNSS m8q; /* GPS sensor*/

static _107_::Servo releaseServo; /* Aerobrake release servo */
static _107_::Servo camServo; /* Aerobrake release servo */

SoftwareSerial XBee(0, 1); // RX, TX
SoftwareSerial SD(5, 4); /* OpenLog SD Card */

void setup(void){
	/* Attempt to open Serial line (testing purposes) */
	//Serial.begin(9600);
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

	/* Command handling */
  //Serial.println("Before CMD");
	if(XBee.available()){
    //Serial.println("Enter CMD");
		XBee.readBytesUntil('\n', cmd, sizeof(cmd));
		//XBee.println(cmd);
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
			else if(strncmp(cmd+CmdPreLen, "SIM,ACTIVATE", 12) == 0){ if(simE){ mode = 1; resetGA = 1; } }
			else if(strncmp(cmd+CmdPreLen, "SIM,DISABLE", 11) == 0){ simE = 0; mode = 0; }
			else if(strncmp(cmd+CmdPreLen, "STATE,HS_RELEASE", 16) == 0){ state = 3; goto ChangeHRelease; }
			else if(strncmp(cmd+CmdPreLen, "SIMP,", 5) == 0){ if(mode){ sscanf(cmd+14, "%f", &simPressure); pressure = simPressure; } }
		}
		for(int i = 0; i < sizeof(cmd_echo); i++){ cmd_echo[i] = '\0'; }
		strncpy(cmd_echo, cmd+CmdPreLen, CmdLength);
		for(int i = 0; i < sizeof(cmd); i++){ cmd[i] = '\0'; }
    //Serial.println("Exit CMD");
	}

	bmp.startForcedConversion();
	if(mode == 0){
		bmp.getMeasurements(temperature, pressure, altitude);
	}else{
		/* Fix later */
		bmp.getMeasurements(temperature, pressure, altitude);
		bmp.getTempPres(temperature, pressure);
		//XBee.println(pressure);
		if(simPressure != -1){ pressure = simPressure / 100; }
		altitude = ((float)powf(1013.23f / pressure, 0.190223f) - 1.0f) * (temperature + 273.15f) / 0.0065f;
		if(resetGA && simPressure != -1){ GroundAltitude = altitude; resetGA = 0; }
	}
	if(GroundAltitude == 0 && altitude > 0){ GroundAltitude = altitude; }

	tilt_x = event.orientation.x;
	tilt_y = event.orientation.y;
	rot_z = event.orientation.z;

	gps_latitude = m8q.getLatitude()/10000000;
	gps_longitude = m8q.getLongitude()/10000000;
	gps_altitude = m8q.getAltitude();

	gpsHour = m8q.getHour();
	gpsMin = m8q.getMinute();
	gpsSec = m8q.getSecond();
	for(int i = 0; i < 16; i++){ gps_time[i] = '\0'; }
	sprintf(gps_time+strlen(gps_time), gpsHour < 10 ? "0%i:" : "%i:", gpsHour);
	sprintf(gps_time+strlen(gps_time), gpsMin < 10 ? "0%i:" : "%i:", gpsMin);
	sprintf(gps_time+strlen(gps_time), gpsSec < 10 ? "0%i" : "%i:", gpsSec);

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



	if(state == 5 && millis() - landedTimer >= 1000){
		digitalWrite(LEDPin, o ? LOW : HIGH);
		digitalWrite(BuzzerPin, o ? LOW : HIGH);
		o = !o;
	}
	strncpy(mission_time, gps_time, 16);

	/* Send telemetry */
	if(cx && millis() - packetTimer >= PacketSpeed){
		velocity = (altitude - pa); pa = altitude;
		//sprintf(packet, "%u,%u,%s,%s,%f,%f,%f,%s,%f,%f,%f,%f,%f,%f,%s,,%f", TEAM_ID, packet_count, Modes[mode], States[state], altitude-GroundAltitude, temperature,pressure, gps_time, gps_altitude, gps_latitude, gps_longitude, tilt_x, tilt_y, rot_z, cmd_echo, simPressure / 100);
		sprintf(packet, "%u,%s,%u,%s,%s,%.1f,%f,%c,%c,%.1f,%.1f,%.1f,%s,%f,%f,%f,%u,%f,%f,%f,%s,,%f", TEAM_ID, mission_time, packet_count, Modes[mode], States[state], altitude - GroundAltitude, air_speed, hs_deployed, pc_deployed, temperature, pressure/10, voltage, gps_time, gps_altitude, gps_latitude, gps_longitude, gps_sats, tilt_x, tilt_y, rot_z, cmd_echo, velocity);
		packet_count++;

		//Serial.println(packet);
		XBee.println(packet);
		//Serial.println(packet);
		packetTimer = millis();
	}
}


