#ifndef _SERIALCONTROL_H_
#define _SERIALCONTROL_H_

#define END_CODE 0

struct PCPPacket {
  enum TPacketType {
    EStatus
  };
  
  PCPPacket(TPacketType type): iType(type) {}
  
  uint8_t iType;
};

struct PCPStatusPacket: PCPPacket {
  PCPStatusPacket(): PCPPacket(EStatus) {}
  
  uint8_t iProgramState; //Thermocycler::ProgramState
  uint16_t iPlateTemp; //C x 10^-1
  uint16_t iLidTemp; //C x 10^-1
  static const uint8_t iEnd = END_CODE;
};

class Thermocycler;

class SerialControl {
public:
  SerialControl(Thermocycler& thermocycler);
  ~SerialControl();
  
  void Process();
  
private:
  void SendStatus();
  
private:
  Thermocycler& iThermocycler;
  unsigned long iLastStatusTime;
};

#endif
