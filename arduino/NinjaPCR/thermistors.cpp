/*
 *  program.cpp - OpenPCR control software.
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
#include "thermistors.h"
#include "thermistor_table.h"


// lid resistance table, in Ohms
#ifdef LID_THERMISTOR_ORIGINAL
PROGMEM const unsigned int LID_RESISTANCE_TABLE[] = LID_THERMISTOR_TABLE_ORIGINAL;
#endif

//  NXFT15XH103FA 10k Ohms
#ifdef LID_THERMISTOR_NXFT15XH103FA
PROGMEM const unsigned int LID_RESISTANCE_TABLE[] = LID_THERMISTOR_TABLE_NXFT15XH103FA;
#endif

// ConstB=3,980.00
#ifdef LID_THERMISTOR_CONSTB_3980
PROGMEM const unsigned int LID_RESISTANCE_TABLE[] = LID_THERMISTOR_TABLE_CONSTB_3980;
#endif

// plate resistance table, in 0.1 Ohms
#if defined(PLATE_THERMISTOR_OPENPCR)
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = THERMISTOR_TABLE_OPENPCR;
#elif defined(PLATE_THERMISTOR_NO1)
	//No.1
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = THERMISTOR_TABLE_NO1;
#elif defined(PLATE_THERMISTOR_NO2)
//No.2
//TODO
#elif defined(PLATE_THERMISTOR_NO3)
//No.3
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = THERMISTOR_TABLE_NO3;
#elif defined(PLATE_THERMISTOR_NO4)
//No.4
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = THERMISTOR_TABLE_NO4;
#elif defined(PLATE_THERMISTOR_NO5)
//No.5
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = #define THERMISTOR_TABLE_NO5;
#elif defined(PLATE_THERMISTOR_NO6)
	//No.6
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = THERMISTOR_TABLE_NO6;
#elif defined(PLATE_THERMISTOR_NO7)
	//No.7, No.10
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = THERMISTOR_TABLE_NO7;
#elif defined(PLATE_THERMISTOR_NO8)
	//No.8
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = THERMISTOR_TABLE_NO8;
#elif defined(PLATE_THERMISTOR_NO9)
	//No.9
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = THERMISTOR_TABLE_NO9;

#elif defined(PLATE_THERMISTOR_NXFT15WF104FA2B100)
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = THERMISTOR_TABLE_NXFT15WF104FA2B100;
#else
	#error "Number of the plate thermistor is not specified!"
#endif

//spi
#define DATAOUT 11//MOSI
#define DATAIN  12//MISO
#define SPICLOCK  13//sck
#define SLAVESELECT 10//ss

//------------------------------------------------------------------------------
float TableLookup(const unsigned long lookupTable[], unsigned int tableSize, int startValue, unsigned long searchValue) {
  //simple linear search for now
  int i;
  for (i = 0; i < tableSize; i++) {
    if (searchValue >= pgm_read_dword_near(lookupTable + i))
      break;
  }

  if (i > 0) {
    unsigned long high_val = pgm_read_dword_near(lookupTable + i - 1);
    unsigned long low_val = pgm_read_dword_near(lookupTable + i);
    return i + startValue - (float)(searchValue - low_val) / (float)(high_val - low_val);
  } else {
    return startValue;
  }
}
//------------------------------------------------------------------------------
float TableLookup(const unsigned int lookupTable[], unsigned int tableSize, int startValue, unsigned long searchValue) {
  //simple linear search for now
  int i;
  for (i = 0; i < tableSize; i++) {
    if (searchValue >= pgm_read_word_near(lookupTable + i))
      break;
  }

  if (i > 0) {
    unsigned long high_val = pgm_read_word_near(lookupTable + i - 1);
    unsigned long low_val = pgm_read_word_near(lookupTable + i);
    return i + startValue - (float)(searchValue - low_val) / (float)(high_val - low_val);
  } else {
    return startValue;
  }
}

////////////////////////////////////////////////////////////////////
// Class CLidThermistor
CLidThermistor::CLidThermistor():
  iTemp(0.0) {
}

//------------------------------------------------------------------------------
void CLidThermistor::ReadTemp() {
#ifdef DEBUG_FIX_HEATER_VALUE
	iTemp = DEBUG_FIX_HEATER_VALUE;
#else
	unsigned long voltage_mv = (unsigned long) analogRead(1) * 5000 / 1024;
  resistance = voltage_mv * 2200 / (5000 - voltage_mv);
  iTemp = TableLookup(LID_RESISTANCE_TABLE, sizeof(LID_RESISTANCE_TABLE) / sizeof(LID_RESISTANCE_TABLE[0]), 0, resistance);
#endif
}

////////////////////////////////////////////////////////////////////
// Class CPlateThermistor
CPlateThermistor::CPlateThermistor():
  iTemp(0.0) {

  //spi setup
  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK,OUTPUT);
  pinMode(SLAVESELECT,OUTPUT);
  digitalWrite(SLAVESELECT,HIGH); //disable device
}
//------------------------------------------------------------------------------
#define RESISTOR_SINGLE 160000
#define RESISTOR_DOUBLE 61538
#define RESISTOR_MODE_THRESHOLD_TEMPERATURE 81.35

void CPlateThermistor::ReadTemp() {
	digitalWrite(SLAVESELECT, LOW);

	//read data
	while (digitalRead(DATAIN)) {}

	uint8_t spiBuf[4];
	memset(spiBuf, 0, sizeof(spiBuf));

	digitalWrite(SLAVESELECT, LOW);
	for (int i = 0; i < 4; i++)
		spiBuf[i] = SPITransfer(0xFF);

	unsigned long conv = (((unsigned long) spiBuf[3] >> 7) & 0x01)
			+ ((unsigned long) spiBuf[2] << 1)
			+ ((unsigned long) spiBuf[1] << 9)
			+ (((unsigned long) spiBuf[0] & 0x1F) << 17); //((spiBuf[0] & 0x1F) << 16) + (spiBuf[1] << 8) + spiBuf[2];

	unsigned long adcDivisor = 0x1FFFFF;
	float voltage = (float) conv * 5.0 / adcDivisor;

	unsigned int convHigh = (conv >> 16);

	digitalWrite(SLAVESELECT, HIGH);
	unsigned long voltage_mv = voltage * 1000;

	unsigned long resistor = (resistorMode==TEMP_LOW)? RESISTOR_SINGLE:RESISTOR_DOUBLE;

	/*
	resistance = (voltage_mv * resistor) / (5000 - voltage_mv); // in hecto ohms
	iTemp = TableLookup(PLATE_RESISTANCE_TABLE,
			sizeof(PLATE_RESISTANCE_TABLE) / sizeof(PLATE_RESISTANCE_TABLE[0]),
			PLATE_RESISTANCE_TABLE_OFFSET, resistance);

	if (resistorMode==TEMP_LOW  && iTemp>RESISTOR_MODE_THRESHOLD_TEMPERATURE) {
		// LOW -> HIGH
		resistorMode = TEMP_HIGH;
		digitalWrite(PIN_LID_RESISTOR_SWITCH, HIGH);
	}
	else if (resistorMode==TEMP_HIGH  && iTemp<RESISTOR_MODE_THRESHOLD_TEMPERATURE) {
		// HIGH -> LOW
		resistorMode = TEMP_HIGH;
		digitalWrite(PIN_LID_RESISTOR_SWITCH, LOW);
	}
	*/

	resistance = voltage_mv * RESISTOR_SINGLE / (5000 - voltage_mv); // in hecto ohms
	//resistance = voltage_mv * 22000 / (5000 - voltage_mv); // in hecto ohms
	iTemp = TableLookup(PLATE_RESISTANCE_TABLE, sizeof(PLATE_RESISTANCE_TABLE) / sizeof(PLATE_RESISTANCE_TABLE[0]), PLATE_RESISTANCE_TABLE_OFFSET, resistance);

}
//------------------------------------------------------------------------------
char CPlateThermistor::SPITransfer(volatile char data) {
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
  {};
  return SPDR;                    // return the received byte
}
