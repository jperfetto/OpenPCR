#include "pcr_includes.h"
#include "serialcontrol.h"

#include "thermocycler.h"

#define BAUD_RATE 19200
#define STATUS_INTERVAL_MS 250

SerialControl::SerialControl(Thermocycler& thermocycler):
  iThermocycler(thermocycler) {
  
  Serial.begin(BAUD_RATE);
  iLastStatusTime = millis();
}

SerialControl::~SerialControl() {
}

void SerialControl::Process() {
//  if (millis() > iLastStatusTime + STATUS_INTERVAL_MS)
//    SendStatus();
}

/////////////////////////////////////////////////////////////////
// Private
void SerialControl::SendStatus() {
  PCPStatusPacket packet;
  
  packet.iProgramState = iThermocycler.GetProgramState();
  packet.iPlateTemp = htons((iThermocycler.GetPlateTemp() * 10));
  packet.iLidTemp = htons((iThermocycler.GetLidTemp() * 10));
  
  Serial.write((byte*)&packet, sizeof(packet));
}
