/*
 *	display.h - OpenPCR control software.
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

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <LiquidCrystal.h>
#include "thermocycler.h"

class Cycle;

class Display {
public:
  Display(Thermocycler& thermocycler);
  
  void SetContrast(uint8_t contrast);
  void SetDisplayCycle(Cycle* pDisplayCycle) { ipDisplayCycle = pDisplayCycle; }
  void SetDebugMsg(char* szDebugMsg);
  void Update();
  
private:
  char* ThermalStateString(Thermocycler::ThermalState state);
  
private:
  LiquidCrystal iLcd;
  char iszDebugMsg[21];
  Thermocycler& iThermocycler;
  Cycle* ipDisplayCycle;
  Thermocycler::ProgramState iLastState;
  unsigned long iLastReset;
  uint8_t iContrast;
};

#endif

