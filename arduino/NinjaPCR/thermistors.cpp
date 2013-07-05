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

// lid resistance table, in Ohms
PROGMEM const unsigned int LID_RESISTANCE_TABLE[] = {  
  32919, 31270, 29715, 28246, 26858, 25547, 24307, 23135, 22026, 20977,
  19987, 19044, 18154, 17310, 16510, 15752, 15034, 14352, 13705, 13090,
  12507, 11953, 11427, 10927, 10452, 10000, 9570, 9161, 8771, 8401,
  8048, 7712, 7391, 7086, 6795, 6518, 6254, 6001, 5761, 5531, 5311,
  5102, 4902, 4710, 4528, 4353, 4186, 4026, 3874, 3728, 3588,
  3454, 3326, 3203, 3085, 2973, 2865, 2761, 2662, 2567, 2476,
  2388, 2304, 2223, 2146, 2072, 2000, 1932, 1866, 1803, 1742,
  1684, 1627, 1573, 1521, 1471, 1423, 1377, 1332, 1289, 1248,
  1208, 1170, 1133, 1097, 1063, 1030, 998, 968, 938, 909,
  882, 855, 829, 805, 781, 758, 735, 714, 693, 673,
  653, 635, 616, 599, 582, 565, 550, 534, 519, 505,
  491, 478, 465, 452, 440, 428, 416, 405, 395, 384,
  374, 364, 355, 345, 337 };

// plate resistance table, in 0.1 Ohms
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = {
  3364790, 3149040, 2948480, 2761940, 2588380, 2426810, 2276320, 2136100, 2005390, 1883490,
  1769740, 1663560, 1564410, 1471770, 1385180, 1304210, 1228470, 1157590, 1091220, 1029060,
  970810, 916210, 865010, 816980, 771900, 729570, 689820, 652460, 617360, 584340,
  553290, 524070, 496560, 470660, 446260, 423270, 401590, 381150, 361870, 343680,
  326500, 310290, 294980, 280520, 266850, 253920, 241700, 230130, 219180, 208820,
  199010, 189710, 180900, 172550, 164630, 157120, 149990, 143230, 136810, 130720,
  124930, 119420, 114190, 109220, 104500, 100000, 95720, 91650, 87770, 84080,
  80570, 77220, 74020, 70980, 68080, 65310, 62670, 60150, 57750, 55450,
  53260, 51170, 49170, 47250, 45430, 43680, 42010, 40410, 38880, 37420,
  36020, 34680, 33400, 32170, 30990, 29860, 28780, 27740, 26750, 25790,
  24880, 24000, 23160, 22350, 21570, 20830, 20110, 19420, 18760, 18130,
  17520, 16930, 16370, 15820, 15300, 14800, 14320, 13850, 13400, 12970,
  12550, 12150, 11770, 11400, 11040, 10700, 10370, 10050, 9738, 9441,
  9155, 8878, 8612, 8354, 8106, 7866, 7635, 7412, 7196, 6987, 6786,
  6591, 6403, 6222, 6046, 5876 };
  
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
  unsigned long voltage_mv = (unsigned long)analogRead(1) * 5000 / 1024;
  unsigned long resistance = voltage_mv * 2200 / (5000 - voltage_mv);
  
  iTemp = TableLookup(LID_RESISTANCE_TABLE, sizeof(LID_RESISTANCE_TABLE) / sizeof(LID_RESISTANCE_TABLE[0]), 0, resistance);
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
void CPlateThermistor::ReadTemp() {
  digitalWrite(SLAVESELECT, LOW);

  //read data
  while(digitalRead(DATAIN)) {}
  
  uint8_t spiBuf[4];
  memset(spiBuf, 0, sizeof(spiBuf));

  digitalWrite(SLAVESELECT, LOW);  
  for(int i = 0; i < 4; i++)
    spiBuf[i] = SPITransfer(0xFF);

  unsigned long conv = (((unsigned long)spiBuf[3] >> 7) & 0x01) + ((unsigned long)spiBuf[2] << 1) + ((unsigned long)spiBuf[1] << 9) + (((unsigned long)spiBuf[0] & 0x1F) << 17); //((spiBuf[0] & 0x1F) << 16) + (spiBuf[1] << 8) + spiBuf[2];
  
  unsigned long adcDivisor = 0x1FFFFF;
  float voltage = (float)conv * 5.0 / adcDivisor;

  unsigned int convHigh = (conv >> 16);
  
  digitalWrite(SLAVESELECT, HIGH);
  
  unsigned long voltage_mv = voltage * 1000;
  unsigned long resistance = voltage_mv * 22000 / (5000 - voltage_mv); // in hecto ohms
 
  iTemp = TableLookup(PLATE_RESISTANCE_TABLE, sizeof(PLATE_RESISTANCE_TABLE) / sizeof(PLATE_RESISTANCE_TABLE[0]), -40, resistance);
}
//------------------------------------------------------------------------------
char CPlateThermistor::SPITransfer(volatile char data) {
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
  {};
  return SPDR;                    // return the received byte
}
