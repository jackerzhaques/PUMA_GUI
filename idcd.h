#ifndef IDCD_H
#define IDCD_H

#include "message.h"

struct Heartbeat : Message{
    uint16_t id = 1;
    uint8_t dlc = 0;
    struct Heartbeat_data{
    };

    Heartbeat(){
        this->ID = ID;
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

#endif // IDCD_H
