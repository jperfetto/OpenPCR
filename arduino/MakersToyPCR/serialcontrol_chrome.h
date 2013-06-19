/*
 *  serialcontrol_chrome.h - OpenPCR control software.
 *  Copyright (C) 2010-2012 Josh Perfetto and Xia Hong. All Rights Reserved.
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

#ifndef _SERIALCONTROL_H_
#define _SERIALCONTROL_H_

#include "thermocycler.h"

#define START_CODE    0xFF
#define ESCAPE_CODE   0xFE

class Display;
class ProgramComponent;
class Cycle;
class Step;
struct SCommand;

typedef enum {
    SEND_CMD       = 0x10,
    STATUS_REQ     = 0x40,
    STATUS_RESP    = 0x80
} PACKET_TYPE;

//packet header
struct PCPPacket {
  PCPPacket(PACKET_TYPE type)
  : startCode(START_CODE)
  , length(0)
  , eType(type)
  {}

  uint8_t startCode;
  uint16_t length;
  uint8_t eType; //lower 4 bits are used for seq
};

class SerialControl {
public:
  SerialControl(Display* pDisplay);
  ~SerialControl();
  
  void Process();
  byte* GetBuffer() { return buf; } //used for stored program parsing at start-up only if no serial command received
  boolean CommandReceived() { return iReceivedStatusRequest; }
  
private:
  boolean ReadPacket(); //returns true if bytes were read
  void ProcessPacket();
  void finishReading ();
  void SendStatus();

  char* AddParam(char* pBuffer, char key, int val, boolean init = false);  
  char* AddParam(char* pBuffer, char key, unsigned long val, boolean init = false);
  char* AddParam(char* pBuffer, char key, float val, int decimalDigits, boolean pad, boolean init = false);
  char* AddParam(char* pBuffer, char key, const char* szVal, boolean init = false);
  char* AddParam_P(char* pBuffer, char key, const char* szVal, boolean init = false);
  
  const char* GetProgramStateString_P(Thermocycler::ProgramState state);
  const char* GetThermalStateString_P(Thermocycler::ThermalState state);
  
private:
  byte buf[MAX_COMMAND_SIZE + 1]; //read or write buffer
  
  typedef enum{
    STATE_START,
    STATE_STARTCODE_FOUND,
    STATE_PACKETLEN_LOW,
    STATE_PACKETHEADER_DONE
  }PACKET_STATE;
  
  PACKET_STATE packetState;
  uint8_t lastPacketSeq;
  uint16_t packetLen, packetRealLen, iCommandId;
  boolean bEscapeCodeFound;
  boolean iReceivedStatusRequest;
  
  Display* ipDisplay;
};

#endif
