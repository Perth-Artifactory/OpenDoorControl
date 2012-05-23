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
#include "SDfiles.h"
#include "control.h"
#include "aux.h"

void pollTimers() {  //see setup for the default values
//  oldMicros = microsHolder;
//  microsHolder = micros();
  for (int i = 0; i < NUMFASTTIMERS; i++) {
    while ((fastTimers[i].active) && ((micros() - fastTimers[i].start) > fastTimers[i].period)) {
      fastTimers[i].start += fastTimers[i].period;
      fastTimers[i].expire();
    }
  }
}

void theTimeIncrement() {
  theTime++;
}

void pollSlowTimers() {
  for (int i = 0; i < NUMSLOWTIMERS; i++){
    while ((slowTimers[i].active) && ((theTime - slowTimers[i].start) > slowTimers[i].period)) {
      slowTimers[i].start += slowTimers[i].period;
      slowTimers[i].expire();
    }
  }
}

void dumpLogs() {
//  if(dumpFile(logFile)){  //used to send the file to the server, now just opens a new one.
  char oldLogName[BUFSIZ];
  //stringCopy(logFile, oldLogName);
	memcpy(&oldLogName, logFile, BUFSIZ);
  fileWrite(logFile, "File Dumped: ",logFile, true);
  nextFileName(logFile, logFilePrefix, logFileSuffix);
  fileWrite(logFile, "Dumped Log: ", oldLogName, true);
  //}
}

void openSpace() {
  fileWrite(logFile, "Opening Space", "", true);

  // TODO: tell the server the space is open
  spaceOpen = true;
}

void closeSpace() {
  fileWrite(logFile, "Space closing, grace period started", "", true);
  slowTimers[TIMEREXITGRACE].start = theTime;
  slowTimers[TIMEREXITGRACE].active = true;

  blinkPin = STATUS_R;
  slowTimers[TIMERLEDBLINK].start = theTime;
  slowTimers[TIMERLEDBLINK].active = true;

  spaceOpen = false;
  guestAccess = false;
  spaceGrace = true;
}

void closeSpaceFinal() {
  fileWrite(logFile, "Closing Space", "", true);
  slowTimers[TIMEREXITGRACE].active = false;
  slowTimers[TIMERLEDBLINK].active = false;
  // TODO: tell the server the space is closed
  spaceOpen = false;
  guestAccess = false;
  spaceGrace = false;
}

void openTheDoor() {
  // Trigger the door strike
  fileWrite(logFile, "Unlocking Door", "", true);
  digitalWrite(DOORSTRIKE, HIGH);
  fastTimers[TIMERSTRIKE].start = micros();
  fastTimers[TIMERSTRIKE].active = true;
  //DoorStatus(0, 1, 0);
  digitalWrite(STATUS_G, HIGH);
}

void closeTheDoor() {
  fileWrite(logFile, "Locking Door", "", true);
  digitalWrite(DOORSTRIKE, LOW);
  fastTimers[TIMERSTRIKE].active = false;
  DoorStatusRefresh();
}

void LCDrefresh() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Artifactory Door");
  lcdDisplayTime(1);
}

void DoorBell() {
	digitalWrite(DOORBELLLED, HIGH);
	slowTimers[TIMERDOORBELL].start = theTime;
	slowTimers[TIMERDOORBELL].active = true;
}

void DoorBellRefresh() {
	digitalWrite(DOORBELLLED, LOW);
	slowTimers[TIMERDOORBELL].active = false;
}

void DoorStatus(bool r, bool g, bool b) {
	digitalWrite(STATUS_R, (r ? HIGH : LOW));
	digitalWrite(STATUS_G, (g ? HIGH : LOW));
	digitalWrite(STATUS_B, (b ? HIGH : LOW));
	slowTimers[TIMERDOORSTATUS].start = theTime;
	slowTimers[TIMERDOORSTATUS].active = true;
}

void DoorStatusRefresh() {
	digitalWrite(STATUS_R, LOW);
	digitalWrite(STATUS_G, LOW);
	digitalWrite(STATUS_B, LOW);
	slowTimers[TIMERDOORSTATUS].active = false;
  blinkCount = 0; //Resetting this to 0 here to stop potential overflows.
}

void ledBlink(){
  if ( blinkPin <= 0) { return; }

  if ( (blinkCount % 2) == 1) {
    digitalWrite(blinkPin, LOW);
  }
  else {
    digitalWrite(blinkPin, HIGH);
  }
  blinkCount++;
}
