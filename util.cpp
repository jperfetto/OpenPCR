/*
 *	util.cpp - OpenPCR control software.
 *  Copyright (C) 2010 Josh Perfetto. All Rights Reserved.
 *
 *  OpenPCR control software is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenPCR control software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pcr_includes.h"

void sprintFloat(char* str, float val, int decimalDigits) {
  int number = (int)val;
  int decimal = (abs(val) - abs(number)) * pow(10, decimalDigits);
  sprintf(str, "%3d.%d", number, decimal);
}

void* operator new(size_t size) {
  return malloc(size);
}

void operator delete(void * ptr) {
  free(ptr);
}

void __cxa_pure_virtual(void) {};

unsigned short htons(unsigned short val) {
  return val << 8 + (byte)val;
}

double UpdatePID(SPid * pid, double error, double position) {
  double pTerm, dTerm, iTerm;
  pTerm = pid->pGain * error; 
  // calculate the proportional term
  // calculate the integral state with appropriate limiting
  pid->iState += error;
  
  if (pid->iState > pid->iMax)
    pid->iState = pid->iMax;
  else if (pid->iState < pid->iMin)
    pid->iState = pid->iMin;
  
  iTerm = pid->iGain * pid->iState; // calculate the integral term
  dTerm = pid->dGain * (position - pid->dState);
  pid->dState = position;
  return pTerm + iTerm - dTerm;
}

