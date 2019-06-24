#ifndef IDCD_H
#define IDCD_H

#include "message.h"

struct Heartbeat : Message{
  uint16_t id = 1;
  uint8_t dlc = 0;

  struct Heartbeat_data{

  };

  Heartbeat(){
      this->ID = id;
      this->pData = reinterpret_cast<uint8_t*>(new Heartbeat_data());
  }
};

struct HeartbeatResponse : Message{
  uint16_t id = 2;
  uint8_t dlc = 0;

  struct HeartbeatResponse_data{

  };

  HeartbeatResponse(){
      this->ID = id;
      this->pData = reinterpret_cast<uint8_t*>(new HeartbeatResponse_data());
  }
};

struct SetX : Message{
  uint16_t id = 3;
  uint8_t dlc = 4;

  struct SetX_data{
      uint32_t floatBytes = 0;
  };

  SetX(){
      this->ID = id;
      this->pData = reinterpret_cast<uint8_t*>(new SetX_data());
  }
};

#endif // IDCD_H
