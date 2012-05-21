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

#ifndef GENERAL_H
#define GENERAL_H

typedef struct {
  bool active;  //this was going to be a funciton call, this is faster.
  void (*expire)();
  unsigned long period;
  unsigned long start;
} timer;


char* hashThis(char* inputData);
/*
bool stringCompare(char* string1, char* string2);
int stringLength(char* string);
void stringCopy(char* stringSource, char* stringDest);
*/

#endif /* GENERAL_H */

