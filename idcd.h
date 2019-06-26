#ifndef IDCD_H
#define IDCD_H

#include "message.h"

struct Heartbeat : Message{
  uint16_t id = 1;
  uint8_t dlc = 0;

  struct Heartbeat_data{

  };
  Heartbeat_data data;

  Heartbeat(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Heartbeat_Response : Message{
  uint16_t id = 2;
  uint8_t dlc = 0;

  struct Heartbeat_Response_data{

  };
  Heartbeat_Response_data data;

  Heartbeat_Response(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Set_Position : Message{
  uint16_t id = 3;
  uint8_t dlc = 5;

  struct Set_Position_data{
      uint32_t floatPos = 0;
      uint8_t Dimension = 1;
  };
  Set_Position_data data;

  Set_Position(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Set_Joint_Angle : Message{
  uint16_t id = 4;
  uint8_t dlc = 5;

  struct Set_Joint_Angle_data{
      uint32_t floatDegrees = 0;
      uint8_t Joint = 1;
  };
  Set_Joint_Angle_data data;

  Set_Joint_Angle(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Get_Encoder_Status : Message{
  uint16_t id = 5;
  uint8_t dlc = 0;

  struct Get_Encoder_Status_data{

  };
  Get_Encoder_Status_data data;

  Get_Encoder_Status(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Get_Encoder_Status_Response : Message{
  uint16_t id = 6;
  uint8_t dlc = 1;

  struct Get_Encoder_Status_Response_data{
      uint8_t Joint = 0;
  };
  Get_Encoder_Status_Response_data data;

  Get_Encoder_Status_Response(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Get_Joint_Current : Message{
  uint16_t id = 7;
  uint8_t dlc = 1;

  struct Get_Joint_Current_data{
      uint8_t Joint = 1;
  };
  Get_Joint_Current_data data;

  Get_Joint_Current(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Get_Joint_Current_Response : Message{
  uint16_t id = 8;
  uint8_t dlc = 5;

  struct Get_Joint_Current_Response_data{
      uint32_t current = 0;
      uint8_t Joint = 0;
  };
  Get_Joint_Current_Response_data data;

  Get_Joint_Current_Response(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Get_Position : Message{
  uint16_t id = 9;
  uint8_t dlc = 1;

  struct Get_Position_data{
      uint8_t Dimension = 1;
  };
  Get_Position_data data;

  Get_Position(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Get_Position_Response : Message{
  uint16_t id = 10;
  uint8_t dlc = 5;

  struct Get_Position_Response_data{
      uint32_t Position = 0;
      uint8_t Dimension = 0;
  };
  Get_Position_Response_data data;

  Get_Position_Response(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Get_Joint_Angle : Message{
  uint16_t id = 11;
  uint8_t dlc = 1;

  struct Get_Joint_Angle_data{
      uint8_t Joint = 1;
  };
  Get_Joint_Angle_data data;

  Get_Joint_Angle(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Get_Joint_Angle_Response : Message{
  uint16_t id = 12;
  uint8_t dlc = 5;

  struct Get_Joint_Angle_Response_data{
      uint32_t angleBytes = 0;
      uint8_t Joint = 0;
  };
  Get_Joint_Angle_Response_data data;

  Get_Joint_Angle_Response(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Get_Control_Status : Message{
  uint16_t id = 13;
  uint8_t dlc = 0;

  struct Get_Control_Status_data{

  };
  Get_Control_Status_data data;

  Get_Control_Status(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Get_Control_Status_Response : Message{
  uint16_t id = 14;
  uint8_t dlc = 1;

  struct Get_Control_Status_Response_data{
      uint8_t status = 0;
  };
  Get_Control_Status_Response_data data;

  Get_Control_Status_Response(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Auto_Home : Message{
  uint16_t id = 15;
  uint8_t dlc = 0;

  struct Auto_Home_data{

  };
  Auto_Home_data data;

  Auto_Home(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Manual_Home : Message{
  uint16_t id = 16;
  uint8_t dlc = 0;

  struct Manual_Home_data{

  };
  Manual_Home_data data;

  Manual_Home(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

struct Home_Response : Message{
  uint16_t id = 17;
  uint8_t dlc = 0;

  struct Home_Response_data{

  };
  Home_Response_data data;

  Home_Response(){
      this->ID = id;
      this->DLC = dlc;
      this->pData = reinterpret_cast<uint8_t*>(&data);
  }
};

#endif // IDCD_H
