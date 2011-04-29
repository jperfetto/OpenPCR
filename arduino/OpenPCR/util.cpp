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
 *  You should have received a copy of the GNU General Public License along with
 *  the OpenPCR control software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pcr_includes.h"

void sprintFloat(char* str, float val, int decimalDigits) {
  long factor = pow(10, decimalDigits);
  long intVal = val * factor + 0.5;
  int decimal = intVal % factor;
  int number = intVal / factor;

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

double absf(double val) {
  if (val < 0)
    return val * -1;
  else
    return val;
}
