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

//reed switch
//close button
//door bell
//guest access
/*
these interrupts simply set flags that get checked in the main loop
*/
/*
we're putting the buttons on:
analogue 8-15,
digital 62-69,
PINK,
PCINT16-32,
PCMSK2,
PCINT2_vect
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include "OpenDoorControl.h"
#include "SDfiles.h"
#include "control.h"

unsigned char interruptFlags = 0;
//unsigned long doorbellPressTime = 0;  //we'll use the sign bit as a debounce-disabled flag.
unsigned long doorbellChangeTime = 0;
unsigned long reedswitchChangeTime = 0;
unsigned long guestAccessChangeTime = 0;
unsigned long lockupChangeTime = 0;


ISR(PCINT2_vect) {  // We ARE going to de-bounce the buttons, the loop is NOT slow enough.
	static uint8_t inputStates = 0xFF;
	static uint8_t changeFlags = 0;
	static uint8_t oldInputStates = 0xFF;

	inputStates = PINK & PCMSK2;  //pink & pcmsk2
	changeFlags = oldInputStates ^ inputStates;

	if ((DOORBELLBIT & changeFlags) != 0x00) {
		if ((DOORBELLBIT & inputStates) == 0x00) {	// button press
			doorbellChangeTime = micros();
		}
		else {										// button release
			if ((micros() - doorbellChangeTime) > DEBOUNCEDELAY) {
				interruptFlags |= DOORBELLBIT;
				doorbellChangeTime = micros();
			}
		}
	}

	if ((REEDSWITCHBIT & changeFlags) != 0x00) {
		interruptFlags |= REEDSWITCHBIT;
		reedswitchState = ((REEDSWITCHBIT & inputStates) != 0x00);  //open = true
		reedswitchChangeTime = theTime;
	}

	if ((GUESTOKBIT & changeFlags) != 0x00) {
		if ((GUESTOKBIT & inputStates) == 0x00) {	// button press
			guestAccessChangeTime = micros();
		}
		else {										// button release
			if ((micros() - guestAccessChangeTime) > DEBOUNCEDELAY) {
				interruptFlags |= GUESTOKBIT;
				guestAccessChangeTime = micros();
			}
		}
	}

	if((LOCKUPBIT & changeFlags) != 0x00) {
		if ((LOCKUPBIT & inputStates) == 0x00) {	// button press
			lockupChangeTime = micros();
		}
		else {										// button release
			if ((micros() - lockupChangeTime) > DEBOUNCEDELAY) {
				interruptFlags |= LOCKUPBIT;
				lockupChangeTime = micros();				
			}
		}
	}

	oldInputStates = inputStates;
}




void runInterruptServices() {  //called from loop, take your time.
  if ((DOORBELLBIT & interruptFlags) != 0) {
    interruptFlags &= ~DOORBELLBIT;

    fileWrite(logFile, "Door Bell!", "", true);

    if (spaceGrace) {				// Special condition if the space is within the 120 second post-lockup grace period.
    	fileWrite(logFile, "Space opened within grace period.","",true);
    	slowTimers[TIMEREXITGRACE].active = false;	// Stop the grace period timer
 		fastTimers[TIMERLEDFADER].active = false;	// Stop the LED blink timer
 		spaceGrace = false;							// Disable the grace period mode and
 		openSpace();								// Reset the space to open
    	openTheDoor();								// Open the door
    }
    else {
		if (spaceOpen) {
			if (guestAccess) {			// If the space is open and guest access is enabled
				openTheDoor();			// just open the door
			}
			else {						// if the space is open without guest access being enabled
				DoorBell();				// sound the doorbell
				DoorStatus(0, 0, 1);	// and illuminate the blue door status LED
			}
		}
		else {							// finally, if the space isnt open				
			DoorStatus(1, 0, 0);		// illuminate the red door status LED
		}
	}
  }

  if ((REEDSWITCHBIT & interruptFlags) != 0) {
    interruptFlags &= ~REEDSWITCHBIT;

    if (reedswitchState) {
      fileWrite(logFile, "door reedswitch", "opened", true);
    }
    else {
      fileWrite(logFile, "door reedswitch", "closed", true);
    }
  }

  if ((GUESTOKBIT & interruptFlags) != 0) {
	interruptFlags &= ~GUESTOKBIT;
/*
	Serial.print("Guest switch = ");
	Serial.print(guestAccessChangeTime);
	Serial.print("    ");
	Serial.print(micros());
	Serial.print("\r\n");
*/
		if (!spaceOpen) {
			fileWrite(logFile, "Cannot Comply", "Space is Locked", true);
		}
		else {
			guestAccess = !guestAccess;

			if (guestAccess) {
				fileWrite(logFile, "Guests", "Allowed", true);
			}
			else {
				fileWrite(logFile, "Guests", "Denied", true);
			}
		}
//    postStateSpace(false);  //tell the server right now.
  }

	if ((LOCKUPBIT & interruptFlags) != 0) {
		interruptFlags &= ~LOCKUPBIT;
		if (spaceOpen) {
			fileWrite(logFile, "Lockup pressed", "", true);
			closeSpace();
		}
		else {
			fileWrite(logFile, "Space already locked", "", true);
		}
	}

	// Status indicators LED settings go here.
	digitalWrite(LOCKUPLED, (spaceOpen ? LOW : HIGH));
	digitalWrite(GUESTOKLED, (guestAccess ? HIGH : LOW));
	digitalWrite(DOORBELLLED, (guestAccess ? HIGH : LOW));
}

