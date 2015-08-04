/*
 *  util.cpp - OpenPCR control software.
 *  Copyright (C) 2010-2012 Josh Perfetto. All Rights Reserved.
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
#include "thermocycler.h"
#include "display.h"

const char FLOAT_PAD_FORM_STR[] PROGMEM = "%3d.%d";
const char FLOAT_FORM_STR[] PROGMEM = "%d.%d";

void sprintFloat(char* str, float val, int decimalDigits, boolean pad) {
  long factor = pow(10, decimalDigits);
  long intVal;

  if (val > 0)
    intVal = val * factor + 0.5;
  else
    intVal = val * factor - 0.5;
    
  int decimal = intVal % factor;
  int number = intVal / factor;

  if (pad)
    sprintf_P(str, FLOAT_PAD_FORM_STR, number, abs(decimal));
  else
    sprintf_P(str, FLOAT_FORM_STR, number, abs(decimal));
}

void* operator new(size_t size) {
  void* pMem = malloc(size);

#ifdef DEBUG_DISPLAY
  if (pMem == NULL) {
    gpThermocycler->GetDisplay()->SetDebugMsg("Out of Memory");  
    delay(5000);
  }
#endif

  return pMem;
}

struct __freelist
{
 size_t sz;
 struct __freelist *nx;
};

extern struct __freelist *__flp;
extern uint8_t* __brkval;

void fix28135_malloc_bug()
 {
   for (__freelist *fp = __flp, *lfp = 0; fp; fp = fp->nx)
   {
     if (((uint8_t*)fp + fp->sz + 2) == __brkval)
     {
       __brkval = (uint8_t*)fp;
       if (lfp)
         lfp->nx = 0;
       else
         __flp = 0;
       break;
     }
     lfp = fp;
   }
 }


void operator delete(void * ptr) {
  free(ptr);
  fix28135_malloc_bug();
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

char* rps(const char* progString) {
  static char buf[21];
  strcpy_P(buf, progString);
  return buf;
}


