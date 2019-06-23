#ifndef MESSAGE_H
#define MESSAGE_H

#define MAX_MESSAGE_DATA_LENGTH     8  //Bytes
#define MAX_MESSAGE_ASCII_LENGTH    31 //8 bytes in ASCII representation + 7 commas

#define MESSAGE_START_BYTE          ':'
#define MESSAGE_ID_END_BYTE         ' '
#define MESSAGE_FRAMING_BYTE        ','     //Separates ascii data bytes
#define MESSAGE_END_BYTE            ';'

#include <cstdint>

struct Message{
    uint16_t ID     = 0;
    uint8_t DLC     = 0;
    uint8_t *pData;
};

void dump_message(Message m);

#endif // MESSAGE_H
