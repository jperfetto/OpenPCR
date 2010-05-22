#ifndef _SERIALCONTROL_H_
#define _SERIALCONTROL_H_

#define START_CODE    0xFF
#define ESCAPE_CODE   0xFE
#define PACKET_HEADER_LENGTH  3

#define MAX_BUFSIZE   128

typedef enum {
    PROGRAM_RUN    = 0x10,
    PROGRAM_STOP   = 0x20,
    ACK            = 0x30,
    STATUS         = 0x40,
    DEBUG          = 0x50
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

//packet payload
struct PCPStatusPacket : PCPPacket {
  PCPStatusPacket(): PCPPacket(STATUS) {}
  
  uint8_t iProgramState; //Thermocycler::ProgramState
  uint16_t iPlateTemp; //C x 10^-1
  uint16_t iLidTemp; //C x 10^-1
};

struct PCPAckPacket : PCPPacket {
  PCPAckPacket(): PCPPacket(ACK) {}
  
  uint8_t iResult;
};

class Thermocycler;

class SerialControl {
public:
  SerialControl(Thermocycler& thermocycler);
  ~SerialControl();
  
  void Process();
  void Debug(char* debugStr);
private:
  void ReadPacket();
  void WritePacket(byte* data, int datasize, byte* header=NULL);
  void ProcessPacket(byte* data, int datasize);
  void SendAck(uint8_t result);
  void SendStatus();
  
private:
  byte buf[MAX_BUFSIZE]; //read or write buffer
  uint8_t readDataLen, lastPacketSeq;
  boolean bStartCodeFound, bEscapeCodeFound;
  
  Thermocycler& iThermocycler;
  unsigned long iLastStatusTime;
};

#endif
