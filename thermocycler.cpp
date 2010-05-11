
#include "pcr_includes.h"
#include "thermocycler.h"

#include "display.h"
#include "program.h"
#include "serialcontrol.h"
#include "../Wire/Wire.h"

//constants
const unsigned long Thermocycler::PLATE_RESISTANCE_TABLE[] = {
  146735, 138447, 130677, 123390, 116554, 110138, 104113,
  98454, 93137, 88138, 83438, 79016, 74855, 70938, 67249, 63773, 60498, 57410, 54498, 51750, 49157, 46709,
  44397, 42213, 40150, 38199, 36354, 34608, 32957, 31394, 29914, 28512, 27183, 25925, 24731, 23600, 22526,
  21508, 20541, 19623, 18751, 17923, 17136, 16388, 15676, 15000, 14356, 13744, 13161, 12606, 12078, 11574,
  11095, 10637, 10202, 9786, 9389, 9011, 8650, 8306, 7976, 7662, 7362, 7075, 6801, 6539, 6289, 6049, 5820,
  5600, 5391, 5190, 4997, 4813, 4637, 4467, 4305, 4150, 4001, 3858, 3721, 3590, 3464, 3343, 3227, 3115,
  3008, 2905, 2806, 2711, 2620, 2532, 2448, 2367, 2288, 2213, 2141, 2072, 2005, 1940, 1878, 1818, 1761,
  1705, 1652, 1601, 1551, 1503, 1457, 1412, 1369, 1328, 1288, 1250, 1212, 1176, 1142, 1108, 1076, 1045,
  1014 };
  
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
  iPlateTemp(0.0),
  iLidTemp(0.0) {
    
  ipDisplay = new Display(*this);
  ipSerialControl = new SerialControl(*this);
  
  //init pins
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  
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
  
  iPeltierPid.pGain = 100.0;
  iPeltierPid.iGain = 0.000010;
  iPeltierPid.dGain = 100;
  iPeltierPid.iMin = -25500000.0;// / 5;
  iPeltierPid.iMax = 25500000.0;// / 5;
  
//  Wire.begin();
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
  if (iProgramState == EOff)
    return ENoPower;
  
  iProgramState = ERunning;
  iThermalState = EHolding;
  iThermalDirection = OFF;
  iPeltierPwm = 0;
  
  //hack
  iRunning = true;
  iRunTime = 0;
  iStartTime = millis();
  
  ipProgram->BeginIteration();
  ipCurrentStep = ipProgram->GetNextStep();
  return ESuccess;
}

// internal
void Thermocycler::Loop() {
  CheckPower();
  ReadPlateTemp();
  ReadLidTemp();
  ControlPeltier();
  ControlLid();
  ipDisplay->Update();  
  ipSerialControl->Process();
}

void Thermocycler::CheckPower() {
  float voltage = analogRead(0) * 5.0 / 1024 * 10 / 3; // 10/3 is for voltage divider
  boolean externalPower = voltage > 7.0;
  if (externalPower && iProgramState == EOff)
    iProgramState = EStopped;
  else if (!externalPower && iProgramState != EOff) {
    Stop();
    iProgramState = EOff;
  }
}

//private

void Thermocycler::ReadLidTemp() {
  float voltage = analogRead(1) * 5.0 / 1024;
  iLidTemp = (voltage - 0.5) * 100;  
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
  for(int i = 0; i < 4; i++) {
    sprintf(buf, "%d = %u\n", i, spiBuf[i]);
    Serial.print(buf);
  }
//  unsigned long conv = *(unsigned long*)spiBuf;
  unsigned long conv = (((unsigned long)spiBuf[3] >> 7) & 0x01) + ((unsigned long)spiBuf[2] << 1) + ((unsigned long)spiBuf[1] << 9) + (((unsigned long)spiBuf[0] & 0x1F) << 17); //((spiBuf[0] & 0x1F) << 16) + (spiBuf[1] << 8) + spiBuf[2];
  
  unsigned long adcDivisor = 0x1FFFFF;
  float voltage = (float)conv * 5.0 / adcDivisor;
  char floatStr[32];
  sprintFloat(floatStr, voltage, 3);
  sprintf(buf, "float = %s\n", floatStr);
  Serial.print(buf);
  sprintf(buf, "intvoltage = %d\n", (int)voltage);
  Serial.print(buf);

  unsigned int convHigh = (conv >> 16);
  sprintf(buf, "High = %u, low = %u, intvolt = %d\n", convHigh, conv, (int)voltage);
  Serial.print(buf);
  
  digitalWrite(SLAVESELECT, HIGH);
  
  unsigned long r = 53; // hecto ohms
  unsigned long voltage_mv = voltage * 1000;
  unsigned long resistance = voltage_mv * 5300 / (5000 - voltage_mv);
  
  //unsigned long r2 = 22; // hecto ohms
  //unsigned long resistance = (r1 * r2 * (unsigned long)voltage_mv * 100) / (5000 * r2 - (r1 + r2) * (unsigned long)voltage_mv);
 // sprintf(buf, "voltage = %d", voltage_mv);
//  Serial.println(buf);
  sprintf(buf, "resistance = %u", resistance);
  Serial.println(buf);
 
  //simple linear search for now
  int i;
  for (i = 0; i < sizeof(PLATE_RESISTANCE_TABLE) / sizeof(PLATE_RESISTANCE_TABLE[0]); i++) {
    if (resistance >= PLATE_RESISTANCE_TABLE[i])
      break;
  }
  unsigned long high_res = PLATE_RESISTANCE_TABLE[i-1];
  unsigned long low_res = PLATE_RESISTANCE_TABLE[i];
  iPlateTemp = i - 20 - (float)(resistance - low_res) / (float)(high_res - low_res); 

  return;
  //old below
  iPlateTemp = iLidTemp;
  return;
  //config ADC
  int chan = 0;
  int res = 3;
  int gain = 0;
  uint8_t adcConfig = MCP342X_START | MCP342X_CHANNEL_1 | MCP342X_CONTINUOUS;
  adcConfig |= chan << 5 | res << 2 | gain;
  uint16_t mvDivisor = 1 << (gain + 2*res);
  
  mcp342xWrite(adcConfig);
  
  //read voltage
//  Wire.requestFrom(MCP3422_ADDRESS, 3);
//  if (Wire.available() != 3) {
//    Serial.println("Wire.available failed");
//    return;
 // }
  
//  int16_t voltage_mv = (Wire.receive() << 8);
//  voltage_mv |= Wire.receive();


}

void Thermocycler::ControlPeltier() {
  float targetTemp = 99;
  ThermalDirection newDirection;
  int newPwm;
  
  //PID
  double drive = UpdatePID(&iPeltierPid, targetTemp - iPlateTemp, iPlateTemp);
  char buf[32];
  if (drive > 255)
    drive = 255;
  else if (drive < -255)
    drive = -255;
  
  newPwm = abs(drive);
  if (drive > 0)
    newDirection = HEAT;
  else if (drive < 0)
    newDirection = COOL; 
  else
    newDirection = OFF;
    
  //temp
  if (iRunning && iPlateTemp >= targetTemp)
    iRunning = false;
  if (iRunning)
    iRunTime = millis() - iStartTime;
    
  //change power/direction slowly
//  if (newDirection != iThermalDirection)
 //   newPwm = 0;
 // if (newPwm > iPeltierPwm + 5)
  //  newPwm = iPeltierPwm + 5;
    
  //if (newDirection == COOL && newPwm > 200)
    //newPwm = 200;
    
  //update state
  iPeltierPwm = newPwm;
  iThermalDirection = newDirection;
  SetPeltier(newDirection, newPwm);
}

void Thermocycler::ControlLid() {
  float target = 100;
  int newPwm;
 
  if (GetProgramState() == ERunning) {
    if (iLidTemp > target)
      newPwm = 0;
    else
      newPwm = 255;
  } else {
    newPwm = 0;
  }
    
  analogWrite(5, newPwm);
}

void Thermocycler::SetPeltier(ThermalDirection dir, int pwm) {
  if (dir == COOL) {
    digitalWrite(2, LOW);
    digitalWrite(4, HIGH);
  } else if (dir == HEAT) {
    digitalWrite(2, HIGH);
    digitalWrite(4, LOW);
  } else {
    digitalWrite(2, LOW);
    digitalWrite(4, LOW);
  }
  
  analogWrite(3, pwm);
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
