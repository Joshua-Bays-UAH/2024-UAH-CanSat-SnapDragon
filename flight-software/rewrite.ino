#include <Adafruit_Sensor.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_BNO055.h>
#include <107-Arduino-Servo-RP2040.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <string>
#include <string.h>
#include <time.h>
#include <utility/imumaths.h>

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
#define BLow digitalWrite(BuzzerPin, LOW)
#define LHigh digitalWrite(LEDPin, HIGH)
#define LLow digitalWrite(LEDPin, LOW)

#define AscentHeight 5
#define ReleaseHeight 760
#define ParaHeight 100

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)


Adafruit_BMP3XX bmp; // Adafruit BMP 388 pressure and temperature sensor
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

uint16_t BNO055_SAMPLERATE_DELAY_MS = 100;

SoftwareSerial XBee(XBeeRX, XBeeTX);
SoftwareSerial SD(SDRX, SDTX);

unsigned GroundElevation;
char missionTime[9]; // "HH:MM:SS"
unsigned packetCount = 0; // packets
char mode; // F or S
char state[18]; // ROCKET_SEPARATION is the longest state name
float altitude; // X.X m
unsigned airSpeed; // X m/s
char hsDeployed; // P or N
char pcDeployed; // C or N
float temperature; // X.X degrees Celsius
float pressure; // X.X kPa
double voltage; // X.X volts
char gpsTime[9]; // HH:MM:SS
float gpsAltitude; // X.x m
float gpsLatitude; // X.XXXX m
float gpsLongitude; // X.XXXX m
unsigned gpsStats; // X satellites
float tiltX; // X.XX degrees
float tiltY; // X.XX degrees
double rotZ; // X.X degrees/s

char gsCmd[64];
_Bool cx = 1;
_Bool simE = 0;
_Bool bcn = 0;

void setup(void){
	Serial.begin(9600);
	XBee.begin(9600); XBee.println("XBee connected");
	SD.begin(9600);
	
	Serial.println("Orientation Sensor Test"); Serial.println("");
	/* Initialize the sensor */
	while(!bno.begin()){ Serial.println("BNO055 not found"); delay(100); }

	XBee.println("Adafruit BMP388 / BMP390 test");
	if(!bmp.begin_I2C()){ XBee.println("Could not find a valid BMP3 sensor, check wiring!"); }

	// Set up oversampling, filter initialization, and ground elevation
	bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
	bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
	bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
	bmp.setOutputDataRate(BMP3_ODR_50_HZ);
	while(!bmp.performReading()){ /* XBee.println("Failed to perform reading"); */ }
	for(unsigned i = 0; i < 100; i++){ GroundElevation = bmp.readAltitude(SEALEVELPRESSURE_HPA); } //XBee.println("Ground Elevation Set");
	delay(1000);
}

void loop(void){
	sensors_event_t orientationData , angVelocityData; // Check official examples for data expansions if needed
	bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
	bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
	
	if(XBee.available() > 0){
		XBee.readBytesUntil('\0', gsCmd, 64);
		if(strcmp(gsCmd, "CMD,2079,CX,") == 0){
			if(strcmp(gsCmd+12, "ON") == 0){ cx = 1; }
			else if(strcmp(gsCmd+12, "OFF") == 0){ cx = 0; }
		}
		if(strcmp(gsCmd, "CMD,2079,ST,") == 0){}
		if(strcmp(gsCmd, "CMD,2079,SIM,") == 0){
			if(strcmp(gsCmd+13, "ENABLE") == 0){ simE = 1; }
			if(strcmp(gsCmd+13, "ACTIVATE") == 0 && simE){ mode = 'S'; }
			if(strcmp(gsCmd+13, "DISABLE") == 0){ simE = 0; mode = 'F'; }
		}
		if(strcmp(gsCmd, "CMD,2079,SIMP,") == 0 && mode == 'S'){}
		if(strcmp(gsCmd, "CMD,2079,CAL") == 0){ GroundElevation = altitude; }
		if(strcmp(gsCmd, "CMD,2079,BCN,") == 0){
			if(strcmp(gsCmd+13, "ON") == 0){ bcn = 1; }
			else if(strcmp(gsCmd+13, "OFF") == 0){ bcn = 0; }
		}
		if(strcmp(gsCmd, "CMD,2079,STATE,") == 0){
			if(strcmp(gsCmd+14, "ROCKET_SEPARATION") == 0){}
			else if(strcmp(gsCmd+14, "HS_RELEASE") == 0){}
			else if(strcmp(gsCmd+14, "LANDED") == 0){}
			else if(strcmp(gsCmd+14, "RESTART") == 0){}
		}
		if(strcmp(gsCmd, "CMD,2079,WIPE") == 0){}
	}
	// Get data
	if(mode == 'F'){
		//altitude; // X.X m
		//pressure; // X.X kPa
	}else if(mode == 's'){
		// pressure read from GS command
		altitude = 44330.0 * (1.0 - pow((pressure / 100.0F) / SEALEVELPRESSURE_HPA, 0.1903));
	}
	//missionTime[9]; // "HH:MM:SS"
	//packetCount = 0; // packets
	//mode; // F or S
	//state[18]; // ROCKET_SEPARATION is the longest state name
	//airSpeed; // X m/s
	//hsDeployed; // P or N
	//pcDeployed; // C or N
	//temperature // X.X degrees Celsius
	//voltage; // X.X volts
	//gpsTime[9]; // HH:MM:SS
	//gpsAltitude; // X.x m
	//gpsLatitude; // X.XXXX m
	//gpsLongitude; // X.XXXX m
	//gpsStats; // X satellites
	//tiltX = orientationData.orientation.x;
	//tiltY = orientationData.orientation.y;
	//rotZ; // X.X degrees/s
	if(cx){} //Send packets
	if(bcn){
		BHigh; LHigh;
		delay(250);
		BLow; LLow;
		delay(250);
	}
}

