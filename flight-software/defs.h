#define BNO055_SAMPLERATE_DELAY_MS (100)

#define CmdPrefix "CMD,2079,"
#define CmdPreLen 9

#define TEAM_ID 2079
#define PacketSpeed 500 /* Send a packet every 1000ms */

#define AscentAlt 20
#define SeparateAltitude 700 /* Altitude to separate from rocket (Should be 700) */
#define HRAltitutde 100 /* Altitude to release heat shield (Should be 100) */
#define LandAlt 5 /* Altitude to determine if landed (Should be 5) */


#define PacketLength 512
#define CmdLength 64

#define CamPin 15
#define BonusPin 14
#define paraServoPin 20
#define releaseServoPin 18
#define camServoPin 19
#define LEDPin 17
#define BuzzerPin 16

#include "pitches.h"
const int Notes[][2] =
  {{NOTE_C5, 326},
  {NOTE_E5, 652},
  {NOTE_E5, 3580},
  {NOTE_C5, 326},
  {NOTE_E5, 625},
  {NOTE_E5, 489},
  {NOTE_B4, 326},
  {NOTE_A4, 163},
  {NOTE_GS4, 2610}};

const int NoteCt = 9;
