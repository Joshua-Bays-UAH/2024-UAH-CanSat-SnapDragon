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

#define AscentHeight 5
#define ReleaseHeight 760
#define ParaHeight 100

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)


Adafruit_BMP3XX ptSensor; // Adafruit BMP 388 pressure and temperature sensor
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

uint16_t BNO055_SAMPLERATE_DELAY_MS = 100;

SoftwareSerial XBee(XBeeRX, XBeeTX);
SoftwareSerial SD(SDRX, SDTX);

unsigned GroundElevation;
double voltage;
double gps_time;
double temperature;
double pressure;
double altitude;
double tilt_x;
double tilt_y;
//double rot_z;

void setup(void){
	Serial.begin(9600);
	XBee.begin(9600); XBee.println("XBee connected");
	SD.begin(9600);
	
	Serial.println("Orientation Sensor Test"); Serial.println("");
	/* Initialize the sensor */
	while(!bno.begin()){ Serial.println("BNO055 not found"); delay(100); }

	XBee.println("Adafruit BMP388 / BMP390 test");
	if(!ptSensor.begin_I2C()){ XBee.println("Could not find a valid BMP3 sensor, check wiring!"); }

	// Set up oversampling, filter initialization, and ground elevation
	ptSensor.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
	ptSensor.setPressureOversampling(BMP3_OVERSAMPLING_4X);
	ptSensor.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
	ptSensor.setOutputDataRate(BMP3_ODR_50_HZ);
	while(!ptSensor.performReading()){ /* XBee.println("Failed to perform reading"); */ }
	for(unsigned i = 0; i < 100; i++){ GroundElevation = ptSensor.readAltitude(SEALEVELPRESSURE_HPA); } //XBee.println("Ground Elevation Set");
	delay(1000);
}

void loop(void){
	sensors_event_t orientationData , angVelocityData; // Check official examples for data expansions if needed
	bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
	bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
	
	voltage = NULL; // UPDATE
	gps_time = NULL; // UPDATE
	temperature = NULL; // UPDATE
	pressure = NULL; // UPDATE
	altitude = NULL; // UPDATE
	tilt_x = orientationData.orientation.x;
	tilt_y = orientationData.orientation.y;
	//double rot_z = angVelocityData->;
}
