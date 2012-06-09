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
#include "RFID.h"
#include "SDfiles.h"
#include "control.h"

//#define DEBUG

void pollRFIDbuffer() {
  static int i = 0;  // position within the raw string
  static char rfidString[RFIDSTRINGLENGTH + 1];	// the raw string off the RFID reader, +null term

  while (Serial1.available() > 0) {
    rfidString[i] = Serial1.read();
    i++;

    if ((rfidString[i-1] == 0x0D) ||
		(rfidString[i-1] == 0x0A) ||
		(rfidString[i-1] == 0x03) ||
		(rfidString[i-1] == 0x02)) {
      i = 0;
    }
    if (i >= RFIDSTRINGLENGTH) {		// about to overwrite the null byte
      if (checksumValid(rfidString)) {	// check the checksum
        char cardUID[RFIDLENGTH + 1];	// copy just the card UID
        for (int j = 0; j < RFIDLENGTH; j++) {
          cardUID[j] = rfidString[j];
        }
        cardUID[RFIDLENGTH] = '\0';
        char *cardHash = hashThis(cardUID);
        checksumOk(cardHash);// the function executed when the checksum is ok
      }
      else {
        checksumfailed(rfidString);
      }
      i = 0;
    }
  }
}

void checksumOk(char *cardHash) {  // when the checksum is ok
//  Serial.print("checksum valid, hash = ");
//  Serial.println(cardHash);

	bool shouldOpenSpace = false; // Not all authentication successes should open the space.

	MemberType result = authCard(cardHash);
	if (result != 0) {
		fileWrite(logFile, "Authentication Success: ", cardHash, true);
		slowTimers[TIMERLCDTIME].start = theTime;
		lcd.setCursor(0, 1);
		switch (result) {
			case MEMBERTYPE_FULL:
				lcd.print("Member Card");
				shouldOpenSpace = true;
				openTheDoor();
				break;

			case MEMBERTYPE_RESTRICTED:
				lcd.print("Restricted Card");
				shouldOpenSpace = true;
				openTheDoor();
				break;

			case MEMBERTYPE_RESTRICTED_OUT_OF_HOURS:
				fileWrite(logFile, "Restricted card out of hours", "", true);
				DoorStatus(1, 1, 0); // Flash yellow to indicate out-of-hours access attempt
				break;

			case MEMBERTYPE_RESTRICTED_NO_RTC:
				fileWrite(logFile, "Restricted card, No RTC", "", true);
				shouldOpenSpace = true;
				openTheDoor();
				break;

			case MEMBERTYPE_ASSOCIATE:
				if (spaceOpen) {
					fileWrite(logFile, "Associate Card", "", false);
					openTheDoor();
				}
				else {
					fileWrite(logFile, "Associate card, space not open.", "", false);
					DoorStatus(1, 1, 0);
				}
				break;

			default:
				lcd.print("Ow! Ow! Ow! Ow!");	// This should never happen
				break;
		}

		if ( ( !spaceOpen ) && ( shouldOpenSpace ) ) {
			openSpace();  // does logging itself
		}
	}
	else {
		fileWrite(logFile, "Authentication Failure: ", cardHash, true);
		lcd.clear();
		lcd.setCursor(0, 0);
		//lcd.print("Who are you?!");
		lcd.print("Auth Failure");
		slowTimers[TIMERLCDTIME].start = theTime;
		DoorStatus(1, 0, 0);
	}
}

void checksumfailed(char *rfidString){  //when the checksum is ok
  fileWrite(logFile, "checksum failed: ", rfidString, true);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Read Error");
  slowTimers[TIMERLCDTIME].start = theTime;
}

bool checksumValid(char *rfidString){
  byte val = 0;
  byte highByte = 0;
  byte code[6];
  byte checksum = 0;

  for (int i = 0; i < RFIDSTRINGLENGTH; i++) {	// hex converter
    val = rfidString[i];
    if ((val >= '0') && (val <= '9')) {			// If val is between 0 and 9
      val = val - '0';							// convert to a 4 bit number
    }
	else if ((val >= 'A') && (val <= 'F')) {	// If val is between A and F
      val = 10 + val - 'A';						// Convert to a 4 bit number
    }											// We now have a 4 bit val
    if (i & 1 == 1) {							// every second char
      code[i >> 1] = ((highByte << 4) | val);	// Now we generate an 8 bit number
    }
	else {										// or
      highByte = val;							// Store the 4 bit number for the next cycle
    }
  }
  for (int i = 0; i < 5; i++) {
    checksum ^= code[i]; //byte XOR
  }
  return (checksum == code[5]);
}

MemberType authCard(char *cardHash) {
	MemberType retVal = MEMBERTYPE_NONE;

	// Checks the cardHash against the full member hash file
	if (cardInFile(fullFile, cardHash) == 0) {
		retVal = MEMBERTYPE_FULL;
	}
	else if (cardInFile(assocFile, cardHash) == 0) {
		retVal = MEMBERTYPE_ASSOCIATE;
	}
	else {
		int schedStatus = cardInFile(restFile, cardHash);

		#ifdef DEBUG
			Serial.print("Card Return Value: ");
			Serial.println(schedStatus);
		#endif

		if (schedStatus == 0) {
			retVal = MEMBERTYPE_RESTRICTED;
		}
		else if (schedStatus == 2) {
			retVal = MEMBERTYPE_RESTRICTED_OUT_OF_HOURS;
		}
		else if (schedStatus == 3) {
			retVal = MEMBERTYPE_RESTRICTED_NO_RTC;
		}
	}
	return retVal;
}
	



