/*
 *	serialcontrol.h - OpenPCR control software.
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

#ifndef _SERIALCONTROL_H_
#define _SERIALCONTROL_H_

#define START_CODE    0xFF
#define ESCAPE_CODE   0xFE
#define PACKET_HEADER_LENGTH  3

#define MAX_BUFSIZE   128 //516

typedef enum {
    SEND_CMD       = 0x10,
    STATUS_REQ     = 0x40,
    STATUS_RESP    = 0x80
} PACKET_TYPE;
  
struct SCommand {
  char name[21];
  uint16_t commandId;
  enum TCommandType {
    ENone = 0,
    EStart,
    EStop
  } command;
  int lidTemp;
};

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

class Thermocycler;
class Display;

class SerialControl {
public:
  SerialControl(Thermocycler& thermocycler, Display* pDisplay);
  ~SerialControl();
  
  void Process();
  
private:
  void ReadPacket();
  void WritePacket(byte* data, int datasize, byte* header=NULL);
  void ProcessPacket(byte* data, int datasize);
  void SendStatus();
  void ParseCommand(char* pCommandBuf);
  void AddCommand(SCommand* pCommand, char key, const char* szValue);
  void ProcessCommand(SCommand* pCommand);
  
private:
  byte buf[MAX_BUFSIZE]; //read or write buffer
  
  typedef enum{
    STATE_START,
    STATE_STARTCODE_FOUND,
    STATE_PACKETLEN_LOW,
    STATE_PACKETHEADER_DONE
  }PACKET_STATE;
  
  PACKET_STATE packetState;
  uint8_t lastPacketSeq;
  uint16_t packetLen, iCommandId;
  boolean bEscapeCodeFound;
  
  Thermocycler& iThermocycler;
  Display* ipDisplay;
};

#endif
