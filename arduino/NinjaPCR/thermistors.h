/*
 *  thermistors.h - OpenPCR control software.
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

#ifndef _LID_THERMISTOR_H_
#define _LID_THERMISTOR_H_
#define PIN_LID_RESISTOR_SWITCH 8

/* Use original ohm-to-celsius table */
//#define LID_THERMISTOR_ORIGINAL
/* Use ohm-to-celsius table for NXFT15XH103FA */
//#define LID_THERMISTOR_NXFT15XH103FA
#define LID_THERMISTOR_CONSTB_3980

// #define PLATE_THERMISTOR_OPENPCR
// #define PLATE_THERMISTOR_NO1
// #define PLATE_THERMISTOR_NO2
// #define PLATE_THERMISTOR_NO3
// #define PLATE_THERMISTOR_NO4
// #define PLATE_THERMISTOR_NO5
// #define PLATE_THERMISTOR_NO6
// #define PLATE_THERMISTOR_NO7
// #define PLATE_THERMISTOR_NO8
//#define PLATE_THERMISTOR_NO9
#define PLATE_THERMISTOR_NXFT15WF104FA2B100

#define LID_RESISTANCE_TABLE_OFFSET 0
#define PLATE_RESISTANCE_TABLE_OFFSET -10

#define DEBUG_THERMISTORS //Send raw voltage values of thermistors by serial


/* For debug */
//#define DEBUG_FORCE_STOP_LID_HEATER
#define DEBUG_FIX_HEATER_VALUE 105

class CLidThermistor {
public:
  CLidThermistor();
  double& GetTemp() { return iTemp; }
  unsigned long& GetResistance() { return resistance; }
  void ReadTemp();
  
private:
  double iTemp;
  unsigned long resistance;
};

typedef enum {
	TEMP_LOW,
	TEMP_HIGH
} ResistorMode;

class CPlateThermistor {
public:
	CPlateThermistor();
	double& GetTemp() {
		return iTemp;
	}
	unsigned long& GetResistance() {
		return resistance;
	}
	/*
	ResistorMode GetResistorMode () {
		return resistorMode;
	}
	*/
	void ReadTemp();
private:
	char SPITransfer(volatile char data);

private:
	double iTemp;
	unsigned long resistance;
	ResistorMode resistorMode;
};

#endif
