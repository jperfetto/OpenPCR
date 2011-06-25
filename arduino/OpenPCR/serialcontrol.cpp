/*
 *  serialcontrol.cpp - OpenPCR control software.
 *  Copyright (C) 2010-2011 Josh Perfetto and Xia Hong. All Rights Reserved.
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

SerialControl::SerialControl(Display* pDisplay)
: ipDisplay(pDisplay)
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
        if (packetLen > MAX_COMMAND_SIZE)
          packetLen = MAX_COMMAND_SIZE;
        if (packetLen >= sizeof(struct PCPPacket) && packetLen <= MAX_COMMAND_SIZE) {
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
  char* pCommandBuf;
  
//  if (packetSeq != lastPacketSeq){ //not retransmission
    switch(packetType){
    case SEND_CMD:
      data[datasize] = '\0';
      SCommand command;
      pCommandBuf = (char*)(data + sizeof(PCPPacket));
      
      //store start commands for restart
      ProgramStore::StoreProgram(pCommandBuf);
      
      CommandParser::ParseCommand(command, pCommandBuf);
      GetThermocycler().ProcessCommand(command);
      iCommandId = command.commandId;
      break;
      
    case STATUS_REQ:
      SendStatus();
      break;
    default:
      break;
   }

    lastPacketSeq = packetSeq;
//  }
}

#define STATUS_FILE_LEN 128

void SerialControl::SendStatus()
{
  char* szStatus;
  Thermocycler::ProgramState state = GetThermocycler().GetProgramState();
  switch (state) {
  case Thermocycler::EOff:
  case Thermocycler::EStopped:
    szStatus = "stopped";
    break;
  case Thermocycler::ELidWait:
    szStatus = "lidwait";
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
  
  char* szThermState = "\0";
  switch (GetThermocycler().GetThermalState()) {
  case Thermocycler::EHeating:
    szThermState = "heating";
    break;
  case Thermocycler::ECooling:
    szThermState = "cooling";
    break;
  case Thermocycler::EHolding:
    szThermState = "holding";
    break;
  case Thermocycler::EIdle:
    szThermState = "idle";
    break;
  }
      
  char statusBuf[STATUS_FILE_LEN];
  char* statusPtr = statusBuf;
  Thermocycler& tc = GetThermocycler();
  statusPtr = AddParam(statusPtr, 'd', (unsigned long)iCommandId, true);
  statusPtr = AddParam(statusPtr, 's', szStatus);
  statusPtr = AddParam(statusPtr, 'l', (int)tc.GetLidTemp());
  statusPtr = AddParam(statusPtr, 'b', tc.GetPlateTemp(), 1, false);
  statusPtr = AddParam(statusPtr, 't', szThermState);

  if (state == Thermocycler::ERunning || state == Thermocycler::EComplete) {
    statusPtr = AddParam(statusPtr, 'e', tc.GetElapsedTimeS());
    statusPtr = AddParam(statusPtr, 'r', tc.GetTimeRemainingS());
    statusPtr = AddParam(statusPtr, 'u', tc.GetNumCycles());
    statusPtr = AddParam(statusPtr, 'c', tc.GetCurrentCycleNum());
    statusPtr = AddParam(statusPtr, 'n', tc.GetProgName());
    if (tc.GetCurrentStep() != NULL)
      statusPtr = AddParam(statusPtr, 'p', tc.GetCurrentStep()->GetName());
  }
  statusPtr++; //to include null terminator
  
  //send packet
  PCPPacket packet(STATUS_RESP);
  packet.length = sizeof(packet) + STATUS_FILE_LEN;
  Serial.write((byte*)&packet, sizeof(packet));
  int statusBufLen = statusPtr - statusBuf;
  Serial.write((byte*)statusBuf, statusBufLen);
  for (int i = statusBufLen; i < STATUS_FILE_LEN; i++)
    Serial.write(0x20);
}

char* SerialControl::AddParam(char* pBuffer, char key, int val, boolean init) {
  if (!init)
    *pBuffer++ = '&';
  *pBuffer++ = key;
  *pBuffer++ = '=';
  itoa(val, pBuffer, 10);
  while (*pBuffer != '\0')
    pBuffer++;
    
  return pBuffer;
}

char* SerialControl::AddParam(char* pBuffer, char key, unsigned long val, boolean init) {
  if (!init)
    *pBuffer++ = '&';
  *pBuffer++ = key;
  *pBuffer++ = '=';
  ltoa(val, pBuffer, 10);
  while (*pBuffer != '\0')
    pBuffer++;
    
  return pBuffer;
}

char* SerialControl::AddParam(char* pBuffer, char key, float val, int decimalDigits, boolean pad, boolean init) {
  if (!init)
    *pBuffer++ = '&';
  *pBuffer++ = key;
  *pBuffer++ = '=';
  sprintFloat(pBuffer, val, decimalDigits, pad);
  while (*pBuffer != '\0')
    pBuffer++;
    
  return pBuffer;
}

char* SerialControl::AddParam(char* pBuffer, char key, const char* szVal, boolean init) {
  if (!init)
    *pBuffer++ = '&';
  *pBuffer++ = key;
  *pBuffer++ = '=';
  strcpy(pBuffer, szVal);
  while (*pBuffer != '\0')
    pBuffer++;
    
  return pBuffer;
}
