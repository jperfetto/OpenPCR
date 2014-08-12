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
	void ReadTemp();
private:
	char SPITransfer(volatile char data);

private:
	double iTemp;
	unsigned long resistance;
	ResistorMode resistorMode;
};

#endif
