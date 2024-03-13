#define BNO055_SAMPLERATE_DELAY_MS (100)

#define CmdPrefix "CMD,2079,"
#define CmdPreLen 9

#define TEAM_ID 2079
#define PacketSpeed 990 /* Send a packet every 1000ms */

#define AscentAlt 4
#define SeparateAltitude 8 /* Altitude to separate from rocket (Should be 700) */
#define HRAltitutde 3 /* Altitude to release heat shield (Should be 100) */
#define LandAlt 1 /* Altitude to determine if landed (Should be 5) */


#define PacketLength 512
#define CmdLength 64

#define CamPin 15
#define ServoPin 16
#define LEDPin 3
#define BuzzerPin 28
