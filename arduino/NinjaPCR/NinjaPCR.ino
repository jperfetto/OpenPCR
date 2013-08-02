/*
 *  openpcr.pde - OpenPCR control software.
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
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  digitalWrite(5,LOW);
  digitalWrite(6,LOW);
  //init factory settings
  if (InitialStart()) {
    EEPROM.write(0, 100); // set contrast to 100
  }
  
  //restart detection
  boolean restarted = !(MCUSR & 1);
  MCUSR &= 0xFE;
  gpThermocycler = new Thermocycler(restarted);
  Serial.begin(4800);
  digitalWrite(5, HIGH);
}
 
bool connected = false;
bool initDone = false;
short INTERVAL_MSEC = 500;
void loop() {
  if (connected) {
    gpThermocycler->Loop();
  } else {
    checkPlugged();
  }
}

bool startLamp = false;
void checkPlugged () {
    Serial.print("pcr1.0.5");
    Serial.print("\n");
    digitalWrite(5, (startLamp)?HIGH:LOW);
    startLamp = !startLamp;
    int timeStart = millis();
    while (millis()<timeStart+INTERVAL_MSEC) {
      while (Serial.available()){
        char ch = Serial.read();
        if (ch=='a'&&!connected) {
          digitalWrite(5, LOW);
          connected = true;
        }
      }
    }
}
