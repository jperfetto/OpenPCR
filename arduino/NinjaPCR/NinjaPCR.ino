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
//#include <SoftwareSerial.h>

//SoftwareSerial mySerial(8,9); // RX, TX

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
    EEPROM.write(0, 100); // set contrast to 100
  }
  
  //restart detection
  boolean restarted = !(MCUSR & 1);
  MCUSR &= 0xFE;
  gpThermocycler = new Thermocycler(restarted);
  //mySerial.begin(4800);
  Serial.begin(4800);
  /*
  pinMode(7, OUTPUT);
  digitalWrite(7,LOW);
  */
}
 
bool connected = false;
bool initDone = false;
short INTERVAL_MSEC = 750;
void loop() {
  if (connected) {
    gpThermocycler->Loop();
  } else {
    checkPlugged();
  }
}



void checkPlugged () {
    Serial.write("pcr");
    Serial.write("1.0.5");
    Serial.print("\n");
    int timeStart = millis();
    while (millis()<timeStart+INTERVAL_MSEC) {
      while (Serial.available()){
        char ch = Serial.read();
        if (ch=='a'&&!connected) {
          connected = true;
        }
      }
    }
}
