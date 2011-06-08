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

#define MAX_BUFSIZE   128

typedef enum {
    PROGRAM_RUN    = 0x10,
    PROGRAM_STOP   = 0x20,
    STATUS_REQ     = 0x40,
    STATUS_RESP    = 0x80
}PACKET_TYPE;
  
//packet header
struct PCPPacket {
  PCPPacket(PACKET_TYPE type)
  : startCode(START_CODE)
  , length(0)
  , eType(type)
  {}

  uint8_t startCode;
  uint8_t length;
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
  
private:
  byte buf[MAX_BUFSIZE]; //read or write buffer
  uint8_t readDataLen, lastPacketSeq;
  boolean bStartCodeFound, bEscapeCodeFound;
  
  Thermocycler& iThermocycler;
  Display* ipDisplay;
};

#endif
