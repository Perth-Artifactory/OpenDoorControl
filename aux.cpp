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

#include "aux.h"
#include "OpenDoorControl.h"
#include <Wire.h>
#include "SDfiles.h"
#include <stdio.h>

void auxSetup() { 
  lcd.begin(16, 2);
  Wire.begin();
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("Artifactory Door");
  lcd.setCursor(0, 1);
  lcd.print("  Booting");
}
void fetchTime() {
  RTC.begin();
  if(RTC.isrunning()){
    fileWrite(logFile, "RTC poll started.", "", true);
    DateTime now = RTC.now();    //when is the RTC epoch?
    unsigned long diffTime = theTime;
    fileWrite(logFile, "RTC online.", "", true);
    theTime = (unsigned long)now.unixtime();
    fileWrite(logFile, "Time Adjusted", "", true);
    diffTime = theTime - diffTime;
    for(int i = 0; i < NUMSLOWTIMERS; i++){
      slowTimers[i].start +=diffTime;
    }
  }else{
    fileWrite(logFile, "RTC not online", "", true);
  }
}

void lcdDisplayTime(int line) {
  DateTime now(theTime);  //thanks jeelabs
  lcd.setCursor(0, line);
	char output[17];
	sprintf(output, "%04d-%02d-%02d %02d:%02d\0", now.year(), now.month(), now.day(), now.hour(), now.minute());
	lcd.print(output);
/*
  lcd.print(now.year(), DEC);
  lcd.print('-');
  lcd.print(now.month(), DEC);
  lcd.print('-');
  lcd.print(now.day(), DEC);
  lcd.print(' ');
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  lcd.print(now.minute(), DEC);
*/
}
