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

#if 0 // Brett's v1
bool cardInFile(char *inputFile, char *cardHash) {
  openFile.close();  //just in case

  openFile = SD.open(inputFile);
  bool retVal = false;
  char line[BUFSIZ];
  int i = 0;

  while((retVal == false) && (openFile.available())){
    //parse file contents.
    char c = ' ';  //I can't think of a better defualt to enter the loop with
    while((c != '\n') /*&& (c != COMMENTCHAR)*/ && (openFile.available()) /*&& (i < BUFSIZ)*/){


      if (i == BUFSIZ) {
//        Serial.println("buffer overrun");
        i=0;
        while((openFile.peek() != '\n') && (openFile.available())){  //seek the end of the line
          openFile.read();
        }
      }
      else if(c == COMMENTCHAR) {
//        Serial.println("comment begins");
        while((openFile.peek() != '\n') && (openFile.available())){  //seek the end of the line
          openFile.read();
        }
      }
      else if((c != ' ')&&(c != '\t')) {
//        Serial.print("*");
        line[i] = c;
        i++;
      }

      c = openFile.read();
//      Serial.print(c);

    }

    line[i] = '\0';
    //now we can have different lengths of hash
//    Serial.print("comparing Strings: ");
//    Serial.print(line);
//    Serial.print(" , ");
//    Serial.println(cardHash);

//    if(stringCompare(line, cardHash)){
//      retVal = true;
//    }
	if (strcmp(line, cardHash) == 0) {
		retVal = true;
	}
    i = 0;
    
  }
  openFile.close();
  return retVal;
}
#endif

typedef enum {
	//PARSE_BEGIN = 0,
	PARSE_HASH = 1,
	PARSE_DAYMASK,
	PARSE_COMMENT
} ParseState;

uint8_t cardInFile(char *inputFile, char *cardHash) {
	if (strlen(cardHash) != HASHLENGTH) {
		//	Serial.println("Noise");
		return 0;
	}

	openFile.close();  // just in case

	openFile = SD.open(inputFile);
	uint8_t retVal = 0;
	char fileHash[BUFSIZ];
	int i;
	char fileDay[BUFSIZ];
	int j;
	ParseState state;

	while ((retVal == false) && (openFile.available())) {	// parse file contents:
		i = 0;
		j = 0;
		state = PARSE_HASH;
		char c = ' ';  // I can't think of a better defualt to enter the loop with
		while ((c != '\n') /*&& (c != COMMENTCHAR)*/ && (openFile.available()) /*&& (i < BUFSIZ)*/) {
			c = openFile.read();

			if (c == COMMENTCHAR) {
				state = PARSE_COMMENT;
			}
			else if ((c == ' ') || (c == '\t')) {
				if ((state == PARSE_HASH) && (i > 0)) {
					state = PARSE_DAYMASK;
				}
				if ((state == PARSE_DAYMASK) && (j > 0)) {
					state = PARSE_COMMENT;
				}
			}
			else {
				if (state == PARSE_HASH) {
					if (i > BUFSIZ) {
						state = PARSE_COMMENT;
					}
					fileHash[i] = c;
					i++;
				}
				else if (state == PARSE_DAYMASK) {
					if (j > BUFSIZ) {
						state = PARSE_COMMENT;
					}
					fileDay[j] = c;
					j++;
				}
				//else { } // PARSE_COMMENT = do nothing
			}
		}

		fileHash[i] = '\0';
		fileDay[j] = '\0';

		if (i <= 0) {
			// Blank line, don't compare
		}
		else if (strcmp(fileHash, cardHash) == 0) {
			if (j > 0) {
				retVal = (uint8_t)(atoi(fileDay) & 0x7f);
				//if ((retVal == INT_MAX) || (retVal == INT_MIN)) {
				//	retVal = 0;
				//}
			}
			else {
				retVal = 0x7f;
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

