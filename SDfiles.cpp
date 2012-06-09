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

#include "OpenDoorControl.h"
//#include "Sha/sha1.h"
#include <stdio.h>
#include <stdlib.h>

#define DEBUG

#define SCHEDULECHARS 41

typedef enum {
	//PARSE_BEGIN = 0,
	PARSE_HASH = 1,
	PARSE_DAYMASK,
	PARSE_COMMENT
} ParseState;

int cardInFile(char *inputFile, char *cardHash) {
	if (strlen(cardHash) != HASHLENGTH) {
		return 0;
	}

  #ifdef DEBUG
    Serial.print("Parsing file: ");
    Serial.println(inputFile);
  #endif

  DateTime now(theTime); // What's the time

	openFile.close();  // just in case

	openFile = SD.open(inputFile);

  int retVal = 1; // 0 = success, 1 = failure, 2 = special exception

	char fileHash[BUFSIZ];
	int i;

	char fileSchedule[41];
	int j;

	ParseState state;

	while ((retVal == 1) && (openFile.available())) {	// parse file contents:
		i = 0;
		j = 0;
		state = PARSE_HASH;
		char c = ' ';  // I can't think of a better default to enter the loop with
    #ifdef DEBUG
      Serial.println("Starting read");
    #endif

		while ((c != '\n') && (openFile.available())) {
			c = openFile.read();

      #ifdef DEBUG
        Serial.print(c);
      #endif

			if (c == COMMENTCHAR) {

        #ifdef DEBUG
          Serial.println();
          Serial.println("Parsing Comment (1)");
        #endif

				state = PARSE_COMMENT;
			}
			else if ((c == ' ') || (c == '\t')) {
				if ((state == PARSE_HASH) && (i > 0)) {
          #ifdef DEBUG
            Serial.println();
            Serial.println("Parsing Schedule");
          #endif
					state = PARSE_DAYMASK;
				}
				if ((state == PARSE_DAYMASK) && (j > 0)) {
          #ifdef DEBUG
            Serial.println();
            Serial.println("Parsing Comment (2)");
          #endif
					state = PARSE_COMMENT;
				}
			}
			else {
				if (state == PARSE_HASH) {
					if (i > BUFSIZ) {
            #ifdef DEBUG
              Serial.println();
              Serial.println("Parsing Comment (3)");
            #endif
						state = PARSE_COMMENT;
					}
					fileHash[i] = c;
					i++;
				}
				else if (state == PARSE_DAYMASK) {
					if (j > SCHEDULECHARS) {
            #ifdef DEBUG
              Serial.println();
              Serial.println("Parsing Comment (4)");
            #endif
						state = PARSE_COMMENT;
					}
					fileSchedule[j] = c;
					j++;
				}
				//else { } // PARSE_COMMENT = do nothing
			}
		}

    #ifdef DEBUG
      Serial.println();
    #endif

		fileHash[i] = '\0';
		fileSchedule[j] = '\0';

      #ifdef DEBUG
        if (i > 0) {
          Serial.print("File Hash: ");
          Serial.println(fileHash);
        }
      #endif

		if (i <= 0) {
		  // Blank line, don't compare
		}
		else if (strcmp(fileHash, cardHash) == 0) {     // If the hash matches,
			if (j > 0) {                                  // the line has schedule information on it
        if(RTC.isrunning()){                        // and the RTC is running

          #ifdef DEBUG
            Serial.print("File Schedule: ");
            Serial.println(fileSchedule);
          #endif

          // Interpret the scheduler data
          // Scheduler data is 41 characters
          int schedule[7][2] = {0};
          for (int d = 0; d <= 6; d++){
            int offset = ( d * 5 ) + ( d * 1 );

            // There is likely a better way to do this but I can't think of it right now.
            char startHour[3] = {0};
            startHour[0] = fileSchedule[offset];
            startHour[1] = fileSchedule[offset+1];
            startHour[2] = fileSchedule[offset+2];

            char finishHour[3] = {0};
            finishHour[0] = fileSchedule[offset+4];
            finishHour[1] = fileSchedule[offset+5];
            finishHour[2] = fileSchedule[offset+6];

            #ifdef DEBUG
              Serial.print("Start Hour for Day ");
              Serial.print(d);
              Serial.print(" :");
              Serial.println(startHour);

              Serial.print("Finish Hour for Day ");
              Serial.print(d);
              Serial.print(" :");
              Serial.println(finishHour);
            #endif
            schedule[d][0] = atoi(startHour);
            schedule[d][1] = atoi(finishHour);
          }

          int dayOfWeek = now.dayOfWeek();
          int hour = now.hour();

          if ( ( hour >= schedule[dayOfWeek][0] ) && ( hour < schedule[dayOfWeek][1] ) ){
            retVal = 0;
          }
          else {
            retVal = 2;
          }
        }
        else {  // If the RTC is offline then we should unconditionally allow restricted users.
          retVal = 3;
        }
			}
			else {
				retVal = 0;
			}
		}
	}
	openFile.close();
	return retVal;
}


bool fileWrite(char *fileName, const char *logMessage, const char *logData, bool timeStamp) {
  //there was going to be a condition for SD/Server activity.
  char timestamp[22] = "\0";
  if (timeStamp) {
    DateTime now(theTime);  //thanks jeelabs
    /*Serial.print(now.year(), DEC);
    Serial.print('-');
    Serial.print(now.month(), DEC);
    Serial.print('-');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.print(": ");*/
	sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d \0", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  }
  Serial.print(timestamp);
  Serial.print(logMessage);
  Serial.println(logData);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(logMessage);
  lcd.setCursor(0, 1);
  lcd.print(logData);
  slowTimers[TIMERLCDTIME].start = theTime;

  openFile.close();
  openFile = SD.open(fileName, FILE_WRITE);
  
  if (openFile) {
    /*if (timeStamp) {
      DateTime now(theTime);  //thanks jeelabs
      openFile.print(now.year(), DEC);
      openFile.print('-');
      openFile.print(now.month(), DEC);
      openFile.print('-');
      openFile.print(now.day(), DEC);
      openFile.print(' ');
      openFile.print(now.hour(), DEC);
      openFile.print(':');
      openFile.print(now.minute(), DEC);
      openFile.print(':');
      openFile.print(now.second(), DEC);
      openFile.print(": ");
    }*/
	openFile.print(timestamp);
    openFile.print(logMessage);
    openFile.print(logData);
    openFile.print("\n");
    openFile.close();
    return true;
  }
  return false;
}

char* nextFileName(char *fileName, const char *filePrefix, const char *fileSuffix) {
//  static char logName[BUFSIZ] = "";  //60 chars should be plenty
  int i = 0;	//file index iterator
  int k;		//hex converter's file index
  int j;		//hex converter iterator
  int l = 0;	//copy iterator
  int m = 0;
  int c;		//character pointer
  
  while (filePrefix[l] != '\0') {  //copy the prefix for the file
    fileName[l] = filePrefix[l];
    l++;
  }
  c = l;
  l += LOGDIGITS;
  while (fileSuffix[m] != '\0') {  //copy the prefix for the file
    fileName[l] = fileSuffix[m];
    m++;
    l++;
  }
  fileName[l] = '\0';  //null terminate

  do {
    k = i;
    for (j = (c + LOGDIGITS - 1); j >= c; j--) { 
      fileName[j] = ("0123456789abcdef"[k & 0x000F]);
      k >>= 4;
    }    
    i++;
  } while(SD.exists(fileName));
  return fileName;
}

/*
char* hashFile(char* fileName){
  static char hashResult[(2 * HASHLENGTH) + 1];  //hex+null terminator
  Sha1.init();
  openFile.close();
  openFile = SD.open(fileName);
  if(openFile){
    Sha1.init();
    while(openFile.available()){
      Sha1.print(openFile.read());
    }
  }
  uint8_t* hash = Sha1.result();
  for(int i = 0; i < HASHLENGTH; i++) {
    hashResult[2*i] = ("0123456789abcdef"[hash[i]>>4]);
    hashResult[(2*i)+1] = ("0123456789abcdef"[hash[i]&0xf]);
  }
  hashResult[2 * HASHLENGTH] = '\0';
  return hashResult;
}
*/

