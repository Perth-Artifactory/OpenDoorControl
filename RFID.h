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

#ifndef RFID_H
#define RFID_H

/*
 In order of privledges:
  Full Members			All times
  Restricted Members	Day mask
  Associate Members		if spaceOpen
  Guests				if guestAccess
*/
typedef enum {
	MEMBERTYPE_NONE = 0,
	MEMBERTYPE_GUEST,
	MEMBERTYPE_ASSOCIATE,
	MEMBERTYPE_RESTRICTED,
  MEMBERTYPE_RESTRICTED_OUT_OF_HOURS,
  MEMBERTYPE_RESTRICTED_NO_RTC,
	MEMBERTYPE_FULL = 0xff
} MemberType;

void pollRFIDbuffer();
void checksumOk(char* cardHash);
void checksumfailed(char* rfidString);
bool checksumValid(char* rfidString);
MemberType authCard(char* cardHash);

#endif /* RFID_H */

