/*
 *  openpcr.pde - OpenPCR control software.
 *  Copyright (C) 2010-2011 Josh Perfetto. All Rights Reserved.
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

#include <LiquidCrystal.h>
#include <EEPROM.h>

#include "pcr_includes.h"
#include "thermocycler.h"

Thermocycler* gpThermocycler = NULL;

boolean InitialStart() {
  for (int i = 0; i < 50; i++) {
    if (EEPROM.read(i) != 0xFF)
      return false;
  }
  
  return true;
}

void setup() {
  //init factory settings
  if (InitialStart()) {
    EEPROM.write(0, 140); // set contrast to 140
  }
  
  //restart detection
  boolean restarted = !(MCUSR & 1);
  MCUSR &= 0xFE;
    
  gpThermocycler = new Thermocycler(restarted);
}

void loop() {
  gpThermocycler->Loop();
}

