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
#define BLow digitalWrite(BuzzerPin, HIGH)

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

void setup(void){
	baseTime = get_baseTime();
	
	Serial.begin(9600);
	XBee.begin(9600); XBee.println("XBee connected");
	SD.begin(9600);
	
	Serial.println("Orientation Sensor Test"); Serial.println("");
	/* Initialise the sensor */
	while(!bno.begin()){ Serial.println("BNO055 not found"); delay(100); }

	XBee.println("Adafruit BMP388 / BMP390 test");
	if(!ptSensor.begin_I2C()){ XBee.println("Could not find a valid BMP3 sensor, check wiring!"); }

	// Set up oversampling, filter initialization, and ground elevation
	ptSensor.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
	ptSensor.setPressureOversampling(BMP3_OVERSAMPLING_4X);
	ptSensor.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
	ptSensor.setOutputDataRate(BMP3_ODR_50_HZ);
	while(!ptSensor.performReading()){ XBee.println("Failed to perform reading"); }
	for(unsigned i = 0; i < 100; i++){ GroundElevation = ptSensor.readAltitude(SEALEVELPRESSURE_HPA); } XBee.println("Ground Elevation Set");

	// Print out packet format
	packet = "TEAM_ID,MISSION_TIME,PACKET_COUNT,SW_STATE, PL_STATE,ALTITUDE, TEMP, VOLTAGE, GPS_LATITUDE, GPS_LONGITUDE, GYRO_R,GYRO_P,GYRO_Y";
	for(unsigned i = 0; i < packet.size(); i++){ XBee.print(packet[i]);} XBee.println();

	pinMode(BuzzerPin, OUTPUT); pinMode(LEDPin, OUTPUT);
	packetTimer = time(0);

	oldAlt = 0; velocity = 0;
  delay(1000);
}

void loop(void){
  //could add VECTOR_ACCELEROMETER, VECTOR_MAGNETOMETER,VECTOR_GRAVITY...
  sensors_event_t orientationData , angVelocityData , linearAccelData, magnetometerData, accelerometerData, gravityData;
  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
  bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);
  bno.getEvent(&magnetometerData, Adafruit_BNO055::VECTOR_MAGNETOMETER);
  bno.getEvent(&accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  bno.getEvent(&gravityData, Adafruit_BNO055::VECTOR_GRAVITY);

  printEvent(&orientationData);
  printEvent(&angVelocityData);
  printEvent(&linearAccelData);
  printEvent(&magnetometerData);
  printEvent(&accelerometerData);
  printEvent(&gravityData);

  int8_t boardTemp = bno.getTemp();
  Serial.println();
  Serial.print(F("temperature: "));
  Serial.println(boardTemp);

  uint8_t system, gyro, accel, mag = 0;
  bno.getCalibration(&system, &gyro, &accel, &mag);
  Serial.println();
  Serial.print("Calibration: Sys=");
  Serial.print(system);
  Serial.print(" Gyro=");
  Serial.print(gyro);
  Serial.print(" Accel=");
  Serial.print(accel);
  Serial.print(" Mag=");
  Serial.println(mag);

  Serial.println("--");
  delay(BNO055_SAMPLERATE_DELAY_MS);
}

void printEvent(sensors_event_t* event) {
  double x = -1000000, y = -1000000 , z = -1000000; //dumb values, easy to spot problem
  if (event->type == SENSOR_TYPE_ACCELEROMETER) {
    Serial.print("Accl:");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else if (event->type == SENSOR_TYPE_ORIENTATION) {
    Serial.print("Orient:");
    x = event->orientation.x;
    y = event->orientation.y;
    z = event->orientation.z;
  }
  else if (event->type == SENSOR_TYPE_MAGNETIC_FIELD) {
    Serial.print("Mag:");
    x = event->magnetic.x;
    y = event->magnetic.y;
    z = event->magnetic.z;
  }
  else if (event->type == SENSOR_TYPE_GYROSCOPE) {
    Serial.print("Gyro:");
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  }
  else if (event->type == SENSOR_TYPE_ROTATION_VECTOR) {
    Serial.print("Rot:");
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  }
  else if (event->type == SENSOR_TYPE_LINEAR_ACCELERATION) {
    Serial.print("Linear:");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else if (event->type == SENSOR_TYPE_GRAVITY) {
    Serial.print("Gravity:");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else {
    Serial.print("Unk:");
  }

  Serial.print("\tx= ");
  Serial.print(x);
  Serial.print(" |\ty= ");
  Serial.print(y);
  Serial.print(" |\tz= ");
  Serial.println(z);
}

