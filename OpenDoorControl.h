/*
  $Id$

  OpenDoorControl
  Copyright (c) 2012 The Perth Artifactory by
    Brett R. Downing <brett@artifactory.org.au>
    Daniel Harmsworth <atrophy@artifactory.org.au>
    Sebastian Southen <southen@gmail.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <LiquidCrystal.h>
#include "RTClib/RTClib.h"
#include <SD.h>
#include "general.h"

//Handy SI units  ***********************************
#define MS *(unsigned long)1000
#define S *((unsigned long)1000 MS)
#define uSPERHOUR (3600 S)

//Pin assignment ************************************
#define DOORSTRIKE 61		// the pin the door strike is on.

#define LCDBACKLIGHT 2		//

#define DOORBELL 62			// 62  //use the analogue pins, then the interrupts are on one port.
#define DOORBELLLED 3		//

#define REEDSWITCH 63		// 63  //in the door frame

#define GUESTOKSWITCH 64	// 64  //doesn't strictly need interrupt
#define GUESTOKLED 9		//

#define LOCKUPBUTTON 65		// 65
#define LOCKUPLED 8			//

#define STATUS_R 5			//
#define STATUS_G 6			//
#define STATUS_B 7			//

#define DEBOUNCEDELAY 50000

// bits on the analogue port (PINK) ********
#define DOORBELLBIT (1<<0)
#define LOCKUPBIT (1<<1)
#define GUESTOKBIT (1<<2)
#define REEDSWITCHBIT (1<<3)
#define SD_CS_PIN 4			// pin four on the ethermega

//LiquidCrystal lcd(32, 30, 28, 26, 24, 22);
extern LiquidCrystal lcd;
//RTC_DS1307 RTC;  			// using hardware i2c 18, 19
extern RTC_DS1307 RTC;

//for RFID strings ***********************************
//#define HASHLENGTH 20		// 20 bytes, 40 hex chars
#define HASHLENGTH 10		//  5 bytes, 10 hex chars (crypto-hashing disable)
#define RFIDLENGTH 10		//  5 bytes, 10 hex chars
#define RFIDSTRINGLENGTH 12	// 10 uid chars, 2 checksum

//for server and SD strings **************************
#define BUFSIZ 60			// the largest string we have to handle should be 41 bytes 
#define COMMENTCHAR '/'
/*
//for server strings *********************************
#define BEGINCHAR '('
#define ENDCHAR ')'
#define MIDCHAR ':'
#define CANCHAR '|'  //cancel the list, reset the state machine
#define FULLFILEALIAS "FullHashes"  //less than BUFSIZ chars
#define ASOCFILEALIAS "AsocHashes"
*/

//state variables for fail-over mechanisms ************
extern bool SDcardPresent;
extern bool spaceOpen;
extern bool guestAccess;
extern bool reedswitchState;
extern bool onNTPtime;
extern bool spaceGrace;
extern bool doorStatusRepeat;

//Fast Timers ***************************************(uS)
#define NUMFASTTIMERS 5
#define TIMERSECOND 0
#define TIMERSLOWPOLL 1
#define TIMERSTRIKE 2
#define TIMERLEDBLINK 3
#define TIMERLEDFADER 4


//#define TIMERSERVER 3
#define NUMSLOWTIMERS 6
#define TIMERLOGDUMP 0
#define TIMERRTCREFRESH 1
#define TIMERLCDTIME 2
#define TIMERDOORBELL 3
#define TIMERDOORSTATUS 4
#define TIMEREXITGRACE 5


extern timer fastTimers[NUMFASTTIMERS];
extern timer slowTimers[NUMSLOWTIMERS];


//the time for logging purposes *******************************
//unsigned long theTime = 0;  //seconds since boot (or 1900 epoch after NTP)
extern unsigned long theTime;
//the time for (sub-hour) timing purposes
//unsigned long microsHolder = 0;  //the current window
//unsigned long oldMicros = 0;  //the last time we checked


//SD card files *************************************
//File openFile; //the open file (the SD lib can't hold two files open?!)
extern File openFile;
extern char fullFile[13];
extern char assocFile[13];
extern char restFile[13];
extern const char* logFilePrefix;
extern const char* logFileSuffix;
extern char logFile[13];
//char* tempFile = "tempFile.txt";  //why am I not allowed to re-name files?
#define LOGDIGITS 4  //integer makes 4hex digits
//archived logs will have LOGDIGITS hex digits following them

extern bool blinkStatus;
extern int blinkPin;

extern int fadePin;
extern float fadeTime;

/*
byte mac[] = {0x31, 0x41, 0x15, 0x92, 0x65, 0x38};  //the mac of the door
  
IPAddress ip(192,168,16,80);  //the default IP address (only used if DHCP fails)
IPAddress gateway(192,168,16, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress server(192,168,16,1); 
unsigned int telnetPort = 3141;      // local port to listen for hash lists
EthernetClient client;
*/

