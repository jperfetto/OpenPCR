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

#define BAUD_RATE 9600
#define STATUS_INTERVAL_MS 250

SerialControl::SerialControl(Thermocycler& thermocycler)
: iThermocycler(thermocycler)
, readDataLen(0)
, lastPacketSeq(0xff)
, bStartCodeFound(false)
, bEscapeCodeFound(false)
{  
  Serial.begin(BAUD_RATE);
  iLastStatusTime = millis();
}

SerialControl::~SerialControl() {
}

void SerialControl::Process() {
  ReadPacket();
  if (millis() > iLastStatusTime + STATUS_INTERVAL_MS)
    SendStatus();
}

/////////////////////////////////////////////////////////////////
// Private
void SerialControl::ReadPacket()
{
   char strBuf[10];
  int availableBytes = Serial.available();
  if (availableBytes == 2) {
    strBuf[0] = Serial.read();
    strBuf[1] = Serial.read();
    strBuf[2] = 0;
    int target = atoi(strBuf);
    iThermocycler.GetCurrentStep()->SetTemp(target);
  }
  return;
  /////////////
  if (readDataLen == 0){ //new packet
    //sync with start code
    while (availableBytes){
      byte incomingByte = Serial.read();
      availableBytes--;
      if (bStartCodeFound && incomingByte < MAX_BUFSIZE-PACKET_HEADER_LENGTH+1){
        readDataLen = incomingByte;
        bStartCodeFound = false;
        break;
      }
      else if (incomingByte == START_CODE && bEscapeCodeFound == false)
        bStartCodeFound = true;
      else if (incomingByte == ESCAPE_CODE)
        bEscapeCodeFound = true;
      else
        bEscapeCodeFound = false;
    }
  }
  if (readDataLen <= availableBytes){ //read entire packet payload at once
    boolean bEscapeCode = false;
    byte incomingByte;
    int j=2;
    uint8_t checksum = 0;
    
    buf[0] = START_CODE;
    buf[1] = readDataLen;
    for (int i=0; i <readDataLen-1; i++){
      incomingByte = Serial.read();
      checksum ^= incomingByte;
      if (incomingByte == ESCAPE_CODE)
        bEscapeCode = true;
      else if (bEscapeCode && incomingByte == START_CODE)
        j--; //erase the escape char
      else
        bEscapeCode = false;
      buf[j++] = incomingByte; 
    }
    //checksum
    incomingByte = Serial.read();
    if (incomingByte == checksum){
      ProcessPacket(buf, j);
    }
    readDataLen = 0; //reset, to find START_CODE again
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
  if (packetSeq != lastPacketSeq){ //not retransmission
    switch(packetType){
    case PROGRAM_RUN: 
      Serial.println("Got run");
      break;
    case PROGRAM_STOP:
      iThermocycler.Stop();
      result = true;
      break;
    default:
      break;
    }
    lastPacketSeq = packetSeq;
  }
  SendAck(result);
}
void SerialControl::SendAck(uint8_t result)
{
  PCPAckPacket packet;
  
  packet.eType |= lastPacketSeq; //with correct seq 
  packet.iResult = result;
   
  //send packet
  WritePacket((byte*)&packet, sizeof(packet));
}
void SerialControl::SendStatus()
{
  PCPStatusPacket packet;
  
  packet.iProgramState = iThermocycler.GetProgramState();
  packet.iPlateTemp = htons((iThermocycler.GetPlateTemp() * 10));
  packet.iLidTemp = htons((iThermocycler.GetLidTemp() * 10));
   
  //send packet
  //WritePacket((byte*)&packet, sizeof(packet));
  //Debug("hello");
  //Serial.println("Test1\0");
}
void SerialControl::Debug(char* debugStr)
{
  PCPPacket packetHeader(DEBUG);
  
  //send packet
  WritePacket((byte*)debugStr, strlen(debugStr), (byte*)&packetHeader);
}
