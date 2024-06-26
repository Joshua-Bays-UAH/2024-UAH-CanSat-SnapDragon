#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <BMP388_DEV.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <Servo.h>

#include <string.h>
#include <utility/imumaths.h>
#include "defs.h"

int landedOn = 0;

const char Modes[2][12] = { "F", "S" };
const char States[6][12] = { "LAUNCH_WAIT", "ASCENT", "SEPARATE", "DESCENT", "HS_RELEASE", "LANDED" };
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
char cmd[CmdLength];
char cmdBuff[CmdLength];
char c;

int gpsHour = 0;
int gpsMin = 0;
int gpsSec = 0;
int offHour = 0;
int offMin = 0;
int offSec = 0;

float GroundAltitude = 0;
float pa = 0;
float simPressure = -1;
float avgAirSpeed = 0;
float apogee = 0;

unsigned long packetTimer;
unsigned long veloTimer;
unsigned long hrReleaseTimer;
unsigned long aeroReleaseTimer;
unsigned long landedTimer;
unsigned long bcnTimer;
unsigned long lockTimer;
short noteCounter = -1;

bool cx = 1;
bool bcn = 0;
bool simE = 0;
bool resetGA = 0;
bool lock = false;

BMP388_DEV bmp;                                  /* BMP 388 sensor */
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28); /* BNO055 sensor*/
Adafruit_BNO055 bno2 = Adafruit_BNO055(55, 0x28); /* BNO055 sensor*/
SFE_UBLOX_GNSS m8q;                              /* GPS sensor*/
Servo paraServo;
Servo releaseServo;
Servo camServo;
sensors_event_t event;

SoftwareSerial XBee(0, 1);  // RX, TX
SoftwareSerial SD(5, 4);    /* OpenLog SD Card */

int melody[] = { 262, 196, 196, 220, 196, 0, 247, 262 };
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};
sensors_event_t e;

bool timer(unsigned long &t, unsigned long l, bool f = 1);
int pid(float a, float b);

void setup() {
  /* Change I2C ports */
  Wire.setSDA(8);
  Wire.setSCL(9);
  Wire.setClock(100000); // Increase I2C clock speed to 400kHz
  Wire.begin();

  Serial.begin(9600);
  // while(!Serial){};

  /* Start XBee */
  XBee.begin(9600);
  for (int i = 0; i < 10 && !XBee; i++) {
    delay(150);
    XBee.begin(9600);
  }
  XBee.setTimeout(25);

  /* Start OpenLogger */
  SD.begin(9600);
  for (int i = 0; i < 10 && !SD; i++) {
    // XBee.println("SD not connected!");
    SD.begin(9600);
    delay(150);
  }
  // XBee.println("SD connected!");

  /* Start rotation sensor */
  for (int i = 0; i < 1 && !bno.begin(); i++) {
    //XBee.println("BNO not connected!");
    delay(150);
  }
  // XBee.println("BNO connected!");
  bno.setExtCrystalUse(true);

  /* Start altitude/pressure sensor */
  for (int i = 0; i < 10 && !bmp.begin(); i++) {
    // XBee.println("BMP not connected!");
    delay(150);
  }
  // XBee.println("BMP connected.");
  bmp.startForcedConversion();

  /* Start GPS */
  for (int i = 0; i < 10 && !m8q.begin(); i++) {
    // XBee.println("M8Q not connected!");
    delay(150);
  }
  // XBee.println("M8Q connected.");
  m8q.setI2COutput(COM_TYPE_UBX); /*Set the I2C port to output UBX only (turn off NMEA noise) */
  m8q.setNavigationFrequency(10);

  // SD.write(26); SD.write(26); SD.write(26);
  // SD.print("new 2079.txt"); SD.write(13);
  // SD.print("append 2079.txt"); SD.write(13);

  //paraServo.attach(paraServoPin);
  //releaseServo.attach(releaseServoPin);
  camServo.attach(camServoPin);
  //paraServo.writeMicroseconds(2000);
  //releaseServo.writeMicroseconds(2000);

  /* Initalize pins */
  pinMode(LEDPin, OUTPUT);
  pinMode(BuzzerPin, OUTPUT);

  for(int i = 0; i < 10; i++){
    avgAirSpeed += sqrt((2*abs( (1000*( ((analogRead(PitotPin) * (3.25 / 1023.0)) * 1.45454545455) / 5 -0.5)/0.2)  ))/1.225);
  }
  avgAirSpeed /= 10;

  /* Start camera */
  pinMode(CamPin, OUTPUT);
  pinMode(BonusPin, OUTPUT);
  digitalWrite(CamPin, HIGH);
  digitalWrite(BonusPin, HIGH);
  delay(5000);
  digitalWrite(CamPin, LOW);
  digitalWrite(BonusPin, LOW);
  SD.println("camera on");

  packetTimer = millis();
  veloTimer = millis();
}

uint8_t sys, gyro, accel, mag = 0;
void loop() {
  /* Command handling */
  if (XBee.available()) {
    int bytesRead = 0;
    bytesRead = XBee.readBytes(cmdBuff, sizeof(cmdBuff));
    if (bytesRead > 0) {
      cmdBuff[bytesRead] = 0;
    }
    for (int i = 0; i < bytesRead; i++) {
      cmd[strlen(cmd)] = cmdBuff[i];
      if(cmdBuff[i] > 127 || cmdBuff[i] < 20){
          for (int i = 0; i < sizeof(cmd); i++) {
            cmd[i] = '\0';
          }
          for (int i = 0; i < sizeof(cmd); i++) {
            cmd[i] = '\0';
            cmdBuff[i] = '\0';
          }
      }else if(cmdBuff[i] == CmdTermChar){
        goto interptetPacket;
      }
    }
    if(cmdBuff[bytesRead - 1] == CmdTermChar){
      interptetPacket:
      cmd[strlen(cmd) - 1] = '\0';
      // for (int i = 0; i < sizeof(cmdBuff); i++) { cmdBuff[i] = '\0'; }
      if (strncmp(cmd, CmdPrefix, CmdPreLen) == 0) {
        if (strncmp(cmd + CmdPreLen, "CX,ON", 5) == 0) {
          cx = 1;
        } else if (strncmp(cmd + CmdPreLen, "CX,OFF", 6) == 0) {
          cx = 0;
        } else if (strncmp(cmd + CmdPreLen, "CAL", 3) == 0) {
          GroundAltitude = altitude;
        } else if (strncmp(cmd + CmdPreLen, "PCKT,", 5) == 0) {
          sscanf(cmd + CmdPreLen + 5, "%i", &packet_count);
        } else if (strncmp(cmd + CmdPreLen, "ST,", 3) == 0) {
          if (strncmp(cmd + CmdPreLen + 3, "GPS", 3) == 0) {
            /* Set to GPS */
            offHour = 0;
            offMin = 0;
            offSec = 0;
          } else{
            /* Set to manual time */
            sscanf(cmd + CmdPreLen + 3, "%i:", &offHour);
            sscanf(cmd + CmdPreLen + 6, "%i:", &offMin);
            sscanf(cmd + CmdPreLen + 9, "%i", &offSec);
            offHour = m8q.getHour() - offHour;
            offMin = m8q.getMinute() - offMin;
            offSec = m8q.getSecond() - offSec;
            offHour += 24;
            offHour %= 24;
            offMin += 60;
            offMin %= 60;
            offSec += 60;
            offSec %= 60;
          }
        } else if (strncmp(cmd + CmdPreLen, "BCN,ON", 6) == 0) {
          bcn = 1;
          bcnTimer = millis();
          noteCounter = 0;
        } else if (strncmp(cmd + CmdPreLen, "BCN,OFF", 7) == 0) {
          bcn = 0;
          digitalWrite(LEDPin, 0);
          noteCounter = -1;
        } else if (strncmp(cmd + CmdPreLen, "STATE,SEPARATE", 14) == 0) {
          goto ChangeSeparate;
        } else if (strncmp(cmd + CmdPreLen, "STATE,HS_RELEASE", 16) == 0) {
          goto ChangeHRelease;
        } else if (strncmp(cmd + CmdPreLen, "STATE,LANDED", 12) == 0) {
          goto ChangeLanded;
        } else if (strncmp(cmd + CmdPreLen, "SIM,ENABLE", 10) == 0) {
          simE = 1;
        } else if (strncmp(cmd + CmdPreLen, "SIM,ACTIVATE", 12) == 0) {
          if (simE) {
            mode = 1;
            resetGA = 1;
          }
        } else if (strncmp(cmd + CmdPreLen, "SIM,DISABLE", 11) == 0) {
          simE = 0;
          mode = 0;
        } else if (strncmp(cmd + CmdPreLen, "SIMP,", 5) == 0) {
          if (mode) {
            sscanf(cmd + 14, "%f", &simPressure);
            pressure = simPressure;
          }
        } else if (strncmp(cmd + CmdPreLen, "LOCK", 4) == 0) {
          lock = true;
          lockTimer = millis();
          releaseServo.attach(releaseServoPin);
          releaseServo.write(140);
          paraServo.attach(paraServoPin);
          paraServo.write(180);
        }
        char buff[sizeof(cmd_echo)];
        for (int i = 0; i < sizeof(cmd_echo); i++) {
          cmd_echo[i] = '\0';
          buff[i] = '\0';
        }
      }
      strncpy(cmd_echo, cmd + CmdPreLen, CmdLength);
      char buff[CmdLength] = "";
      for (int i = 0; i < strlen(cmd_echo); i++) {
        if (cmd_echo[i] == ',') { continue; }
        buff[strlen(buff)] = cmd_echo[i];
      }
      strncpy(cmd_echo, buff, sizeof(cmd_echo));
      for (int i = 0; i < sizeof(cmd); i++) {
        cmd[i] = '\0';
      }
      for (int i = 0; i < sizeof(cmd); i++) {
        cmd[i] = '\0';
        cmdBuff[i] = '\0';
      }
    }
  }
  /* Sample sensors */
  bmp.startForcedConversion();
  bmp.getMeasurements(temperature, pressure, altitude);
  // bno.getEvent(&event);
  event = e;
  // sys = 0;
  // gyro = 0;
  // accel = 0;
  // mag = 0;
  // bno.getCalibration(&sys, &gyro, &accel, &mag);
  tilt_x = event.orientation.x;
  tilt_y = event.orientation.y;
  rot_z = event.orientation.z;

  /* Pitot Tube air Speed */
  air_speed = sqrt((2*abs( (1000*( ((analogRead(PitotPin) * (3.25 / 1023.0)) * 1.45454545455) / 5 -0.5)/0.2)  ))/1.225); //1.225 is ISA density(kg/m^3)
  voltage = 0.0064264849 * analogRead(VoltagePin);

  // XBee.println("GPS Start");

  if(m8q.getPVT()){
    gps_latitude = (float)m8q.getLatitude() / 10000000;
    gps_longitude = (float)m8q.getLongitude() / 10000000;
    gps_altitude = (float)m8q.getAltitude();
  }
  // XBee.println("GPS TIME");
  gpsHour = m8q.getHour() - offHour + 24;
  gpsHour %= 24;
  gpsMin = m8q.getMinute() - offMin + 60;
  gpsMin %= 60;
  gpsSec = m8q.getSecond() - offSec + 60;
  gpsSec %= 60;
  m8q.flushPVT();

  // XBee.println("GPS End");

  /* Simulation mode pressure */
  if (mode != 0) {
    if (simPressure != -1) {
      pressure = simPressure / 100;
    }
    altitude = ((float)powf(1013.23 / pressure, 0.190223) - 1.0) * (temperature + 273.15) / 0.0065;
    if (resetGA && simPressure != -1) {
      GroundAltitude = altitude;
      resetGA = 0;
    }
  }

  if (GroundAltitude == 0 && altitude > 0) {
    GroundAltitude = altitude;
  }

  if(altitude - GroundAltitude >= apogee){
	apogee = altitude - GroundAltitude;
  }

  for (int i = 0; i < 16; i++) {
    gps_time[i] = '\0';
    mission_time[i] = '\0';
  }
  sprintf(mission_time + strlen(mission_time), ((gpsHour - offHour + 24) % 24 < 10 ? "0%i:" : "%i:"), (gpsHour - offHour + 24) % 24);
  sprintf(mission_time + strlen(mission_time), ((gpsMin - offMin + 60) % 60 < 10 ? "0%i:" : "%i:"), (gpsMin - offMin + 60) % 60);
  sprintf(mission_time + strlen(mission_time), ((gpsSec - offSec + 60) % 60 < 10 ? "0%i" : "%i"), (gpsSec - offSec + 60) % 60);

  sprintf(gps_time + strlen(gps_time), gpsHour < 10 ? "0%i:" : "%i:", gpsHour);
  sprintf(gps_time + strlen(gps_time), gpsMin < 10 ? "0%i:" : "%i:", gpsMin);
  sprintf(gps_time + strlen(gps_time), gpsSec < 10 ? "0%i" : "%i", gpsSec);

  if (state == 0 && altitude - GroundAltitude >= AscentAlt) {
ChangeAscent:
    state++;
  } else if (state == 1 && altitude - GroundAltitude >= SeparateAltitude && altitude - GroundAltitude <= apogee - 5){
ChangeSeparate:
    state++;
	aeroReleaseTimer = millis();
	  releaseServo.attach(releaseServoPin);
    releaseServo.writeMicroseconds(1000);
    hs_deployed = 'P';
  } else if (state == 2 && velocity <= -.5) {
ChangeDescent:
    state++;
  } else if (state == 3 && altitude - GroundAltitude <= HRAltitutde) {
ChangeHRelease:
    state++;
    hrReleaseTimer = millis();
	paraServo.attach(paraServoPin);
    paraServo.writeMicroseconds(1000);
    pc_deployed = 'C';
  } else if (state == 4 && altitude - GroundAltitude <= LandAlt) {
ChangeLanded:
    state++;
    cx = 0;
    landedTimer = millis();
    landedOn = 1;
    digitalWrite(LEDPin, HIGH);
    digitalWrite(BuzzerPin, HIGH);
    digitalWrite(CamPin, HIGH);
    digitalWrite(BonusPin, HIGH);
    delay(5000);
    digitalWrite(CamPin, LOW);
    digitalWrite(BonusPin, LOW);
    digitalWrite(BuzzerPin, LOW);
    bcn = 1;
    bcnTimer = millis();
    noteCounter = 0;
    goto sendPacket;
  }

  if (timer(landedTimer, 1000, bcn)) {
    digitalWrite(LEDPin, landedOn ? LOW : HIGH);
    digitalWrite(BuzzerPin, landedOn ? LOW : HIGH);
    landedOn = !landedOn;
  }

  // if(packetTimer - millis() >= 900){
  if (timer(packetTimer, PacketSpeed, cx)) {
    sendPacket:
    velocity = (altitude - pa);
    pa = altitude;
    packet_count++;
    //sprintf(packet, "%u,%u,%s,%s,%f,%f,%f,%s,%f,%f,%f,%f,%f,%f,%s,,%f", TEAM_ID, packet_count, Modes[mode], States[state], altitude-GroundAltitude, temperature,pressure, gps_time, gps_altitude, gps_latitude, gps_longitude, tilt_x, tilt_y, rot_z, cmd_echo, simPressure / 100);
    sprintf(packet, "%u,%s,%u,%s,%s,%.1f,%f,%c,%c,%.1f,%.1f,%.1f,%s,%.1f,%.4f,%.4f,%u,%.2f,%.2f,%.2f,%s,,%f", TEAM_ID, mission_time, packet_count, Modes[mode], States[state], altitude - GroundAltitude, air_speed - avgAirSpeed, hs_deployed, pc_deployed, temperature, voltage, pressure / 10, gps_time, gps_altitude, gps_latitude, gps_longitude, gps_sats, tilt_x, tilt_y, rot_z, cmd_echo, velocity);
    XBee.println(packet);
    // Serial.println(packet);
    packetTimer = millis();
    SD.println(packet);
  }

 if(timer(aeroReleaseTimer, 1000, state != 3)){
   releaseServo.detach();
   }
  if(timer(hrReleaseTimer, 1000, state > 3)) {
    paraServo.detach();
  }
  if(timer(lockTimer, 500, lock)) {
     paraServo.detach();
     releaseServo.detach();
     lock = false;
  }
// }

// void setup1(){
  // Wire.setSDA(8);
  // Wire.setSCL(9);
  // Ubno2.begin();
// }

// void loop1(){
  bno.getEvent(&e);
  camServo.writeMicroseconds(pid(e.orientation.x - 180, e.orientation.z));
  // delay(5);
}

bool timer(unsigned long &t, unsigned long l, bool f) {
  if (!f) {
    return 0;
  }
  if (millis() - t >= l) {
    t = millis();
    return 1;
  }
  return 0;
}

int pid(float a, float b) {
  if (fabs(b) < 120) {
    if (a > 20) {
      return 1800;
    } else if (a < -20) {
      return 1200;
    }
  } else {
    if (fabs(a) < 145) {
      if (a > 0) {
        return 1800;
      } else if (a < 0) {
        return 1200;
      }
    }
  }
  return 1500;
}

