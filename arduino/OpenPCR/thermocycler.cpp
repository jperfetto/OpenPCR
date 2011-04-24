/*
 *	thermocycler.cpp - OpenPCR control software.
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
#include "thermocycler.h"

#include "display.h"
#include "program.h"
#include "serialcontrol.h"
#include "../Wire/Wire.h"
#include <avr/pgmspace.h>

//constants
// in 0.1 Ohms
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

// in Ohms
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
  
// I2C address for MCP3422 - base address for MCP3424
#define MCP3422_ADDRESS 0X68

// fields in configuration register
#define MCP342X_GAIN_FIELD 0X03 // PGA field
#define MCP342X_GAIN_X1    0X00 // PGA gain X1
#define MCP342X_GAIN_X2    0X01 // PGA gain X2
#define MCP342X_GAIN_X4    0X02 // PGA gain X4
#define MCP342X_GAIN_X8    0X03 // PGA gain X8

#define MCP342X_RES_FIELD  0X0C // resolution/rate field
#define MCP342X_RES_SHIFT  2    // shift to low bits
#define MCP342X_12_BIT     0X00 // 12-bit 240 SPS
#define MCP342X_14_BIT     0X04 // 14-bit 60 SPS
#define MCP342X_16_BIT     0X08 // 16-bit 15 SPS
#define MCP342X_18_BIT     0X0C // 18-bit 3.75 SPS

#define MCP342X_CONTINUOUS 0X10 // 1 = continuous, 0 = one-shot

#define MCP342X_CHAN_FIELD 0X60 // channel field
#define MCP342X_CHANNEL_1  0X00 // select MUX channel 1
#define MCP342X_CHANNEL_2  0X20 // select MUX channel 2
#define MCP342X_CHANNEL_3  0X40 // select MUX channel 3
#define MCP342X_CHANNEL_4  0X60 // select MUX channel 4

#define MCP342X_START      0X80 // write: start a conversion
#define MCP342X_BUSY       0X80 // read: output not ready

#define DATAOUT 11//MOSI
#define DATAIN  12//MISO 
#define SPICLOCK  13//sck
#define SLAVESELECT 10//ss

#define CYCLE_START_TOLERANCE 0.2
#define LID_START_TOLERANCE 2.0

#define PLATE_PID_FAR_P 800
#define PLATE_PID_FAR_I 2
#define PLATE_PID_FAR_D 5

#define PLATE_PID_NEAR_P 800
#define PLATE_PID_NEAR_I 100
#define PLATE_PID_NEAR_D 10

//public
Thermocycler::Thermocycler():
  ipDisplay(NULL),
  ipProgram(NULL),
  ipSerialControl(NULL),
  iProgramState(EOff),
  iThermalState(EHolding),
  ipCurrentStep(NULL),
  iThermalDirection(OFF),
  iPeltierPwm(0),
  iLidPwm(0),
  iPlateTemp(0.0),
  iLidTemp(0.0),
  iCycleStartTime(0),
  iRamping(true),
  iPlatePid(&iPlateTemp, &iPeltierPwm, &iTargetPlateTemp, PLATE_PID_FAR_P, PLATE_PID_FAR_I, PLATE_PID_FAR_D, DIRECT),
  iLidPid(&iLidTemp, &iLidPwm, &iTargetLidTemp, 200, 0.2, 60, DIRECT),
  iTargetLidTemp(110) {
    
  ipDisplay = new Display(*this);
  ipSerialControl = new SerialControl(*this);
  
  //init pins
  pinMode(15, INPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  
  //spi pins
  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK,OUTPUT);
  pinMode(SLAVESELECT,OUTPUT);
  digitalWrite(SLAVESELECT,HIGH); //disable device 
  
    // SPCR = 01010000
  //interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
  //sample on leading edge of clk,system clock/4 rate (fastest)
  int clr;
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<4);
  clr=SPSR;
  clr=SPDR;
  delay(10); 

  iPlatePid.SetOutputLimits(-1023, 1023);
  iPlatePid.SetMode(AUTOMATIC);
  iLidPid.SetOutputLimits(0, 255);
  iLidPid.SetMode(AUTOMATIC);
  
  // Peltier PWM
  TCCR1A |= (1<<WGM11) | (1<<WGM10);
  TCCR1B = _BV(CS21);
  
  // Lid PWM
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS22);
  
  // Set contrast
  analogWrite(5, 10);
}

Thermocycler::~Thermocycler() {
  delete ipSerialControl;
  delete ipDisplay;
}

// control
void Thermocycler::SetProgram(Cycle* pProgram, Cycle* pDisplayCycle) {
  Stop();
  
  delete ipProgram;
  ipProgram = pProgram;
  ipDisplay->SetDisplayCycle(pDisplayCycle);
}

void Thermocycler::Stop() {
  if (iProgramState != EOff)
    iProgramState = EStopped;
}

PcrStatus Thermocycler::Start() {
  if (ipProgram == NULL)
    return ENoProgram;
//  if (iProgramState == EOff) // DANGEROUS TEMP HACK, REMOVE ONCE SERIAL CONTROL IN PLACE
  //  return ENoPower;
  
  iProgramState = ERunning;
  iThermalState = EHolding;
  iThermalDirection = OFF;
  iPeltierPwm = 0;
  
  ipProgram->BeginIteration();
  iRamping = true;
  ipCurrentStep = ipProgram->GetNextStep();
  SetPlateTarget(ipCurrentStep->GetTemp());
  return ESuccess;
}

// internal
void Thermocycler::Loop() {
  CheckPower();
 
  ReadPlateTemp();
  ReadLidTemp(); 

  //update program
  if (iProgramState == ERunning) {
    if (iRamping && abs(ipCurrentStep->GetTemp() - iPlateTemp) <= CYCLE_START_TOLERANCE) {
      iRamping = false;
      iCycleStartTime = millis();
      if (iThermalDirection == COOL && iTargetPlateTemp > 20) {
        iPlatePid.ResetI();
        SetPlateTarget(ipCurrentStep->GetTemp());
      }
      
    } else if (!iRamping && !ipCurrentStep->IsFinal() && millis() - iCycleStartTime > (unsigned long)ipCurrentStep->GetDuration() * 1000) {
      float prevTemp = ipCurrentStep->GetTemp();
      ipCurrentStep = ipProgram->GetNextStep();

      if (ipCurrentStep == NULL) {
        iProgramState = EFinished;
      } else {
        SetPlateTarget(ipCurrentStep->GetTemp());
        if (prevTemp != ipCurrentStep->GetTemp())
          iRamping = true;
      }
    }
  }
 
  ControlPeltier();
  ControlLid();
  
  ipDisplay->Update();  
}

void Thermocycler::CheckPower() {
  float voltage = analogRead(0) * 5.0 / 1024 * 10 / 3; // 10/3 is for voltage divider
  boolean externalPower = digitalRead(A0); //voltage > 7.0;
  if (externalPower && iProgramState == EOff) {
    iProgramState = EStopped;
  } else if (!externalPower && iProgramState != EOff) {
    Stop();
    iProgramState = EOff;
  }
}

//private

void Thermocycler::ReadLidTemp() {
  unsigned long voltage_mv = (unsigned long)analogRead(1) * 5000 / 1024;
  unsigned long resistance = voltage_mv * 2200 / (5000 - voltage_mv);
  
  iLidTemp = TableLookup(LID_RESISTANCE_TABLE, sizeof(LID_RESISTANCE_TABLE) / sizeof(LID_RESISTANCE_TABLE[0]), 0, resistance);
}

char spi_transfer(volatile char data)
{
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
  {
  };
  return SPDR;                    // return the received byte
}


void Thermocycler::ReadPlateTemp() {
  byte eeprom_output_data;
  byte eeprom_input_data=0;
  byte clr;
  int address=0;
  //data buffer
  char buffer [128]; 

  digitalWrite(SLAVESELECT, LOW);

  //read data
  while(digitalRead(DATAIN)) {
  }
  
  char buf[32];
  uint8_t spiBuf[4];
  memset(spiBuf, 0, sizeof(spiBuf));

  digitalWrite(SLAVESELECT, LOW);  
  for(int i = 0; i < 4; i++)
    spiBuf[i] = spi_transfer(0xFF);

  unsigned long conv = (((unsigned long)spiBuf[3] >> 7) & 0x01) + ((unsigned long)spiBuf[2] << 1) + ((unsigned long)spiBuf[1] << 9) + (((unsigned long)spiBuf[0] & 0x1F) << 17); //((spiBuf[0] & 0x1F) << 16) + (spiBuf[1] << 8) + spiBuf[2];
  
  unsigned long adcDivisor = 0x1FFFFF;
  float voltage = (float)conv * 5.0 / adcDivisor;

  unsigned int convHigh = (conv >> 16);
  
  digitalWrite(SLAVESELECT, HIGH);
  
  unsigned long voltage_mv = voltage * 1000;
  unsigned long resistance = voltage_mv * 22000 / (5000 - voltage_mv); // in hecto ohms
 
  iPlateTemp = TableLookup(PLATE_RESISTANCE_TABLE, sizeof(PLATE_RESISTANCE_TABLE) / sizeof(PLATE_RESISTANCE_TABLE[0]), -40, resistance);
}

void Thermocycler::ControlPeltier() {
  ThermalDirection newDirection = HEAT;
  int newPwm = 0;
  
  if (iProgramState == ERunning) {
    //PID
    iPlatePid.Compute();
    
    newPwm = abs(iPeltierPwm);
    if (iPeltierPwm > 0)
      newDirection = HEAT;
    else if (iPeltierPwm < 0)
      newDirection = COOL; 
    else
      newDirection = OFF;
  }

  iThermalDirection = newDirection;
  SetPeltier(newDirection, newPwm);
}

void Thermocycler::SetPlateTarget(double target) {
  iTargetPlateTemp = target;
  if (abs(iPlateTemp - iTargetPlateTemp) > 3)
    iPlatePid.SetTunings(PLATE_PID_FAR_P, PLATE_PID_FAR_I, PLATE_PID_FAR_D);
  else
    iPlatePid.SetTunings(PLATE_PID_NEAR_P, PLATE_PID_NEAR_I, PLATE_PID_NEAR_D);
}

void Thermocycler::ControlLid() {
  double drive = 0;
  
  if (iProgramState == ERunning) {
    iLidPid.Compute();
    drive = iLidPwm;   
  }
   
  analogWrite(3, drive);
}

void Thermocycler::SetPeltier(ThermalDirection dir, int pwm) {
  if (dir == COOL) {
    digitalWrite(2, HIGH);
    digitalWrite(4, LOW);
  } else if (dir == HEAT) {
    digitalWrite(2, LOW);
    digitalWrite(4, HIGH);
  } else {
    digitalWrite(2, LOW);
    digitalWrite(4, LOW);
  }
  
  analogWrite(9, pwm);
}

uint8_t Thermocycler::mcp342xRead(int32_t &data)
{
  // pointer used to form int32 data
  uint8_t *p = (uint8_t *)&data;
  // timeout - not really needed?
  uint32_t start = millis();
  do {
    // assume 18-bit mode
    Wire.requestFrom(MCP3422_ADDRESS, 4);
    if (Wire.available() != 4) {
      return false;
    }
    for (int8_t i = 2; i >= 0; i--) {
      p[i] = Wire.receive();
    }
    // extend sign bits
    p[3] = p[2] & 0X80 ? 0XFF : 0;
    // read config/status byte
    uint8_t s = Wire.receive();
    if ((s & MCP342X_RES_FIELD) != MCP342X_18_BIT) {
      // not 18 bits - shift bytes for 12, 14, or 16 bits
      p[0] = p[1];
      p[1] = p[2];
      p[2] = p[3];
    }
    if ((s & MCP342X_BUSY) == 0) return true;
  } while (millis() - start < 500); //allows rollover of millis()
  return false;
}
//------------------------------------------------------------------------------
// write mcp342x configuration byte
uint8_t Thermocycler::mcp342xWrite(uint8_t config)
{
  Wire.beginTransmission(MCP3422_ADDRESS);
  Wire.send(config);
  Wire.endTransmission();
}
//------------------------------------------------------------------------------
float Thermocycler::TableLookup(const unsigned long lookupTable[], unsigned int tableSize, int startValue, unsigned long searchValue) {
  //simple linear search for now
  int i;
  for (i = 0; i < tableSize; i++) {
    if (searchValue >= pgm_read_dword_near(lookupTable + i))
      break;
  }
  unsigned long high_val = pgm_read_dword_near(lookupTable + i - 1);
  unsigned long low_val = pgm_read_dword_near(lookupTable + i);
  return i + startValue - (float)(searchValue - low_val) / (float)(high_val - low_val);
}
//------------------------------------------------------------------------------
float Thermocycler::TableLookup(const unsigned int lookupTable[], unsigned int tableSize, int startValue, unsigned long searchValue) {
  //simple linear search for now
  int i;
  for (i = 0; i < tableSize; i++) {
    if (searchValue >= pgm_read_word_near(lookupTable + i))
      break;
  }
  unsigned long high_val = pgm_read_word_near(lookupTable + i - 1);
  unsigned long low_val = pgm_read_word_near(lookupTable + i);
  return i + startValue - (float)(searchValue - low_val) / (float)(high_val - low_val);
}
