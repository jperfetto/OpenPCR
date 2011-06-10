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
, lastPacketSeq(0xff)
, bEscapeCodeFound(false)
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
  char strBuf[10];
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
        packetState = STATE_PACKETHEADER_DONE;
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
  
  if (packetState == STATE_PACKETHEADER_DONE && packetLen > 0 && (packetLen - 3) <= availableBytes){ //read entire packet payload at once
    boolean bEscapeCode = false;
    byte incomingByte;
    int payload_size = 3;
    uint8_t checksum = 0;
    
    buf[0] = START_CODE;
    buf[1] = packetLen & 0xff;
    buf[2] = (packetLen & 0xff00)>>8;
    for (int i = 3; i < packetLen; i++){
      incomingByte = Serial.read();
      checksum ^= incomingByte;
      if (incomingByte == ESCAPE_CODE)
        bEscapeCode = true;
      else if (bEscapeCode && incomingByte == START_CODE)
        payload_size--; //erase the escape char
      else
        bEscapeCode = false;
      buf[payload_size++] = incomingByte; 
    }
    //checksum
  //  incomingByte = Serial.read();
  //  if (incomingByte == checksum){
      ProcessPacket(buf, payload_size);
  //  }
  
    //reset, to find START_CODE again
    packetState = STATE_START;
  }
}
void SerialControl::WritePacket(byte* data, int datasize, byte* header)
{
  int j=0;
  uint8_t checksum = 0;
  
  if (header == NULL){
    header = data;
    data = &data[PACKET_HEADER_LENGTH];
    datasize -= PACKET_HEADER_LENGTH;
  }
  
  //checksum the header
  for(int i=0; i<PACKET_HEADER_LENGTH; i++){
    buf[j] = header[i];
    if (i != 0) //don't checksum START_CODE
      checksum ^= buf[j];
    j++;
  }
  //escape the payload and checksum the payload
  for(int i=0; i<datasize; i++){
    if (data[i] == START_CODE){
      buf[j] = ESCAPE_CODE;
      checksum ^= buf[j];
      j++;
    }
    buf[j] = data[i]; 
    checksum ^= buf[j];
    j++;
  }
  //check sum
  buf[j] = checksum;
  j++;
  //set datasize
  datasize = j;
  //set length
  buf[1] = datasize-PACKET_HEADER_LENGTH+1;
  
  Serial.write(buf, datasize);
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
      ipDisplay->SetDebugMsg("Got Command");
      data[datasize - sizeof(PCPPacket)] = '\0';
      ParseCommand((char*)(data + sizeof(PCPPacket)));
      break;
    case STATUS_REQ:
      ipDisplay->SetDebugMsg("Got Status");
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
 // ipDisplay->SetDebugMsg("Sending Status");
  
  // temp
  iCommandId = 65535;
  
  char* szStatus;
  switch (iThermocycler.GetProgramState()) {
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
  char progName[32] = "No Name";
    
  char statusBuf[256];
  char* statusPtr = statusBuf;
  int lidTemp = iThermocycler.GetLidTemp();
  statusPtr += sprintf(statusPtr, "cmdId=%u", iCommandId);
  statusPtr += sprintf(statusPtr, "&status=%s", szStatus);
  statusPtr += sprintf(statusPtr, "&timeElapsed=%u", iThermocycler.GetElapsedTimeS());
  statusPtr += sprintf(statusPtr, "&timeRemaining=%u", iThermocycler.GetTimeRemainingS());
  statusPtr += sprintf(statusPtr, "&lidTemp=%d", lidTemp);
  statusPtr += sprintf(statusPtr, "&blockTemp=%s", blockTempStr);
  statusPtr += sprintf(statusPtr, "&numCycle=%d", iThermocycler.GetNumCycles());
  statusPtr += sprintf(statusPtr, "&curCycles=%d", iThermocycler.GetCurrentCycleNum());
  statusPtr += sprintf(statusPtr, "&progName=%s", iThermocycler.GetProgName());
  statusPtr += sprintf(statusPtr, "&stepName=%s", iThermocycler.GetCurrentStep()->GetName());
  statusPtr += sprintf(statusPtr, "&thermState=%s", szThermState);
  
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
    //create program
    Cycle* pMaster = new Cycle(1);
    pMaster->AddComponent(new Step("IDenaturing", 30, 95));
    Cycle* pMain = new Cycle(35);
    pMain->AddComponent(new Step("Denaturing", 30, 95));
    pMain->AddComponent(new Step("Annealing", 30, 55));
    pMain->AddComponent(new Step("Extending", 30, 72));
    pMaster->AddComponent(pMain);
    pMaster->AddComponent(new Step("FExtending", 30, 72));
    pMaster->AddComponent(new Step("Holding", 0, 4));
  
    iCommandId = pCommand->commandId;
    iThermocycler.SetProgram(pMaster, pMain, pCommand->name, pCommand->lidTemp);
  } else if (pCommand->command == SCommand::EStop) {
    iThermocycler.Stop();
  }
}

void SerialControl::AddCommand(SCommand* pCommand, char key, const char* szValue) {
  switch(key) {
  case 'n':
    strncpy(pCommand->name, szValue, sizeof(pCommand->name));
    pCommand->name[sizeof(pCommand->name)] = '\0';
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
  }
}
