/*
 *	serialcontrol.cpp - OpenPCR control software.
 *  Copyright (C) 2010 Josh Perfetto and Xia Hong. All Rights Reserved.
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
#include "serialcontrol.h"

#include "thermocycler.h"
#include "program.h"
#include "display.h"

#define BAUD_RATE 9600
#define STATUS_INTERVAL_MS 250

SerialControl::SerialControl(Thermocycler& thermocycler, Display* pDisplay)
: iThermocycler(thermocycler)
, ipDisplay(pDisplay)
, packetState(STATE_START)
, packetLen(0)
, packetRealLen(0)
, lastPacketSeq(0xff)
, bEscapeCodeFound(false)
, iCommandId(0)
{  
  Serial.begin(BAUD_RATE);
}

SerialControl::~SerialControl() {
}

void SerialControl::Process() {
  ReadPacket();
}

/////////////////////////////////////////////////////////////////
// Private
void SerialControl::ReadPacket()
{
  char dbuf[10];
  int availableBytes = Serial.available();
 
  if (packetState < STATE_PACKETHEADER_DONE){ //new packet
    //sync with start code
    while (availableBytes){
      byte incomingByte = Serial.read();
      availableBytes--;
      if (packetState == STATE_STARTCODE_FOUND){
        packetLen = incomingByte;
        packetState = STATE_PACKETLEN_LOW;
      } 
      else if (packetState == STATE_PACKETLEN_LOW) {
        packetLen |= incomingByte << 8;
        if (packetLen > MAX_BUFSIZE)
          packetLen = MAX_BUFSIZE;
        if (packetLen >= sizeof(struct PCPPacket) && packetLen <= MAX_BUFSIZE){
          packetState = STATE_PACKETHEADER_DONE;
          buf[0] = START_CODE;
          buf[1] = packetLen & 0xff;
          buf[2] = (packetLen & 0xff00)>>8;
          bEscapeCodeFound = false;
          checksum = 0;
          packetRealLen = 3;
          packetLen -= 3;
        }
        else{
          packetState = STATE_START; //reset
        }
        break;
      } 
      else if (incomingByte == START_CODE && bEscapeCodeFound == false)
        packetState = STATE_STARTCODE_FOUND;
      else if (incomingByte == ESCAPE_CODE)
        bEscapeCodeFound = true;
      else
        bEscapeCodeFound = false;
    }
  }
  
  if (packetState == STATE_PACKETHEADER_DONE){
    while(availableBytes > 0 && packetLen > 0){
      byte incomingByte = Serial.read();
      availableBytes--;
      packetLen--;
      checksum ^= incomingByte;
      if (incomingByte == ESCAPE_CODE)
        bEscapeCodeFound = true;
      else if (bEscapeCodeFound && incomingByte == START_CODE)
        packetRealLen--; //erase the escape char
      else
        bEscapeCodeFound = false;
      buf[packetRealLen++] = incomingByte; 
    }
    
    if (packetLen == 0){
      //checksum
      //if (incomingByte == checksum){
        ProcessPacket(buf, packetRealLen);
      //  }
  
      //reset, to find START_CODE again
      packetState = STATE_START;
    }
  }
}

void SerialControl::ProcessPacket(byte* data, int datasize)
{
  PCPPacket* packet = (PCPPacket*)data;
  uint8_t packetType = packet->eType & 0xf0;
  uint8_t packetSeq = packet->eType & 0x0f;
  uint8_t result = false;
  int len;
  
//  if (packetSeq != lastPacketSeq){ //not retransmission
    switch(packetType){
    case SEND_CMD:
      data[datasize] = '\0';
      ParseCommand((char*)(data + sizeof(PCPPacket)));
      break;
    case STATUS_REQ:
      SendStatus();
      break;
    default:
      break;
 //   }
    lastPacketSeq = packetSeq;
  }
}

#define STATUS_FILE_LEN 295

void SerialControl::SendStatus()
{
  char* szStatus;
  Thermocycler::ProgramState state = iThermocycler.GetProgramState();
  switch (state) {
  case Thermocycler::EOff:
  case Thermocycler::EStopped:
    szStatus = "stopped";
    break;
  case Thermocycler::ERunning:
    szStatus = "running";
    break;
  case Thermocycler::EComplete:
    szStatus = "complete";
    break;
  case Thermocycler::EError:
  default:
    szStatus = "error";
  }
  
  char* szThermState;
  if (iThermocycler.Ramping()) {
    if (iThermocycler.GetThermalDirection() == Thermocycler::HEAT)
      szThermState = "heating";
    else
      szThermState = "cooling";
  } else {
    szThermState = "holding";
  }
  
  char blockTempStr[8];
  sprintFloat(blockTempStr, iThermocycler.GetPlateTemp(), 1, false);
    
  char statusBuf[128];
  char* statusPtr = statusBuf;
  int lidTemp = iThermocycler.GetLidTemp();
  statusPtr += sprintf(statusPtr, "d=%u", iCommandId);
  statusPtr += sprintf(statusPtr, "&s=%s", szStatus);
  statusPtr += sprintf(statusPtr, "&l=%d", lidTemp);
  statusPtr += sprintf(statusPtr, "&b=%s", blockTempStr);
  statusPtr += sprintf(statusPtr, "&t=%s", szThermState);

  if (state == Thermocycler::ERunning || Thermocycler::EComplete) {
    statusPtr += sprintf(statusPtr, "&e=%u", iThermocycler.GetElapsedTimeS());
    statusPtr += sprintf(statusPtr, "&r=%u", iThermocycler.GetTimeRemainingS());
    statusPtr += sprintf(statusPtr, "&u=%d", iThermocycler.GetNumCycles());
    statusPtr += sprintf(statusPtr, "&c=%d", iThermocycler.GetCurrentCycleNum());
    statusPtr += sprintf(statusPtr, "&n=%s", iThermocycler.GetProgName());
    statusPtr += sprintf(statusPtr, "&p=%s", iThermocycler.GetCurrentStep()->GetName());
  }
  
  //send packet
  PCPPacket packet(STATUS_RESP);
  packet.length = sizeof(packet) + STATUS_FILE_LEN;
  Serial.write((byte*)&packet, sizeof(packet));
  Serial.write((byte*)statusBuf, strlen(statusBuf));
  for (int i = strlen(statusBuf); i < STATUS_FILE_LEN; i++)
    Serial.write(0x20);
}

void SerialControl::ParseCommand(char* pCommandBuf) {
  char* pValue;
  SCommand command;
  memset(&command, NULL, sizeof(command));
  
  char buf[32];
    
  char* pParam = strtok(pCommandBuf, "&");
  while (pParam) {  
    pValue = strchr(pParam, '=');
    *pValue++ = '\0';
    AddCommand(&command, pParam[0], pValue);
    pParam = strtok(NULL, "&");
  }

  ProcessCommand(&command);
}

void SerialControl::ProcessCommand(SCommand* pCommand) {
  if (pCommand->command == SCommand::EStart) {
    iThermocycler.Stop();
    
    //find display cycle
    Cycle* pProgram = pCommand->pProgram;
    Cycle* pDisplayCycle = pProgram;
    
    for (int i = 0; i < pProgram->GetNumComponents(); i++) {
      ProgramComponent* pComp = pProgram->GetComponent(i);
      if (pComp->GetType() == ProgramComponent::ECycle) {
        pDisplayCycle = (Cycle*)pComp;
        break;
      }
    }
    
    //start program
    iThermocycler.SetProgram(pProgram, pDisplayCycle, pCommand->name, pCommand->lidTemp);
    iCommandId = pCommand->commandId;
    iThermocycler.Start();
    
  } else if (pCommand->command == SCommand::EStop) {
    iThermocycler.Stop();
  }
}

void SerialControl::AddCommand(SCommand* pCommand, char key, char* szValue) {
  switch(key) {
  case 'n':
    strncpy(pCommand->name, szValue, sizeof(pCommand->name) - 1);
    pCommand->name[sizeof(pCommand->name) - 1] = '\0';
    break;
  case 'c':
    if (strcmp(szValue, "start") == 0)
      pCommand->command = SCommand::EStart;
    else if (strcmp(szValue, "stop") == 0)
      pCommand->command = SCommand::EStop;
    break;
  case 't':
    //just set contrast right now, not part of a larger program
    ipDisplay->SetContrast(atoi(szValue));
    break;
  case 'l':
    pCommand->lidTemp = atoi(szValue);
    break;
  case 'd':
    pCommand->commandId = atoi(szValue);
    break;
  case 'p':
    pCommand->pProgram = ParseProgram(szValue);
    break;
  }
}

Cycle* SerialControl::ParseProgram(char* pBuffer) {
  Cycle* pProgram = new Cycle(1);
	
  char* pCycBuf = strtok(pBuffer, "()");
  while (pCycBuf != NULL) {
    pProgram->AddComponent(ParseCycle(pCycBuf));
    pCycBuf = strtok(NULL, "()");
  }
  
  return pProgram;
}

//(1[300|95|IDenaturing])
ProgramComponent* SerialControl::ParseCycle(char* pBuffer) {
  char countBuf[5];
	
  //find first step
  char* pStep = strchr(pBuffer, '[');
	
  //get cycle count
  int countLen = pStep - pBuffer;
  strncpy(countBuf, pBuffer, countLen);
  countBuf[countLen] = '\0';
  int cycCount = atoi(countBuf);
  
  Cycle* pCycle = NULL;
  if (cycCount > 1)	
    pCycle = new Cycle(cycCount);
	
  //add steps
  while (pStep != NULL) {
    *pStep++ = '\0';
    char* pStepEnd = strchr(pStep, ']');
    *pStepEnd++ = '\0';

    Step* pNewStep = ParseStep(pStep);
    if (pCycle != NULL)
      pCycle->AddComponent(pNewStep);
    else
      return pNewStep;
    pStep = strchr(pStepEnd, '[');
  }
  
  return pCycle;
}

Step* SerialControl::ParseStep(char* pBuffer) {
  char* pTemp = strchr(pBuffer, '|');
  *pTemp++ = '\0';
  char* pName = strchr(pTemp, '|');
  *pName++ = '\0';
  char* pEnd = strchr(pName, ']');
  *pEnd = '\0';
	
  int duration = atoi(pBuffer);
  float temp = atof(pTemp);
	
  return new Step(pName, duration, temp);
}
