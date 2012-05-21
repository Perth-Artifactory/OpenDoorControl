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

#include "general.h"
#include "OpenDoorControl.h"
//#include "Sha/sha1.h"

char* hashThis(char *inputData) {  //input data must be null terminated
/*
  static char hashResult[(2 * HASHLENGTH) + 1];  //hex+null terminator
  Sha1.init();
  Sha1.print(inputData);
  uint8_t* hash = Sha1.result();
  for(int i = 0; i < HASHLENGTH; i++) {
    hashResult[2*i] = ("0123456789abcdef"[hash[i]>>4]);
    hashResult[(2*i)+1] = ("0123456789abcdef"[hash[i]&0xf]);
  }
  hashResult[2 * HASHLENGTH] = '\0';
  return hashResult;
*/
	return inputData;
}

/*
bool stringCompare(char* string1, char* string2){
  boolean equal = (stringLength(string1) == stringLength(string2));
  while((equal == true)&&(*string1 !='\0')){  //the end of one implies the end of the other.
    equal = (*string1 == *string2);
    string1++;
    string2++;
  }
  return equal;
}

int stringLength(char* string){
  int i = 0; 
  while(*string != '\0'){
    string++;
    i++;
  }
  return i;
}

void stringCopy(char* stringSource, char* stringDest){
  while(*stringSource != '\0'){
    *stringDest = *stringSource;
    stringDest++;
    stringSource++;
  }
  *stringDest = '\0';
}

*/
